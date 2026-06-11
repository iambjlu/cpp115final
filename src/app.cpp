#include "app.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>
#include <sys/stat.h>

DiaryApp::DiaryApp(const std::string& dataDir)
    : storage_(dataDir + "/entries.json") {
    // Ensure data directory exists
    mkdir("data", 0755);
    accountManager_.load();
}

void DiaryApp::loadEntries() {
    entries_ = storage_.load_entries();
}

void DiaryApp::saveEntries() {
    storage_.save_entries(entries_);
}

std::vector<DiaryEntry*> DiaryApp::getOwnerDiaries(const std::string& ownerId) {
    std::vector<DiaryEntry*> result;
    for (auto& entry : entries_) {
        if (entry->type() == "diary") {
            DiaryEntry* de = static_cast<DiaryEntry*>(entry.get());
            if (de->ownerId() == ownerId) {
                result.push_back(de);
            }
        }
    }
    return result;
}

void DiaryApp::addEntry() {
    ui_.clearScreen();
    std::cout << "\033[1;35m===== 新增日記 =====\033[0m\n";

    std::string title = ui_.promptInput("日記標題");
    if (title.empty()) {
        ui_.printError("標題不能為空");
        ui_.pressEnterToContinue();
        return;
    }

    std::cout << "\033[1;36m日記內容 (輸入空行結束):\033[0m\n";
    std::string content, line;
    while (std::getline(std::cin, line) && !line.empty()) {
        if (!content.empty()) content += "\n";
        content += line;
    }

    if (content.empty()) {
        ui_.printError("內容不能為空");
        ui_.pressEnterToContinue();
        return;
    }

    std::string date = ui_.promptInput("日期 (YYYY-MM-DD)");
    if (date.empty()) {
        // Default to today
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
        date = buf;
    }

    std::string tagInput = ui_.promptInput("標籤 (以逗號分隔，可留空)");

    auto entry = std::make_unique<DiaryEntry>();
    entry->setTitle(title);
    entry->setContent(content);
    entry->setDate(date);
    entry->setOwnerId(accountManager_.getCurrentUser()->id);

    if (!tagInput.empty()) {
        size_t pos = 0;
        while ((pos = tagInput.find(',', pos)) != std::string::npos || pos == 0) {
            if (pos == 0 && tagInput.find(',', 0) == std::string::npos) {
                entry->addTag(tagInput);
                break;
            }
            size_t start = (pos == 0) ? 0 : pos + 1;
            if (pos == 0 && tagInput.find(',', 0) != std::string::npos) {
                // first comma found
            }
            pos = tagInput.find(',', pos);
            if (pos == std::string::npos) {
                std::string tag = tagInput.substr(start);
                // Trim
                while (!tag.empty() && tag.front() == ' ') tag.erase(0, 1);
                while (!tag.empty() && tag.back() == ' ') tag.pop_back();
                if (!tag.empty()) entry->addTag(tag);
                break;
            }
        }
        // Simpler approach: re-parse
        entry = std::make_unique<DiaryEntry>();
        entry->setTitle(title);
        entry->setContent(content);
        entry->setDate(date);
        entry->setOwnerId(accountManager_.getCurrentUser()->id);

        size_t commaPos = 0;
        while ((commaPos = tagInput.find(',', commaPos)) != std::string::npos) {
            std::string tag = tagInput.substr(0, commaPos);
            while (!tag.empty() && tag.front() == ' ') tag.erase(0, 1);
            while (!tag.empty() && tag.back() == ' ') tag.pop_back();
            if (!tag.empty()) entry->addTag(tag);
            tagInput.erase(0, commaPos + 1);
        }
        while (!tagInput.empty() && tagInput.front() == ' ') tagInput.erase(0, 1);
        while (!tagInput.empty() && tagInput.back() == ' ') tagInput.pop_back();
        if (!tagInput.empty()) entry->addTag(tagInput);
    }

    entries_.push_back(std::move(entry));

    // Add system log
    auto log = std::make_unique<SystemLog>();
    log->setLevel("INFO");
    log->setMessage("New diary entry created: " + title);
    entries_.push_back(std::move(log));

    saveEntries();
    ui_.printSuccess("日記已成功新增");
    ui_.pressEnterToContinue();
}

void DiaryApp::deleteEntry() {
    auto diaries = getOwnerDiaries(accountManager_.getCurrentUser()->id);
    ui_.showDiaryList(diaries);

    if (diaries.empty()) {
        ui_.pressEnterToContinue();
        return;
    }

    int idx = ui_.promptNumber("選擇要刪除的日記編號", 1, (int)diaries.size());
    std::string targetId = diaries[idx - 1]->id();

    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(),
            [&targetId](const std::unique_ptr<BaseEntry>& e) {
                return e->type() == "diary" && e->id() == targetId;
            }),
        entries_.end()
    );

    // Add system log
    auto log = std::make_unique<SystemLog>();
    log->setLevel("INFO");
    log->setMessage("Diary entry deleted: " + targetId);
    entries_.push_back(std::move(log));

    saveEntries();
    ui_.printSuccess("日記已成功刪除");
    ui_.pressEnterToContinue();
}

