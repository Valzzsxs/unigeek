# Network MITM Attack — How It Works

A Man-in-the-Middle (MITM) attack intercepts traffic between devices on a network and the router. The device must first be connected to a WiFi network (via **WiFi > Network**), then the attack is launched from **WiFi > Network > MITM Attack**.

## How It Works

The attack combines up to four components that work together:

### 1. DHCP Starvation

Floods the network's legitimate DHCP server with fake lease requests using spoofed MAC addresses. Each request claims an IP address from the pool. Once the pool is exhausted (detected by receiving NAK responses), no new devices can get an IP from the real router.

- Sends DISCOVER → waits for OFFER → sends REQUEST → receives ACK/NAK
- Uses realistic OUI prefixes (Apple, Samsung, Intel, etc.) for spoofed MACs
- Non-blocking: runs one step per frame so the UI stays responsive
- Stops automatically when the pool is exhausted (NAK threshold) or stuck (20 consecutive timeouts)
- Status bar shows: ACK count, NAK count, timeout count, consecutive timeouts

### 2. Rogue DHCP Server

After starvation empties the real DHCP pool (or immediately if starvation is off), a rogue DHCP server starts on the ESP32. New devices joining the network — or existing devices renewing their lease — receive IP assignments from the ESP32 instead of the real router.

The rogue server sets itself as:
- **Gateway** — all traffic routes through the ESP32
- **DNS server** — all DNS queries go to the ESP32
- **WPAD proxy** — pushes a proxy auto-config URL via DHCP option 252, so HTTP traffic from devices that support WPAD is automatically proxied through the ESP32

This is what makes it a true MITM — the attacker becomes the network's gateway and DNS resolver.

### 3. DNS Spoofing

Intercepts DNS queries from victims and returns fake responses, redirecting specific domains to attacker-controlled pages (phishing portals, credential capture forms, etc.).

- Configured via `/unigeek/wifi/portals/dns_config` — maps domains to portal folders
- Serves static HTML/CSS/JS files from the mapped portal directories
- Handles captive portal detection domains (Apple, Google, Microsoft, Firefox) to trigger automatic portal popups
- Supports WPAD proxy auto-config on port 80 to intercept HTTP traffic
- Can forward unmatched domains to the real upstream DNS
- Captures and logs form POST data from phishing pages
- Credentials saved to `/unigeek/wifi/captives/`

### 4. Web File Manager

Optionally runs a file manager on port 8080, allowing the attacker to browse and manage device storage from a browser during the attack.

## Attack Flow

```
1. Connect to target WiFi (WiFi > Network)
2. Open MITM Attack
3. Enable desired components
4. Start

If DHCP Starvation + Rogue DHCP are both enabled:
  Starvation runs first → exhausts real DHCP pool
  → Rogue DHCP starts automatically after
  → New/renewing clients get our gateway + DNS

If only Rogue DHCP (no starvation):
  Starts immediately, races with real DHCP server
  (less reliable — real server may respond first)
```

## Setup

1. Connect to the target network via **WiFi > Network**
2. Go to **WiFi > Network > MITM Attack**
3. **Rogue DHCP** — Toggle on to become the network's gateway/DNS
4. **DNS Spoof** — Toggle on to redirect domains to phishing portals (requires `dns_config` file)
5. **File Manager** — Toggle on for remote file access on port 8080
6. **DHCP Starvation** — Toggle on to exhaust the real DHCP pool first (recommended with Rogue DHCP)
7. **Start** — Launches the attack

## During Attack

- Log screen shows real-time events: DHCP clients, DNS visits, POST captures
- Status bar shows starvation progress (ACK/NAK/timeout) or DHCP client count
- Press **BACK** or **Press** to stop

## Requirements

- Must be connected to a WiFi network first
- DNS Spoof requires `/unigeek/wifi/portals/dns_config` file with domain mappings
- File Manager requires web file manager HTML files on storage
- Works on the same network segment — devices on different VLANs won't be affected

## Notes

- DHCP Starvation is most effective on networks with small IP pools (e.g., /24 = 254 addresses)
- Enterprise networks with 802.1X, DHCP snooping, or dynamic ARP inspection will resist this attack
- The rogue DHCP only catches devices that request a new lease — already-connected devices with valid leases are unaffected until renewal
- WPAD interception only works on devices/browsers that support proxy auto-discovery (mainly Windows, some Linux)