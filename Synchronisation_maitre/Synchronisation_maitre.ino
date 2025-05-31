#include <WiFi.h>
#include <esp_now.h>
#include "time.h"
#include <WiFiClient.h>
#include <esp_wifi.h>


const char* ssid = "iPhone de Aaliyah";
const char* password = "244466666";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 1;
const int   daylightOffset_sec = 3600;

const int OFFSET_synchro = 5;

//  Adresses MAC des deux récepteurs
uint8_t claraMac[]    = {0xF0, 0xF5, 0xBD, 0x2C, 0x25, 0xE8};
uint8_t doniettaMac[] = {0xF0, 0xF5, 0xBD, 0x2C, 0x23, 0x38};

typedef struct struct_message {
  int hour;
  int minute;
  int second;
  int OFFSET_synchro;
} struct_message;

struct_message timeData;

void addPeer(uint8_t *mac) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println(" Failed to add peer");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (esp_now_init() != ESP_OK) {
    Serial.println(" Error initializing ESP-NOW");
    return;
  }

  // Ajout des deux peers
  addPeer(claraMac);
  addPeer(doniettaMac);
  Serial.println(" ESP-NOW Peers added");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    timeData.hour = timeinfo.tm_hour;
    timeData.minute = timeinfo.tm_min;
    timeData.second = (timeinfo.tm_sec + OFFSET_synchro) % 60;
    timeData.OFFSET_synchro = OFFSET_synchro;

    // Envoi vers Clara
    esp_err_t result1 = esp_now_send(claraMac, (uint8_t *) &timeData, sizeof(timeData));
    // Envoi vers Donietta
    esp_err_t result2 = esp_now_send(doniettaMac, (uint8_t *) &timeData, sizeof(timeData));

    if (result1 == ESP_OK) {
      Serial.printf("Time sent to Clara: %02d:%02d:%02d + %d s\n",
        timeData.hour, timeData.minute, timeData.second, timeData.OFFSET_synchro);
    } else {
      Serial.println("Failed to send to Clara");
    }

    if (result2 == ESP_OK) {
      Serial.printf("Time sent to Donietta: %02d:%02d:%02d + %d s\n",
        timeData.hour, timeData.minute, timeData.second, timeData.OFFSET_synchro);
    } else {
      Serial.println(" Failed to send to Donietta");
    }
  } else {
    Serial.println("Failed to get time");
  }

  delay(1000);
}

void loop() {
}


