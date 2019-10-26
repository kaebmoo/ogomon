/*
  Simple example for receiving
  
  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ArduinoJson.h> // https://arduinojson.org/v5/assistant/

RCSwitch mySwitch = RCSwitch();
unsigned long channelID = 803;
const char* server = "mqtt.ogonan.com"; 
char mqttUserName[] = "kaebmoo";  // Can be any name.
char mqttPass[] = "sealwiththekiss";  // Change this your MQTT API Key from Account > MyProfile.
String subscribeTopic = "channels/" + String( channelID ) + "/subscribe/fields/field3";
String publishTopic = "/rooms/count/" + String( channelID ) + "";


WiFiClient client; 
PubSubClient mqttClient(client); 

bool shouldSaveConfig = false;
int count = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  setupWifi();
}

void loop() {
  /*
  if (mySwitch.available()) {
    
    Serial.print("Received ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );

    mySwitch.resetAvailable();
  }
  */
  if (!mqttClient.connected()) 
  {
    reconnect();
  }
  mqttClient.loop();   // Call the loop continuously to establish connection to the server.

  if (mySwitch.available()) {
    Serial.print("Received ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("delay ");
    Serial.print(mySwitch.getReceivedDelay());
    Serial.print(" / ");
    // Serial.print("raw data ");
    // Serial.print(mySwitch.getReceivedRawdata());
    // Serial.print(" / ");
    Serial.print("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );
    counting();    
    mySwitch.resetAvailable();
  }

  // if (count >= 10) {
  //   Serial.println("Triger ");
  //   count = 0;    
  // }
}


void counting()
{
  Serial.print("Triger : ");
  count++;
  Serial.println(count);

  // send to mqtt server, node-red, google sheet 
  String payload = "{";
  payload += "\"count\":"; payload += count; payload += ",";
  payload += "\"id\":"; payload += channelID;
  payload += "}";
  const char *msgBuffer;
  msgBuffer = payload.c_str();
  
  const char *topicBuffer;
  topicBuffer = publishTopic.c_str();

  Serial.print(topicBuffer); 
  Serial.print(" ");
  Serial.println(msgBuffer);
  
  mqttClient.publish(topicBuffer, msgBuffer);

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
  
  
  count = 0;
}

void setupWifi()
{
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setConfigPortalTimeout(60);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  String alias = "ogosense-"+String(ESP.getChipId());
  if (!wifiManager.autoConnect(alias.c_str(), "")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  
  Serial.println("connected...yeey :)");
  // ThingSpeak.begin( client );
  mqttConnect();
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void mqttConnect()
{
  mqttClient.setServer(server, 1883);   // Set the MQTT broker details.
  mqttClient.setCallback(callback);  
}

void reconnect() 
{
  char clientID[9];

  // Loop until reconnected.
  
    Serial.print("Attempting MQTT connection...");
    /*
    // Generate ClientID
    for (int i = 0; i < 8; i++) {
        clientID[i] = alphanum[random(51)];
    }
    clientID[8]='\0';
    */
    
    // Connect to the MQTT broker
    if (mqttClient.connect(clientID,mqttUserName,mqttPass)) {
      Serial.print("Connected with Client ID:  ");
      Serial.print(String(clientID));
      Serial.print(", Username: ");
      Serial.print(mqttUserName);
      Serial.print(" , Passwword: ");
      Serial.println(mqttPass);
      mqttClient.subscribe( subscribeTopic.c_str() );
    } 
    else {
      Serial.print("failed, rc=");
      // Print to know why the connection failed.
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  int i;
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  /*
  if (!strncmp(p, "on", 2) || !strncmp(p, "1", 1)) {
    digitalWrite(RELAY1, HIGH);
    idTimerSwitch = timerSwitch.after(10000, turnOn);
  }
  else if (!strncmp(p, "off", 3) || !strncmp(p, "0", 1) ) {
    digitalWrite(RELAY1, LOW);
  }
  */
}
