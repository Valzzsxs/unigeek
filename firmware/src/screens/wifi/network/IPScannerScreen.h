#pragma once
#include "ui/templates/ListScreen.h"

class IPScannerScreen : public ListScreen {
public:
  IPScannerScreen();

  const char* title()         override { return "IP Scanner"; }
  bool inhibitPowerSave()     override { return _state == STATE_SCANNING_IP || _state == STATE_SCANNING_PORT; }

  void onInit() override;
  void onBack() override;
  void onItemSelected(uint8_t index) override;

private:
  enum State {
    STATE_CONFIGURATION,
    STATE_SCANNING_IP,
    STATE_SCANNING_PORT,
    STATE_RESULT_IP,
    STATE_RESULT_PORT
  };

  State   _state   = STATE_CONFIGURATION;
  int     _startIp = 1;
  int     _endIp   = 254;
  String  _startIpSub;
  String  _endIpSub;

  static constexpr uint8_t MAX_FOUND = 64;
  static constexpr uint8_t MAX_PORTS = 80;

  struct FoundIP  { char ip[16]; char hostname[50]; };
  struct OpenPort { char label[24]; char service[20]; };

  FoundIP  _foundIPs[MAX_FOUND];
  uint8_t  _foundCount = 0;
  ListItem _foundItems[MAX_FOUND];

  OpenPort _openPorts[MAX_PORTS];
  uint8_t  _openCount = 0;
  ListItem _openItems[MAX_PORTS];

  ListItem _configItems[3];

  void _showConfiguration();
  void _scanIP();
  void _scanPort(const char* ip);
};
