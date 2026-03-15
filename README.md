# UniGeek Firmware

Multi-tool firmware for ESP32-based handheld devices. Built with PlatformIO + Arduino framework + TFT_eSPI.

---

## Supported Devices

| Device | Keyboard | Speaker | USB HID | SD Card | Power Off |
|--------|----------|---------|---------|---------|-----------|
| M5StickC Plus 1.1 | — | Buzzer | — | — | Yes |
| LilyGO T-Lora Pager | TCA8418 | I2S | Yes | Yes | Yes |
| M5Stack Cardputer | GPIO Matrix | I2S | Yes | Yes | — |
| M5Stack Cardputer ADV | TCA8418 | I2S + ES8311 | Yes | Yes | — |
| LilyGO T-Display 16MB | 2 Buttons | — | — | — | — |

---

## Features

### WiFi
- **Network** — Connect to a WiFi network, sync time via NTP, world clock display
- **Access Point** — Broadcast a custom access point
- **WiFi Analyzer** — Scan and display nearby networks with signal strength and channel info
- **Packet Monitor** — Visualize raw 802.11 traffic by channel
- **Evil Twin** — Clone a target AP's SSID with a captive portal to capture credentials; supports custom portal templates, optional deauthentication of the real AP, and real-time password verification against the target network
- **WiFi Deauther** — Send deauthentication frames to a target network
- **Deauther Detector** — Monitor and list detected deauthentication attacks
- **Beacon Spam** — Broadcast fake SSIDs repeatedly
- **CIW Zeroclick** — Broadcast crafted SSIDs containing injection payloads (command injection, XSS, CRLF, JNDI, buffer overflow, format string, etc.) to test how nearby devices handle untrusted network names; tracks which devices connect and flags potential crashes
- **ESPNOW Chat** — Peer-to-peer text chat over ESP-NOW (no router required)
- **EAPOL Capture** — Passive WPA2 handshake capture; auto-discovers APs, deauths clients, waits for M1+M2 reconnect handshake, saves to PCAP on storage
- **EAPOL Brute Force** — Offline WPA2 password cracking from a captured PCAP; select wordlist from storage or use a built-in test list; runs PBKDF2-HMAC-SHA1 + PTK/MIC verification on-device
- **Download** — Download web file manager HTML and firmware sample files (portal templates, DuckyScript payloads, QR codes, password wordlists) directly from GitHub to device storage

### Bluetooth
- **BLE Analyzer** — Scan nearby BLE devices, display RSSI, name, address, and advertisement data
- **BLE Spam** — Spam BLE advertisement packets
- **BLE Detector** — Passive BLE scanner that detects Flipper Zero devices (by service UUID, checks for spoofed MAC vs legit OUI), credit card skimmers (by known module names like HC-05/HC-06 and suspicious MAC prefixes), Apple AirTags/FindMy trackers (by manufacturer data signature, with distance estimation and movement trend), and BLE spam attacks (Apple, Android, Samsung, Windows patterns matched against manufacturer data)

### Keyboard (HID)
- **BLE Keyboard** — Act as a wireless Bluetooth HID keyboard (all devices)
- **USB Keyboard** — Act as a wired USB HID keyboard (ESP32-S3 devices only)
- **Keyboard Relay** — Forward physical keypresses directly to the connected host in real time (keyboard devices only)
- **Ducky Script** — Run `.ds` script files from storage to automate keystrokes (supports STRING, DELAY, GUI, CTRL, ALT, SHIFT, ENTER, REM)

### Utility
- **I2C Detector** — Scan I2C bus and list all responding device addresses
- **QR Code** — Generate and display a QR code from typed or file-loaded text; supports WiFi QR format
- **File Manager** — Browse, rename, copy, cut, paste, and delete files and folders on storage; hold 1s to open context menu

### Games
- **HEX Decoder** — Wordle-style game using hexadecimal characters (0–9, A–F)
  - Guess a 4-character hex code in the fewest attempts
  - Color-coded feedback: green = correct position, orange = wrong position, red = not in code
  - 4 difficulty levels: Easy (14 attempts, 3 min), Medium (7 attempts, 90 sec), Hard (unlimited, 3 min), Extreme (unlimited, 90 sec)
  - Keyboard devices type directly; non-keyboard devices cycle characters with UP/DOWN and use the `<` erase option
- **Wordle** — Classic word-guessing game in English and Indonesian
  - Guess a 5-letter word in up to 10 attempts
  - Color-coded feedback: green = correct position, orange = wrong position, red = not in word
  - 3 difficulty levels: Easy (10 attempts, colors + alphabet hint), Medium (7 attempts, colors), Hard (7 attempts, no colors)
  - Choose between Common (curated) or Full word database
  - Available in English (EN) and Indonesian (ID)

### Settings
- Device name
- Auto display-off and display-off timeout
- Auto power-off and power-off timeout
- Brightness
- Volume (on boards with hardware volume control)
- Navigation sound toggle
- Theme color
- Navigation mode — Default or Encoder (M5StickC Plus only)

---

## Building

