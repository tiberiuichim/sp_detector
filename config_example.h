const char WIFI_SSID[] = "";
const char WIFI_PASSWORD[] = "";
const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 600; // 10 minutes x 60 seconds

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "solar_power_detector";
const PROGMEM char* MQTT_SERVER_IP = "192.168.100.203";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "";
const PROGMEM char* MQTT_PASSWORD = "";

// MQTT: topic
const PROGMEM char* MQTT_SENSOR_TOPIC = "solar_power_detector";

String CONFIG_MESSAGE = "{"
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

const char* CONFIG_PATH  = "homeassistant/sensor/solar_power_detector_001/config";
