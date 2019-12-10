#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define DEBUG true
#define SSID "SSID"
#define PASSWORD "PASSWORD"
#define HOST "script.google.com"
#define HTTP_PORT 443
#define TEMP_SENSOR_GPIO 4
#define TIME_BETWEEN_MEASUREMENT_MILISECONDS 500
#define DEEP_SLEEP_TIME_MILISECONDS 5000

String url = "MACRO_URL";
WiFiClientSecure client;
OneWire oneWire(TEMP_SENSOR_GPIO);
DallasTemperature sensor(&oneWire);
float probeTemperatures[3];

void connectToWiFi(const char* _ssid, const char* _password){
    digitalWrite(LED_BUILTIN, LOW);
    if(DEBUG){ 
        Serial.print("Try to connect to ");
        Serial.print(_ssid);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid,_password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        if(DEBUG) Serial.print(".");
    }
    if(DEBUG){
        Serial.print("Connected\n");
        Serial.println(WiFi.localIP());
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

float getTemperature(){
    probeTemperatures[0] = probeTemperatures[1] = probeTemperatures[2] = 0;
    
    sensor.requestTemperatures();
    probeTemperatures[0] = sensor.getTempCByIndex(0);
    delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS);
    sensor.requestTemperatures();
    probeTemperatures[1] = sensor.getTempCByIndex(0);
    delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS);
    sensor.requestTemperatures();
    probeTemperatures[2] = sensor.getTempCByIndex(0);
    if(DEBUG){
        Serial.print("Temperatura 1: ");
        Serial.println(probeTemperatures[0]);
        Serial.print("Temperatura 1: ");
        Serial.println(probeTemperatures[1]);
        Serial.print("Temperatura 1: ");
        Serial.println(probeTemperatures[2]);
    }
    return ((probeTemperatures[0]+probeTemperatures[1]+probeTemperatures[2])/3);
} //~1s

void sendData(float temperature){
    if (!client.connect(HOST, HTTP_PORT)) {
        if(DEBUG){
            Serial.println("connection failed\n");
            Serial.print("My ip: ");
            Serial.println(WiFi.localIP());
        }
        return;
    }
    if(DEBUG) Serial.print("Wysylam dane\n");
    client.print(String("GET ") + url + "temp=" + String(temperature) + " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

void setup(){
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.persistent(false);
    connectToWiFi(SSID,PASSWORD);
    client.setInsecure();
    sensor.begin();
}

void loop(){ 
    sendData(getTemperature());
    delay(DEEP_SLEEP_TIME_MILISECONDS);
    //ESP.deepSleep(DEEP_SLEEP_TIME_MILISECONDS);
}