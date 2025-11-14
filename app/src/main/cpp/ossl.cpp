#include "ossl.h"
#include "core.h"

static OSSL_PROVIDER *fips_provider = nullptr;
static OSSL_PROVIDER *base_provider = nullptr;
static OSSL_LIB_CTX *ctx = nullptr;

static int print_provider(OSSL_PROVIDER *prov, void *arg) {
    const char *name = OSSL_PROVIDER_get0_name(prov);
    LOGDBG("provider: %s", name);
    return 1; // continue iteration
}

bool initializeSSL(const char* fipsConfigPath, const char* modulesPath)
{
    LOGDBG("initialize SSL FIPS mode ...");

    if (fips_provider != nullptr) {
        LOGDBG("SSL FIPS mode already initialized ...");
        return true;
    }

    if (fipsConfigPath == nullptr) {
        LOGERR("fipsConfigPath not provided");
        return false;
    }

//    ctx = OSSL_LIB_CTX_new();

    if (modulesPath != nullptr) {
        LOGDBG("OSSL_PROVIDER_set_default_search_path: %s", modulesPath);
        OSSL_PROVIDER_set_default_search_path(ctx, modulesPath);
    } else {
        LOGDBG("modules path not provided");
    }


    // Load FIPS provider
    if (fipsConfigPath != nullptr) {
        OSSL_LIB_CTX_load_config(ctx, fipsConfigPath);
    }

    list_providers();

    OSSL_LIB_CTX_set0_default(ctx);

    list_providers();

    fips_provider = OSSL_PROVIDER_load(ctx, "fips");
    if (fips_provider == nullptr) {
        LOGERR("Failed to load FIPS provider");
        ERR_print_errors_cb(android_err_print_cb, NULL);
        return false;
    }

    // Load base provider for non-FIPS algorithms
//    base_provider = OSSL_PROVIDER_load(ctx, "base");
//    if (base_provider == nullptr) {
//        LOGERR("Failed to load base provider");
//        ERR_print_errors_cb(android_err_print_cb, NULL);
//        OSSL_PROVIDER_unload(fips_provider);
//        fips_provider = nullptr;
//        return false;
//    }

    // Verify FIPS mode is active
    if (EVP_default_properties_is_fips_enabled(ctx)) {
        LOGDBG("FIPS mode successfully enabled");
        return true;
    }

    LOGDBG("FIPS provider loaded but not in FIPS mode, enabling...");
    EVP_default_properties_enable_fips(ctx, 1);

    if (EVP_default_properties_is_fips_enabled(ctx)) {
        LOGDBG("FIPS mode now enabled");
        return true;
    }

    LOGERR("Failed to enable FIPS mode");
    return false;
}

void list_providers() {
    LOGDBG("list_providers");

    if (!OSSL_PROVIDER_do_all(NULL, print_provider, NULL)) {
        LOGERR("Failed to list providers");
    }
}

