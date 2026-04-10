#include <curl/curl.h>
#include <stdlib.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                           void *userp);

int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size,
                   void *clientp);
