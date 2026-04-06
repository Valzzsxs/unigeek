#pragma once

#include "ui/templates/ListScreen.h"
#include "ui/views/LogView.h"
#include <NimBLEDevice.h>

class WhisperPairScreen : public ListScreen {
public:
  const char* title() override { return "WhisperPair"; }
  bool inhibitPowerOff() override { return true; }

  ~WhisperPairScreen() override;
  void onInit() override;
  void onUpdate() override;
  void onItemSelected(uint8_t index) override;
  void onRender() override;
  void onBack() override;

private:
  enum State {
    STATE_SCAN,
    STATE_LIST,
    STATE_TESTING,
    STATE_RESULT,
  } _state = STATE_SCAN;

  static constexpr uint8_t kMaxDevices = 20;

  NimBLEScan*       _bleScan      = nullptr;
  NimBLEScanResults _scanResults;
  bool              _testPending  = false;
  bool              _isVulnerable = false;
  bool              _hasFpDevices = false;
  uint8_t           _devCount     = 0;
  uint8_t           _selScanIdx   = 0;
  uint8_t           _scanIdx[kMaxDevices];

  String   _devLabel[kMaxDevices];
  String   _devSub[kMaxDevices];
  ListItem _devItems[kMaxDevices];

  LogView _log;

  void _startScan();
  void _showList();
  void _runTest();
  bool _doKbpTest(NimBLEAdvertisedDevice dev);
  void _deriveAesKey(const uint8_t* peerKey, bool hasKey, uint8_t* aesKey);
  void _encryptAes(const uint8_t* plaintext, uint8_t* ciphertext, const uint8_t* key);
};