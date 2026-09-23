# IoT-boj – Projektsammanfattning (uppdaterad)

*Senast uppdaterad 2026-09-23.*

> Denna fil ersätter tidigare version. Den äldre versionen (med djupen -0,2/-1/-2 m) finns kvar i [`projektsammanfattning-v1-arkiv.md`](projektsammanfattning-v1-arkiv.md) för historik, men uppgifterna nedan är de aktuella.

## Syfte

Ett projekt bestående av en boj som mäter vattentemperaturen i en insjö i Stockholmsområdet, samt en mottagare/basstation som vidarebefordrar datan till en NAS/server som i sin tur publicerar den på en hemsida.

## Funktion

- Bojen har tre temperatursonder på tre djup: **0,3 m, 1 m, 2 m**
- Sonderna sitter monterade på ett **2 m långt aluminiumrör** under bojen
- Bojen skickar temperaturdata **en gång i timmen**, tillsammans med batteristatus
- Data tas emot av en mottagare ca **100 m** bort
- Mottagaren skickar vidare datan till en **NAS/server**, som publicerar den på en **hemsida**

## Hårdvara – Boj

- Styrenhet/radio: **Heltec Wireless Stick Lite (V3)** – ESP32-S3 + SX1262 LoRa-nod (Meshtastic- och LoRaWAN-kompatibel)
- 2x 18650-batterier med integrerad BMU (batterihanteringskrets)
- 3x DS18B20 vattentäta temperatursensorer
- Solcell, 6 V / 2 A
- Laddkrets: CN3065
- Buck-boost-converter (spänningsreglering)
- INA219 för batteritelemetri (spänning/ström)

## Hårdvara – Mottagare

- **Waveshare ESP32-S3-POE-ETH**
- Extern **SX1276** LoRa-modul

## Krav (bakgrund, se även syfte-och-krav.md)

- Placeras i allmänt vatten → ska vara hyfsat billig (risk för vandalisering/stöld)
- Ute året runt i Stockholm → måste tåla kyla
- Höljet 3D-printas
- Solcellsladdat batteri
- LoRa 868 MHz, ca 100 m genom vegetation
- Kommunikation sker boj/mottagare → server (push), aldrig tvärtom

## Förankring – beräkning och rekommendation

**Given flytkraft:** 5 kg

### Slutsats
En **25 kg säck betong**, gjuten till en klump, räcker med god marginal.

### Motivering
- Vindyta på bojen är liten → vindkraft ca 3–5 N även vid kraftig byvind
- Ström i insjö är svag (0,1–0,3 m/s) → liten kraft på nedsänkt del
- Betong väger ca 2000–2400 kg/m³ och tappar ~45–55 % av sin vikt i vatten
- 25 kg betong i luft → ca 13–14 kg nedsänkt vikt → 2,5–3x marginal mot flytkraften

### Praktiska rekommendationer
- Gjut betongen i en hink/spann, gjut in en ögla/bult för infästning
- Kort kätting (0,5–1 m) närmast klumpen, sedan lina upp till bojen
- **Scope (linlängd):** sikta på 6–9 m totalt (2–3x vattendjupet på 3 m)

### ⚠️ Viktig varning – is
Den verkliga risken året runt i Stockholm är **is**, inte vind/vågor. Ett rörligt isflak kan ge krafter som vida överstiger vad ett betongankare klarar av. Vanliga lösningar:
- Ta upp bojen inför islägg, **eller**
- Acceptera risk för förlust/skada under vintern

Betongankaret löser inte is-problematiken.

## Noteringar

- ESP32-C3 Super Mini används **inte** i det slutgiltiga projektet
- Sändarfrekvens 868 MHz, duty cycle max 1 % i bandet 868,0–868,6 MHz (~36 sek/timme) enligt PTS
- Basstationen sitter fast vid sjön (hos föräldrarna); servern flyttar med användaren till andra nät
