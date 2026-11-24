// Pengukur Intensitas Cahaya BH1750 — ESP32 + BH1750 + LCD I2C
//
// Alat ukur intensitas cahaya (lux) untuk satu ruangan. Sensor BH1750
// mengukur cahaya lewat I2C, ESP32 membacanya tiap 1 detik, dan hasilnya
// tampil di LCD 16x2: angka lux di baris pertama, penilaian cahaya di
// baris kedua. Lampu LED menyala bila cahaya di bawah ambang gelap,
// sebagai tanda ruangan terlalu redup untuk membaca.
//
// Wiring:
//   BH1750 VCC  -> 3V3 ESP32        BH1750 GND -> GND
//   BH1750 SDA  -> GPIO21           BH1750 SCL -> GPIO22
//   LCD SDA     -> GPIO21 (bus I2C yang sama, alamat 0x27)
//   LCD SCL     -> GPIO22
//   LCD VCC     -> 5V               LCD GND    -> GND
//   LED anoda   -> GPIO25 lewat resistor 220 ohm, katoda -> GND
//
// Library yang dibutuhkan:
//   BH1750 1.3.0 (Christopher Laws) + LiquidCrystal_I2C 1.1.2
//
// Terverifikasi compile: ESP32 core 4.0.0, arduino-cli 1.6.0.

#include <Wire.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>

// ---------- Konfigurasi pin ----------
const uint8_t PIN_SDA = 21;      // Bus I2C: data (BH1750 + LCD)
const uint8_t PIN_SCL = 22;      // Bus I2C: clock (BH1750 + LCD)
const uint8_t PIN_LED_GELAP = 25; // LED penanda ruangan terlalu gelap

// ---------- Alamat I2C ----------
const uint8_t ALAMAT_LCD = 0x27;    // Alamat backpack LCD 16x2
const uint8_t ALAMAT_BH1750 = 0x23; // Alamat BH1750 (pin ADDR ke GND)

// ---------- Parameter pengukuran ----------
const unsigned long INTERVAL_BACA_MS = 1000; // Baca sensor tiap 1 detik
const float AMBANG_GELAP_LUX = 100.0;  // Di bawah ini: terlalu gelap
const float AMBANG_NYAMAN_LUX = 500.0; // 100-500 lux: nyaman dibaca
const uint32_t BAUD_SERIAL = 115200;

// ---------- Objek perangkat ----------
BH1750 sensorCahaya;
LiquidCrystal_I2C lcd(ALAMAT_LCD, 16, 2);

// ---------- State ----------
unsigned long waktuBacaTerakhirMs = 0;
float luxTerakhir = 0.0;

void setup() {
  Serial.begin(BAUD_SERIAL);
  pinMode(PIN_LED_GELAP, OUTPUT);
  digitalWrite(PIN_LED_GELAP, LOW);

  Wire.begin(PIN_SDA, PIN_SCL);

  // BH1750: ukur terus-menerus, resolusi tinggi (0,5 lux, ~120 ms).
  if (!sensorCahaya.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,
                           ALAMAT_BH1750)) {
    Serial.println("BH1750 tidak ditemukan di bus I2C! Periksa wiring.");
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("BH1750 tidak ada");
    lcd.setCursor(0, 1);
    lcd.print("Cek wiring I2C!");
    while (true) {
      delay(1000); // Berhenti: tanpa sensor, angka lux tidak ada artinya.
    }
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Pengukur Cahaya");
  lcd.setCursor(0, 1);
  lcd.print("BH1750 siap...");
  delay(1500);
  lcd.clear();

  Serial.println("Pengukur cahaya BH1750 siap.");
}

// Baca intensitas cahaya dari BH1750 (satuan lux).
float bacaCahayaLux() {
  return sensorCahaya.readLightLevel();
}

// Nilai cahaya ke penilaian yang mudah dibaca manusia.
// Acuan umum: membaca nyaman di 300-500 lux; di bawah 100 lux terlalu redup.
const char *klasifikasiCahaya(float lux) {
  if (lux < AMBANG_GELAP_LUX) {
    return "Terlalu gelap!";
  }
  if (lux < AMBANG_NYAMAN_LUX) {
    return "Nyaman dibaca";
  }
  return "Sangat terang";
}

// Tampilkan angka lux + penilaian di LCD 16x2.
void tampilkanLcd(float lux) {
  lcd.setCursor(0, 0);
  lcd.print("Cahaya: ");
  if (lux < 1000.0) {
    lcd.print(lux, 1); // Satu angka di belakang koma di bawah 1000 lux.
  } else {
    lcd.print((int)lux); // Di atas 1000 lux, bilangan bulat cukup.
  }
  lcd.print(" lx   ");

  lcd.setCursor(0, 1);
  lcd.print(klasifikasiCahaya(lux));
  lcd.print("            ");
}

// LED menyala bila ruangan terlalu gelap (di bawah ambang).
void updateLedGelap(float lux) {
  digitalWrite(PIN_LED_GELAP, lux < AMBANG_GELAP_LUX ? HIGH : LOW);
}

// Cetak hasil ukur ke Serial Monitor untuk pencatatan.
void cetakSerial(float lux) {
  Serial.print("Cahaya: ");
  Serial.print(lux, 1);
  Serial.print(" lx - ");
  Serial.println(klasifikasiCahaya(lux));
}

void loop() {
  unsigned long sekarang = millis();
  if (sekarang - waktuBacaTerakhirMs < INTERVAL_BACA_MS) {
    return; // Belum waktunya baca lagi; loop tetap responsif.
  }
  waktuBacaTerakhirMs = sekarang;

  luxTerakhir = bacaCahayaLux();
  tampilkanLcd(luxTerakhir);
  updateLedGelap(luxTerakhir);
  cetakSerial(luxTerakhir);
}
