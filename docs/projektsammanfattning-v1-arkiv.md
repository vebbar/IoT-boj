# IoT-boj – Projektsammanfattning

*Sammanställd 2026-08-14 utifrån projektunderlag och diskussion.*

## Syfte
En temperaturboj som mäter vattentemperatur på flera djup och skickar data trådlöst till en basstation.

## Krav
- Mäter vattentemperatur på minst tre nivåer: **-0,2 m, -1 m, -2 m**
- Ev. lufttemperatur
- Placeras i **allmänt vatten** → ska vara **hyfsat billig** (risk för vandalisering/stöld)
- Ligger ute **året runt i Stockholm** → måste tåla kyla
- Höljet **3D-printas** (Bambu Lab P1S)
- Strömförsörjs via **solcell** som laddar batteri
- Kommunicerar via LoRa ca **100 m** till basstationen, genom vegetation ("foliage")
- Basstation: Raspberry Pi Zero W2 (finns redan), sitter kvar vid sjön hos föräldrarna
- Server flyttar med användaren till annat nät → kommunikation sker **Pi → server (push)**, aldrig tvärtom

## Fysiska mått (boj)
- Ungefär rund
- Omkrets ca **200 mm**
- Höjd/djup ca **150 mm**
- 3D-printas på Bambu Lab P1S

## Plats/miljö
- Insjö
- Vattendjup: **3 m**
- Vindutsatt läge

## Inköpslista – elektronik/IoT

### Styrenhet & radio
- ESP32 med inbyggt LoRa (SX1262), 868 MHz — motsvarande YourCee-modulen
- 3x DS18B20 vattentät temperatursensor (för -0,2, -1, -2 m djup)

### Kraftsystem
- 2–3x 18650 Li-ion-celler, gärna "protected"
- Batterihållare för 2–3 celler (parallellkoppling)
- Solar charge controller för 18650, t.ex. CN3065-baserad modul eller Adafruit/DFRobot Solar Lipo Charger
- Solpanel, ca 5–6V, 1–2W (extra marginal rekommenderas pga Stockholms vinter)
- Spänningsregulator (buck/boost till stabil 3,3V), om inte redan inbyggd
- Ev. INA219-modul för batterispänning/laddström (telemetri, systemhälsa)

### Övrig elektronik
- Vattentät SMA-genomföring (om extern antenn behövs)
- Vattentäta kabelgenomföringar (cable glands) för sensorkablar
- Reed switch (valfritt, för strömbrytning)

### Basstation (Raspberry Pi Zero W2 – finns redan)
- SX1276/78 LoRa-modul (868 MHz) — samma typ som SZFYDOSH-modulen
- SPI-kablage/pinheader till Pi:ns GPIO
- Antenn till LoRa-modulen (ingår ofta i kit)
- microSD-kort (tillräcklig storlek/klass för SQLite-loggning)

### Noteringar
- ESP32-C3 Super Mini används **inte** i slutgiltiga projektet
- Sändarfrekvens 868 MHz, duty cycle max 1% i bandet 868,0–868,6 MHz (~36 sek/timme) enligt PTS
- Basstationen sitter fast vid sjön; servern flyttar med användaren

## Förankring – beräkning och rekommendation

**Given flytkraft:** 5 kg

### Slutsats
En **25 kg säck betong**, gjuten till en klump, räcker med god marginal.

### Motivering
- Vindyta på bojen är liten (Ø~200 mm, ~150 mm höjd) → vindkraft ca 3–5 N även vid kraftig byvind
- Ström i insjö är svag (0,1–0,3 m/s) → liten kraft på nedsänkt del
- Betong väger ca 2000–2400 kg/m³ och tappar ~45–55% av sin vikt i vatten
- 25 kg betong i luft → ca 13–14 kg nedsänkt vikt → 2,5–3x marginal mot flytkraften, plus marginal mot vind/våg/ström

### Praktiska rekommendationer
- Gjut betongen i en hink/spann, gjut in en ögla/bult för infästning
- Använd en kort kätting (0,5–1 m) närmast klumpen, sedan lina upp till bojen — ger horisontellt drag och minskar risk att ankaret lyfts
- **Scope (linlängd):** sikta på 6–9 m totalt (2–3x vattendjupet på 3 m)

### ⚠️ Viktig varning – is
Den verkliga risken året runt i Stockholm är **is**, inte vind/vågor. Ett rörligt isflak kan ge krafter som vida överstiger vad ett betongankare klarar av, och kan dra ner eller krossa bojen. Vanliga lösningar:
- Ta upp bojen inför islägg, **eller**
- Acceptera risk för förlust/skada under vintern

Betongankaret löser inte is-problematiken.
