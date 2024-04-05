#include <WiFi.h>  // Library untuk mengakses fitur WiFi pada perangkat ESP32
#include <PubSubClient.h> // Library untuk mengimplementasikan protokol MQTT pada perangkat ESP32
#include <DHT.h> // Library untuk berkomunikasi dengan sensor DHT (Digital Humidity and Temperature)
#include <ArduinoJson.h> // Library untuk memanipulasi data dalam format JSON

const int DHT_PIN = 22;  // Pin yang digunakan untuk sensor DHT
const char* ssid = "fachrizaldanvarick"; // Nama SSID (WiFi) yang akan dihubungkan
const char* password = "sbmdapetA"; // Kata sandi WiFi
const char* mqtt_server = "192.168.210.119"; // Alamat IP server MQTT (Mosquitto)

JsonDocument doc; // Objek untuk menyimpan data JSON
DHT sensor_dht(DHT_PIN, DHT22); // Objek sensor DHT dengan pin yang telah ditentukan
WiFiClient espClient; // Objek untuk koneksi WiFi menggunakan ESP32
PubSubClient client(espClient); // Objek PubSubClient untuk komunikasi MQTT
unsigned long lastMsg = 0; // Variabel untuk menyimpan waktu terakhir data dikirim
float temp = 0; // Variabel untuk menyimpan suhu
float hum = 0; // Variabel untuk menyimpan kelembaban

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : "); 
  Serial.println(ssid); // Menampilkan SSID (WiFi) yang terhubung

  WiFi.mode(WIFI_STA); // Mengatur mode WiFi sebagai mode Station (menyambung ke jaringan WiFi)
  WiFi.begin(ssid, password); // Menghubungkan ke jaringan WiFi dengan SSID dan password yang telah ditentukan

  while (WiFi.status() != WL_CONNECTED) { // Loop sampai terhubung ke WiFi
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros()); // Inisialisasi pembangkit bilangan acak dengan nilai yang tidak pasti

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP()); // Menampilkan alamat IP lokal ESP32 setelah terkoneksi
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]); // Menampilkan pesan yang diterima dari MQTT broker
  }
}

void reconnect() { 
  while (!client.connected()) { // Loop sampai koneksi berhasil dibuat
    setup_wifi(); // Membuat koneksi WiFi
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Membuat ID klien MQTT secara acak

    if (client.connect(clientId.c_str())) { // Mencoba melakukan koneksi ke broker MQTT
      Serial.println("Connected to Aedes MQTT Broker");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); // Menampilkan kode status koneksi MQTT
      Serial.println(" try again in 5 seconds");
      delay(5000); // Delay sebelum mencoba koneksi kembali
    }
  }
}

void setup() {
  //pinMode(2, OUTPUT);  // Set pin mode, dalam komentar karena tidak digunakan
  Serial.begin(115200); // Memulai komunikasi serial dengan baud rate 115200
  sensor_dht.begin(); // Memulai sensor DHT
  setup_wifi();  // Mengatur koneksi WiFi
  client.setServer(mqtt_server, 1883); // Mengatur server MQTT dan portnya
  client.setCallback(callback); // Mengatur fungsi callback untuk menerima pesan MQTT
  }

void loop() {
  if (!client.connected()) { // Jika klien tidak terhubung dengan broker MQTT
    reconnect(); // Mencoba untuk terhubung kembali
  }
  client.loop(); // Loop PubSubClient
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) { // Jika telah lewat 2 detik sejak pengiriman terakhir
    lastMsg = now;
    
    float temp = sensor_dht.readTemperature(); // Membaca suhu dari sensor DHTS
    float hum = sensor_dht.readHumidity(); // Membaca kelembaban dari sensor DHT
    
    // Serial Dalam Bentuk JSON
    doc["Temperature"] = temp; // Menyimpan nilai suhu dalam objek JSON
    doc["Humidity"] = hum; // Menyimpan nilai kelembaban dalam objek JSON
    String doc_str = doc.as<String>(); // Mengonversi objek JSON menjadi String
    client.publish("/fachrizal_varick/json", doc_str.c_str()); // Mengirim data JSON ke topik MQTT
    serializeJson(doc, Serial); // Serialize objek JSON ke Serial
    Serial.println(); // Menampilkan Dokumen JSON di serial monitor

    // Serial Bukan dalam bentuk JSON
    char temp_str[8]; 
    dtostrf(temp, 1, 2, temp_str); // Mengonversi nilai suhu menjadi string
    client.publish("/fachrizal_varick/temp", temp_str); // Mengirim data suhu ke topik MQTT
    char hum_str[8];
    dtostrf(hum, 1, 2, hum_str); // Mengonversi nilai kelembaban menjadi string
    client.publish("/fachrizal_varick/hum", temp_str); // Mengirim data kelembaban ke topik MQTT
    Serial.print("Temperature: ");
    Serial.println(temp); // Menampilkan nilai Temperature di serial monitor
    Serial.print("Humidity: ");
    Serial.println(hum); // Menampilkan nilai Humidity di serial monitor
    }
}