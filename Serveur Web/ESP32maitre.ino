#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_wifi.h>
#include <time.h>
#include "home.h" // Inclure le fichier d'en-tête qui contient la définition de ConnectedClient

// Déclarations anticipées (si nécessaire)
void sendPositionToClient(int clientIndex, int xpos, int ypos);
void updateConnectedClients();

// Variables globales
ConnectedClient clients[10];
int clientCount = 0;

const char* ssid = "ESP32_Master_Network";
const char* password = "123456789";

WebServer server(80);
DNSServer dnsServer;

void setup() {
  Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Utiliser les fonctions définies dans home.cpp (que vous créerez)
  server.on("/", [=]() { handleRoot(server, clientCount, clients); });
  server.on("/set_position", HTTP_POST, [=]() { handleSetPosition(server, clientCount, clients, sendPositionToClient); });
  server.begin();
}

void loop() {
  server.handleClient();
  dnsServer.processNextRequest();

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 2000) {
    lastCheck = millis();
    updateConnectedClients();
  }
}

void updateConnectedClients() {
  wifi_sta_list_t stationList;
  if (esp_wifi_ap_get_sta_list(&stationList) != ESP_OK) {
    Serial.println("Erreur liste clients");
    return;
  }

  clientCount = min(stationList.num, 10);

  for (int i = 0; i < clientCount; i++) {
    wifi_sta_info_t station = stationList.sta[i];

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             station.mac[0], station.mac[1], station.mac[2],
             station.mac[3], station.mac[4], station.mac[5]);

    if (clients[i].mac != macStr) {
      clients[i].mac = macStr;
      clients[i].hostname = "Client-" + String(i + 1);
      clients[i].ip = "192.168.4." + String(i + 2);

      time_t now;
      time(&now);
      clients[i].connectionTime = String(now); // Store Unix timestamp as string

      sendPositionToClient(i, clients[i].x_pos, clients[i].y_pos);
    }
  }
}

// Implémentation de sendPositionToClient (doit rester ici car il accède aux variables globales)
void sendPositionToClient(int clientIndex, int xpos, int ypos) {
  if (clientIndex >= 0 && clientIndex < clientCount) {
    WiFiClient client;
    String clientIP = clients[clientIndex].ip;

    if (client.connect(clientIP.c_str(), 80)) {
      String url = "/update_position?x=" + String(xpos) + "&y=" + String(ypos);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + clientIP + "\r\n" +
                  "Connection: close\r\n\r\n");

      Serial.println("Position envoyée au client " + clients[clientIndex].mac +
                   " (X=" + String(xpos) + ", Y=" + String(ypos) + ")");
      client.stop();

      // Mettre à jour la position locale du client
      clients[clientIndex].x_pos = xpos;
      clients[clientIndex].y_pos = ypos;
    } else {
      Serial.println("Échec de connexion au client " + clients[clientIndex].mac);
    }
  } else {
    Serial.println("Index de client invalide: " + String(clientIndex));
  }
}