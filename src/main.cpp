/*
 * Homie for SonOff relay.
 */

#include <Homie.h>
#include <EEPROM.h>
#include <Bounce2.h>


#ifdef SONOFF
// sonoff
#define PIN_RELAY 12
#define PIN_LED 13
#define PIN_BUTTON 0
#define RELAY_ON_STATE HIGH
#define RELAY_OFF_STATE LOW
#endif

#ifdef GENERICRELAY
// esp01 generic relay with different pin configuration
#define PIN_RELAY 2
#define PIN_BUTTON 0
//#define PIN_LED 1
#define RELAY_ON_STATE LOW
#define RELAY_OFF_STATE HIGH
#endif

unsigned long downCounterStart;
unsigned long downCounterLimit=0;
unsigned long watchDogCounterStart=0;

unsigned long keepAliveReceived=0;
int lastButtonValue = 1;

// EEPROM structure
struct EEpromDataStruct {
  int keepAliveTimeOut; // 0 - disabled, keepalive time - seconds
  bool initialState;  // Initial state (just after boot - homie independet)
  int watchDogTimeOut; // 0 - disabled, watchdog time limit - seconds
};

EEpromDataStruct EEpromData;

Bounce debouncerButton = Bounce();

HomieNode relayNode("relay01", "relay");
HomieNode keepAliveNode("keepalive", "keepalive");
HomieNode watchDogNode("watchdog", "Watchdog mode");

HomieSetting<bool> reverseMode("reverse mode", "Relay reverse mode");

/*
 * Recevied tick message for watchdog
 */
bool watchdogTickHandler(const HomieRange& range, const String& value)
{
  if (value == "0")
  {
    watchDogCounterStart = 0;
  } else {
    watchDogCounterStart = millis();
  }
  return true;
}
/*
 * Received watchdog timeout value
 */
bool watchdogTimeOutHandler(const HomieRange& range, const String& value)
{
  int oldValue = EEpromData.watchDogTimeOut;
  if (value.toInt() > 15)
  {
    EEpromData.watchDogTimeOut = value.toInt();
  }
  if (value=="0")
  {
    EEpromData.watchDogTimeOut = 0;
  }

  if (oldValue!=EEpromData.watchDogTimeOut)
  {
    String outMsg = String(EEpromData.watchDogTimeOut);
    watchDogNode.setProperty("timeOut").send(outMsg);
    EEPROM.put(0, EEpromData);
    EEPROM.commit();
  }
}
/*
 *
 */
bool relayStateHandler(const HomieRange& range, const String& value)
{
  if (value == "ON") {
    if (reverseMode.get())
    {
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
    } else {
      digitalWrite(PIN_RELAY, RELAY_ON_STATE);
    }
    #ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
    #endif
    relayNode.setProperty("relayState").send("ON");
  } else if (value == "OFF") {
    if (reverseMode.get())
    {
      digitalWrite(PIN_RELAY, RELAY_ON_STATE);
    } else {
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
    }
    #ifdef PIN_LED
    digitalWrite(PIN_LED, HIGH);
    #endif
    relayNode.setProperty("relayState").send("OFF");
  } else {
    return false;
  }
  return true;
}

/*
 * Keepliave tick handler
 */
bool keepAliveTickHandler(HomieRange range, String value)
{
  keepAliveReceived=millis();
  return true;
}

/*
 * Keepalive message handler
 */
bool keepAliveTimeOutHandler(HomieRange range, String value)
{
  int oldValue = EEpromData.keepAliveTimeOut;
  if (value.toInt() > 0)
  {
    EEpromData.keepAliveTimeOut = value.toInt();
  }
  if (value=="0")
  {
    EEpromData.keepAliveTimeOut = 0;
  }
  if (oldValue!=EEpromData.keepAliveTimeOut)
  {
    String outMsg = String(EEpromData.keepAliveTimeOut);
    keepAliveNode.setProperty("timeOut").send(outMsg);
    EEPROM.put(0, EEpromData);
    EEPROM.commit();
  }
}

/*
 * Timer handler
 */
bool relayTimerHandler(HomieRange range, String value)
{
  if (value.toInt() > 0)
  {
    if (reverseMode.get())
    {
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
    } else {
      digitalWrite(PIN_RELAY, RELAY_ON_STATE);
    }
    #ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
    #endif
    downCounterStart = millis();
    downCounterLimit = value.toInt()*1000;
    relayNode.setProperty("relayState").send("ON");
    relayNode.setProperty("relayTimer").send(value);
    return true;
  } else {
    return false;
  }
}

/*
 * Initial mode handler
 */
bool relayInitModeHandler(HomieRange range, String value)
{
  int oldValue = EEpromData.initialState;
  if (value.toInt() == 1 or value=="ON")
  {
    relayNode.setProperty("relayInitMode").send("1");
    EEpromData.initialState=1;
  } else {
    relayNode.setProperty("relayInitMode").send( "0");
    EEpromData.initialState=0;
  }
  if (oldValue!=EEpromData.initialState)
  {
    EEPROM.put(0, EEpromData);
    EEPROM.commit();
  }
  return true;
}

/*
 * Homie setup handler
 */
