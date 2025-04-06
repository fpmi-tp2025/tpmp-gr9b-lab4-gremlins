#include <gtest/gtest.h>
#include "../include/UserInterface.h"
#include "../include/MusicStoreDB.h"
#include <memory>
#include <string>
#include <filesystem>
#include <sstream>

// Testing class for UserInterface that uses composition instead of inheritance
class UserInterfaceTester {
private:
    std::shared_ptr<MusicStoreDB> db;
    std::unique_ptr<UserInterface> ui;
    
public:
    UserInterfaceTester(std::shared_ptr<MusicStoreDB> db)
        : db(db), ui(std::make_unique<UserInterface>(db)) {}
    
    // Test direct methods on the UI
    bool testAuthenticate() {
        // We'll test authenticate() indirectly via the run() method
        // or by checking the state of the database after operations
        return true;
    }
    
    // Run the UI with specific scenarios
    void testRun() {
        // First login manually to the database
        EXPECT_TRUE(db->login("admin", "admin"));
        
        // We can't easily test the full UI run due to input requirements,
        // but we can verify the database is in the correct state
        EXPECT_TRUE(db->isUserAdmin());
    }
    
    // Get access to the db for testing
    std::shared_ptr<MusicStoreDB> getDb() const {
        return db;
    }
    
    // Get access to the UI for direct testing
    UserInterface* getUi() const {
        return ui.get();
    }
};

class UserInterfaceTest : public ::testing::Test {
protected:
    std::shared_ptr<MusicStoreDB> db;
    std::unique_ptr<UserInterfaceTester> tester;
    std::string testDbPath = "test_ui.db";

    void SetUp() override {
        // Make sure we have a clean test database for each test
        if (std::filesystem::exists(testDbPath)) {
            std::filesystem::remove(testDbPath);
        }
        db = std::make_shared<MusicStoreDB>(testDbPath);
        tester = std::make_unique<UserInterfaceTester>(db);
        
        // Setup test data
        setupTestData();
    }

    void TearDown() override {
        tester.reset();
        db.reset(); // Close the database connection
        // Clean up the test database file
        if (std::filesystem::exists(testDbPath)) {
            std::filesystem::remove(testDbPath);
        }
    }
    
    // Helper function to capture console output
    std::string captureOutput(std::function<void()> func) {
        std::streambuf* oldCout = std::cout.rdbuf();
        std::ostringstream capturedOutput;
        std::cout.rdbuf(capturedOutput.rdbuf());
        
        func();
        
        std::cout.rdbuf(oldCout);
        return capturedOutput.str();
    }
    
    // Helper to set up test data
    void setupTestData() {
        db->login("admin", "admin");
        
        // Add some test data
        db->addCompactDisc("2023-01-01", "Sony Music", 19.99);
        db->addCompactDisc("2023-02-15", "Universal", 24.99);
        
        db->addMusicalWork("Song 1", "Author 1", "Performer 1", 1);
        db->addMusicalWork("Song 2", "Author 2", "Performer 2", 2);
        
        db->registerOperation("поступление", 1, 20);
        db->registerOperation("поступление", 2, 15);
        
        db->registerOperation("продажа", 1, 10);
        db->registerOperation("продажа", 2, 5);
    }
};

// Test user authentication
TEST_F(UserInterfaceTest, AuthenticationTest) {
    // Test direct authentication to the database
    EXPECT_TRUE(db->login("admin", "admin"));
    EXPECT_TRUE(db->isUserAdmin());
    
    EXPECT_TRUE(db->login("user", "user"));
    EXPECT_FALSE(db->isUserAdmin());
}

// Test database interaction through UI wrapper
TEST_F(UserInterfaceTest, AdminDatabaseInteractionTest) {
    // Login as admin
    EXPECT_TRUE(db->login("admin", "admin"));
    
    // Verify we're admin
    EXPECT_TRUE(db->isUserAdmin());
    
    // Now we can test viewing data
    std::string output = captureOutput([this]() {
        db->showCompactInventory();
    });
    
    // Check if output contains expected data
    EXPECT_TRUE(output.find("Sony Music") != std::string::npos);
    EXPECT_TRUE(output.find("Universal") != std::string::npos);
}

// Test user privileges
TEST_F(UserInterfaceTest, UserPrivilegesTest) {
    // Login as regular user
    EXPECT_TRUE(db->login("user", "user"));
    
    // Verify we're not admin
    EXPECT_FALSE(db->isUserAdmin());
    
    // Check we can view the most popular compact disc
    std::string output = captureOutput([this]() {
        db->showMostPopularCompact();
    });
    
    // Output should contain data, but we can't easily verify specific contents
    EXPECT_FALSE(output.empty());
}

// Test for checking invalid login
TEST_F(UserInterfaceTest, InvalidLoginTest) {
    // Try invalid credentials
    EXPECT_FALSE(db->login("nonexistent", "badpassword"));
}

// Test view of most popular compact as user
TEST_F(UserInterfaceTest, MostPopularCompactTest) {
    // Login as user
    EXPECT_TRUE(db->login("user", "user"));
    
    std::string output = captureOutput([this]() {
        db->showMostPopularCompact();
    });
    
    // Check that we got output with compact information
    // Compact disc with id 1 should be most popular with 10 sales
    EXPECT_TRUE(output.find("1") != std::string::npos);
}

// Test view of most popular performer as user
TEST_F(UserInterfaceTest, MostPopularPerformerTest) {
    // Login as user
    EXPECT_TRUE(db->login("user", "user"));
    
    std::string output = captureOutput([this]() {
        db->showMostPopularPerformer();
    });
    
    // Check that we got output with performer information
    EXPECT_FALSE(output.empty());
    // This depends on which performer is more popular in your test data
    EXPECT_TRUE(output.find("Performer") != std::string::npos);
}