#ifndef LICENSE_HANDLER_HPP
#define LICENSE_HANDLER_HPP

#include <openssl/ssl.h>
#include <string>

void handleLicenseRequest(SSL* ssl, const std::string licenseName);

#endif // LICENSE_HANDLER_HPP