void DiaryApp::editEntry() {
    auto diaries = getOwnerDiaries(accountManager_.getCurrentUser()->id);
    ui_.showDiaryList(diaries);

    if (diaries.empty()) {
        ui_.pressEnterToContinue();
        return;
    }

    int idx = ui_.promptNumber("選擇要修改的日記編號", 1, (int)diaries.size());
    DiaryEntry* target = diaries[idx - 1];

    ui_.clearScreen();
    std::cout << "\033[1;35m===== 修改日記 =====\033[0m\n";
    std::cout << "目前標題: " << target->title() << "\n";
    std::string newTitle = ui_.promptInput("新標題 (留空保持不變)");
    if (!newTitle.empty()) target->setTitle(newTitle);

    std::cout << "目前內容:\n" << target->content() << "\n";
    std::cout << "\033[1;36m新內容 (輸入空行結束, 留空保持不變):\033[0m\n";
    std::string newContent, line;
    bool hasContent = false;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            if (hasContent) break;
            // First empty line means no new content
            break;
        }
        hasContent = true;
        if (!newContent.empty()) newContent += "\n";
        newContent += line;
    }
    if (!newContent.empty()) target->setContent(newContent);

    // Add system log
    auto log = std::make_unique<SystemLog>();
    log->setLevel("INFO");
    log->setMessage("Diary entry edited: " + target->id());
    entries_.push_back(std::move(log));

    saveEntries();
    ui_.printSuccess("日記已成功修改");
    ui_.pressEnterToContinue();
}

void DiaryApp::searchEntries() {
    int choice = ui_.showSearchMenu();

    auto diaries = getOwnerDiaries(accountManager_.getCurrentUser()->id);
    std::vector<DiaryEntry*> results;

    if (choice == 0) return;

    if (choice == 1) {
        std::string date = ui_.promptInput("搜尋日期 (YYYY-MM-DD)");
        results = searchEngine_.searchByDate(entries_, date);
    } else if (choice == 2) {
        std::string keyword = ui_.promptInput("搜尋關鍵字");
        results = searchEngine_.searchByKeyword(entries_, keyword);
    } else if (choice == 3) {
        std::string tag = ui_.promptInput("搜尋標籤");
        results = searchEngine_.searchByTag(entries_, tag);
    }

    // Filter results to only show owner's diaries
    std::vector<DiaryEntry*> filtered;
    for (auto* r : results) {
        if (r->ownerId() == accountManager_.getCurrentUser()->id) {
            filtered.push_back(r);
        }
    }

    ui_.showDiaryList(filtered);
    ui_.pressEnterToContinue();
}

void DiaryApp::listEntries() {
    auto diaries = getOwnerDiaries(accountManager_.getCurrentUser()->id);
    ui_.showDiaryList(diaries);
    ui_.pressEnterToContinue();
}

void DiaryApp::deleteAccount() {
    ui_.clearScreen();
    std::cout << "\033[1;35m===== 刪除帳號 =====\033[0m\n";
    ui_.printError("警告：此操作將永久刪除您的帳號及所有日記");
    std::string password = ui_.promptPassword("請輸入密碼確認");

    // Save ownerId before deletion because deleteAccount nullifies currentUser_
    std::string ownerId = accountManager_.getCurrentUser()->id;

    if (accountManager_.deleteAccount(password)) {
        // Remove all diary entries belonging to this account
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                [&ownerId](const std::unique_ptr<BaseEntry>& e) {
                    return e->type() == "diary" &&
                           static_cast<DiaryEntry*>(e.get())->ownerId() == ownerId;
                }),
            entries_.end()
        );

        saveEntries();
        accountManager_.save();
        ui_.printSuccess("帳號已成功刪除");
        ui_.pressEnterToContinue();
    } else {
        ui_.printError("密碼錯誤，刪除取消");
        ui_.pressEnterToContinue();
    }
}

void DiaryApp::saveAndExit() {
    saveEntries();
    accountManager_.save();
    accountManager_.logout();
    ui_.printSuccess("資料已儲存，再見！");
}

void DiaryApp::run() {
    loadEntries();

    // Add startup log
    auto log = std::make_unique<SystemLog>();
    log->setLevel("INFO");
    log->setMessage("Application started");
    entries_.push_back(std::move(log));

    bool running = true;
    while (running) {
        if (!accountManager_.isLoggedIn()) {
            // Main menu
            ui_.showMainMenu();
            int choice = ui_.promptNumber("請選擇", 1, 3);

            if (choice == 1) {
                auto [user, pass] = ui_.showLoginScreen();
                if (accountManager_.login(user, pass)) {
                    ui_.printSuccess("登入成功！歡迎 " + user);
                } else {
                    ui_.printError("帳號或密碼錯誤");
                }
                ui_.pressEnterToContinue();
            } else if (choice == 2) {
                auto [user, pass] = ui_.showRegisterScreen();
                if (accountManager_.registerAccount(user, pass)) {
                    ui_.printSuccess("註冊成功！正在登入...");
                    accountManager_.login(user, pass);
                    accountManager_.save();
                } else {
                    ui_.printError("使用者名稱已存在");
                }
                ui_.pressEnterToContinue();
            } else {
                saveAndExit();
                running = false;
            }
        } else {
            // Feature menu
            ui_.showFeatureMenu();
            int choice = ui_.promptNumber("請選擇", 1, 8);

            switch (choice) {
                case 1: addEntry(); break;
                case 2: listEntries(); break;
                case 3: searchEntries(); break;
                case 4: editEntry(); break;
                case 5: deleteEntry(); break;
                case 6:
                    deleteAccount();
                    // After deletion, user is logged out, reload entries
                    loadEntries();
                    break;
                case 7:
                    accountManager_.logout();
                    ui_.printSuccess("已登出");
                    ui_.pressEnterToContinue();
                    break;
                case 8:
                    saveAndExit();
                    running = false;
                    break;
            }
        }
    }
}
