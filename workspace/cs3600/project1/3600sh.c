/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * Clayton Mentzer & Ronny Brzezinski
 * GROUP NAME: aaaswagblaster420
 * Due Date: 30 September 2013
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

int file_out;
int bad_escape;
int exitcode;
int file_in;
int error_out;

int stdout_copy;
int stdin_copy;
int stderr_copy;

//Main
int main(int argc, char*argv[]) {

  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
  if(argc > 1){
	freopen(argv[1], "r", stdin);
  }
  //This while loop prints the shell's prompt and reads input
  //from the user, also handles exit command. 
  while(1){
    bad_escape = 0;
    //String to store user's input
    char cmd[2000000];

    //Array to store argument words
    char *childargv[30];
    int childargc = 0;

    //Variables to be used in the printing of the prompt
    char* cwd = getcwd(NULL, 0);
    char* user = getenv("USER");
    char host[1024];
    if(feof(stdin)){
      do_exit();
    }

    //Get the hostname, username, and directory to print in the prompt
    gethostname(host, sizeof(host));	
    printf("%s@%s:%s> ", user, host, cwd);
    getargs(cmd, &childargc, childargv);

    //bad_escape is set to TRUE if there is an improper escape sequence
    //this if statement prevents part of the input from running if there
    //is a bad escape sequence somewhere else in the input. 
    if(bad_escape == 1){
	childargc = 0;
    }
    if(childargc > 0 && strcmp(childargv[0], "exit") == 0){
      do_exit();
    }
    //execute the arguments given
    else { execute(childargc, childargv); } 
  }
  return(0);
}
//the getword function takes in an input string and 
static char * getword(char * begin, char **end_ptr){
	//this while loop removes leading spaces
	while(*begin == ' ' || *begin == '\t' || *begin == '\n'){
		begin++;
	}
	exitcode = 1; //dont exit
	int len = strlen(begin);
	int i = 0;
	char * temp = begin;
	char * end = begin; //declare pointer to the current character
	//step through the string until a special character is found
	while(*end != '\0' && *end != '\n' && *end != ' ' && *end != '#'){
		//if a / is found, check the next character after the / and 
		//act accordingly.
		if(*end == 0x5c){
			temp = end;
			end++;
			//if space, apersand, or /, remove the preceding / 
			//and cat the two strings
			if(*end == ' ' || *end == 0x26 || *end == 0x5c){
				memcpy(temp, end, ((len - i) * sizeof(char)));
				end++;
			//if tab, remove the / and cat the two strings, leaving the tab
			} else if(*end == 't'){
				memcpy(temp, end, ((len - i) * sizeof(char)));
				end--;
				*end = 0x09;
				end++;
			//else if any other char, bad things happen
			} else {
				printf("Error: Unrecognized esacpe sequence.\n");
				bad_escape = 1;
				return NULL;
			}
		} else {
			end++;
		}
	}
	//no more arguments left to parse
	if(end == begin){ return NULL; }
	//make this location a null byte and reutrn the parsed word
	else { *end = '\0', *end_ptr = end; }
  return begin;
}

//getargs uses getword to parse the user input into an array of strings in argv
static void getargs(char cmd[], int *argcp, char *argv[])
{
	char* cmdp = cmd;
	char* end; 
	int i = *argcp;
	//read from stdin
	if(fgets(cmd, 1024, stdin) == NULL && feof(stdin)){
		do_exit();
	}
	//call getword to get the first word of the input string
	while((cmdp = getword(cmdp, &end)) != NULL){
	//if >, handle output redirection
	  if(strcmp(cmdp, ">") == 0){
	    //move to the next char
	    cmdp = end + 1;
	    cmdp = getword(cmdp, &end);
	    if(cmdp) {
	      file_out = open(cmdp, O_WRONLY | O_CREAT);
	      stdout_copy = dup(STDOUT_FILENO);
	      dup2(file_out, STDOUT_FILENO);
	      cmdp = end + 1;
	      exitcode = 0;
	    }
	    else
	      printf("Error: unable to open redirection file\n");
	  }
	//if <, handle input redirection
	  else if(strcmp(cmdp, "<") == 0){
	    //move to the next char
	    cmdp = end + 1;
	    cmdp = getword(cmdp, &end);
	    if( cmdp ){
	      file_in = open(cmdp, O_RDONLY);
	      stdin_copy = dup(STDIN_FILENO);
	      dup2(file_in, STDIN_FILENO);
	      cmdp = end + 1;
	      exitcode = 0;
	    }
	    else
	      printf("Error: Unable to open redirection file\n");

	  }
	  //more output redirection
	  else if(strcmp(cmdp, "2>") == 0){
 	    //move to the next char
	    cmdp = end + 1;
	    cmdp = getword(cmdp, &end);
	    if( cmdp ){
	      error_out = open(cmdp, O_WRONLY);
	      stderr_copy = dup(STDERR_FILENO);
	      dup2(error_out, STDERR_FILENO);
	      exitcode = 0;
	      cmdp = end + 1;
	    }
	    else
	      printf("Error: Unable to open redirection file\n");
	  }
	  //else set this word equal to the first member of the argv[] array. 
	  else{
	    argv[i++] = cmdp;
	    cmdp = end + 1;
	  }
	}
	argv[i] = NULL;
	//increment the number of arguments.
	*argcp = i;
}

void interrupt_handler(int signum){
	pid_t childpid = signum;
	printf(" (^C recognized, killing child process\n");
	kill(childpid, SIGINT);
}

//read from argv and execute the commands found there
static void execute(int argc, char *argv[]){
  if(argc == 0){
	return;
  }
  pid_t childpid;
  signal(SIGINT, interrupt_handler);
  //create child process
  childpid = fork();
  //handle errors
  if(argc < 0){
	printf("argc is less than 0");
  }
  if(childpid == -1){
	printf("Error: Failed to execute command.\n");	
	exit(1);
  }
  //if the given argument does not have a corresponding executable
  if(childpid == 0 && exitcode){
	if(-1 == execvp(argv[0], argv)){
		printf("Error: Command not found.\n");
	}
	exit(1);
  //wait for child process to finish
  } else {
	waitpid(childpid, NULL, 0);
  }

  //handle output redirection 
  if(file_out){
    if(-1 == close(file_out)){
      printf("Error: Unable to open redirection file\n");
    }
    //cpoy to the output file and then close it. 
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    file_out = 0;
  }
  //handle input redirection
  if(file_in){
    if(-1 == close(file_in)){
      printf("Error: Unable to open redirection file\n");
    }
    //read from the file and then close it
    dup2(stdin_copy, STDIN_FILENO);
    close(stdin_copy);
    file_in = 0;
  }
  //handle input/output redirection errors
  if(error_out){
    if(-1 == close(error_out)){
      printf("Error: Unable to open redirection file\n");
    }
    dup2(stderr_copy, STDERR_FILENO);
    close(stderr_copy);
    error_out = 0;
  }
  argv = NULL;
  argc = 0;

  return;
}

// Function which exits, printing the necessary message
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}





