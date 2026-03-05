#include "WifiDeauthDetectorScreen.h"
#include "core/Device.h"
#include "core/ScreenManager.h"
#include "screens/wifi/WifiMenuScreen.h"

#include <vector>
#include <cstring>

// ── Static definitions ────────────────────────────────────────────────────

std::unordered_map<
  WifiDeauthDetectorScreen::MacAddr,
  std::string,
  WifiDeauthDetectorScreen::MacHash,
  WifiDeauthDetectorScreen::MacEqual
> WifiDeauthDetectorScreen::_ssidMap = {};

std::unordered_map<
  WifiDeauthDetectorScreen::MacAddr,
  WifiDeauthDetectorScreen::DeauthEntry,
  WifiDeauthDetectorScreen::MacHash,
  WifiDeauthDetectorScreen::MacEqual
> WifiDeauthDetectorScreen::_deauthMap = {};

portMUX_TYPE  WifiDeauthDetectorScreen::_deauthLock  = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE  WifiDeauthDetectorScreen::_ssidLock     = portMUX_INITIALIZER_UNLOCKED;
volatile bool WifiDeauthDetectorScreen::_newDetection = false;

// ── Destructor ────────────────────────────────────────────────────────────

WifiDeauthDetectorScreen::~WifiDeauthDetectorScreen()
{
  esp_wifi_set_promiscuous_rx_cb(nullptr);
  esp_wifi_set_promiscuous(false);
  _deauthMap.clear();
  _ssidMap.clear();
}

// ── Lifecycle ─────────────────────────────────────────────────────────────

void WifiDeauthDetectorScreen::onInit()
{
  _state      = STATE_EMPTY;
  _channel    = 1;
  _itemCount  = 0;
  _newDetection = false;
  _deauthMap.clear();
  _ssidMap.clear();

  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&WifiDeauthDetectorScreen::_promiscuousCb);

  render();
}

void WifiDeauthDetectorScreen::onUpdate()
{
  if (_newDetection) {
    _newDetection = false;
    if (Uni.Speaker) Uni.Speaker->playNotification();
  }

  if (millis() - _lastUpdate >= 1000) {
    _lastUpdate = millis();
    _channel = (_channel % 13) + 1;
    esp_wifi_set_channel(_channel, WIFI_SECOND_CHAN_NONE);
    _refresh();
  }

  if (_state == STATE_LISTED) {
    ListScreen::onUpdate();
  } else {
    if (Uni.Nav->wasPressed() && Uni.Nav->readDirection() == INavigation::DIR_BACK) {
      onBack();
    }
  }
}

void WifiDeauthDetectorScreen::onRender()
{
  if (_state == STATE_LISTED) {
    ListScreen::onRender();
    return;
  }

  TFT_eSprite sp(&Uni.Lcd);
  sp.createSprite(bodyW(), bodyH());
  sp.fillSprite(TFT_BLACK);
  sp.setTextDatum(MC_DATUM);
  sp.setTextColor(TFT_DARKGREY, TFT_BLACK);
  sp.drawString("Waiting for deauth packets...", bodyW() / 2, bodyH() / 2);
  sp.pushSprite(bodyX(), bodyY());
  sp.deleteSprite();
}

void WifiDeauthDetectorScreen::onBack()
{
  Screen.setScreen(new WifiMenuScreen());
}

// ── Private ───────────────────────────────────────────────────────────────

