#include "db_manager.hpp"

DBManager::DBManager(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }
}

DBManager::~DBManager() {
    sqlite3_close(db);
}

void DBManager::createTables() {
    const char* createLicensesTable = R"(
        CREATE TABLE IF NOT EXISTS Licenses (
            /* LicenseID INTEGER PRIMARY KEY AUTOINCREMENT, */
            LicenseID INTEGER PRIMARY KEY AUTOINCREMENT,
            LicenseName TEXT NOT NULL,
            MaxUsage INTEGER
        );
    )";

const char* createCurrentLicensesTable = R"(
    CREATE TABLE IF NOT EXISTS Current_Licenses (
        /* LicenseKey TEXT PRIMARY KEY, */
        LicenseKey TEXT PRIMARY KEY,
        ClientID TEXT NOT NULL,
        LicenseID INTEGER NOT NULL,
        ExpiryTime INTEGER,
        UsageLeft INTEGER,
        FOREIGN KEY (LicenseID) REFERENCES Licenses(LicenseID)
    );
)";


    char* errorMessage = nullptr;

    if (sqlite3_exec(db, createLicensesTable, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string error = "Failed to create Licenses table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    if (sqlite3_exec(db, createCurrentLicensesTable, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string error = "Failed to create Current_Licenses table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

void DBManager::insertLicenseType(const std::string& licenseName, int maxUsage) {
    const char* sql = R"(
        INSERT INTO Licenses (LicenseName, MaxUsage)
        VALUES (?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare insert statement");
    }

    sqlite3_bind_text(stmt, 1, licenseName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, maxUsage);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert license type");
    }

    sqlite3_finalize(stmt);
}

void DBManager::issueLicense(const std::string& licenseKey, const std::string& clientID, int licenseID, time_t expiryTime, int usageLeft) {
    const char* sql = R"(
        INSERT INTO Current_Licenses (LicenseKey, ClientID, LicenseID, ExpiryTime, UsageLeft)
        VALUES (?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare issue statement");
    }

    sqlite3_bind_text(stmt, 1, licenseKey.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, clientID.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, licenseID);
    if (expiryTime != 0)
        sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(expiryTime));
    else
        sqlite3_bind_null(stmt, 4);
    if (usageLeft != 0)
        sqlite3_bind_int(stmt, 5, usageLeft);
    else
        sqlite3_bind_null(stmt, 5);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to issue license");
    }

    sqlite3_finalize(stmt);
}

void DBManager::queryActiveLicenses(const std::string& clientID) {
    const char* sql = R"(
        SELECT CL.LicenseKey, L.LicenseName, CL.ExpiryTime, CL.UsageLeft
        FROM Current_Licenses CL
        JOIN Licenses L ON CL.LicenseID = L.LicenseID
        WHERE CL.ClientID = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare query statement");
    }

    sqlite3_bind_text(stmt, 1, clientID.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string licenseKey = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string licenseName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        time_t expiryTime = sqlite3_column_type(stmt, 2) != SQLITE_NULL ? sqlite3_column_int64(stmt, 2) : 0;
        int usageLeft = sqlite3_column_type(stmt, 3) != SQLITE_NULL ? sqlite3_column_int(stmt, 3) : -1;

        std::cout << "LicenseKey: " << licenseKey << ", LicenseName: " << licenseName
                  << ", ExpiryTime: " << (expiryTime ? std::ctime(&expiryTime) : "No Expiry")
                  << ", UsageLeft: " << (usageLeft >= 0 ? std::to_string(usageLeft) : "Unlimited")
                  << std::endl;
    }

    sqlite3_finalize(stmt);
}
