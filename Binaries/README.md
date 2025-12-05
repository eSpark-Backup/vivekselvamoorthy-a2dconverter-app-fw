# ESP32 Flashing Guide (Flash Download Tool)

## Files Included
- **bootloader.bin**
- **partition-table.bin**
- **Firmware_idf.bin**
- **flash_addrs.json** (address reference)

---

## Requirements
1. **ESP32 Flash Download Tool** (from Espressif)
   - Download: [https://www.espressif.com/en/support/download/other-tools](https://www.espressif.com/en/support/download/other-tools)
2. USB cable and drivers installed (CP210x or CH340)
3. Identify your COM port (e.g., COM3 / COM4)

---

## Flash Settings

| File | Address |
|------|----------|
| bootloader.bin | 0x1000 |
| partition-table.bin | 0x8000 |
| Firmware_idf.bin | 0x10000 |

Baud rate: **460800** (or **115200** if unstable)  
SPI Mode: **DIO**  
Flash Size: **4MB**  
Crystal: **40MHz**

---

## Steps

1. Open **ESP32 Download Tool**.
2. Select chip type → **ESP32**.
3. Check **SPI Download** mode.
4. Add files and their addresses as shown above.
5. Select correct **COM port** and **baud rate**.
6. Hold **BOOT** button on the ESP32 and click **START**.
7. Wait until “FINISH” appears — flashing complete!
8. Press **EN (RESET)** on the ESP32 to reboot.

---

## Notes
- If flashing fails → try lower baud (115200)  
- Always use matching binaries (bootloader, partitions, firmware)  
- Addresses can be verified in `flasher_args.json`

**Your ESP32 is now programmed successfully!**

# 📱 BLE Services & Characteristics – Documentation for Mobile App Developers

## Overview
This document provides the Bluetooth Low Energy (BLE) **Service** and **Characteristic UUIDs**, descriptions, data formats, and read/notify permissions to help mobile app developers integrate with the device over BLE.

---
## Primary Services


- **Service Name:**  GATTS_CHAR_UUID (custom)
- **Service UUID:** `0x00FF`

---

## Characteristic: 

### Transmit

- **UUID:** `0xFF01`
- **Descriptor_UUID:** `0x2902`
- **Properties:** `READ and Notify`
- **Data Format:**
  - Data formate is JSON and it look like "{"VOLT_SIG":1023,"SWITCH":1,"TACH":3500,...}"
  - Enable notifications on the characteristic
---

---

## Connection Behavior

- **Peripheral Role:** Advertises service on startup
- **MTU Size:** 512 bytes (dynamic)
- **Preferred Connection Interval:** 7.5–20 ms

---

## Example Workflow

1. **Scan for device** named `ESP_{MAC_ADDRESS}`.
2. **Connect** and discover services.
3. **Enable notifications** .
  
