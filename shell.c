#include "shell.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80
#define MAX_ARGS 10

char lastCommand[MAX_LINE];

void executeCommand(char *line) {
  char *tokens[MAX_ARGS];
  char *token = strtok(line, " ");
  int tokenCount = 0;
  while (token != NULL) {
    printf(" toekn is: %s\n", token);
    tokens[tokenCount++] = token;
    token = strtok(NULL, " ");
  }

  tokens[tokenCount] = NULL;

  int background = 0;
  if (strcmp(tokens[tokenCount - 1], "&") == 0) {
    background = 1;
    tokens[tokenCount - 1] = NULL;
  }

  int redirectOutput = 0;
  char *outputFile = NULL;
  /*printf(" tokens and index are: %s, %s, %s\n", tokens[0], tokens[1],
         tokens[2]);*/

  if (strcmp(tokens[1], ">") == 0) {
    outputFile = tokens[2];
  }

  printf("output file is %s\n", outputFile);

  /*for (int i = 0; i < tokenCount; ++i) {
    printf("%s\n", tokens[i]);

    if (strcmp(tokens[i], ">") == 0) {
      redirectOutput = 1;
      if (i < tokenCount - 1) {
        outputFile = tokens[i + 1];
        *tokens[i] = NULL;
        *tokens[i + 1] = NULL;
      } else {
        printf("Error: No output file in input!\n");
        return;
      }
    }
  }*/

  // printf("Length of tokens %d\n:", tokenCount);
  // printf("Tokens 2 is %s\n", tokens[2]);

  // printf("output file is: %s\n", outputFile);

  /*int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  if (dup2(fd, STDOUT_FILENO) == -1) {
    perror("dup2");
    close(fd);
    exit(EXIT_FAILURE);
  }

  close(fd);*/

  int redirectInput = 0;
  char *inputFile = NULL;
  for (int i = 0; i < tokenCount; ++i) {
    if (strcmp(tokens[i], "<") == 0) {
      redirectInput = 1;
      if (i < tokenCount - 2) {
        inputFile = tokens[i + 1];
        tokens[i] = NULL;
        tokens[i + 1] = NULL;
      } else {
        printf("Error: No input file in input!\n");
        return;
      }
    }
  }

  int pipeIndex = -1;
  for (int i = 0; i < tokenCount; ++i) {
    if (strcmp(tokens[i], "|") == 0) {
      pipeIndex = i;
      break;
    }
  }

  if (pipeIndex != -1) {
    char *command1[MAX_ARGS];
    char *command2[MAX_ARGS];
    for (int i = 0; i < pipeIndex; ++i) {
      command1[i] = tokens[i];
    }
    command1[pipeIndex] = NULL;

    int j = 0;
    for (int i = pipeIndex + 1; i < tokenCount; ++i) {
      command2[j++] = tokens[i];
    }
    command2[j] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
      execvp(command1[0], command1);
      perror("execvp");
      exit(EXIT_FAILURE);
    }

    pid2 = fork();
    if (pid2 == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      execvp(command2[0], command2);
      perror("execvp");
      exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
  } else {
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      // printf("token 0 %s\n", tokens[0]);
      char *args[] = {"", NULL};
      FILE *file = freopen(tokens[2], "w", stdout);
      if (file == NULL) {
        perror("Error opening file");
      }
      if (execvp("ls", args) == -1) {
        perror("execvp");
      }
      // execvp("ls", args);
      //  execvp(tokens[0], tokens[1]);
      perror("execvp");
      exit(EXIT_FAILURE);
    } else {

      int status;
      waitpid(pid, &status, 0);
      if (WIFEXITED(status)) {
        strcpy(lastCommand, line);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--interactive") == 0) {
    interactiveShell();
  } else {
    runTests();
  }
  return 0;
}

int interactiveShell() {
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while (1) {
    printf(PROMPT);
    fflush(stdout);

    if ((nread = getline(&line, &len, stdin)) != -1) {
      line[strcspn(line, "\n")] = '\0'; // Remove newline character
      if (strcmp(line, "exit") == 0) {
        break;
      }
      executeCommand(line);
    } else {
      break;
    }
  }

  free(line);
  return 0;
}

int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAX_LINE] = {
      "ls",      "ls -al", "ls & whoami", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "whoami"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    executeCommand(lines[i]);
  }

  return 0;
}
