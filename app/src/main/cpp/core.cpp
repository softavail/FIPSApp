#include "core.h"
#include "ossl.h"
#include "http.h"

#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

// Custom callback that forwards to Android log
int android_err_print_cb(const char *str, size_t len, void *u) {
    // Remove trailing newline if present
    if (len > 0 && str[len-1] == '\n') {
        len--;
    }
    LOGERR("%.*s", (int)len, str);
    return 1; // success
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_fipsapp_MainActivity_initializeFIPS(JNIEnv* env, jobject /* this */,
                                                     jstring j_fipsConfigPath,
                                                     jstring j_modulesPath) {

    jboolean res = JNI_FALSE;
    const char* fipsConfigPath = env->GetStringUTFChars(j_fipsConfigPath, nullptr);
    const char* modulesPath = env->GetStringUTFChars(j_modulesPath, nullptr);

    if ( initializeSSL(fipsConfigPath, modulesPath) ) {
        res = JNI_TRUE;
    }

    initializeCURL();

    list_providers();

    env->ReleaseStringUTFChars(j_fipsConfigPath, fipsConfigPath);
    env->ReleaseStringUTFChars(j_modulesPath, modulesPath);

    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_fipsapp_MainActivity_computeSHA256(JNIEnv* env, jobject /* this */, jstring message) {
    const char* msg = env->GetStringUTFChars(message, nullptr);
    if (msg == nullptr) {
        return nullptr;
    }

    LOGDBG("Computing SHA-256 hash in FIPS mode...");
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        LOGERR("Failed to create EVP_MD_CTX");
        env->ReleaseStringUTFChars(message, msg);
        return nullptr;
    }
    
    const EVP_MD* md = EVP_sha256();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1 ||
        EVP_DigestUpdate(mdctx, msg, strlen(msg)) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        LOGERR("Failed to compute hash");
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        env->ReleaseStringUTFChars(message, msg);
        return nullptr;
    }
    
    EVP_MD_CTX_free(mdctx);
    env->ReleaseStringUTFChars(message, msg);
    
    // Convert hash to hex string
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    std::string hexHash = ss.str();
    LOGDBG("Hash computed successfully: %s", hexHash.c_str());
    
    return env->NewStringUTF(hexHash.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_fipsapp_MainActivity_performHTTPSGet(JNIEnv* env, jobject /* this */,
                                                        jstring url) {

    jstring result = nullptr;
    const char *url_str = env->GetStringUTFChars(url, nullptr);

    char* response_data = nullptr;
    size_t response_size = 0;
    int res_code = http_get_request(url_str, &response_data, &response_size);
    if (res_code == -1) {
        LOGERR("HTTP Get failed");
    } else {
        if (response_data != nullptr) {

            result = env->NewStringUTF(response_data);

            free(response_data);
        }
    }

    env->ReleaseStringUTFChars(url, url_str);
    return result;
}