void setupHandler()
{
  HomieRange emptyRange;
  if (EEpromData.initialState)
  {
    if (reverseMode.get())
      relayStateHandler(emptyRange, "OFF");
    else
      relayStateHandler(emptyRange, "ON");
    relayNode.setProperty("relayInitMode").send("1");
  } else {
    if (reverseMode.get())
      relayStateHandler(emptyRange, "ON");
    else
      relayStateHandler(emptyRange, "OFF");

    relayNode.setProperty("relayInitMode").send("0");
  }
  String outMsg = String(EEpromData.keepAliveTimeOut);
  keepAliveNode.setProperty("timeOut").send(outMsg);
  outMsg = EEpromData.watchDogTimeOut;
  watchDogNode.setProperty("timeOut").send(outMsg);
  keepAliveReceived=millis();
}


/*
 * Homie loop handler
 */
void loopHandler()
{
  if (downCounterLimit>0)
  {
    if ((millis() - downCounterStart ) > downCounterLimit)
    {
      // Turn off relay
      if (reverseMode.get())
      {
        digitalWrite(PIN_RELAY, RELAY_ON_STATE);
      } else {
        digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
      }
      #ifdef PIN_LED
      digitalWrite(PIN_LED, HIGH);
      #endif
      relayNode.setProperty("relayState").send("OFF");
      relayNode.setProperty("relayTimer").send("0");
      downCounterLimit=0;
    }
  }
  int buttonValue = debouncerButton.read();

  if (buttonValue != lastButtonValue)
  {
    lastButtonValue = buttonValue;
    int relayValue = digitalRead(PIN_RELAY);
    if (buttonValue == HIGH)
    {
      HomieRange emptyRange;
      if (
          ((!reverseMode.get()) && relayValue == RELAY_ON_STATE) ||
          (( reverseMode.get()) && relayValue == RELAY_OFF_STATE) )
      {
        relayStateHandler(emptyRange, "OFF");
      } else {
        relayStateHandler(emptyRange, "ON");
      }
    }
  }
  debouncerButton.update();

  // Check if keepalive is supported and expired
  if (EEpromData.keepAliveTimeOut != 0 && (millis() - keepAliveReceived) > EEpromData.keepAliveTimeOut*1000 )
  {
    ESP.restart();
  }
  if (watchDogCounterStart!=0 && EEpromData.watchDogTimeOut!=0 && (millis() - watchDogCounterStart) > EEpromData.watchDogTimeOut * 1000 )
  {
    HomieRange emptyRange;
    relayTimerHandler(emptyRange, "10"); // Disable relay for 10 sec
    watchDogCounterStart = millis();
  }
}

/*
 * Homie event handler
 */
void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::CONFIGURATION_MODE: // Default eeprom data in configuration mode
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
      EEpromData.keepAliveTimeOut = 0;
      EEpromData.initialState = false;
      EEpromData.watchDogTimeOut = 0;
      EEPROM.put(0, EEpromData);
      EEPROM.commit();
      break;
    case HomieEventType::NORMAL_MODE:
      // Do whatever you want when normal mode is started
      break;
    case HomieEventType::OTA_STARTED:
      // Do whatever you want when OTA mode is started
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
      break;
    case HomieEventType::ABOUT_TO_RESET:
      // Do whatever you want when the device is about to reset
      break;
    case HomieEventType::WIFI_CONNECTED:
      // Do whatever you want when Wi-Fi is connected in normal mode
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      // Do whatever you want when Wi-Fi is disconnected in normal mode
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      // Do whatever you want when MQTT is disconnected in normal mode
      break;
  }
}

/*
 * Main setup
 */
void setup()
{
  #ifdef SONOFF
  Serial.begin(115200);
  Serial.println("\n\n");
  #endif
  EEPROM.begin(sizeof(EEpromData));
  EEPROM.get(0,EEpromData);

  pinMode(PIN_RELAY, OUTPUT);
  #ifdef PIN_LED
  pinMode(PIN_LED, OUTPUT);
  #endif
  pinMode(PIN_BUTTON, INPUT);
	digitalWrite(PIN_BUTTON, HIGH);
	debouncerButton.attach(PIN_BUTTON);
	debouncerButton.interval(50);

  if (EEpromData.initialState)
  {
    #ifdef SONOFF
    Serial.println( "Sonoff ON - initial state\n");
    #endif
    digitalWrite(PIN_RELAY,RELAY_ON_STATE);
  } else {
    #ifdef SONOFF
    Serial.println( "Sonoff OFF - initial state\n");
    #endif
    digitalWrite(PIN_RELAY,RELAY_OFF_STATE);
    EEpromData.initialState=0;
  }


  Homie_setFirmware(FIRMWARE_NAME, FIRMWARE_VER);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  #ifdef PIN_LED
  Homie.setLedPin(PIN_LED, LOW);
  #else
  Homie.disableLedFeedback();
  #endif
  #ifdef GENERICRELAY
  Homie.disableLogging();
  #endif
  Homie.setResetTrigger(PIN_BUTTON, LOW, 10000);
  relayNode.advertise("relayState").settable(relayStateHandler);
  relayNode.advertise("relayTimer").settable(relayTimerHandler);
  relayNode.advertise("relayInitMode").settable(relayInitModeHandler);
  watchDogNode.advertise("tick").settable(watchdogTickHandler);
  watchDogNode.advertise("timeOut").settable(watchdogTimeOutHandler);
  keepAliveNode.advertise("tick").settable(keepAliveTickHandler);
  keepAliveNode.advertise("timeOut").settable(keepAliveTimeOutHandler);
  Homie.onEvent(onHomieEvent);
  reverseMode.setDefaultValue(false).setValidator([] (bool candidate) {
    return (candidate >= 0) && (candidate <= 1);
  });
  Homie.setup();
}

/*
 * Main loop
 */
void loop()
{
  Homie.loop();
}
