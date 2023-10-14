#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include "wifi_credentials.h"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient net;
MQTTClient mqtt_client;

// Constants
int analogInput = A0;

const char TOPIC[] = "/voltage";

IPAddress MQTT_IP(192, 168, 100, 203);
const int MQTT_PORT = 1883;

float vout = 0.0;
float vin = 0.0;
float R1 = 98800;  // Resistor R1 100k
float R2 = 9710;   // Resistor R2 10k

// Global values
int value = 0;
String $vin = "0";
unsigned long lastMillis = 0;

String message = "{"
                 "\"name\": \"Solar Power Detector\","
                 "\"unique_id\": \"solar_power_detector_001\","
                 "\"state_topic\": \"solar_power_detector/voltage\","
                 "\"availability_topic\": \"solar_power_detector/status\","
                 "\"payload_available\": \"online\","
                 "\"payload_not_available\": \"offline\","
                 "\"device\": {"
                 "\"identifiers\": \"solar_power_detector_001\","
                 "\"manufacturer\": \"Tiberiu\","
                 "\"name\": \"Solar Power Detector\","
                 "\"model\": \"ESP8266\","
                 "\"sw_version\": \"1.0\""
                 "},"
                 "\"device_class\": \"voltage\","
                 "\"unit_of_measurement\": \"V\""
                 "}";

void connect_mqtt() {
  mqtt_client.begin(MQTT_IP, MQTT_PORT, net);
  Serial.print("checking wifi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting to mqtt server...");

  while (!mqtt_client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected to mqtt broker!");
  // mqtt_client.publish("homeassistant/sensor/solar_power_detector_001/config", message.c_str());
}

void connect_wifi() {
  Serial.println("checking wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
    delay(1000);
  }
  Serial.print("Connected, IP address: ");
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
  connect_mqtt();

  display.clearDisplay();  // Clear the buffer
}

void loop(void) {
  value = analogRead(analogInput);
  vout = ((value - 6) * 3.3) / 1023.0;  // could be 3.3 or 5
  vin = vout / (R2 / (R1 + R2));
  $vin = String(vin);
  Serial.println(value);

  mqtt_client.loop();

  // if (!mqtt_client.connected()) {
  //   connect_mqtt();
  // }

  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    if (!mqtt_client.publish("solar_power_detector/voltage/get", $vin)) {
      Serial.println("Failed to publish");
    };

    mqtt_client.publish(TOPIC, $vin);  // publish a message roughly every second.
  }

  showonoled();
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

