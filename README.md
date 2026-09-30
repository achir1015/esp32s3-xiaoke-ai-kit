# 小柯 AI 聊天機器人（OpenAI 版）— AI 聊天機器人組合套件 ESP32-S3 N16R8
<img width="1442" height="829" alt="image" src="https://github.com/user-attachments/assets/f84a0107-3b67-4cac-b865-dd28fba4cd8b" />
<img width="1430" height="887" alt="image" src="https://github.com/user-attachments/assets/140f8a41-815d-4f62-8131-7e82ccfe9064" />
<img width="1047" height="891" alt="image" src="https://github.com/user-attachments/assets/a6753c2d-1135-4613-8963-04535b35088b" />
<img width="527" height="339" alt="image" src="https://github.com/user-attachments/assets/eb19b21e-4c18-4f95-90fb-6d01344fa6c1" />
<img width="608" height="507" alt="image" src="https://github.com/user-attachments/assets/ed6b05c6-da62-4316-bba7-63962e7409be" />
<img width="1477" height="1108" alt="image" src="https://github.com/user-attachments/assets/8af6c2c5-f7db-4135-867d-3219753b5bad" />

把套件原廠的「小智」韌體換成 **小柯 OpenAI 版**：直接說話 → OpenAI 語音轉文字 → GPT 回答 → OpenAI TTS 從 3W 喇叭講出來；1.54 吋螢幕顯示可愛表情、時鐘、農曆和對話文字。

由 `E:\Aduino多功能 AI語音影像辨識聊天機器人\AI_Voice_Robot` 移植而來。本套件沒有鏡頭，所以拿掉了拍照與雲端硬碟功能。

## 硬體

| 元件 | 說明 |
|---|---|
| 核心板 | Goouuu ESP32-S3 N16R8（16MB Flash、8MB OPI PSRAM，CH340 → COM3）|
| 擴展板 | S3 智能擴展板 V1.3（XZ-AI_KZB，原理圖見上層資料夾 `XZ-AI_KZBV1.3原理图.pdf`）|
| 螢幕 | 1.54" ST7789 240×240（7P）|
| 麥克風 / 功放 | INMP441 / MAX98357A + 3W 喇叭 |

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

## 還原原廠小智韌體

原廠韌體完整備份在 `backup/xiaozhi_original_16MB.bin`（2026-09-30 從 COM3 讀出）：

```bash
esptool --chip esp32s3 --port COM3 --baud 921600 write-flash 0 backup/xiaozhi_original_16MB.bin
```

## 創意開發者

此系統是**吳玉柱先生**與 Claude AI 共同開發，有任何意見請聯絡 achir1015@gmail.com。
