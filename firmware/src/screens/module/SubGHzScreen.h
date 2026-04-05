//
// Sub-GHz (CC1101) Screen
//

#pragma once

#include "ui/templates/ListScreen.h"
#include "utils/rf/CC1101Util.h"

class SubGHzScreen : public ListScreen {
public:
  const char* title() override { return _titleBuf; }
  bool inhibitPowerSave() override { return _state == STATE_RECEIVING; }
  bool inhibitPowerOff() override { return _state == STATE_RECEIVING || _state == STATE_JAMMING; }

  void onInit() override;
  void onUpdate() override;
  void onRender() override;
  void onBack() override;
  void onItemSelected(uint8_t index) override;

private:
  enum State {
    STATE_MENU,
    STATE_RECEIVING,
    STATE_CAPTURED_LIST,
    STATE_SEND_BROWSE,
    STATE_JAMMING,
  } _state = STATE_MENU;

  CC1101Util _rf;
  int8_t _csPin   = -1;
  int8_t _gdo0Pin = -1;
  char _titleBuf[32] = "Sub-GHz";

  // Menu (5 items)
  static constexpr uint8_t kMenuCount = 5;
  ListItem _menuItems[kMenuCount] = {
    {"CC1101 CS Pin"},
    {"CC1101 GDO0 Pin"},
    {"Receive"},
    {"Send"},
    {"Jammer"},
  };
  String _csPinSub;
  String _gdo0PinSub;
  void _showMenu();
  void _updatePinSublabels();

  // Receive — captured signal buffer (not auto-saved)
  static constexpr uint8_t kMaxCapture = 10;
  CC1101Util::Signal _capturedSignals[kMaxCapture];
  String _capturedTimes[kMaxCapture];
  bool   _capturedSaved[kMaxCapture];
  String _capturedSubLabels[kMaxCapture];
  ListItem _capturedItems[kMaxCapture];
  uint8_t _capturedCount = 0;
  void _showCapturedList();
  void _saveSignal(uint8_t index, const String& name);
  String _generateTimestampName();

  // Jammer state
  uint32_t _jamStart = 0;

  // Send — file browser
  static constexpr uint8_t kMaxBrowse = 30;
  static constexpr const char* kRootPath = "/unigeek/rf";
  String _browsePath;
  String _browseNames[kMaxBrowse];
  String _browsePaths[kMaxBrowse];
  bool   _browseIsDir[kMaxBrowse];
  ListItem _browseItems[kMaxBrowse];
  uint8_t _browseCount = 0;
  bool    _holdFired = false;
  void _loadBrowseDir(const String& path);
  void _sendBrowseFile(uint8_t index);
  void _showBrowseOptions(uint8_t index);
  String _makeUniquePath(const String& name);
};