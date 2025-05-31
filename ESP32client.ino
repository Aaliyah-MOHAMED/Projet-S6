#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "Audio.h" // Bibliothèque Audio I2S

// Configuration réseau
const char* ssid = "ESP32_Master_Network";
const char* password = "123456789";
const char* serverIP = "192.168.4.1";
const int serverPort = 80;
const char* audioEndpoint = "/audio.wav";

// Configuration audio
Audio audio;
const int I2S_BCLK = 26;
const int I2S_LRC = 25;
const int I2S_DOUT = 22;

int currentX = 0;
int currentY = 100;

void connectToAP() {
  Serial.println("Connexion au réseau AP...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void downloadAndPlayAudio() {
  WiFiClient client;
  HTTPClient http;
  
  Serial.println("Téléchargement du fichier audio...");
  http.begin(client, "http://" + String(serverIP) + audioEndpoint);
  
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    // Configuration I2S
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
    // Lecture du flux audio
    audio.connecttohost("http://" + String(serverIP) + audioEndpoint);
    Serial.println("Lecture audio démarrée");
    
    // Maintenir la lecture
    while(audio.isRunning()) {
      audio.loop();
      delay(1);
    }
  } else {
    Serial.printf("Erreur HTTP: %d\n", httpCode);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  connectToAP();
  
  // Initialisation audio
  audio.setVolume(10); // Volume 0-21
}

void loop() {
  // Télécharge et joue le fichier audio
  downloadAndPlayAudio();
  
  // Votre logique existante pour la position
  // ... (gardez votre code de gestion de position ici)
  
  delay(10000); // Attend 10s entre chaque lecture
}
