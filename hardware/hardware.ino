//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################
//
// Updated to match your required wiring (NOT the template pin map):
// - ONE button on GPIO 4
// - LED_A on GPIO 18
// - LED_B on GPIO 19
//
// Still meets Experiment 1 behavior:
// - Startup shows 8 on 7-seg
// - Button press generates digit [0..9], displays it, publishes JSON:
//   {"id":"6200167361","timestamp":<epoch>,"number":<0-9>,"ledA":0/1,"ledB":0/1}
// - Subscribed messages with {"type":"toggle","device":"LED A"} or "LED B"
//   toggle the correct LED and publish updated state
//
// WiFi: SSID GalaxyS24, PASS 02032026
// MQTT broker: www.yanacreations.com:1883
// Topics: pub "<ID>", sub "<ID>_sub" and "/elet2415"

#include <rom/rtc.h>

#ifndef _WIFI_H
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif

#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

#define ARDUINOJSON_USE_DOUBLE 1

// ===== 7-SEG PINS (ESP32 GPIO) =====
#define a 15
#define b 32
#define c 33
#define d 25
#define e 26
#define f 27
#define g 14
#define dp 12

// ===== YOUR ACTUAL WIRING =====
// LEDs and button (custom wiring)
#define LED_A 18
#define LED_B 19
#define BTN_A 4

// MQTT CLIENT CONFIG
static const char *pubtopic = "6200167361";                       // Add your ID number here
static const char *subtopic[] = {"6200167361_sub", "/elet2415"};  // Topics to subscribe to
static const char *mqtt_server = "www.yanacreations.com";         // Broker
static uint16_t mqtt_port = 1883;

// WIFI CREDENTIALS
const char *ssid = "GalaxyS24";
const char *password = "02032026";

// TASK HANDLES
TaskHandle_t xMQTT_Connect = NULL;
TaskHandle_t xNTPHandle = NULL;
TaskHandle_t xLOOPHandle = NULL;
TaskHandle_t xUpdateHandle = NULL;
TaskHandle_t xButtonCheckeHandle = NULL;

// FUNCTION DECLARATION
void checkHEAP(const char *Name);
void initMQTT(void);
unsigned long getTimeStamp(void);
void callback(char *topic, byte *payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload);
void vButtonCheck(void *pvParameters);
void vUpdate(void *pvParameters);
void GDP(void);

// UTIL FUNCTIONS (required by template)
void Display(unsigned char number);
int8_t getLEDStatus(int8_t LED);
void setLEDState(int8_t LED, int8_t state);
void toggleLED(int8_t LED);

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables
uint8_t number = 8;  // Startup shows 8

