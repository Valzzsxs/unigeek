# Karma Attack — How It Works

Karma exploits the way devices automatically search for saved WiFi networks. When a phone or laptop has WiFi on but isn't connected, it broadcasts **probe requests** — announcing the names of networks it has connected to before. Karma listens for these probes, then creates a fake open AP with that exact name. The device thinks it found its known network and auto-connects, landing on a captive portal.

## Setup

1. Go to **WiFi > Karma Attack**
2. **Save WiFi List** — Toggle on/off; when enabled, all discovered SSIDs are saved to `/unigeek/wifi/karma/list.txt`
3. **Captive Portal** — Select a portal template from `/unigeek/wifi/portals/`; download sample portals via **WiFi > Network > Download > Firmware Sample Files** if none exist
4. **Waiting Time** — How long (in seconds) to keep each fake AP active waiting for a device to connect (default: 15s)
5. **Waiting Input** — How long (in seconds) to wait for the connected user to submit credentials on the portal (default: 120s); resets on each form submission
6. **Start** — Launches the attack

## During Attack

The attack runs in an automated cycle:

1. **Sniff** — Promiscuous mode captures probe request frames from nearby devices
2. **Deploy** — For each new SSID discovered, a fake open AP is created with a captive portal
3. **Wait for connect** — If no device connects within the waiting time, the SSID is blacklisted and the next one is tried
4. **Wait for input** — If a device connects but submits nothing within the input timeout, the SSID is blacklisted and the cycle continues
5. **Capture** — When the victim submits the portal form, credentials are saved and a notification sounds

The status bar shows real-time stats:
- **AP** — Total probe SSIDs captured
- **V** — Portal page visits
- **P** — Form submissions (POSTs)
- Right side shows the current SSID being attacked, or "Sniffing..." when listening

Press **BACK** or **Press** to stop the attack.

## Files

| Path | Description |
|------|-------------|
| `/unigeek/wifi/captives/karma_ssid.txt` | Saved probe SSIDs, format: `{epoch}:{ssid}` per line |
| `/unigeek/wifi/captives/karma_<SSID>.txt` | Captured credentials per SSID |
| `/unigeek/wifi/portals/<name>/` | Portal templates (shared with Evil Twin) |

## Notes

- Karma works best against older devices, IoT gadgets, and laptops. Modern iOS/Android versions use randomized or hidden probes, reducing effectiveness.
- The blacklist is session-only — SSIDs that timed out won't be retried during the current attack, but are cleared when you stop and restart.
- Karma creates **open** APs (no password), so there is no WPA handshake — this is purely a captive portal phishing attack.
- Cannot run simultaneously with other WiFi features (EAPOL capture, BLE, etc.) since they share the radio.