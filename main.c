#include "cJSON.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdbool.h>  // bool type
#include <sys/stat.h> // stat

bool file_exists(char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

struct Problem {
  char name[30];
};

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size,
                   void *clientp) {

};

int main(int argc, char **argv) {

  int getlogin_r(char *buf, size_t bufsize);

  char *p_username = getlogin();
  char username[strlen(p_username)];
  strcpy(username, p_username);

  int dirLen = strlen(username) + strlen("/home//.leetcode/config.conf") + 1;
  char dir[dirLen];
  dir[0] = '\0';

  strcat(dir, "/home/");
  strcat(dir, username);
  strcat(dir, "/.leetcode/config.conf");

  char *sessionToken = malloc(0);
  char *csrfToken = malloc(0);

  if (file_exists(dir)) {
    // printf("%s exists\n", dir);

    size_t allocatedSize = 0;
    FILE *fptr;

    char *line = NULL;
    char *value = NULL;
    char *key = NULL;

    fptr = fopen(dir, "r");
    while (getline(&line, &allocatedSize, fptr) != -1) {
      strtok(line, "= ");

      key = line;
      value = strtok(NULL, "= ");

      if (strcmp(key, "LEETCODE_SESSION") == 0) {
        char *tempSessionToken =
            realloc(sessionToken, strlen(value) + sizeof(char));
        if (tempSessionToken == NULL) {
          printf("Could not reallocate access token string");
          return 1;
        }
        sessionToken = tempSessionToken;
        strcpy(sessionToken, value);
        sessionToken[strlen(sessionToken) - 1] = '\0';
      } else if (strcmp(key, "CSRF_TOKEN") == 0) {
        char *tempCsrfToken = realloc(csrfToken, strlen(value) + sizeof(char));
        if (tempCsrfToken == NULL) {
          printf("Could not reallocate csrf token string");
          return 1;
        }
        csrfToken = tempCsrfToken;
        strcpy(csrfToken, value);
        csrfToken[strlen(csrfToken) - 1] = '\0';
      }
    }

    // Write some text to the file
    fclose(fptr);

  } else {
    FILE *fptr;
    fptr = fopen(dir, "w");
    printf("%s does not exist\n", dir);
  }

  char *queryArg = NULL;

  if (!argv[1]) {
    printf("No query string provided");
    return 0;
  } else {
    queryArg = argv[1];
  }

  int len = snprintf(NULL, 0, "Cookie: LEETCODE_SESSION=%s; csrftoken=%s",
                     sessionToken, csrfToken);
  char *tokenHeaderStr = malloc(len + 1);
  snprintf(tokenHeaderStr, len + 1, "Cookie: LEETCODE_SESSION=%s; csrftoken=%s",
           sessionToken, csrfToken);

  CURL *curl;
  CURLcode result;
  curl = curl_easy_init();

  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  char *jsonRequestString = malloc(
      strlen(
          "{\"query\":\"    query searchQuestionList($filters: "
          "QuestionFilterInput, $limit: Int, $searchKeyword: String, $skip: "
          "Int, "
          "$sortBy: QuestionSortByInput, $categorySlug: String) {  "
          "problemsetQuestionListV2(    filters: $filters    limit: $limit    "
          "searchKeyword: $searchKeyword    skip: $skip    sortBy: $sortBy    "
          "categorySlug: $categorySlug  ) {    questions {      id      "
          "titleSlug  "
          "    title      translatedTitle      questionFrontendId      "
          "paidOnly    "
          "  difficulty      topicTags {        name        slug        "
          "nameTranslated      }      status      isInMyFavorites      "
          "frequency   "
          "   acRate      contestPoint    }    totalLength    finishedLength   "
          " "
          "hasMore  }}    \",\"variables\":{\"searchKeyword\":\"\0") +
      strlen(queryArg) +
      strlen("\",\"limit\":16,\"skip\":0},\"operationName\":"
             "\"searchQuestionList\"}"));

  strcat(jsonRequestString,
         "{\"query\":\"    query searchQuestionList($filters: "
         "QuestionFilterInput, $limit: Int, $searchKeyword: String, $skip: "
         "Int, "
         "$sortBy: QuestionSortByInput, $categorySlug: String) {  "
         "problemsetQuestionListV2(    filters: $filters    limit: $limit    "
         "searchKeyword: $searchKeyword    skip: $skip    sortBy: $sortBy    "
         "categorySlug: $categorySlug  ) {    questions {      id      "
         "titleSlug  "
         "    title      translatedTitle      questionFrontendId      "
         "paidOnly    "
         "  difficulty      topicTags {        name        slug        "
         "nameTranslated      }      status      isInMyFavorites      "
         "frequency   "
         "   acRate      contestPoint    }    totalLength    finishedLength   "
         " "
         "hasMore  }}    \",\"variables\":{\"searchKeyword\":\"\0");
  strcat(jsonRequestString, queryArg);
  strcat(jsonRequestString, "\",\"limit\":10,\"skip\":0},\"operationName\":"
                            "\"searchQuestionList\"}");

  cJSON *questions;

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

  free(jsonRequestString);
  free(tokenHeaderStr);
  free(sessionToken);
  free(csrfToken);

  initscr();

  int yMax, xMax;
  getmaxyx(stdscr, yMax, xMax);

  WINDOW *mennuwin = newwin(0, 0, 0, 0);

  noecho();
  cbreak();

  wrefresh(mennuwin);
  refresh();

  struct Problem problem2;
  strcpy(problem2.name, "Problem 2");

  // sizeof() returns the size in bytes, to change it to the correct index
  // divide it by the size of the first elemeent ini the array
  // int length = sizeof(problems) / sizeof(problems[0]);

  // if (selectedIdx == i) {
  //   wattron(mennuwin, A_REVERSE);
  // }
  // mvwprintw(mennuwin, i + 1, 2, problems[i].name);
  // wattroff(mennuwin, A_REVERSE);

  int selectedIdx = 0;

  while (1) {

    const cJSON *element = NULL;
    int idx = 0;
    cJSON_ArrayForEach(element, questions) {
      cJSON *title = cJSON_GetObjectItem(element, "title");
      char *titleString = cJSON_Print(title);

      if (selectedIdx == idx) {
        wattron(mennuwin, A_REVERSE);
      }

      mvwprintw(mennuwin, idx + 1, 2, titleString);
      idx++;

      wattroff(mennuwin, A_REVERSE);
    }

    int input = wgetch(mennuwin);

    if (input == 'j') {
      if (selectedIdx >= idx - 1) {
        selectedIdx = 0;
      } else {
        selectedIdx++;
      }
    } else if (input == 'k') {
      if (selectedIdx == 0) {
        selectedIdx = idx - 1;
      } else {
        selectedIdx--;
      }
    } else if (input == 'f') {
      endwin();
      return 0;
    }
  }
  endwin();

  free(sessionToken);
  free(csrfToken);

  return 0;
}
