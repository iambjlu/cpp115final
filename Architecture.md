# 智慧日記本 — 系統架構說明

## 1. 專案概述

智慧日記本是一個基於 C++ 的終端機應用程式，提供日記的新增、刪除、修改、搜尋功能，支援多帳號管理與密碼保護，所有資料透過 JSON 檔案持久化存儲。

## 2. 技術堆疊

| 層級 | 技術 |
|------|------|
| 語言 | C++17 |
| 建置系統 | CMake 3.18+ |
| 測試框架 | Google Test (via FetchContent) |
| 資料格式 | JSON（自實作輕量解析器） |
| 終端 UI | ANSI Escape Codes |

## 3. 專案目錄結構

```
cpp115final/
├── CMakeLists.txt
├── include/
│   ├── entry.h          # BaseEntry, DiaryEntry, SystemLog
│   ├── account.h        # Account, AccountManager
│   ├── storage.h        # Storage, JSONStorage
│   ├── search.h         # SearchEngine
│   ├── ui.h             # TerminalUI
│   └── app.h            # DiaryApp
├── src/
│   ├── entry.cpp
│   ├── account.cpp
│   ├── storage.cpp
│   ├── search.cpp
│   ├── ui.cpp
│   ├── app.cpp
│   └── main.cpp
├── tests/
│   └── tests.cpp
├── openspec/
│   └── spec.md
├── data/                # 運行時資料目錄
└── README.md
```

## 4. 類別設計

### 4.1 核心條目層（`include/entry.h`, `src/entry.cpp`）

#### `BaseEntry`（抽象基類）
- **職責**：定義所有系統條目的共同介面
- **成員**：建立時間戳（`std::chrono::system_clock::time_point`）、唯一 ID（`std::string`）
- **純虛函數**：`virtual std::string type() const = 0`、`virtual std::string to_json() const = 0`、`virtual void from_json(const std::string&) = 0`
- **虛析構**：確保正確清理子類別資源

#### `DiaryEntry`（繼承 `BaseEntry`）
- **職責**：代表使用者的日記條目
- **成員**：
  - `std::string title` — 日記標題
  - `std::string content` — 日記內容
  - `std::string date` — 日期字串（YYYY-MM-DD）
  - `std::vector<std::string> tags` — 標籤列表（STL vector）
  - `std::string ownerId` — 擁有者帳號 ID
- **方法**：`type()` 返回 `"diary"`，完整的 `to_json()` / `from_json()` 序列化

#### `SystemLog`（繼承 `BaseEntry`）
- **職責**：記錄系統層級的運作日誌
- **成員**：
  - `std::string level` — 日誌等級（INFO / WARN / ERROR）
  - `std::string message` — 日誌訊息
- **方法**：`type()` 返回 `"system_log"`，完整的 `to_json()` / `from_json()` 序列化

### 4.2 帳號管理層（`include/account.h`, `src/account.cpp`）

#### `Account`
- **職責**：封裝單一使用者帳號資訊
- **成員**：
  - `std::string id` — 唯一 ID（UUID）
  - `std::string username` — 使用者名稱
  - `std::string passwordHash` — SHA-256 雜湊密碼
  - `std::string createdAt` — 建立時間
- **方法**：`verifyPassword(plainText)` 驗證密碼、`to_json()` / `from_json()` 序列化

#### `AccountManager`
- **職責**：管理所有帳號的註冊、登入、查詢、刪除
- **成員**：`std::map<std::string, Account> accounts`（以 username 為 key）
- **方法**：
  - `registerAccount(username, password)` — 註冊新帳號
  - `login(username, password)` — 驗證並回傳目前登入的 Account 指標
  - `logout()` — 登出目前帳號
  - `deleteAccount(password)` — 驗證密碼後刪除目前登入的帳號
  - `getCurrentUser()` — 回傳目前登入使用者
  - `save()` / `load()` — 持久化帳號資料

### 4.3 資料存儲層（`include/storage.h`, `src/storage.cpp`）

#### `Storage`（抽象基類）
- **職責**：定義資料持久化的共同介面
- **純虛函數**：
  - `virtual bool save_entries(const std::vector<std::unique_ptr<BaseEntry>>&) = 0`
  - `virtual std::vector<std::unique_ptr<BaseEntry>> load_entries() = 0`

