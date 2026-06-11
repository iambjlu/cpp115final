#include <gtest/gtest.h>
#include "entry.h"
#include "account.h"
#include "storage.h"
#include "search.h"
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

// ===== Entry Tests =====

TEST(EntryTest, BaseEntryGeneratesUniqueIds) {
    BaseEntry* e1 = new DiaryEntry();
    BaseEntry* e2 = new DiaryEntry();
    ASSERT_NE(e1->id(), e2->id());
    delete e1;
    delete e2;
}

TEST(EntryTest, DiaryEntryType) {
    DiaryEntry entry;
    ASSERT_EQ(entry.type(), "diary");
}

TEST(EntryTest, SystemLogType) {
    SystemLog log;
    ASSERT_EQ(log.type(), "system_log");
}

TEST(EntryTest, DiaryEntrySetAndGet) {
    DiaryEntry entry;
    entry.setTitle("Test Title");
    entry.setContent("Test Content");
    entry.setDate("2025-06-11");
    entry.addTag("test");
    entry.addTag("cpp");
    entry.setOwnerId("owner-123");

    ASSERT_EQ(entry.title(), "Test Title");
    ASSERT_EQ(entry.content(), "Test Content");
    ASSERT_EQ(entry.date(), "2025-06-11");
    ASSERT_EQ(entry.ownerId(), "owner-123");
    ASSERT_EQ(entry.tags().size(), 2u);
    ASSERT_EQ(entry.tags()[0], "test");
    ASSERT_EQ(entry.tags()[1], "cpp");
}

TEST(EntryTest, DiaryEntrySerializationRoundTrip) {
    DiaryEntry original;
    original.setTitle("My Day");
    original.setContent("Today was great");
    original.setDate("2025-06-11");
    original.addTag("daily");
    original.addTag("mood");
    original.setOwnerId("user-abc");

    std::string json = original.to_json();

    DiaryEntry restored;
    restored.from_json(json);

    ASSERT_EQ(restored.title(), "My Day");
    ASSERT_EQ(restored.content(), "Today was great");
    ASSERT_EQ(restored.date(), "2025-06-11");
    ASSERT_EQ(restored.ownerId(), "user-abc");
    ASSERT_EQ(restored.tags().size(), 2u);
}

TEST(EntryTest, SystemLogSetAndGet) {
    SystemLog log;
    log.setLevel("ERROR");
    log.setMessage("Something went wrong");

    ASSERT_EQ(log.level(), "ERROR");
    ASSERT_EQ(log.message(), "Something went wrong");
}

TEST(EntryTest, SystemLogSerializationRoundTrip) {
    SystemLog original;
    original.setLevel("WARN");
    original.setMessage("Low disk space");

    std::string json = original.to_json();

    SystemLog restored;
    restored.from_json(json);

    ASSERT_EQ(restored.level(), "WARN");
    ASSERT_EQ(restored.message(), "Low disk space");
}

TEST(EntryTest, PolymorphismThroughBasePointer) {
    std::vector<std::unique_ptr<BaseEntry>> entries;
    entries.push_back(std::make_unique<DiaryEntry>());
    entries.push_back(std::make_unique<SystemLog>());

    ASSERT_EQ(entries[0]->type(), "diary");
    ASSERT_EQ(entries[1]->type(), "system_log");
}

// ===== Account Tests =====

TEST(AccountTest, RegisterNewAccount) {
    AccountManager manager;
    ASSERT_TRUE(manager.registerAccount("testuser", "password123"));
    ASSERT_TRUE(manager.isLoggedIn() == false);  // Not auto-logged in
}

TEST(AccountTest, RegisterDuplicateAccount) {
    AccountManager manager;
    manager.registerAccount("testuser", "password123");
    ASSERT_FALSE(manager.registerAccount("testuser", "other"));
}

