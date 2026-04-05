#include "SubGHzScreen.h"
#include "core/ScreenManager.h"
#include "core/Device.h"
#include "core/PinConfigManager.h"
#include "screens/module/ModuleMenuScreen.h"
#include "ui/actions/InputNumberAction.h"
#include "ui/actions/InputTextAction.h"
#include "ui/actions/InputSelectOption.h"
#include "ui/actions/ShowStatusAction.h"
#include "ui/views/ProgressView.h"

#ifndef CC1101_SCK_PIN
  #define CC1101_SCK_PIN  SPI_SCK_PIN
#endif
#ifndef CC1101_MISO_PIN
  #define CC1101_MISO_PIN SPI_MISO_PIN
#endif
#ifndef CC1101_MOSI_PIN
  #define CC1101_MOSI_PIN SPI_MOSI_PIN
#endif

void SubGHzScreen::onInit() {
  _csPin   = PinConfig.get(PIN_CONFIG_CC1101_CS,   PIN_CONFIG_CC1101_CS_DEFAULT).toInt();
  _gdo0Pin = PinConfig.get(PIN_CONFIG_CC1101_GDO0, PIN_CONFIG_CC1101_GDO0_DEFAULT).toInt();
  _showMenu();
}

void SubGHzScreen::onUpdate() {
  if (_state == STATE_RECEIVING) {
    bool cancelled = false;

    CC1101Util::Signal sig;
    bool got = _rf.receive(sig, 10000, [&cancelled]() -> bool {
      Uni.update();
      if (Uni.Nav->wasPressed()) {
        auto dir = Uni.Nav->readDirection();
        if (dir == INavigation::DIR_BACK || dir == INavigation::DIR_PRESS) {
          cancelled = true;
          return true;
        }
      }
      return false;
    });

    if (cancelled) {
      _rf.end();
      if (_capturedCount > 0) {
        _showCapturedList();
      } else {
        _showMenu();
      }
      return;
    }

    if (got && _capturedCount < kMaxCapture) {
      _capturedSignals[_capturedCount] = sig;
      _capturedTimes[_capturedCount]   = _generateTimestampName();
      _capturedSaved[_capturedCount]   = false;
      _capturedCount++;
      render();
    }
    return;
  }

  if (_state == STATE_JAMMING) {
    if (Uni.Nav->wasPressed()) {
      auto dir = Uni.Nav->readDirection();
      if (dir == INavigation::DIR_BACK || dir == INavigation::DIR_PRESS) {
        digitalWrite(_gdo0Pin, LOW);
        _rf.end();
        _showMenu();
        return;
      }
    }

    for (int i = 0; i < 50; i++) {
      uint32_t pw  = 5 + (micros() % 46);
      uint32_t gap = 5 + (micros() % 96);
      digitalWrite(_gdo0Pin, HIGH); delayMicroseconds(pw);
      digitalWrite(_gdo0Pin, LOW);  delayMicroseconds(gap);
    }
    yield();

    if (millis() - _jamStart > 500) {
      render();
      _jamStart = millis();
    }
    return;
  }

  if (_state == STATE_SEND_BROWSE) {
    if (!_holdFired && Uni.Nav->isPressed() && Uni.Nav->heldDuration() >= 1000) {
      _holdFired = true;
      uint8_t idx = _selectedIndex;
      if (idx < _browseCount && !_browseIsDir[idx])
        _showBrowseOptions(idx);

      Serial.println("Hold fired on index " + String(idx));
      return;
    }
  }
  if (_holdFired) {
    Uni.Nav->readDirection();
    _holdFired = false;
    Serial.println("Draining hold event");
    return;
  }

  ListScreen::onUpdate();
}

