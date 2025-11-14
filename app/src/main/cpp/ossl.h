#include <openssl/evp.h>
#include <openssl/provider.h>
#include <openssl/err.h>

bool initializeSSL(const char* fipsConfigPath, const char* modulesPath);
void list_providers();