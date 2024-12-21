// client/tls_client.hpp
#ifndef TLS_CLIENT_HPP
#define TLS_CLIENT_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>

class TLSClient {
public:
    TLSClient();
    ~TLSClient();

    bool connect(const std::string& host, int port);
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    void disconnect();
    SSL* getSSL() { return ssl; }

private:
    SSL_CTX* ctx;
    SSL* ssl;
    int socketfd;

    void initSSLContext();
    void handleSSLError(int ret);
};

#endif // TLS_CLIENT_HPP