void SubGHzScreen::onRender() {
  if (_state == STATE_RECEIVING) {
    TFT_eSprite sp(&Uni.Lcd);
    sp.createSprite(bodyW(), bodyH());
    sp.fillSprite(TFT_BLACK);
    sp.setTextDatum(MC_DATUM);
    sp.setTextSize(1);

    if (_rf.isScanning()) {
      sp.drawString("Scanning...", bodyW() / 2, bodyH() / 2 - 20);
      char scanStr[32];
      snprintf(scanStr, sizeof(scanStr), "%.2f MHz  RSSI:%d", _rf.getScanFreq(), _rf.getScanRssi());
      sp.drawString(scanStr, bodyW() / 2, bodyH() / 2);
    } else {
      char freqStr[24];
      snprintf(freqStr, sizeof(freqStr), "%.2f MHz", _rf.getFrequency());
      sp.drawString(freqStr, bodyW() / 2, bodyH() / 2 - 20);
      sp.drawString("Waiting for signal...", bodyW() / 2, bodyH() / 2);
    }

    char countStr[24];
    if (_capturedCount >= kMaxCapture) {
      snprintf(countStr, sizeof(countStr), "Full: %d/%d", _capturedCount, kMaxCapture);
    } else {
      snprintf(countStr, sizeof(countStr), "Captured: %d", _capturedCount);
    }
    sp.drawString(countStr, bodyW() / 2, bodyH() / 2 + 16);

    sp.fillRect(0, bodyH() - 16, bodyW(), 16, Config.getThemeColor());
    sp.setTextColor(TFT_WHITE, Config.getThemeColor());
    #ifdef DEVICE_HAS_KEYBOARD
      sp.drawString("BACK: Stop", bodyW() / 2, bodyH() - 8, 1);
    #else
      sp.drawString("< Stop", bodyW() / 2, bodyH() - 8, 1);
    #endif

    sp.pushSprite(bodyX(), bodyY());
    sp.deleteSprite();
    return;
  }

  if (_state == STATE_JAMMING) {
    TFT_eSprite sp(&Uni.Lcd);
    sp.createSprite(bodyW(), bodyH());
    sp.fillSprite(TFT_BLACK);
    sp.setTextDatum(MC_DATUM);
    sp.setTextSize(1);

    char freqStr[16];
    snprintf(freqStr, sizeof(freqStr), "%.2f MHz", _rf.getFrequency());
    sp.drawString(freqStr, bodyW() / 2, bodyH() / 2 - 20);
    sp.drawString("Jamming...", bodyW() / 2, bodyH() / 2);

    #ifdef DEVICE_HAS_KEYBOARD
      sp.drawString("BACK: Stop", bodyW() / 2, bodyH() - 10);
    #else
      sp.fillRect(0, bodyH() - 16, bodyW(), 16, Config.getThemeColor());
      sp.setTextColor(TFT_WHITE, Config.getThemeColor());
      sp.drawString("< Stop", bodyW() / 2, bodyH() - 8, 1);
    #endif

    sp.pushSprite(bodyX(), bodyY());
    sp.deleteSprite();
    return;
  }

  ListScreen::onRender();
}

void SubGHzScreen::onBack() {
  if (_state == STATE_MENU) {
    _rf.end();
    Screen.setScreen(new ModuleMenuScreen());
  } else if (_state == STATE_RECEIVING) {
    _rf.end();
    if (_capturedCount > 0) {
      _showCapturedList();
    } else {
      _showMenu();
    }
  } else if (_state == STATE_CAPTURED_LIST) {
    _showMenu();
  } else if (_state == STATE_JAMMING) {
    digitalWrite(_gdo0Pin, LOW);
    _rf.end();
    _showMenu();
  } else if (_state == STATE_SEND_BROWSE) {
    if (_browsePath == kRootPath) {
      _showMenu();
    } else {
      int slash = _browsePath.lastIndexOf('/');
      if (slash > 0) _loadBrowseDir(_browsePath.substring(0, slash));
      else _showMenu();
    }
  }
}

void SubGHzScreen::onItemSelected(uint8_t index) {
  if (_state == STATE_MENU) {
    switch (index) {
      case 0: { // CS Pin
        int pin = InputNumberAction::popup("CC1101 CS Pin", 0, 48, _csPin);
        if (pin >= 0) {
          _csPin = (int8_t)pin;
          PinConfig.set(PIN_CONFIG_CC1101_CS, String(pin));
          PinConfig.save(Uni.Storage);
        }
        _updatePinSublabels();
        render();
        return;
      }
      case 1: { // GDO0 Pin
        int pin = InputNumberAction::popup("CC1101 GDO0 Pin", 0, 48, _gdo0Pin);
        if (pin >= 0) {
          _gdo0Pin = (int8_t)pin;
          PinConfig.set(PIN_CONFIG_CC1101_GDO0, String(pin));
          PinConfig.save(Uni.Storage);
        }
        _updatePinSublabels();
        render();
        return;
      }
      case 2: { // Receive
        if (_csPin < 0 || _gdo0Pin < 0) {
          ShowStatusAction::show("Set CS and GDO0 pins first");
          render();
          return;
        }
        if (!_rf.begin(Uni.Spi, _csPin, _gdo0Pin,
                       CC1101_SCK_PIN, CC1101_MISO_PIN, CC1101_MOSI_PIN)) {
          ShowStatusAction::show("CC1101 not found");
          render();
          return;
        }
        _capturedCount = 0;
        _state = STATE_RECEIVING;
        strcpy(_titleBuf, "Sub-GHz RX");
        render();
        break;
      }
      case 3: { // Send
        if (_csPin < 0) {
          ShowStatusAction::show("Set CS pin first");
          render();
          return;
        }
        _loadBrowseDir(kRootPath);
        break;
      }
      case 4: { // Jammer
        if (_csPin < 0 || _gdo0Pin < 0) {
          ShowStatusAction::show("Set CS and GDO0 pins first");
          render();
          return;
        }
        if (!_rf.begin(Uni.Spi, _csPin, _gdo0Pin,
                       CC1101_SCK_PIN, CC1101_MISO_PIN, CC1101_MOSI_PIN)) {
          ShowStatusAction::show("CC1101 not found");
          render();
          return;
        }
        _rf.startTx();
        _state = STATE_JAMMING;
        _jamStart = millis();
        strcpy(_titleBuf, "Sub-GHz Jam");
        render();
        break;
      }
      default:
        return;
    }
    return;
  }

  if (_state == STATE_CAPTURED_LIST) {
    if (index >= _capturedCount) return;

    if (_capturedSaved[index]) {
      ShowStatusAction::show("Already saved");
      render();
      return;
    }

    String name = InputTextAction::popup("Save As", _capturedTimes[index].c_str());
    if (name.length() == 0) {
      render();
      return;
    }
    _saveSignal(index, name);
    render();
    return;
  }

  if (_state == STATE_SEND_BROWSE) {
    if (index >= _browseCount) return;
    if (_browseIsDir[index]) {
      _loadBrowseDir(_browsePaths[index]);
      return;
    }
    _sendBrowseFile(index);
    return;
  }
}

