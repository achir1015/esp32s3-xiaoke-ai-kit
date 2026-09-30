# 小柯 AI 聊天機器人（OpenAI 版）— AI 聊天機器人組合套件 ESP32-S3 N16R8

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
| S1 喚醒鍵（BOOT） | 瀏覽功能說明，最後一頁再按回到聊天 |
| S3 音量- | 短按 -10%、按住每 0.35 秒 +10%（音量會保存）|
| 瀏覽器開螢幕左下角網址 / `http://xiaoke.local` | 修改 Wi-Fi、API Key |

## 還原原廠小智韌體

原廠韌體完整備份在 `backup/xiaozhi_original_16MB.bin`（2026-09-30 從 COM3 讀出）：

```bash
esptool --chip esp32s3 --port COM3 --baud 921600 write-flash 0 backup/xiaozhi_original_16MB.bin
```

## 創意開發者

此系統是**吳玉柱先生**與 Claude AI 共同開發，有任何意見請聯絡 achir1015@gmail.com。
