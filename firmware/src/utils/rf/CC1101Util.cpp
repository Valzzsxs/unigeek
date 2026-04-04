//
// CC1101 Sub-GHz Utility — send/receive raw RF signals
// Reference: Bruce firmware (https://github.com/pr3y/Bruce)
//

#include "CC1101Util.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

bool CC1101Util::begin(SPIClass* spi, int8_t csPin, int8_t gdo0Pin) {
  _csPin = csPin;
  _gdo0Pin = gdo0Pin;
  if (csPin < 0 || gdo0Pin < 0) return false;

  // Hold CS high before init to avoid bus conflicts
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);

  if (spi) {
    ELECHOUSE_cc1101.setSPIinstance(spi);
  } else {
    ELECHOUSE_cc1101.setSPIinstance(nullptr);
    ELECHOUSE_cc1101.setBeginEndLogic(true);
  }

  ELECHOUSE_cc1101.setSpiPin(
    spi ? spi->pinSCK() : -1,
    spi ? spi->pinMISO() : -1,
    spi ? spi->pinMOSI() : -1,
    csPin
  );
  ELECHOUSE_cc1101.setGDO0(gdo0Pin);

  ELECHOUSE_cc1101.Init();
  if (!ELECHOUSE_cc1101.getCC1101()) {
    _initialized = false;
    return false;
  }

  _initialized = true;

  // Default config: ASK/OOK, 50kBaud, async serial, narrow RX BW
  ELECHOUSE_cc1101.setRxBW(256);
  ELECHOUSE_cc1101.setClb(1, 13, 15);
  ELECHOUSE_cc1101.setClb(2, 16, 19);
  ELECHOUSE_cc1101.setModulation(2); // ASK/OOK
  ELECHOUSE_cc1101.setDRate(50);
  ELECHOUSE_cc1101.setPktFormat(3);  // Async serial mode
  setFrequency(_freq);

  return true;
}

void CC1101Util::end() {
  if (_initialized) {
    ELECHOUSE_cc1101.setSidle();
    _initialized = false;
  }
  if (_csPin >= 0) {
    digitalWrite(_csPin, HIGH);
  }
}

bool CC1101Util::setFrequency(float mhz) {
  bool valid = (mhz >= 280 && mhz <= 350) ||
               (mhz >= 387 && mhz <= 468) ||
               (mhz >= 779 && mhz <= 928);
  if (!valid) return false;
  _freq = mhz;
  if (_initialized) {
    ELECHOUSE_cc1101.setMHZ(mhz);
  }
  return true;
}

bool CC1101Util::isConnected() {
  return _initialized && ELECHOUSE_cc1101.getCC1101();
}

// ── Receive ─────────────────────────────────────────────────────────────────

// Capture raw pulses via GDO0 using GPIO polling + micros()
bool CC1101Util::receive(Signal& out, uint32_t timeoutMs,
                         std::function<bool()> cancelCb) {
  if (!_initialized) return false;
  _initRx();

  // Buffer for raw pulse durations (signed: + = HIGH, - = LOW)
  int32_t* buf = new int32_t[MAX_RAW_LEN];
  uint16_t count = 0;

  // Wait for first signal edge
  uint32_t startWait = millis();
  bool lastLevel = digitalRead(_gdo0Pin);

  while (millis() - startWait < timeoutMs) {
    if (cancelCb && cancelCb()) {
      delete[] buf;
      return false;
    }
    if (digitalRead(_gdo0Pin) != lastLevel) break;
    delayMicroseconds(10);
  }

  if (millis() - startWait >= timeoutMs) {
    delete[] buf;
    return false;
  }

  // Capture pulses
  uint32_t lastEdge = micros();
  bool level = digitalRead(_gdo0Pin);
  uint32_t silenceStart = 0;

  while (count < MAX_RAW_LEN) {
    bool now = digitalRead(_gdo0Pin);
    if (now != level) {
      uint32_t duration = micros() - lastEdge;
      if (duration > 0 && duration < 500000) { // ignore > 500ms
        buf[count++] = level ? (int32_t)duration : -(int32_t)duration;
      }
      level = now;
      lastEdge = micros();
      silenceStart = 0;
    } else {
      // Check for end of signal (silence > 20ms after we have data)
      if (count >= 5) {
        if (silenceStart == 0) silenceStart = micros();
        else if (micros() - silenceStart > 20000) break;
      }
      delayMicroseconds(1);
    }

    if (cancelCb && (count % 100 == 0) && cancelCb()) {
      delete[] buf;
      return false;
    }
  }

  if (count < 5) {
    delete[] buf;
    return false;
  }

  // Build raw data string
  String rawStr;
  rawStr.reserve(count * 6);
  for (uint16_t i = 0; i < count; i++) {
    if (i > 0) rawStr += ' ';
    rawStr += String(buf[i]);
  }
  delete[] buf;

  out.frequency = _freq;
  out.preset = "0";
  out.protocol = "RAW";
  out.rawData = rawStr;

  return true;
}

void CC1101Util::_initRx() {
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setPktFormat(3);
  ELECHOUSE_cc1101.SetRx();
  pinMode(_gdo0Pin, INPUT);
}

// ── Send ────────────────────────────────────────────────────────────────────

