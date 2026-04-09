#pragma once
#include <map>
#include "IStorage.h"

class AchievementStorage
{
public:
  static AchievementStorage& getInstance() {
    static AchievementStorage instance;
    return instance;
  }

  void load(IStorage* storage) {
    if (!storage || !storage->isAvailable()) return;
    String content = storage->readFile("/unigeek/achievements");
    if (content.length() == 0) return;
    _data.clear();
    int start = 0;
    while (start < (int)content.length()) {
      int nl = content.indexOf('\n', start);
      if (nl < 0) nl = content.length();
      String line = content.substring(start, nl);
      line.trim();
      int sep = line.indexOf('=');
      if (sep > 0) _data[line.substring(0, sep)] = line.substring(sep + 1);
      start = nl + 1;
    }
  }

  void save(IStorage* storage) {
    if (!storage || !storage->isAvailable()) return;
    storage->makeDir("/unigeek");
    String content;
    for (auto& kv : _data) content += kv.first + "=" + kv.second + "\n";
    storage->writeFile("/unigeek/achievements", content.c_str());
  }

  String get(const String& key, const String& def = "") const {
    auto it = _data.find(key);
    return it != _data.end() ? it->second : def;
  }

  void set(const String& key, const String& value) {
    _data[key] = value;
  }

  AchievementStorage(const AchievementStorage&)            = delete;
  AchievementStorage& operator=(const AchievementStorage&) = delete;

private:
  AchievementStorage() = default;
  std::map<String, String> _data;
};

#define AchStore AchievementStorage::getInstance()