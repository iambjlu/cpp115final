#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>

class BaseEntry {
protected:
    std::string id_;
    std::chrono::system_clock::time_point createdAt_;

public:
    BaseEntry();
    virtual ~BaseEntry() = default;

    virtual std::string type() const = 0;
    virtual std::string to_json() const = 0;
    virtual void from_json(const std::string& json) = 0;

    std::string id() const { return id_; }
    std::string createdAt() const;

    static std::string generate_id();
    static std::string timestamp_to_string(std::chrono::system_clock::time_point tp);
};

class DiaryEntry : public BaseEntry {
private:
    std::string title_;
    std::string content_;
    std::string date_;
    std::vector<std::string> tags_;
    std::string ownerId_;

public:
    DiaryEntry();
    ~DiaryEntry() override = default;

    std::string type() const override;
    std::string to_json() const override;
    void from_json(const std::string& json) override;

    // Getters and setters
    std::string title() const { return title_; }
    void setTitle(const std::string& title) { title_ = title; }
    std::string content() const { return content_; }
    void setContent(const std::string& content) { content_ = content; }
    std::string date() const { return date_; }
    void setDate(const std::string& date) { date_ = date; }
    std::vector<std::string> tags() const { return tags_; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void addTag(const std::string& tag);
    std::string ownerId() const { return ownerId_; }
    void setOwnerId(const std::string& id) { ownerId_ = id; }
};

class SystemLog : public BaseEntry {
private:
    std::string level_;
    std::string message_;

public:
    SystemLog();
    ~SystemLog() override = default;

    std::string type() const override;
    std::string to_json() const override;
    void from_json(const std::string& json) override;

    std::string level() const { return level_; }
    void setLevel(const std::string& level) { level_ = level; }
    std::string message() const { return message_; }
    void setMessage(const std::string& message) { message_ = message; }
};
