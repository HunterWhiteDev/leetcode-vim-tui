CFLAGS = -Wall -Wextra
LDFLAGS =-lncursesw -lcurl
SOURCES = main.c solveQuestion.c cJSON.c debugCallback.c loadQuestion.c remove_all_chars.c fetchQuestions.c testQuestion.c

.PHONY: all debug
all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o leetcode

debug:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -ggdb3 -o leetcode
