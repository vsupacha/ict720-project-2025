#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <M5Stack.h>

// 
void show_qr_code(String name) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println(name);
  uint16_t sz = 160;
  M5.Lcd.qrcode(name, (320-sz)/2, (240-sz)/2, sz, 1);
}

void setup() {
  // init HW
  M5.begin();
  
  // init BLE
  BLEDevice::init("Asset-0");
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  show_qr_code("Asset-0");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    // change broadcast name
    BLEDevice::stopAdvertising();
    esp_ble_gap_set_device_name("Asset-1");
    BLEDevice::startAdvertising(); 
    show_qr_code("Asset-1");
  }
  if (M5.BtnB.wasPressed()) {
    // change broadcast name
    BLEDevice::stopAdvertising();
    esp_ble_gap_set_device_name("Asset-2");
    BLEDevice::startAdvertising(); 
    show_qr_code("Asset-2");
  }
  if (M5.BtnC.wasPressed()) {
    // change broadcast name
    BLEDevice::stopAdvertising();
    esp_ble_gap_set_device_name("Asset-3");
    BLEDevice::startAdvertising(); 
    show_qr_code("Asset-3");
  }
  delay(100);
}


