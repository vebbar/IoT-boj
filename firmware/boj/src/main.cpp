#define HELTEC_WIRELESS_STICK_LITE   // måste stå FÖRE include-raden nedan
#define HELTEC_NO_DISPLAY            // Stick Lite saknar skärm, slipper onödig init
#include <heltec_unofficial.h>       // skapar en färdig 'radio'-instans med rätt pinnar för just detta kort

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// Data-sladd för DS18B20 sensorer på GPIO 15
const int ONE_WIRE_PIN = 15;
//SDA från INA219 på GPIO 2
const int SDA_PIN = 2;
//SCL från INA219 på GPIO 7
const int SCL_PIN = 7;

Adafruit_INA219 ina219;
OneWire oneWireBus(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWireBus);

const int NUM_SENSORS = 3;

// Sensoradresser för DS18B20
// Sensor 1 (0.3m): 28CAFDC3000000B5
// Sensor 2 (1.0m): 28AE33C4000000AC
// Sensor 3 (2.0m): 2833C9C3000000CA
DeviceAddress sensorAddresses[NUM_SENSORS] = {
  {0x28, 0xCA, 0xFD, 0xC3, 0x00, 0x00, 0x00, 0xB5},
  {0x28, 0xAE, 0x33, 0xC4, 0x00, 0x00, 0x00, 0xAC},
  {0x28, 0x33, 0xC9, 0xC3, 0x00, 0x00, 0x00, 0xCA}
};

// Etiketter för utskrift, en per sensor/djup
const char* depthLabels[NUM_SENSORS] = {"0.3m", "1.0m", "2.0m"};

// Senaste kända bra värde per sensor (används om en mätning misslyckas) sparat i RTC minne
RTC_DATA_ATTR float lastGoodTemp[NUM_SENSORS] = {0.0, 0.0, 0.0};

bool ina219Available = false;  // global variabel, börjar som "inte tillgänglig"

// OBS: 'radio' (SX1262) skapas automatiskt av heltec_unofficial.h ovan,
// med korrekta pinnar för Wireless Stick Lite V3 - ingen egen Module()-instans behövs.

struct TemperatureResult {
  float averageTemperature;
  bool isSuccessful;
  float rawTemperature;
};

void getSafeTemperatures(TemperatureResult results[], int samples);

