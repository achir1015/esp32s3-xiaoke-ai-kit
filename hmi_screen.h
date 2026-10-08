// ============================================================================
//  7 吋串口屏（淘晶馳 TJC8048X570_011R_Y，800x480）：小柯的大臉
//  接線：螢幕 RX ← GPIO17、螢幕 TX → GPIO18、GND 共地、螢幕 5V 接 5V 2A 電源
//  用螢幕內建繪圖指令（cls/fill/cirs）作畫，不需要先下載 HMI 專案
//  依賴主程式的 uiState / speakExpr / speakLevel / Expr 列舉
// ============================================================================
#pragma once
#include <vector>

#ifndef HMI_ENABLE
#define HMI_ENABLE 1
#endif
#ifndef PIN_HMI_TX
#define PIN_HMI_TX 17
#endif
#ifndef PIN_HMI_RX
#define PIN_HMI_RX 18
#endif

#if HMI_ENABLE

namespace hmi {

HardwareSerial &port = Serial1;
bool ready = false;

// 螢幕內建 MV：page1 = 全螢幕 v0、page2 = 置中 400x240 v0；vid = 視頻資源 ID
struct Mv { const char *ytId; int page; int vid; int sec; const char *title; };
const Mv MVS[] = {
  {"N8uhpLWKAgs", 1, 0, 210, "五常行處見春風"},
  {"lmGEJO3eyWU", 2, 1, 302, "日照藍天情"},
};
constexpr int MV_COUNT = sizeof(MVS) / sizeof(MVS[0]);
volatile int mvReq = -1;

int mvIndex(const String &ytId) {
  for (int i = 0; i < MV_COUNT; i++) if (ytId == MVS[i].ytId) return i;
  return -1;
}
void mvStart(int i) { mvReq = i; }

// 7 吋屏音量跟著小柯的音量走；螢幕功放較小聲，乘 2 補償（小柯 50% = 螢幕滿格）
int screenVol() { return constrain((int)volumePct * 2, 0, 100); }

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}
constexpr uint16_t BG = 0, WHITE = 0xFFFF;
constexpr uint16_t CYAN = rgb(0, 230, 255), PINK = rgb(255, 90, 160), RED = rgb(255, 40, 40);
constexpr uint16_t GREEN = rgb(40, 230, 90), YELLOW = rgb(255, 210, 0), BLUE = rgb(60, 140, 255);
constexpr uint16_t BLUSH = rgb(255, 120, 150);
constexpr uint16_t ORANGE = rgb(255, 150, 40), GRAY = rgb(140, 150, 160), LINE = rgb(0, 90, 110);
constexpr int HB_BAR = 72, CAP_Y = 380;                 // 上方時間列 / 下方字幕區
constexpr int LX = 260, RX = 540, EY = 185, ER = 65;   // 眼睛
constexpr int MX = 400, MY = 315;                      // 嘴巴
constexpr int F24 = 0, F48 = 1;                       // HMI 專案字型 ID（tw24 中文、tw48n 只有時鐘數字）

void cmd(const char *fmt, ...) {
  char buf[96];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  port.print(buf);
  port.write(0xFF); port.write(0xFF); port.write(0xFF);
}

bool probe(uint32_t baud) {
  port.updateBaudRate(baud);
  while (port.available()) port.read();
  port.write(0xFF); port.write(0xFF); port.write(0xFF);
  cmd("connect");
  String r;
  uint32_t t0 = millis();
  while (millis() - t0 < 400) {
    while (port.available()) r += (char)port.read();
    if (r.indexOf("comok") >= 0) return true;
    delay(10);
  }
  return false;
}

void fill(int x, int y, int w, int h, uint16_t c);
void disc(int x, int y, int r, uint16_t c);

// 文字：xstr x,y,w,h,字型,字色,底色,水平對齊,垂直對齊,填底,"字串"（xcen 0 左 1 中 2 右）
void text(int x, int y, int w, int h, int font, uint16_t pco, int xcen, const String &s) {
  String t = s;
  t.replace("\"", "”");
  t.replace("\r", "");
  t.replace("\n", " ");
  String c = "xstr " + String(x) + "," + y + "," + w + "," + h + "," + font + "," + pco + "," + BG + "," +
             xcen + ",1,1,\"" + t + "\"";
  port.print(c);
  port.write(0xFF); port.write(0xFF); port.write(0xFF);
}

