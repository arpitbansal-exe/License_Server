#include "license_handler.hpp"
#include <iostream>
#include <string>
#include <openssl/ssl.h>

void handleLicenseRequest(SSL* ssl, const std::string licenseName) {
    // Prepare dummy data based on the license name
    std::string licenseInfo = "License ID: LIC12345\n";
    licenseInfo += "License Name: " + licenseName + "\n";
    licenseInfo += "Expiration Date: 1672531199\n";

    // Send the dummy license details back to the client
    SSL_write(ssl, licenseInfo.c_str(), licenseInfo.length());
}