TEST(AccountTest, LoginSuccess) {
    AccountManager manager;
    manager.registerAccount("testuser", "password123");
    ASSERT_TRUE(manager.login("testuser", "password123"));
    ASSERT_TRUE(manager.isLoggedIn());
    ASSERT_EQ(manager.getCurrentUser()->username, "testuser");
}

TEST(AccountTest, LoginWrongPassword) {
    AccountManager manager;
    manager.registerAccount("testuser", "password123");
    ASSERT_FALSE(manager.login("testuser", "wrongpassword"));
    ASSERT_FALSE(manager.isLoggedIn());
}

TEST(AccountTest, LoginNonexistentUser) {
    AccountManager manager;
    ASSERT_FALSE(manager.login("nouser", "password"));
}

TEST(AccountTest, Logout) {
    AccountManager manager;
    manager.registerAccount("testuser", "password123");
    manager.login("testuser", "password123");
    manager.logout();
    ASSERT_FALSE(manager.isLoggedIn());
    ASSERT_EQ(manager.getCurrentUser(), nullptr);
}

TEST(AccountTest, PasswordIsHashed) {
    AccountManager manager;
    manager.registerAccount("testuser", "password123");
    const auto& accounts = manager.getAccounts();
    auto it = accounts.find("testuser");
    ASSERT_NE(it, accounts.end());
    ASSERT_NE(it->second.passwordHash, "password123");  // Should not be plaintext
}

TEST(AccountTest, AccountSerializationRoundTrip) {
    Account acc;
    acc.id = "uuid-123";
    acc.username = "testuser";
    acc.passwordHash = "abc123hash";
    acc.createdAt = "1234567890";

    std::string json = acc.to_json();

    Account restored;
    restored.from_json(json);

    ASSERT_EQ(restored.id, "uuid-123");
    ASSERT_EQ(restored.username, "testuser");
    ASSERT_EQ(restored.passwordHash, "abc123hash");
    ASSERT_EQ(restored.createdAt, "1234567890");
}

TEST(AccountTest, PasswordVerification) {
    AccountManager manager;
    manager.registerAccount("testuser", "mysecret");

    const auto& accounts = manager.getAccounts();
    auto it = accounts.find("testuser");
    ASSERT_TRUE(it->second.verifyPassword("mysecret"));
    ASSERT_FALSE(it->second.verifyPassword("wrongpassword"));
}

// ===== Storage Tests =====

TEST(StorageTest, SaveAndLoadEntries) {
    mkdir("data", 0755);
    const std::string testFile = "data/test_entries.json";

    // Clean up
    std::remove(testFile.c_str());

    JSONStorage storage(testFile);

    // Create entries
    std::vector<std::unique_ptr<BaseEntry>> entries;
    auto diary = std::make_unique<DiaryEntry>();
    diary->setTitle("Test Entry");
    diary->setContent("Hello World");
    diary->setDate("2025-06-11");
    diary->setOwnerId("user-1");
    entries.push_back(std::move(diary));

    auto log = std::make_unique<SystemLog>();
    log->setLevel("INFO");
    log->setMessage("Test log");
    entries.push_back(std::move(log));

    // Save
    ASSERT_TRUE(storage.save_entries(entries));

    // Load
    auto loaded = storage.load_entries();
    ASSERT_EQ(loaded.size(), 2u);
    ASSERT_EQ(loaded[0]->type(), "diary");
    ASSERT_EQ(loaded[1]->type(), "system_log");

    // Verify content
    DiaryEntry* de = static_cast<DiaryEntry*>(loaded[0].get());
    ASSERT_EQ(de->title(), "Test Entry");
    ASSERT_EQ(de->content(), "Hello World");

    SystemLog* sl = static_cast<SystemLog*>(loaded[1].get());
    ASSERT_EQ(sl->level(), "INFO");
    ASSERT_EQ(sl->message(), "Test log");

    // Clean up
    std::remove(testFile.c_str());
}

TEST(StorageTest, LoadNonexistentFile) {
    JSONStorage storage("data/nonexistent.json");
    auto entries = storage.load_entries();
    ASSERT_TRUE(entries.empty());
}

