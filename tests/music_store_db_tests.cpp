#include <gtest/gtest.h>
#include "../include/MusicStoreDB.h"
#include <memory>
#include <string>
#include <filesystem>
#include <sstream>

class MusicStoreDBTest : public ::testing::Test {
protected:
    std::shared_ptr<MusicStoreDB> db;
    std::string testDbPath = "test_db_specific.db";

    void SetUp() override {
        // Make sure we have a clean test database for each test
        if (std::filesystem::exists(testDbPath)) {
            std::filesystem::remove(testDbPath);
        }
        db = std::make_shared<MusicStoreDB>(testDbPath);
        
        // Login as admin for most tests
        db->login("admin", "admin");
    }

    void TearDown() override {
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
    
    // Helper function to capture console errors
    std::string captureError(std::function<void()> func) {
        std::streambuf* oldCerr = std::cerr.rdbuf();
        std::ostringstream capturedOutput;
        std::cerr.rdbuf(capturedOutput.rdbuf());
        
        func();
        
        std::cerr.rdbuf(oldCerr);
        return capturedOutput.str();
    }
    
    // Helper to set up test data
    void setupTestData() {
        // Add several compact discs
        db->addCompactDisc("2023-01-01", "Sony Music", 19.99);
        db->addCompactDisc("2023-02-15", "Universal", 24.99);
        db->addCompactDisc("2023-03-10", "Warner", 14.99);
        
        // Add musical works
        db->addMusicalWork("Song 1", "Author 1", "Performer 1", 1);
        db->addMusicalWork("Song 2", "Author 2", "Performer 2", 1);
        db->addMusicalWork("Song 3", "Author 1", "Performer 3", 2);
        db->addMusicalWork("Song 4", "Author 3", "Performer 1", 3);
        
        // Register inventory operations
        db->registerOperation("поступление", 1, 20);
        db->registerOperation("поступление", 2, 15);
        db->registerOperation("поступление", 3, 10);
        
        db->registerOperation("продажа", 1, 10);
        db->registerOperation("продажа", 2, 5);
        db->registerOperation("продажа", 3, 2);
    }
};

// Test database initialization
TEST_F(MusicStoreDBTest, InitializationTest) {
    // The database should already be initialized by SetUp()
    // We verify this by checking that default users exist
    EXPECT_TRUE(db->login("admin", "admin"));
    EXPECT_TRUE(db->isUserAdmin());
    
    EXPECT_TRUE(db->login("user", "user"));
    EXPECT_FALSE(db->isUserAdmin());
}

// Test compact disc management
TEST_F(MusicStoreDBTest, CompactDiscManagementTest) {
    // Add a new compact disc
    db->addCompactDisc("2023-04-20", "Test Label", 29.99);
    
    // Check if it appears in inventory
    std::string output = captureOutput([this]() { db->showCompactInventory(); });
    EXPECT_TRUE(output.find("Test Label") != std::string::npos);
    EXPECT_TRUE(output.find("29.9") != std::string::npos);
    
    // Update the compact disc
    db->updateCompactDisc(1, "Updated Label", 39.99);
    
    // Check if update is reflected
    output = captureOutput([this]() { db->showCompactInventory(); });
    EXPECT_TRUE(output.find("Updated Label") != std::string::npos);
    EXPECT_TRUE(output.find("39.9") != std::string::npos);
    
    // Delete the compact disc
    db->deleteCompactDisc(1);
    
    // Check if it's removed from inventory
    output = captureOutput([this]() { db->showCompactInventory(); });
    EXPECT_FALSE(output.find("Updated Label") != std::string::npos);
}

// Test musical work management
TEST_F(MusicStoreDBTest, MusicalWorkManagementTest) {
    // Add a compact disc first
    db->addCompactDisc("2023-04-20", "Test Label", 29.99);
    
    // Add a musical work
    db->addMusicalWork("Test Song", "Test Author", "Test Performer", 1);
    
    // We can't easily verify the musical work directly without complex queries
    // In a real test scenario, you might want to expose a method to list works
}

// Test operation management
TEST_F(MusicStoreDBTest, OperationManagementTest) {
    // Add a compact disc
    db->addCompactDisc("2023-04-20", "Test Label", 29.99);
    
    // Register receipt of items
    db->registerOperation("поступление", 1, 50);
    
    // Check inventory
    std::string output = captureOutput([this]() { db->showCompactInventory(); });
    EXPECT_TRUE(output.find("50") != std::string::npos);
    
    // Register sale
    db->registerOperation("продажа", 1, 20);
    
    // Check updated inventory
    output = captureOutput([this]() { db->showCompactInventory(); });
    // Should now show 30 remaining
    EXPECT_TRUE(output.find("30") != std::string::npos);
    
    // Try to sell more than available
    std::string error = captureError([this]() { 
        db->registerOperation("продажа", 1, 40); 
    });
    
    EXPECT_TRUE(error.find("SQL error") != std::string::npos);
}

// Test reports and statistics
TEST_F(MusicStoreDBTest, ReportsAndStatisticsTest) {
    setupTestData();
    
    // Test period statistics
    std::string output = captureOutput([this]() { 
        db->calculatePeriodStatistics("2023-01-01", "2026-12-31"); 
    });
    
    // Check if report contains correct data
    EXPECT_TRUE(output.find("Sony Music") != std::string::npos);
    EXPECT_TRUE(output.find("Universal") != std::string::npos);
    EXPECT_TRUE(output.find("Warner") != std::string::npos);
    
    // Test most popular compact
    output = captureOutput([this]() { db->showMostPopularCompact(); });
    // Disc 1 had the most sales (10)
    EXPECT_TRUE(output.find("1") != std::string::npos);
    
    // Test sales by author
    output = captureOutput([this]() { db->showAuthorSales(); });
    EXPECT_TRUE(output.find("Author 1") != std::string::npos);
    EXPECT_TRUE(output.find("Author 2") != std::string::npos);
    EXPECT_TRUE(output.find("Author 3") != std::string::npos);
}

// Test sales information for a specific period
TEST_F(MusicStoreDBTest, SalesInfoTest) {
    setupTestData();
    
    // Test compact sales for a period
    std::string output = captureOutput([this]() { 
        db->showCompactSales(1, "2023-01-01", "2026-12-31"); 
    });
    
    // Check if sales info is shown correctly
    EXPECT_TRUE(output.find("Sony Music") != std::string::npos);
    EXPECT_TRUE(output.find("10") != std::string::npos); // 10 discs sold
    
    // Test for a different user perspective
    output = captureOutput([this]() { 
        db->getCompactSalesInfo(1, "2023-01-01", "2026-12-31"); 
    });
    
    EXPECT_TRUE(output.find("Sony Music") != std::string::npos);
    EXPECT_TRUE(output.find("10") != std::string::npos); // 10 discs sold
}

// Test user permissions
TEST_F(MusicStoreDBTest, UserPermissionsTest) {
    // Admin user should already be verified in other tests
    
    // Test regular user
    EXPECT_TRUE(db->login("user", "user"));
    EXPECT_FALSE(db->isUserAdmin());
    
    // Regular user should still be able to view data
    std::string output = captureOutput([this]() { 
        db->showMostPopularCompact(); 
    });
    
    // Basic output should be available, though might be empty in fresh DB
    EXPECT_FALSE(output.empty());
}