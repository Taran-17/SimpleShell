#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>  // Added for isspace()
#define MAX_COMMAND_LEN 1024
#define MAX_ARG_LEN 100
#define HISTORY_SIZE 100

// Struct to hold command execution details
typedef struct {
    char *command;
    pid_t pid;
    time_t start_time;
    double duration;  // in seconds
} CommandHistory;

// History storage
CommandHistory history[HISTORY_SIZE];
int history_count = 0;

// Add command to history with additional details
void add_to_history(char *command, pid_t pid, time_t start_time, double duration) {
    if (history_count < HISTORY_SIZE) {
        history[history_count].command = strdup(command);
        history[history_count].pid = pid;
        history[history_count].start_time = start_time;
        history[history_count].duration = duration;
        history_count++;
    }
}

// Display command history with additional details
void show_history() {
    printf("\nCommand History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: Command: %s, PID: %d, Start Time: %s, Duration: %.2f seconds\n",
               i + 1,
               history[i].command,
               history[i].pid,
               ctime(&history[i].start_time),  // Convert time to readable string
               history[i].duration);
    }
}

// Graceful termination handler
void graceful_termination() {
    printf("\nSimpleShell is exiting. Showing command history and details:\n");
    show_history();
    exit(0);
}

// Function to parse and tokenize commands
void tokenize_command(char *cmd, char **args) {
    char *token = strtok(cmd, " \n");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL; // Null-terminate the array of arguments
}

// Execute command without waiting for input/output redirection
void exec_single_cmd(char *cmd) {
    char *args[MAX_ARG_LEN];
    tokenize_command(cmd, args);

    // Execute the command
    if (execvp(args[0], args) == -1) {
        perror("exec failed");
        exit(1);  // Exit the child process if exec fails
    }
}

// Function to execute multiple piped commands and track each command's details
void execute_piped_commands(char **cmds, int num_cmds, int background) {
    int pipefd[2 * (num_cmds - 1)];
    pid_t pids[num_cmds];
    int i, j;

    // Create pipes
    for (i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe failed");
            exit(1);
        }
    }

    // Execute each command in the pipeline
    for (i = 0; i < num_cmds; i++) {
        // Track start time
        time_t start_time = time(NULL);
        double duration = 0;

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pids[i] == 0) {
            // Redirect input if not the first command
            if (i > 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }

            // Redirect output if not the last command
            if (i < num_cmds - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }

            // Close all pipe fds in the child process
            for (j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefd[j]);
            }

            // Execute the command directly
            exec_single_cmd(cmds[i]);
        }
    }

    // Parent process closes pipe fds
    for (j = 0; j < 2 * (num_cmds - 1); j++) {
        close(pipefd[j]);
    }

    // Wait for all children unless it's a background process
    if (!background) {
        for (i = 0; i < num_cmds; i++) {
            int status;
            waitpid(pids[i], &status, 0);
            if (WIFEXITED(status)) {
                printf("[Command %s exited with status %d]\n", cmds[i], WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("[Command %s terminated by signal %d]\n", cmds[i], WTERMSIG(status));
            }
        }
    } else {
        for (i = 0; i < num_cmds; i++) {
            printf("[Background process started with PID: %d]\n", pids[i]);
        }
    }
}
// Function to trim whitespace from a string
char* trim_whitespace(char *str) {
    char *end;
    
    // Trim leading spaces
    while(isspace((unsigned char)*str)) str++;

    // If the string is all spaces, return it as empty
    if(*str == 0)
        return str;

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write a new null terminator
    end[1] = '\0';

    return str;
}
// Function to check for invalid use of '&'
int validate_ampersand_usage(char *command) {
    // Check if & appears somewhere in the middle
    char *ampersand_pos = strchr(command, '&');
    if (ampersand_pos != NULL) {
        // Trim the command after the ampersand
        ampersand_pos = trim_whitespace(ampersand_pos + 1);
        if (*ampersand_pos != '\0') {
            return 0;  // Invalid: & is followed by extra characters
        }
    }
    return 1;  // Valid if no trailing characters after &
}


// Parse and execute commands with pipes and background processes
void parse_and_execute_command(char *command) {
    int background = 0;

      char *ampersand_pos = strchr(command, '&');
    if (ampersand_pos != NULL) {
        // Check if the ampersand usage is valid
        if (!validate_ampersand_usage(command)) {
            printf("Error: '&' can only appear at the end of the command with no trailing characters.\n");
            return;
        }

        background = 1;
        *ampersand_pos = '\0';  // Remove '&' from the command string
    }

    // Split the command by pipes
    char *cmds[MAX_ARG_LEN];
    int num_cmds = 0;
    char *pipe_token = strtok(command, "|");
    while (pipe_token != NULL) {
        cmds[num_cmds++] = pipe_token;
        pipe_token = strtok(NULL, "|");
    }

    // If there are pipes, execute piped commands
    if (num_cmds > 1) {
        execute_piped_commands(cmds, num_cmds, background);


    } else {
        // Execute a single command in a child process
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pid == 0) {
            exec_single_cmd(command);  // Child executes the command
        } else {
            // Parent waits for the child process
            if (!background) {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    printf("[Command exited with status %d]\n", WEXITSTATUS(status));
                          time_t start_time = time(NULL); // Get the start time
            double duration = difftime(time(NULL), start_time); // Calcula add_to_history(command, pid, start_time, duration); } else if (WIFSIGNALED(status)) {
                    printf("[Command terminated by signal %d]\n", WTERMSIG(status));
                }
            } else {
                printf("[Background process started with PID: %d]\n", pid);
            }
        }
                time_t start_time = time(NULL);add_to_history(command, pid, start_time, 0); // Duration can be set to 0 for background
    }
}

int main() {
    char command[MAX_COMMAND_LEN];

    // Signal handling for graceful termination (Ctrl+C)
    signal(SIGINT, graceful_termination);

    while (1) {
        // Display shell prompt
        printf("my-shell> ");
        // Read command from standard input
        if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
            perror("fgets failed");
            continue;
        }

        // Remove newline from command
        command[strcspn(command, "\n")] = 0;

        // Check for the history command
        if (strcmp(command, "history") == 0) {
            show_history();
        } else {
            // Parse and execute the command
            parse_and_execute_command(command);
        }
    }

    return 0;
}

