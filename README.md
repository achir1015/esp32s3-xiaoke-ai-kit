# 小柯 AI 聊天機器人（OpenAI 版）— AI 聊天機器人組合套件 ESP32-S3 N16R8
<img width="1442" height="829" alt="image" src="https://github.com/user-attachments/assets/f84a0107-3b67-4cac-b865-dd28fba4cd8b" />
<img width="1430" height="887" alt="image" src="https://github.com/user-attachments/assets/140f8a41-815d-4f62-8131-7e82ccfe9064" />
<img width="1047" height="891" alt="image" src="https://github.com/user-attachments/assets/a6753c2d-1135-4613-8963-04535b35088b" />
<img width="527" height="339" alt="image" src="https://github.com/user-attachments/assets/eb19b21e-4c18-4f95-90fb-6d01344fa6c1" />
<img width="608" height="507" alt="image" src="https://github.com/user-attachments/assets/ed6b05c6-da62-4316-bba7-63962e7409be" />
<img width="1477" height="1108" alt="image" src="https://github.com/user-attachments/assets/8af6c2c5-f7db-4135-867d-3219753b5bad" />

把套件原廠的「小智」韌體換成 **小柯 OpenAI 版**：直接說話 → OpenAI 語音轉文字 → GPT 回答 → OpenAI TTS 從 3W 喇叭講出來；1.54 吋螢幕顯示可愛表情、時鐘、農曆和對話文字。

