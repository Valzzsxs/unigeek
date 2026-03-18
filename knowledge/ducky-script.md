# Ducky Script

Ducky Script is a simple scripting language for automating keyboard input. UniGeek executes `.ds` or `.txt` script files from storage, sending keystrokes over BLE or USB HID to a connected host computer.

## How to Use

1. Place script files in `/unigeek/keyboard/duckyscript/` on the device storage
2. Go to **Keyboard > BLE Keyboard** or **Keyboard > USB Keyboard**
3. Connect to the target device
4. Select **Ducky Script** and choose a script file to run

## Commands

| Command | Description | Example |
|---------|-------------|---------|
| `STRING <text>` | Type the text exactly as written | `STRING Hello World` |
| `STRINGLN <text>` | Type the text and press Enter | `STRINGLN ipconfig` |
| `ENTER` | Press the Enter key | `ENTER` |
| `DELAY <ms>` | Wait for the specified milliseconds | `DELAY 500` |
| `GUI <key>` | Hold Windows/Command key and press a key | `GUI r` (opens Run dialog) |
| `CTRL <key>` | Hold Ctrl and press a key | `CTRL c` (copy) |
| `ALT <key>` | Hold Alt and press a key | `ALT F4` (close window) |
| `SHIFT <key>` | Hold Shift and press a key | `SHIFT a` (types uppercase A) |
| `REM <comment>` | Comment line, ignored during execution | `REM This is a comment` |

## Key Parameter

The `<key>` parameter for `GUI`, `CTRL`, `ALT`, and `SHIFT` accepts:

- **Single character** — any printable ASCII character (e.g., `r`, `a`, `c`)
- **Function keys** — `F1` through `F12`

## Example Script

```
REM Opens notepad and types Hello World
DELAY 500
GUI r
DELAY 500
STRING notepad
ENTER
DELAY 1000
STRING Hello World from UniGeek!
```

## Sample Scripts

The following sample scripts are available via **WiFi > Network > Download > Firmware Sample Files**:

- `hello_world.txt` — Opens Notepad and types a message
- `rick_roll.txt` — Opens a browser to Rick Astley
- `wifi_password.txt` — Extracts saved WiFi passwords (Windows)
- `reverse_shell.txt` — Opens a reverse shell (Windows)
- `disable_defender.txt` — Disables Windows Defender
