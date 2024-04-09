#include "shell.h"
#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_LINE 80

char *lastCommand;

int main(int argc, char **argv) {
  lastCommand = calloc(1, MAXLINE);
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell() {
  const char *arguments[] = {"ls", ">", "junk.txt", NULL};
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  while (should_run) {
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    // ^D results in n == -1
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }
    if (equal(line, "")) {
      continue;
    }
    if (equal(line, "test")) {
      child(arguments);
    }
    if (equal(line, "!!")) {
      history();
    }

    *lastCommand = *line;

    processLine(line);
  }
  free(line);
  free(lastCommand);
  return 0;
}

void history() {
  if (lastCommand == NULL) {
    printf("No command found %s\n");
  } else {
    printf("Last command: %s\n", lastCommand);
  }
}

int listFiles(char **args, int argCount) {
  bool allInfo = false; // boolean for whether -al is present in args
  for (int i = 1; i < argCount; i++) {
    if (equal(args[i], "-al")) {
      printf("allInfo = true");
      allInfo = true;
    }
  }

  // Opens directory and checks to make sure the directory is found
  DIR *directory =
      opendir("/Users/rimjhimsudhesh/Downloads/p1-shell-starter-main");
  if (directory == NULL) {
    perror("Directory not found!");
    return EXIT_FAILURE;
  }

  // Iterates through all files within directory
  struct dirent *entry;
  while ((entry = readdir(directory)) != NULL) {
    if (allInfo) {
      getFileInformation(
          "/Users/rimjhimsudhesh/Downloads/p1-shell-starter-main", entry);
    }
    printf("%s\n", entry->d_name);
  }
  return EXIT_SUCCESS;
}

/*
  Returns additional file information including the owner, time modified, file
  size, permissions?

  Called by the allInfo boolean from listFiles
*/
int getFileInformation(const char *path, struct dirent *entry) {
  struct stat fileStat;

  // Construct full path to the file
  char fullpath[PATH_MAX];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

  // Getting the file information
  if (stat(fullpath, &fileStat) < 0) {
    perror("stat");
    return EXIT_FAILURE;
  }

  // File Name
  printf("Name: %s\n", entry->d_name);

  // File Types and Permissions
  printf("Types and Permissions: ");
  printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
  printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
  printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
  printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
  printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
  printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
  printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
  printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
  printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
  printf((fileStat.st_mode & S_IXOTH) ? "x\n" : "-\n");

  // Number of hard links
  printf("Number of hard links: %ld\n", (long)fileStat.st_nlink);

  // Owner user and group
  struct passwd *pw = getpwuid(fileStat.st_uid);
  struct group *gr = getgrgid(fileStat.st_gid);
  printf("Owner: %s\n", (pw != NULL) ? pw->pw_name : "");
  printf("Group: %s\n", (gr != NULL) ? gr->gr_name : "");

  // File size
  printf("Size: %lld bytes\n", (long long)fileStat.st_size);

  // Last modification time
  char date[30];
  strftime(date, 30, "%b %d %H:%M", localtime(&(fileStat.st_mtime)));
  printf("Last modification time: %s\n", date);

  printf("\n");

  return EXIT_SUCCESS;
}

int standardout(char **args) {
  // open the corresponding file and use dup to direct stdout to file
  FILE *fp = fopen("myfile.txt", "w");

  if (fp == NULL) {
    printf("Error opening file!\n");
    return 1;
  }

  // Opens directory and checks to make sure the directory is found
  DIR *directory =
      opendir("/Users/rimjhimsudhesh/Downloads/p1-shell-starter-main");
  if (directory == NULL) {
    perror("Directory not found!");
    return EXIT_FAILURE;
  }

  // Iterates through all files within directory
  struct dirent *entry;
  int result;
  while ((entry = readdir(directory)) != NULL) {
    printf("%s\n", entry->d_name);
    // const char *text = "Hello, world!\n";
    result = fputs(entry->d_name, fp);
    result = fputs("\n", fp);
  }

  if (result == EOF) {
    printf("Error writing to file!\n");
    fclose(fp);
    return 1;
  }

  printf("Successfully wrote text to file.\n");
  fclose(fp);

  return 0;
}

int doPipe(char **args, int pipei) {}

void processLine(char *line) {
  char *token = strtok(line, " ");
  char *args[MAX_INPUT];
  int index = 0;
  while (token != NULL) {
    printf("%s\n", token);
    if (equal(token, ";")) {
      // No need to check for NULL here, handled in loop
      char *first = args[0];
      if (equal(first, "ls")) {
        printf("Working\n");
        listFiles(args, 0);
      }
      break; // Exit the loop after encountering a semicolon
    } else {
      args[index] = token;
      index++;
      token = strtok(NULL, " ");
    }
  }
  args[index] = NULL;
  if (args[0] != NULL) {
    char *first = args[0];
    if (equal(first, "ls")) {
      listFiles(args,
                index); // args[index] == NULL therefore index is argsCount
    }
  } else {
    printf("Null first address!\n");
  }

  /*WORKING
  printf("processing line: %s\n", line);
  if (equal(line, "!!")) {
    history();
  } else {
    lastCommand = line;
    printf("Here\n");
  }*/
}

int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls",      "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0';
  }
  return n;
}