#### `JSONStorage`（繼承 `Storage`）
- **職責**：將條目序列化為 JSON 檔案
- **成員**：`std::string filepath` — 資料檔路徑
- **方法**：實作 `save_entries()` 與 `load_entries()`，使用自實作的 JSON 解析器

### 4.4 搜尋引擎（`include/search.h`, `src/search.cpp`）

#### `SearchEngine`
- **職責**：提供日記搜尋功能
- **方法**：
  - `searchByDate(entries, date)` — 依日期精確匹配
  - `searchByKeyword(entries, keyword)` — 在標題與內容中搜尋關鍵字（不區分大小寫）
  - `searchByTag(entries, tag)` — 依標籤篩選

### 4.5 終端機 UI（`include/ui.h`, `src/ui.cpp`）

#### `TerminalUI`
- **職責**：提供基於 ANSI Escape Code 的終端機互動介面
- **功能**：
  - 主選單渲染（登入/註冊/功能選單）
  - 日記列表顯示（分頁）
  - 表單輸入（標題、內容、標籤）
  - 密碼輸入隱藏（以 `*` 號顯示）
  - 搜尋結果顯示
  - 色彩主題（ANSI 色彩碼）

### 4.6 應用程式控制器（`include/app.h`, `src/app.cpp`）

#### `DiaryApp`
- **職責**：整合所有模組，管理應用程式生命週期
- **成員**：
  - `AccountManager accountManager`
  - `JSONStorage storage`
  - `SearchEngine searchEngine`
  - `TerminalUI ui`
  - `std::vector<std::unique_ptr<BaseEntry>> entries` — 目前使用者的所有條目
- **方法**：
  - `run()` — 主循環
  - `addEntry()`、`deleteEntry()`、`editEntry()`
  - `searchEntries()`、`listEntries()`
  - `deleteAccount()` — 驗證密碼後刪除帳號及相關日記
  - `saveAndExit()`

## 5. 資料流

```
使用者輸入 → TerminalUI → DiaryApp → AccountManager/Storage/SearchEngine → 檔案系統
                                                                                    ↓
使用者輸出 ← TerminalUI ← DiaryApp ← 處理結果 ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ←
```

## 6. 安全設計

- 密碼以 SHA-256 雜湊後存儲（不存儲明文）
- 每位使用者的日記獨立存儲，透過 `ownerId` 隔離
- 未登入狀態無法存取日記功能

## 7. 開發迭代流程

| 迭代 | Branch | 內容 |
|------|--------|------|
| v1 | `feature/core-models` | 核心類別層（BaseEntry、DiaryEntry、SystemLog） |
| v2 | `feature/account-system` | 帳號系統（Account、AccountManager） |
| v3 | `feature/storage` | 檔案存儲（Storage、JSONStorage） |
| v4 | `feature/search` | 搜尋引擎 |
| v5 | `feature/ui` | 終端機 UI |
| v6 | `feature/integration` | 整合、測試、main.cpp |

## 8. Use Case 說明

| ID | 用例 | 參與者 | 說明 |
|----|------|--------|------|
| UC1 | 註冊帳號 | 使用者 | 輸入帳號密碼，建立新帳號並持久化 |
| UC2 | 登入系統 | 使用者 | 驗證帳號密碼，登入後方可使用日記功能 |
| UC3 | 登出 | 使用者 | 結束目前登入狀態 |
| UC4 | 刪除帳號 | 使用者 | 驗證密碼後刪除帳號及所有相關日記（extend UC2，須已登入） |
| UC5 | 新增日記 | 使用者 | 輸入標題、內容、標籤，建立日記條目 |
| UC6 | 修改日記 | 使用者 | 選擇既有日記，修改標題或內容 |
| UC7 | 刪除日記 | 使用者 | 選擇既有日記並刪除 |
| UC8 | 瀏覽日記列表 | 使用者 | 分頁顯示目前使用者的所有日記 |
| UC9 | 搜尋日記 | 使用者 | 依日期、關鍵字、標籤三種方式搜尋（include UC9a/9b/9c） |
| UC10 | 資料持久化 | 系統 | 將日記與帳號資料寫入 JSON 檔案（被多項操作 include） |
