#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "ArduinoJson.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SoftwareSerial.h>
#include <SPI.h>
#include <string.h>
#include <stdio.h>

#define TFT_CS        D3
#define TFT_RST       D4 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        D2

#define RX D1
#define TX D0

SoftwareSerial mySerial(D1, D0); // RX, TX
#include <SPI.h>

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

char a[15] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
int k = 0;

char ssid[] = "Tarr";
char password[] = "11223344";

WiFiClientSecure client;
const char* host = "00j5ig24l4.execute-api.ap-southeast-1.amazonaws.com";
String url = "/test/junk/scan";
const char fingerprint[] = "3E 45 18 D3 53 0E 50 A6 A7 A1 1C 8C EF 9E B3 DA 5E 12 C9 84";

void setup() {
  // Monitor and Barcode Reader
  Serial.begin(9600);
  while (!Serial);
  mySerial.begin(9600);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0); // set display orientation
  tft.fillScreen(ST77XX_BLACK);

  //WiFi connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  checkAWS();
}

void loop() {
  if (mySerial.available()) {
    // read the incoming byte:
    char r = mySerial.read();
    // Serial.println(r);
    a[k] = r;
    k++;
    // Serial.println(k);

    if (k == 15) {
      Serial.print("ID : ");
      for (int i = 0; i < 15; i++) {
        Serial.print(a[i]);
      }
      Serial.println(a[15]);
      scan(String(a));
      k = 0;
      resetCharArray(a, sizeof(a) - 1);
    }
  }
  delay(10);
}

void checkAWS() {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, 443)) {
    Serial.println("connection failed");
    return;
  }
  
  Serial.print("requesting URL: ");
  Serial.println(url);
}



void scan(String junkId) {
  String body = "{\"junkId\": \"" + junkId + "\"}";
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded \r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "\r\n" + body);

               Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  lcd(line);
  delay(1000);
}

void resetCharArray(char arr[], int length) {
  // Fill the array with '0' characters
  memset(arr, '0', length);
  // Add null terminator at the end of the array
  arr[length] = '\0';
}

void lcd(String text) {

  if (text == "\"D\"") {
    tft.fillScreen(ST77XX_BLUE);
    tft.print("GENERAL");
    Serial.println("GENERAL");
  }
  if (text == "\"G\"") {
    tft.fillScreen(ST77XX_GREEN);
    tft.print("RECYCLE");
    Serial.println("RECYCLE");
  }
  if (text == "\"Y\"") {
    tft.fillScreen(ST77XX_YELLOW);
    tft.print("YELLOW");
    Serial.println("YELLOW");
  }
  if (text == "\"T\"") {
    tft.fillScreen(ST77XX_RED);
    tft.print("HAZRAD");
    Serial.println("HAZARD");
  }
  if (text == "SCAN") {
    tft.fillScreen(ST77XX_BLACK);
    tft.print("SCAN");
    Serial.println("SCAN");
  }
}