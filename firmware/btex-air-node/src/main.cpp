#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_BME680.h>
#include <Adafruit_SCD30.h>
#include "Adafruit_LC709203F.h"

#define NODE_UID "bb-btex-air-001-1"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BME680 bme;
Adafruit_SCD30 scd30;
Adafruit_LC709203F battery;

bool bme_ok = false;
bool scd_ok = false;
bool battery_ok = false;

void setupTFT() {
#ifdef TFT_BACKLITE
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
#endif

#ifdef TFT_I2C_POWER
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(500);
#endif

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(8, 6);
  tft.println("BTEX Air");

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(8, 30);
  tft.println(NODE_UID);
}

void setupBattery() {
  Serial.println("Initializing LC709203F battery monitor at 0x36...");

  battery_ok = battery.begin(&Wire);

  if (!battery_ok) {
    Serial.println("Battery monitor FAIL");
    return;
  }

  Serial.println("Battery monitor OK");

  // Best available preset. Your pack is 10050 mAh, but the library uses broad profiles.
  battery.setPackSize(LC709203F_APA_3000MAH);
}

void setupBME688() {
  Serial.println("Initializing BME688 at 0x77...");

  bme_ok = bme.begin(0x77);

  if (!bme_ok) {
    Serial.println("BME688 FAIL");
    return;
  }

  Serial.println("BME688 OK");

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
}

void setupSCD30() {
  Serial.println("Initializing SCD30 at 0x61...");

  scd_ok = scd30.begin();

  if (!scd_ok) {
    Serial.println("SCD30 FAIL");
    return;
  }

  Serial.println("SCD30 OK");
  scd30.setMeasurementInterval(2);
}

void showStartupStatus() {
  tft.fillRect(0, 44, 240, 42, ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(8, 46);
  tft.setTextColor(battery_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("Battery: ");
  tft.println(battery_ok ? "OK" : "FAIL");

  tft.setCursor(8, 60);
  tft.setTextColor(bme_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("BME688:  ");
  tft.println(bme_ok ? "OK" : "FAIL");

  tft.setCursor(8, 74);
  tft.setTextColor(scd_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("SCD30:   ");
  tft.println(scd_ok ? "OK" : "FAIL");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("BTEX Air Sensor Boot");
  Serial.println(NODE_UID);

  setupTFT();

  Wire.begin();
  Wire.setClock(100000);
  delay(200);

  setupBattery();
  setupBME688();
  setupSCD30();

  showStartupStatus();
}

void loop() {
  Serial.println();
  Serial.println("--- Readings ---");

  tft.fillRect(0, 88, 240, 47, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  int y = 90;

  if (battery_ok) {
    float batt_v = battery.cellVoltage();
    float batt_pct = battery.cellPercent();

    Serial.printf("Battery Voltage: %.3f V\n", batt_v);
    Serial.printf("Battery Percent: %.1f %%\n", batt_pct);

    tft.setCursor(8, y);
    tft.printf("Batt %.2fV %.0f%%", batt_v, batt_pct);
    y += 12;
  } else {
    Serial.println("Battery monitor unavailable");
  }

  if (bme_ok && bme.performReading()) {
    float temp_c = bme.temperature;
    float rh = bme.humidity;
    float pressure_hpa = bme.pressure / 100.0;
    float gas_kohm = bme.gas_resistance / 1000.0;

    Serial.printf("BME688 Temp:     %.2f C\n", temp_c);
    Serial.printf("BME688 RH:       %.2f %%\n", rh);
    Serial.printf("BME688 Pressure: %.2f hPa\n", pressure_hpa);
    Serial.printf("BME688 Gas:      %.2f kOhm\n", gas_kohm);

    tft.setCursor(8, y);
    tft.printf("T %.1fC RH %.1f%%", temp_c, rh);
    y += 12;

    tft.setCursor(8, y);
    tft.printf("P %.1fhPa", pressure_hpa);
    y += 12;
  } else {
    Serial.println("BME688 read failed");
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(8, y);
    tft.print("BME688 read fail");
    y += 12;
    tft.setTextColor(ST77XX_WHITE);
  }

  if (scd_ok && scd30.dataReady()) {
    if (scd30.read()) {
      Serial.printf("SCD30 CO2:       %.0f ppm\n", scd30.CO2);
      Serial.printf("SCD30 Temp:      %.2f C\n", scd30.temperature);
      Serial.printf("SCD30 RH:        %.2f %%\n", scd30.relative_humidity);

      tft.setCursor(8, y);
      tft.printf("CO2 %.0f ppm", scd30.CO2);
    } else {
      Serial.println("SCD30 read failed");
      tft.setTextColor(ST77XX_RED);
      tft.setCursor(8, y);
      tft.print("SCD30 read fail");
    }
  } else {
    Serial.println("SCD30 warming / not ready");
    tft.setCursor(8, y);
    tft.print("SCD30 warming...");
  }

  delay(2000);
}