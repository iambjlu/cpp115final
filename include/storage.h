#pragma once

#include "entry.h"
#include <string>
#include <vector>
#include <memory>

class Storage {
public:
    virtual ~Storage() = default;
    virtual bool save_entries(const std::vector<std::unique_ptr<BaseEntry>>& entries) = 0;
    virtual std::vector<std::unique_ptr<BaseEntry>> load_entries() = 0;
};

class JSONStorage : public Storage {
private:
    std::string filepath_;

    // Simple JSON helper functions
    static std::string escape_json(const std::string& s);
    static std::string unescape_json(const std::string& s);
    static std::vector<std::string> split_json_array(const std::string& data);
    static std::string extract_json_field(const std::string& json, const std::string& key);

public:
    explicit JSONStorage(const std::string& filepath);
    ~JSONStorage() override = default;

    bool save_entries(const std::vector<std::unique_ptr<BaseEntry>>& entries) override;
    std::vector<std::unique_ptr<BaseEntry>> load_entries() override;
};
