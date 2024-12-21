#include "licence.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <cstring>
#include <regex>

CLIENT_LICENCE::CLIENT_LICENCE(SSL* ssl) : ssl(ssl) {}

int CLIENT_LICENCE::requestLicense(const std::string licenseName) {
    std::string requestMessage = "LICENSE_REQUEST " + licenseName;
    std::cout << "Requesting license for: " << licenseName << std::endl;
    std::cout << "Request Message: " << requestMessage << std::endl;
    int len = SSL_write(ssl, requestMessage.c_str(), requestMessage.length());
    if (len <= 0) {
        std::cerr << "Error sending license request." << std::endl;
        return -1;
    }
    std::cout << "License request sent." << std::endl;
    License license = getLicenseFromServer();
    // if (validateLicense(license)) {
    //     printLicenseInfo(license);
    // } else {
    //     std::cout << "Invalid or expired license." << std::endl;
    // }
    std::cout << "License received." << std::endl;
    printLicenseInfo(license);
    return 0;
}

bool CLIENT_LICENCE::validateLicense(const License& license) {
    std::time_t currentTime = std::time(nullptr);
    return license.expirationDate > currentTime;
}

License CLIENT_LICENCE::getLicenseFromServer() {
    char buffer[1024];
    int bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        std::cerr << "Error reading license from server." << std::endl;
        return License();
    }

    buffer[bytesRead] = '\0';
    std::string licenseData(buffer);

    License license;
    size_t pos = 0;
    std::cout << "License Data: " << licenseData << std::endl;
    license = parseLicenseData(licenseData);

    return license;
}

void CLIENT_LICENCE::printLicenseInfo(const License& license) {
    std::cout << "License ID: " << license.licenseID << std::endl;
    std::cout << "License Name: " << license.licenceName << std::endl;
    
    // Convert expiration timestamp to readable format
    char buffer[26];
    std::time_t expDate = license.expirationDate;
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&expDate));
    
    std::cout << "Expiration Date: " << buffer << std::endl;
}

License CLIENT_LICENCE::parseLicenseData(const std::string& licenseData) {
    License license;
    
    std::regex licenseIDRegex("License ID: ([^\n]+)");
    std::regex licenseNameRegex("License Name: ([^\n]+)");
    std::regex expirationDateRegex("Expiration Date: (\\d+)");
    
    std::smatch match;
    
    if (!std::regex_search(licenseData, match, licenseIDRegex)) {
        std::cerr << "Failed to parse License ID" << std::endl;
        return license;
    }
    license.licenseID = match[1].str();
    
    if (!std::regex_search(licenseData, match, licenseNameRegex)) {
        std::cerr << "Failed to parse License Name" << std::endl;
        return license;
    }
    license.licenceName = match[1].str();
    
    if (!std::regex_search(licenseData, match, expirationDateRegex)) {
        std::cerr << "Failed to parse Expiration Date" << std::endl;
        return license;
    }
    license.expirationDate = std::stoi(match[1].str());
    
    return license;
}