# FlameWorks ESP32-C6 Complete Setup Guide

## Overview
This guide will help you set up the complete FlameWorks burner control system using the ESP32-C6 microcontroller. The system hosts a web application directly on the burner controller, eliminating the need for internet connectivity or mobile app installation.

## System Architecture
- **ESP32-C6**: Main controller hosting WiFi Access Point and web server
- **SPIFFS**: File system storing the web application
- **WebSocket**: Real-time communication between web app and hardware
- **Production Hardware**: Relay modules, flame sensors, temperature sensors

## Required Hardware

### ESP32-C6 Development Board
- Any ESP32-C6 development board (recommended: ESP32-C6-DevKitC-1)
- Built-in WiFi 6, Bluetooth 5.0
- 320KB SRAM, 4MB Flash minimum

### Burner Control Hardware
- **Relay Modules**: 8-channel 5V relay board for burner control
- **Flame Sensors**: UV Tron flame sensors (R2868 or similar)
- **Temperature Sensors**: K-type thermocouples with MAX6675 amplifiers
- **Safety Components**: Emergency stop button, indicator LEDs
- **Power Supply**: 12V DC for relays, 3.3V/5V for ESP32-C6

### Wiring (Production Configuration)
```
ESP32-C6 GPIO Assignments:
- GPIO 0-7: Burner relay controls (active LOW)
- GPIO 8: Emergency stop input (active LOW with pullup)
- GPIO 9: System status LED (active HIGH)
- GPIO 10: Fault indicator LED (active HIGH)
- GPIO 18-21: SPI for temperature sensors (CLK, MISO, CS0-1)
- GPIO 4-7: Flame sensor inputs (digital, active HIGH)
```

## Software Setup

### 1. Install Arduino IDE
1. Download Arduino IDE 2.x from https://www.arduino.cc/en/software
2. Install and launch Arduino IDE

### 2. Configure ESP32-C6 Support
1. Open Arduino IDE
2. Go to **File > Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools > Board > Boards Manager**
5. Search for "esp32" and install "esp32 by Espressif Systems"
6. Select **Tools > Board > ESP32 Arduino > ESP32C6 Dev Module**

### 3. Install Required Libraries
Go to **Tools > Manage Libraries** and install:
- **AsyncTCP** by me-no-dev
- **ESPAsyncWebServer** by me-no-dev  
- **WebSocketsServer** by Markus Sattler
- **ArduinoJson** by Benoit Blanchon

### 4. Configure Board Settings
In Arduino IDE, set these board parameters:
- **Board**: ESP32C6 Dev Module
- **CPU Frequency**: 160MHz
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: Default 4MB with spiffs
- **Core Debug Level**: None (for production)

## Firmware Installation

### 1. Prepare the Web Application
The web application files need to be uploaded to the ESP32-C6's SPIFFS file system:

1. Create folder structure:
   ```
   ESP32-Burner-Controller/
   ├── FlameWorks_Burner_ESP32_C6.ino
   └── data/
       └── index.html
   ```

2. The `data/index.html` file contains the complete FlameWorks web interface

### 2. Upload File System (SPIFFS)
1. Install **ESP32 Sketch Data Upload** tool:
   - Download from: https://github.com/lorol/arduino-esp32fs-plugin
   - Extract to `Arduino/tools/ESP32FS/tool/`
   - Restart Arduino IDE

2. Upload SPIFFS:
   - Connect ESP32-C6 to computer via USB
   - Select correct COM port in **Tools > Port**
   - Go to **Tools > ESP32 Sketch Data Upload**
   - Wait for upload to complete

### 3. Upload Firmware
1. Open `FlameWorks_Burner_ESP32_C6.ino` in Arduino IDE
2. Connect ESP32-C6 via USB
3. Select correct COM port
4. Click **Upload** button
5. Wait for compilation and upload to complete

### 4. Verify Installation
1. Open Serial Monitor (**Tools > Serial Monitor**) at 115200 baud
2. Press ESP32-C6 reset button
3. Look for startup messages:
   ```
   FlameWorks Burner Controller - ESP32-C6
   Mounting SPIFFS...
   SPIFFS mounted successfully
   WiFi Access Point: FlameWorks-XXXXXX
   WiFi Password: flame2024
   Web server started on port 80
   WebSocket server started on port 81
   System initialized with 1 burner(s)
   ```

## System Operation

