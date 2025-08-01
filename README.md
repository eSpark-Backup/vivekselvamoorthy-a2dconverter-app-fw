
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

## BLE COMMANDS: structure and usage

Hex Code | Command | Type | Input | Response Format | Description
-- | -- | -- | -- | -- | --

0x0D | Set Presence Detection Duration | Set | 0x0D | 0x0D [DURATION_VALUE] | Sets the duration after which module returns no detection. Value Range: 0x00-0xFF
0x0E | Get Presence Detection Range | Get | 0x0E | 0x0E [GATE_RESOLUTION] [NUMBER_OF_ACTIVATE_GATES] [SENSOR_RANGE] | Gets the gate resolution ,number of active gates, and the total sensor range


---

## Connection Behavior

- **Peripheral Role:** Advertises service on startup
- **MTU Size:** 512 bytes (negotiable)
- **Preferred Connection Interval:** 7.5–20 ms

---

## Example Workflow

1. **Scan for device** named `ESP{MAC_ADDRESS}`.
2. **Connect** and discover services.
3. **Enable notifications** .
  
