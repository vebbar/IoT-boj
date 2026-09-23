# IoT-Boj LoRa Mottagare

LoRa-mottagare byggd på **Waveshare ESP32-S3-POE-ETH** med extern **SX1276** LoRa-modul. Tar emot textpaket från sändarbojen (Heltec Wireless Stick Lite V3) och skriver ut dem i Serial Monitor. Loggar också till den inbyggda TF-kortläsaren.

## Hårdvara

| Komponent | Modell |
|---|---|
| Mottagare | Waveshare ESP32-S3-POE-ETH |
| LoRa-modul | SX1276 (extern, via kablage) |
| (Valfritt) | TF-kort (inbyggt på mottagar-kortet) |

## Projektbeskrivning

Denna kod körs på en Waveshare ESP32-S3-POE-ETH och tar emot textpaket från sändarbojen via en extern SX1276 LoRa-modul. Varje mottaget paket skrivs ut i Serial Monitor tillsammans med RSSI och SNR, och loggas till den inbyggda TF-kortläsaren på kortet.

LoRa-inställningarna måste matcha sändaren: 868.0 MHz, bandbredd 125 kHz, spreading factor 7, coding rate 4/5 och sync word 0x3B. Payloaden innehåller temperaturer, sensorstatus och batterivärden från bojen och behandlas i nuläget som text.

## Viktiga pin-konflikter (ESP32-S3-POE-ETH)

ESP32-S3-POE-ETH har GPIO4/5/6/7 hårdkopplade till den inbyggda TF-kortplatsen, och GPIO9/10/11/12/13/14 hårdkopplade till det inbyggda W5500 Ethernet-chippet. De pinnarna kan INTE återanvändas för den externa LoRa-modulen. Därför körs LoRa och TF-kortet nu på två separata SPI-bussar med olika pinnar, istället för att dela en buss som i den gamla ESP32-C3-versionen.

GPIO0, GPIO3, GPIO45 och GPIO46 är strapping-pinnar och undviks medvetet.

NAS-stöd är förberett men avstängt. Senare kan `USE_NAS` och `sendToNas()` byggas ut med WiFi/HTTP (eller Ethernet via W5500) så att mottagna data skickas vidare till NAS.

> **OBS – innan detta pushas till en publik server:** `NAS_URL` är just nu en privat (lokal) IP-adress och skadar inte att dela. Om ni senare pekar mot en publik adress med autentisering, lägg autentiseringsuppgifter i en `.env`/hemlighetsfil (se `.gitignore` i repo-roten), aldrig direkt i koden.

## Pinout

### LoRa (extern SX1276, egen SPI-buss "loraSPI")
| Funktion | GPIO |
|---|---|
| SCK | 18 |
| MISO | 16 |
| MOSI | 17 |
| CS | 1 |
| RESET | 15 |
| DIO0 | 2 |

### TF-kort (inbyggt på kortet, egen SPI-buss "sdSPI", FASTA pinnar)
| Funktion | GPIO |
|---|---|
| SCK | 7 |
| MISO | 5 |
| MOSI | 6 |
| CS | 4 |

### Inbyggd LED (NeoPixel, GPIO 21)
| Status | LED |
|---|---|
| Lyssnar (ingen data) | Grön puls, mycket dim |
| Fel (CRC/error) | Röd, mycket dim |
| Paket mottaget | Blå blink (1 sekund) |

LED-styrning är standard inaktiverad (`USE_LED=0` i `main.cpp`). För att aktivera: sätt `#define USE_LED 1`

### Reserverat av Ethernet (W5500) — används inte av denna kod
GPIO9 (ETH_RST), GPIO10 (ETH_INT), GPIO11 (ETH_MOSI), GPIO12 (ETH_MISO), GPIO13 (ETH_CLK), GPIO14 (ETH_CS)

GND är gemensam för alla moduler.

## LoRa-inställningar (måste matcha sändare)

| Parameter | Värde |
|---|---|
| Frekvens | 868.0 MHz |
| Bandbredd | 125 kHz |
| Spreading factor | 7 |
| Coding rate | 4/5 |
| Sync word | 0x3B |

## Bygg och ladda upp

```bash
pio run -t upload
pio device monitor
```

Serial Monitor körs på 115200 baud (inställt i `platformio.ini`).

## Status

- `USE_SD` — SD-loggning aktiverad (sätt till 0 för felsökning utan TF-kort)
- `USE_NAS` — inaktivt (för framtida WiFi/Ethernet-integrering)
