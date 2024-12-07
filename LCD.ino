#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD 핀 설정 (I2C 주소와 크기 설정)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소가 0x27이고, 16x2 LCD인 경우

void setup() {
    Serial.begin(9600);   // 시리얼 통신 시작
    lcd.begin(16, 2);     // LCD 초기화 (16열, 2행)
    lcd.backlight();      // 백라이트 켜기
}

void loop() {
    if (Serial.available() > 0) {
        String message = Serial.readStringUntil('\n');  // 메시지 읽기
        lcd.clear();  // LCD 내용 지우기
        lcd.print(message);  // 수신한 메시지를 LCD에 표시
    }
}
