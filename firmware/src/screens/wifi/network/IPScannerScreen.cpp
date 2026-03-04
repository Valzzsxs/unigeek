#include "IPScannerScreen.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include "core/ScreenManager.h"
#include "screens/wifi/network/NetworkMenuScreen.h"
#include "ui/actions/InputNumberAction.h"
#include "ui/actions/ShowProgressAction.h"

// ── DNS PTR resolver ───────────────────────────────────

static bool parseDnsPtr(const uint8_t* resp, int len, int ancount, int i, char* out, size_t outLen) {
  for (int a = 0; a < ancount && i < len; a++) {
    if (i < len && (resp[i] & 0xC0) == 0xC0) i += 2;
    else { while (i < len && resp[i]) i += resp[i] + 1; if (i < len) i++; }

    if (i + 10 > len) break;
    uint16_t type  = ((uint16_t)resp[i]   << 8) | resp[i + 1];
    uint16_t rdlen = ((uint16_t)resp[i+8] << 8) | resp[i + 9];
    i += 10;

    if (type == 0x000C) {
      size_t outIdx = 0;
      int j = i;
      bool first = true;
      int hops = 0;
      while (j < len && resp[j] != 0 && outIdx < outLen - 1 && hops < 8) {
        if ((resp[j] & 0xC0) == 0xC0) {
          if (j + 1 >= len) break;
          j = ((resp[j] & 0x3F) << 8) | resp[j + 1];
          hops++;
          continue;
        }
        uint8_t llen = resp[j++];
        if (!first && outIdx < outLen - 1) out[outIdx++] = '.';
        first = false;
        for (uint8_t k = 0; k < llen && outIdx < outLen - 1; k++)
          out[outIdx++] = (char)resp[j++];
      }
      out[outIdx] = '\0';
      return outIdx > 0;
    }
    i += rdlen;
  }
  return false;
}

static bool resolveHostname(const char* ip, char* out, size_t outLen) {
  IPAddress dns = WiFi.dnsIP();
  if (dns == IPAddress(0, 0, 0, 0)) return false;

  int a, b, c, d;
  if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) return false;

  uint8_t pkt[128];
  memset(pkt, 0, sizeof(pkt));
  uint8_t* p = pkt;
  *p++ = 0xAB; *p++ = 0xCD;
  *p++ = 0x01; *p++ = 0x00;
  *p++ = 0x00; *p++ = 0x01;
  p += 6;

  char qname[50];
  snprintf(qname, sizeof(qname), "%d.%d.%d.%d.in-addr.arpa", d, c, b, a);
  char tmp[50];
  strncpy(tmp, qname, sizeof(tmp) - 1);
  char* tok = strtok(tmp, ".");
  while (tok) {
    uint8_t llen = (uint8_t)strlen(tok);
    *p++ = llen;
    memcpy(p, tok, llen);
    p += llen;
    tok = strtok(nullptr, ".");
  }
  *p++ = 0x00;
  *p++ = 0x00; *p++ = 0x0C;
  *p++ = 0x00; *p++ = 0x01;

  WiFiUDP udp;
  udp.begin(0);
  udp.beginPacket(dns, 53);
  udp.write(pkt, (size_t)(p - pkt));
  if (!udp.endPacket()) { udp.stop(); return false; }

  uint32_t start = millis();
  while (millis() - start < 500) {
    int len = udp.parsePacket();
    if (len > 0) {
      uint8_t resp[256];
      len = udp.read(resp, min((int)sizeof(resp), len));
      udp.stop();
      if (len < 12 || resp[0] != 0xAB || resp[1] != 0xCD || !(resp[2] & 0x80)) return false;
      uint16_t ancount = ((uint16_t)resp[6] << 8) | resp[7];
      if (ancount == 0) return false;
      int i = 12;
      while (i < len && resp[i]) {
        if ((resp[i] & 0xC0) == 0xC0) { i += 2; break; }
        i += resp[i] + 1;
      }
      if (i < len && resp[i] == 0) i++;
      i += 4;
      return parseDnsPtr(resp, len, ancount, i, out, outLen);
    }
    delay(5);
  }
  udp.stop();
  return false;
}

// ── screen methods ─────────────────────────────────────

IPScannerScreen::IPScannerScreen() {
  memset(_foundIPs,    0, sizeof(_foundIPs));
  memset(_foundItems,  0, sizeof(_foundItems));
  memset(_openPorts,   0, sizeof(_openPorts));
  memset(_openItems,   0, sizeof(_openItems));
  memset(_configItems, 0, sizeof(_configItems));
}

void IPScannerScreen::onInit() {
  _showConfiguration();
}