// 依顯示寬度斷行（中文字算 2 格、英數 1 格；tw24 每格 12px）
std::vector<String> wrapText(const String &s, int maxUnits) {
  std::vector<String> lines;
  String cur;
  int units = 0;
  for (size_t i = 0; i < s.length();) {
    uint8_t c = s[i];
    int len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : 4;
    String ch = s.substring(i, i + len);
    i += len;
    if (ch == "\n") { lines.push_back(cur); cur = ""; units = 0; continue; }
    int u = len == 1 ? 1 : 2;
    if (units + u > maxUnits) { lines.push_back(cur); cur = ""; units = 0; }
    cur += ch;
    units += u;
  }
  if (cur.length()) lines.push_back(cur);
  return lines;
}

// 上方時間列（仿小柯主畫面：日期星期、農曆、大時間、音量、Wi-Fi 燈）
void drawTopBar(bool full) {
  struct tm t;
  bool ok = getLocalTime(&t, 0);
  static const char *WD[] = {"週日", "週一", "週二", "週三", "週四", "週五", "週六"};
  if (full) {
    fill(0, 0, 800, HB_BAR, BG);
    fill(0, HB_BAR - 2, 800, 2, LINE);
    char d[8] = "--/--";
    if (ok) strftime(d, sizeof(d), "%m/%d", &t);
    text(12, 4, 240, 32, F24, CYAN, 0, String(d) + " " + (ok ? WD[t.tm_wday] : ""));
    String l = ok ? lunarString(t) : String("");
    text(12, 36, 240, 32, F24, ORANGE, 0, l.startsWith("閏") || l.isEmpty() ? l : "農曆" + l);
    text(640, 4, 100, 32, F24, YELLOW, 2, "音量");
    text(640, 36, 100, 32, F24, YELLOW, 2, String((int)volumePct) + "%");
  }
  char tm[8] = "--:--";
  if (ok) strftime(tm, sizeof(tm), "%H:%M", &t);
  if (ok && t.tm_sec % 2) tm[2] = ' ';                  // 冒號每秒閃爍
  text(300, 8, 200, 56, F48, WHITE, 1, tm);
  disc(770, 20, 8, WiFi.status() == WL_CONNECTED ? GREEN : RED);
}

// 下方字幕：標題一行 + 內文兩行一頁
void drawCaption(const String &title, const std::vector<String> &lines, int page) {
  fill(0, CAP_Y, 800, 480 - CAP_Y, BG);
  fill(20, CAP_Y, 760, 1, LINE);
  if (title.length()) text(20, CAP_Y + 4, 760, 30, F24, YELLOW, 0, title);
  for (int i = 0; i < 2; i++) {
    size_t n = page * 2 + i;
    if (n < lines.size()) text(20, CAP_Y + 36 + i * 32, 760, 30, F24, WHITE, 0, lines[n]);
  }
}

// 螢幕開機是 9600；連上後暫時提速到 115200（螢幕斷電後自動回 9600）
bool begin() {
  port.setTxBufferSize(4096);
  port.begin(115200, SERIAL_8N1, PIN_HMI_RX, PIN_HMI_TX);
  if (!probe(115200)) {
    if (!probe(9600)) return false;
    cmd("baud=115200");
    port.flush();
    delay(100);
    port.updateBaudRate(115200);
    delay(50);
  }
  cmd("bkcmd=0");
  return true;
}

// ---- 繪圖小工具 ----
void fill(int x, int y, int w, int h, uint16_t c) { cmd("fill %d,%d,%d,%d,%u", x, y, w, h, c); }
void disc(int x, int y, int r, uint16_t c) { cmd("cirs %d,%d,%d,%u", x, y, r, c); }

void arc(int cx, int cy, float rad, float a0, float a1, int r, uint16_t c, float sy = 1.0f, int n = 18) {
  for (int i = 0; i <= n; i++) {
    float a = radians(a0 + (a1 - a0) * i / n);
    disc(cx + rad * cosf(a), cy - rad * sy * sinf(a), r, c);
  }
}

void line(int x0, int y0, int x1, int y1, int r, uint16_t c, int n = 14) {
  for (int i = 0; i <= n; i++) disc(x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, r, c);
}

