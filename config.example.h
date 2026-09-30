// ============================================================================
//  config.example.h — 設定範本（複製成 config.h 再填入自己的值；沒有 config.h 時自動使用本檔）
//  config.h — 所有可調整的設定都在這裡（Wi-Fi、API Key、腳位、模型、音量…）
//  ⚠ 此檔含 API Key，請勿上傳到 GitHub 或分享給他人
// ============================================================================
#pragma once

// ---------------------------------------------------------------- Wi-Fi ----
#define WIFI_SSID       "你的WiFi名稱"
#define WIFI_PASSWORD   "你的WiFi密碼"

// --------------------------------------------------------------- OpenAI ----
#define OPENAI_API_KEY  "sk-請填入你的OpenAI金鑰"

#define STT_MODEL       "gpt-4o-mini-transcribe"   // 或 "whisper-1"
#define CHAT_MODEL      "gpt-4o-mini"
#define TTS_MODEL       "gpt-4o-mini-tts"          // 或 "tts-1"
#define TTS_VOICE       "nova"                     // alloy / ash / coral / echo / fable / nova / onyx / sage / shimmer
#define TTS_INSTRUCTIONS "用可愛、活潑、溫暖的台灣口音中文說話，語氣像貼心的小朋友，語速稍快。"

#define ROBOT_NAME      "小柯"
#define MAX_HISTORY_MSGS 10        // 記住最近幾則對話（user+assistant 各算 1 則）
#define CHAT_MAX_TOKENS  300

// ------------------------------------------------------------ 上網搜尋 ----
// 1 = 問天氣、新聞、股價等即時資訊時，AI 會自己決定上網搜尋再回答（OpenAI web_search，搜尋會另外計費）
#define ENABLE_WEB_SEARCH 1
#define SEARCH_CITY       "Taipei"   // 搜尋預設地區（英文城市名）
#define SEARCH_CITY_ZH    "台北"     // 沒講地點時以這裡為準

// ------------------------------------------------------------- 功能開關 ----
#define HW_TEST_MODE    0          // 1 = 開機只跑硬體測試（螢幕/喇叭/麥克風/按鍵），先確認接線用

// ------------------------------------------------------------ 唱歌（YouTube 歌單）----
// 說「唱首歌」會從這個播放清單隨機選一首，顯示封面、歌名與 QR 碼（手機掃描在 YouTube 播放）
#define YT_PLAYLIST_URL   "https://www.youtube.com/playlist?list=PLMiXt5EIkXI0"
#define SONG_COMPOSER     "吳玉柱"   // 詞曲創作者（版權所有）
#define SONG_SHOW_SEC     180        // 唱歌畫面停留秒數（按鍵可提早回到聊天）

// ------------------------------------------------------------ 聲控（免按鍵）----
#define VOICE_ACTIVATION  1        // 1 = 直接說話就會開始聆聽；0 = 只用按鍵（按住 BOOT 說話）
#define VAD_MIN_RMS       250      // 觸發錄音的最低音量（環境吵、常誤觸就調高；喊很大聲才有反應就調低）
#define VAD_RATIO         3.0      // 音量需高於背景噪音幾倍才觸發
#define VAD_SILENCE_MS    900      // 停頓多久視為說完
#define VAD_MIN_SPEECH_MS 400      // 少於這個長度的聲音視為雜音
#define SLEEPY_AFTER_SEC  90       // 多久沒互動表情變想睡

// ================================================================ 腳位 ====
//  「S3 智能擴展板 V1.3」(XZ-AI_KZB) + Goouuu ESP32-S3 N16R8 核心板
//  依原理圖 XZ-AI_KZBV1.3原理图.pdf 與 小智控制固件接线说明.pdf
//  PSRAM(OPI) 佔用 35,36,37，不可使用
// ---------------------------------------------------------------------------

// ---- 1.54 吋 ST7789 240x240（7P：GND VCC SCL SDA RES DC BLK）----
#define TFT_SCK         21
#define TFT_MOSI        47
#define TFT_CS          -1         // 7P 版沒有 CS 腳（8P 版接 41）
#define TFT_DC          40
#define TFT_RST         45
#define TFT_BL          42         // 背光
#define TFT_ROTATION    2          // 0~3，本套件實測為 2（設 0 會上下顛倒）
#define TFT_INVERT      1          // IPS 面板通常要反相；顏色黑白顛倒就改 0
#define TFT_SPI_MODE    3          // 沒有 CS 的 ST7789 模組需要 SPI MODE3

// ---- INMP441 I2S 麥克風（擴展板 P6，L/R 接 GND = 左聲道）----
#define MIC_WS          4
#define MIC_SCK         5
#define MIC_SD          6
#define MIC_USE_RIGHT   0
#define MIC_GAIN_SHIFT  14         // 錄音音量：數字越小越大聲（11~16），聲音太小/破音時調整

// ---- MAX98357A I2S 擴大機（擴展板 P7）----
#define AMP_DIN         7
#define AMP_BCLK        15
#define AMP_LRC         16

// ---- 按鍵 ----
#define PIN_BUTTON      0          // S1 / BOOT：喚醒、功能說明（按下為 LOW）
#define BUTTON_ACTIVE_LOW 1
#define PIN_VOL_DOWN    39         // S3 音量-：短按 -10%，長按 +10%（循環調整）；沒接設 -1
//  S2 音量+ 在原理圖上接 GPIO40，和螢幕 DC 共用，按下會干擾畫面，所以不使用

// ---------------------------------------------------------------- 音訊 ----
#define MIC_SAMPLE_RATE  16000
#define MAX_RECORD_SEC   12
#define TTS_SAMPLE_RATE  24000     // OpenAI TTS pcm 格式固定 24kHz / 16bit / mono
#define DEFAULT_VOLUME   70        // 0~100
