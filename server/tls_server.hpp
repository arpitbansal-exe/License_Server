#ifndef TLS_SERVER_HPP
#define TLS_SERVER_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <unistd.h>

class TLSServer {
private:
    SSL_CTX* ctx;       // SSL Context for the entire server
    int serverSocket;   // Socket file descriptor for listening

    // Private methods to set up SSL context and create socket
    void initSSLContext(const std::string& certFile, const std::string& keyFile);
    int createSocket(int port);
    void monitorClient(SSL* ssl, int clientSocket);

public:
    // Constructor takes three parameters:
    // 1. Port number to listen on
    // 2. Path to server certificate
    // 3. Path to server private key
    TLSServer(int port, const std::string& certFile, const std::string& keyFile) {
        // Initialize SSL context with provided certificate and key
        initSSLContext(certFile, keyFile);

        // Create server socket bound to specified port
        serverSocket = createSocket(port);
    }

    // Destructor to clean up resources
    ~TLSServer() {
        // Close the server socket
        close(serverSocket);

        // Free the SSL context
        SSL_CTX_free(ctx);
    }
    std::string readRequest(SSL* ssl);
    // Method to start accepting connections
    void start();
};

#endif // TLS_SERVER_HPP