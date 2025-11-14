#include <stddef.h>

void initializeCURL();

/**
 * Performs an HTTP GET request using cURL
 *
 * @param url The URL to request
 * @param response_data Pointer to store the response data (caller must free)
 * @param response_size Pointer to store the response size
 * @return 0 on success, non-zero on failure
 */
int http_get_request(const char *url, char **response_data, size_t *response_size);
