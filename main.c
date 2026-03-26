#include <curses.h>
#include <ncurses.h>
#include <string.h>

struct Problem {
  char name[30];
};

void printList(int idx) {}

int main() {

  initscr();
  // sizeof() returns the size in bytes, to change it to the correct index
  // divide it by the size of the fir eleemnt ini the array

  noecho();
  cbreak();

  struct Problem problem1;
  strcpy(problem1.name, "Problem 1");

  int yMax, xMax;
  getmaxyx(stdscr, yMax, xMax);

  WINDOW *mennuwin = newwin(0, 0, 0, 0);

  wrefresh(mennuwin);
  refresh();

  struct Problem problem2;
  strcpy(problem2.name, "Problem 2");

  struct Problem problems[] = {problem1, problem2};

  int length = sizeof(problems) / sizeof(problems[0]);
  int selectedIdx = 0;

  while (1) {

    for (int i = 0; i < length; i++) {

      if (selectedIdx == i) {
        wattron(mennuwin, A_REVERSE);
      }
      mvwprintw(mennuwin, i + 1, 2, problems[i].name);
      wattroff(mennuwin, A_REVERSE);
    }

    int input = wgetch(mennuwin);

    if (input == 'j') {
      if (selectedIdx >= length - 1) {
        selectedIdx = 0;
      } else {
        selectedIdx++;
      }
    } else if (input == 'k') {
      if (selectedIdx == 0) {
        selectedIdx = length - 1;
      } else {
        selectedIdx--;
      }
    }
  }
  endwin();

  return 0;
  // endwin();
}
