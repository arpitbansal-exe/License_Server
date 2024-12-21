#include "tls_client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

TLSClient::TLSClient() : ctx(nullptr), ssl(nullptr), socketfd(-1) {
    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
    initSSLContext();
}

TLSClient::~TLSClient() {
    disconnect();
    SSL_CTX_free(ctx);
    EVP_cleanup();
}

void TLSClient::initSSLContext() {
    const SSL_METHOD* method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        throw std::runtime_error("Unable to create SSL context");
    }
    if (!SSL_CTX_load_verify_locations(ctx, "../server/certs/server.crt", nullptr)) {
        throw std::runtime_error("Failed to load CA certificates");
    }
    // Optional: Set verification mode
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
}

bool TLSClient::connect(const std::string& host, int port) {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address");
    }

    if (::connect(socketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Connection failed");
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socketfd);

    int ret = SSL_connect(ssl);
    if (ret != 1) {
        handleSSLError(ret);
        return false;
    }

    return true;
}

bool TLSClient::sendMessage(const std::string& message) {
    if (!ssl) return false;

    int bytesWritten = SSL_write(ssl, message.c_str(), message.length());
    return bytesWritten > 0;
}

std::string TLSClient::receiveMessage() {
    if (!ssl) return "";

    char buffer[4096];
    int bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        return std::string(buffer);
    }
    
    return "";
}

void TLSClient::disconnect() {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }

    if (socketfd != -1) {
        close(socketfd);
        socketfd = -1;
    }
}

void TLSClient::handleSSLError(int ret) {
    int err = SSL_get_error(ssl, ret);
    char errorBuffer[256];
    ERR_error_string_n(err, errorBuffer, sizeof(errorBuffer));
    throw std::runtime_error(std::string("SSL Error: ") + errorBuffer);
}