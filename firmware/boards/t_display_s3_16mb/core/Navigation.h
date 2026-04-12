#pragma once
#include "core/INavigation.h"

class NavigationImpl : public INavigation {
public:
    void begin() override {
        pinMode(BTN_UP, INPUT_PULLUP);
        pinMode(BTN_B,  INPUT_PULLUP);
    }

    void update() override {
        bool btnUp   = (digitalRead(BTN_UP) == LOW);
        bool btnDown = (digitalRead(BTN_B)  == LOW);
        uint32_t now = millis();

        // Synthetic releases
        if (_syntheticUp) {
            _syntheticUp = false;
            updateState(DIR_NONE);
            return;
        }
        if (_syntheticDown) {
            _syntheticDown = false;
            updateState(DIR_NONE);
            return;
        }

        // BTN_UP logic (Single = UP, Double = BACK, Hold = LEFT)
        if (btnUp && !_btnUpWasDown) {
            _btnUpWasDown = true;
            if (_waitDblClickUp && (now - _lastUpTime) <= DBL_CLICK_MS) {
                _waitDblClickUp = false;
                _heldDir = DIR_BACK;
                updateState(DIR_BACK);
            } else {
                _waitDblClickUp = true;
                _lastUpTime = now;
                _heldDir = DIR_NONE;
                updateState(DIR_NONE);
            }
        } else if (btnUp) {
            if (_waitDblClickUp) {
                if ((now - _lastUpTime) > DBL_CLICK_MS) {
                    _waitDblClickUp = false;
                    _heldDir = DIR_LEFT;
                    updateState(DIR_LEFT);
                } else {
                    updateState(DIR_NONE);
                }
            } else {
                updateState(_heldDir);
            }
        } else if (_btnUpWasDown) {
            _btnUpWasDown = false;
            if (_heldDir == DIR_LEFT || _heldDir == DIR_BACK) {
                _heldDir = DIR_NONE;
            }

            if (_waitDblClickUp) {
                if ((now - _lastUpTime) > DBL_CLICK_MS) {
                    _waitDblClickUp = false;
                    _syntheticUp = true;
                    updateState(DIR_UP);
                    return;
                }
            }
        } else if (!btnUp && _waitDblClickUp) {
            if ((now - _lastUpTime) > DBL_CLICK_MS) {
                _waitDblClickUp = false;
                _syntheticUp = true;
                updateState(DIR_UP);
                return;
            } else {
                updateState(DIR_NONE);
            }
        }

        // BTN_B logic (Single = DOWN, Double = PRESS, Hold = RIGHT)
        if (btnDown && !_btnBWasDown) {
            _btnBWasDown = true;
            if (_waitDblClickDown && (now - _lastDownTime) <= DBL_CLICK_MS) {
                _waitDblClickDown = false;
                _heldDir = DIR_PRESS;
                updateState(DIR_PRESS);
            } else {
                _waitDblClickDown = true;
                _lastDownTime = now;
                _heldDir = DIR_NONE;
                updateState(DIR_NONE);
            }
        } else if (btnDown) {
            if (_waitDblClickDown) {
                if ((now - _lastDownTime) > DBL_CLICK_MS) {
                    _waitDblClickDown = false;
                    _heldDir = DIR_RIGHT;
                    updateState(DIR_RIGHT);
                } else {
                    updateState(DIR_NONE);
                }
            } else {
                updateState(_heldDir);
            }
        } else if (_btnBWasDown) {
            _btnBWasDown = false;
            if (_heldDir == DIR_RIGHT || _heldDir == DIR_PRESS) {
                _heldDir = DIR_NONE;
            }

            if (_waitDblClickDown) {
                if ((now - _lastDownTime) > DBL_CLICK_MS) {
                    _waitDblClickDown = false;
                    _syntheticDown = true;
                    updateState(DIR_DOWN);
                    return;
                }
            }
        } else if (!btnDown && _waitDblClickDown) {
            if ((now - _lastDownTime) > DBL_CLICK_MS) {
                _waitDblClickDown = false;
                _syntheticDown = true;
                updateState(DIR_DOWN);
                return;
            } else {
                updateState(DIR_NONE);
            }
        }

        if (!btnUp && !btnDown && !_waitDblClickUp && !_waitDblClickDown) {
            updateState(DIR_NONE);
        }
    }

private:
    static constexpr uint32_t DBL_CLICK_MS = 250;

    uint32_t _lastUpTime = 0;
    bool _btnUpWasDown = false;
    bool _waitDblClickUp = false;
    bool _syntheticUp = false;

    uint32_t _lastDownTime = 0;
    bool _btnBWasDown = false;
    bool _waitDblClickDown = false;
    bool _syntheticDown = false;

    Direction _heldDir = DIR_NONE;
};
