#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#if defined(TFT_I2C_POWER)
  #define HAS_TFT_I2C_POWER
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("BTEX Air Node Boot");

#ifdef TFT_BACKLITE
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
#endif

#ifdef HAS_TFT_I2C_POWER
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
#endif

  tft.init(135, 240);
  tft.setRotation(3);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);

  tft.setCursor(10, 20);
  tft.println("BTEX Air");

  tft.setCursor(10, 50);
  tft.println("Sensor");

  tft.setCursor(10, 80);
  tft.println("Boot OK");

  Serial.println("TFT initialized");
}

void loop() {
  static uint32_t counter = 0;

  Serial.printf("alive %lu\n", counter);

  tft.fillRect(10, 110, 220, 20, ST77XX_BLACK);
  tft.setCursor(10, 110);
  tft.print("Count: ");
  tft.print(counter++);

  delay(1000);
}