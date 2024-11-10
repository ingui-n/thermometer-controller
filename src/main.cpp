#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <env.h>
#include <ESP8266HTTPClient.h>
#include <espnow.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

IPAddress localIP(192, 168, 0, 2); // change to your local IP
IPAddress gateway(192, 168, 0, 1); // change to your gateway
IPAddress subnet(255, 255, 255, 0); // change to your subnet
IPAddress primaryDNS(192, 168, 0, 1); // change to your primary DNS
IPAddress secondaryDNS(1, 1, 1, 1); // change to your secondary DNS

JsonDocument messageDoc;
const JsonArray messagesArray = messageDoc.to<JsonArray>();

void saveTemperature(const String &device = "", const double temperature = 0.0) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, String(POCKETBASE_URL) + String(POCKETBASE_TEMPERATURE_ENDPOINT));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(POCKETBASE_BEARER));

    const String postData = "{\"temperature\": " + String(temperature, 2) + R"(, "device": ")" + device
                            + "\"}";

    http.setTimeout(10000);

    Serial.println("Sending POST request of: " + postData);
    http.POST(postData);
    http.end();
}

void processNewMessages() {
    for (size_t i = 0; i < messagesArray.size(); i++) {
        JsonObject message = messagesArray[i];

        const char *device = message["device"];
        const char *password = message["password"];
        const double temperature = message["temperature"];

        if (strcmp(password, ESP_NOW_PASSWORD) == 0) {
            saveTemperature(device, temperature);
        }

        messagesArray.remove(i);
    }
}

void onDataReceive(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
    JsonDocument message;
    DeserializationError error = deserializeJson(message, incomingData);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    const JsonObject newMessage = messagesArray.add<JsonObject>();
    newMessage["device"] = message["device"];
    newMessage["password"] = message["password"];
    newMessage["temperature"] = message["temperature"];
}

void setup() {
    Serial.flush();
    Serial.begin(SERIAL_BAUDRATE);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Waiting for WiFi to connect...");
        delay(500);
    }

    Serial.println("WiFi connected");

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataReceive);
}

void loop() {
    processNewMessages();
}
