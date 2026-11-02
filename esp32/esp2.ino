#include <WiFi.h>
#include <FirebaseESP32.h>

// Cấu hình Wi-Fi & Firebase
#define WIFI_SSID "Bao Cao"
#define WIFI_PASSWORD "88888888@"
#define FIREBASE_HOST "bai-do-xe-thong-minh-e7f0e-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "Izdlneqg5dRPT5MCCORm1m7kZLiBPeLIqQKXnRuj"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Khai báo chân cảm biến IR 4 ô đỗ
#define IR_SLOT1 25
#define IR_SLOT2 26
#define IR_SLOT3 32
#define IR_SLOT4 33

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("\n==========================================");
  Serial.println("   ESP32 NODE 2: ZONE MONITORING SYSTEM   ");
  Serial.println("==========================================");

  pinMode(IR_SLOT1, INPUT);
  pinMode(IR_SLOT2, INPUT);
  pinMode(IR_SLOT3, INPUT);
  pinMode(IR_SLOT4, INPUT);

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[OK] WiFi Connected!");

  // Cấu hình Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("[OK] Da ket noi Firebase Realtime Database!");
  Serial.println("------------------------------------------\n");
}

void loop() {
  // 1. Đọc trạng thái cảm biến (LOW: Có xe = 1, HIGH: Trống = 0)
  int s1 = (digitalRead(IR_SLOT1) == LOW) ? 1 : 0;
  int s2 = (digitalRead(IR_SLOT2) == LOW) ? 1 : 0;
  int s3 = (digitalRead(IR_SLOT3) == LOW) ? 1 : 0;
  int s4 = (digitalRead(IR_SLOT4) == LOW) ? 1 : 0;

  // 2. Tính toán tổng số chỗ đỗ (dùng giá trị 1)
  int totalOccupied = (s1 == 1) + (s2 == 1) + (s3 == 1) + (s4 == 1);
  int totalEmpty = 4 - totalOccupied;

  // 3. Hiển thị trạng thái ra Serial Monitor
  String statusS1 = (s1 == 0) ? "[O: Trong]" : "[X: Co Xe]";
  String statusS2 = (s2 == 0) ? "[O: Trong]" : "[X: Co Xe]";
  String statusS3 = (s3 == 0) ? "[O: Trong]" : "[X: Co Xe]";
  String statusS4 = (s4 == 0) ? "[O: Trong]" : "[X: Co Xe]";

  Serial.println("==========================================");
  Serial.println("     [CAP NHAT TRANG THAI BAI DO XE]      ");
  Serial.print("  - Slot 1 (D25): "); Serial.println(statusS1);
  Serial.print("  - Slot 2 (D26): "); Serial.println(statusS2);
  Serial.print("  - Slot 3 (D32): "); Serial.println(statusS3);
  Serial.print("  - Slot 4 (D33): "); Serial.println(statusS4);
  Serial.println("------------------------------------------");
  Serial.print("  => Tong so xe dang do: "); Serial.print(totalOccupied); Serial.println("/4");
  Serial.print("  => So cho con trong   : "); Serial.print(totalEmpty); Serial.println("/4");

  // 4. Đồng bộ lên Firebase
  bool firebaseSuccess = false;
  if (Firebase.setInt(fbdo, "/BTL_SmartParking/parking_slots/slot1", s1)) {
    Firebase.setInt(fbdo, "/BTL_SmartParking/parking_slots/slot2", s2);
    Firebase.setInt(fbdo, "/BTL_SmartParking/parking_slots/slot3", s3);
    Firebase.setInt(fbdo, "/BTL_SmartParking/parking_slots/slot4", s4);
    firebaseSuccess = true;
  }

  if (firebaseSuccess) {
    Serial.println("  [Sync] Firebase: THANH CONG");
  } else {
    Serial.print("  [Sync Error]: "); Serial.println(fbdo.errorReason());
  }
  Serial.println("==========================================\n");

  delay(1500); // Đọc định kỳ mỗi 1.5 giây
}
