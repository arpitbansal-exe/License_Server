#ifndef DB_MANAGER_HPP
#define DB_MANAGER_HPP

#include <string>
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <ctime>

class DBManager {
public:
    DBManager(const std::string& dbPath);
    ~DBManager();

    void createTables();
    void insertLicenseType(const std::string& licenseName, int maxUsage = 0);
    void issueLicense(const std::string& licenseKey, const std::string& clientID, int licenseID, time_t expiryTime = 0, int usageLeft = 0);
    void queryActiveLicenses(const std::string& clientID);

private:
    sqlite3* db;
};

#endif // DB_MANAGER_HPP