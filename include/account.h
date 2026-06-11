#pragma once

#include <string>
#include <map>
#include <memory>

class AccountManager;

struct Account {
    std::string id;
    std::string username;
    std::string passwordHash;
    std::string createdAt;

    Account();
    bool verifyPassword(const std::string& plainText) const;
    std::string to_json() const;
    void from_json(const std::string& json);

    friend class AccountManager;
};

class AccountManager {
private:
    std::map<std::string, Account> accounts_;  // keyed by username
    Account* currentUser_;

    static std::string generate_uuid();

public:
    AccountManager();
    ~AccountManager() = default;

    bool registerAccount(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
    void logout();
    Account* getCurrentUser() const { return currentUser_; }
    bool isLoggedIn() const { return currentUser_ != nullptr; }

    // Delete current account after password verification
    bool deleteAccount(const std::string& password);

    void save();
    void load();

    // Expose accounts for testing
    const std::map<std::string, Account>& getAccounts() const { return accounts_; }
};
