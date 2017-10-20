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

#ifdef MYRELAY
// esp01 my relay with different pin configuration
#define PIN_RELAY 2
#define PIN_BUTTON 0
#define RELAY_ON_STATE LOW
#define RELAY_OFF_STATE HIGH
#endif

unsigned long downCounterStart;
unsigned long downCounterLimit=0;

unsigned long keepAliveReceived=0;
int lastButtonValue = 1;

// EEPROM structure
struct EEpromDataStruct {
  int keepAliveValue; // 0 - disabled, keepalive time - seconds
};

EEpromDataStruct EEpromData;

Bounce debouncerButton = Bounce();

HomieNode relayNode("relay01", "relay");
HomieNode keepAliveNode("keepalive", "keepalive");

HomieSetting<bool> initialState("initialState", "Relay initial state");


/*
 *
 */
bool relayStateHandler(const HomieRange& range, const String& value) {
  if (value == "ON") {
    digitalWrite(PIN_RELAY, RELAY_ON_STATE);
    #ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
    #endif
    relayNode.setProperty("relayState").send("ON");
  } else if (value == "OFF") {
    digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
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
bool keepAliveValueHandler(HomieRange range, String value)
{
  int oldValue = EEpromData.keepAliveValue;
  if (value.toInt() > 0)
  {
    EEpromData.keepAliveValue = value.toInt();
  }
  if (value=="0")
  {
    EEpromData.keepAliveValue = 0;
  }
  if (oldValue!=EEpromData.keepAliveValue)
  {
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
    digitalWrite(PIN_RELAY, RELAY_ON_STATE);
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
 * Homie setup handler
 */
void setupHandler()
{
  if (initialState.get())
  {
    relayNode.setProperty("relayState").send("ON");
  } else {
    relayNode.setProperty("relayState").send("OFF");
  }
  String outMsg = String(EEpromData.keepAliveValue);
  keepAliveNode.setProperty("keepAliveValue").send(outMsg);
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
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
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
      if (relayValue == RELAY_ON_STATE)
      {
        relayStateHandler(emptyRange, "OFF");
      } else {
        relayStateHandler(emptyRange, "ON");
      }
    }
  }
  debouncerButton.update();

  // Check if keepalive is supported and expired
  if (EEpromData.keepAliveValue != 0 && (millis() - keepAliveReceived) > EEpromData.keepAliveValue*1000 )
  {
    ESP.restart();
  }
}

/*
 * Homie event handler
 */
void onHomieEvent(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::CONFIGURATION_MODE: // Default eeprom data in configuration mode
      digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
      EEpromData.keepAliveValue = 0;
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
    case HomieEventType::MQTT_CONNECTED:
      // Do whatever you want when MQTT is connected in normal mode
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

  if (initialState.get())
  {
    digitalWrite(PIN_RELAY, RELAY_ON_STATE);
    #ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
    #endif
  } else {
    digitalWrite(PIN_RELAY, RELAY_OFF_STATE);
    #ifdef PIN_LED
    digitalWrite(PIN_LED, HIGH);
    #endif
  }

  Homie_setFirmware(FIRMWARE_NAME, FIRMWARE_VER);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  #ifdef PIN_LED
  Homie.setLedPin(PIN_LED, LOW);
  #else
  Homie.disableLedFeedback();
  Homie.disableLogging();
  #endif
  Homie.setResetTrigger(PIN_BUTTON, LOW, 10000);
  relayNode.advertise("relayState").settable(relayStateHandler);
  relayNode.advertise("relayTimer").settable(relayTimerHandler);
  keepAliveNode.advertise("tick").settable(keepAliveTickHandler);
  keepAliveNode.advertise("keepAliveValue").settable(keepAliveValueHandler);
  Homie.onEvent(onHomieEvent);
  initialState.setDefaultValue(false).setValidator([] (bool candidate) {
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
