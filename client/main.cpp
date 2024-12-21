#include "tls_client.hpp"
#include <openssl/err.h>
#include <iostream>
#include "licence.hpp"

int main() {
    // Initialize OpenSSL error strings for more detailed error reporting
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    try {
        TLSClient client;
        
        // Print any pending OpenSSL errors before connection
        unsigned long err;
        char errBuffer[256];
        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, errBuffer, sizeof(errBuffer));
            std::cerr << "OpenSSL Error (before connect): " << errBuffer << std::endl;
        }
        
        // Connect to server on localhost and correct port
        if (client.connect("127.0.0.1", 8443)) {
            std::cout << "Connected to server successfully!" << std::endl;
            
            // Send a test message
            client.sendMessage("Hello, License Server!");
            
            // Receive response
            std::string response = client.receiveMessage();
            std::cout << "Server response: " << response << std::endl;
            
            CLIENT_LICENCE clientLicense(client.getSSL());
            clientLicense.requestLicense("FullLicense");

            client.disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "Caught Exception: " << e.what() << std::endl;
        
        // Print any pending OpenSSL errors after exception
        unsigned long err;
        char errBuffer[256];
        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, errBuffer, sizeof(errBuffer));
            std::cerr << "OpenSSL Error (after exception): " << errBuffer << std::endl;
        }
        
        return 1;
    }

    // Clean up OpenSSL error strings
    ERR_free_strings();

    return 0;
}