// Digit segments map (common cathode: HIGH = on)
// If common anode, invert writes in Display().
static const uint8_t digits[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

void setup() {
  Serial.begin(115200); // INIT SERIAL

  // CONFIGURE THE ARDUINO PINS OF THE 7SEG AS OUTPUT
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(dp, OUTPUT);

  // CONFIGURE LED PINS AS OUTPUT
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // CONFIGURE BUTTON PIN AS INPUT WITH PULLUP
  pinMode(BTN_A, INPUT_PULLUP);

  // Default states
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(dp, LOW);

  // Display the integer ‘8’, on the seven-segment display on system start-up.
  Display(8);

  // Seed RNG
  randomSeed((uint32_t)esp_random());

  initialize(); // INIT WIFI, MQTT & NTP

  // Create button task (so it definitely runs)
  xTaskCreatePinnedToCore(
      vButtonCheck,
      "BTN",
      4096,
      (void *)1,
      1,
      &xButtonCheckeHandle,
      1);
}

void loop() {
  // Most course templates keep loop empty because FreeRTOS tasks do the work.
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

//####################################################################
//#                          UTIL FUNCTIONS                          #
//####################################################################
void vButtonCheck(void *pvParameters) {
  configASSERT(((uint32_t)pvParameters) == 1);

  static int lastA = HIGH;

  for (;;) {
    int nowA = digitalRead(BTN_A);

    // detect press (INPUT_PULLUP => pressed == LOW)
    if (lastA == HIGH && nowA == LOW) {
      GDP(); // On a button press: Generate, Display, Publish
    }

    lastA = nowA;

    vTaskDelay(50 / portTICK_PERIOD_MS); // basic debounce
  }
}

void vUpdate(void *pvParameters) {
  configASSERT(((uint32_t)pvParameters) == 1);

  for (;;) {
    // Optional periodic publish (NOT required for the button-only spec)
    JsonDocument doc;
    char message[256] = {0};

    doc["id"] = "6200167361";
    doc["timestamp"] = getTimeStamp();
    doc["number"] = number;
    doc["ledA"] = getLEDStatus(LED_A);
    doc["ledB"] = getLEDStatus(LED_B);

    serializeJson(doc, message);

    if (mqtt.connected()) {
      publish(pubtopic, message);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

unsigned long getTimeStamp(void) {
  // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
  time_t now;
  time(&now);
  return (unsigned long)now;
}

void callback(char *topic, byte *payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO

  Serial.printf("\nMessage received : ( topic: %s ) \n", topic);

  char *received = new char[length + 1]{0};
  for (unsigned int i = 0; i < length; i++) {
    received[i] = (char)payload[i];
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n", received);

  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);
  delete[] received;

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // PROCESS MESSAGE
  const char *type = doc["type"] | "";

  if (strcmp(type, "toggle") == 0) {
    // Process messages with ‘{"type": "toggle", "device": "LED A"}’ Schema
    const char *led = doc["device"] | "";

    if (strcmp(led, "LED A") == 0) {
      toggleLED(LED_A);
    }
    if (strcmp(led, "LED B") == 0) {
      toggleLED(LED_B);
    }

    // PUBLISH UPDATE BACK TO FRONTEND
    JsonDocument out;
    char message[256] = {0};

    // ‘{"id": "student_id", "timestamp": 1702212234, "number": 9, "ledA": 0, "ledB": 0}’
    out["id"] = "6200167361";
    out["timestamp"] = getTimeStamp();
    out["number"] = number;
    out["ledA"] = getLEDStatus(LED_A);
    out["ledB"] = getLEDStatus(LED_B);

    serializeJson(out, message);

    // Publish to a topic that only the Frontend subscribes to.
    publish(subtopic[0], message);
  }
}

bool publish(const char *topic, const char *payload) {
  bool res = false;
  try {
    res = mqtt.publish(topic, payload);
    if (!res) {
      throw false;
    }
  } catch (...) {
    Serial.printf("\nError (%d) >> Unable to publish message\n", res);
  }
  return res;
}

//***** Complete the util functions below ******

void Display(unsigned char n) {
  /* This function takes an integer between 0 and 9 as input.
     This integer must be written to the 7-Segment display */
  n = n % 10;

  digitalWrite(a, digits[n][0] ? HIGH : LOW);
  digitalWrite(b, digits[n][1] ? HIGH : LOW);
  digitalWrite(c, digits[n][2] ? HIGH : LOW);
  digitalWrite(d, digits[n][3] ? HIGH : LOW);
  digitalWrite(e, digits[n][4] ? HIGH : LOW);
  digitalWrite(f, digits[n][5] ? HIGH : LOW);
  digitalWrite(g, digits[n][6] ? HIGH : LOW);
  digitalWrite(dp, LOW);
}

int8_t getLEDStatus(int8_t LED) {
  // RETURNS THE STATE OF A SPECIFIC LED. 0 = LOW, 1 = HIGH
  return (int8_t)digitalRead(LED);
}

void setLEDState(int8_t LED, int8_t state) {
  // SETS THE STATE OF A SPECIFIC LED
  digitalWrite(LED, state ? HIGH : LOW);
}

void toggleLED(int8_t LED) {
  // TOGGLES THE STATE OF SPECIFIC LED
  int8_t s = getLEDStatus(LED);
  setLEDState(LED, !s);
}

void GDP(void) {
  // GENERATE, DISPLAY THEN PUBLISH INTEGER

  // GENERATE a random integer and assign to number
  number = (uint8_t)random(0, 10);

  // DISPLAY integer on 7Seg.
  Display(number);

  // PUBLISH number to topic.
  JsonDocument doc;
  char message[256] = {0};

  // ‘{"id": "student_id", "timestamp": 1702212234, "number": 9, "ledA": 0, "ledB": 0}’
  doc["id"] = "6200167361";
  doc["timestamp"] = getTimeStamp();
  doc["number"] = number;
  doc["ledA"] = getLEDStatus(LED_A);
  doc["ledB"] = getLEDStatus(LED_B);

  serializeJson(doc, message);
  publish(pubtopic, message);
}