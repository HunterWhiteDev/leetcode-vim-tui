#include "cJSON.h"
#include "debugCallback.h"
#include "remove_all_chars.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

      char *responseStr = cJSON_Print(response);

      cJSON *question = cJSON_GetObjectItem(data, "question");

      char *questionStr = cJSON_Print(question);

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

      char *commentedString = malloc(strlen(strippedString));
      strcat(commentedString, "/*");
      strcat(commentedString, strippedString);
      strcat(commentedString, "*/");

      remove_all_chars(strippedString, '"');

      cJSON *code = cJSON_GetObjectItem(langSnippet, "code");
      char *codeString = cJSON_Print(code);

      int contentStringLen =
          snprintf(NULL, 0, "%s\n \n %s\n", commentedString, codeString);
      char *contentString = malloc(contentStringLen + 1);

      snprintf(contentString, contentStringLen + 1, "%s\n \n %s\n",
               commentedString, codeString);

      // printf("%s", contentString);

      char *parsedContent = malloc(contentStringLen);

      bool skipChar = false;
      int lastPIdx = 0;

      for (int i = 0; i < contentStringLen; i++) {

        // Skip here

        if (contentString[i] == '\\' && contentString[i + 1] == 'n') {
          parsedContent[lastPIdx] = '\n';
          lastPIdx++;

          skipChar = true;

          if (contentString[i + 2] == '\\' && contentString[i + 3] == 'n') {
            i = i + 2;
          }

          continue;
        }

        if (skipChar == true) {
          skipChar = false;
          continue;
        } else {
          parsedContent[lastPIdx] = contentString[i];
          skipChar = false;
          lastPIdx++;
        }
      }

      remove_all_chars(parsedContent, '"');

      int fileStringLen =
          snprintf(NULL, 0, "/home/%s/.leetcode/problems/JavaScript/%s.js",
                   username, slugTitle);
      char *fileString = malloc(fileStringLen + 1);
      snprintf(fileString, fileStringLen + 1,
               "/home/%s/.leetcode/problems/JavaScript/%s.js", username,
               slugTitle);
      fileString[fileStringLen + 1] = '\0';

      FILE *fptr;
      printf("%s", parsedContent);

      fptr = fopen(fileString, "w");
      fprintf(fptr, "%s", parsedContent);
      fclose(fptr);

      int fileJsonStringLen =
          snprintf(NULL, 0, "/home/%s/.leetcode/problems/JavaScript/%s.json",
                   username, slugTitle);
      char *fileJsonSring = malloc(fileJsonStringLen + 1);
      snprintf(fileJsonSring, fileJsonStringLen + 1,
               "/home/%s/.leetcode/problems/JavaScript/%s.json", username,
               slugTitle);
      fileJsonSring[fileJsonStringLen + 1] = '\0';

      // printf("%s", fileJsonSring);

      FILE *JsonFilePtr;

      cJSON *testCaseList =
          cJSON_GetObjectItem(question, "exampleTestcaseList");

      char parsedTestcaseString[1024];
      parsedTestcaseString[0] = '\0';

      cJSON *elementBuffer;
      cJSON_ArrayForEach(elementBuffer, testCaseList) {
        char *elementString = cJSON_Print(elementBuffer);
        remove_all_chars(elementString, '"');
        strcat(parsedTestcaseString, elementString);
      }

      printf("%s", parsedTestcaseString);

      char *questionId =
          cJSON_Print(cJSON_GetObjectItem(question, "questionId"));

      JsonFilePtr = fopen(fileJsonSring, "w");

      fprintf(JsonFilePtr, "%s%s%s%s%s%s%s", "{\"exampleTestcaseList\":\"",
              parsedTestcaseString, "\", \"question_id\":", questionId,
              ",\"slug\": \"", slugTitle, "\"}");
      fclose(JsonFilePtr);

      char nvimCmd[fileStringLen + 9];
      nvimCmd[0] = '\0';

      strcat(nvimCmd, "/bin/nvim ");
      strcat(nvimCmd, fileString);

      system(nvimCmd);

      free(fileJsonSring);
      free(fileString);
      free(contentString);
      free(commentedString);
    }

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
  }

  free(queryString);
}
