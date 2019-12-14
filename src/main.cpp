#include "config.h"

WiFiClientSecure client;
ESP8266WebServer configServer(CONFIG_SERVER_PORT);

OneWire oneWire(TEMP_SENSOR_GPIO);
byte sensorAddrs[8];
DallasTemperature sensor(&oneWire);

float probeTemperatures[2];
float avgTemperature = 0;

void connectToWiFi()
{
#if DEBUG
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Try to connect to WiFi");
#endif

    WiFi.mode(WiFiMode::WIFI_STA);
    WiFi.begin();
    for (int i = 0; i < HOW_MUCH_CHECKS_FOR_CONNECTION; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            break;
        }
#if DEBUG
        Serial.print(".");
#endif
        delay(DELAY_TIME_TO_CHECK_FOR_CONNECTION);
    }

#if DEBUG
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
#endif
}

void setAP()
{
    IPAddress ip(AP_IP_OCTET_1, AP_IP_OCTET_2, AP_IP_OCTET_3, AP_IP_OCTET_4);
    IPAddress gateway(AP_GATEWAY_OCTET_1, AP_GATEWAY_OCTET_2, AP_GATEWAY_OCTET_3, AP_GATEWAY_OCTET_4);
    IPAddress subnet(AP_SUBNET_OCTET_1, AP_SUBNET_OCTET_2, AP_SUBNET_OCTET_3, AP_SUBNET_OCTET_4);

    WiFi.persistent(true);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.mode(WiFiMode::WIFI_AP_STA);

    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
#if DEBUG
    Serial.println(WiFi.softAPIP());
#endif
}

void handleConnection()
{
    configServer.sendContent(page);
}

void handleGetRequest()
{
    String ssid = configServer.arg("SSID");
    String password = configServer.arg("PASSWORD");
#if DEBUG
    Serial.println(ssid);
    Serial.println(password);
#endif
    WiFi.begin(ssid, password);
    configServer.sendContent("<p>Probuje polaczyc z " + String(ssid) + "...<p/>");
    for (int i = 0; i < HOW_MUCH_CHECKS_FOR_CONNECTION; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
#if DEBUG
            Serial.println(WiFi.localIP());
#endif
            system_restart();
            break;
        }
        delay(DELAY_TIME_TO_CHECK_FOR_CONNECTION);
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect();
    }
}

void sendData(float temperature)
{
    if (!client.connect(HOST, HTTPS_PORT))
    {
#if DEBUG
        Serial.println("connection failed");
        Serial.print("My ip: ");
        Serial.println(WiFi.localIP());
#endif
        return;
    }

#if DEBUG
    Serial.print("Wysylam dane\n");
#endif

    client.print(
        String("GET ") + SCRIPT_URL +
        '?' + SERVER_FUNCTION_VARIABLE + '=' + SERVER_FUNCTION_TO_ADD_DATA +
        '&' + SERVER_TEMPERATURE_VARIABLE + '=' + String(temperature) +
        '&' + SERVER_ERROR_VARIABLE + '=' + NO_ERROR_CODE +
        " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

void sendSensorError()
{
    if (!client.connect(HOST, HTTPS_PORT))
    {
#if DEBUG
        Serial.println("connection failed");
        Serial.print("My ip: ");
        Serial.println(WiFi.localIP());
#endif
        return;
    }

#if DEBUG
    Serial.print("Wysylam dane\n");
#endif

    client.print(
        String("GET ") + SCRIPT_URL +
        '?' + SERVER_FUNCTION_VARIABLE + '=' + SERVER_FUNCTION_TO_ADD_DATA +
        '&' + SERVER_TEMPERATURE_VARIABLE + '=' + 0 +
        '&' + SERVER_ERROR_VARIABLE + '=' + DISCONNECTED_SENSOR_ERROR_CODE +
        " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

bool checkIsSensorConnected()
{
    if (!oneWire.search(sensorAddrs))
    {
#if DEBUG
        Serial.println("Sensor disconnected!");
#endif
        connectToWiFi();
        sendSensorError();
        return false;
    }
    return true;
}

bool checkIsMeasurementError()
{
    float checkValue = probeTemperatures[0] - probeTemperatures[1];
    if (abs(checkValue) > MAX_MEASUREMENTS_ERROR)
    {
#if DEBUG
        Serial.println("Blad pomiaru");
#endif
        return true;
    }
    else
    {
        return false;
    }
}

float getTemperature()
{
    probeTemperatures[0] = probeTemperatures[1] = 0;

    do
    {
        sensor.requestTemperatures();
        delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
        probeTemperatures[0] = sensor.getTempCByIndex(0);

        delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS);

        sensor.requestTemperatures();
        delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
        probeTemperatures[1] = sensor.getTempCByIndex(0);
    } while (checkIsMeasurementError());

#if DEBUG
    Serial.print("Temperatura 1: ");
    Serial.println(probeTemperatures[0]);
    Serial.print("Temperatura 2: ");
    Serial.println(probeTemperatures[1]);
#endif

    return ((probeTemperatures[0] + probeTemperatures[1]) / 2);
}

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    system_deep_sleep_set_option(2);
    system_phy_set_rfoption(3);
}

void setup()
{
#if DEBUG
    Serial.begin(115200);
    Serial.setDebugOutput(true);
#endif

    pinMode(BUTTON_GPIO, INPUT_PULLUP);
#if DEBUG
    pinMode(LED_BUILTIN, OUTPUT);
#endif
}

void loop()
{

    if (digitalRead(BUTTON_GPIO))
    { //Tryb pracy
#if DEBUG
        digitalWrite(LED_BUILTIN, HIGH);
#endif
        client.setInsecure();
        sensor.begin();
        if (checkIsSensorConnected())
        {
            avgTemperature = getTemperature();
            WiFi.persistent(false);
            connectToWiFi();
            if (WiFi.status() == WL_CONNECTED)
                sendData(avgTemperature);
        }

        system_deep_sleep_instant(DEEP_SLEEP_TIME_SECONDS);
    }

    setAP();
    configServer.on(CONFIG_SERVER_HTML_DISPLAY_LOCATION, HTTP_GET, handleConnection);
    configServer.on(CONFIG_SERVER_DATA_POST_LOCATION, HTTP_POST, handleGetRequest);
    configServer.begin();
    for (;;)
    {
        configServer.handleClient();
    }
}