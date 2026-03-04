#include "WebFileManagerScreen.h"
#include "core/ScreenManager.h"
#include "core/Device.h"
#include "core/ConfigManager.h"
#include "screens/wifi/network/NetworkMenuScreen.h"
#include "ui/actions/InputTextAction.h"
#include "ui/actions/ShowStatusAction.h"
#include "ui/actions/ShowProgressAction.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

void WebFileManagerScreen::onInit() {
  if (!Uni.Storage || !Uni.Storage->isAvailable()) {
    ShowStatusAction::show("No storage available");
    Screen.setScreen(new NetworkMenuScreen());
    return;
  }
  _showMenu();
}

void WebFileManagerScreen::onBack() {
  if (_state == STATE_RUNNING) {
    _stop();
  } else {
    Screen.setScreen(new NetworkMenuScreen());
  }
}

void WebFileManagerScreen::onItemSelected(uint8_t index) {
  switch (index) {
    case 0:
      _downloadWebPage();
      break;
    case 1: {
      String pw = InputTextAction::popup("Web Password",
        Config.get(APP_CONFIG_WEB_PASSWORD, APP_CONFIG_WEB_PASSWORD_DEFAULT).c_str());
      if (pw.length() > 0) {
        Config.set(APP_CONFIG_WEB_PASSWORD, pw);
        Config.save(Uni.Storage);
      }
      _showMenu();
      break;
    }
    case 2:
      _start();
      break;
  }
}

// ── private ────────────────────────────────────────────

void WebFileManagerScreen::_showMenu() {
  _state = STATE_MENU;

  _versionSub = "Not installed";
  if (Uni.Storage) {
    String sha = Uni.Storage->readFile(
      (String(WebFileManager::WEB_PATH) + "/version.txt").c_str());
    if (sha.length() == 40) _versionSub = "v" + sha.substring(0, 7);
    else if (sha.length() > 0) _versionSub = sha;
  }
  _menuItems[0] = {"Download Web Page", _versionSub.c_str()};

  _passwordSub = Config.get(APP_CONFIG_WEB_PASSWORD, APP_CONFIG_WEB_PASSWORD_DEFAULT);
  _menuItems[1] = {"Password", _passwordSub.c_str()};
  _menuItems[2] = {"Start"};
  setItems(_menuItems, 3);
}

void WebFileManagerScreen::_start() {
  ShowStatusAction::show("Starting server...", 0);
  if (!_server.begin()) {
    ShowStatusAction::show(_server.getError().c_str());
    _showMenu();
    return;
  }
  _state = STATE_RUNNING;
  setItems(nullptr, 0);

  String ipUrl   = "http://" + WiFi.localIP().toString() + "/";
  String mdnsUrl = "http://unigeek.local/";
  int cx     = bodyX() + bodyW() / 2;
  int midY   = Uni.Lcd.height() / 2;

  Uni.Lcd.setTextFont(1);
  Uni.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  Uni.Lcd.drawCentreString(ipUrl,   cx, midY - 10, 1);
  Uni.Lcd.drawCentreString(mdnsUrl, cx, midY + 4,  1);

  Uni.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  Uni.Lcd.drawCentreString("BACK to stop", cx, bodyY() + bodyH() - 14, 1);
}

void WebFileManagerScreen::_stop() {
  _server.end();
  _showMenu();
}

void WebFileManagerScreen::_downloadWebPage() {
  if (!Uni.Storage || !Uni.Storage->isAvailable()) {
    ShowStatusAction::show("No storage available");
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    ShowStatusAction::show("WiFi not connected");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  // ── Fetch latest commit SHA ────────────────────────────
  ShowProgressAction::show("Fetching version...", 0);
  String sha = "";
  http.begin(client,
    "https://api.github.com/repos/lshaf/puteros-file-manager/git/ref/heads/main");
  http.addHeader("User-Agent", "ESP32");
  if (http.GET() == HTTP_CODE_OK) {
    String body = http.getString();
    int idx = body.indexOf("\"sha\":");
    if (idx >= 0) {
      int start = idx + 6;
      while (start < (int)body.length() && (body[start] == ' ' || body[start] == '"')) start++;
      if (start + 40 <= (int)body.length()) sha = body.substring(start, start + 40);
    }
  }
  http.end();

  // ── Check if already up to date ───────────────────────
  if (sha.length() == 40) {
    const String base = WebFileManager::WEB_PATH;
    String local = Uni.Storage->readFile((base + "/version.txt").c_str());
    bool filesExist = Uni.Storage->exists((base + "/index.htm").c_str()) &&
                      Uni.Storage->exists((base + "/index.css").c_str()) &&
                      Uni.Storage->exists((base + "/index.js").c_str());
    if (local == sha && filesExist) {
      ShowStatusAction::show(("Already up to date\nv" + sha.substring(0, 7)).c_str(), 1500);
      _showMenu();
      return;
    }
  }

  // ── Download files ─────────────────────────────────────
  struct FileEntry { const char* url; const char* path; };
  static constexpr FileEntry kFiles[] = {
    {"https://raw.githubusercontent.com/lshaf/puteros-file-manager/main/interface/index.html",
     "/index.htm"},
    {"https://raw.githubusercontent.com/lshaf/puteros-file-manager/main/interface/index.css",
     "/index.css"},
    {"https://raw.githubusercontent.com/lshaf/puteros-file-manager/main/interface/index.js",
     "/index.js"},
  };
  static constexpr uint8_t kFileCount = sizeof(kFiles) / sizeof(kFiles[0]);

  const String base = WebFileManager::WEB_PATH;
  Uni.Storage->makeDir(base.c_str());

  for (uint8_t i = 0; i < kFileCount; i++) {
    ShowProgressAction::show("Downloading...", 10 + i * 28);

    http.begin(client, kFiles[i].url);
    http.addHeader("User-Agent", "ESP32");
    http.addHeader("Cache-Control", "no-cache");
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
      http.end();
      ShowStatusAction::show(("Download failed (" + String(code) + ")").c_str());
      return;
    }

    String content = http.getString();
    http.end();

    String path = base + kFiles[i].path;
    if (!Uni.Storage->writeFile(path.c_str(), content.c_str())) {
      ShowStatusAction::show("Failed to save file");
      return;
    }
  }

  // ── Write version.txt ──────────────────────────────────
  ShowProgressAction::show("Saving version...", 95);
  String ver = sha.length() > 0 ? sha : "installed";
  Uni.Storage->writeFile((base + "/version.txt").c_str(), ver.c_str());

  ShowProgressAction::show("Done!", 100);
  String msg = sha.length() >= 7
    ? ("Done! v" + sha.substring(0, 7))
    : "Done!";
  ShowStatusAction::show(msg.c_str(), 1500);
  _showMenu();
}