void WifiDeauthDetectorScreen::_refresh()
{
  const unsigned long now = millis();

  // Collect stale keys, then erase — never modify map while iterating
  std::vector<MacAddr> toErase;
  for (auto& kv : _deauthMap) {
    if (now - kv.second.timestamp > WINDOW_MS) toErase.push_back(kv.first);
  }
  for (auto& k : toErase) _deauthMap.erase(k);

  int newCount = 0;
  for (auto& kv : _deauthMap) {
    if (newCount >= MAX_ITEMS) break;

    const MacAddr&    mac = kv.first;
    const DeauthEntry& e  = kv.second;

    if (!e.ssid.empty()) {
      snprintf(_labels[newCount], sizeof(_labels[newCount]),
               "%s (%d)", e.ssid.c_str(), e.counter);
    } else {
      snprintf(_labels[newCount], sizeof(_labels[newCount]),
               "%02X:%02X:%02X:%02X:%02X:%02X (%d)",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], e.counter);
    }

    const unsigned long secsAgo = (now - e.timestamp) / 1000UL;
    if (secsAgo == 0) {
      snprintf(_sublabels[newCount], sizeof(_sublabels[newCount]), "just now");
    } else {
      snprintf(_sublabels[newCount], sizeof(_sublabels[newCount]), "%lus ago", secsAgo);
    }

    _items[newCount] = {_labels[newCount], _sublabels[newCount]};
    newCount++;
  }

  if (newCount == 0) {
    if (_state != STATE_EMPTY) {
      _state     = STATE_EMPTY;
      _itemCount = 0;
      render();
    }
    return;
  }

  const bool countChanged = (newCount != _itemCount);
  _itemCount = newCount;
  _state     = STATE_LISTED;

  if (countChanged) {
    setItems(_items, _itemCount);
  } else {
    render();
  }
}

// ── Promiscuous callback (IRAM_ATTR — must be in .cpp) ────────────────────

void IRAM_ATTR WifiDeauthDetectorScreen::_promiscuousCb(void* buf, wifi_promiscuous_pkt_type_t type)
{
  if (type != WIFI_PKT_MGMT || buf == nullptr) return;

  const auto     pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  const uint8_t* pay = pkt->payload;
  const size_t   len = pkt->rx_ctrl.sig_len;

  if (len < 4) return;

  const uint8_t fc_sub = (pay[0] >> 4) & 0x0F;
  const uint8_t fc_type = (pay[0] >> 2) & 0x03;
  if (fc_type != 0) return;

  // Deauth frame (subtype 0xC)
  if (fc_sub == 0xC && len >= 16) {
    MacAddr mac{};
    memcpy(mac.data(), pay + 10, 6);  // addr2 = transmitter

    const unsigned long now = millis();

    portENTER_CRITICAL(&_deauthLock);
    auto it = _deauthMap.find(mac);
    if (it == _deauthMap.end()) {
      DeauthEntry e{};
      e.timestamp = now;
      e.counter   = 1;
      auto ssidIt = _ssidMap.find(mac);
      if (ssidIt != _ssidMap.end()) e.ssid = ssidIt->second;
      _deauthMap.emplace(mac, e);
      _newDetection = true;
    } else {
      ++it->second.counter;
      it->second.timestamp = now;
      if (it->second.ssid.empty()) {
        auto ssidIt = _ssidMap.find(mac);
        if (ssidIt != _ssidMap.end()) it->second.ssid = ssidIt->second;
      }
    }
    portEXIT_CRITICAL(&_deauthLock);
  }

  // Beacon (8) or Probe Response (5) — passively build SSID→MAC map
  if ((fc_sub == 8 || fc_sub == 5) && len >= 36) {
    MacAddr bssid{};
    memcpy(bssid.data(), pay + 16, 6);  // addr3

    size_t pos = 36;  // 24-byte header + 12 fixed params
    while (pos + 2 <= len) {
      const uint8_t id   = pay[pos];
      const uint8_t elen = pay[pos + 1];
      if (pos + 2 + elen > len) break;
      if (id == 0 && elen > 0) {
        portENTER_CRITICAL(&_ssidLock);
        if (_ssidMap.find(bssid) == _ssidMap.end()) {
          _ssidMap.emplace(bssid,
            std::string(reinterpret_cast<const char*>(pay + pos + 2), elen));
        }
        portEXIT_CRITICAL(&_ssidLock);
        break;
      }
      pos += 2 + elen;
    }
  }
}