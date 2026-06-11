#include "storage.h"
#include <fstream>
#include <sstream>
#include <algorithm>

JSONStorage::JSONStorage(const std::string& filepath)
    : filepath_(filepath) {}

std::string JSONStorage::escape_json(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c; break;
        }
    }
    return result;
}

std::string JSONStorage::unescape_json(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"':  result += '"'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case 'n':  result += '\n'; ++i; break;
                case 'r':  result += '\r'; ++i; break;
                case 't':  result += '\t'; ++i; break;
                default:   result += s[i]; break;
            }
        } else {
            result += s[i];
        }
    }
    return result;
}

std::string JSONStorage::extract_json_field(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\":\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    size_t end = pos;
    while (end < json.size()) {
        if (json[end] == '"') {
            if (end > 0 && json[end - 1] == '\\') {
                ++end;
                continue;
            }
            break;
        }
        ++end;
    }
    return unescape_json(json.substr(pos, end - pos));
}

bool JSONStorage::save_entries(const std::vector<std::unique_ptr<BaseEntry>>& entries) {
    std::ofstream file(filepath_);
    if (!file.is_open()) return false;

    file << "[";
    for (size_t i = 0; i < entries.size(); ++i) {
        if (i > 0) file << ",";
        file << entries[i]->to_json();
    }
    file << "]";
    file.close();
    return true;
}

std::vector<std::unique_ptr<BaseEntry>> JSONStorage::load_entries() {
    std::vector<std::unique_ptr<BaseEntry>> result;
    std::ifstream file(filepath_);
    if (!file.is_open()) return result;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    file.close();

    // Parse each JSON object
    size_t pos = 0;
    while ((pos = data.find("{", pos)) != std::string::npos) {
        // Find matching closing brace
        int depth = 0;
        size_t start = pos;
        size_t end = pos;
        for (size_t i = pos; i < data.size(); ++i) {
            if (data[i] == '{') ++depth;
            else if (data[i] == '}') {
                --depth;
                if (depth == 0) {
                    end = i;
                    break;
                }
            }
        }
        if (depth != 0) break;

        std::string obj = data.substr(start, end - start + 1);
        std::string type = extract_json_field(obj, "type");

        if (type == "diary") {
            auto entry = std::make_unique<DiaryEntry>();
            entry->from_json(obj);
            result.push_back(std::move(entry));
        } else if (type == "system_log") {
            auto entry = std::make_unique<SystemLog>();
            entry->from_json(obj);
            result.push_back(std::move(entry));
        }

        pos = end + 1;
    }

    return result;
}
