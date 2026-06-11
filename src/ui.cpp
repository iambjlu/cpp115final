#include "ui.h"
#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>

void TerminalUI::clearScreen() {
    std::cout << "\033[2J\033[H";
}

void TerminalUI::printError(const std::string& msg) {
    std::cout << "\033[1;31m[✗] " << msg << "\033[0m" << std::endl;
}

void TerminalUI::printSuccess(const std::string& msg) {
    std::cout << "\033[1;32m[✓] " << msg << "\033[0m" << std::endl;
}

void TerminalUI::printInfo(const std::string& msg) {
    std::cout << "\033[1;34m[ℹ] " << msg << "\033[0m" << std::endl;
}

std::string TerminalUI::promptInput(const std::string& prompt) {
    std::cout << "\033[1;36m" << prompt << ": \033[0m";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int TerminalUI::promptNumber(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << "\033[1;36m" << prompt << " (" << minVal << "-" << maxVal << "): \033[0m";
        int val;
        if (std::cin >> val && val >= minVal && val <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        printError("請輸入有效數字");
    }
}

void TerminalUI::pressEnterToContinue() {
    std::cout << "\033[1;33m按 Enter 鍵繼續...\033[0m";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void TerminalUI::showMainMenu() {
    clearScreen();
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║\033[1;35m         歡迎使用智慧日記本          \033[0m║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  1. 登入                            ║\n";
    std::cout << "║  2. 註冊新帳號                      ║\n";
    std::cout << "║  3. 離開                            ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

void TerminalUI::showFeatureMenu() {
    clearScreen();
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║\033[1;35m           功能選單                  \033[0m║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  1. 新增日記                        ║\n";
    std::cout << "║  2. 查看所有日記                    ║\n";
    std::cout << "║  3. 搜尋日記                        ║\n";
    std::cout << "║  4. 修改日記                        ║\n";
    std::cout << "║  5. 刪除日記                        ║\n";
    std::cout << "║  6. 登出                            ║\n";
    std::cout << "║  7. 儲存並離開                      ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

std::pair<std::string, std::string> TerminalUI::showLoginScreen() {
    clearScreen();
    std::cout << "\033[1;35m===== 登入 =====\033[0m\n";
    std::string username = promptInput("使用者名稱");
    std::string password = promptInput("密碼");
    return {username, password};
}

std::pair<std::string, std::string> TerminalUI::showRegisterScreen() {
    clearScreen();
    std::cout << "\033[1;35m===== 註冊新帳號 =====\033[0m\n";
    std::string username = promptInput("設定使用者名稱");
    std::string password = promptInput("設定密碼");
    return {username, password};
}

int TerminalUI::showSearchMenu() {
    clearScreen();
    std::cout << "\033[1;35m===== 搜尋日記 =====\033[0m\n";
    std::cout << "1. 依日期搜尋\n";
    std::cout << "2. 依關鍵字搜尋\n";
    std::cout << "3. 依標籤搜尋\n";
    std::cout << "0. 返回\n";
    return promptNumber("選擇搜尋方式", 0, 3);
}

void TerminalUI::showDiaryList(const std::vector<DiaryEntry*>& entries) {
    if (entries.empty()) {
        printInfo("沒有找到日記");
        return;
    }

    clearScreen();
    std::cout << "\033[1;35m===== 日記列表 =====\033[0m\n";
    std::cout << "共 " << entries.size() << " 筆記錄\n\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        std::cout << "\033[1;33m[" << (i + 1) << "]\033[0m ";
        std::cout << "\033[1;37m" << entries[i]->title() << "\033[0m";
        std::cout << " \033[0;37m(" << entries[i]->date() << ")\033[0m\n";

        // Show tags
        auto tags = entries[i]->tags();
        if (!tags.empty()) {
            std::cout << "   標籤: ";
            for (size_t j = 0; j < tags.size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << "\033[0;36m#" << tags[j] << "\033[0m";
            }
            std::cout << "\n";
        }

        // Show content preview
        std::string preview = entries[i]->content();
        if (preview.size() > 60) {
            preview = preview.substr(0, 60) + "...";
        }
        std::cout << "   " << preview << "\n\n";
    }
}
