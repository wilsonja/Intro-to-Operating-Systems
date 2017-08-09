/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: Jacob Wilson
* Course: CS344_400
* Assignment: Program 3 - smallsh
* Due: 11/17/16
* Description: A simple shell that allows a user to run a few basic
*  built-in commands, or any other command outside of the shell.
*  Shell allows for background processes, comments, directory
*  changes, exit, status, and I/O redirection.
* REFERENCES: http://brennan.io/2015/01/16/write-a-shell-in-c/ was
*  used as an overview for how a shell is built.
* (1) man7.org/linux/man-pages/man2/dup.2.html was a reference
*  for how to use other file descriptor duplicators
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

// FUNCTION PROTOTYPES
void shellLoop();
char** tokenizeInput(char*, int*);
int setBackground(char**, int*);
void changeDirectory(char**, int*);
void setInOut(char**, int*, int, int, int);
void runOtherCommands(char**, int*, int);
void catchInt(int signo);

// MAIN FUNCTION
int main() {
  // sigaction struct set-up taken from CS344 Lecture 13
  struct sigaction act;
  act.sa_handler = catchInt;
  act.sa_flags = 0;
  sigfillset(&(act.sa_mask));

  sigaction(SIGINT, &act, NULL);

  // shell functions are bundled into a loop
  shellLoop();
  return 0;
}

