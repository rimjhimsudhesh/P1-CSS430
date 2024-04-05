#include "shell.h"
#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell() {
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
    processLine(line);
  }
  free(line);
  return 0;
}

int listFiles(char *args) {
  printf(
      "Entering!\n"); // Check to see if we are getting to the listFiles method
  // Check for bugs in this code snippet below
  bool allInfo = false; // boolean for whether -al is present in args
  int index = 1;
  while (index < MAX_INPUT &&
         args[index] !=
             NULL) { // Should we return EXIT_FAILURE if an arg is not "-al"?
    if (equal(args[index], "-al")) {
      allInfo = true;
    }
  }
  // ^^^^^^^^^^^^^^^^^^^^^^

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

void processLine(char *line) {
  char *token = strtok(line, " ");
  char args[MAX_INPUT];
  int index = 0;
  while (token != NULL) {
    // Fix looping issue!
    printf("Looping!\n");
    if (equal(token, ";")) {
      char *first = args[0];
      if (equal(first, "ls")) {
        printf("Working\n"); // Make sure it properly compares the first index
                             // "ls" to "ls"
        listFiles(args);
      }
    }
    args[index] = token;
    token = strtok(token, " "); // token vs line vs NULL as first param
  }
  /* WORKING
  printf("processing line: %s\n", line);
  if (equal(line, "ls")) {
    listFiles();
  } else {
    printf("Failed\n");
  }
  */
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
