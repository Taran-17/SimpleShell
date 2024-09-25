

# SimpleShell

## Overview

SimpleShell is a lightweight command-line shell implementation in C that allows users to execute commands, including those with multiple pipes and background processes. This shell tracks command execution history, including the command, process ID, start time, and duration.

## Features

- **Command Execution**: Execute single or piped commands.
- **History Tracking**: Keep track of executed commands with details such as PID, start time, and duration.
- **Background Processing**: Run commands in the background using the `&` symbol.
- **Graceful Termination**: Exit the shell while displaying the command history.

## Getting Started

### Prerequisites

- A C compiler (e.g., `gcc`)
- A Unix-based environment (Linux, macOS, etc.)

### Compilation

To compile the SimpleShell, use the following command:

```bash
gcc -o simpleshell simpleshell.c
```

### Usage

1. **Run the Shell**:
   Start the shell by executing the compiled binary:
   ```bash
   ./simpleshell
   ```

2. **Basic Commands**:
   You can execute commands just like in a standard shell. For example:
   ```bash
   ls -l
   ```

3. **Piped Commands**:
   To execute multiple commands in a pipeline, separate them with the `|` character. For example:
   ```bash
   ls -l | grep ".c" | wc -l
   ```
   This command lists files in long format, filters for `.c` files, and counts them.

4. **Background Processes**:
   To run a command in the background, append `&` to the command:
   ```bash
   sleep 10 &
   ```
   The shell will print the PID of the background process.

5. **Command History**:
   To view the history of executed commands, simply type:
   ```bash
   history
   ```
   This will display a list of all commands executed in the current session along with their details.

6. **Exiting the Shell**:
   To exit the shell gracefully, press `Ctrl+C`. This will display the command history before exiting.

## Implementation Details

### Core Components

- **Command History Struct**: The `CommandHistory` struct is used to store details about each executed command, including:
  - Command string
  - Process ID (PID)
  - Start time
  - Duration of execution

- **Functions**:
  - `add_to_history()`: Adds executed command details to the history.
  - `show_history()`: Displays the command history.
  - `tokenize_command()`: Splits a command string into tokens for execution.
  - `exec_single_cmd()`: Executes a single command without waiting for input/output redirection.
  - `execute_piped_commands()`: Manages the execution of multiple piped commands, including setting up pipes and redirecting input/output.
  - `parse_and_execute_command()`: Parses a command string to determine if it should execute a single command or multiple piped commands.

### Signal Handling

The shell uses the `signal()` function to handle graceful termination, allowing it to cleanly exit and show the command history when the user presses `Ctrl+C`.

## Conclusion

SimpleShell is an educational project designed to demonstrate basic shell functionalities, process management, and command history tracking. It provides a straightforward interface for users to execute commands while learning about the underlying operations of a shell.

## Future Improvements

- Add support for more shell features like environment variables, input/output redirection, and custom built-in commands.
- Implement error handling for invalid commands and improve user feedback.

--- 

