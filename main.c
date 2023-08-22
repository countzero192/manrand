#include <curses.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// Double Linked list impl
typedef struct {
  char *file;
  void *next;
} Node;

Node *head = NULL;
Node *tail = NULL;

Node *add_item(const char *str) {
  Node *new = malloc(sizeof(Node));
  new->file = strdup(str);
  new->next = NULL;

  if (head == NULL) {
    head = new;
    tail = new;
  } else {
    tail->next = new;
    tail = new;
  }
  return new;
}

int len(void) {
  int counter = 0;
  for (Node *cur = head; cur != NULL; cur = cur->next)
    counter++;
  return counter;
}

void cleanlist(void) {
  Node *cur = head;

  for (; cur != NULL; cur = head) {
    head = head->next;
    free(cur->file);
    free(cur);
  }
}

// Main programm
char basedir[] = "/usr/share/man/";
char *categories[] = {" 1 - user commands ",
                      " 2 - syscalls ",
                      " 3 - functions from libraries ",
                      " 4 - device files ",
                      " 5 - config files ",
                      " 7 - other ",
                      " 8 - admin tools ",
                      "exit"};
int cat_len = sizeof(categories) / sizeof(categories[0]);
char *pages[] = {"man1", "man2", "man3", "man4", "man5", "man7", "man8"};
int pages_len = 7;

// Helper functions
bool isdir(const char *name) {
  struct stat buf;
  if (stat(name, &buf) == -1)
    exit(1);

  if (buf.st_mode & S_IFDIR)
    return true;
  else
    return false;
}

void list_to_array(Node *h, char **a) {
  for (int i = 0; i < len() && h != NULL; i++, h = h->next) {
    a[i] = h->file;
  }
}

#if 0
char *basename(const char *name) {
  char *temp = malloc(strlen(name) + 1);
  strcpy(temp, name);
  while (*temp) {
    if (*temp == '.') {
      *temp = '\0';
      break;
    }
    temp++;
  }
  return temp;
}
#endif

// Display functions
void print_banner() {
  attron(A_REVERSE);
  mvprintw(0, 0, "Manlib");
  mvprintw(0, COLS - 4, "0.1");
  refresh();
  attroff(A_REVERSE);
}

WINDOW *create_win(int height, int width, int starty, int startx) {
  WINDOW *local = newwin(height, width, starty, startx);
  refresh();
  box(local, 0, 0);
  wrefresh(local);
  return local;
}

void print_menu(WINDOW *win, int hightlight) {
  int x, y;
  x = 2;
  y = 1;
  box(win, 0, 0);
  mvwprintw(win, 0, 1, "Categories");
  for (int i = 0; i < cat_len; i++) {
    if (hightlight == i) {
      wattron(win, A_REVERSE);
      mvwprintw(win, y, x, "%s", categories[i]);
      wattroff(win, A_REVERSE);
    } else
      mvwprintw(win, y, x, "%s", categories[i]);
    y++;
  }
  wrefresh(win);
}

int category_chooser(WINDOW *win) {
  int c, choice = -1;
  int highlight = 0;
  print_menu(win, highlight);
  refresh();

  while (1) {
    c = wgetch(win);
    switch (c) {
    case 'k':
      if (highlight == 0)
        highlight = cat_len - 1;
      else
        highlight--;
      break;
    case 'j':
      if (highlight == cat_len - 1)
        highlight = 0;
      else
        highlight++;
      break;
    case 10:
      choice = highlight;
      break;
    }
    print_menu(win, highlight);
    if (choice >= 0)
      break;
  }
  return choice;
}

void manpage(int category) {
  // Read directory
  char *full_path = malloc(64);
  strncpy(full_path, basedir, 64);
  strcat(full_path, pages[category]);
  if (!isdir(full_path))
    exit(1);

  DIR *dir;
  struct dirent *ent;
  dir = opendir(full_path);
  if (dir == NULL)
    exit(1);

  while ((ent = readdir(dir))) {
    if ((!strcmp(ent->d_name, ".")) || (!strcmp(ent->d_name, "..")))
      continue;
    add_item(ent->d_name);
  }
  srand(time(NULL));
  char **items = malloc(len() * sizeof(char *));
  list_to_array(head, items);

  strcat(full_path, "/");
  strcat(full_path, items[rand() % len()]);
  def_prog_mode();
  endwin();
  char *command = malloc(64);
  sprintf(command, "man %s", full_path);
  system(command);
  free(command);
  reset_prog_mode();
  refresh();
  cleanlist();
  free(full_path);
  closedir(dir);
}

int main(void) {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);
  WINDOW *win = create_win(LINES - 1, COLS, 1, 0);

  while (1) {
    print_banner();
    int choice = category_chooser(win);
    if (choice == 7) {
      delwin(win);
      endwin();
      return 0;
    }
    delwin(win);
    win = create_win(LINES - 1, COLS, 1, 0);
    manpage(choice);
  }

  delwin(win);
  endwin();
  return 0;
}
