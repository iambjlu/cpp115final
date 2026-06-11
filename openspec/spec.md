# OpenSpec — 智慧日記本規格說明

## 1. 需求規格

### 1.1 功能性需求

| ID | 需求描述 | 優先級 |
|----|----------|-------|
| FR-01 | 系統允許使用者註冊新帳號（使用者名稱 + 密碼） | 必須 |
| FR-02 | 系統允許已註冊使用者登入/登出 | 必須 |
| FR-03 | 系統允許登入使用者新增日記（標題、內容、日期、標籤） | 必須 |
| FR-04 | 系統允許登入使用者刪除指定日記 | 必須 |
| FR-05 | 系統允許登入使用者修改既有日記內容 | 必須 |
| FR-06 | 系統允許依日期搜尋日記 | 必須 |
| FR-07 | 系統允許依關鍵字搜尋日記（標題與內容） | 必須 |
| FR-08 | 系統允許依標籤搜尋日記 | 應該 |
| FR-09 | 系統在退出時將所有資料寫入本地檔案 | 必須 |
| FR-10 | 系統在啟動時從本地檔案載入資料 | 必須 |

### 1.2 非功能性需求

| ID | 需求描述 |
|----|----------|
| NFR-01 | 密碼不得以明文存儲，必須使用雜湊演算法 |
| NFR-02 | 系統使用 C++ STL 容器管理資料 |
| NFR-03 | 系統使用類別繼承實現多型行為 |
| NFR-04 | 終端機介面使用 ANSI 色彩碼提供視覺回饋 |
| NFR-05 | 系統支援 UTF-8 編碼以處理中文內容 |

## 2. 類別規格

### 2.1 BaseEntry（抽象基類）

```
Class: BaseEntry
Type: Abstract Base Class
Responsibility: Define the common interface for all entries in the system

Attributes:
  - id: string (unique identifier, generated via UUID)
  - createdAt: chrono::system_clock::time_point

Virtual Methods:
  + type() -> string: Return the entry type identifier
  + to_json() -> string: Serialize the entry to a JSON string
  + from_json(json: string): Deserialize from a JSON string
  + ~BaseEntry() [virtual destructor]
```

### 2.2 DiaryEntry

```
Class: DiaryEntry
Inherits: BaseEntry
Responsibility: Represent a user-created diary entry

Attributes:
  - title: string
  - content: string
  - date: string (YYYY-MM-DD format)
  - tags: vector<string>
  - ownerId: string (account ID of the owner)

Methods:
  + type() -> string: Returns "diary"
  + to_json() -> string
  + from_json(json: string)
  + getters/setters for all attributes
```

### 2.3 SystemLog

```
Class: SystemLog
Inherits: BaseEntry
Responsibility: Record system-level operational logs

Attributes:
  - level: string (INFO / WARN / ERROR)
  - message: string

Methods:
  + type() -> string: Returns "system_log"
  + to_json() -> string
  + from_json(json: string)
```

### 2.4 Account

```
Class: Account
Responsibility: Encapsulate a single user account

Attributes:
  - id: string (UUID)
  - username: string
  - passwordHash: string (SHA-256 hash)
  - createdAt: string

Methods:
  + verifyPassword(plainText: string) -> bool
  + to_json() -> string
  + from_json(json: string)
```

### 2.5 AccountManager

```
Class: AccountManager
Responsibility: Manage account registration, login, and queries

Attributes:
  - accounts: map<string, Account> (keyed by username)
  - currentUser: Account* (pointer to logged-in account)

Methods:
  + registerAccount(username: string, password: string) -> bool
  + login(username: string, password: string) -> bool
  + logout()
  + getCurrentUser() -> Account*
  + save()
  + load()
```

### 2.6 Storage（抽象基類）

```
Class: Storage
Type: Abstract Base Class
Responsibility: Define the persistence interface

Virtual Methods:
  + save_entries(entries: vector<unique_ptr<BaseEntry>>) -> bool
  + load_entries() -> vector<unique_ptr<BaseEntry>>
```

### 2.7 JSONStorage

```
Class: JSONStorage
Inherits: Storage
Responsibility: Serialize entries to/from JSON files

Attributes:
  - filepath: string

Methods:
  + save_entries(entries: vector<unique_ptr<BaseEntry>>) -> bool
  + load_entries() -> vector<unique_ptr<BaseEntry>>
```

### 2.8 SearchEngine

```
Class: SearchEngine
Responsibility: Provide search capabilities for diary entries

Methods:
  + searchByDate(entries: vector<DiaryEntry*>, date: string) -> vector<DiaryEntry*>
  + searchByKeyword(entries: vector<DiaryEntry*>, keyword: string) -> vector<DiaryEntry*>
  + searchByTag(entries: vector<DiaryEntry*>, tag: string) -> vector<DiaryEntry*>
```

### 2.9 TerminalUI

```
Class: TerminalUI
Responsibility: Provide terminal-based interactive interface

Methods:
  + showMainMenu()
  + showLoginScreen() -> pair<string, string>
  + showRegisterScreen() -> pair<string, string>
  + showDiaryList(entries: vector<DiaryEntry*>)
  + showSearchMenu() -> int
  + promptInput(prompt: string) -> string
  + clearScreen()
  + printError(msg: string)
  + printSuccess(msg: string)
```

### 2.10 DiaryApp

```
Class: DiaryApp
Responsibility: Integrate all modules and manage application lifecycle

Attributes:
  - accountManager: AccountManager
  - storage: JSONStorage
  - searchEngine: SearchEngine
  - ui: TerminalUI
  - entries: vector<unique_ptr<BaseEntry>>

Methods:
  + run()
  + addEntry()
  + deleteEntry()
  + editEntry()
  + searchEntries()
  + listEntries()
  + saveAndExit()
```

## 3. 開發迭代計劃

| 迭代 | 分支名稱 | 交付內容 | 依賴 |
|------|----------|----------|------|
| Sprint 1 | `feature/core-models` | BaseEntry、DiaryEntry、SystemLog 類別及其序列化 | 無 |
| Sprint 2 | `feature/account-system` | Account、AccountManager 帳號管理 | Sprint 1 |
| Sprint 3 | `feature/storage` | Storage、JSONStorage 持久化層 | Sprint 1 |
| Sprint 4 | `feature/search` | SearchEngine 搜尋引擎 | Sprint 1 |
| Sprint 5 | `feature/ui` | TerminalUI 終端介面 | Sprint 1 |
| Sprint 6 | `feature/integration` | DiaryApp 整合、main.cpp、單元測試 | Sprint 1-5 |

## 4. 測試規格

| 測試模組 | 測試內容 |
|----------|----------|
| EntryTest | 驗證 BaseEntry 多型、DiaryEntry 序列化/反序列化、SystemLog 序列化 |
| AccountTest | 驗證帳號註冊、登入、密碼雜湊、密碼驗證 |
| StorageTest | 驗證 JSON 存檔、讀檔、資料完整性 |
| SearchTest | 驗證依日期、關鍵字、標籤搜尋 |
