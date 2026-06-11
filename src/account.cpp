#include "account.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <random>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#endif

static std::string hashPassword(const std::string& password) {
#ifdef __APPLE__
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(password.c_str(), password.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    }
    return oss.str();
#else
    // Fallback: simple hash for non-macOS platforms
    std::hash<std::string> hasher;
    size_t h = hasher(password);
    return std::to_string(h);
#endif
}

Account::Account()
    : id(), username(), passwordHash(), createdAt() {}

bool Account::verifyPassword(const std::string& plainText) const {
    return passwordHash == hashPassword(plainText);
}

std::string AccountManager::generate_uuid() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    uint64_t a = dis(gen);
    uint64_t b = dis(gen);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << a << "-" << std::setw(16) << b;
    return oss.str();
}

AccountManager::AccountManager()
    : currentUser_(nullptr) {}

bool AccountManager::registerAccount(const std::string& username, const std::string& password) {
    if (accounts_.find(username) != accounts_.end()) {
        return false;  // Already exists
    }
    Account acc;
    acc.id = generate_uuid();
    acc.username = username;
    acc.passwordHash = hashPassword(password);
    acc.createdAt = std::to_string(std::time(nullptr));
    accounts_[username] = acc;
    return true;
}

bool AccountManager::login(const std::string& username, const std::string& password) {
    auto it = accounts_.find(username);
    if (it == accounts_.end()) {
        return false;
    }
    if (!it->second.verifyPassword(password)) {
        return false;
    }
    currentUser_ = &(it->second);
    return true;
}

void AccountManager::logout() {
    currentUser_ = nullptr;
}

bool AccountManager::deleteAccount(const std::string& password) {
    if (!currentUser_) {
        return false;
    }
    if (!currentUser_->verifyPassword(password)) {
        return false;
    }
    accounts_.erase(currentUser_->username);
    currentUser_ = nullptr;
    return true;
}

void AccountManager::save() {
    std::ofstream file("data/accounts.json");
    if (!file.is_open()) return;

    file << "[";
    bool first = true;
    for (auto& [name, acc] : accounts_) {
        if (!first) file << ",";
        file << acc.to_json();
        first = false;
    }
    file << "]";
    file.close();
}

void AccountManager::load() {
    std::ifstream file("data/accounts.json");
    if (!file.is_open()) return;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    file.close();

    // Parse simple JSON array of accounts
    size_t pos = 0;
    while ((pos = data.find("{", pos)) != std::string::npos) {
        size_t end = data.find("}", pos);
        if (end == std::string::npos) break;
        std::string obj = data.substr(pos, end - pos + 1);
        Account acc;
        acc.from_json(obj);
        accounts_[acc.username] = acc;
        pos = end + 1;
    }
}

std::string Account::to_json() const {
    std::ostringstream json;
    json << "{\"id\":\"" << id << "\""
         << ",\"username\":\"" << username << "\""
         << ",\"passwordHash\":\"" << passwordHash << "\""
         << ",\"createdAt\":\"" << createdAt << "\"}";
    return json.str();
}

void Account::from_json(const std::string& json) {
    auto extract = [&](const std::string& key) -> std::string {
        std::string pattern = "\"" + key + "\":\"";
        size_t p = json.find(pattern);
        if (p == std::string::npos) return "";
        p += pattern.size();
        size_t e = json.find("\"", p);
        if (e == std::string::npos) return "";
        return json.substr(p, e - p);
    };
    id = extract("id");
    username = extract("username");
    passwordHash = extract("passwordHash");
    createdAt = extract("createdAt");
}
