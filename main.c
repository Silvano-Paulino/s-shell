#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

#define MODE_APPEND 1
#define MODE_TRUNCATE 2

bool is_valid_token(char *token, int i)
{
    return token != NULL && i < MAX_ARGS - 1;
}

char *handler_redirection(char **args, int *mode)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            char *filename = args[i + 1];
            args[i] = NULL;

            *mode = MODE_TRUNCATE;
            return filename;
        }

        if (strcmp(args[i], ">>") == 0)
        {
            char *filename = args[i + 1];
            args[i] = NULL;

            *mode = MODE_APPEND;
            return filename;
        }
    }
    return NULL;
}

int main()
{
    char line[1024];
    char *args[MAX_ARGS];
    char *token;

    while (1)
    {
        printf("s-shell> ");

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

        __pid_t process_id = fork();

        if (process_id == 0)
        {
            int mode = 0;
            char *outFilename = handler_redirection(args, &mode);

            if (outFilename)
            {
                int flags = (mode == MODE_APPEND) ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;

                int file = open(outFilename, flags, 0644);

                if (file < 0)
                {
                    perror("Error opening file for redirection");
                    exit(EXIT_FAILURE);
                }

                if (dup2(file, STDOUT_FILENO) < 0)
                {
                    perror("Error redirecting output");
                    exit(EXIT_FAILURE);
                }

                close(file);
            }

            if (execvp(args[0], args) == -1)
            {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        }
        else if (process_id < 0)
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