#include "search.h"
#include <algorithm>
#include <cctype>

std::string SearchEngine::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<DiaryEntry*> SearchEngine::searchByDate(
    const std::vector<std::unique_ptr<BaseEntry>>& entries,
    const std::string& date) {
    std::vector<DiaryEntry*> result;
    for (auto& entry : entries) {
        if (entry->type() == "diary") {
            DiaryEntry* de = static_cast<DiaryEntry*>(entry.get());
            if (de->date() == date) {
                result.push_back(de);
            }
        }
    }
    return result;
}

std::vector<DiaryEntry*> SearchEngine::searchByKeyword(
    const std::vector<std::unique_ptr<BaseEntry>>& entries,
    const std::string& keyword) {
    std::vector<DiaryEntry*> result;
    std::string lowerKeyword = toLower(keyword);
    for (auto& entry : entries) {
        if (entry->type() == "diary") {
            DiaryEntry* de = static_cast<DiaryEntry*>(entry.get());
            std::string lowerTitle = toLower(de->title());
            std::string lowerContent = toLower(de->content());
            if (lowerTitle.find(lowerKeyword) != std::string::npos ||
                lowerContent.find(lowerKeyword) != std::string::npos) {
                result.push_back(de);
            }
        }
    }
    return result;
}

std::vector<DiaryEntry*> SearchEngine::searchByTag(
    const std::vector<std::unique_ptr<BaseEntry>>& entries,
    const std::string& tag) {
    std::vector<DiaryEntry*> result;
    std::string lowerTag = toLower(tag);
    for (auto& entry : entries) {
        if (entry->type() == "diary") {
            DiaryEntry* de = static_cast<DiaryEntry*>(entry.get());
            for (auto& t : de->tags()) {
                if (toLower(t) == lowerTag) {
                    result.push_back(de);
                    break;
                }
            }
        }
    }
    return result;
}