Install [PlatformIO](https://platformio.org/), then run:

```bash
# Build
pio run -e m5stickcplus_11
pio run -e t_lora_pager
pio run -e m5_cardputer
pio run -e m5_cardputer_adv
pio run -e t_display_16mb

# Flash
pio run -e m5stickcplus_11 -t upload
pio run -e t_lora_pager -t upload
pio run -e m5_cardputer -t upload
pio run -e m5_cardputer_adv -t upload
pio run -e t_display_16mb -t upload

# Serial monitor
pio device monitor
```

---

## Navigation

Navigation varies by device:

| Action | M5StickC (Default) | M5StickC (Encoder) | Cardputer / T-Lora Pager |
|--------|--------------------|--------------------|--------------------------|
| Up | AXP button | Rotate CCW | `;` key |
| Down | BTN\_B | Rotate CW | `.` key |
| Select | BTN\_A | Encoder press | `Enter` key |
| Back | — | BTN\_A (short press) | `Backspace` key |
| Left | — | AXP button | `,` key |
| Right | — | BTN\_B | `/` key |

On M5StickC, hold BTN\_A for 3 seconds to reset navigation mode to Default.

---

## Storage

Files are stored under `/unigeek/` on either SD card or LittleFS (fallback):

```
/unigeek/config                    device configuration
/unigeek/keyboard/duckyscript/     Ducky Script files (.ds)
/unigeek/wifi/eapol/               WPA2 handshake captures (.pcap)
/unigeek/wifi/portals/             Evil Twin portal templates (HTML/CSS/JS)
/unigeek/wifi/captives/            Captured credentials from Evil Twin
/unigeek/qrcode/                   QR code content files
/unigeek/utility/passwords/        Password wordlists for EAPOL brute force
/unigeek/web/file_manager/         Web file manager HTML files
```

SD card is used when available. LittleFS is always present as a fallback.

Sample files can be downloaded directly to the device via **WiFi > Network > Download > Firmware Sample Files** (requires WiFi connection).

---

## Evil Twin — How It Works

Evil Twin creates a fake WiFi access point with the same name as a real network, serving a captive portal page to trick users into entering their credentials.

### Setup

1. Go to **WiFi > Evil Twin**
2. **Network** — Scan and select the target WiFi network to clone
3. **Deauth** (optional) — Toggle on to send deauth frames to the real AP, forcing clients to disconnect and reconnect to your fake AP
4. **Check Password** (optional) — Toggle on to verify captured passwords against the real network in real-time; the victim sees a "Connecting..." page while verification runs, then "Connected!" or "Incorrect password"
5. **Portal** — Select a portal template from storage (HTML login pages stored in `/unigeek/wifi/portals/`); download sample portals via **WiFi > Network > Download > Firmware Sample Files** if none exist
6. **Start** — Launches the attack

### During Attack

- The device creates an open AP cloning the target's SSID and channel
- A DNS server redirects all requests to the captive portal
- When a victim connects and opens a browser, they see the portal login page
- Submitted credentials are saved to `/unigeek/wifi/captives/`
- Browse captured credentials from any connected device at `http://<device-ip>/captives`
- The log screen shows real-time events (portal visits, passwords received, verification results)
- Press **BACK** or **Press** to stop the attack

### Custom Portal Templates

Create your own portal by adding a folder under `/unigeek/wifi/portals/`:

```
/unigeek/wifi/portals/my-portal/
  index.htm       main login page (required)
  success.htm     shown after credentials are submitted (optional)
  style.css       stylesheet (optional)
  script.js       JavaScript (optional)
```

The login form must use `POST` method to `/`. Include an input named `password` to enable password verification. All POST parameters are captured regardless of field names.

---

## BLE Detector — How It Works

BLE Detector passively scans for nearby Bluetooth Low Energy devices and identifies potential threats based on known signatures.

### What It Detects

| Type | How It's Identified |
|------|-------------------|
| **Flipper Zero** | Detected by its service UUID; distinguishes real Flipper devices (legit OUI) from spoofed MACs |
| **Credit Card Skimmers** | Matches known Bluetooth module names (HC-05, HC-06, etc.) and suspicious MAC prefixes commonly used in skimming hardware |
| **Apple AirTags / FindMy Trackers** | Identified by Apple manufacturer data signature; shows estimated distance and movement trend (approaching/stationary/moving away) |
| **BLE Spam Attacks** | Detects spam patterns targeting Apple, Android, Samsung, and Windows devices by matching against known manufacturer data signatures |

### Usage

1. Go to **Bluetooth > BLE Detector**
2. The scanner runs continuously, listing detected threats in real-time
3. Each entry shows the device type, name (if available), RSSI signal strength, and MAC address
4. Press **BACK** or **Press** to stop scanning

---

## Web File Manager

Manage device files from a browser over WiFi.

### Setup

1. Connect the device to a WiFi network via **WiFi > Network**
2. Download the web page via **WiFi > Network > Download > Web File Manager**
3. Go to **WiFi > Network > Web File Manager** (or **WiFi > Access Point > File Manager** for AP mode)
4. Set a password and start the server
5. Open `http://unigeek.local` or the displayed IP address in a browser

---

## Project Structure

```
firmware/
├── boards/              board-specific hardware implementations
│   ├── m5stickplus_11/
│   ├── t_lora_pager/
│   ├── m5_cardputer/
│   └── m5_cardputer_adv/
└── src/
    ├── core/            interfaces and shared drivers (IStorage, ISpeaker, etc.)
    ├── screens/         all UI screens organized by category
    │   ├── wifi/
    │   ├── ble/
    │   ├── keyboard/
    │   ├── utility/
    │   ├── game/
    │   └── ...
    ├── ui/              templates, components, and action overlays
    └── utils/           keyboard HID utilities, DuckyScript runner
```

---

## TODO

- Karma Attack
- GPS Wardriving
- NFC Hack
- Lora