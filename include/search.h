#pragma once

#include "entry.h"
#include <vector>
#include <string>

class SearchEngine {
private:
    static std::string toLower(const std::string& s);

public:
    SearchEngine() = default;
    ~SearchEngine() = default;

    std::vector<DiaryEntry*> searchByDate(
        const std::vector<std::unique_ptr<BaseEntry>>& entries,
        const std::string& date);

    std::vector<DiaryEntry*> searchByKeyword(
        const std::vector<std::unique_ptr<BaseEntry>>& entries,
        const std::string& keyword);

    std::vector<DiaryEntry*> searchByTag(
        const std::vector<std::unique_ptr<BaseEntry>>& entries,
        const std::string& tag);
};
