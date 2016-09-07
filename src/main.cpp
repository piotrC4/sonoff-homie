/*
 * Homie for SonOff relay - with timer and initial state configuration
 *
 */

#include <Homie.h>
#include <EEPROM.h>

const int PIN_RELAY = 12;
const int PIN_LED = 13;
const int PIN_BUTTON = 0;

unsigned long downCounterStart;
unsigned long downCounterLimit=0;

// EEPROM structure
struct EEpromDataStruct {
  int initialState; // 1 - ON, other - OFF
};
EEpromDataStruct EEpromData;

HomieNode relayNode("relay01", "relay");

// Event typu relay - manualne wymuszenie stanu
bool relayStateHandler(String value) {
  if (value == "ON") {
    digitalWrite(PIN_RELAY, HIGH);
    digitalWrite(PIN_LED, LOW);
    Homie.setNodeProperty(relayNode, "relayState", "ON");
    //Serial.println("Switch is on");
  } else if (value == "OFF") {
    digitalWrite(PIN_RELAY, LOW);
    digitalWrite(PIN_LED, HIGH);
    Homie.setNodeProperty(relayNode, "relayState", "OFF");
    //Serial.println("Switch is off");
  } else {
    return false;
  }
  return true;
}

// Czasowe wymuszenie stanu
bool relayTimerHandler(String value)
{
  if (value.toInt() > 0)
  {
    digitalWrite(PIN_RELAY, HIGH);
    digitalWrite(PIN_LED, LOW);
    downCounterStart = millis();
    downCounterLimit = value.toInt()*1000;
    Homie.setNodeProperty(relayNode, "relayState", "ON");
    Homie.setNodeProperty(relayNode, "relayTimer", value);
    return true;
  } else {
    return false;
  }
}

// Initial mode handler
bool relayInitModeHandler(String value)
{
  int oldValue = EEpromData.initialState;
  if (value.toInt() == 1 or value=="ON")
  {
    Homie.setNodeProperty(relayNode, "relayInitMode", "1");
    EEpromData.initialState=1;
  } else {
    Homie.setNodeProperty(relayNode, "relayInitMode", "0");
    EEpromData.initialState=0;
  }
  if (oldValue!=EEpromData.initialState)
  {
    EEPROM.put(0, EEpromData);
    EEPROM.commit();
  }
  return true;
}

// Homie setup handler
void setupHandler()
{

  if (EEpromData.initialState==1)
  {
    Homie.setNodeProperty(relayNode, "relayState", "ON");
    Homie.setNodeProperty(relayNode, "relayInitMode", "1");
  } else {
    Homie.setNodeProperty(relayNode, "relayState", "OFF");
    Homie.setNodeProperty(relayNode, "relayInitMode", "0");
  }

}

// Homie loop handler
void loopHandler()
{
  if (downCounterLimit>0)
  {
    if ((millis() - downCounterStart ) > downCounterLimit)
    {
      // Turn off relay
      digitalWrite(PIN_RELAY, LOW);
      digitalWrite(PIN_LED, HIGH);
      Homie.setNodeProperty(relayNode, "relayState", "OFF");
      Homie.setNodeProperty(relayNode, "relayTimer", "0");
      downCounterLimit=0;
    }
  }
}

// Main setup
void setup()
{
  EEPROM.begin(sizeof(EEpromData));
  EEPROM.get(0,EEpromData);

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  if (EEpromData.initialState==1)
  {
    digitalWrite(PIN_RELAY, HIGH);
    digitalWrite(PIN_LED, LOW);
  } else {
    digitalWrite(PIN_RELAY, LOW);
    digitalWrite(PIN_LED, HIGH);
    EEpromData.initialState==0;
  }

  Homie.setFirmware("sonoff", "0.10");
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setLedPin(PIN_LED, LOW);
  Homie.setResetTrigger(PIN_BUTTON, LOW, 5000);
  relayNode.subscribe("relayState", relayStateHandler);
  relayNode.subscribe("relayInitMode", relayInitModeHandler);
  relayNode.subscribe("relayTimer", relayTimerHandler);
  Homie.registerNode(relayNode);
  Homie.setup();
}

// Main loop
void loop()
{
  Homie.loop();
}
