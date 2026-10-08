// ============================================================================
//  7 吋屏「快速查詢面板」：點大臉 → 選單 → 我的班表 / 今日工作流程 / 工作站工具 QR / 唱首歌
//  由 hmi_screen.h 在 namespace hmi 內 include；觸控用 sendxy=1 回傳的座標判斷
//  資料：Firebase Firestore REST（公開讀取，免金鑰），與 AI_Voice_Robot 小柯相同來源
// ============================================================================

#ifndef FIREBASE_PROJECT
#define FIREBASE_PROJECT "monthlyschedule-a5dea"        // 月班表管理系統
#endif
#ifndef TIMELINE_PROJECT
#define TIMELINE_PROJECT "project-timeline-d2188"       // 專案進度時間軸
#endif
#ifndef SCHEDULE_MY_NAME
#define SCHEDULE_MY_NAME "吳玉柱"
#endif

enum Mode { M_FACE, M_MENU, M_SCHED, M_FLOWS, M_TASKS, M_TOOLS, M_QR };
Mode mode = M_FACE;
uint32_t panelAt = 0;                                   // 最後一次觸控（60 秒沒動作回到大臉）
volatile bool wantSong = false, wantStopSong = false;   // 交給主迴圈處理

constexpr uint16_t PANEL = rgb(18, 28, 40), PANEL_HI = rgb(10, 60, 90);
constexpr int B_BACK = 99, B_PREV = 98, B_NEXT = 97;

struct Btn { int x, y, w, h, id; };
std::vector<Btn> btns;

// ---- 繪圖小工具 ----
void textOn(int x, int y, int w, int h, int font, uint16_t pco, uint16_t bco, int xcen, const String &s) {
  String t = s;
  t.replace("\"", "”");
  String c = "xstr " + String(x) + "," + y + "," + w + "," + h + "," + font + "," + pco + "," + bco + "," +
             xcen + ",1,1,\"" + t + "\"";
  port.print(c);
  port.write(0xFF); port.write(0xFF); port.write(0xFF);
}

void frame(int x, int y, int w, int h, uint16_t c) {
  cmd("draw %d,%d,%d,%d,%u", x, y, x + w - 1, y + h - 1, c);
  cmd("draw %d,%d,%d,%d,%u", x + 1, y + 1, x + w - 2, y + h - 2, c);
}

void button(int x, int y, int w, int h, uint16_t bg, uint16_t fg, const String &label, int id) {
  fill(x, y, w, h, bg);
  textOn(x + 4, y + 2, w - 8, h - 4, F24, fg, bg, 1, label);
  btns.push_back({x, y, w, h, id});
}

void panelBase(const String &title) {
  btns.clear();
  fill(0, HB_BAR, 800, 480 - HB_BAR, BG);
  text(16, HB_BAR + 8, 560, 36, F24, YELLOW, 0, title);
  button(680, HB_BAR + 6, 108, 40, rgb(80, 90, 100), WHITE, "返回", B_BACK);
}

void busy(const String &msg) { text(0, 250, 800, 40, F24, GRAY, 1, msg); }

// ---- Firestore ----
String firestoreGet(const char *project, const String &path) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.begin(client, String("https://firestore.googleapis.com/v1/projects/") + project +
                         "/databases/(default)/documents/" + path);
  int code = http.GET();
  String body = code == 200 ? http.getString() : String("");
  if (code != 200) Serial.printf("[7吋屏] Firestore %s：%d\n", path.c_str(), code);
  http.end();
  return body;
}

// ---- 我的班表 ----
int schedY = 0, schedM = 0;
String schedCodes[32], schedLabel;
bool schedOk = false;

bool loadMyMonth(int y, int m) {
  static const char *CATS[] = {"assistant", "nurse"}, *LABELS[] = {"病房助理", "護理師"};
  for (int i = 0; i < 32; i++) schedCodes[i] = "";
  for (int c = 0; c < 2; c++) {
    char doc[40];
    snprintf(doc, sizeof(doc), "schedules/%s-%04d-%02d", CATS[c], y, m);
    String body = firestoreGet(FIREBASE_PROJECT, doc);
    if (body.isEmpty()) continue;
    JsonDocument filter;
    JsonObject f = filter["fields"]["employees"]["arrayValue"]["values"][0]["mapValue"]["fields"].to<JsonObject>();
    f["name"]["stringValue"] = true;
    f["shifts"]["mapValue"]["fields"] = true;
    JsonDocument d;
    if (deserializeJson(d, body, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(24))) continue;
    for (JsonObject e : d["fields"]["employees"]["arrayValue"]["values"].as<JsonArray>()) {
      JsonObject ef = e["mapValue"]["fields"];
      if (String(ef["name"]["stringValue"] | "") != SCHEDULE_MY_NAME) continue;
      JsonObject sh = ef["shifts"]["mapValue"]["fields"];
      for (int dd = 1; dd <= 31; dd++) schedCodes[dd] = sh[String(dd)]["stringValue"] | "";
      schedLabel = LABELS[c];
      return true;
    }
  }
  return false;
}

