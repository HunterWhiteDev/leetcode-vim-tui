#include "cJSON.h"
#include "debugCallback.h"
#include <curl/curl.h>
#include <stdio.h>
void fetchQuestions(char *tokenHeaderStr, char *jsonRequestString,
                    cJSON *questions) {

  CURL *curl;
  CURLcode result;
  curl = curl_easy_init();

  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  if (curl) {
    /* First set the URL that is about to receive our POST. This URL can
       be an https:// URL if that is what should receive the data. */
    //

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "content-type: application/json");
    headers = curl_slist_append(headers, tokenHeaderStr);

    curl_easy_setopt(curl, CURLOPT_URL, "https://leetcode.com/graphql/");
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonRequestString);

    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    else {

      cJSON *response = cJSON_Parse(chunk.memory);
      if (response == NULL) {
        printf("Response is null");
      }
      cJSON *data = cJSON_GetObjectItem(response, "data");

      cJSON *problemsetQuestionList =
          cJSON_GetObjectItem(data, "problemsetQuestionListV2");
      questions = cJSON_GetObjectItem(problemsetQuestionList, "questions");
    }

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
  }
};
