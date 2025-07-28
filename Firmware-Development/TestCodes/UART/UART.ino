/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Arduino.h"
#include "ADS1115-Driver.h"

ADS1115 ads1115 = ADS1115(ADS1115_I2C_ADDR_SCL);


BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
uint16_t value0 = 0;
uint16_t value1 = 0;
uint16_t value2 = 0;
uint16_t value3 = 0;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
uint16_t readAdsValue(uint8_t input);


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }

      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup() {
  
  Serial.begin(115200);
  
  ///==========================================
  
  // pinMode(21, INPUT_PULLUP);
  // pinMode(22, INPUT_PULLUP);
  pinMode(33, OUTPUT);

  digitalWrite(33, HIGH);
	ads1115.reset();
	ads1115.setDeviceMode(ADS1115_MODE_SINGLE);
	ads1115.setDataRate(ADS1115_DR_250_SPS);
	ads1115.setPga(ADS1115_PGA_6_144);

  ///==========================================

  // Create the BLE Device
  BLEDevice::init("Vivek-a2D-converter");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
char buffer[12];
void loop() {

  value0 = readAdsValue(ADS1115_MUX_AIN0_GND);
  // float vol = value0 / 279.0; // scale it properly
  value1 = readAdsValue(ADS1115_MUX_AIN1_GND);
  // value2 = readAdsValue(ADS1115_MUX_AIN2_GND);
  // value3 = readAdsValue(ADS1115_MUX_AIN3_GND);
  printf("val AN0: %d AN1: %d \n",value0,value1);
  // printf("val AN0: %d AN1: %d AN2: %d and AN3: %d\n",value0,value1,value2,value3);

  if (deviceConnected) {
    snprintf(buffer, sizeof(buffer), "%.2f", vol);  // Use %u for uint16_t
    // Serial.printf("Buffer: %.2f\n", vol);
    pTxCharacteristic->setValue((uint8_t*)buffer, strlen(buffer));
    pTxCharacteristic->notify();
    // txValue++;
    delay(10);  // bluetooth stack will go into congestion, if too many packets are sent
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}


uint16_t readAdsValue(uint8_t input) {
	ads1115.setMultiplexer(input);
	ads1115.startSingleConvertion();

	delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 uS to do that

	while (ads1115.getOperationalStatus() == 0);

	return ads1115.readConvertedValue();
}