uint16_t shiftColor(const String &c) {
  if (c.indexOf("OFF") >= 0 || c.indexOf("休") >= 0 || c.indexOf("國") >= 0) return GREEN;
  if (c.indexOf("例") >= 0) return GRAY;
  if (c.indexOf("N") >= 0 || c.indexOf("夜8") >= 0) return BLUE;
  if (c.indexOf("E") >= 0) return ORANGE;
  if (c.indexOf("D") >= 0 || c.indexOf("白值") >= 0) return YELLOW;
  return WHITE;
}

void drawSchedule() {
  panelBase(String(schedY) + "年" + schedM + "月 我的班表");
  button(452, HB_BAR + 6, 100, 40, PANEL_HI, WHITE, "上個月", B_PREV);
  button(562, HB_BAR + 6, 100, 40, PANEL_HI, WHITE, "下個月", B_NEXT);
  busy("讀取班表中…");
  schedOk = loadMyMonth(schedY, schedM);
  fill(0, 200, 800, 100, BG);
  if (!schedOk) { busy("找不到 " SCHEDULE_MY_NAME " 這個月的班表"); return; }
  text(16, HB_BAR + 8, 430, 36, F24, YELLOW, 0, String(schedY) + "年" + schedM + "月 " + schedLabel + "班表");

  static const char *W[] = {"日", "一", "二", "三", "四", "五", "六"};
  const int X0 = 22, Y0 = 158, CW = 108, CH = 50;
  for (int i = 0; i < 7; i++) text(X0 + i * CW, 128, CW, 28, F24, i == 0 || i == 6 ? ORANGE : GRAY, 1, W[i]);
  int first = (daysSince2000(schedY, schedM, 1) + 6) % 7;
  int days = daysSince2000(schedM == 12 ? schedY + 1 : schedY, schedM == 12 ? 1 : schedM + 1, 1) -
             daysSince2000(schedY, schedM, 1);
  struct tm t;
  int today = getLocalTime(&t, 0) && t.tm_year + 1900 == schedY && t.tm_mon + 1 == schedM ? t.tm_mday : 0;
  for (int dd = 1; dd <= days; dd++) {
    int idx = first + dd - 1, x = X0 + (idx % 7) * CW, y = Y0 + (idx / 7) * CH;
    uint16_t bg = dd == today ? PANEL_HI : PANEL;
    fill(x + 2, y + 2, CW - 4, CH - 4, bg);
    textOn(x + 6, y + 4, 34, 24, F24, GRAY, bg, 0, String(dd));
    const String &c = schedCodes[dd];
    if (c.length()) textOn(x + 30, y + 16, CW - 36, 30, F24, shiftColor(c), bg, 1, c);
    if (dd == today) frame(x + 1, y + 1, CW - 2, CH - 2, YELLOW);
  }
}

// ---- 工作流程 ----
struct Task { long st, en; String name, desc; };
struct Shift { String project, name; std::vector<Task> tasks; };
std::vector<Shift> flows;
uint32_t flowsAt = 0;
int flowSel = 0, flowPage = 0;

long jsonNum(JsonObject v) {
  if (v["integerValue"].is<const char *>()) return atol(v["integerValue"]);
  if (v["doubleValue"].is<double>()) return (long)v["doubleValue"].as<double>();
  if (v["stringValue"].is<const char *>()) return atol(v["stringValue"]);
  return 0;
}