// ── Browse helpers ────────────────────────────────────────────────────────

void SubGHzScreen::_sendBrowseFile(uint8_t index) {
  String content = Uni.Storage->readFile(_browsePaths[index].c_str());
  if (content.length() == 0) {
    ShowStatusAction::show("Failed to read file");
    render();
    return;
  }
  CC1101Util::Signal sig;
  if (!CC1101Util::loadFile(content, sig)) {
    ShowStatusAction::show("Invalid .sub file");
    render();
    return;
  }
  if (!_rf.begin(Uni.Spi, _csPin, _gdo0Pin,
                 CC1101_SCK_PIN, CC1101_MISO_PIN, CC1101_MOSI_PIN)) {
    ShowStatusAction::show("CC1101 not found");
    render();
    return;
  }
  ProgressView::show(("Sending " + _browseNames[index]).c_str(), 50);
  _rf.sendSignal(sig);
  _rf.end();
  ShowStatusAction::show(("Sent: " + _browseNames[index]).c_str(), 1000);
  render();
}

void SubGHzScreen::_showBrowseOptions(uint8_t index) {
  static constexpr InputSelectAction::Option fileOpts[] = {
    {"Send",   "send"},
    {"Rename", "rename"},
    {"Delete", "delete"},
  };
  const char* choice = InputSelectAction::popup("Options", fileOpts, 3, "send");
  if (!choice) { render(); return; }

  if (strcmp(choice, "send") == 0) {
    _sendBrowseFile(index);

  } else if (strcmp(choice, "rename") == 0) {
    String curName = _browseNames[index];
    if (curName.endsWith(".sub")) curName = curName.substring(0, curName.length() - 4);
    String newName = InputTextAction::popup("Rename", curName.c_str());
    if (newName.length() == 0) { render(); return; }
    String content = Uni.Storage->readFile(_browsePaths[index].c_str());
    String newPath = _makeUniquePath(newName);
    if (Uni.Storage->writeFile(newPath.c_str(), content.c_str())) {
      Uni.Storage->deleteFile(_browsePaths[index].c_str());
      ShowStatusAction::show("Renamed", 1000);
      _loadBrowseDir(_browsePath);
    } else {
      ShowStatusAction::show("Rename failed");
      render();
    }

  } else if (strcmp(choice, "delete") == 0) {
    if (Uni.Storage->deleteFile(_browsePaths[index].c_str())) {
      ShowStatusAction::show("Deleted", 1000);
      _loadBrowseDir(_browsePath);
    } else {
      ShowStatusAction::show("Delete failed");
      render();
    }
  }
}

// ── Menu ──────────────────────────────────────────────────────────────────

void SubGHzScreen::_showMenu() {
  _state = STATE_MENU;
  strcpy(_titleBuf, "Sub-GHz");
  _updatePinSublabels();
  setItems(_menuItems, kMenuCount);
}

void SubGHzScreen::_updatePinSublabels() {
  _csPinSub  = (_csPin   >= 0) ? ("GPIO " + String(_csPin))   : "Not set";
  _menuItems[0].sublabel = _csPinSub.c_str();
  _gdo0PinSub = (_gdo0Pin >= 0) ? ("GPIO " + String(_gdo0Pin)) : "Not set";
  _menuItems[1].sublabel = _gdo0PinSub.c_str();
}

