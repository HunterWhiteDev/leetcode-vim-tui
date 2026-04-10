all:
	gcc main.c cJSON.c solveQuestion.c debugCallback.c loadQuestion.c remove_all_chars.c fetchQuestions.c testQuestion.c -I./ -lncursesw -lcurl -o main.out
