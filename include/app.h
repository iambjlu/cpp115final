#pragma once

#include "entry.h"
#include "account.h"
#include "storage.h"
#include "search.h"
#include "ui.h"
#include <vector>
#include <memory>
#include <string>

class DiaryApp {
private:
    AccountManager accountManager_;
    JSONStorage storage_;
    SearchEngine searchEngine_;
    TerminalUI ui_;
    std::vector<std::unique_ptr<BaseEntry>> entries_;

    void loadEntries();
    void saveEntries();

    // Feature methods
    void addEntry();
    void deleteEntry();
    void editEntry();
    void searchEntries();
    void listEntries();

    // Helper to filter diary entries by owner
    std::vector<DiaryEntry*> getOwnerDiaries(const std::string& ownerId);

public:
    explicit DiaryApp(const std::string& dataDir);
    ~DiaryApp() = default;

    void run();
    void saveAndExit();
};
