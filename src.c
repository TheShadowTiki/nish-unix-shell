#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>


// Struct arg for parsing input command in to white-space separated arguments into a linked list
typedef struct arg {
    char *str;
    struct arg *next;
    struct arg *prev;
} arg ;

// Error function to handle message and stderr when throwing errors
void raiseError()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

// Checks if an executable exists in a search path.
int execExists(char *arg, char *path, char *full_path)
{
    char *input, *tok;
    input = malloc(100*sizeof(char));
    strcpy(input, path);

    // Iterates through space separated path list
    while ((tok = strsep(&input, " \t")) != NULL)
    {
        if (*tok == '\0') continue;
        strcpy(full_path, tok);
        strcat(full_path, "/");
        strcat(full_path, arg);
        if (access(full_path, X_OK) == 0) { return 0; } // Returns if arg found in path
    }
    free(input);
    return 1; // Returns with err code if arg is not found in path
}

int main(int argc, char *argv[])
{
    // The shell must be invoked with no arguments
    if (argc > 1) {
        raiseError();
        exit(1); 
    }

    // Initialize path variable
    char path[100] = "/bin";

    // While loop which waits for user input and executes user commands
    while (true)
    {
        printf("nish> ");
        fflush(stdout);
        fflush(stdin);

        // Read in input command
        char *input = NULL;
        size_t size = 0;
        ssize_t chars_read;
        chars_read = getline(&input, &size, stdin);
        if (chars_read == -1) // Throw error if reading input failed
        {
            raiseError();
            continue;
        }

        // Check if input is just white space and continue to await next user input if it is
        if (input != NULL)
        {
            size_t len = strspn(input, " \t\n");
            if (input[len] == '\0') { continue; }
        }

        // Remove trailing newline character
        if (input[chars_read - 1] == '\n')
        {
            input[chars_read - 1] = '\0';
        }

        // Initialze linked list storing arguments of user input
        arg *head = malloc(sizeof(arg*));
        head->str = NULL;
        head->next = NULL;
        arg *ptr = head;

        // Parse arguments
        char *tok = input, *end = input;
        // Iterates through space separated elements adding sequential elements to the linked list
        while ((tok = strsep(&input, " \t")) != NULL)
        {
            if (*tok == '\0') continue;
            ptr->str = tok;
            ptr->next = malloc(sizeof(arg*));
            ptr = ptr->next;
            ptr->str = NULL;
            ptr->next = NULL;
        }
        
        // Check for built-in arguments
        // If input is built-in command "exit", then program exits
        if (strcmp(head->str, "exit") == 0)
        {
            if (head->next->str != NULL)
            {
                raiseError();
                continue;
            }
            exit(0);
        }
        
        // If input is built-in command "cd", execute chdir
        if (strcmp(head->str, "cd") == 0) 
        {
            if (head->next->str == NULL || head->next->next->str != NULL || chdir(head->next->str) == -1) { raiseError(); }
            continue;
        }

        // If input is built-in command "path"
        if (strcmp(head->str, "path") == 0) 
        {
            if (head->next->str == NULL)
            {
                strcpy(path, "");
            }
            else
            {
                // Create new space-separated path string
                char new_path[100];
                strcpy(new_path, head->next->str);
                
                arg *p = head->next->next;
                while (p->next != NULL)
                {
                    strcat(new_path, " ");
                    strcat(new_path, p->str);
                    p = p->next;
                }
                strcpy(path, new_path);
            }
            continue;
        }

        // Execute commands (non-built-in)
        // Move through arguments in linked list
        arg *curr = head, *next;
        int pids[100], num_pids = 0;
        bool redirect = false; // Checks if redirect occurs twice without other arguments which is an error
        while (curr->next != NULL) // Until end of linked list is reached
        {
            char full_path[100];
            char *args[100];
            // If execExists returns error code 1 on an input command that is not "&", then throw error
            if (execExists(curr->str, path, full_path) && strcmp(curr->str, "&")!=0)
            {
                raiseError();
                break;
            }
            else if (curr->next != NULL && strcmp(curr->str, "&") == 0) // If "&" is encountered, reset redirect flag and continue to the next command
            {
                redirect = false;
                curr = curr->next;
            }
            else // Else, execute the command starting at curr
            {
                // Fill args array with the args following the command at curr
                int i = 0;
                while(curr->next != NULL && strcmp(curr->str, ">")!=0 && strcmp(curr->str, "&")!=0) // Fill until end of linked list, or until redirection or a parallel command needs to be run
                {
                    args[i] = curr->str;
                    curr = curr->next;
                    i++;
                }
                args[i] = NULL; // Make array NULL terminated for execv

                // If the a redirection is encountered, then handle writing to file
                if (curr->next != NULL && strcmp(curr->str, ">") == 0)
                {
                    curr = curr->next;
                    if (curr->str == NULL || strcmp(curr->str, ">")==0 || strcmp(curr->str, "&")==0 || redirect) {
                        raiseError();
                        break;
                    } // If next arg is not a file name or a redirection has already occured, raise error
                    
                    // Open file for redirecting to
                    FILE *fp = fopen(curr->str, "w");
                    if (fp == NULL) {
                        raiseError();
                        break;
                    }
                    
                    // Save stdout fileno to restore after redirection
                    int fd = dup(STDOUT_FILENO);
                    // Replace stdout fileno with the newly opened file's so ouput redirects to it
                    dup2(fileno(fp), STDOUT_FILENO);
                    
                    //Create a new process
                    pid_t pid;
                    pid = fork();
                    if (pid == -1) // Throw error if fork failed
                    {
                        raiseError();
                        break;
                    }
                    else if (pid == 0) // Execute command in child process
                    {
                        if (execv(full_path, args) == -1)
                        {
                            raiseError();
                            break;
                        }
                    }
                    else // Parent process keeps track of child pids
                    {
                        pids[num_pids] = pid;
                        num_pids++;
                    }
                    
                    // Restore stdout fileno
                    dup2(fd, STDOUT_FILENO);
                    fclose(fp);
                    curr = curr->next;
                    redirect = true; // Keep track of how many redirects are used in one command
                }
                else // If there is no redirection, simply execute command
                {
                    //Create a new process
                    pid_t pid;
                    pid = fork();
                    if (pid == -1) // Throw error if fork failed
                    {
                        raiseError();
                        break;
                    }
                    else if (pid == 0) // Execute command in child process
                    {
                        if (execv(full_path, args) == -1)
                        {
                            raiseError();
                            break;
                        }
                    }
                    else // Parent process keeps track of child pids
                    {
                        pids[num_pids] = pid;
                        num_pids++;
                    }
                }
            }
        }
        
        // Once entire input is processed, wait on all children processes
        for (int i = 0; i < num_pids; i++)
        {
            int status;
            waitpid(pids[i], &status, 0);
        }

        // Free list of arguments once processing of input is complete
        arg *p = head, *q;
        while (p->next != NULL)
        {
            q = p->next;
            free(p);
            p = q;
        }
    }

    return 0;
}