### 1. Connect to FlameWorks Network
1. On your phone/tablet/computer, go to WiFi settings
2. Connect to network: **FlameWorks-XXXXXX** (XXXXXX = ESP32 chip ID)
3. Password: **flame2024**
4. Open web browser and go to: **http://192.168.4.1**

### 2. Web Interface Features
- **Master Control**: Turn system on/off
- **Individual Burner Control**: Select and control each burner
- **Real-time Status**: Temperature, flame detection, system state
- **Safety Systems**: Emergency stop, fault detection
- **System Logs**: Real-time activity logging

### 3. Multi-Burner Configuration
To enable multiple burners, modify this line in the firmware:
```cpp
const int ACTIVE_BURNERS = 1;  // Change to 2, 3, 4, etc.
```

### 4. Safety Features
- **Emergency Stop**: Immediately shuts down all burners
- **Flame Detection**: Monitors pilot and main flame presence  
- **Temperature Monitoring**: Prevents overheating
- **Fault Recovery**: Automatic retry with lockout protection
- **Watchdog Timer**: System reset if frozen

## Testing and Validation

### 1. LED Testing (Safe Hardware Test)
Before connecting real burner hardware, test with LEDs:
```cpp
// In setup(), add LED testing
for(int i = 0; i < ACTIVE_BURNERS; i++) {
    digitalWrite(RELAY_PINS[i], LOW);   // LED ON
    delay(500);
    digitalWrite(RELAY_PINS[i], HIGH);  // LED OFF
    delay(500);
}
```

### 2. Relay Testing
1. Connect 8-channel relay board to ESP32-C6
2. Use web interface to test each burner
3. Verify relay click/LED indicators
4. Test emergency stop functionality

### 3. Production Hardware Integration
1. **NEVER** test with live gas without proper safety procedures
2. Integrate flame sensors and temperature monitoring
3. Test all safety interlocks
4. Validate emergency stop systems
5. Perform supervised test burns

## Troubleshooting

### Connection Issues
- **Can't connect to WiFi**: Check ESP32-C6 power, restart device
- **No web page**: Verify SPIFFS upload, check serial monitor for errors
- **WebSocket errors**: Ensure port 81 is not blocked

### Hardware Issues
- **Relays not switching**: Check GPIO assignments, power supply
- **Temperature readings wrong**: Verify SPI connections, sensor power
- **Flame sensors not working**: Check sensor orientation, sensitivity

### System Issues  
- **System resets**: Check power supply stability, add capacitors
- **Memory errors**: Reduce ACTIVE_BURNERS or optimize code
- **WiFi instability**: Move closer to device, reduce interference

## Advanced Configuration

### 1. Network Settings
Modify WiFi credentials in firmware:
```cpp
const char* ssid = "FlameWorks-Custom";
const char* password = "your-secure-password";
```

### 2. Hardware Pin Assignments
Customize GPIO pins for your hardware:
```cpp
const int RELAY_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};  // Modify as needed
const int EMERGENCY_PIN = 8;
const int STATUS_LED = 9;
```

### 3. Safety Parameters
Adjust safety timings:
```cpp
const unsigned long PILOT_TIMEOUT = 10000;    // 10 seconds
const unsigned long MAIN_TIMEOUT = 5000;      // 5 seconds  
const unsigned long WATCHDOG_TIMEOUT = 60000; // 1 minute
```

## Production Deployment Checklist

- [ ] Hardware connections verified and secure
- [ ] All safety systems tested and functional
- [ ] Emergency stop procedures established
- [ ] Temperature monitoring calibrated
- [ ] Flame detection sensitivity adjusted
- [ ] System backup and recovery procedures documented
- [ ] Staff training on web interface completed
- [ ] Regular maintenance schedule established

## Support and Updates

### Firmware Updates
To update firmware:
1. Download new `.ino` file
2. Upload via Arduino IDE (SPIFFS data preserved)
3. Monitor serial output for successful boot

### Hardware Modifications
- GPIO pins can be reassigned by modifying pin arrays
- Additional sensors can be added using available GPIO pins
- System supports 1-8 burners with current hardware design

### Safety Certification
**WARNING**: This system controls gas burners. Ensure compliance with:
- Local fire codes and regulations
- Gas appliance safety standards
- Professional installation and inspection requirements
- Regular safety system testing and maintenance

---

**FlameWorks ESP32-C6 Controller - Complete Offline Burner Management System**

*No internet required • No app installation needed • Direct hardware control*
