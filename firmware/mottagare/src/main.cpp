// Se README.md för full dokumentation av pinout, LoRa-inställningar och LED-beteende.

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// TF-kortet sitter inbyggt på kortet, så SD-loggning är påslaget som standard.
// Sätt till 0 om du vill köra utan SD-loggning (t.ex. för felsökning).
#define USE_SD 1
// Forberedelse for framtida NAS-integration (t.ex. via Ethernet/W5500).
constexpr bool USE_NAS = false;
constexpr char NAS_URL[] = "http://192.168.1.100:8086/api/v1/temps";

// ---------- LoRa (extern SX1276-modul, egen SPI-buss) ----------
// OBS! GPIO21 är enligt schemat (R15, 0R) hårdkopplad till den inbyggda WS2812
// RGB-lysdioden. LORA_CS flyttad hit ifrån för att slippa krocka med den.
constexpr int LORA_SCK   = 18;
constexpr int LORA_MISO  = 16;
constexpr int LORA_MOSI  = 17;
constexpr int LORA_CS    = 1;
constexpr int LORA_RESET = 15;
constexpr int LORA_DIO0  = 2;

constexpr float LORA_FREQUENCY = 868.0;
constexpr float LORA_BANDWIDTH = 125.0;
constexpr uint8_t LORA_SPREADING_FACTOR = 7;
constexpr uint8_t LORA_CODING_RATE = 5;
constexpr uint8_t LORA_SYNC_WORD = 0x3B;
constexpr int8_t LORA_POWER = 12;

// ---------- TF-kort (inbyggt, fasta pinnar - får INTE ändras) ----------
constexpr int SD_SCK  = 7;
constexpr int SD_MISO = 5;
constexpr int SD_MOSI = 6;
constexpr int SD_CS   = 4;

// Två separata SPI-bussar så att LoRa och TF-kortet inte krockar.
SPIClass loraSPI(FSPI);
#if USE_SD
SPIClass sdSPI(HSPI);
#endif

Module loraModule(LORA_CS, LORA_DIO0, LORA_RESET, RADIOLIB_NC, loraSPI);
SX1276 radio(&loraModule);

unsigned long lastHeartbeat = 0;
constexpr unsigned long HEARTBEAT_INTERVAL = 30000; // 30 sekunder

bool sdReady = false;

// ---------- LED (inbyggd WS2812 NeoPixel, GPIO 21 på Waveshare ESP32-S3-POE-ETH) ----------
// GPIO21 är nu fri för LED eftersom LORA_CS flyttades till GPIO1 ovan.
#define USE_LED 1
constexpr int LED_PIN = 21;
constexpr int LED_DIM = 5;                    // Mycket svag hellighet (0-255)
constexpr int LED_BLUE_DURATION = 1000;       // 1 sekund
constexpr int LED_BLINK_MS = 200;             // Blink-intervall

#if USE_LED
unsigned long ledBlueStart = 0;
bool ledBlueActive = false;
#endif

#if USE_SD
#include <SD.h>
#endif

void logToSd(const String& payload) {
#if USE_SD
  if (!sdReady) {
    return;
  }

  File logFile = SD.open("/temperature_log.txt", FILE_APPEND, true);
  if (!logFile) {
    Serial.println("SD: kunde inte oppna temperature_log.txt");
    return;
  }

  logFile.println(payload);
  logFile.close();
#endif
}

void sendToNas(const String& payload) {
  if (USE_NAS) {
    // NAS skickas inte annu. Behall payloaden har for framtida WiFi/HTTP-stod.
    Serial.print("NAS ej implementerad, URL: ");
    Serial.println(NAS_URL);
    Serial.println(payload);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Längre delay för USB CDC att komma igång

  Serial.println();
  Serial.println("=== IoT-Boj LoRa-mottagare BOOT (ESP32-S3-POE-ETH) ===");
  Serial.println("LoRa-SPI: SCK=GPIO18 MISO=GPIO16 MOSI=GPIO17");
  Serial.println("LoRa: CS=GPIO1 RESET=GPIO15 DIO0=GPIO2");
  Serial.println("TF-kort (inbyggt): SCK=GPIO7 MISO=GPIO5 MOSI=GPIO6 CS=GPIO4");

  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

#if USE_SD
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("SD: initiering misslyckades (kontrollera att TF-kort sitter i), fortsatter utan SD");
    sdReady = false;
  } else {
    Serial.println("SD: initiering OK, loggar till /temperature_log.txt");
    sdReady = true;
  }
#else
  Serial.println("SD: avstangd i kod (USE_SD=0), LoRa fungerar utan SD-kort");
#endif

  Serial.println("Initierar LoRa...");
  int state = radio.begin(
      LORA_FREQUENCY,
      LORA_BANDWIDTH,
      LORA_SPREADING_FACTOR,
      LORA_CODING_RATE,
      LORA_SYNC_WORD,
      LORA_POWER,
      8);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa init misslyckades, felkod: ");
    Serial.println(state);
    Serial.println("Kontrollera strom, GND, SPI-pinnar och SX1276-kopplingen.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("LoRa init OK");
  Serial.println("Vantar pa paket...");

#if USE_LED
  neopixelWrite(LED_PIN, 0, 0, 0);  // Stäng av inbyggd LED direkt
#endif
}

void loop() {
  unsigned long now = millis();

#if USE_LED
  // LED: blå blink (1 sekund) efter mottaget paket
  if (ledBlueActive && now - ledBlueStart < LED_BLUE_DURATION) {
    neopixelWrite(LED_PIN, 0, 0, ((now / LED_BLINK_MS) % 2) ? LED_DIM : 0);
  } else if (ledBlueActive && now - ledBlueStart >= LED_BLUE_DURATION) {
    ledBlueActive = false;
    neopixelWrite(LED_PIN, 0, 0, 0);
  }

  // LED: grön puls när ingen blå blink pågår
  if (!ledBlueActive) {
    unsigned long t = now % 2000;
    uint8_t brightness = (t < 1000) ? (t * LED_DIM / 1000) : ((2000 - t) * LED_DIM / 1000);
    neopixelWrite(LED_PIN, 0, brightness, 0);
  }
#endif

  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    Serial.print("[HB] ");
    Serial.print(now / 1000);
    Serial.print("s - lyssnar på 868.0 MHz (SF7, BW125, CR4/5, Sync 0x12) - SD: ");
    Serial.println(sdReady ? "OK" : "AV");
  }

  String payload;
  int state = radio.receive(payload);

  if (state == RADIOLIB_ERR_NONE) {
#if USE_LED
    ledBlueActive = true;
    ledBlueStart = millis();
    neopixelWrite(LED_PIN, 0, 0, LED_DIM);
#endif

    Serial.println();
    Serial.println("--- LoRa-paket mottaget ---");
    Serial.print("RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.println(" dBm");
    Serial.print("SNR: ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");
    Serial.print("Payload: ");
    Serial.println(payload);

    logToSd(payload);
    sendToNas(payload);
    Serial.println("---------------------------");
  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    Serial.println("LoRa-paket avvisat: CRC-fel");
#if USE_LED
    if (!ledBlueActive) neopixelWrite(LED_PIN, LED_DIM, 0, 0);
#endif
  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    delay(100);
  } else {
    Serial.print("LoRa mottagningsfel: ");
    Serial.println(state);
#if USE_LED
    if (!ledBlueActive) neopixelWrite(LED_PIN, LED_DIM, 0, 0);
#endif
    delay(100);
  }
}
