#include "entry.h"
#include <algorithm>

// BaseEntry
BaseEntry::BaseEntry()
    : id_(generate_id()), createdAt_(std::chrono::system_clock::now()) {}

std::string BaseEntry::createdAt() const {
    return timestamp_to_string(createdAt_);
}

std::string BaseEntry::generate_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    uint64_t a = dis(gen);
    uint64_t b = dis(gen);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << a << std::setw(16) << b;
    return oss.str();
}

std::string BaseEntry::timestamp_to_string(std::chrono::system_clock::time_point tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// DiaryEntry
DiaryEntry::DiaryEntry()
    : title_(""), content_(""), date_(""), ownerId_("") {}

std::string DiaryEntry::type() const { return "diary"; }

std::string DiaryEntry::to_json() const {
    std::ostringstream json;
    json << "{\"type\":\"" << type() << "\",\"id\":\"" << id_ << "\""
         << ",\"createdAt\":\"" << createdAt() << "\""
         << ",\"title\":\"" << title_ << "\""
         << ",\"content\":\"" << content_ << "\""
         << ",\"date\":\"" << date_ << "\""
         << ",\"ownerId\":\"" << ownerId_ << "\""
         << ",\"tags\":[";
    for (size_t i = 0; i < tags_.size(); ++i) {
        if (i > 0) json << ",";
        json << "\"" << tags_[i] << "\"";
    }
    json << "]}";
    return json.str();
}

void DiaryEntry::from_json(const std::string& json) {
    auto extract = [&](const std::string& key) -> std::string {
        std::string pattern = "\"" + key + "\":\"";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return "";
        pos += pattern.size();
        size_t end = json.find("\"", pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    };

    id_ = extract("id");
    title_ = extract("title");
    content_ = extract("content");
    date_ = extract("date");
    ownerId_ = extract("ownerId");

    // Parse tags
    tags_.clear();
    size_t tagsPos = json.find("\"tags\":[");
    if (tagsPos != std::string::npos) {
        tagsPos += 8;
        size_t endBrace = json.find("]", tagsPos);
        if (endBrace != std::string::npos) {
            std::string tagsStr = json.substr(tagsPos, endBrace - tagsPos);
            size_t pos = 0;
            while ((pos = tagsStr.find("\"", pos)) != std::string::npos) {
                size_t end = tagsStr.find("\"", pos + 1);
                if (end != std::string::npos) {
                    tags_.push_back(tagsStr.substr(pos + 1, end - pos - 1));
                    pos = end + 1;
                } else {
                    break;
                }
            }
        }
    }
}

void DiaryEntry::addTag(const std::string& tag) {
    tags_.push_back(tag);
}

// SystemLog
SystemLog::SystemLog()
    : level_("INFO"), message_("") {}

std::string SystemLog::type() const { return "system_log"; }

std::string SystemLog::to_json() const {
    std::ostringstream json;
    json << "{\"type\":\"" << type() << "\",\"id\":\"" << id_ << "\""
         << ",\"createdAt\":\"" << createdAt() << "\""
         << ",\"level\":\"" << level_ << "\""
         << ",\"message\":\"" << message_ << "\"}";
    return json.str();
}

void SystemLog::from_json(const std::string& json) {
    auto extract = [&](const std::string& key) -> std::string {
        std::string pattern = "\"" + key + "\":\"";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return "";
        pos += pattern.size();
        size_t end = json.find("\"", pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    };

    id_ = extract("id");
    level_ = extract("level");
    message_ = extract("message");
}
