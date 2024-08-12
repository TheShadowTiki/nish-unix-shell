
# nish: A Simple Unix Shell

`nish` is a simple and efficient Unix shell written in C. It serves as a command-line interpreter (CLI) that executes user commands by creating child processes. This shell supports basic command execution, built-in commands, output redirection, and parallel command execution.

## Features

- **Command Execution**: Execute standard Unix commands by typing them at the prompt.
- **Built-in Commands**: Supports `exit`, `cd`, and `path`.
- **Output Redirection**: Redirect command output to a file using `>`.
- **Parallel Execution**: Run multiple commands in parallel using `&`.
- **Error Handling**: Provides a unified error message for any shell-related errors.

## Installation

To compile and run `nish`, you need to have GCC installed on your Unix system.

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/nish.git
   cd nish
   ```

2. Compile the source code using the provided `Makefile`:
   ```bash
   make
   ```

3. Run the shell:
   ```bash
   ./nish
   ```

## Usage

Once the shell is running, it will display the prompt `nish>`. You can then type commands as you would in any Unix shell.

### Built-in Commands

- **exit**: Exit the shell.
  ```bash
  nish> exit
  ```

- **cd**: Change the current directory.
  ```bash
  nish> cd /path/to/directory
  ```

- **path**: Set the search path for executables.
  ```bash
  nish> path /bin /usr/bin
  ```

### Output Redirection

Redirect the output of a command to a file.

```bash
nish> ls -l > output.txt
```

### Parallel Execution

Run multiple commands in parallel using the `&` operator.

```bash
nish> cmd1 & cmd2 & cmd3
```

### Error Handling

The shell provides a unified error message for any syntax or execution errors. This error message is printed to standard error (stderr).

```c
An error has occurred

```

## Demonstration

Below is a demonstration of `nish` showcasing various features:

```bash
# Display the current directory
nish> pwd
/home/mikoz/nish

# List all files in the directory
nish> ls -lah
total 44K
drwxr-xr-x  2 mikoz mikoz 4.0K Aug 12 15:45 .
drwxr-x--- 10 mikoz mikoz 4.0K Aug 12 15:24 ..
-rw-r--r--  1 mikoz mikoz   31 Aug 12 15:26 Makefile
-rwxr-xr-x  1 mikoz mikoz  17K Aug 12 15:26 nish
-rw-r--r--  1 mikoz mikoz 9.4K Aug 12 15:26 src.c

# Redirect output to a file
nish> echo "Hello, World!" > hello.txt

# Display the contents of a file
nish> cat hello.txt
"Hello, World!"

# Run commands in parallel
nish> echo "I can do this" & echo "I can do that" & ls
"I can do this"
"I can do that"
Makefile  hello.txt  nish  src.c

# Demonstrate error handling with incorrect syntax
nish> cat "this will not work" > > whoops.txt
An error has occurred

# Exit the shell
nish> exit
```

## Files

- `Makefile`: A makefile for building the `nish` executable.
- `src.c`: The C source file containing the implementation of `nish`.
