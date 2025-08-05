/*
 * FlameWerks Burner Controller Firmware
 * Arduino IDE Compatible - Save as Firmware.ino
 * 
 * Pin Configuration:
 * GPIO 2  -> Pilot Valve Relay (via optocoupler)
 * GPIO 4  -> Main Valve Relay (via optocoupler) 
 * GPIO 5  -> Igniter Control (via optocoupler)
 * A0      -> Flame Sensor Input (0-3.3V from amplifier)
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// --- Pin Definitions ---
#define PILOT_RELAY_PIN  2    // GPIO for pilot valve relay
#define MAIN_RELAY_PIN   4    // GPIO for main valve relay
#define IGNITER_PIN      5    // GPIO for igniter control
#define FLAME_SENSE_PIN  A0   // Analog pin for flame detection

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";        // Change to your WiFi network
const char* password = "YOUR_WIFI_PASSWORD"; // Change to your WiFi password
const char* hostname = "flamewerks-burner";

// --- State Machine ---
enum BurnerState {
  OFF,
  PILOT_IGNITING,
  MAIN_ON,
  FLAMEOUT,
  FAULT
};

struct Burner {
  int id;
  BurnerState state;
  unsigned long stateStartTime;
  int retryCount;
  bool flameDetected;
};

// For now, single burner demo (expandable to multiple)
Burner burner1 = {1, OFF, 0, 0, false};

// --- Timing Parameters ---
const unsigned long IGNITION_TIMEOUT_MS = 5000;  // 5 seconds for pilot ignition
const unsigned long FLAME_CHECK_INTERVAL = 100;  // Check flame every 100ms
const int MAX_RETRIES = 3;
const unsigned long COOLDOWN_MS = 3000;         // 3 second cooldown between retries
const int FLAME_THRESHOLD = 150;                // ADC threshold for flame detection

// --- Web Server ---
ESP8266WebServer server(80);

// --- Hardware Control Functions ---
void setPilotValve(bool state) {
  digitalWrite(PILOT_RELAY_PIN, state ? HIGH : LOW);
  Serial.print("Pilot valve: ");
  Serial.println(state ? "OPEN" : "CLOSED");
}

void setMainValve(bool state) {
  digitalWrite(MAIN_RELAY_PIN, state ? HIGH : LOW);
  Serial.print("Main valve: ");
  Serial.println(state ? "OPEN" : "CLOSED");
}

void setIgniter(bool state) {
  digitalWrite(IGNITER_PIN, state ? HIGH : LOW);
  Serial.print("Igniter: ");
  Serial.println(state ? "ON" : "OFF");
}

bool readFlameStatus() {
  int reading = analogRead(FLAME_SENSE_PIN);
  bool detected = reading > FLAME_THRESHOLD;
  
  // Debug output every 2 seconds when checking flame
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    Serial.print("Flame sensor ADC: ");
    Serial.print(reading);
    Serial.print(", Detected: ");
    Serial.println(detected ? "YES" : "NO");
    lastDebug = millis();
  }
  
  return detected;
}

void emergencyStop() {
  setPilotValve(false);
  setMainValve(false);
  setIgniter(false);
  burner1.state = OFF;
  burner1.retryCount = 0;
  Serial.println("EMERGENCY STOP - All systems OFF");
}

// --- State Machine Logic ---
void updateBurnerState() {
  unsigned long now = millis();
  burner1.flameDetected = readFlameStatus();
  
  switch (burner1.state) {
    case OFF:
      // All systems off, waiting for command
      setPilotValve(false);
      setMainValve(false);
      setIgniter(false);
      break;
      
    case PILOT_IGNITING:
      // Open pilot valve and activate igniter
      setPilotValve(true);
      setMainValve(false);
      setIgniter(true);
      
      if (burner1.flameDetected) {
        // Flame detected! Move to main valve operation
        setIgniter(false);  // Turn off igniter
        setMainValve(true); // Open main valve
        burner1.state = MAIN_ON;
        burner1.stateStartTime = now;
        Serial.println("STATE: Flame detected -> MAIN_ON");
      } 
      else if (now - burner1.stateStartTime > IGNITION_TIMEOUT_MS) {
        // Timeout - ignition failed
        setIgniter(false);
        setPilotValve(false);
        
        if (burner1.retryCount < MAX_RETRIES) {
          burner1.retryCount++;
          burner1.state = FLAMEOUT;
          burner1.stateStartTime = now;
          Serial.print("STATE: Ignition failed, retry ");
          Serial.print(burner1.retryCount);
          Serial.print("/");
          Serial.println(MAX_RETRIES);
        } else {
          burner1.state = FAULT;
          burner1.stateStartTime = now;
          Serial.println("STATE: Max retries exceeded -> FAULT");
        }
      }
      break;
      
    case MAIN_ON:
      // Normal operation - monitor for flame loss
      setPilotValve(true);
      setMainValve(true);
      setIgniter(false);
      
      if (!burner1.flameDetected) {
        // Flame lost!
        emergencyStop();
        burner1.state = FLAMEOUT;
        burner1.stateStartTime = now;
        Serial.println("STATE: Flame lost -> FLAMEOUT");
      }
      break;
      
    case FLAMEOUT:
      // Cooldown period before retry
      setPilotValve(false);
      setMainValve(false);
      setIgniter(false);
      
      if (now - burner1.stateStartTime > COOLDOWN_MS) {
        if (burner1.retryCount < MAX_RETRIES) {
          // Retry ignition
          burner1.state = PILOT_IGNITING;
          burner1.stateStartTime = now;
          Serial.println("STATE: Cooldown complete -> PILOT_IGNITING");
        } else {
          burner1.state = FAULT;
          Serial.println("STATE: Max retries exceeded -> FAULT");
        }
      }
      break;
      
    case FAULT:
      // System fault - manual reset required
      setPilotValve(false);
      setMainValve(false);
      setIgniter(false);
      break;
  }
}

// --- Web Server Handlers ---
void handleRoot() {
  // Serve the web UI
  server.sendHeader("Content-Type", "text/html");
  server.send(200, "text/html", getWebUI());
}

void handleStatus() {
  // Return status as JSON array (for multi-burner compatibility)
  String json = "[{";
  json += "\"id\":1,";
  json += "\"state\":\"";
  
  switch (burner1.state) {
    case OFF: json += "off"; break;
    case PILOT_IGNITING: json += "igniting"; break;
    case MAIN_ON: json += "on"; break;
    case FLAMEOUT: json += "flameout"; break;
    case FAULT: json += "fault"; break;
  }
  
  json += "\",\"flame\":";
  json += burner1.flameDetected ? "true" : "false";
  json += ",\"retries\":";
  json += String(burner1.retryCount);
  json += "}]";
  
  server.send(200, "application/json", json);
}

void handleControl() {
  String response = "";
  
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    
    if (cmd == "on") {
      if (burner1.state == OFF || burner1.state == FAULT) {
        burner1.state = PILOT_IGNITING;
        burner1.stateStartTime = millis();
        burner1.retryCount = 0;
        response = "Ignition sequence started";
        Serial.println("WEB COMMAND: Start ignition");
      } else {
        response = "Burner already active or igniting";
      }
    }
    else if (cmd == "off") {
      emergencyStop();
      response = "Emergency stop activated";
      Serial.println("WEB COMMAND: Emergency stop");
    }
    else if (cmd == "reset") {
      if (burner1.state == FAULT) {
        burner1.state = OFF;
        burner1.retryCount = 0;
        response = "System reset from FAULT to OFF";
        Serial.println("WEB COMMAND: Reset from fault");
      } else {
        response = "Reset only available in FAULT state";
      }
    }
    else {
      response = "Unknown command: " + cmd;
    }
  } else {
    response = "Missing cmd parameter";
  }
  
  server.send(200, "text/plain", response);
}

void handleNotFound() {
  server.send(404, "text/plain", "File not found");
}

// --- Serial Command Interface ---
void handleSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "i" || cmd == "ignite") {
      if (burner1.state == OFF || burner1.state == FAULT) {
        burner1.state = PILOT_IGNITING;
        burner1.stateStartTime = millis();
        burner1.retryCount = 0;
        Serial.println("SERIAL: Ignition sequence started");
      } else {
        Serial.println("SERIAL: Burner already active");
      }
    }
    else if (cmd == "e" || cmd == "stop") {
      emergencyStop();
      Serial.println("SERIAL: Emergency stop");
    }
    else if (cmd == "r" || cmd == "reset") {
      burner1.state = OFF;
      burner1.retryCount = 0;
      Serial.println("SERIAL: System reset");
    }
    else if (cmd == "s" || cmd == "status") {
      Serial.print("State: ");
      switch (burner1.state) {
        case OFF: Serial.print("OFF"); break;
        case PILOT_IGNITING: Serial.print("PILOT_IGNITING"); break;
        case MAIN_ON: Serial.print("MAIN_ON"); break;
        case FLAMEOUT: Serial.print("FLAMEOUT"); break;
        case FAULT: Serial.print("FAULT"); break;
      }
      Serial.print(", Flame: ");
      Serial.print(burner1.flameDetected ? "YES" : "NO");
      Serial.print(", Retries: ");
      Serial.println(burner1.retryCount);
    }
    else if (cmd == "help") {
      Serial.println("Commands: i/ignite, e/stop, r/reset, s/status, help");
    }
    else if (cmd.length() > 0) {
      Serial.println("Unknown command. Type 'help' for commands.");
    }
  }
}

// --- Web UI HTML (embedded) - Moved to PROGMEM to save RAM ---
const char PROGMEM webUI[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>FlameWerks Burner Control</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 1em; background: #f5f5f5; }
    .container { max-width: 800px; margin: 0 auto; }
    h1 { text-align: center; color: #333; }
    .status-panel { background: white; padding: 1em; border-radius: 8px; margin: 1em 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .burner-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1em; margin: 1em 0; }
    .burner-card { background: white; padding: 1em; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .burner-status { font-size: 1.2em; font-weight: bold; margin: 0.5em 0; }
    .status-off { color: #d32f2f; }
    .status-igniting { color: #ff9800; animation: blink 1s infinite; }
    .status-on { color: #388e3c; }
    .status-fault { color: #d32f2f; background: #ffebee; padding: 0.2em; }
    @keyframes blink { 0%, 50% { opacity: 1; } 51%, 100% { opacity: 0.3; } }
    .controls { display: flex; gap: 0.5em; flex-wrap: wrap; margin: 1em 0; }
    .btn { padding: 0.7em 1.2em; border: none; border-radius: 4px; cursor: pointer; font-size: 1em; transition: background 0.2s; }
    .btn-primary { background: #1976d2; color: white; }
    .btn-primary:hover { background: #1565c0; }
    .btn-danger { background: #d32f2f; color: white; }
    .btn-danger:hover { background: #c62828; }
    .btn-secondary { background: #757575; color: white; }
    .btn-secondary:hover { background: #616161; }
    .emergency { background: #ffcdd2; border: 2px solid #d32f2f; padding: 1em; border-radius: 8px; margin: 1em 0; text-align: center; }
    .flame-indicator { display: inline-block; width: 12px; height: 12px; border-radius: 50%; margin-left: 0.5em; }
    .flame-yes { background: #4caf50; }
    .flame-no { background: #f44336; }
  </style>
</head>
<body>
  <div class="container">
    <h1>FlameWerks Burner Control</h1>
    <div class="emergency">
      <button class="btn btn-danger" onclick="emergencyStop()">EMERGENCY STOP</button>
      <p>Immediately shuts off all valves and igniter</p>
    </div>
    <div class="status-panel">
      <h3>System Status</h3>
      <div id="systemStatus">Loading...</div>
    </div>
    <div class="burner-grid">
      <div class="burner-card">
        <h3>Burner 1</h3>
        <div id="burner1Status" class="burner-status">Loading...</div>
        <div id="burner1Flame">Flame: <span class="flame-indicator flame-no"></span></div>
        <div id="burner1Retries">Retries: 0/3</div>
        <div class="controls">
          <button class="btn btn-primary" onclick="startIgnition()">IGNITE</button>
          <button class="btn btn-danger" onclick="stopBurner()">STOP</button>
          <button class="btn btn-secondary" onclick="resetBurner()">RESET</button>
        </div>
      </div>
    </div>
  </div>

  <script>
    setInterval(updateStatus, 2000);
    updateStatus();

    async function updateStatus() {
      try {
        const response = await fetch('/status');
        const burners = await response.json();
        const burner = burners[0];
        
        const statusEl = document.getElementById('burner1Status');
        const flameEl = document.getElementById('burner1Flame');
        const retriesEl = document.getElementById('burner1Retries');
        
        let statusClass = '';
        let statusText = '';
        switch(burner.state) {
          case 'off': statusClass = 'status-off'; statusText = 'OFF'; break;
          case 'igniting': statusClass = 'status-igniting'; statusText = 'IGNITING...'; break;
          case 'on': statusClass = 'status-on'; statusText = 'RUNNING'; break;
          case 'flameout': statusClass = 'status-fault'; statusText = 'FLAMEOUT'; break;
          case 'fault': statusClass = 'status-fault'; statusText = 'FAULT'; break;
          default: statusClass = 'status-off'; statusText = 'UNKNOWN'; break;
        }
        
        statusEl.className = 'burner-status ' + statusClass;
        statusEl.textContent = statusText;
        
        const flameIndicator = flameEl.querySelector('.flame-indicator');
        flameIndicator.className = 'flame-indicator ' + (burner.flame ? 'flame-yes' : 'flame-no');
        flameEl.innerHTML = 'Flame: <span class="flame-indicator ' + (burner.flame ? 'flame-yes' : 'flame-no') + '"></span> ' + (burner.flame ? 'YES' : 'NO');
        
        retriesEl.textContent = 'Retries: ' + burner.retries + '/3';
        
        document.getElementById('systemStatus').textContent = 
          'Burner 1: ' + statusText + ' | Flame: ' + (burner.flame ? 'Detected' : 'None') + ' | IP: ' + location.hostname;
          
      } catch (error) {
        console.error('Status update failed:', error);
        document.getElementById('systemStatus').textContent = 'Connection Error - Check firmware';
      }
    }

    async function sendCommand(cmd) {
      try {
        const response = await fetch('/control', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'cmd=' + cmd
        });
        const result = await response.text();
        console.log('Command result:', result);
        setTimeout(updateStatus, 500);
      } catch (error) {
        console.error('Command failed:', error);
        alert('Command failed - check connection');
      }
    }

    function startIgnition() { sendCommand('on'); }
    function stopBurner() { sendCommand('off'); }
    function resetBurner() { sendCommand('reset'); }
    function emergencyStop() { 
      if (confirm('Emergency stop will immediately shut off all systems. Continue?')) {
        sendCommand('off'); 
      }
    }
  </script>
</body>
</html>
)rawliteral";

String getWebUI() {
  return FPSTR(webUI);
}

// --- Setup Function ---
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n=== FlameWerks Burner Controller Starting ===");
  
  // Initialize GPIO pins
  pinMode(PILOT_RELAY_PIN, OUTPUT);
  pinMode(MAIN_RELAY_PIN, OUTPUT);
  pinMode(IGNITER_PIN, OUTPUT);
  
  // Ensure all outputs start OFF
  setPilotValve(false);
  setMainValve(false);
  setIgniter(false);
  
  // Initialize burner state
  burner1.state = OFF;
  burner1.stateStartTime = millis();
  burner1.retryCount = 0;
  
  Serial.println("Hardware initialized - All systems OFF");
  
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
    
    // Start mDNS responder
    if (MDNS.begin(hostname)) {
      Serial.println("mDNS responder started");
      Serial.print("Access via: http://");
      Serial.print(hostname);
      Serial.println(".local/");
    }
  } else {
    Serial.println("\nWiFi connection failed!");
    Serial.println("System will continue in offline mode");
  }
  
  // Configure web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/control", HTTP_POST, handleControl);
  server.onNotFound(handleNotFound);
  
  // Start web server
  server.begin();
  Serial.println("Web server started on port 80");
  
  Serial.println("\n=== System Ready ===");
  Serial.println("Commands: i=ignite, e=stop, r=reset, s=status, help");
  Serial.println("Web UI: http://" + WiFi.localIP().toString() + "/");
  Serial.println("========================\n");
}

// --- Main Loop ---
void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Handle mDNS
  MDNS.update();
  
  // Handle serial commands
  handleSerialCommands();
  
  // Update burner state machine
  updateBurnerState();
  
  // Small delay to prevent excessive CPU usage
  delay(50);
}