void heart(int cx, int cy, int s, uint16_t c) {
  disc(cx - s / 2, cy - s / 4, s * 55 / 100, c);
  disc(cx + s / 2, cy - s / 4, s * 55 / 100, c);
  int h = s * 13 / 10;
  for (int i = 0; i < h; i += 2) {                     // 下半部倒三角
    int hw = s * 105 / 100 * (h - i) / h;
    fill(cx - hw, cy - s / 20 + i, hw * 2, 2, c);
  }
}

void eyesRound(uint16_t c = CYAN, int r = ER, int dx = 0, int dy = 0) {
  for (int x : {LX, RX}) {
    disc(x, EY, r, c);
    disc(x + dx - r * 3 / 10, EY + dy - r * 3 / 10, r / 4, WHITE);
  }
}
void eyeCaret(int x, uint16_t c = CYAN) { arc(x, EY + 30, ER * 0.9f, 20, 160, 12, c); }
void mouthSmile(int w = 90, uint16_t c = CYAN) { arc(MX, MY - 60, w, 200, 340, 9, c, 0.7f); }
void mouthFrown(uint16_t c = CYAN) { arc(MX, MY + 40, 70, 30, 150, 9, c, 0.6f); }
void mouthFlat(uint16_t c = CYAN) { line(MX - 60, MY, MX + 60, MY, 8, c, 12); }

// 說話嘴型：0=閉（微笑）1=半開 2=全開
void mouthTalk(int level, uint16_t c) {
  fill(MX - 130, MY - 60, 260, 120, BG);
  if (level == 0) { mouthSmile(70, c); return; }
  int r = level == 1 ? 30 : 48;
  disc(MX, MY, r, c);
  disc(MX, MY - r / 4, r * 3 / 4, BG);
}

// ---- 整張臉 ----
void drawFace(int st, int ex) {
  fill(0, HB_BAR, 800, CAP_Y - HB_BAR, BG);
  if (st == UI_LISTEN) ex = EX_LISTEN;
  else if (st == UI_THINK) ex = EX_THINK;
  else if (st != UI_SPEAK) ex = EX_NORMAL;

  switch (ex) {
    case EX_HAPPY: eyeCaret(LX); eyeCaret(RX); mouthSmile(110); break;
    case EX_WINK:
      disc(LX, EY, ER, CYAN); disc(LX - 20, EY - 20, 18, WHITE);
      eyeCaret(RX); mouthSmile(90); break;
    case EX_LOVE: heart(LX, EY, 70, PINK); heart(RX, EY, 70, PINK); mouthSmile(90, PINK); break;
    case EX_SURPRISE:
      for (int x : {LX, RX}) { disc(x, EY, ER + 10, CYAN); disc(x, EY, ER - 15, BG); disc(x, EY, 25, CYAN); }
      disc(MX, MY, 40, CYAN); disc(MX, MY, 26, BG); break;
    case EX_SAD:
      eyesRound(BLUE, 60);
      fill(LX - 90, EY - 90, 180, 70, BG); fill(RX - 90, EY - 90, 180, 70, BG);
      line(LX - 80, EY - 50, LX + 50, EY - 80, 7, BLUE); line(RX - 50, EY - 80, RX + 80, EY - 50, 7, BLUE);
      disc(RX + 50, EY + 90, 14, BLUE);
      mouthFrown(BLUE); break;
    case EX_ANGRY:
      eyesRound(RED, 60);
      fill(LX - 90, EY - 90, 180, 75, BG); fill(RX - 90, EY - 90, 180, 75, BG);
      line(LX - 80, EY - 85, LX + 60, EY - 35, 9, RED); line(RX - 60, EY - 35, RX + 80, EY - 85, 9, RED);
      mouthFlat(RED); break;
    case EX_SHY:
      eyesRound(CYAN, 55);
      disc(LX - 20, EY + 110, 40, BLUSH); disc(RX + 20, EY + 110, 40, BLUSH);
      mouthSmile(50); break;
    case EX_BORED:
      eyesRound(CYAN, 60);
      fill(LX - 80, EY - 80, 160, 95, BG); fill(RX - 80, EY - 80, 160, 95, BG);
      mouthFlat(); break;
    case EX_SLEEPY:
      line(LX - 60, EY + 10, LX + 60, EY + 10, 9, CYAN); line(RX - 60, EY + 10, RX + 60, EY + 10, 9, CYAN);
      mouthFlat(); break;
    case EX_LISTEN: {
      eyesRound(GREEN);
      const int hs[] = {40, 80, 120, 80, 40};
      for (int i = 0; i < 5; i++) {
        fill(40 + i * 22, EY - hs[i] / 2, 12, hs[i], GREEN);
        fill(660 + i * 22, EY - hs[i] / 2, 12, hs[i], GREEN);
      }
      disc(MX, MY, 22, GREEN); break;
    }
    case EX_THINK:
      eyesRound(YELLOW, ER, 25, -25);
      for (int i = 0; i < 3; i++) disc(MX - 50 + i * 50, MY, 14, YELLOW);
      break;
    case EX_DEAD:
      for (int x : {LX, RX}) { line(x - 50, EY - 50, x + 50, EY + 50, 9, CYAN); line(x - 50, EY + 50, x + 50, EY - 50, 9, CYAN); }
      mouthFlat(); break;
    default: eyesRound(); mouthSmile(70); break;
  }
}

