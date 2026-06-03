#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_BME680.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_MAX1704X.h>
#include <Adafruit_PM25AQI.h>

#define NODE_UID "bb-btex-air-001-1"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BME680 bme;
Adafruit_SCD30 scd30;
Adafruit_MAX17048 maxlipo;
Adafruit_PM25AQI pms;
PM25_AQI_Data pms_data;

bool bme_ok = false;
bool scd_ok = false;
bool battery_ok = false;
bool pms_ok = false;

uint8_t screen_page = 0;

void tftHeader(const char *title) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(6, 4);
  tft.print(NODE_UID);

  tft.setTextSize(2);
  tft.setCursor(6, 20);
  tft.print(title);
}

void tftLine(int y, const char *label, float value, const char *unit, uint8_t decimals = 1) {
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(6, y);
  tft.print(label);
  tft.print(" ");
  tft.print(value, decimals);
  tft.print(unit);
}

void tftLineInt(int y, const char *label, int value, const char *unit) {
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(6, y);
  tft.print(label);
  tft.print(" ");
  tft.print(value);
  tft.print(unit);
}

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
  tftHeader("BTEX Air");
}

void setupBattery() {
  Serial.println("Initializing MAX17048 battery monitor at 0x36...");
  battery_ok = maxlipo.begin();
  Serial.println(battery_ok ? "Battery monitor OK" : "Battery monitor FAIL");
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

void setupPMSA003I() {
  Serial.println("Initializing PMSA003I at 0x69...");
  pms_ok = pms.begin_I2C();
  Serial.println(pms_ok ? "PMSA003I OK" : "PMSA003I FAIL");
}

void showStartupStatus() {
  tftHeader("Startup");

  tft.setTextSize(2);

  tft.setCursor(6, 48);
  tft.setTextColor(battery_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("BAT ");
  tft.println(battery_ok ? "OK" : "FAIL");

  tft.setCursor(6, 68);
  tft.setTextColor(bme_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("BME ");
  tft.println(bme_ok ? "OK" : "FAIL");

  tft.setCursor(6, 88);
  tft.setTextColor(scd_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("SCD ");
  tft.println(scd_ok ? "OK" : "FAIL");

  tft.setCursor(6, 108);
  tft.setTextColor(pms_ok ? ST77XX_GREEN : ST77XX_RED);
  tft.print("PMS ");
  tft.println(pms_ok ? "OK" : "FAIL");

  delay(2500);
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
  setupPMSA003I();

  showStartupStatus();
}

void loop() {
  float batt_v = NAN;
  float batt_pct = NAN;
  float temp_c = NAN;
  float rh = NAN;
  float pressure_hpa = NAN;
  float gas_kohm = NAN;
  float co2 = NAN;

  bool bme_read_ok = false;
  bool scd_read_ok = false;
  bool pms_read_ok = false;

  Serial.println();
  Serial.println("--- Readings ---");

  if (battery_ok) {
    batt_v = maxlipo.cellVoltage();
    batt_pct = maxlipo.cellPercent();
    Serial.printf("Battery: %.3f V, %.1f %%\n", batt_v, batt_pct);
  }

  if (bme_ok && bme.performReading()) {
    bme_read_ok = true;
    temp_c = bme.temperature;
    rh = bme.humidity;
    pressure_hpa = bme.pressure / 100.0;
    gas_kohm = bme.gas_resistance / 1000.0;

    Serial.printf("BME Temp: %.2f C\n", temp_c);
    Serial.printf("BME RH: %.2f %%\n", rh);
    Serial.printf("BME Pressure: %.2f hPa\n", pressure_hpa);
    Serial.printf("BME Gas: %.2f kOhm\n", gas_kohm);
  } else {
    Serial.println("BME688 read failed");
  }

  if (scd_ok && scd30.dataReady() && scd30.read()) {
    scd_read_ok = true;
    co2 = scd30.CO2;

    Serial.printf("SCD CO2: %.0f ppm\n", co2);
    Serial.printf("SCD Temp: %.2f C\n", scd30.temperature);
    Serial.printf("SCD RH: %.2f %%\n", scd30.relative_humidity);
  } else {
    Serial.println("SCD30 warming / not ready");
  }

  if (pms_ok && pms.read(&pms_data)) {
    pms_read_ok = true;

    Serial.println("===== PMSA003I =====");

    Serial.printf("PM1.0 std : %u ug/m3\n", pms_data.pm10_standard);
    Serial.printf("PM2.5 std : %u ug/m3\n", pms_data.pm25_standard);
    Serial.printf("PM10  std : %u ug/m3\n", pms_data.pm100_standard);

    Serial.printf("PM1.0 env : %u ug/m3\n", pms_data.pm10_env);
    Serial.printf("PM2.5 env : %u ug/m3\n", pms_data.pm25_env);
    Serial.printf("PM10  env : %u ug/m3\n", pms_data.pm100_env);

    Serial.printf(">0.3 um   : %u count/0.1L\n", pms_data.particles_03um);
    Serial.printf(">0.5 um   : %u count/0.1L\n", pms_data.particles_05um);
    Serial.printf(">1.0 um   : %u count/0.1L\n", pms_data.particles_10um);
    Serial.printf(">2.5 um   : %u count/0.1L\n", pms_data.particles_25um);
    Serial.printf(">5.0 um   : %u count/0.1L\n", pms_data.particles_50um);
    Serial.printf(">10  um   : %u count/0.1L\n", pms_data.particles_100um);
  } else {
    Serial.println("PMSA003I read failed / not ready");
  }

  if (screen_page == 0) {
    tftHeader("Air 1/3");

    if (pms_read_ok) {
      tftLineInt(48, "PM1", pms_data.pm10_env, "");
      tftLineInt(74, "PM2.5", pms_data.pm25_env, "");
      tftLineInt(100, "PM10", pms_data.pm100_env, "");
    } else {
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_RED);
      tft.setCursor(6, 60);
      tft.print("PMS FAIL");
    }
  }

  if (screen_page == 1) {
    tftHeader("Gas 2/3");

    if (scd_read_ok) {
      tftLineInt(48, "CO2", (int)co2, "ppm");
    } else {
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_RED);
      tft.setCursor(6, 48);
      tft.print("CO2 warm");
    }

    if (bme_read_ok) {
      tftLine(78, "Gas", gas_kohm, "k", 1);
      tftLine(104, "Pres", pressure_hpa, "", 0);
    }
  }

  if (screen_page == 2) {
    tftHeader("Node 3/3");

    if (battery_ok) {
      tftLine(48, "Batt", batt_pct, "%", 0);
      tftLine(74, "Volt", batt_v, "V", 2);
    }

    if (bme_read_ok) {
      tftLine(100, "T", temp_c, "C", 1);
    }
  }

    if (screen_page == 3) {
    tftHeader("PMS std 4/5");

    if (pms_read_ok) {
      tftLineInt(48, "PM1", pms_data.pm10_standard, "");
      tftLineInt(74, "PM2.5", pms_data.pm25_standard, "");
      tftLineInt(100, "PM10", pms_data.pm100_standard, "");
    } else {
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_RED);
      tft.setCursor(6, 60);
      tft.print("PMS FAIL");
    }
  }

  if (screen_page == 4) {
    tftHeader("PMS cnt 5/5");

    if (pms_read_ok) {
      tftLineInt(42, ">0.3", pms_data.particles_03um, "");
      tftLineInt(64, ">0.5", pms_data.particles_05um, "");
      tftLineInt(86, ">1.0", pms_data.particles_10um, "");
      tftLineInt(108, ">2.5", pms_data.particles_25um, "");
    } else {
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_RED);
      tft.setCursor(6, 60);
      tft.print("PMS FAIL");
    }
  }

  screen_page = (screen_page + 1) % 5;
  delay(2500);
}