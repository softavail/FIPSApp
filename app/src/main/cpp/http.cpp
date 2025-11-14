#include "http.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <curl/curl.h>

// Structure to hold response data
typedef struct {
    char *data;
    size_t size;
} HttpResponse;

static size_t debug_callback(CURL * handle,
                             curl_infotype type,
                             char * data,
                             size_t size,
                             void * userp) {

    std::string contents;

    switch (type)
    {
        case CURLINFO_TEXT:
            contents.assign(data, size);
            LOGDBG("%s", contents.c_str());
            break;

        case CURLINFO_HEADER_OUT:
            contents.assign(data, size);
            LOGDBG("%s", contents.c_str());
            break;

        case CURLINFO_DATA_OUT:
            LOGDBG("BODY size: %d", size);
            break;

        case CURLINFO_SSL_DATA_OUT:
            LOGDBG("SSL BODY size: %d", size);
            break;

        case CURLINFO_HEADER_IN:
            contents.assign(data, size);
            LOGDBG("%s", contents.c_str());
            break;

        case CURLINFO_DATA_IN:
            LOGDBG("BODY size: %d", size);
            break;

        case CURLINFO_SSL_DATA_IN:
            LOGDBG("SSL BODY size: %d", size);
            break;

        default:
            break;
    }

    return 0;
}

// Callback function to handle received data
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {

    HttpResponse *response = (HttpResponse *)userp;
    size_t realsize = size * nmemb;

    char *ptr = static_cast<char *>(realloc(response->data, response->size + realsize + 1));
    if (ptr == NULL) {
        // Out of memory
        LOGERR("Not enough memory (realloc returned NULL)");
        return 0;
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0; // Null-terminate

    return realsize;
}

int http_get_request(const char *url, char **response_data, size_t *response_size) {
    CURL *curl;
    CURLcode res;
    HttpResponse response = {0};
    int return_code = 0;

    // Initialize response buffer
    response.data = static_cast<char *>(malloc(1));
    if (response.data == NULL) {
        LOGERR("Failed to allocate initial memory");
        return -1;
    }
    response.size = 0;

    // Initialize cURL
    curl = curl_easy_init();
    if (!curl) {
        LOGERR("Failed to initialize cURL");
        free(response.data);
        return -1;
    }

    // Set cURL options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

    // Set timeout (30 seconds)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // Enable SSL verification (important for security)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, &debug_callback);

    LOGDBG("Connecting to %s", url);

    // Perform the request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
        LOGERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        free(response.data);
        curl_easy_cleanup(curl);
        return -1;
    }

    // Get HTTP response code
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code >= 400) {
        LOGERR("HTTP error: %ld", http_code);
        return_code = (int)http_code;
    }

    // Set output parameters
    *response_data = response.data;
    *response_size = response.size;

    // Cleanup
    curl_easy_cleanup(curl);

    return return_code;
}

// Example usage function
void example_usage() {
    char *response_data = NULL;
    size_t response_size = 0;

    // Initialize cURL globally (call once per application)
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Perform GET request
    int result = http_get_request("https://api.example.com/data", &response_data, &response_size);

    if (result == 0 && response_data != NULL) {
        printf("Response received (%zu bytes):\n%s\n", response_size, response_data);
        free(response_data); // Don't forget to free the response data
    } else {
        fprintf(stderr, "Request failed with code: %d\n", result);
    }

    // Cleanup cURL globally (call once before application exits)
    curl_global_cleanup();
}


void initializeCURL()
{
    LOGDBG("initialize CURL ...");
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
