#include "cJSON.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <curses.h>
#include <ncurses.h>
#include <regex.h>
#include <stdbool.h>
#include <stdbool.h> // bool type
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // stat
#include <unistd.h>
#define TRUE 1
#define FALSE 0

void remove_all_chars(char *str, char c) {
  char *pr = str, *pw = str;
  while (*pr) {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

bool file_exists(char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

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

void loadQuestion(char *slugTitle, char *tokenHeaderStr, char *username) {

  int len = snprintf(
      NULL, 0,
      "{\"query\":\"query questionDetail($titleSlug: String!) {  "
      "languageList {    id    name  }  submittableLanguageList {    id    "
      "name    verboseName  }  statusList {    id    name  }  "
      "questionDiscussionTopic(questionSlug: $titleSlug) {    id    "
      "commentCount    topLevelCommentCount  }  "
      "ugcArticleOfficialSolutionArticle(questionSlug: $titleSlug) {    "
      "uuid   "
      " chargeType    canSee    hasVideoArticle  }  question(titleSlug: "
      "$titleSlug) {    title    titleSlug    questionId    "
      "questionFrontendId "
      "   questionTitle    translatedTitle    content    translatedContent "
      "   "
      "categoryTitle    difficulty    stats    companyTagStatsV2    "
      "topicTags "
      "{      name      slug      translatedName    }    positionLevelTags "
      "{   "
      "   name      nameTranslated      slug    }    similarQuestionList { "
      "    "
      " difficulty      titleSlug      title      translatedTitle      "
      "isPaidOnly    }    mysqlSchemas    dataSchemas    frontendPreviews  "
      "  "
      "likes    dislikes    isPaidOnly    status    canSeeQuestion    "
      "enableTestMode    metaData    enableRunCode    enableSubmit    "
      "enableDebugger    envInfo    isLiked    nextChallenges {      "
      "difficulty      title      titleSlug      questionFrontendId    }   "
      " "
      "libraryUrl    adminUrl    hints    codeSnippets {      code      "
      "lang   "
      "   langSlug    }    exampleTestcaseList    hasFrontendPreview    "
      "featuredContests {      titleSlug      title    }  }}    "
      "\",\"variables\":{\"titleSlug\":\"%s\"},\"operationName\":"
      "\"questionDetail\"}",
      slugTitle);

  char *queryString = malloc(len + 1);
  snprintf(
      queryString, len + 1,
      "{\"query\":\"query questionDetail($titleSlug: String!) {  "
      "languageList {    id    name  }  submittableLanguageList {    id    "
      "name    verboseName  }  statusList {    id    name  }  "
      "questionDiscussionTopic(questionSlug: $titleSlug) {    id    "
      "commentCount    topLevelCommentCount  }  "
      "ugcArticleOfficialSolutionArticle(questionSlug: $titleSlug) {    "
      "uuid   "
      " chargeType    canSee    hasVideoArticle  }  question(titleSlug: "
      "$titleSlug) {    title    titleSlug    questionId    "
      "questionFrontendId "
      "   questionTitle    translatedTitle    content    translatedContent "
      "   "
      "categoryTitle    difficulty    stats    companyTagStatsV2    "
      "topicTags "
      "{      name      slug      translatedName    }    positionLevelTags "
      "{   "
      "   name      nameTranslated      slug    }    similarQuestionList { "
      "    "
      " difficulty      titleSlug      title      translatedTitle      "
      "isPaidOnly    }    mysqlSchemas    dataSchemas    frontendPreviews  "
      "  "
      "likes    dislikes    isPaidOnly    status    canSeeQuestion    "
      "enableTestMode    metaData    enableRunCode    enableSubmit    "
      "enableDebugger    envInfo    isLiked    nextChallenges {      "
      "difficulty      title      titleSlug      questionFrontendId    }   "
      " "
      "libraryUrl    adminUrl    hints    codeSnippets {      code      "
      "lang   "
      "   langSlug    }    exampleTestcaseList    hasFrontendPreview    "
      "featuredContests {      titleSlug      title    }  }}    "
      "\",\"variables\":{\"titleSlug\":\"%s\"},\"operationName\":"
      "\"questionDetail\"}",
      slugTitle);

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

    curl_easy_setopt(curl, CURLOPT_URL, "https://leetcode.com/graphql/");
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

      cJSON *response = cJSON_Parse(chunk.memory);
      if (response == NULL) {
        printf("Response is null");
      }
      cJSON *data = cJSON_GetObjectItem(response, "data");
      cJSON *question = cJSON_GetObjectItem(data, "question");
      cJSON *content = cJSON_GetObjectItem(question, "content");

      char *jsonString = cJSON_Print(content);
      int jsonStringLen = strlen(jsonString);
      char strippedString[jsonStringLen + 1];

      int lastIdx = 0;
      int isStripping = 0;

      for (int i = 0; i < jsonStringLen; i++) {
        if (jsonString[i] == '<') {
          isStripping = 1;
        }

        if (isStripping == 0) {

          strippedString[lastIdx] = jsonString[i];
          strippedString[lastIdx + 1] = '\0';
          lastIdx++;
        }

        if (jsonString[i] == '>') {
          isStripping = 0;
        }
      }

      cJSON *codeSnippets = cJSON_GetObjectItem(question, "codeSnippets");

      cJSON *langSnippet;

      cJSON *element;
      cJSON_ArrayForEach(element, codeSnippets) {
        cJSON *lang = cJSON_GetObjectItem(element, "lang");
        char *langString = cJSON_Print(lang);
        remove_all_chars(langString, '"');
        int cmp = strcmp(langString, "JavaScript");
        if (cmp == 0) {
          langSnippet = element;
        };
      }

      cJSON *code = cJSON_GetObjectItem(langSnippet, "code");
      char *codeString = cJSON_Print(code);

      printf("%s", codeString);

      int contentStringLen =
          snprintf(NULL, 0, "%s \n %s", strippedString, codeString);
      char *contentString = malloc(contentStringLen + 1);

      snprintf(contentString, contentStringLen + 1, "%s \n %s", strippedString,
               codeString);

      printf("%s", contentString);

      int fileStringLen =
          snprintf(NULL, 0, "/home/%s/.leetcode/problems/JavaScript/%s.js",
                   username, slugTitle);
      char *fileString = malloc(fileStringLen + 1);
      snprintf(fileString, fileStringLen + 1,
               "/home/%s/.leetcode/problems/JavaScript/%s.js", username,
               slugTitle);
      fileString[fileStringLen + 1] = '\0';

      printf("%s", fileString);

      FILE *fptr;

      fptr = fopen(fileString, "wt");
      fprintf(fptr, "%s", contentString);

      fclose(fptr);

      free(fileString);
      free(contentString);
    }

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
  }

  free(queryString);
}

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
    fclose(fptr);
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

  int selectedIdx = 0;

  while (1) {

    const cJSON *element = NULL;
    int idx = 0;
    cJSON_ArrayForEach(element, questions) {
      cJSON *title = cJSON_GetObjectItem(element, "title");
      char *titleString = cJSON_Print(title);
      remove_all_chars(titleString, '"');

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
