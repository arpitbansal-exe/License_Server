#include "tls_server.hpp"
#include "license_handler.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread> 
#include <chrono> 

void TLSServer::initSSLContext(const std::string& certFile, const std::string& keyFile) {
    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        throw std::runtime_error("Unable to create SSL context");
    }
    
    // Load certificate
    if (SSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Failed to load certificate");
    }

    // Load private key
    if (SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Failed to load private key");
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        throw std::runtime_error("Private key does not match the certificate");
    }
}

int TLSServer::createSocket(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    // Allow socket reuse
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        throw std::runtime_error("Bind failed");
    }

    if (listen(sock, 5) < 0) {
        close(sock);
        throw std::runtime_error("Listen failed");
    }

    return sock;
}

void TLSServer::start() {
    std::cout << "Server listening on port..." << std::endl;
    
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    // Accept connection
    int client = accept(serverSocket, (struct sockaddr*)&addr, &len);
    if (client < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }

    // Create SSL connection
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);

    // Perform SSL handshake
    int ret = SSL_accept(ssl);
    if (ret <= 0) {
        int err = SSL_get_error(ssl, ret);
        char errorBuffer[256];
        ERR_error_string_n(err, errorBuffer, sizeof(errorBuffer));
        std::cerr << "SSL accept error: " << errorBuffer << std::endl;
        
        SSL_free(ssl);
        close(client);
        return;
    }
    
    std::cout << "Client connected with TLS!" << std::endl;

    // Read and discard the initial hello message
    std::string initialMessage = readRequest(ssl);
    std::cout << "Discarding initial message: " << initialMessage << std::endl;
    SSL_write(ssl, "Hello, Client!", 14);
    // Read the actual license request
    std::string request = readRequest(ssl);
    std::cout << "Received request: " << request << std::endl;
    
    // Handle the license request
    if (!request.empty() && request.find("LICENSE_REQUEST ") == 0) {
        std::string licenseName = request.substr(16); // Extract the license name after the command
        std::cout << "Received license request for: " << licenseName << std::endl;

        handleLicenseRequest(ssl, licenseName);
    } else {
        // Invalid request
        const char* errorMessage = "Invalid request format.\n";
        SSL_write(ssl, errorMessage, strlen(errorMessage));
    }
    std::thread monitorThread(&TLSServer::monitorClient, this, ssl, client);
    monitorThread.detach();
}
// Read a request from the client
std::string TLSServer::readRequest(SSL* ssl) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    int bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        std::cerr << "Error reading request from client" << std::endl;
        return "";
    }
    buffer[bytesRead] = '\0';  // Null-terminate the string
    std::cout << "Received " << bytesRead << " bytes from client: ";
    for (int i = 0; i < bytesRead; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
    return std::string(buffer);
}

// Monitor client activity
void TLSServer::monitorClient(SSL* ssl, int clientSocket) {
    const int checkIntervalSeconds = 10; // Check every 10 seconds
    char buffer[256];
    
    try {
        while (true) {
            // Check if the connection is still active
            int ret = SSL_read(ssl, buffer, sizeof(buffer) - 1);

            if (ret <= 0) {
                int err = SSL_get_error(ssl, ret);
                if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL) {
                    std::cout << "Client disconnected or connection error." << std::endl;
                    break; // Exit the loop if client disconnected
                }
            }

            std::cout << "Client still active." << std::endl;

            // Wait before checking again
            std::this_thread::sleep_for(std::chrono::seconds(checkIntervalSeconds));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error in monitorClient: " << ex.what() << std::endl;
    }

    // Cleanup after client disconnects
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(clientSocket);
    std::cout << "Cleaned up client connection." << std::endl;
}