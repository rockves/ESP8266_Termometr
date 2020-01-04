//Dołączone biblioteki
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include "configPage.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG false

//Ustawienia łączenia z wifi
#define HOW_MUCH_CHECKS_FOR_CONNECTION 100     //Iloraz tych dwóch to czas oczekiwania na połączenie z siecią
#define DELAY_TIME_TO_CHECK_FOR_CONNECTION 100 //[ms]

//Ustawienia połączenia ze skryptem google
#define HOST "script.google.com"    //Adres skryptów google
#define SCRIPT_URL "https://script.google.com/macros/s/AKfycbz1tq8IRyaVZyfy-0BTZiumuc_s7IyiYQSKqXFcAe1IokDR__Me/exec" //Adres makra google
#define HTTPS_PORT 443              //Port HTPPS

//Ustawienia punktu dostępu
#define AP_SSID "Esp-term"      //Nazwa punktu dostępu do konfiguracji ustawień wifi
#define AP_PASSWORD "esp123456" //Hasło do punktu dostępu (musi rozpoczynać się od litery i mieć długość 8)
#define AP_IP_OCTET_1 192       //Pierwszy oktet adresu IP
#define AP_IP_OCTET_2 168       //Drugi oktet adresu IP
#define AP_IP_OCTET_3 0         //Trzeci oktet adresu IP
#define AP_IP_OCTET_4 1         //Czwarty oktet adresu IP
#define AP_GATEWAY_OCTET_1 0    //Pierwszy oktet adresu bramy
#define AP_GATEWAY_OCTET_2 0    //Drugi oktet adresu bramy
#define AP_GATEWAY_OCTET_3 0    //Trzeci oktet adresu bramy
#define AP_GATEWAY_OCTET_4 0    //Czwarty oktet adresu bramy
#define AP_SUBNET_OCTET_1 255   //Pierwszy oktet adresu podsieci
#define AP_SUBNET_OCTET_2 255   //Drugi oktet adresu podsieci
#define AP_SUBNET_OCTET_3 255   //Trzeci oktet adresu podsieci
#define AP_SUBNET_OCTET_4 0     //Czwarty oktet adresu podsieci

//Serwer HTTP do konfiguracji sieci
#define CONFIG_SERVER_PORT 80                    //Port serwera configuracji ustawień wifi
#define CONFIG_SERVER_HTML_DISPLAY_LOCATION "/"  //Lokalizacja wyświetlenia strony do wprowadzania ustawień wifi
#define CONFIG_SERVER_DATA_POST_LOCATION "/post" //Lokalizacja przesłania ustawień wifi z formularza HTML

//Interakcja ze skryptem google
#define SERVER_FUNCTION_TO_ADD_DATA "addData" //Funkcja skryptu google służąca do dodania danych
#define SERVER_FUNCTION_VARIABLE "func"       //Nazwa zmiennej skryptu google z nazwą funkcji do wykonania
#define SERVER_TEMPERATURE_VARIABLE "temp"    //Nazwa zmiennej skryptu google z wartością temperatury
#define SERVER_ERROR_VARIABLE "errorCode"     //Nazwa zmiennej skryptu google z wartością kodu błędu

//Kody błędu
#define NO_ERROR_CODE '0'                  //Kod błędu oznaczający brak błędu
#define DISCONNECTED_SENSOR_ERROR_CODE '1' //Kod błędu oznaczający odłączony czujnik

//Czujnik temperatury
#define TEMP_SENSOR_GPIO 5   
#define TEMP_SENSOR_POWER_CONTROLL_GPIO 14                      //Numer pinu do którego podłączony jest czujnik temperatury
#define TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS 800 //Czas pomiędzy sprawdzeniem a odczytem temperatury [ms]
#define TIME_BETWEEN_MEASUREMENT_MILISECONDS 300                //Czas pomiędzy pomiarami [ms]
#define MAX_MEASUREMENTS_ERROR 2                                //Maksymalna różnica w pomiarach

//Przycisk
#define BUTTON_AP_GPIO 4 //Numer pinu do którego jest podłączony przycisk włączenia punktu dostępowego

//Głęboki sen
#define SECOND_IN_MICROSECOND 1000000                     //Sekunda w mikrosekundach
#define DEEP_SLEEP_TIME_SECONDS 2 * SECOND_IN_MICROSECOND //Czas głębokiego snu ESP8266 [s]

//Odczyt stanu baterii
#define ADC_PIN A0