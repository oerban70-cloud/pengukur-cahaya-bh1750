# Pengukur Intensitas Cahaya BH1750 — ESP32 + BH1750 + LCD I2C

Kamar terasa gelap, tapi lampumu sudah paling terang? Alat kecil ini
menjawabnya dengan angka, bukan kira-kira: sensor BH1750 mengukur
intensitas cahaya ruangan, ESP32 membacanya tiap 1 detik, dan hasilnya
tampil di LCD 16x2 — angka lux di baris pertama, penilaian cahayanya di
baris kedua. Begitu cahaya jatuh di bawah 100 lux, LED menyala sebagai
tanda ruangan terlalu redup untuk membaca.

Video penjelasan (60 detik):
link menyusul setelah upload

## Komponen

| Bagian | Keterangan |
|---|---|
| ESP32 DevKit | Otak alat |
| Modul BH1750 / GY-302 (alamat I2C 0x23) | Sensor cahaya digital, rentang 1–65535 lux, resolusi 0,5 lux |
| LCD 16x2 I2C (alamat 0x27) | Menampilkan angka lux + penilaian cahaya |
| LED kuning + resistor 220 ohm | Menyala saat cahaya di bawah 100 lux (ruangan terlalu gelap) |
| Catu USB 5V | Adaptor atau port laptop, untuk ESP32 + LCD |

## Wiring

| Dari | Ke |
|---|---|
| BH1750 VCC | ESP32 3V3 |
| BH1750 GND | GND |
| BH1750 SDA | ESP32 GPIO21 |
| BH1750 SCL | ESP32 GPIO22 |
| BH1750 ADDR | GND (alamat I2C menjadi 0x23) |
| LCD SDA | ESP32 GPIO21 (bus I2C yang sama dengan BH1750) |
| LCD SCL | ESP32 GPIO22 (bus I2C yang sama dengan BH1750) |
| LCD VCC | 5V |
| LCD GND | GND |
| LED anoda | ESP32 GPIO25, lewat resistor 220 ohm ke GND |
| USB 5V | ESP32 VIN (lewat konektor USB board) |
| Semua GND | Digabung |

Skema lengkap: [schematic.png](schematic.png)

## Cara kerja

BH1750 adalah sensor cahaya digital: di dalamnya ada fotodioda dan ADC
16-bit yang mengubah cahaya menjadi angka lux langsung, tanpa perlu
kalibrasi pembagi tegangan seperti LDR analog. Sketch memakai mode
pengukuran terus-menerus resolusi tinggi (CONTINUOUS_HIGH_RES_MODE):
tiap pengukuran selesai dalam sekitar 120 ms dengan resolusi 0,5 lux.

Setiap 1 detik, ESP32 membaca nilai lux dari BH1750 lewat I2C dan
menampilkannya di LCD. Baris pertama LCD menunjukkan angka lux — di
bawah 1000 lux dengan satu angka di belakang koma, di atas itu bilangan
bulat. Baris kedua menunjukkan penilaiannya: di bawah 100 lux
"Terlalu gelap!", 100–500 lux "Nyaman dibaca", di atas 500 lux
"Sangat terang". Acuan ini mengikuti anjuran umum untuk membaca, yaitu
300–500 lux. Bila cahaya di bawah 100 lux, LED kuning menyala dan
tetap menyala sampai cahaya kembali di atas ambang. Semua angka juga
dicetak ke Serial Monitor (115200 baud) tiap 1 detik.

BH1750 dan LCD berbagi satu bus I2C di GPIO21 (SDA) dan GPIO22 (SCL)
dengan alamat berbeda — 0x23 untuk BH1750 (pin ADDR ke GND) dan 0x27
untuk LCD — jadi tidak ada konflik. Kalau pin ADDR BH1750 disambung ke
VCC, alamatnya berubah menjadi 0x5C dan sketch harus diubah.

## Angka compile terverifikasi

- Board: ESP32 DevKit (esp32:esp32:esp32), ESP32 core 4.0.0
- Sketch: 317.809 byte (24%) dari program storage
- RAM global: 24.640 byte (7%) dari dynamic memory
- Library: BH1750 1.3.0 (Christopher Laws), LiquidCrystal_I2C 1.1.2
- Perintah verifikasi: `arduino-cli compile --fqbn esp32:esp32:esp32 PengukurCahayaBH1750.ino`

## Konfigurasi

Ubah konstanta di bagian atas sketch bila perlu:

- `AMBANG_GELAP_LUX` (100,0): di bawah nilai ini LED menyala.
- `AMBANG_NYAMAN_LUX` (500,0): batas atas zona "Nyaman dibaca".
- `INTERVAL_BACA_MS` (1000): jeda antar pembacaan sensor.
- `ALAMAT_BH1750` (0x23): ganti ke 0x5C bila pin ADDR ke VCC.
- `PIN_LED_GELAP` (GPIO25): pin LED penanda ruangan gelap.
