#include "cJSON.h"
#include "debugCallback.h"
#include "fetchQuestions.h"
#include "loadQuestion.h"
#include "remove_all_chars.h"
#include "solveQuestion.h"
#include "testQuestion.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <regex.h>
#include <stdbool.h>
#include <stdbool.h> // bool type
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // stat
#include <unistd.h>

bool file_exists(char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

int main(int argc, char **argv) {
  mkdir("~/.leetcode",S_IRWXU);
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

  char *sessionToken = NULL;
  char *csrfToken = NULL;

  if (file_exists(dir)) {

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
        char *tempSessionToken = realloc(sessionToken, strlen(value)+1);
        if (tempSessionToken == NULL) {
          printf("Could not reallocate access token string");
          return 1;
        }
        if (sessionToken < value) {
           fprintf(stderr,"value is bigger than sessionToken size.This could lead to buffer overflow.\n") ;
            return 1;
        }
        strcpy(sessionToken, value);
        sessionToken[strlen(sessionToken) - 1] = '\0';
      } else if (strcmp(key, "CSRF_TOKEN") == 0) {
        char *tempCsrfToken = realloc(csrfToken, strlen(value) + 1);
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
    fclose(fptr);
  }

  char *queryArg = NULL;

  int len = snprintf(NULL, 0, "Cookie: LEETCODE_SESSION=%s; csrftoken=%s",
                     sessionToken, csrfToken);
  char *tokenHeaderStr = malloc(len + 1);
  snprintf(tokenHeaderStr, len + 1, "Cookie: LEETCODE_SESSION=%s; csrftoken=%s",
           sessionToken, csrfToken);

  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];
    if (strcmp(arg, "--test") == 0) {
      char *fileName = argv[i + 1];
      testQuestion(tokenHeaderStr, fileName, csrfToken);
      return 0;
    }

    if (strcmp(arg, "--solve") == 0) {
      char *fileName = argv[i + 1];
      solveQuestion(tokenHeaderStr, fileName, csrfToken);
      return 0;
    }
  }

  if (!argv[1]) {
    printf("No query string provided");
    return 0;
  } else {
    queryArg = argv[1];
  }

  int jsonRequestStringLen = snprintf(
      NULL, 0,
      "{\"query\":\"query searchQuestionList($filters:"
      "QuestionFilterInput, $limit: Int, $searchKeyword: String, $skip:"
      "Int,"
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
      "hasMore  }}    \",\"variables\":{\"searchKeyword\": \"%s\",\"limit\":"
      "20,\"skip\":"
      "0},"
      "\"operationN"
      "ame\":"
      "\"searchQues"
      "tionList\"}",
      queryArg);

  char *jsonRequestString = malloc(jsonRequestStringLen + 1);
  snprintf(
      jsonRequestString, jsonRequestStringLen + 1,
      "{\"query\":\"query searchQuestionList($filters:"
      "QuestionFilterInput, $limit: Int, $searchKeyword: String, $skip:"
      "Int,"
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
      "hasMore  }}    \",\"variables\":{\"searchKeyword\": \"%s\",\"limit\":"
      "20,\"skip\":"
      "0},"
      "\"operationN"
      "ame\":"
      "\"searchQues"
      "tionList\"}",
      queryArg);

  cJSON *questions;
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
    headers = curl_slist_append(headers, "content-type: application/json ");
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
      printf("%s", cJSON_Print(data));

      cJSON *problemsetQuestionList =
          cJSON_GetObjectItem(data, "problemsetQuestionListV2");
      questions = cJSON_GetObjectItem(problemsetQuestionList, "questions");
    }

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
  }


  setlocale(LC_ALL, "");

  initscr();

  int yMax, xMax;
  getmaxyx(stdscr, yMax, xMax);

  WINDOW *mennuwin = newwin(0, 0, 0, 0);

  noecho();
  cbreak();

  wrefresh(mennuwin);
  refresh();

  int selectedIdx = 0;

  while (1) {

    const cJSON *element = NULL;
    int idx = 0;
    cJSON_ArrayForEach(element, questions) {
      cJSON *title = cJSON_GetObjectItem(element, "title");
      char *titleString = cJSON_Print(title);
      char *paidOnly = cJSON_Print(cJSON_GetObjectItem(element, "paidOnly"));
      char *difficulty =
          cJSON_Print(cJSON_GetObjectItem(element, "difficulty"));
      char *status = cJSON_Print(cJSON_GetObjectItem(element, "status"));
      remove_all_chars(paidOnly, '"');
      remove_all_chars(titleString, '"');
      remove_all_chars(difficulty, '"');
      remove_all_chars(status, '"');

      use_default_colors();
      start_color(); /* Start color 			*/

      if (strcmp(difficulty, "EASY") == 0) {

        init_pair(idx + 1, COLOR_GREEN, -1);
      } else if (strcmp(difficulty, "MEDIUM") == 0) {
        init_pair(idx + 1, COLOR_YELLOW, -1);
      } else if (strcmp(difficulty, "HARD") == 0) {
        init_pair(idx + 1, COLOR_RED, -1);
      } else {
        init_pair(idx + 1, COLOR_BLACK, -1);
      }

      wattron(mennuwin, COLOR_PAIR(idx + 1));

      if (selectedIdx == idx) {
        wattron(mennuwin, A_REVERSE);
      }

      int titleLen = strlen(titleString);

      if (strcmp(status, "SOLVED") == 0) {
        strcat(titleString, " ✔️");
      }

      if (strcmp(paidOnly, "true") == 0) {
        strcat(titleString, " ⭐");
      }

      mvwprintw(mennuwin, idx + 1, 2, "%d%s%s", idx + 1, ") ", titleString);

      // mvwprintw(mennuwin, idx + 1, titleLen + 10, "%d%s", idx, difficulty);

      wattroff(mennuwin, COLOR_PAIR(idx + 1));
      wattroff(mennuwin, A_REVERSE);

      idx++;
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
      cJSON *question = cJSON_GetArrayItem(questions, selectedIdx);
      cJSON *titleSlug = cJSON_GetObjectItem(question, "titleSlug");
      char *titleSlugString = cJSON_Print(titleSlug);
      remove_all_chars(titleSlugString, '"');

      loadQuestion(titleSlugString, tokenHeaderStr, username);

      return 0;
    }
  }
  endwin();

  free(sessionToken);
  free(csrfToken);

  return 0;
}
