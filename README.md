# IoT-boj – temperaturmätning i insjö

> **Status: pågående projekt.** Detta repo speglar var projektet befinner sig just nu – hårdvaruval och krav är på plats, men kod, CAD och testning är inte klara ännu. Bidrag, förslag och issues är välkomna!

## Syfte

En solcellsdriven boj som mäter vattentemperatur på flera djup i en insjö och skickar mätdata trådlöst (LoRa) till en mottagare/basstation, som i sin tur vidarebefordrar datan till en NAS/server som publicerar den på en hemsida.

## Krav och funktion i korthet

- Mäter vattentemperatur på tre djup: **-0,3 m, -1 m, -2 m**, monterade på ett 2 m långt aluminiumrör under bojen
- Skickar temperatur + batteristatus **en gång i timmen**
- Placeras i allmänt vatten → ska vara **hyfsat billig** (risk för vandalisering/stöld)
- Ute **året runt i Stockholmsområdet** → måste tåla kyla
- Höljet **3D-printas**
- Strömförsörjs via **solcell (6 V/2 A) + 2x 18650-batteri**
- Kommunicerar via **LoRa (868 MHz)**, ca 100 m genom vegetation, till en mottagare
- Kommunikation sker alltid **boj → mottagare → server** (push), aldrig tvärtom

> Se [`docs/projektsammanfattning.md`](docs/projektsammanfattning.md) för den senaste, fullständiga versionen.

Fullständiga krav finns i [`docs/syfte-och-krav.md`](docs/syfte-och-krav.md) och en mer utförlig sammanfattning (inkl. förankringsberäkning) i [`docs/projektsammanfattning.md`](docs/projektsammanfattning.md).

## Arkitektur

```
[Boj: Heltec Wireless Stick Lite V3    --LoRa 868MHz-->  [Mottagare: Waveshare
 (ESP32-S3+SX1262) + 3x DS18B20]                          ESP32-S3-POE-ETH + SX1276]
                                                                    |
                                                             push data
                                                                    v
                                                          [NAS/server → hemsida]
```

Mottagaren står fast vid sjön. Servern kan flytta med användaren till andra nät – därför initieras kommunikationen alltid från mottagaren mot servern, aldrig tvärtom.

## CAD

Höljet och de mekaniska delarna designas i Onshape: se [`cad/onshape-link.md`](cad/onshape-link.md) för länk och status.

kod för NAS/server-sidan som publicerar datan på hemsidan.

## Hårdvara (sammanfattning)

**Boj**
- Heltec Wireless Stick Lite V3 (ESP32-S3 + inbyggt LoRa SX1262), 868 MHz
- 3x DS18B20 vattentäta temperatursensorer (0,3/1/2 m djup, på aluminiumrör)
- 2x 18650 Li-ion-celler med integrerad BMU
- Solcell 6 V/2 A + laddkrets CN3065 + buck-boost-converter
- INA219 för batteritelemetri

**Mottagare**
- Waveshare ESP32-S3-POE-ETH
- extern SX1276 LoRa-modul

Se hela inköpslistan i [`hardware/inkopslista/inkopslista.pdf`](hardware/inkopslista/inkopslista.pdf) (obs: listan speglar ett tidigare hårdvaruval med Raspberry Pi som mottagare — den slutgiltiga mottagaren är nu Waveshare ESP32-S3-POE-ETH, se ovan).

**Regulatoriskt:** Sändare på 868 MHz, duty cycle max 1 % i bandet 868,0–868,6 MHz (~36 sek/timme) enligt PTS.

## Kända öppna frågor / risker

- **Is** är den största risken året runt i Stockholm – inte vind/vågor. Ett betongankare löser inte problemet med isflak. Att ta upp bojen inför islägget är den säkraste lösningen.
- Exakt placering (sjönamn/koordinater) publiceras medvetet **inte** här, för att minska risken för stöld/vandalisering.

## Licens

Ingen licens är vald ännu. Lägg till en [`LICENSE`](https://choosealicense.com/)-fil innan ni delar koden brett om ni vill klargöra hur andra får använda materialet (t.ex. MIT för öppen återanvändning).

## Bidra

Detta är ett pågående hobbyprojekt. Öppna gärna en issue om du har frågor eller förslag.
