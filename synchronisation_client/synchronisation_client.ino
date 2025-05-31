#include <WiFi.h>
#include <esp_now.h>
#include "time.h"
#include "driver/i2s.h"
#include "sound_sample.h"  // contains audio[] and length

#define I2S_DOUT  4  // DIN on MAX98357A
#define I2S_BCLK  3  // BCLK
#define I2S_LRC   2  // LRC (WS)

const char* ssid = "iPhone de Aaliyah"; // Nom de la box Wi-Fi
const char* password = "244466666"; // MDP de la box Wi-Fi

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 1;
const int   daylightOffset_sec = 3600;

typedef struct struct_message {
  int hour;
  int minute;
  int second;
  int OFFSET_synchro;
} struct_message;

struct_message incomingTime;

// Updated callback to match new API
void onReceiveData(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  memcpy(&incomingTime, incomingData, sizeof(incomingTime));
  Serial.printf("Received time: %02d:%02d:%02d %d\n", incomingTime.hour, incomingTime.minute, incomingTime.second, incomingTime.OFFSET_synchro);

}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // On configure le seveur NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
///////////////////////////////////////////////////////////////////
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceiveData);
  Serial.println("ESP-NOW Receiver Initialized");
  /////////////////////////////////////////////////////////
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 8000, //8000
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, //I2S_BITS_PER_SAMPLE_16BIT
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  int currentSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  int targetSeconds = incomingTime.hour * 3600 + incomingTime.minute * 60 + incomingTime.second + incomingTime.OFFSET_synchro;

  if (currentSeconds == targetSeconds) {
    Serial.println("Time matched! Playing sound.");
    for (size_t i = 0; i < audio_length; i++) {
      uint8_t s = audio[i];
      int16_t sample = ((int16_t)s - 128) * 85; // Adjust volume
      size_t written;
      i2s_write(I2S_NUM_0, &sample, sizeof(sample), &written, portMAX_DELAY);
    }

    delay(2000); // Optional delay to avoid repeat
  }

  delay(100); // Check more frequently
}

