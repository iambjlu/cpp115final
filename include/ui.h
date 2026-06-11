#pragma once

#include "entry.h"
#include <string>
#include <vector>

class TerminalUI {
public:
    TerminalUI() = default;
    ~TerminalUI() = default;

    void showMainMenu();
    std::pair<std::string, std::string> showLoginScreen();
    std::pair<std::string, std::string> showRegisterScreen();
    void showDiaryList(const std::vector<DiaryEntry*>& entries);
    int showSearchMenu();
    std::string promptInput(const std::string& prompt);
    void clearScreen();
    void printError(const std::string& msg);
    void printSuccess(const std::string& msg);
    void printInfo(const std::string& msg);
    void showFeatureMenu();
    int promptNumber(const std::string& prompt, int minVal, int maxVal);
    void pressEnterToContinue();
};