void IPScannerScreen::onBack() {
  switch (_state) {
    case STATE_RESULT_PORT:
      _state = STATE_RESULT_IP;
      setItems(_foundItems, (_foundCount == 0) ? 1 : _foundCount);
      break;
    case STATE_RESULT_IP:
      _showConfiguration();
      break;
    default:
      Screen.setScreen(new NetworkMenuScreen());
      break;
  }
}

void IPScannerScreen::onItemSelected(uint8_t index) {
  if (_state == STATE_CONFIGURATION) {
    switch (index) {
      case 0:
        _startIp = InputNumberAction::popup("Start IP", 1, _endIp, _startIp);
        _showConfiguration();
        break;
      case 1:
        _endIp = InputNumberAction::popup("End IP", _startIp, 254, _endIp);
        _showConfiguration();
        break;
      case 2:
        _scanIP();
        break;
    }
  } else if (_state == STATE_RESULT_IP) {
    if (_foundCount == 0 || _foundIPs[index].ip[0] == '\0') {
      _showConfiguration();
    } else {
      _scanPort(_foundIPs[index].ip);
    }
  }
}

// ── private ────────────────────────────────────────────

void IPScannerScreen::_showConfiguration() {
  _state = STATE_CONFIGURATION;
  _startIpSub = String(_startIp);
  _endIpSub   = String(_endIp);
  _configItems[0] = {"Start IP", _startIpSub.c_str()};
  _configItems[1] = {"End IP",   _endIpSub.c_str()};
  _configItems[2] = {"Start Scan"};
  setItems(_configItems, 3);
}

void IPScannerScreen::_scanIP() {
  _state = STATE_SCANNING_IP;

  memset(_foundIPs,   0, sizeof(_foundIPs));
  memset(_foundItems, 0, sizeof(_foundItems));
  _foundCount = 0;

  IPAddress localIP = WiFi.localIP();
  if (localIP[0] == 0 && localIP[1] == 0 && localIP[2] == 0 && localIP[3] == 0) {
    _foundItems[0] = {"No devices found"};
    _state = STATE_RESULT_IP;
    setItems(_foundItems, 1);
    return;
  }

  // Find the WiFi STA netif (first ETHARP-capable, UP interface)
  struct netif* netif = nullptr;
  for (struct netif* ni = netif_list; ni != nullptr; ni = ni->next) {
    if ((ni->flags & NETIF_FLAG_UP) && (ni->flags & NETIF_FLAG_ETHARP)) {
      netif = ni;
      break;
    }
  }
  if (!netif) {
    _foundItems[0] = {"ARP unavailable"};
    _state = STATE_RESULT_IP;
    setItems(_foundItems, 1);
    return;
  }

  char baseIp[16];
  snprintf(baseIp, sizeof(baseIp), "%d.%d.%d.", localIP[0], localIP[1], localIP[2]);

  int total    = _endIp - _startIp + 1;
  uint8_t lastPct = 255;

  // Phase 1: blast ARP requests (0–60%)
  for (int i = _startIp; i <= _endIp; i++) {
    uint8_t pct = (uint8_t)((i - _startIp) * 60 / total);
    if (pct != lastPct) {
      ShowProgressAction::show("ARP scanning...", pct);
      lastPct = pct;
    }

    char ipStr[16];
    snprintf(ipStr, sizeof(ipStr), "%s%d", baseIp, i);
    ip4_addr_t target;
    ipaddr_aton(ipStr, (ip_addr_t*)&target);
    etharp_request(netif, &target);
    delay(10);
  }

  // Brief wait for late ARP replies
  ShowProgressAction::show("ARP scanning...", 62);
  delay(300);

  // Phase 2: read ARP table + resolve hostnames (65–100%)
  lastPct = 255;
  for (int i = _startIp; i <= _endIp && _foundCount < MAX_FOUND; i++) {
    uint8_t pct = 65 + (uint8_t)((i - _startIp) * 35 / total);
    if (pct != lastPct) {
      ShowProgressAction::show("Resolving...", pct);
      lastPct = pct;
    }

    if (i == localIP[3]) continue;

    char ipStr[16];
    snprintf(ipStr, sizeof(ipStr), "%s%d", baseIp, i);
    ip4_addr_t target;
    ipaddr_aton(ipStr, (ip_addr_t*)&target);

    struct eth_addr*     eth_ret  = nullptr;
    const ip4_addr_t*    ip_ret   = nullptr;
    if (etharp_find_addr(nullptr, &target, &eth_ret, &ip_ret) >= 0) {
      strncpy(_foundIPs[_foundCount].ip, ipStr, sizeof(_foundIPs[0].ip) - 1);

      if (!resolveHostname(ipStr, _foundIPs[_foundCount].hostname,
                           sizeof(_foundIPs[0].hostname)) && eth_ret) {
        // Fallback: show MAC address
        snprintf(_foundIPs[_foundCount].hostname, sizeof(_foundIPs[0].hostname),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 eth_ret->addr[0], eth_ret->addr[1], eth_ret->addr[2],
                 eth_ret->addr[3], eth_ret->addr[4], eth_ret->addr[5]);
      }

      _foundItems[_foundCount] = {_foundIPs[_foundCount].ip, _foundIPs[_foundCount].hostname};
      _foundCount++;
    }
  }

  ShowProgressAction::show("ARP scanning...", 100);

  if (_foundCount == 0) {
    _foundItems[0] = {"No devices found"};
    _state = STATE_RESULT_IP;
    setItems(_foundItems, 1);
    return;
  }

  _state = STATE_RESULT_IP;
  setItems(_foundItems, _foundCount);
}

