#include <jni.h>

#include <android/log.h>


#define LOG_TAG "FIPSCore"
#define LOGDBG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGERR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

int android_err_print_cb(const char *str, size_t len, void *u);