void setup() {
  heltec_setup();  // Initierar Serial, SPI (hspi->begin) och display (om aktiv)
  Serial.println("CHECKPOINT 1: heltec_setup() klar");

  heltec_ve(true); // Slår på Vext-strömskenan -- annars är externa sensorer (t.ex. INA219) strömlösa
  Serial.println("CHECKPOINT 2: heltec_ve(true) klar");

  delay(300);      // ge Vext-skenan gott om tid att stabiliseras innan sensorer pratas med
  Serial.println("CHECKPOINT 3: delay(300) klar");

  unsigned long serialWaitStart = millis(); //väntar på usb kontakt i 3 sekunder
  while (!Serial && millis() - serialWaitStart < 3000) {
    delay(10);
  }
  Serial.println("CHECKPOINT 4: serial-vantan klar");

  sensors.begin();
  Serial.println("CHECKPOINT 5: sensors.begin() klar");

  int deviceCount = sensors.getDeviceCount();
  Serial.println("CHECKPOINT 6: sensors.getDeviceCount() klar");
  Serial.print("DS18B20: hittade ");
  Serial.print(deviceCount);
  Serial.println(" sensorer.");
  if (deviceCount < NUM_SENSORS) {
    Serial.println("VARNING: färre sensorer hittades än förväntat!");
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("CHECKPOINT 7: Wire.begin() klar");

  Wire.setClock(100000);  // konservativ I2C-hastighet (100kHz) för tillförlitligare läsningar
  Serial.println("CHECKPOINT 8: Wire.setClock() klar");

  // "Väck" I2C-bussen genom att snabbt pinga alla möjliga adresser en gång.
  // Detta visade sig lösa ett problem där INA219 inte svarade tillförlitligt
  // direkt efter Vext-strömpåslag - troligen på grund av hur ESP32:ans
  // I2C-drivrutin återhämtar bussen efter flera transaktioner (inklusive NACK:ar).
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    Wire.endTransmission();
  }
  Serial.println("CHECKPOINT 9: I2C-bus-scan klar");

  bool ina219Found = false;
  for (int attempt = 1; attempt <= 5 && !ina219Found; attempt++) {
    if (ina219.begin()) {
      ina219Found = true;
    } else {
      delay(50);
    }
  }
  Serial.println("CHECKPOINT 10: ina219.begin()-forsok klara");

  if (!ina219Found) {
    Serial.println("Kunde inte hitta INA219! Fortsätter utan batterimätning denna cykel.");
    // ina219Available förblir false, ingen while(1) här
  } else {
    ina219Available = true;
    Serial.println("INA219 hittad, startar mätning...");
  }

  // Initiera LoRa (SX1262) - 'radio' kommer från heltec_unofficial.h
  Serial.println("Initierar LoRa...");
  int state = radio.begin(868.0, 125.0, 7, 5, 0x3B, 12);
  Serial.println("CHECKPOINT 11: radio.begin() klar");
  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("LoRa init misslyckades: " + String(state));
  } else {
    Serial.println("LoRa init OK.");
  }

  // Mät alla sensorer
  TemperatureResult results[NUM_SENSORS];
  getSafeTemperatures(results, 3);
  Serial.println("CHECKPOINT 12: getSafeTemperatures() klar");

  // Bygg payload (textformat)
  String payload = "";
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (results[i].isSuccessful) {
      lastGoodTemp[i] = results[i].averageTemperature;
    }

    payload += "T_";
    payload += depthLabels[i];
    payload += ":";
    payload += String(lastGoodTemp[i], 1);
    payload += ",S";
    payload += String(i);
    payload += ":";
    payload += results[i].isSuccessful ? "1" : "0";

    if (i < NUM_SENSORS - 1) {
      payload += ",";
    }
  }

  if (ina219Available) {
    float voltage = ina219.getBusVoltage_V() + (ina219.getShuntVoltage_mV() / 1000);
    float current = ina219.getCurrent_mA();
    float power = ina219.getPower_mW();
    payload += ",Vbat:";
    payload += String(voltage, 2);
    payload += "V,I:";
    payload += String(current, 0);
    payload += "mA,P:";
    payload += String(power, 0);
    payload += "mW";
  } else {
    payload += ",Vbat:OFFLINE";
  }
  Serial.println("CHECKPOINT 13: payload byggd");

  // Skriv till Serial för felsökning
  Serial.println("Payload: " + payload);

  // Sänd via LoRa
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Sänder via LoRa...");
    int txState = radio.transmit(payload);
    Serial.println("CHECKPOINT 14: radio.transmit() klar");
    if (txState != RADIOLIB_ERR_NONE) {
      Serial.println("LoRa sändning misslyckades: " + String(txState));
    } else {
      Serial.println("LoRa sändning klar.");
    }
  }

  // Gå i deep sleep i 1 timme
  Serial.println("Går i deep sleep i 1 timme...");
  heltec_ve(false);  // Stäng av Vext -- annars drar pull-up-resistorn ström i onödan under hela sömnen
  delay(100); // ge tid för Serial-att avsluta
  heltec_deep_sleep(3600);  // Sätter radio i sleep, sätter timer wakeup + esp_deep_sleep_start()
}

void loop() {
  // Tom - inget händer här efter deep sleep-setup
}

/**
 * @brief Mäter temperaturen på alla sensorer samtidigt, flera gånger,
 *        och beräknar medelvärdet av de godkända mätningarna per sensor.
 *
 * @param results Array (storlek NUM_SENSORS) dit resultaten skrivs.
 * @param samples Antal mätomgångar som ska göras.
 */
void getSafeTemperatures(TemperatureResult results[], int samples) {
  float sumOfGoodTemps[NUM_SENSORS] = {0};
  int countOfGoodTemps[NUM_SENSORS] = {0};
  float lastObservedTemp[NUM_SENSORS] = {0};

  for (int i = 0; i < samples; i++) {
    sensors.requestTemperatures();  // EN broadcast per sample, alla sensorer samtidigt

    for (int s = 0; s < NUM_SENSORS; s++) {
      float currentTemp = sensors.getTempC(sensorAddresses[s]);
      lastObservedTemp[s] = currentTemp;

      // Kontrollera om värdet är inom det godkända intervallet
      if (currentTemp > -30.0 && currentTemp < 40.0) {
        sumOfGoodTemps[s] += currentTemp;
        countOfGoodTemps[s]++;
      }
    }
  }

  for (int s = 0; s < NUM_SENSORS; s++) {
    if (countOfGoodTemps[s] > 0) {
      results[s].averageTemperature = sumOfGoodTemps[s] / countOfGoodTemps[s];
      results[s].isSuccessful = true;
      results[s].rawTemperature = lastObservedTemp[s];
    } else {
      results[s].averageTemperature = 0.0;
      results[s].isSuccessful = false;
      results[s].rawTemperature = lastObservedTemp[s];
    }
  }
}