bool loadFlows() {
  if (!flows.empty() && millis() - flowsAt < 600000UL) return true;     // 10 分鐘快取
  flows.clear();
  JsonDocument pf;
  JsonObject pff = pf["documents"][0].to<JsonObject>();
  pff["name"] = true;
  pff["fields"]["name"]["stringValue"] = true;
  pff["fields"]["legacy"]["booleanValue"] = true;
  JsonDocument pd;
  if (deserializeJson(pd, firestoreGet(TIMELINE_PROJECT, "projects?pageSize=100"), DeserializationOption::Filter(pf),
                      DeserializationOption::NestingLimit(24)))
    return false;
  for (JsonObject pj : pd["documents"].as<JsonArray>()) {
    String id = pj["name"] | "";
    id = id.substring(id.lastIndexOf('/') + 1);
    String pname = pj["fields"]["name"]["stringValue"] | "專案";
    bool legacy = pj["fields"]["legacy"]["booleanValue"] | false;
    JsonDocument sf;
    JsonObject sff = sf["documents"][0]["fields"].to<JsonObject>();
    sff["name"]["stringValue"] = true;
    JsonObject tf = sff["tasks"]["arrayValue"]["values"][0]["mapValue"]["fields"].to<JsonObject>();
    tf["name"]["stringValue"] = true;
    tf["desc"]["stringValue"] = true;
    tf["startMin"] = true;
    tf["endMin"] = true;
    JsonDocument sd;
    String path = (legacy ? String("shifts") : "projects/" + id + "/shifts") + "?pageSize=100";
    if (deserializeJson(sd, firestoreGet(TIMELINE_PROJECT, path), DeserializationOption::Filter(sf),
                        DeserializationOption::NestingLimit(24)))
      continue;
    for (JsonObject sh : sd["documents"].as<JsonArray>()) {
      JsonObject f = sh["fields"];
      Shift s{pname, f["name"]["stringValue"] | "班別", {}};
      for (JsonObject t : f["tasks"]["arrayValue"]["values"].as<JsonArray>()) {
        JsonObject t2 = t["mapValue"]["fields"];
        String desc = t2["desc"]["stringValue"] | "";
        desc.replace("\n", " ");
        s.tasks.push_back({jsonNum(t2["startMin"]), jsonNum(t2["endMin"]), t2["name"]["stringValue"] | "", desc});
      }
      std::sort(s.tasks.begin(), s.tasks.end(), [](const Task &a, const Task &b) { return a.st < b.st; });
      flows.push_back(s);
    }
  }
  flowsAt = millis();
  return !flows.empty();
}

String hhmm(long mins) {
  mins = ((mins % 1440) + 1440) % 1440;
  char b[8];
  snprintf(b, sizeof(b), "%02ld:%02ld", mins / 60, mins % 60);
  return b;
}

String cutUnits(const String &s, int maxUnits) {           // 依顯示寬度截斷（中文 2 格、英數 1 格）
  std::vector<String> l = wrapText(s, maxUnits);
  if (l.empty()) return "";
  return l.size() > 1 ? l[0].substring(0, l[0].length() - 3) + "…" : l[0];
}

// 今天我的班別 → 對應的班別名稱關鍵字
String todayShiftKey() {
  struct tm t;
  if (!getLocalTime(&t, 0) || !schedOk || schedY != t.tm_year + 1900 || schedM != t.tm_mon + 1) return "";
  String c = schedCodes[t.tm_mday];
  if (c.indexOf("N") >= 0 || c.indexOf("夜8") >= 0) return "大夜";
  if (c.indexOf("E") >= 0) return "小夜";
  if (c.indexOf("D") >= 0 || c.indexOf("白值") >= 0) return "白";
  return "";
}

void drawFlows() {
  panelBase("今日工作流程：選擇班別");
  busy("讀取工作流程中…");
  if (!loadFlows()) { fill(0, 200, 800, 100, BG); busy("工作流程讀取失敗"); return; }
  fill(0, 200, 800, 100, BG);
  String key = todayShiftKey();
  for (size_t i = 0; i < flows.size() && i < 12; i++) {
    int x = 16 + (i % 2) * 390, y = 130 + (i / 2) * 58;
    bool mine = key.length() && flows[i].name.indexOf(key) >= 0;
    button(x, y, 376, 50, mine ? PANEL_HI : PANEL, mine ? YELLOW : WHITE,
           cutUnits(flows[i].name + "（" + flows[i].project + "）", 28), 10 + i);
    if (mine) frame(x, y, 376, 50, YELLOW);
  }
}

