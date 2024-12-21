#ifndef CLIENT_LICENCE_HPP
#define CLIENT_LICENCE_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <ctime>
#include <iostream>

struct License {
    std::string licenseID;
    std::string licenceName;
    std::time_t expirationDate;

    // Default constructor
    License() : expirationDate(0) {}
};

class CLIENT_LICENCE {
private:
    SSL* ssl;  
    License currentLicense;

public:
    CLIENT_LICENCE(SSL* ssl);
    int requestLicense(const std::string licenseName);
    bool validateLicense(const License& license);
    License getLicenseFromServer();
    void printLicenseInfo(const License& license);
    License getCurrentLicense();
    
    // Corrected method declaration
    License parseLicenseData(const std::string& licenseData);
};

#endif