void CC1101Util::sendSignal(const Signal& sig) {
  if (!_initialized) return;

  // Apply frequency from signal
  float freq = sig.frequency;
  if (freq > 0) setFrequency(freq);

  // Parse preset for modulation settings
  int modulation = 2; // ASK/OOK default
  float deviation = 1.58;
  float rxBW = 270.83;
  float dataRate = 10;

  if (sig.preset == "FuriHalSubGhzPresetOok270Async") {
    modulation = 2; rxBW = 270;
  } else if (sig.preset == "FuriHalSubGhzPresetOok650Async") {
    modulation = 2; rxBW = 650;
  } else if (sig.preset == "FuriHalSubGhzPreset2FSKDev238Async") {
    modulation = 0; deviation = 2.380371; rxBW = 238;
  } else if (sig.preset == "FuriHalSubGhzPreset2FSKDev476Async") {
    modulation = 0; deviation = 47.60742; rxBW = 476;
  } else if (sig.preset == "FuriHalSubGhzPresetMSK99_97KbAsync") {
    modulation = 4; deviation = 47.60742; dataRate = 99.97;
  } else if (sig.preset == "FuriHalSubGhzPresetGFSK9_99KbAsync") {
    modulation = 1; deviation = 19.042969; dataRate = 9.996;
  }

  _initTx();
  ELECHOUSE_cc1101.setModulation(modulation);
  ELECHOUSE_cc1101.setDeviation(deviation);
  ELECHOUSE_cc1101.setRxBW(rxBW);
  ELECHOUSE_cc1101.setDRate(dataRate);
  ELECHOUSE_cc1101.setPA(12);
  ELECHOUSE_cc1101.SetTx();

  // Parse and transmit raw data
  String data = sig.rawData;
  int start = 0;
  for (int i = 0; i <= (int)data.length(); i++) {
    if (i == (int)data.length() || data[i] == ' ') {
      if (i > start) {
        int32_t val = data.substring(start, i).toInt();
        if (val > 0) {
          digitalWrite(_gdo0Pin, HIGH);
          delayMicroseconds(val);
        } else if (val < 0) {
          digitalWrite(_gdo0Pin, LOW);
          delayMicroseconds(-val);
        }
      }
      start = i + 1;
    }
  }
  digitalWrite(_gdo0Pin, LOW);

  // Return to idle
  ELECHOUSE_cc1101.setSidle();
}

void CC1101Util::_initTx() {
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setPktFormat(3);
  pinMode(_gdo0Pin, OUTPUT);
  ELECHOUSE_cc1101.setPA(12);
  ELECHOUSE_cc1101.SetTx();
}

// ── File I/O ────────────────────────────────────────────────────────────────

bool CC1101Util::loadFile(const String& content, Signal& out) {
  out = Signal();
  out.frequency = 0;
  out.preset = "0";
  out.protocol = "RAW";

  String rawAccum;
  int start = 0;

  while (start < (int)content.length()) {
    int nl = content.indexOf('\n', start);
    if (nl < 0) nl = content.length();
    String line = content.substring(start, nl);
    line.trim();
    start = nl + 1;

    if (line.startsWith("Filetype:") || line.startsWith("Version")) continue;

    int colonIdx = line.indexOf(':');
    if (colonIdx < 0) continue;

    String key = line.substring(0, colonIdx);
    String val = line.substring(colonIdx + 1);
    val.trim();
    if (val.endsWith("\r")) val.remove(val.length() - 1);

    if (key == "Frequency") {
      // Stored as Hz in file, convert to MHz
      out.frequency = val.toFloat() / 1000000.0f;
    } else if (key == "Preset") {
      out.preset = val;
    } else if (key == "Protocol") {
      out.protocol = val;
    } else if (key == "RAW_Data" || key == "Data_RAW") {
      if (rawAccum.length() > 0) rawAccum += ' ';
      rawAccum += val;
    }
  }

  out.rawData = rawAccum;
  return out.rawData.length() > 0 && out.frequency > 0;
}

String CC1101Util::saveToString(const Signal& sig) {
  String content = "Filetype: SubGhz Signal File\nVersion 1\n";
  content += "Frequency: " + String((uint32_t)(sig.frequency * 1000000)) + "\n";
  content += "Preset: " + sig.preset + "\n";
  content += "Protocol: " + sig.protocol + "\n";

  // Split RAW_Data into lines of max 512 values (Flipper compat)
  String data = sig.rawData;
  int valCount = 0;
  int lineStart = 0;
  bool inLine = false;

  for (int i = 0; i <= (int)data.length(); i++) {
    if (i == (int)data.length() || data[i] == ' ') {
      if (i > lineStart || (i == (int)data.length() && i > lineStart)) {
        if (!inLine) {
          content += "RAW_Data: ";
          inLine = true;
        }
        content += data.substring(lineStart, i);
        valCount++;
        if (i < (int)data.length()) content += ' ';
        if (valCount % 512 == 0 && i < (int)data.length()) {
          content += "\nRAW_Data: ";
        }
      }
      lineStart = i + 1;
    }
  }
  content += "\n";
  return content;
}

String CC1101Util::signalLabel(const Signal& sig) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f MHz", sig.frequency);
  return String(buf);
}