void drawTasks() {
  const Shift &s = flows[flowSel];
  panelBase(cutUnits(s.name + "（" + s.project + "）", 44));
  const int PER = 6;
  int pages = max(1, (int)(s.tasks.size() + PER - 1) / PER);
  struct tm t;
  long now = getLocalTime(&t, 0) ? t.tm_hour * 60 + t.tm_min : -1;
  for (int i = 0; i < PER; i++) {
    size_t n = flowPage * PER + i;
    if (n >= s.tasks.size()) break;
    const Task &k = s.tasks[n];
    int y = 128 + i * 50;
    bool cur = now >= 0 && (k.st <= k.en ? (now >= k.st && now < k.en) : (now >= k.st || now < k.en));
    uint16_t bg = cur ? PANEL_HI : (i % 2 ? BG : PANEL);
    fill(10, y, 780, 46, bg);
    textOn(16, y + 8, 150, 30, F24, cur ? YELLOW : CYAN, bg, 0, hhmm(k.st) + "-" + hhmm(k.en));
    textOn(170, y + 8, 612, 30, F24, WHITE, bg, 0, cutUnits(k.name + (k.desc.length() ? "：" + k.desc : ""), 50));
  }
  if (pages > 1) {
    text(300, 432, 200, 40, F24, GRAY, 1, String(flowPage + 1) + " / " + pages);
    if (flowPage > 0) button(20, 430, 160, 44, PANEL_HI, WHITE, "上一頁", B_PREV);
    if (flowPage < pages - 1) button(620, 430, 160, 44, PANEL_HI, WHITE, "下一頁", B_NEXT);
  }
}

// ---- 工作站工具 QR ----
struct Tool { const char *label, *name, *url; };
const Tool TOOLS[] = {
  {"專案進度時間軸", "專案進度時間軸", "https://achir1015.github.io/project-timeline/"},
  {"月班表管理", "月班表管理系統", "https://achir1015.github.io/rcw-month/"},
  {"A13 輸入資料", "A13 輸入資料", "https://achir1015.github.io/A13input/"},
  {"雲端電子書", "雲端硬碟電子書", "https://achir1015.github.io/my-library/"},
  {"QR 產生器", "QR Code 產生器", "https://achir1015.github.io/QR-Code/"},
  {"地址填寫工具", "台灣地址填寫工具", "https://achir1015.github.io/taiwan-address-tool/"},
  {"單一級學科", "照服員單一級學科測驗", "https://achir1015.github.io/quiz/"},
  {"GLB 檢視器", "GLB 立體檢視器", "https://achir1015.github.io/GLB-3D-Viewer/"},
  {"YouTube轉MP3", "YouTube 轉 MP3（自架 NAS）", "https://ipmos.ngrok.app/ytmp3/"},
  {"術科練習本", "照服員術科練習本", "https://achir1015.github.io/care-exam-practice/"},
  {"GPS 到達測試", "GPS 到達測試工具", "https://achir1015.github.io/gps-arrival-test/"},
  {"AI 創作歌曲", "AI 創作歌曲播放清單", "https://www.youtube.com/playlist?list=PLMiXt5EIkXI0"},
  {"督導員手冊", "居家服務督導員工作手冊", "https://achir1015.github.io/home-care-supervisor-manual/"},
  {"長照組合計算", "長照組合計算系統", "https://achir1015.github.io/Long-term-care-combined-calculation-system/"},
  {"照服員訓練指引", "照顧服務員訓練指引", "https://achir1015.github.io/caregiver-training-guide/"},
  {"照服員技術稽核", "照顧服務員技術稽核", "https://achir1015.github.io/caregiver-skills-audit/"},
  {"工作站首頁", "病房助理工作站", "https://achir1015.github.io/ward-assistant-portal/"},
};
constexpr int TOOL_COUNT = sizeof(TOOLS) / sizeof(TOOLS[0]);
int toolSel = 0;

void drawTools() {
  panelBase("病房助理工作站：點選工具顯示 QR Code");
  static const uint16_t C[] = {BLUE, GREEN, ORANGE, PINK};
  for (int i = 0; i < 16; i++) {
    int x = 10 + (i % 4) * 196, y = 128 + (i / 4) * 84;
    button(x, y, 188, 76, PANEL, WHITE, TOOLS[i].label, 20 + i);
    fill(x, y, 6, 76, C[i % 4]);
  }
  button(452, HB_BAR + 6, 210, 40, PANEL_HI, YELLOW, "工作站首頁 QR", 36);
}

void drawQr() {
  const Tool &t = TOOLS[toolSel];
  panelBase(t.name);
  qrSize = 0;
  esp_qrcode_config_t qc = {};
  qc.display_func = qrCapture;
  qc.max_qrcode_version = 7;
  qc.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
  esp_qrcode_generate(&qc, t.url);
  int n = qrSize;
  if (n <= 0) { busy("QR Code 產生失敗"); return; }
  int m = 330 / (n + 4), side = m * (n + 4), ox = 30, oy = 128;
  fill(ox, oy, side, side, WHITE);
  for (int y = 0; y < n; y++)
    for (int x = 0; x < n;) {
      if (!qrMods[y * n + x]) { x++; continue; }
      int x0 = x;
      while (x < n && qrMods[y * n + x]) x++;
      fill(ox + (x0 + 2) * m, oy + (y + 2) * m, (x - x0) * m, m, 0);
    }
  text(400, 140, 390, 36, F24, YELLOW, 0, "用手機相機掃描");
  text(400, 180, 390, 36, F24, WHITE, 0, "就能在手機上開啟這個工具");
  std::vector<String> l = wrapText(t.url, 30);
  for (size_t i = 0; i < l.size() && i < 4; i++) text(400, 250 + i * 32, 390, 30, F24, GRAY, 0, l[i]);
}

