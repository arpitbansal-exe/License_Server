#include "tls_server.hpp"
#include "db_manager.hpp"
#include <iostream>
#include <ctime>
#include <thread>

int main() {
  ERR_print_errors_fp(stderr);
    try {
        // Initialize database
        DBManager dbManager("test_licenses.db");

        // Create tables
        dbManager.createTables();
        std::cout << "Tables created successfully." << std::endl;

        // Insert license types
        dbManager.insertLicenseType("Standard License", 0);
        dbManager.insertLicenseType("Pro License", 100);
        std::cout << "License types inserted successfully." << std::endl;

        // Issue licenses to clients
        // time_t now = std::time(nullptr);
        // try{
        //     dbManager.issueLicense("LIC123", "CLIENT1", 1, now + 30 * 24 * 60 * 60, 0); // Standard License, 30-day expiry
        //     dbManager.issueLicense("LIC124", "CLIENT2", 2, 0, 50);    
        //     std::cout << "Licenses issued successfully." << std::endl;
        // }
        // catch(const std::exception& ex){
        //     std::cerr << "Error: " << ex.what() << std::endl;
        //     std::cout<<"Skipped Licence isuuing"<<std::endl;
        // }

        // Create TLS Server
        TLSServer server(8443, "../server/certs/server.crt", "../server/certs/server.key");
        
        std::cout << "Server starting on port 8443..." << std::endl;
        
        // Continuously accept connections
        while (true) {
            server.start();
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}