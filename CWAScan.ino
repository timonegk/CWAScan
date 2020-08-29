#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "CWAScanConfig.h"

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
std::vector<std::string> cwaAddresses;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Filter for CWA service id (fd6f)
      if (advertisedDevice.getServiceUUID().toString().rfind("0000fd6f", 0) == 0) {
        cwaAddresses.push_back(advertisedDevice.getAddress().toString());
      }
    }
};

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
  
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("Scan done!");
  for (std::string a: cwaAddresses) {
    Serial.println(a.c_str());
  }
  cwaAddresses.clear();
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client->setCACert(ROOT_CA);
    HTTPClient https;
    if (https.begin(*client, REMOTE_URL)) {
      https.setAuthorization(REMOTE_USER, REMOTE_PASS);
      String data;
      data.concat(time(nullptr));
      data.concat("\n");
      for (std::string& a: cwaAddresses) {
        data.concat(a.c_str());
        data.concat("\n");
      }
      int resp = https.POST(data);
      Serial.println(resp);
      https.end();
    }
  }
  
  delay(2000);
}
