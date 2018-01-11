//// LIBRARIES
#include <DHT.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include <Wire.h>

////SENSORS
// DHT22 - temperature and humidity
#define DHTPIN 2            //Data Pin
#define DHTTYPE DHT22       // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// photoresistance
int PRPIN = A0;


//// ACTUATORS
// led
int LEDPIN = 8;
int state = 0;


//// SERVER
// MQTT connection
#define MQTT_SERVER "m23.cloudmqtt.com"       //MQTT Server
#define MQTT_TOPIC "arduino"                   //TOPIC to Publish
const char* mqttUser = "fhuaqwke";
const char* mqttPassword = "CqSslASh3VrS";

//Ethernet Shield Setup - Start.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 4);       // IP address of the arduino
IPAddress server (34, 249, 42, 33); // IP address of the mqtt server


//Setting up mqtt client
EthernetClient ethClient;
PubSubClient client(ethClient);


void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   
  // initialize DHT
  Serial.println("DHT22 Initailzied");
  dht.begin();

  // initialize led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  // initialize RGB LED
  /*pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);*/

  //initialize MQTTserver
  Serial.println("Setting up server");
  Ethernet.begin(mac, ip);
  delay(2000);

  Serial.println("Setting up server");
  client.setServer(server,17980);
  client.setCallback(callback);
  MQTT_connect();

  //subscribe
  boolean action = client.subscribe("actuator",1);
}


void loop() {

// DHT22 - temperature and humidity
  // humidity rate
  float hum = dht.readHumidity();
  // temperature (°C)
  float temp = dht.readTemperature();
  // temperature (°F)
  float f = dht.readTemperature(true);
  
  // Stop the program if no value returned by the sensor
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT22 sensor");
    return;
  }

  // perceived temperature
  float hi = dht.computeHeatIndex(f, hum);
  hi = dht.convertFtoC(hi);  

// reading from light sensor
  float lig = analogRead(PRPIN);
  if (isnan(lig)) {
     Serial.println("Failed to read from light sensor!");
     return;
  }

// mqtt connection
  //Build json object
 
  String jsonPayloadLig = "light : ";
    jsonPayloadLig += lig;
  String jsonPayloadDHT ="hum : ";
    jsonPayloadDHT += hum;
    jsonPayloadDHT +=", temp : ";
    jsonPayloadDHT += temp;
    jsonPayloadDHT +=", ptem : ";
    jsonPayloadDHT += hi;

  String jsonPayloadLED = "state of the light : ";
    jsonPayloadLED += state;

  
  const char *payloadLig = jsonPayloadLig.c_str(); // convert string to const char*
  Serial.println(jsonPayloadLig);
  if(!client.connected()){
    Serial.println("Not Connected");
  }

  const char *payloadDHT = jsonPayloadDHT.c_str(); // convert string to const char*
  Serial.println(jsonPayloadDHT);
  if(!client.connected()){
    Serial.println("Not Connected");
  }

  const char *payloadLED = jsonPayloadLED.c_str(); // convert string to const char*
  Serial.println(jsonPayloadLED);
  if(!client.connected()){
    Serial.println("Not Connected");
  }

  String jsonPayloadValL = " ";
    jsonPayloadValL += lig;
  const char *payloadValL = jsonPayloadValL.c_str();
  
  //publish light
  boolean resultL = client.publish("light", payloadValL);
  Serial.println(resultL);

  //publish DHT
  boolean resultDHT = client.publish("DHT", payloadDHT);
  Serial.println(resultDHT);

  //publish state of the LED
  boolean resultLED = client.publish("led", payloadLED);
  Serial.println(resultLED);

  client.loop();
  delay(2000);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
     Serial.println("Not connected");
     boolean connectionResult = client.connect("pjd-mqtt",mqttUser,mqttPassword,MQTT_TOPIC,1,0,"Test Connection");
     Serial.println(connectionResult);
     Serial.println("MQTT Connected! ");      if(!client.connected()){

  }
  
}


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived from ");
    Serial.println(topic);
    String message = "";
    for (int i=0;i<length;i++) {
      char receivedChar = (char)payload[i];
      message += receivedChar;
      Serial.print(receivedChar);
    }
      if (message == "switch" && state == 0){
        digitalWrite(LEDPIN, HIGH);
        state = 1;
        //Serial.println("ON");
      }
      else if (message == "switch" && state == 1){
        digitalWrite(LEDPIN, LOW);
        state = 0;
        //Serial.println("OFF");
      }
  Serial.println();
}
