#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define DEBUG true
#define SSID "Dom_03_2.4G"
#define PASSWORD "gn9j-0dln-2cp1"
#define HOST "script.google.com"
#define HTTP_PORT 443
#define TEMP_SENSOR_GPIO 4
#define TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS 800
#define TIME_BETWEEN_MEASUREMENT_MILISECONDS 300
#define DEEP_SLEEP_TIME_MILISECONDS 5000

String url = "https://script.google.com/macros/s/AKfycbz1tq8IRyaVZyfy-0BTZiumuc_s7IyiYQSKqXFcAe1IokDR__Me/exec?func=addData";
WiFiClientSecure client;

OneWire oneWire(TEMP_SENSOR_GPIO);
DallasTemperature sensor(&oneWire);

float probeTemperatures[2];
float avgTemperature = 0;

void connectToWiFi(const char* _ssid, const char* _password){
    digitalWrite(LED_BUILTIN, LOW);
    
    #if DEBUG
        Serial.print("Try to connect to ");
        Serial.print(_ssid);
    #endif
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid,_password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        #if DEBUG 
            Serial.print("."); 
        #endif
    }
    
    #if DEBUG
        Serial.print("Connected\n");
        Serial.println(WiFi.localIP());
    #endif
    
    digitalWrite(LED_BUILTIN, HIGH);
}

float getTemperature(){
    probeTemperatures[0] = probeTemperatures[1] = probeTemperatures[2] = 0;
    
    sensor.requestTemperatures();
    delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
    probeTemperatures[0] = sensor.getTempCByIndex(0);

    delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS);

    sensor.requestTemperatures();
    delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
    probeTemperatures[1] = sensor.getTempCByIndex(0);

    #if DEBUG
        Serial.print("Temperatura 1: ");
        Serial.println(probeTemperatures[0]);
        Serial.print("Temperatura 2: ");
        Serial.println(probeTemperatures[1]);
    #endif

    return ((probeTemperatures[0]+probeTemperatures[1])/2);
} //~

void sendData(float temperature){
    if (!client.connect(HOST, HTTP_PORT)) {
        #if DEBUG
            Serial.println("connection failed\n");
            Serial.print("My ip: ");
            Serial.println(WiFi.localIP());
        #endif
        return;
    }
    
    #if DEBUG
        Serial.print("Wysylam dane\n");
    #endif
    
    client.print(
        String("GET ") + url + 
        "&temp=" + String(temperature) + 
        " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

void setup(){
    #if DEBUG
        Serial.begin(115200);
    #endif

    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.persistent(false);
    client.setInsecure();
    sensor.begin();

    avgTemperature = getTemperature();
    WiFi.forceSleepWake();
    connectToWiFi(SSID,PASSWORD);
    sendData(avgTemperature);
    ESP.deepSleep(8e6, WAKE_RF_DISABLED);
}

void loop(){
}