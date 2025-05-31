#include <esp_now.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>

// Définition des broches utilisées pour la communication SPI avec la carte SD
#define SD_CS    5     // Chip Select
#define SD_MOSI  23    // Master Out Slave In
#define SD_MISO  19    // Master In Slave Out
#define SD_SCK   18    // Horloge SPI

SPIClass spiSD;       // Instance SPI dédiée à la carte SD
File mp3File;         // Objet pour manipuler le fichier MP3

// Adresse MAC du client (ESP32 récepteur) à qui les fragments MP3 seront envoyés
uint8_t clientMAC[] = {0x24, 0x6F, 0x28, 0xAB, 0xCD, 0xEF};

// Structure représentant un fragment de fichier MP3
typedef struct {
  uint16_t index;     // Index du fragment (numéro de chunk)
  uint16_t total;     // Nombre total de fragments
  uint8_t data[200];  // Données audio (200 octets par fragment)
} Mp3Fragment;

bool ackReceived = false;  // Drapeau indiquant la réception d’un accusé de réception

// Callback déclenché après l’envoi d’un message ESP-NOW
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Sent FAIL");
}

// Callback appelé à la réception d’un message via ESP-NOW (attend un ACK du client)
void onAck(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (incomingData[0] == 1) {
    ackReceived = true;  // Si la donnée reçue commence par 1, considérer comme un ACK
  }
}

void setup() {
  Serial.begin(115200); // Initialisation de la communication série pour le débogage

  // Configuration du Wi-Fi en mode station (STA) pour activer ESP-NOW
  WiFi.mode(WIFI_STA);

  // Initialisation de ESP-NOW
  esp_now_init();
  esp_now_register_send_cb(onDataSent);  // Enregistrement du callback d’envoi
  esp_now_register_recv_cb(onAck);       // Enregistrement du callback de réception

  // Ajout du client comme pair ESP-NOW
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, clientMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Initialisation du bus SPI dédié à la carte SD
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("SD init failed");  // Échec de l'initialisation de la carte SD
    return;
  }

  // Ouverture du fichier MP3 depuis la carte SD
  mp3File = SD.open("/music.mp3", FILE_READ);
  if (!mp3File) {
    Serial.println("Failed to open file"); // Échec à l’ouverture du fichier
    return;
  }

  // Calcul du nombre de fragments de 200 octets nécessaires
  uint32_t fileSize = mp3File.size();
  uint16_t totalChunks = (fileSize + 199) / 200;
  Serial.printf("File size: %d, Chunks: %d\n", fileSize, totalChunks);

  // Lecture et envoi de chaque fragment via ESP-NOW
  for (uint16_t i = 0; i < totalChunks; i++) {
    Mp3Fragment fragment;
    fragment.index = i;
    fragment.total = totalChunks;
    memset(fragment.data, 0, 200);              // Initialisation à 0 du buffer
    mp3File.read(fragment.data, 200);           // Lecture des 200 octets suivants

    ackReceived = false;
    esp_now_send(clientMAC, (uint8_t *)&fragment, sizeof(fragment));  // Envoi du fragment

    // Attente d’un accusé de réception pendant 500 ms maximum
    uint32_t start = millis();
    while (!ackReceived && millis() - start < 500) {
      delay(10);  // Délai court pour éviter la surcharge CPU
    }

    // Si aucun ACK reçu, arrêter le processus
    if (!ackReceived) {
      Serial.printf("No ACK for chunk %d, aborting.\n", i);
      break;
    }
  }

  // Fermeture du fichier après transmission
  mp3File.close();
  Serial.println("File sent.");
}

void loop() {
  // Boucle principale vide — le programme ne fait qu’un envoi au démarrage
}