// ---- 選單與觸控 ----
// 音量：小柯與 7 吋屏共用同一個設定（存進 NVS）
void changeVolume(int delta) {
  volumePct = constrain((int)volumePct + delta, 0, 100);
  saveVolume();
  lastInteraction = millis();
}

void drawVolumeRow() {
  text(200, 404, 400, 60, F24, WHITE, 1, "音量 " + String((int)volumePct) + "%");
}

void drawMenu() {
  panelBase("快速查詢");
  button(40, 128, 345, 120, BLUE, WHITE, "我的班表", 1);
  button(415, 128, 345, 120, GREEN, WHITE, "今日工作流程", 2);
  button(40, 262, 345, 120, ORANGE, WHITE, "工作站工具 QR", 3);
  button(415, 262, 345, 120, PINK, WHITE, "唱首歌（MV）", 4);
  button(40, 400, 150, 68, PANEL_HI, WHITE, "－ 小聲", 5);
  button(610, 400, 150, 68, PANEL_HI, WHITE, "＋ 大聲", 6);
  drawVolumeRow();
}

void openMode(Mode m) {
  mode = m;
  switch (m) {
    case M_MENU: drawMenu(); break;
    case M_SCHED: drawSchedule(); break;
    case M_FLOWS: drawFlows(); break;
    case M_TASKS: drawTasks(); break;
    case M_TOOLS: drawTools(); break;
    case M_QR: drawQr(); break;
    default: break;
  }
}

// 回傳 true = 要回到大臉
bool panelTap(int x, int y) {
  panelAt = millis();
  int id = -1;
  for (auto &b : btns)
    if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) id = b.id;
  if (id < 0) return false;
  if (id == B_BACK) {
    if (mode == M_MENU) return true;
    openMode(mode == M_TASKS ? M_FLOWS : mode == M_QR ? M_TOOLS : M_MENU);
    return false;
  }
  switch (mode) {
    case M_MENU:
      if (id == 1) {
        struct tm t;
        if (getLocalTime(&t, 0)) { schedY = t.tm_year + 1900; schedM = t.tm_mon + 1; }
        openMode(M_SCHED);
      } else if (id == 2) {
        if (!schedOk) {                                   // 先讀本月班表，才知道今天上什麼班
          struct tm t;
          if (getLocalTime(&t, 0)) { schedY = t.tm_year + 1900; schedM = t.tm_mon + 1; schedOk = loadMyMonth(schedY, schedM); }
        }
        openMode(M_FLOWS);
      } else if (id == 3) openMode(M_TOOLS);
      else if (id == 4) { wantSong = true; return true; }
      else if (id == 5 || id == 6) { changeVolume(id == 5 ? -10 : 10); drawVolumeRow(); drawTopBar(true); }
      break;
    case M_SCHED:
      if (id == B_PREV && --schedM < 1) { schedM = 12; schedY--; }
      if (id == B_NEXT && ++schedM > 12) { schedM = 1; schedY++; }
      drawSchedule();
      break;
    case M_FLOWS:
      if (id >= 10) { flowSel = id - 10; flowPage = 0; openMode(M_TASKS); }
      break;
    case M_TASKS:
      if (id == B_PREV) flowPage--;
      if (id == B_NEXT) flowPage++;
      drawTasks();
      break;
    case M_TOOLS:
      if (id >= 20) { toolSel = id - 20; openMode(M_QR); }
      break;
    default: break;
  }
  return false;
}

// 讀螢幕回傳：sendxy 觸控 0x67 xh xl yh yl 狀態 FF FF FF；回傳 true 表示「放開」的那一下
bool pollTouch(int &tx, int &ty) {
  static uint8_t buf[16];
  static int n = 0;
  bool tapped = false;
  while (port.available()) {
    uint8_t c = port.read();
    if (n == 0 && c != 0x67) continue;
    buf[n++] = c;
    if (n == 9) {
      if (buf[6] == 0xFF && buf[7] == 0xFF && buf[8] == 0xFF && buf[5] == 0) {
        tx = (buf[1] << 8) | buf[2];
        ty = (buf[3] << 8) | buf[4];
        tapped = true;
      }
      n = 0;
    }
  }
  return tapped;
}
