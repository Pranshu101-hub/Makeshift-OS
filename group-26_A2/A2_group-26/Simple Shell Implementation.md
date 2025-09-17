
**SIMPLESHELL IMPLEMENTATION:**
---

Core Logic: The shell is built around an infinite while loop in the shell\_loop function. This loop continuously displays a prompt, reads a line of user input, and then dispatches the command to the correct execution function.

Data Structure for History: A struct named cmdent is used to store comprehensive details about each executed command, including the command string, its process ID (PID), start time, and total execution duration. These structs are stored in a global array named history\_log.

Command Parsing: User input is read using read\_cmdline (which utilizes fgets for safety) and then parsed into arguments. The parse\_arguments function uses strtok to split a command string by spaces into a NULL-terminated array suitable for execvp.



Execution Paths:

Simple Commands: For commands without pipes, the exec\_cmd function employs the standard fork-exec-wait model to create a child process and wait for its completion.

Piped Commands: The exec\_pipecmd function handles commands with one or more pipes. It first splits the input string by the | character into an array of sub-commands. It then iteratively creates a child process for each sub-command, connecting them by creating pipes and redirecting their standard input and output using the dup2 system call. The parent process waits for all children in the pipeline to finish.


Special Features:

History: A built-in history command is implemented to display the detailed log of commands from the current session.

Signal Handling: The shell catches the SIGINT signal (Ctrl+C) using a custom handler (handle\_sigint), which displays the command history before gracefully terminating the program.



**GitHub Link - https://github.com/Pranshu101-hub/OperatingSystems-Assignment/tree/main/group-26_A2/A2_group-26

**Contributions:**
*Dewang -*
1. Implemented command execution via process creation (fork, execvp, wait).
2. Built pipeline execution (single and multi-stage pipe handling with proper redirection).
3. Added argument parsing and input reading with safe memory management.
4. Integrated program entry point (main) with cleanup before exit.

*Pranshu -*
1. Implemented command history management (storing, printing, and cleanup).
2. Wrote logic for graceful signal handling (Ctrl+C → show history \& exit).
3. Designed time formatting utilities for better log readability.
4. Developed the interactive shell loop (prompt display, built-in exit and history handling, command dispatch).







   


