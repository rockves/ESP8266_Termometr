#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8x8lib.h>
#define DEBUG true
#define SSID "SSID"
#define PASSWORD "PASSWORD"
#define HOST "script.google.com"
#define HTTP_PORT 443
#define TEMP_SENSOR_GPIO 0                                          //GPIO dla sensora DS18B20
#define TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS 800     //Czas między sprawdzeniem temperatury a jej odczytaniem z sensora
#define TIME_BETWEEN_MEASUREMENT_MILISECONDS 200                    //Czas pomiędzy dwoma pomiarami
#define TIME_BETWEEN_SENDING_DATA_MILISECONDS 10000                 //Czas po którym nastąpi wysłanie temperatury
#define TIME_BETWEEN_DISPLAYING_DATA 2000                           //Czas pomiędzy wyświetleniem temperatury

String url = "MACRO URL"; //Link do makro w google script
WiFiClientSecure client;

OneWire oneWire(TEMP_SENSOR_GPIO); //Interfejs OneWire
DallasTemperature sensor(&oneWire); //Obiekt sensora DS18B20

U8X8_SSD1306_128X64_NONAME_SW_I2C oled(/* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE); //Obiekt wyświetlacza OLED

float probeTemperatures[2]; //Tablica pomiarów częściowych
float avgTemperature = 0;   //Średnia temperatura liczona z pomiarów częściowych

unsigned long timer_displaying = 0; //Kiedy odbyło się ostatnie wyświetlenie
unsigned long timer_sending = 0;    //Kiedy odbyło się ostatnie wysłanie
unsigned long time_read;            //Odczyt ile czasu program jest już uruchomiony

void connectToWiFi(const char* _ssid, const char* _password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid,_password);
}

float getTemperature(){
    probeTemperatures[0] = probeTemperatures[1] = 0;
    
    sensor.requestTemperatures();                                   //Prośba o sprawdzenie temperatury
    delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);     //Czekanie na odczyt   
    probeTemperatures[0] = sensor.getTempCByIndex(0);               //Pobranie temperatury z czujnika

    delay(TIME_BETWEEN_MEASUREMENT_MILISECONDS); //Czekanie między pomiarami

    sensor.requestTemperatures();
    delay(TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS);
    probeTemperatures[1] = sensor.getTempCByIndex(0);

    if(DEBUG){
        char temp[25];
        float tempF;

        tempF = probeTemperatures[0];
        dtostrf(tempF, 2, 2, temp);
        oled.drawString(0,4,strcat(temp, " - Temp 1"));

        tempF = probeTemperatures[1];
        dtostrf(tempF, 2, 2, temp);
        oled.drawString(0,6,strcat(temp, " - Temp 2"));
    }
    return ((probeTemperatures[0]+probeTemperatures[1])/2); //Zwrot uśrednionych odczytów
} //~1800ms

void sendData(float temperature){
    if (!client.connect(HOST, HTTP_PORT)) { //Test czy klient połączył się do hosta przez HTTPS
        if(DEBUG){
            Serial.println("connection failed\n");
            Serial.print("My ip: ");
            Serial.println(WiFi.localIP());
        }
        return;
    }
    if(DEBUG) Serial.print("Wysylam dane\n");
    //Wysłanie GET na URL makra z temperaturą
    client.print(String("GET ") + url + "temp=" + String(temperature) + " HTTP/1.1\r\n" +
        "Host: " + HOST + "\r\n" +
        "User-Agent: ESP8266\r\n" +
        "Connection: close\r\n\r\n");
}

void displayTemperature(){
    char temp[10]; //Tablica znaków dla wartości temperatury
    dtostrf(avgTemperature, 2, 2, temp); //Konwersja temperatury średnioj na znaki
    (avgTemperature>0) ? strcat(temp, "°C ") : strcat(temp, "°C"); //Dołączenie znaku stopni celcjusza
    oled.drawString(0,2,temp); //Wypisanie na ekran
}

void setup(){
    //Serial.begin(115200);
    oled.begin(); //Inicjalizacja ekranu
    oled.setFont(u8x8_font_pcsenior_f); //Ustawienie czcionki
    client.setInsecure(); //Ustawienie niezabezpieczonego połączenie klienta aby wysłać dane bez sprawdzania certyfikatu
    sensor.begin(); //Inicjalizacja sensora
    WiFi.persistent(true); //Ustawienie zapamiętania ustawień sieci WiFi

    oled.drawString(0,0,"OFFLINE"); //Wypisanie na wyświetlaczu
    displayTemperature(); //Wypisanie temperatury
    connectToWiFi(SSID,PASSWORD); //Start łączenia z siecią
}

void loop(){ 
    time_read = millis(); //Odczyt czasu od początku programu

    (WiFi.status() == WL_CONNECTED) ? oled.drawString(0,0,"ONLINE ") : oled.drawString(0,0,"OFFLINE"); //Sprawdzenie stanu połączenia WiFi
    
    if (time_read - timer_displaying >= TIME_BETWEEN_DISPLAYING_DATA || time_read - timer_sending >= TIME_BETWEEN_SENDING_DATA_MILISECONDS){ //Sprawdzenie czy wykonać pomiar temperatury
        avgTemperature = getTemperature(); //Pomiar temperatury

        if (time_read - timer_displaying >= TIME_BETWEEN_DISPLAYING_DATA){
            displayTemperature(); //Wypisanie temperatury
            timer_displaying = time_read; //Odświeżenie czasu ostatniego wypisania
        }
        
        if (time_read - timer_sending >= TIME_BETWEEN_SENDING_DATA_MILISECONDS){
            sendData(avgTemperature); //Wysłanie temperatury
            timer_sending = time_read; //Odświeżenie czasu ostatniego wysłania
        } 
    }
}