// 唱歌：開心臉 + 音符（之後改成播放螢幕內建 MV）
void drawSong() {
  fill(0, HB_BAR, 800, CAP_Y - HB_BAR, BG);
  eyeCaret(LX); eyeCaret(RX);
  mouthTalk(2, PINK);
  const int nx[] = {90, 690}, ny[] = {300, 270};
  const uint16_t nc[] = {PINK, YELLOW};
  for (int i = 0; i < 2; i++) {
    disc(nx[i], ny[i], 28, nc[i]);
    fill(nx[i] + 20, ny[i] - 130, 9, 130, nc[i]);
    fill(nx[i] + 20, ny[i] - 130, 45, 14, nc[i]);
  }
}

void blink() {
  for (int x : {LX, RX}) { disc(x, EY, ER + 2, BG); line(x - 60, EY, x + 60, EY, 8, CYAN, 10); }
  delay(150);
  for (int x : {LX, RX}) fill(x - 75, EY - 15, 150, 30, BG);
  eyesRound();
}

#include "hmi_panel.h"   // 觸控快速查詢面板

bool talkMouth(int ex) { return ex != EX_SURPRISE && ex != EX_SAD && ex != EX_ANGRY; }

void task(void *) {
  for (int i = 0; !(ready = begin()); i++) {           // 螢幕可能比小柯晚開機
    if (i == 0) Serial.println("[7吋屏] 找不到螢幕，持續重試…");
    delay(3000);
  }
  Serial.println("[7吋屏] 已連線 @ 115200");
  cmd("cls %u", BG);
  cmd("sendxy=1");                                        // 觸控座標回傳給小柯
  drawTopBar(true);
  int lastSt = -1, lastEx = -1, lastMouth = -1, lastSec = -1, lastMin = -1, lastVol = -1, page = 0;
  uint32_t lastBlink = millis(), lastMouthAt = 0, pageAt = 0;
  String capTitle, capText;
  bool forceCap = false;
  std::vector<String> capLines;
  bool mvPlaying = false;
  for (;;) {
    int st = uiState, ex = speakExpr;
    int touchX = 0, touchY = 0;
    bool tap = pollTouch(touchX, touchY);
    if (mvPlaying && tap) {                                 // MV 播放中：左 1/3 小聲、右 1/3 大聲、中間停止
      if (touchX < 267 || touchX > 533) {
        changeVolume(touchX < 267 ? -10 : 10);
        text(250, 20, 300, 40, F24, YELLOW, 1, "音量 " + String((int)volumePct) + "%");
      } else wantStopSong = true;
    }
    if (mode != M_FACE && !mvPlaying) {                     // 查詢面板
      bool leave = (tap && panelTap(touchX, touchY)) || millis() - panelAt > 60000 ||
                   (st != lastSt && (st == UI_SPEAK || st == UI_SONG));   // 小柯開口說話或唱歌就回到大臉
      if (leave) {
        mode = M_FACE;
        btns.clear();
        fill(0, HB_BAR, 800, 480 - HB_BAR, BG);
        lastSt = -1;
        forceCap = true;
      } else {
        struct tm t;
        if (getLocalTime(&t, 0) && t.tm_sec != lastSec) {
          bool full = t.tm_min != lastMin || volumePct != lastVol;
          lastSec = t.tm_sec; lastMin = t.tm_min; lastVol = volumePct;
          drawTopBar(full);
        }
        delay(30);
        continue;
      }
    } else if (tap && st != UI_SONG && !mvPlaying) {         // 點大臉 → 打開選單
      panelAt = millis();
      openMode(M_MENU);
      continue;
    }
    if (mvReq >= 0 && st == UI_SONG) {                    // 切到 MV 頁播放
      const Mv &m = MVS[mvReq];
      cmd("page %d", m.page);
      if (m.page == 2) {                                  // 小畫面：周圍塗黑、下方顯示歌名
        delay(50);
        cmd("cls %u", BG);
        text(0, 380, 800, 50, F24, YELLOW, 1, String("♪ ") + m.title + " ♪");
        text(0, 430, 800, 40, F24, GRAY, 1, "作詞作曲：" SONG_COMPOSER);
      }
      cmd("volume=%d", screenVol());
      cmd("v0.vid=%d", m.vid);
      cmd("v0.en=1");
      mvReq = -1;
      mvPlaying = true;
    }
    if (mvPlaying) {
      static int mvVol = -1;
      if (screenVol() != mvVol) { mvVol = screenVol(); cmd("volume=%d", mvVol); }   // MV 中按音量鍵即時生效
      if (st == UI_SONG) { delay(50); continue; }
      cmd("v0.en=0");                                 // 唱完（或按鍵中斷）：回到小柯主畫面
      cmd("page 0");
      delay(100);
      cmd("sendxy=1");
      cmd("cls %u", BG);
      drawTopBar(true);
      mvPlaying = false;
      lastSt = -1;
      forceCap = true;
    }
    struct tm t;
    if (getLocalTime(&t, 0) && t.tm_sec != lastSec) {      // 時間列：每秒更新時間，換分鐘或音量時整列重畫
      bool full = t.tm_min != lastMin || volumePct != lastVol;
      lastSec = t.tm_sec; lastMin = t.tm_min; lastVol = volumePct;
      drawTopBar(full);
    }
    xSemaphoreTake(uiLock, portMAX_DELAY);
    String ti = uiTitle, tx = uiText;
    xSemaphoreGive(uiLock);
    if (st == UI_SONG) tx = "";                            // 唱歌時 uiText 是 YouTube id
    if (st == UI_IDLE && ti.isEmpty() && tx.isEmpty()) { ti = ""; tx = "隨時可以直接跟我說話喔！"; }
    if (forceCap || ti != capTitle || tx != capText) {
      forceCap = false;
      capTitle = ti; capText = tx;
      capLines = wrapText(tx, 62);
      page = 0; pageAt = millis();
      drawCaption(capTitle, capLines, page);
    } else if (capLines.size() > 2 && millis() - pageAt > 4500) {   // 長字幕自動翻頁
      page = (page + 1) % ((capLines.size() + 1) / 2);
      pageAt = millis();
      drawCaption(capTitle, capLines, page);
    }
    if (st != lastSt || (st == UI_SPEAK && ex != lastEx)) {
      if (st == UI_SONG) drawSong();
      else drawFace(st, ex);
      lastSt = st; lastEx = ex; lastMouth = -1;
      lastBlink = millis();
    }
    if (st == UI_SPEAK && talkMouth(ex) && millis() - lastMouthAt > 90) {   // 嘴巴跟著 TTS 音量開合
      int lv = speakLevel < 800 ? 0 : speakLevel < 3000 ? 1 : 2;
      if (lv != lastMouth) {
        mouthTalk(lv, ex == EX_LOVE ? PINK : CYAN);
        lastMouth = lv;
        lastMouthAt = millis();
      }
    }
    if ((st == UI_IDLE || st == UI_HELP || st == UI_MSG) && millis() - lastBlink > 4000) {
      blink();
      lastBlink = millis();
    }
    delay(30);
  }
}

void start() { xTaskCreatePinnedToCore(task, "hmi", 16384, nullptr, 1, nullptr, 0); }

}  // namespace hmi

#endif