// ── Captured List ──────────────────────────────────────────────────────────

void SubGHzScreen::_showCapturedList() {
  _state = STATE_CAPTURED_LIST;
  strcpy(_titleBuf, "Captured");
  for (uint8_t i = 0; i < _capturedCount; i++) {
    const CC1101Util::Signal& sig = _capturedSignals[i];
    if (!_capturedSaved[i] && sig.protocol == "RcSwitch") {
      char keyStr[24];
      snprintf(keyStr, sizeof(keyStr), "P%s 0x%llX",
               sig.preset.c_str(), (unsigned long long)sig.key);
      _capturedSubLabels[i] = String(keyStr);
    } else {
      _capturedSubLabels[i] = _capturedSaved[i] ? "Saved" : "Tap to save";
    }
    _capturedItems[i] = {_capturedTimes[i].c_str(), _capturedSubLabels[i].c_str()};
  }
  setItems(_capturedItems, _capturedCount);
}

void SubGHzScreen::_saveSignal(uint8_t index, const String& name) {
  if (!Uni.Storage || !Uni.Storage->isAvailable()) {
    ShowStatusAction::show("No storage");
    return;
  }
  Uni.Storage->makeDir(kRootPath);
  String path    = _makeUniquePath(name);
  String content = CC1101Util::saveToString(_capturedSignals[index]);
  if (Uni.Storage->writeFile(path.c_str(), content.c_str())) {
    _capturedSaved[index] = true;
    _capturedSubLabels[index] = "Saved";
    _capturedItems[index].sublabel = _capturedSubLabels[index].c_str();
    int lastSlash = path.lastIndexOf('/');
    String fname = (lastSlash >= 0) ? path.substring(lastSlash + 1) : path;
    ShowStatusAction::show(fname.c_str(), 1200);
  } else {
    ShowStatusAction::show("Save failed");
  }
}

String SubGHzScreen::_generateTimestampName() {
  uint32_t s = millis() / 1000;
  char buf[20];
  snprintf(buf, sizeof(buf), "rf_%05lu", (unsigned long)(s % 100000));
  return String(buf);
}

// ── File Browser ──────────────────────────────────────────────────────────

String SubGHzScreen::_makeUniquePath(const String& name) {
  String base = String(kRootPath) + "/" + name + ".sub";
  if (!Uni.Storage || !Uni.Storage->exists(base.c_str())) return base;
  for (int n = 2; n < 1000; n++) {
    String candidate = String(kRootPath) + "/" + name + "_(" + n + ").sub";
    if (!Uni.Storage->exists(candidate.c_str())) return candidate;
  }
  return base;
}

void SubGHzScreen::_loadBrowseDir(const String& path) {
  _browsePath  = path;
  _browseCount = 0;
  _state = STATE_SEND_BROWSE;

  int lastSlash = path.lastIndexOf('/');
  String folderName = (lastSlash >= 0) ? path.substring(lastSlash + 1) : path;
  snprintf(_titleBuf, sizeof(_titleBuf), "RF: %s", folderName.c_str());

  if (!Uni.Storage || !Uni.Storage->isAvailable()) {
    setItems(_browseItems, 0);
    return;
  }

  Uni.Storage->makeDir(path.c_str());

  IStorage::DirEntry entries[kMaxBrowse];
  uint8_t total = Uni.Storage->listDir(path.c_str(), entries, kMaxBrowse);

  // Sort: dirs first, then alphabetical
  for (uint8_t i = 0; i < total; i++) {
    for (uint8_t j = i + 1; j < total; j++) {
      bool swap = false;
      if (entries[j].isDir && !entries[i].isDir) swap = true;
      else if (entries[i].isDir == entries[j].isDir &&
               strcasecmp(entries[j].name.c_str(), entries[i].name.c_str()) < 0) swap = true;
      if (swap) {
        IStorage::DirEntry tmp = entries[i];
        entries[i] = entries[j];
        entries[j] = tmp;
      }
    }
  }

  for (uint8_t i = 0; i < total && _browseCount < kMaxBrowse; i++) {
    const String& name = entries[i].name;
    bool isDir = entries[i].isDir;
    if (!isDir && !name.endsWith(".sub")) continue;

    _browseNames[_browseCount]  = name;
    _browsePaths[_browseCount]  = path + "/" + name;
    _browseIsDir[_browseCount]  = isDir;
    _browseItems[_browseCount]  = {_browseNames[_browseCount].c_str(),
                                   isDir ? ">" : nullptr};
    _browseCount++;
  }

  setItems(_browseItems, _browseCount);
}