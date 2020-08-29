#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include "CWAScanConfig.h"

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}


void setup() {
  Serial.begin(115200);

  // Connecting to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFi.status() != WL_CONNECTED)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" connected");

  setClock();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  // Filter for CWA service UUID
  std::vector<std::string> cwaAddresses;
  for (int i = 0; i < foundDevices.getCount(); ++i) {
    if (foundDevices.getDevice(i).getServiceUUID().toString().rfind("0000fd6f", 0) == 0) {
      cwaAddresses.push_back(foundDevices.getDevice(i).getAddress().toString());
    }
  }

  WiFiClientSecure client;
  client.setCACert(ROOT_CA);

  HTTPClient https;
  if (https.begin(client, REMOTE_URL)) {
    https.setAuthorization(BASIC_AUTH_USER, BASIC_AUTH_PASSWORD);
    https.addHeader("Content-Type", "application/json");

    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(cwaAddresses.size()) + JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    doc["time"] = time(nullptr);

    JsonArray location = doc.createNestedArray("location");
    location.add(53.56);
    location.add(10.00);

    JsonArray data = doc.createNestedArray("data");
    for (const std::string& addr: cwaAddresses) {
      data.add(addr.c_str());
    }
    String jsonData;
    serializeJson(doc, jsonData);
    Serial.println(jsonData);

    int resp = https.POST(jsonData);
    Serial.println(resp);
    https.end();
  }

  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
}
