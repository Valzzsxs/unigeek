#pragma once

#include "ui/templates/BaseScreen.h"

class GameWordleScreen : public BaseScreen
{
public:
  enum Language : uint8_t { LANG_EN = 0, LANG_ID = 1 };

  GameWordleScreen(Language lang = LANG_EN) : _language(lang) {}

  const char* title()         override { return _language == LANG_ID ? "Wordle ID" : "Wordle EN"; }
  bool inhibitPowerSave()     override { return _state == STATE_PLAY; }
  bool inhibitPowerOff()      override { return true; }

  void onInit()   override;
  void onUpdate() override;
  void onRender() override;

private:
  enum State { STATE_MENU, STATE_PLAY, STATE_RESULT } _state = STATE_MENU;

  static constexpr uint8_t kWordLen    = 5;
  static constexpr uint8_t kMaxHistory = 10;
  static constexpr uint8_t kAlphaLen   = 26;

  Language _language;

  // Menu
  int8_t  _menuIdx    = 0;
  uint8_t _difficulty = 0;   // 0=Easy 1=Medium 2=Hard
  bool    _useCommon  = true;

  // Game state
  char     _target[kWordLen]                       = {};
  char     _current[kWordLen]                      = {};
  int8_t   _cursor   = 0;
  int8_t   _cycleIdx = 0;

  char     _history[kMaxHistory][kWordLen]         = {};
  uint8_t  _histSize    = 0;
  uint8_t  _totalInputs = 0;
  uint8_t  _alphabetUsed[kAlphaLen]               = {};  // 0=not tried, 1=tried
  bool     _win         = false;
  uint32_t _startMs     = 0;

  // Helpers
  const char* _diffStr()    const;
  int         _maxAttempts() const;
  int         _colorGuess(uint8_t idx, char c) const;

  void _initGame();
  void _pushHistory();
  void _submitGuess();
  void _backChar();
  void _confirmChar();
  void _handleKeyInput(char c);

  void _renderMenu();
  void _renderPlay();
  void _renderResult();
};