# 智慧日記本 — 期末專題

一個基於 C++17 的終端機日記應用程式，使用類別繼承、STL 類別庫、檔案讀寫等功能，提供完整的日記管理體驗。

## 功能特色

- **帳號系統**：註冊、登入、登出，密碼以 SHA-256 雜湊存儲
- **日記管理**：新增、刪除、修改日記，支援標題、內容、日期、標籤
- **多維度搜尋**：依日期、關鍵字、標籤搜尋日記
- **資料持久化**：JSON 格式存檔，重開機不遺失
- **終端機 UI**：ANSI 色彩碼美化介面
- **系統日誌**：自動記錄操作日誌
- **單元測試**：25 個測試案例覆蓋核心功能

## 技術需求

- C++17 編譯器（clang++ / g++）
- CMake 3.18+
- macOS / Linux（SHA-256 使用 CommonCrypto / 備用雜湊）

## 編譯與執行

```bash
# 1. 配置並編譯
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8

# 2. 執行程式
./diaryapp

# 3. 執行單元測試
ctest --verbose
# 或
./run_tests
```

## 專案結構

```
cpp115final/
├── CMakeLists.txt          # 建置設定
├── include/                # 標頭檔
│   ├── entry.h            # BaseEntry, DiaryEntry, SystemLog
│   ├── account.h          # Account, AccountManager
│   ├── storage.h          # Storage, JSONStorage
│   ├── search.h           # SearchEngine
│   ├── ui.h               # TerminalUI
│   └── app.h              # DiaryApp
├── src/                    # 原始碼
├── tests/                  # 單元測試
├── openspec/               # OpenSpec 規格文件
├── data/                   # 運行時資料
├── Architecture.md         # 系統架構說明
├── diagram.md              # 系統圖表
└── README.md
```

## 架構說明

### 類別繼承

```
BaseEntry (抽象基類)
├── DiaryEntry    — 使用者日記條目
└── SystemLog     — 系統運作日誌

Storage (抽象基類)
└── JSONStorage   — JSON 檔案存儲
```

### 核心模組

| 模組 | 說明 |
|------|------|
| `entry.h/cpp` | 核心資料模型，展示類別繼承與多型 |
| `account.h/cpp` | 帳號管理，含密碼雜湊與登入驗證 |
| `storage.h/cpp` | 資料持久化，JSON 序列化/反序列化 |
| `search.h/cpp` | 搜尋引擎，支援日期/關鍵字/標籤搜尋 |
| `ui.h/cpp` | 終端機 UI，ANSI 色彩與選單系統 |
| `app.h/cpp` | 應用程式控制器，整合所有模組 |

### STL 使用

- `std::vector` — 管理日記條目列表
- `std::map` — 帳號索引
- `std::unique_ptr` — 多型資源管理
- `std::string` / `std::pair` — 資料封裝
- `std::chrono` — 時間戳處理
- `std::make_unique` — 安全記憶體管理

## 開發迭代流程

| Sprint | 分支 | 內容 |
|--------|------|------|
| 1 | `feature/core-models` | 核心類別層 |
| 2 | `feature/account-system` | 帳號管理 |
| 3 | `feature/storage` | 檔案存儲 |
| 4 | `feature/search` | 搜尋引擎 |
| 5 | `feature/ui` | 終端機 UI |
| 6 | `feature/integration` | 整合與測試 |

## 測試覆蓋

| 測試模組 | 測試數 | 內容 |
|----------|--------|------|
| EntryTest | 8 | 多型、序列化、getter/setter |
| AccountTest | 9 | 註冊、登入、密碼雜湊、驗證 |
| StorageTest | 3 | 存檔、讀檔、空資料 |
| SearchTest | 5 | 日期、關鍵字、標籤搜尋 |
| **總計** | **25** | |

## 規格文件

詳細規格請參閱：
- [OpenSpec 規格](openspec/spec.md)
- [系統架構](Architecture.md)
- [系統圖表](diagram.md)
