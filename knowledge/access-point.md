# Access Point

Broadcast a custom WiFi access point with optional rogue DNS, captive portal, and file manager.

## Menu Options

- **SSID** — Set the AP name (max 32 chars)
- **Password** — Set password (min 8 chars, or empty for open network)
- **Hidden** — Toggle hidden SSID
- **Rogue DNS** — Redirect configured domains to custom pages served from device storage; requires `dns_config` file
- **Captive Portal** — Select a portal template shown on WiFi connectivity checks (the "Sign in to network" popup); choose from folders in `/unigeek/wifi/portals/`
- **File Manager** — Enable web file manager accessible at `unigeek.local`; requires web files installed
- **Start** — Launch the AP and show real-time log

## Rogue DNS

When enabled, the device runs its own DHCP and DNS servers. All DNS queries resolve to the ESP32's IP. The web server routes requests based on the `Host` header:

- **Configured domains** (from `dns_config`) — Served from the path specified in the config
- **Unconfigured domains** — Served from `/unigeek/wifi/portals/default/`
- **Connectivity check domains** (captive.apple.com, etc.) — If Captive Portal is enabled, serves the selected portal template; otherwise returns HTTP 204

### dns_config File

Located at `/unigeek/wifi/portals/dns_config`. Format:

```
# domain:path
google.com:/unigeek/wifi/portals/google
facebook.com:/unigeek/wifi/portals/facebook
```

## Captive Portal

The captive portal only triggers on WiFi connectivity check URLs (used by iOS, Android, Windows, etc. to detect login pages). It does NOT affect regular browsing.

POST form submissions are saved to `/unigeek/wifi/captives/{domain}.txt`.

## File Manager in AP Mode

When File Manager is enabled, visiting `unigeek.local` redirects to the file manager on port 8080. This only works when the toggle is on.
