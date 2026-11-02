#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// Cấu hình Wi-Fi & Firebase
#define WIFI_SSID "Hoang Ku Pu"
#define WIFI_PASSWORD "88888888@"
#define FIREBASE_HOST "bai-do-xe-thong-minh-e7f0e-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "Izdlneqg5dRPT5MCCORm1m7kZLiBPeLIqQKXnRuj"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo servoEntry; 
Servo servoExit;  

// Khai báo chân phần cứng Node 1
#define SERVO_ENTRY_PIN 12
#define SERVO_EXIT_PIN 13
#define IR_ENTRY 18
#define IR_EXIT 19

unsigned long lastLCDCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n==========================================");
  Serial.println("   ESP32 NODE 1: GATE & LCD CONTROL");
  Serial.println("==========================================");

  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);
  
  servoEntry.attach(SERVO_ENTRY_PIN);
  servoExit.attach(SERVO_EXIT_PIN);
  servoEntry.write(0); 
  servoExit.write(0);

  // Cấu hình chống kẹt I2C cho màn hình LCD
  Wire.begin(21, 22);
  Wire.setTimeOut(1000);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[OK] WiFi Connected!");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  config.timeout.serverResponse = 1000; // Chờ phản hồi Firebase tối đa 1s
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("[OK] Firebase Connected!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  delay(1000);
}

void loop() {
  // ===================================================
  // 1. KIỂM TRA LỆNH CỔNG VÀO (XÁC THỰC KÉP)
  // ===================================================
  if (Firebase.getInt(fbdo, "/BTL_SmartParking/gate/entry_cmd")) {
    if (fbdo.intData() == 1) {
      Serial.println("\n[WEB APP] Nhan lenh MO CONG VAO!");
      
      // Kiểm tra cảm biến IR cổng vào
      if (digitalRead(IR_ENTRY) == LOW) {
        Serial.println("  => XE DANG CHO: MO BARIE VÀO!");
        servoEntry.write(90); 
        delay(3000);          
        servoEntry.write(0);  
        Serial.println("  => DONG BARIE VÀO.");
      } else {
        Serial.println("  => TU CHOI: Co lenh nhung KHONG THAY XE o cong vao!");
      }
      
      // Reset lệnh trên Firebase
      Firebase.setInt(fbdo, "/BTL_SmartParking/gate/entry_cmd", 0); 
    }
  }

  // ===================================================
  // 2. KIỂM TRA LỆNH CỔNG RA (XÁC THỰC KÉP)
  // ===================================================
  if (Firebase.getInt(fbdo, "/BTL_SmartParking/gate/exit_cmd")) {
    if (fbdo.intData() == 1) {
      Serial.println("\n[WEB APP] Nhan lenh MO CONG RA!");
      
      if (digitalRead(IR_EXIT) == LOW) {
        Serial.println("  => XE DANG CHO: MO BARIE RA!");
        servoExit.write(90);  
        delay(3000);          
        servoExit.write(0);   
        Serial.println("  => DONG BARIE RA.");
      } else {
        Serial.println("  => TU CHOI: Co lenh nhung KHONG THAY XE o cong ra!");
      }
      
      // Reset lệnh trên Firebase
      Firebase.setInt(fbdo, "/BTL_SmartParking/gate/exit_cmd", 0); 
    }
  }

  // ===================================================
  // 3. CẬP NHẬT MÀN HÌNH LCD (ĐỊNH KỲ 3 GIÂY)
  // ===================================================
  if (millis() - lastLCDCheck >= 3000) {
    lastLCDCheck = millis();

    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    if (Firebase.getInt(fbdo, "/BTL_SmartParking/parking_slots/slot1")) s1 = fbdo.intData();
    if (Firebase.getInt(fbdo, "/BTL_SmartParking/parking_slots/slot2")) s2 = fbdo.intData();
    if (Firebase.getInt(fbdo, "/BTL_SmartParking/parking_slots/slot3")) s3 = fbdo.intData();
    if (Firebase.getInt(fbdo, "/BTL_SmartParking/parking_slots/slot4")) s4 = fbdo.intData();

    int totalOccupied = (s1 == 2) + (s2 == 2) + (s3 == 2) + (s4 == 2);
    int totalEmpty = 4 - totalOccupied;

    Serial.print("[SYSTEM LOOP] Slot trong: ");
    Serial.print(totalEmpty);
    Serial.println("/4");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Trong: ");
    lcd.print(totalEmpty);
    lcd.print("/4 slot");

    lcd.setCursor(0, 1);
    lcd.print(s1 == 0 ? "[O]" : "[X]");
    lcd.print(s2 == 0 ? "[O]" : "[X]");
    lcd.print(s3 == 0 ? "[O]" : "[X]");
    lcd.print(s4 == 0 ? "[O]" : "[X]");
  }
}