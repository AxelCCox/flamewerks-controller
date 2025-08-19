# 🔥 FlameWorks ESP32-C6 - Quick Test & Upload Guide

## Ready for Upload! 🚀

Your FlameWorks ESP32-C6 firmware is now ready with comprehensive testing capabilities.

## Upload Steps:

### 1. Upload SPIFFS Data (Web App)
1. **Connect ESP32-C6** to USB
2. **Open Arduino IDE**
3. **Open** `FlameWorks_Burner_ESP32_C6.ino`
4. **Select Board**: Tools → Board → ESP32 Arduino → ESP32C6 Dev Module
5. **Select Port**: Tools → Port → (your COM port)
6. **Upload SPIFFS**: Tools → ESP32 Sketch Data Upload
7. **Wait** for "SPIFFS Image Uploaded" message

### 2. Upload Firmware  
1. **Click Upload** (→ button) in Arduino IDE
2. **Wait** for compilation and upload
3. **Open Serial Monitor** (Tools → Serial Monitor, 115200 baud)
4. **Press Reset** on ESP32-C6

## What You'll See in Serial Monitor:

```
=============================================
🔥 FlameWorks Burner Controller - ESP32-C6
=============================================
Chip Model: ESP32-C6
Flash Size: 4 MB
Free Heap: 200000+ bytes
---------------------------------------------
📌 Initializing GPIO pins...
✅ GPIO pins initialized
🔥 Initializing burner systems...
✅ Burner systems ready
💾 Mounting SPIFFS file system...
✅ SPIFFS mounted successfully
📶 Creating WiFi Access Point...
✅ WiFi AP created: FlameWorks-ABCDEF
🔑 Password: flame2024
🌐 IP Address: 192.168.4.1
🌐 Starting web server...
✅ Web server started on port 80
🔌 Starting WebSocket server...
✅ WebSocket server started on port 81

=============================================
🎉 SYSTEM READY!
=============================================
📡 Connect to WiFi: FlameWorks-ABCDEF
🔑 Password: flame2024
🌐 Web Interface: http://192.168.4.1
🔥 Active Burners: 1
=============================================

📝 SERIAL COMMANDS:
   status    - Show system status
   test      - Test all relays (LED mode)
   ignite X  - Ignite burner X (1-8)
   stop X    - Stop burner X (1-8)
   emergency - Emergency stop all
   reset     - Reset system
   help      - Show this help
=============================================
```

## Testing Commands:

### Safe LED Testing (No Gas!)
```
test          - Test all relay outputs with LEDs
status        - Show complete system status
help          - Show all available commands
```

### Burner Control (Use with LEDs first!)
```
ignite 1      - Start ignition sequence for burner 1
stop 1        - Stop burner 1
emergency     - Emergency stop all burners
reset         - Restart the ESP32-C6
```

### Example Test Session:
1. Type `status` - See system overview
2. Type `test` - Watch LEDs flash on relay outputs
3. Type `ignite 1` - Start burner 1 (relay should activate)
4. Type `status` - See burner state change
5. Type `stop 1` - Stop burner 1
6. Type `emergency` - Test emergency stop

## Web Interface Testing:

### 1. Connect to WiFi
- **Network**: FlameWorks-XXXXXX (XXXXXX = your chip ID)
- **Password**: flame2024

### 2. Open Web App
- **URL**: http://192.168.4.1
- **You Should See**: Professional burner control interface

### 3. Test Web Controls
- **Click burner cards** to select them
- **Try Master Control** button
- **Watch real-time status** updates
- **Check system logs** at bottom

## Hardware Connections for Testing:

### LED Testing (Safe):
```
ESP32-C6 Pin → LED + Resistor → Ground
GPIO 0       → LED 1 (Burner 1 Relay)
GPIO 1       → LED 2 (Burner 2 Relay) [if using multiple]
GPIO 9       → LED (Status LED)
```

### Relay Testing:
```
ESP32-C6 Pin → Relay Module Pin
GPIO 0       → IN1 (Burner 1)
GPIO 1       → IN2 (Burner 2)
GPIO 9       → Status LED
GPIO 8       → Emergency Stop Button (to Ground)
5V/3V3       → VCC on Relay Module
GND          → GND on Relay Module
```

## Troubleshooting:

### Serial Monitor Shows Nothing:
- Check baud rate = 115200
- Press Reset button on ESP32-C6
- Try different USB cable/port

### Can't Connect to WiFi:
- Look for "FlameWorks-" network in WiFi list
- Password is exactly: flame2024
- Move closer to ESP32-C6

### Web Page Won't Load:
- Ensure SPIFFS uploaded successfully
- Try http://192.168.4.1 exactly
- Check Serial Monitor for errors

### Commands Not Working:
- Type exactly as shown (lowercase)
- Press Enter after each command
- Check for typos

## Next Steps After Testing:

1. **✅ Verify LED/Relay Operation**: All outputs working
2. **✅ Test Web Interface**: Full functionality  
3. **✅ Test Serial Commands**: All commands responsive
4. **⚠️ Safety First**: Never test with gas without proper safety procedures
5. **🔧 Hardware Integration**: Add flame sensors, temperature monitoring
6. **🏭 Production Setup**: Professional installation and testing

## Success Indicators:

- **Serial Monitor**: Clear startup messages, no errors
- **WiFi**: Can connect to FlameWorks network  
- **Web App**: Loads at http://192.168.4.1
- **LEDs/Relays**: Respond to commands
- **Real-time Updates**: Web interface updates from serial commands

**Your FlameWorks ESP32-C6 system is ready for testing! 🎉**

**Safe testing first - Real hardware integration only after full validation!**
