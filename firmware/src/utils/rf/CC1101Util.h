//
// CC1101 Sub-GHz Utility — send/receive raw RF signals
// Reference: Bruce firmware (https://github.com/pr3y/Bruce)
//

#pragma once
#include <Arduino.h>
#include <functional>

class CC1101Util {
public:
  static constexpr float DEFAULT_FREQ = 433.92;
  static constexpr uint16_t MAX_RAW_LEN = 2048;

  struct Signal {
    float frequency;    // MHz
    String preset;      // modulation preset name or number
    String protocol;    // "RAW" for raw signals
    String rawData;     // space-separated signed durations (+ = HIGH, - = LOW) in µs
  };

  // Initialize CC1101 on shared SPI with CS and GDO0 pins
  bool begin(SPIClass* spi, int8_t csPin, int8_t gdo0Pin);
  void end();

  // Set frequency in MHz (280-928, valid sub-bands only)
  bool setFrequency(float mhz);
  float getFrequency() const { return _freq; }

  // Check if CC1101 is connected
  bool isConnected();

  // Receive — captures raw signal via GDO0, blocks until signal or timeout
  // Returns true if a signal was captured
  bool receive(Signal& out, uint32_t timeoutMs = 20000,
               std::function<bool()> cancelCb = nullptr);

  // Send a signal
  void sendSignal(const Signal& sig);

  // File I/O — Bruce SubGhz .sub format
  static bool loadFile(const String& content, Signal& out);
  static String saveToString(const Signal& sig);

  // Display helper
  static String signalLabel(const Signal& sig);

private:
  int8_t _csPin = -1;
  int8_t _gdo0Pin = -1;
  float _freq = DEFAULT_FREQ;
  bool _initialized = false;

  void _initTx();
  void _initRx();
};
