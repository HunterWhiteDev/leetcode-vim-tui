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

  // printf("%s", dir);
  // printf("%c", printf(&dir[dirLen]));

  char *sessionToken = malloc(0);
  char *csfrToken = malloc(0);

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
      } else if (strcmp(key, "CSFR_TOKEN") == 0) {
        char *tempCsfrToken = realloc(csfrToken, strlen(value) + sizeof(char));
        if (tempCsfrToken == NULL) {
          printf("Could not reallocate csfr token string");
          return 1;
        }
        csfrToken = tempCsfrToken;
        strcpy(csfrToken, value);
        csfrToken[strlen(csfrToken) - 1] = '\0';
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

  char tokenHeaderStr[strlen("Cookie: LEETCODE_SESSION=") +
                      strlen("; csfrtoken=") + strlen(sessionToken) +
                      strlen(csfrToken) + sizeof(char)];
  tokenHeaderStr[0] = '\0';
  strcat(tokenHeaderStr, "Cookie: LEETCODE_SESSION=");
  strcat(tokenHeaderStr, sessionToken);
  strcat(tokenHeaderStr, "; csfrtoken=");
  strcat(tokenHeaderStr, csfrToken);
  printf("%s", tokenHeaderStr);

  // struct Problem problem1;
  // strcpy(problem1.name, "Problem 1");

  CURL *curl;
  CURLcode result;
  curl = curl_easy_init();

  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  // if (curl) {
  //   /* First set the URL that is about to receive our POST. This URL can
  //      be an https:// URL if that is what should receive the data. */
  //   curl_easy_setopt(curl, CURLOPT_URL, "'https://leetcode.com/graphql/");
  //
  //   curl_easy_setopt(curl, CURLOPT_HEADER, "Content-Type: application/json");
  //
  //   curl_easy_setopt(curl, CURLOPT_HEADER, tokenHeaderStr);
  //   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  //   curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  //
  //   /* Perform the request, result gets the return code */
  //   result = curl_easy_perform(curl);
  //   /* Check for errors */
  //   if (result != CURLE_OK)
  //     fprintf(stderr, "curl_easy_perform() failed: %s\n",
  //             curl_easy_strerror(result));
  //   else {
  //     // printf(chunk.memory);
  //   }
  //
  //   /* always cleanup */
  //
  //   curl_easy_cleanup(curl);
  // }

  free(sessionToken);
  free(csfrToken);
  // return 1;
  // initscr();
  //
  // int yMax, xMax;
  // getmaxyx(stdscr, yMax, xMax);
  //
  // WINDOW *mennuwin = newwin(0, 0, 0, 0);
  //
  // noecho();
  // cbreak();
  //
  // wrefresh(mennuwin);
  // refresh();
  //
  // struct Problem problem2;
  // strcpy(problem2.name, "Problem 2");
  //
  // struct Problem problems[] = {problem1, problem2};
  //
  // // sizeof() returns the size in bytes, to change it to the correct index
  // // divide it by the size of the first elemeent ini the array
  // int length = sizeof(problems) / sizeof(problems[0]);
  // int selectedIdx = 0;
  //
  // while (1) {
  //
  //   for (int i = 0; i < length; i++) {
  //
  //     if (selectedIdx == i) {
  //       wattron(mennuwin, A_REVERSE);
  //     }
  //     mvwprintw(mennuwin, i + 1, 2, problems[i].name);
  //     wattroff(mennuwin, A_REVERSE);
  //   }
  //
  //   int input = wgetch(mennuwin);
  //
  //   if (input == 'j') {
  //     if (selectedIdx >= length - 1) {
  //       selectedIdx = 0;
  //     } else {
  //       selectedIdx++;
  //     }
  //   } else if (input == 'k') {
  //     if (selectedIdx == 0) {
  //       selectedIdx = length - 1;
  //     } else {
  //       selectedIdx--;
  //     }
  //   }
  // }
  // endwin();
  //
  // free(sessionToken);
  // free(csfrToken);

  return 0;
}
