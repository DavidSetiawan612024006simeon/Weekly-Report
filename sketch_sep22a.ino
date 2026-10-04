#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// PIN I2C ESP32-S3
// =====================================================
#define SDA_PIN 8
#define SCL_PIN 9

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// SENSOR
// =====================================================
Adafruit_AHTX0 aht;
Adafruit_SGP30 sgp;

// =====================================================
// STATUS SENSOR
// =====================================================
bool oledOK = false;
bool ahtOK  = false;
bool sgpOK  = false;

// =====================================================
// TIMER RETRY
// =====================================================
unsigned long lastRetry = 0;

const unsigned long RETRY_INTERVAL = 2000; // 2 detik


// =====================================================
// FUNGSI: COBA DETEKSI OLED
// =====================================================
void checkOLED() {

  if (oledOK) {
    return;
  }

  Serial.println("[OLED] Mencoba mendeteksi...");

  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {

    oledOK = true;

    Serial.println("[OLED] BERHASIL ditemukan.");

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("Indoor Air Quality");

    display.setCursor(0, 16);
    display.println("OLED detected");

    display.display();

  } else {

    Serial.println("[OLED] Tidak ditemukan.");
  }
}


// =====================================================
// FUNGSI: COBA DETEKSI AHT20
// =====================================================
void checkAHT20() {

  if (ahtOK) {
    return;
  }

  Serial.println("[AHT20] Mencoba mendeteksi...");

  if (aht.begin(&Wire)) {

    ahtOK = true;

    Serial.println("[AHT20] BERHASIL ditemukan.");

  } else {

    Serial.println("[AHT20] Tidak ditemukan.");
  }
}


// =====================================================
// FUNGSI: COBA DETEKSI SGP30
// =====================================================
void checkSGP30() {

  if (sgpOK) {
    return;
  }

  Serial.println("[SGP30] Mencoba mendeteksi...");

  if (sgp.begin(&Wire)) {

    sgpOK = true;

    Serial.println("[SGP30] BERHASIL ditemukan.");

  } else {

    Serial.println("[SGP30] Tidak ditemukan.");
  }
}


// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("==========================================");
  Serial.println(" INDOOR AIR QUALITY SYSTEM");
  Serial.println(" ESP32-S3 + AHT20 + SGP30 + OLED");
  Serial.println("==========================================");

  // ---------------------------------------------------
  // MULAI I2C
  // ---------------------------------------------------

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println();
  Serial.println("I2C dimulai.");
  Serial.println("SDA = GPIO8");
  Serial.println("SCL = GPIO9");

  Serial.println();
  Serial.println("Memulai pemeriksaan sensor...");
  Serial.println();

  // ---------------------------------------------------
  // COBA SEMUA SENSOR
  // ---------------------------------------------------

  checkOLED();
  checkAHT20();
  checkSGP30();

  Serial.println();
  Serial.println("==========================================");
  Serial.println(" STATUS AWAL");
  Serial.println("==========================================");

  Serial.print("OLED  : ");
  Serial.println(oledOK ? "OK" : "TIDAK TERDETEKSI");

  Serial.print("AHT20 : ");
  Serial.println(ahtOK ? "OK" : "TIDAK TERDETEKSI");

  Serial.print("SGP30 : ");
  Serial.println(sgpOK ? "OK" : "TIDAK TERDETEKSI");

  Serial.println("==========================================");
  Serial.println();

  lastRetry = millis();
}