void IPScannerScreen::_scanPort(const char* ip) {
  _state = STATE_SCANNING_PORT;

  struct PortEntry { int port; const char* service; };
  static constexpr PortEntry kPorts[] = {
    {20,    "FTP Data"},       {21,    "FTP"},            {22,    "SSH"},
    {23,    "Telnet"},         {25,    "SMTP"},           {53,    "DNS"},
    {67,    "DHCP"},           {68,    "DHCP"},           {69,    "TFTP"},
    {80,    "HTTP"},           {110,   "POP3"},           {123,   "NTP"},
    {135,   "MS-RPC"},        {137,   "NetBIOS"},        {139,   "NetBIOS"},
    {143,   "IMAP"},          {161,   "SNMP"},           {162,   "SNMP Trap"},
    {389,   "LDAP"},          {443,   "HTTPS"},          {445,   "SMB"},
    {465,   "SMTPS"},         {514,   "Syslog"},         {554,   "RTSP"},
    {587,   "SMTP Submit"},   {631,   "IPP"},            {636,   "LDAPS"},
    {873,   "rsync"},         {993,   "IMAPS"},          {995,   "POP3S"},
    {1194,  "OpenVPN"},       {1433,  "MSSQL"},          {1521,  "Oracle"},
    {1723,  "PPTP"},          {2049,  "NFS"},            {2181,  "Zookeeper"},
    {2375,  "Docker"},        {2376,  "DockerTLS"},      {3306,  "MySQL"},
    {3389,  "RDP"},           {3690,  "SVN"},            {5000,  "UPnP"},
    {5432,  "PostgreSQL"},    {5555,  "ADB"},            {5900,  "VNC"},
    {5985,  "WinRM HTTP"},    {5986,  "WinRM HTTPS"},    {6379,  "Redis"},
    {8000,  "HTTP Alt"},      {8080,  "HTTP Proxy"},     {8443,  "HTTPS Alt"},
    {8888,  "HTTP Alt"},      {9000,  "SonarQube"},      {9100,  "JetDirect"},
    {9200,  "Elasticsearch"}, {10000, "Webmin"},         {11211, "Memcached"},
    {27017, "MongoDB"},
  };
  static constexpr uint8_t kPortCount = sizeof(kPorts) / sizeof(kPorts[0]);

  memset(_openPorts, 0, sizeof(_openPorts));
  memset(_openItems, 0, sizeof(_openItems));
  _openCount = 0;

  uint8_t lastPct = 255;

  for (uint8_t i = 0; i < kPortCount && _openCount < MAX_PORTS; i++) {
    uint8_t pct = (uint8_t)(i * 100 / kPortCount);
    if (pct != lastPct) {
      ShowProgressAction::show("Port scanning...", pct);
      lastPct = pct;
    }

    yield();
    WiFiClient client;
    if (client.connect(ip, kPorts[i].port, 300)) {
      client.stop();
      snprintf(_openPorts[_openCount].label,  sizeof(_openPorts[0].label),   "%s:%d", ip, kPorts[i].port);
      strncpy(_openPorts[_openCount].service, kPorts[i].service,             sizeof(_openPorts[0].service) - 1);
      _openItems[_openCount] = {_openPorts[_openCount].label, _openPorts[_openCount].service};
      _openCount++;
    }
  }

  ShowProgressAction::show("Port scanning...", 100);

  if (_openCount == 0) {
    _openItems[0] = {"No ports open"};
    _state = STATE_RESULT_PORT;
    setItems(_openItems, 1);
    return;
  }

  _state = STATE_RESULT_PORT;
  setItems(_openItems, _openCount);
}
