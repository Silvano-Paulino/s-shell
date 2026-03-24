#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

bool is_valid_token(char *token, int i)
{
    return token != NULL && i < MAX_ARGS - 1;
}

int main()
{
    char line[1024];
    char *args[MAX_ARGS];
    char *token;

    while (1)
    {
        printf("x-shell> ");

        if (!fgets(line, sizeof(line), stdin))
            break;

        int i = 0;
        token = strtok(line, DELIM);

        while (is_valid_token(token, i))
        {
            args[i++] = token;
            token = strtok(NULL, DELIM);
        }

        args[i] = NULL;

        if (args[0] == NULL)
            continue;

        __pid_t pid = fork();

        if (pid == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                perror("Error executing command");
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("Error no process created");
            exit(EXIT_FAILURE);
        }
        else
        {
            wait(NULL);
        }
    }

    return 0;
}