// =====================================================
// LOOP
// =====================================================
void loop() {

  // ===================================================
  // 1. JIKA ADA SENSOR YANG BELUM TERDETEKSI
  //    MAKA COBA LAGI SETIAP 2 DETIK
  // ===================================================

  if (millis() - lastRetry >= RETRY_INTERVAL) {

    lastRetry = millis();

    if (!oledOK) {
      checkOLED();
    }

    if (!ahtOK) {
      checkAHT20();
    }

    if (!sgpOK) {
      checkSGP30();
    }

    Serial.println();
  }


  // ===================================================
  // 2. BACA AHT20
  // ===================================================

  sensors_event_t humidity;
  sensors_event_t temperature;

  if (ahtOK) {

    aht.getEvent(&humidity, &temperature);

  } else {

    // Nilai default jika AHT20 belum tersedia
    temperature.temperature = 0;
    humidity.relative_humidity = 0;
  }


  // ===================================================
  // 3. BACA SGP30
  // ===================================================

  bool sgpReadOK = false;

  if (sgpOK) {

    sgpReadOK = sgp.IAQmeasure();

    // Jika pembacaan gagal
    if (!sgpReadOK) {

      Serial.println("[SGP30] Pembacaan gagal.");

      // Tandai sebagai tidak aktif
      // sehingga akan dicoba lagi oleh sistem
      sgpOK = false;
    }
  }


  // ===================================================
  // 4. SERIAL MONITOR
  // ===================================================

  Serial.println("------------------------------------------");

  // AHT20
  if (ahtOK) {

    Serial.print("Temperature : ");
    Serial.print(temperature.temperature, 2);
    Serial.println(" C");

    Serial.print("Humidity    : ");
    Serial.print(humidity.relative_humidity, 2);
    Serial.println(" %");

  } else {

    Serial.println("Temperature : SENSOR ERROR");
    Serial.println("Humidity    : SENSOR ERROR");
  }


  // SGP30
  if (sgpReadOK) {

    Serial.print("TVOC        : ");
    Serial.print(sgp.TVOC);
    Serial.println(" ppb");

    Serial.print("eCO2        : ");
    Serial.print(sgp.eCO2);
    Serial.println(" ppm");

  } else {

    Serial.println("TVOC        : SENSOR ERROR");
    Serial.println("eCO2        : SENSOR ERROR");
  }


  // Status
  Serial.println();

  Serial.print("STATUS | OLED: ");
  Serial.print(oledOK ? "OK" : "ERROR");

  Serial.print(" | AHT20: ");
  Serial.print(ahtOK ? "OK" : "ERROR");

  Serial.print(" | SGP30: ");
  Serial.println(sgpOK ? "OK" : "ERROR");

  Serial.println("------------------------------------------");


  // ===================================================
  // 5. OLED
  // ===================================================

  if (oledOK) {

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    // -----------------------------------------------
    // Judul
    // -----------------------------------------------

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("Indoor Air Quality");

    display.drawLine(
      0, 10,
      127, 10,
      SSD1306_WHITE
    );


    // -----------------------------------------------
    // Temperature
    // -----------------------------------------------

    display.setCursor(0, 15);

    if (ahtOK) {

      display.print("Temp : ");
      display.print(temperature.temperature, 1);
      display.println(" C");

    } else {

      display.println("Temp : ERROR");
    }


    // -----------------------------------------------
    // Humidity
    // -----------------------------------------------

    display.setCursor(0, 27);

    if (ahtOK) {

      display.print("RH   : ");
      display.print(humidity.relative_humidity, 1);
      display.println(" %");

    } else {

      display.println("RH   : ERROR");
    }


    // -----------------------------------------------
    // TVOC
    // -----------------------------------------------

    display.setCursor(0, 39);

    if (sgpReadOK) {

      display.print("TVOC : ");
      display.print(sgp.TVOC);
      display.println(" ppb");

    } else {

      display.println("TVOC : ERROR");
    }


    // -----------------------------------------------
    // eCO2
    // -----------------------------------------------

    display.setCursor(0, 51);

    if (sgpReadOK) {

      display.print("eCO2 : ");
      display.print(sgp.eCO2);
      display.println(" ppm");

    } else {

      display.println("eCO2 : ERROR");
    }


    display.display();
  }


  // ===================================================
  // 6. SAMPLING 1 DETIK
  // ===================================================

  delay(1000);
}