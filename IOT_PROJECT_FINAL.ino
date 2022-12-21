#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ThingSpeak.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "HTTPClient.h"
#include "time.h"
#include <ArduinoJson.h>
#include "DHT.h"
#define led1Pin 35
#define led2Pin 34
#define buzzer1Pin 5
#define buzzer2Pin 18
#define DHTpin 21
#define DHTTYPE DHT11
DHT dht(DHTpin, DHTTYPE);
int delay_in_conn = 1;
float duration_1;
float distance_1;
int door_pin[] = {26, 14};
int person_pin = 33;
int door_range[] = {100, 2000};
int data1, data2;
int count = 0;
const int ldrPin = 32;
WebServer server(80);
char ssid[] = "****";
char password[] = "*****";
const char *servers = "mqtt3.thingspeak.com";
const char *mqttUserName = "GAotKjQCATkBJwUyJgQuNTM";
const char *mqttPass = "uLKEjz1NK0yARs7Sx67XBOqZ";
int writeChannelID = 1769420;
char writeAPIKey[] = "B1LYVZQUG2VR3YAQ";
char clientID[] = "GAotKjQCATkBJwUyJgQuNTM";
int mqttPort = 1883;
String cse_ip = "192.168.8.51"; // YOUR IP from ipconfig/ifconfig
String cse_port = "8080";
String Server = "http://192.168.8.51:8080/~/in-cse/in-name/"; //+ cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String ae = "SmartHome";
String cnts[] = {"PersonSensor","DoorSensor","PeopleCount","Humidity","Temperature","Light"};

WiFiClient client;
PubSubClient mqttClient(client);

void createCI(String val,String cnt){
HTTPClient http;
http.begin(Server + ae + "/" + cnt + "/");
http.addHeader("X-M2M-Origin", "admin:admin");
http.addHeader("Content-Type", "application/json;ty=4");
int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": \"" + String(val) + "\"}}");
//Serial.println(code);
if (code == -1) {
Serial.println("UNABLE TO CONNECT TO THE SERVER");
}
http.end();
}
void updateOm2m(int val[]){
  for(int i=0;i<6;i++){
    String temp=(String)val[i];
String cnt=cnts[i];
    createCI(temp,cnt);
  }
   String temp=(String)val[0];
String cnt=cnts[0];
    createCI(temp,cnt);
}
void setup()
{
    Serial.begin(9600);
    pinMode(door_pin[1], OUTPUT);
    pinMode(door_pin[0], INPUT);
    pinMode(person_pin, INPUT);
    pinMode(ldrPin, INPUT);
    Serial.println("humidity sensors");
    dht.begin();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("Connecting...");
    }
    Serial.println("");
    Serial.println("Successfully connected to WiFi.");
    Serial.println("IP address is : ");
    Serial.println(WiFi.localIP());
    mqttClient.setServer(servers, mqttPort);
    ThingSpeak.begin(client);
}

float detect_sensor(int number)
{
    if (number == 1)
    {
        digitalWrite(door_pin[1], HIGH);
        delayMicroseconds(10);
        digitalWrite(door_pin[1], LOW);
        duration_1 = pulseIn(door_pin[0], HIGH);
        distance_1 = duration_1 * 0.034 / 2;
        return distance_1;
    }
    else if (number == 2)
    {
        int val = digitalRead(person_pin);
        return val;
    }
    else if (number == 3)
    {
        int ldrStatus = analogRead(ldrPin);
        //Serial.println(ldrStatus);
        if (ldrStatus <= 200)
        {
            Serial.println("dark dark");
        }
        else
        {
            Serial.println("bright bright");
        }
        return ldrStatus;
    }
    else if (number == 4)
    {
        float t = dht.readTemperature();
       //Serial.println("temperature:");
        //Serial.print(t);
        return t;
    }
    else if (number == 5)
    {
        float h = dht.readHumidity();
       // Serial.println("Humidity:");
        ///Serial.print(h);
        return h;
    }
    else
        return 0;
}
float humidity = 70, temperature = 27, light;
void publish(int people, int door_sensor, int person_sensor)
{
      int val[6];
      val[0]=person_sensor,val[1]=door_sensor,val[2]=people;
    int field_to_publish[8] = {0, 1, 1, 1, 1, 1, 1, 0};
    light = detect_sensor(3);
    delay(10);
    float hh = (detect_sensor(5));
    if (isnan(hh) || hh > 100)
        humidity = humidity;
    else
        humidity = hh;
    delay(10);
    float tt = (detect_sensor(4));
    if (isnan(tt) || tt > 100)
        temperature = temperature;
    else
        temperature = tt;
    val[3]=humidity,val[4]=temperature,val[5]=light;
    ThingSpeak.setField(2, person_sensor);
    ThingSpeak.setField(3, people);
    ThingSpeak.setField(4, humidity);
    ThingSpeak.setField(5, temperature);
    ThingSpeak.setField(6, light);
    ThingSpeak.setField(7, door_sensor);
    ThingSpeak.writeFields(writeChannelID, writeAPIKey);/
    updateOm2m(val);
}
int x = 0;
void loop()
{

    if (x == 600)                     
    {
        publish(count, data1, data2);
        x = 0;
    }
    data2 = detect_sensor(2);
   // Serial.print("data2: ");
    //Serial.println(data2);
    data1 = detect_sensor(1);
    //Serial.print("data1: ");
    //Serial.println(data1);
    if (data2 == 1 && data1 < door_range[0])
    {
        count--;
        Serial.print("count: ");
        Serial.println(count);
        publish(count, data1, data2);
        x = 0;
        delay(5000);
    }
    else if (data2 == 0 && data1 < door_range[0])
    {
        count++;
        Serial.print("count:");
        Serial.println(count);
        publish(count, data1, data2);
        x = 0;
        delay(5000);
    }

    delay(100);
    x++;
}
