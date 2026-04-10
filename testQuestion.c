#include "cJSON.h"
#include "debugCallback.h"
#include "remove_all_chars.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void testQuestion(char *tokenHeaderStr, char *fileName, char *csrftoken) {

  // Current working directory
  char cwd[2048];
  getcwd(cwd, 2048);

  // Build full file path
  char filePath[2048];
  strcat(filePath, cwd);
  strcat(filePath, "/");
  strcat(filePath, fileName);

  FILE *fptr;
  fptr = fopen(filePath, "r");

  char *buffer = 0;
  long length;

  if (fptr) {
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    buffer = malloc(length);
    if (buffer) {
      fread(buffer, 1, length, fptr);
    }
  }

  fclose(fptr);

  char jsonFilePath[2048];
  jsonFilePath[0] = '\0';
  strcat(jsonFilePath, filePath);
  strcat(jsonFilePath, "on");

  FILE *jsonFptr;
  jsonFptr = fopen(jsonFilePath, "r");

  char *jsonBuffer = 0;
  long jsonLength;

  if (jsonFptr) {
    fseek(jsonFptr, 0, SEEK_END);
    jsonLength = ftell(jsonFptr);
    fseek(jsonFptr, 0, SEEK_SET);
    jsonBuffer = malloc(jsonLength);
    if (jsonBuffer) {
      fread(jsonBuffer, 1, jsonLength, jsonFptr);
    }
  }

  fclose(jsonFptr);
  remove_all_chars(buffer, '\n');

  cJSON *json = cJSON_Parse(jsonBuffer);
  cJSON *testCaseExampleList = cJSON_GetObjectItem(json, "exampleTestcaseList");
  char *testCaseExampleListString = cJSON_Print(testCaseExampleList);

  char *questionId = cJSON_Print(cJSON_GetObjectItem(json, "question_id"));
  char *slug = cJSON_Print(cJSON_GetObjectItem(json, "slug"));
  remove_all_chars(slug, '"');

  char *lang = "javascript\0";

  int len =
      snprintf(NULL, 0,
               "{ \"data_input\": %s, \"lang\": \"%s\", \"question_id\": %s, "
               "\"typed_code\": \" %s \" }",
               testCaseExampleListString, lang, questionId, buffer);

  char *queryString = malloc(len + 1);
  snprintf(queryString, len + 1,
           "{ \"data_input\": %s, \"lang\": \"%s\", \"question_id\": %s, "
           "\"typed_code\": \" %s \" }",
           testCaseExampleListString, lang, questionId, buffer);

  queryString[len + 1] = '\0';

  char *csrfTokenHeaderString =
      malloc(strlen("x-csrftoken: ") + strlen(csrftoken) + 1);

  csrfTokenHeaderString[0] = '\0';
  strcat(csrfTokenHeaderString, "x-csrftoken: ");
  strcat(csrfTokenHeaderString, csrftoken);

  int problemUrlLen = snprintf(
      NULL, 0, "https://leetcode.com/problems/%s/interpret_solution/", slug);
  char *problemUrl = malloc(problemUrlLen + 1);
  snprintf(problemUrl, problemUrlLen + 1,
           "https://leetcode.com/problems/%s/interpret_solution/", slug);
  problemUrl[problemUrlLen + 1] = '\0';

  CURL *curl;
  CURLcode result;
  curl = curl_easy_init();
  if (curl) {
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0;           /* no data at this point */

    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "content-type: application/json");
    headers = curl_slist_append(headers, tokenHeaderStr);
    headers = curl_slist_append(headers, csrfTokenHeaderString);
    headers = curl_slist_append(
        headers, "Referer: https://leetcode.com/problems/two-sum/");

    curl_easy_setopt(curl, CURLOPT_URL, problemUrl);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryString);

    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    else {
      cJSON *responseJson = cJSON_Parse(chunk.memory);
      char *interpretId =
          cJSON_Print(cJSON_GetObjectItem(responseJson, "interpret_id"));

      remove_all_chars(interpretId, '"');

      int urlLen =
          snprintf(NULL, 0, "https://leetcode.com/submissions/detail/%s/check/",
                   interpretId);

      char *url = malloc(urlLen + 1);
      snprintf(url, urlLen + 1,
               "https://leetcode.com/submissions/detail/%s/check/",
               interpretId);

      url[urlLen + 1] = '\0';
      printf("%s", url);

      int requestCount = 0;
      while (requestCount < 4) {
        sleep(3);
        CURL *curl2;
        CURLcode result2;
        curl2 = curl_easy_init();
        if (curl2) {
          struct MemoryStruct chunk2;

          chunk2.memory =
              malloc(1);   /* will be grown as needed by the realloc above */
          chunk2.size = 0; /* no data at this point */

          curl_easy_setopt(curl2, CURLOPT_URL, url);
          curl_easy_setopt(curl2, CURLOPT_DEBUGFUNCTION, debug_callback);

          curl_easy_setopt(curl2, CURLOPT_HTTPHEADER, headers);
          curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
          curl_easy_setopt(curl2, CURLOPT_WRITEDATA, (void *)&chunk2);

          result = curl_easy_perform(curl2);
          /* Check for errors */
          if (result2 != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result2));
            requestCount = 6;
          }

          else {
            requestCount++;
            cJSON *responseJson = cJSON_Parse(chunk2.memory);
            char *state =
                cJSON_Print(cJSON_GetObjectItem(responseJson, "state"));
            remove_all_chars(state, '"');

            if (strcmp(state, "SUCCESS") == 0) {
              printf("Success");
            } else {
              printf("fail");
            }
            requestCount++;
          }
        }
      }
    }
  }

  free(buffer);
  free(jsonBuffer);
};