可再外接 **7 吋觸控螢幕**（淘晶馳 TJC8048X570）：顯示小柯的大臉主畫面、播放內建 MV，並提供班表、工作流程、病房助理工作站 QR 的觸控查詢面板（見[下方說明](#7-吋觸控螢幕淘晶馳-tjc8048x570)）。

由 `E:\Aduino多功能 AI語音影像辨識聊天機器人\AI_Voice_Robot` 移植而來。本套件沒有鏡頭，所以拿掉了拍照與雲端硬碟功能。

## 硬體

| 元件 | 說明 |
|---|---|
| 核心板 | Goouuu ESP32-S3 N16R8（16MB Flash、8MB OPI PSRAM，CH340 → COM3）|
| 擴展板 | S3 智能擴展板 V1.3（XZ-AI_KZB，原理圖見上層資料夾 `XZ-AI_KZBV1.3原理图.pdf`）|
| 螢幕 | 1.54" ST7789 240×240（7P）|
| 麥克風 / 功放 | INMP441 / MAX98357A + 3W 喇叭 |
| 7 吋觸控螢幕（選配）| 淘晶馳 TJC8048X570_011R_Y，800×480 電阻觸控，串口接 GPIO17/18 |

腳位（依原理圖與 `小智控制固件接线说明.pdf`，已寫在 `config.h`）：

| 元件 | 腳位 |
|---|---|
| ST7789 7P | SCL=21、SDA=47、RES=45、DC=40、BLK=42（沒有 CS）|
| INMP441 | WS=4、SCK=5、SD=6（L/R 接地 = 左聲道）|
| MAX98357A | DIN=7、BCLK=15、LRC=16 |
| S1 喚醒鍵 | GPIO0（BOOT）|
| S3 音量- | GPIO39 |
| S2 音量+ | GPIO40，和螢幕 DC 共用 → **不使用** |

## 編譯與上傳

Arduino IDE 2.x + esp32 by Espressif **3.x**，函式庫：Adafruit GFX、Adafruit ST7735 and ST7789、ArduinoJson 7.x。

| 工具選單 | 設定 |
|---|---|
| 開發板 | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| PSRAM | **OPI PSRAM** |
| Upload Speed | 921600 |

命令列：

```bash
arduino-cli compile --fqbn "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,UploadSpeed=921600" --output-dir build .
```

```bash
arduino-cli upload -p COM3 --fqbn "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,UploadSpeed=921600" --input-dir build .
```

## 設定 `config.h`

複製 `config.example.h` 成 `config.h`，填入 Wi-Fi（只支援 2.4GHz）與 OpenAI API Key。也可以不填：開機連不上時，機器人會開熱點 `XiaoKe-Setup`（密碼 `xiaoke123`），用手機開 `http://192.168.4.1` 設定。

螢幕問題：
- 顏色黑白顛倒 → `TFT_INVERT` 改 0
- 畫面方向：`TFT_ROTATION` 實測為 2（0 會上下顛倒）
- 全黑 → 先用 `HW_TEST_MODE 1` 測試

## 使用方式

| 操作 | 效果 |
|---|---|
| 直接說話 | 停頓一下就自動送出，小柯用繁體中文回答並唸出來 |
| 「大聲一點」「小聲一點」 | 調整音量 |
| 「清除記憶」「重新開始」 | 忘掉先前的對話 |
| 天氣、新聞、股價、比賽結果… | 小柯會先上網搜尋（OpenAI web search）再用口語回答 |
| 「唱首歌」「來首歌」 | 從吳玉柱的 YouTube 創作歌單隨機選一首，顯示封面、歌名與 QR 碼，手機掃描就能在 YouTube 播放 |
| 「更新歌單」 | 重新下載播放清單（開機也會自動更新）|
| S1 喚醒鍵（BOOT） | 短按：功能說明（最後一頁再按回到聊天）；按住：說話，放開送出（環境吵雜時用）；唱歌畫面時按：回到聊天 |
| S3 音量- | 短按 -10%、按住每 0.35 秒 +10%（音量會保存）|
| 瀏覽器開螢幕左下角網址 / `http://xiaoke.local` | 修改 Wi-Fi、API Key、YouTube 歌單網址，或按「更新歌單」|

## 上網查詢（天氣、新聞…）

對話改用 OpenAI **Responses API** 並開啟 `web_search` 工具：遇到需要即時資訊的問題，AI 會自己決定上網搜尋，再用口語摘要回答（來源網址會自動拿掉不唸）。一般聊天不會搜尋。

- `config.h`：`ENABLE_WEB_SEARCH`（0 = 關閉）、`SEARCH_CITY` / `SEARCH_CITY_ZH`（沒講地點時的預設城市）
- 有搜尋的回答約多 3–8 秒，而且每次搜尋 OpenAI 會另外計費（請見 OpenAI 價目表的 web search 項目）

## 唱歌（YouTube 創作歌單）

歌曲為**吳玉柱**詞曲創作，版權所有。預設歌單在 `config.h` 的 `YT_PLAYLIST_URL`，也可以在設定網頁修改。機器人會下載播放清單網頁解析出歌名與影片 ID（最多 80 首，存在 NVS），唱歌畫面顯示 180 秒（`SONG_SHOW_SEC`），期間暫停聲控，避免手機放歌時誤觸發。

也同步了原專案的防誤觸：一路沒停頓錄到上限（多半是音樂或電視）不送出，並自動提高觸發門檻，安靜後會慢慢降回。

## 7 吋觸控螢幕（淘晶馳 TJC8048X570）

外接淘晶馳 X5 系列 7 吋串口屏 **TJC8048X570_011R_Y**（800×480、電阻觸控、內建喇叭接口），當小柯的「大螢幕」。程式在 `hmi_screen.h`（畫面、MV）與 `hmi_panel.h`（觸控查詢面板）；沒接螢幕時小柯照常運作，不想用可在 `config.h` 加 `#define HMI_ENABLE 0`。

### 接線

螢幕 4P 端子（XH2.54）：

| 螢幕 | 接到 | 說明 |
|---|---|---|
| GND | ESP32 GND | 一定要共地 |
| RX | **GPIO17**（ESP32 TX）| 交叉接 |
| TX | **GPIO18**（ESP32 RX）| 交叉接 |
| 5V | 5V 2A 電源 | 螢幕最大約 430mA，接喇叭更多，只靠電腦 USB 可能重開機 |

- 螢幕串口是 3.3V/5V TTL，ESP32 可直接接，不需電平轉換。
- 螢幕開機是 9600 bps，小柯連上後會暫時提速到 115200。
- 螢幕喇叭接背面 PH1.25 接口（≤2W）。**不能和小柯共用同一顆喇叭**（兩個功放輸出會互灌）。

### 畫面功能

| 功能 | 說明 |
|---|---|
| 大臉主畫面 | 上方時間列（日期、星期、農曆、時鐘、音量、Wi-Fi 燈號）、中間表情（聆聽／思考／說話依情緒變化，嘴巴跟著說話音量開合，待機會眨眼）、下方對話字幕（太長自動翻頁）|
| 唱歌播 MV | 說「唱首歌」或從選單點「唱首歌」，有接 7 吋屏時會優先選有內建 MV 的歌：《五常行處見春風》全螢幕、《日照藍天情》400×240 置中。播放中點螢幕**左 1/3 小聲、右 1/3 大聲、中間停止**；播完自動回到大臉。小螢幕照常顯示 QR 碼 |
| 快速查詢面板 | 在大臉點螢幕任何地方開啟選單，60 秒沒操作或小柯開口說話就回到大臉 |
| ‧ 我的班表 | 讀「月班表管理系統」Firebase（`monthlyschedule-a5dea`），以月曆顯示 `SCHEDULE_MY_NAME` 的班別：白班黃、小夜橘、大夜藍、休假綠，今天加黃框，可切換上／下個月 |
| ‧ 今日工作流程 | 讀「專案進度時間軸」Firebase（`project-timeline-d2188`），今天上的班自動標亮；點進去依時間列出任務，目前時段反白 |
| ‧ 工作站工具 QR | 病房助理工作站 16 個工具＋首頁，點一下顯示大 QR 碼與網址，手機掃描開啟 |
| ‧ 音量 | 選單底部「－ 小聲／＋ 大聲」，與小柯共用同一個音量設定（會保存）；7 吋屏實際音量 = 小柯音量 × 2（小柯 50% 以上即滿格）|

> 這塊螢幕沒有作業系統與瀏覽器，**無法開啟網頁**；網頁工具請用 QR 碼在手機開啟。

### HMI 專案（螢幕端）

螢幕端資源用官方 **USART HMI** 編輯器製作，檔案不放在這個 repo（含 MV 約 115MB）：`E:\電阻式觸摸控制\XiaoKeHMI\XiaoKe.HMI`

| 資源 | 內容 |
|---|---|
| 編碼 | utf-8，型號 TJC8048X570_011，橫向 0° |
| 字庫 ID0 `tw24` | 微軟正黑體 24px，Big5 常用＋次常用 13,811 字（「指定字符」貼上 `charset_tw.txt`）|
| 字庫 ID1 `tw48n` | 48px，只含時鐘數字 `0123456789: -/%` |
| page0 | 空白頁，所有畫面由小柯用 `cls / fill / cirs / xstr` 指令即時繪製，觸控用 `sendxy=1` 回傳座標 |
| page1 / 視頻 ID0 | 全螢幕影片控件 `v0`：《五常行處見春風》800×480、12fps、極低畫質、11025Hz |
| page2 / 視頻 ID1 | 置中 400×240 影片控件 `v0`：《日照藍天情》10fps、11025Hz |

影片轉換：USART HMI「工具 → 視頻/音頻轉換」（VideoBox）。內建 Flash 約 120MB，全螢幕 MV 一首就要約 100MB，所以第二首只能縮小；之後插 **32GB 以下 FAT32 microSD 卡**，即可把多首 MV 都改成全螢幕高畫質。

**下載 HMI 到螢幕**：螢幕 RX/TX 要暫時從 GPIO17/18 改接 USB 轉 TTL（例如 PL2303），在 USART HMI 按「下載」、鮑率 921600（115MB 約 27 分鐘），完成後再接回小柯。只改小柯程式（畫面、選單、版面）**不需要**重新下載 HMI。

新增 MV：在 HMI 專案加入影片資源與頁面後，於 `hmi_screen.h` 的 `MVS[]` 加一行 `{YouTube ID, 頁面, 視頻 ID, 秒數, 歌名}`。

## 還原原廠小智韌體

原廠韌體完整備份在 `backup/xiaozhi_original_16MB.bin`（2026-09-30 從 COM3 讀出）：

```bash
esptool --chip esp32s3 --port COM3 --baud 921600 write-flash 0 backup/xiaozhi_original_16MB.bin
```

## 創意開發者

此系統是**吳玉柱先生**與 Claude AI 共同開發，有任何意見請聯絡 achir1015@gmail.com。
