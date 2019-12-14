#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include "configPage.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG true
#define SSID "Dom_03_2.4G"
#define PASSWORD "gn9j-0dln-2cp1"
#define HOST "script.google.com"
#define HTTPS_PORT 443

#define AP_SSID "Esp-term"
#define AP_PASSWORD "esp"

#define CONFIG_SERVER_PORT 80
#define CONFIG_SERVER_HTML_FILE_LOCATION "/"

#define SERVER_FUNCTION_TO_ADD_DATA "addData"

#define SERVER_FUNCTION_VARIABLE "func"
#define SERVER_TEMPERATURE_VARIABLE "temp"
#define SERVER_ERROR_VARIABLE "errorCode"

#define NO_ERROR_CODE '0'
#define DISCONNECTED_SENSOR_ERROR_CODE '1'

#define TEMP_SENSOR_GPIO D1
#define TIME_BETWEEN_CHECK_AND_READ_TEMPERATURE_MILISECONDS 800
#define TIME_BETWEEN_MEASUREMENT_MILISECONDS 300
#define MAX_MEASUREMENTS_ERROR  2

#define BUTTON_GPIO D2

#define SECOND_IN_MICROSECOND 1000000
#define DEEP_SLEEP_TIME_SECONDS 5 * SECOND_IN_MICROSECOND