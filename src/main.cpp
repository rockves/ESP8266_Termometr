#include "main.h"

String url = "https://script.google.com/macros/s/AKfycbz1tq8IRyaVZyfy-0BTZiumuc_s7IyiYQSKqXFcAe1IokDR__Me/exec";
WiFiClientSecure client;
ESP8266WebServer configServer(CONFIG_SERVER_PORT);

OneWire oneWire(TEMP_SENSOR_GPIO);
byte sensorAddrs[8];
DallasTemperature sensor(&oneWire);

float probeTemperatures[2];
float avgTemperature = 0;

void connectToWiFi(const char* _ssid, const char* _password){
    digitalWrite(LED_BUILTIN, LOW);
    
    #if DEBUG
        Serial.print("Try to connect to ");
        Serial.print(_ssid);
    #endif
    
    WiFi.mode(WiFiMode::WIFI_STA);
    WiFi.begin();
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

void setAP(){
    IPAddress ip(192,168,0,1);
    IPAddress gateway(192,168,0,1);
    IPAddress subnet(255,255,255,0);
    
    WiFi.persistent(true);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.mode(WiFiMode::WIFI_AP_STA);
    

    WiFi.softAPConfig(ip,gateway,subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println(WiFi.softAPIP());
}

void handleConnection(){
    configServer.sendContent(page);
}

void handleGetRequest(){
    String ssid = configServer.arg("SSID");
    String password = configServer.arg("PASSWORD");
    Serial.println(ssid);
    Serial.println(password);
    //WiFi.softAPdisconnect();
    //WiFi.mode(WiFiMode::WIFI_STA);
    WiFi.begin(ssid, password);
    configServer.sendContent("<p>Probuje polaczyc z " + String(ssid) + "...<p/>");
    for(int i = 0; i<200; i++){
        if(WiFi.status() == WL_CONNECTED){
            //configServer.sendContent("<p>Polaczono</p>");
            //WiFi.softAPdisconnect();
            //WiFi.mode(WiFiMode::WIFI_STA);
            Serial.println(WiFi.localIP());
            system_restart();
            break;
        }
        delay(100);
    }
    if(WiFi.status() != WL_CONNECTED){
        //configServer.sendContent("<p>Nie udalo się polaczyc</p><br><a href = '/'>Powrot</a>");
        WiFi.disconnect();
    }
    
}

void sendData(float temperature){
    if (!client.connect(HOST, HTTPS_PORT)) {
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
        '?' + SERVER_FUNCTION_VARIABLE + '=' + SERVER_FUNCTION_TO_ADD_DATA +
        '&' + SERVER_TEMPERATURE_VARIABLE + '=' + String(temperature) + 
        '&' + SERVER_ERROR_VARIABLE + '=' + NO_ERROR_CODE +
        " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

void sendSensorError(){
    if (!client.connect(HOST, HTTPS_PORT)) {
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
        '?' + SERVER_FUNCTION_VARIABLE + '=' + SERVER_FUNCTION_TO_ADD_DATA +
        '&' + SERVER_TEMPERATURE_VARIABLE + '=' + 0 + 
        '&' + SERVER_ERROR_VARIABLE + '=' + DISCONNECTED_SENSOR_ERROR_CODE +
        " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

bool checkIsSensorConnected(){
    if ( !oneWire.search(sensorAddrs)) {
        Serial.println("Sensor disconnected!");
        connectToWiFi(SSID, PASSWORD);
        sendSensorError();
        return false;
    }
    return true;
}

bool checkIsMeasurementError(){
    float checkValue = probeTemperatures[0] - probeTemperatures[1];
    if(abs(checkValue) > MAX_MEASUREMENTS_ERROR){
        #if DEBUG
            Serial.println("Blad pomiaru");
        #endif
        return true;
    }else{
        return false;
    }
}

float getTemperature(){
    probeTemperatures[0] = probeTemperatures[1] = 0;
    
    do{
        sensor.requestTemperatures();
        delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
        probeTemperatures[0] = sensor.getTempCByIndex(0);

        delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS);

        sensor.requestTemperatures();
        delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
        probeTemperatures[1] = sensor.getTempCByIndex(0);
    }while (checkIsMeasurementError());

    #if DEBUG
        Serial.print("Temperatura 1: ");
        Serial.println(probeTemperatures[0]);
        Serial.print("Temperatura 2: ");
        Serial.println(probeTemperatures[1]);
    #endif

    return ((probeTemperatures[0]+probeTemperatures[1])/2);
}

void ICACHE_FLASH_ATTR user_pre_init(void){
    system_deep_sleep_set_option(2);
    system_phy_set_rfoption(3);
}

void setup(){
    #if DEBUG
        Serial.begin(115200);
        Serial.setDebugOutput(true);
    #endif
    
    pinMode(BUTTON_GPIO, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    if(digitalRead(BUTTON_GPIO)){ //Tryb pracy
        digitalWrite(LED_BUILTIN, HIGH);
        client.setInsecure();
        sensor.begin();
        if(checkIsSensorConnected()){
            avgTemperature = getTemperature();
            //WiFi.printDiag(Serial);
            WiFi.persistent(false);
            connectToWiFi(SSID,PASSWORD);
            sendData(avgTemperature);
            //WiFi.disconnect(true);
        }

        system_deep_sleep_instant(DEEP_SLEEP_TIME_SECONDS);
    }

    setAP();
    configServer.on("/", HTTP_GET, handleConnection);
    configServer.on("/post", HTTP_POST, handleGetRequest);
    configServer.begin();
}

void loop(){
    configServer.handleClient();
}