// ALL OTHER FUNCTIONS
/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: shellLoop
* Parameters: none
* Return: none
* Description: Function gets user input, processes it,
*  then calls all other functions. Loop runs until
*  user calls exit. Booleans and exit value are used
*  throughout for tracking purposes.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void shellLoop() {
  // variables to track running of loop and current exit value
  int running = 1;
  int exit_value = 0;

  // loop runs until exit command is called
  do {
    // variables for user input
    char user_input[2048];        // input string
    char **user_args;             // array of string tokens

    // bool variables for tracking purposes
    int is_background = 0;
    int is_input = 0;
    int is_output = 0;

    // additional variables
    int num_args = 0;             // for tracking number of args from user
    int in, out;                  // for adjusting I/O functionality

    // display prompt
    printf(": ");
    fflush(stdout);

    // obtain user input, break into tokenx, and set background bool
    fgets(user_input, sizeof(user_input), stdin);
    user_args = tokenizeInput(user_input, &num_args);
    is_background = setBackground(user_args, &num_args);

    // examine user args to determine if built-in function is called
    if (user_args[0] == NULL || !(strncmp(user_args[0], "#", 1))) {
      exit_value = 0;
    } else if (strcmp(user_args[0], "status") == 0) {    // to check current status
      printf("Exit value %d\n", exit_value);
      exit_value = 0;
    } else if (strcmp(user_args[0], "cd") == 0) {        // to change directory
      changeDirectory(user_args, &exit_value);
    } else if (strcmp(user_args[0], "exit") == 0) {      // to exit shell
      running = 0;
      exit_value = 0;
    } else if (num_args == 3 && ((strcmp(user_args[1], ">") == 0) || (strcmp(user_args[1], "<") == 0))) {
      // if user chooses to redirect input or output
      // first store current standard I/O file descriptors
      in = dup(0);          // dup() creates a copy of a file descriptor and stores it (1)
      out = dup(1);

      // check for redirection of input versus output, then call function
      if ((strcmp(user_args[1], ">") == 0)) {
        is_output = 1;             // set output bool
        setInOut(user_args, &exit_value, is_background, is_input, is_output);
      } else if ((strcmp(user_args[1], "<") == 0)) {
        is_input = 1;             // set input bool
        setInOut(user_args, &exit_value, is_background, is_input, is_output);
      };
      // restore standard I/O file descriptors and close them for use in subsequent loops
      dup2(in, 0);
      dup2(out, 1);
      close(in);
      close(out);
    } else {
      // if anything other than a built-in command was entered, run it in a child process
      runOtherCommands(user_args, &exit_value, is_background);
    };

    // free memory allocated only for argument array
    free(user_args);
  } while (running);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: tokenizeInput
* Parameters: char string, pointer to int
* Return: char array
* Description: Function enters tokens of a user
*  inputted string into an array of chars. Also
*  tracks and sets the number of tokens.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
char** tokenizeInput(char *user_input, int *num_args) {
  char **args = malloc(512 * sizeof(char*));      // allocate space for token array
  char *token;                                    // holds each new token
  int index = 0;                                  // index to store token

  // grab the first token in the user input string
  token = strtok(user_input, " \n");

  // add tokens to the argument array
  while (token != NULL) {
    args[index] = token;              // store the token
    index++;                          // move to next index
    token = strtok(NULL, " \n");      // move to next token
  };
  // nullify last entry in array, set number of arguments, and return array to loop
  args[index] = NULL;
  *num_args = index;
  return args;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: setBackground
* Parameters: char array, pointer to int
* Return: int
* Description: Function compares last argument of
*  char array to determine if '&' character is present.
*  Resets last entry to null if present, returns
*  true or false depending on result.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
int setBackground(char **args, int *num_args) {
  // if user entered '&' at end of command, sets as a background process
  if ((strncmp(args[*num_args - 1], "&", 1)) == 0) {
    args[*num_args - 1] = NULL;       // remove and replace '&' with NULL
    return 1;                         // return true if it is background
  } else {
    return 0;                         // or false if it is foreground
  };
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: changeDirectory
* Parameters: char array, pointer to int
* Return: none
* Description: Function calls chdir() to change
*  directory based on user arguments.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void changeDirectory(char **args, int *exit_value) {
  // if user enters cd without a destination, moves to home directory
  if (args[1] == NULL) {
    chdir(getenv("HOME"));
    *exit_value = 0;
  } else {
    // if user entered a directory that doesn't exist
    if (chdir(args[1]) != 0) {
      printf("File or directory does not exist\n");
      *exit_value = 1;
    } else {
      // if user enters an existing directory
      chdir(args[1]);
    };
  };
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: setInOut
* Parameters: char array, pointer to int, three ints
* Return: none
* Description: Function redirects standard I/O. Will
*  create new file pointer based on redirection, then
*  runs any additional commands.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void setInOut(char **args, int *exit_value, int is_background, int in, int out) {
  // variable to store the file pointer
  int fp = 0;

  // if user has chosen to redirect output
  if (out == 1) {
    // open a file for writing to, error if file doesn't exist
    fp = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fp == -1) {
      printf("Error: unable to open file for writing\n");
      *exit_value = 1;
    } else {
      // redirect the file pointer to stdout, run outside commands
      dup2(fp, 1);
      close(fp);
      args[1] = NULL;             // remove arg from array, no longer needed
      runOtherCommands(args, exit_value, is_background);
    };
  } else if (in == 1) {
    // open a file for reading, error if file doesn't exist
    fp = open(args[2], O_RDONLY);
    if (fp == -1) {
      printf("Error: unable to open file for reading\n");
      *exit_value = 1;
    } else {
      // redirect the file pointer to stdin, run outside commands
      dup2(fp, 0);
      close(fp);
      args[1] = NULL;           // remove arg from array, no longer needed
      runOtherCommands(args, exit_value, is_background);
    };
  };
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: runOtherCommands
* Parameters: char array, pointer to int, int
* Return: none
* Description: Function creates a child process with
*  fork(), then runs a new process in the child and
*  has the parent wait or continue operation.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void runOtherCommands(char **args, int *exit_value, int is_background) {
  // set up pids per CS344 lecture 9
  pid_t pid = -5;
  pid_t wpid = -5;
  int status = 0;

  // spawn a child process
  pid = fork();

  if (pid == 0) {
    // execute outside commands in the child process, error if nonexistant
    if (execvp(args[0], args) == -1) {
      printf("Error: unable to execute command\n");
      exit(1);
    } else {
      // kill off previous running and run new command
      execvp(args[0], args);
    };
  } else if (pid > 0) {
    // in the parent process
    do {
      // wait for child if it is in foreground, or do not wait if background
      if (is_background == 1) {
        wpid = waitpid(-1, &status, WNOHANG);
      } else if (is_background == 0) {
        wpid = waitpid(pid, &status, WUNTRACED);
      };
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  } else {
      // the fork can also fail
      printf("Error: unable to fork");
  };

  // diplay info when background processes complete
  if (is_background == 1) {
    printf("Background pid %d with exit value %d\n", pid, *exit_value);
  };

  // parent sets the exit status
  *exit_value = WEXITSTATUS(status);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * *
* Name: catchInt
* Parameters: int
* Return: none
* Description: Function displays signals that were
*  used to kill a process.
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void catchInt(int signo) {
  // display the caught signal
  printf(" terminated by signal: %d\n", signo);
};
