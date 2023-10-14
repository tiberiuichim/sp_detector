#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
// #include <MQTT.h>
#include "config.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient wifiClient;
PubSubClient pubsub_client(wifiClient);

// Constants
int analogInput = A0;

float vout = 0.0;
float vin = 0.0;
float R1 = 98800;  // Resistor R1 100k
float R2 = 9710;   // Resistor R2 10k

// Global values
int value = 0;
String $vin = "0";
unsigned long lastMillis = 0;

void publishData(float p_voltage) {
  DynamicJsonDocument doc(200);
  doc["voltage"] = (String)p_voltage;

  serializeJson(doc, Serial);
  Serial.println("");

  char data[200];
  serializeJson(doc, data);

  pubsub_client.publish(MQTT_SENSOR_TOPIC, data, true);
  yield();
}

void publishConfig() {
  int len = CONFIG_MESSAGE.length();
  char arrMsg[len];
  CONFIG_MESSAGE.toCharArray(arrMsg, len);

  if (!pubsub_client.publish(CONFIG_PATH, arrMsg, true)) {
    Serial.println("Unable to publish config");
  };
  yield();
}


// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
}

void reconnect() {
  // Loop until we're reconnected
  while (!pubsub_client.connected()) {
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (pubsub_client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(pubsub_client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}

void connect_wifi() {
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.println("INFO: IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  pinMode(analogInput, INPUT);
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.display();

  connect_wifi();

  pubsub_client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  pubsub_client.setCallback(callback);
  //reconnect();

  display.clearDisplay();  // Clear the buffer
}

void loop(void) {
  value = analogRead(analogInput);

  if (!pubsub_client.connected()) {
    reconnect();
    publishConfig();
  }
  pubsub_client.loop();

  vout = ((value - 6) * 3.3) / 1023.0;  // could be 3.3 or 5
  vin = vout / (R2 / (R1 + R2));
  $vin = String(vin);
  Serial.println(value);

  showonoled();
  publishData(vin);
  publishConfig();
  delay(1000);
}

void showonoled() {
  display.clearDisplay();
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.print($vin + F("V") + "\n");
  display.display();
}