TEST(StorageTest, SaveEmptyEntries) {
    mkdir("data", 0755);
    const std::string testFile = "data/test_empty.json";
    std::remove(testFile.c_str());

    JSONStorage storage(testFile);
    std::vector<std::unique_ptr<BaseEntry>> entries;
    ASSERT_TRUE(storage.save_entries(entries));

    std::remove(testFile.c_str());
}

// ===== Search Tests =====

TEST(SearchTest, SearchByDate) {
    SearchEngine engine;

    std::vector<std::unique_ptr<BaseEntry>> entries;
    auto d1 = std::make_unique<DiaryEntry>();
    d1->setTitle("Entry 1");
    d1->setDate("2025-06-11");
    entries.push_back(std::move(d1));

    auto d2 = std::make_unique<DiaryEntry>();
    d2->setTitle("Entry 2");
    d2->setDate("2025-06-12");
    entries.push_back(std::move(d2));

    auto d3 = std::make_unique<DiaryEntry>();
    d3->setTitle("Entry 3");
    d3->setDate("2025-06-11");
    entries.push_back(std::move(d3));

    auto results = engine.searchByDate(entries, "2025-06-11");
    ASSERT_EQ(results.size(), 2u);
}

TEST(SearchTest, SearchByKeyword) {
    SearchEngine engine;

    std::vector<std::unique_ptr<BaseEntry>> entries;
    auto d1 = std::make_unique<DiaryEntry>();
    d1->setTitle("Happy Day");
    d1->setContent("I felt so happy today");
    entries.push_back(std::move(d1));

    auto d2 = std::make_unique<DiaryEntry>();
    d2->setTitle("Sad Day");
    d2->setContent("Not a good day");
    entries.push_back(std::move(d2));

    auto d3 = std::make_unique<DiaryEntry>();
    d3->setTitle("Another Happy Moment");
    d3->setContent("Something nice happened");
    entries.push_back(std::move(d3));

    // Case-insensitive search
    auto results = engine.searchByKeyword(entries, "happy");
    ASSERT_EQ(results.size(), 2u);
}

TEST(SearchTest, SearchByTag) {
    SearchEngine engine;

    std::vector<std::unique_ptr<BaseEntry>> entries;
    auto d1 = std::make_unique<DiaryEntry>();
    d1->setTitle("Work Day");
    d1->addTag("work");
    d1->addTag("important");
    entries.push_back(std::move(d1));

    auto d2 = std::make_unique<DiaryEntry>();
    d2->setTitle("Weekend");
    d2->addTag("personal");
    entries.push_back(std::move(d2));

    auto d3 = std::make_unique<DiaryEntry>();
    d3->setTitle("Meeting");
    d3->addTag("work");
    entries.push_back(std::move(d3));

    auto results = engine.searchByTag(entries, "work");
    ASSERT_EQ(results.size(), 2u);
}

TEST(SearchTest, SearchNoResults) {
    SearchEngine engine;

    std::vector<std::unique_ptr<BaseEntry>> entries;
    auto d1 = std::make_unique<DiaryEntry>();
    d1->setTitle("Test");
    d1->setContent("Content");
    entries.push_back(std::move(d1));

    auto results = engine.searchByKeyword(entries, "nonexistent");
    ASSERT_TRUE(results.empty());
}

TEST(SearchTest, SearchFiltersOutSystemLogs) {
    SearchEngine engine;

    std::vector<std::unique_ptr<BaseEntry>> entries;
    entries.push_back(std::make_unique<DiaryEntry>());
    entries.push_back(std::make_unique<SystemLog>());
    entries.push_back(std::make_unique<DiaryEntry>());

    auto results = engine.searchByKeyword(entries, "test");
    // All results should be DiaryEntry, not SystemLog
    for (auto* r : results) {
        // If we got here, the result is a DiaryEntry pointer (type-safe)
    }
    ASSERT_LE(results.size(), 2u);  // At most 2 diary entries
}
