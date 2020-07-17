#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

const int BUF_SIZE = 256;
struct termios original_mode;
pid_t pid;

void shell();
void no_shell();
void reset_mode();
void change_mode();
void check_err();
void check_err_sh();
int process_opt();

int main(int argc, char* argv[]){
  int opt;
  
  // Save the current terminal mode for restoration
  check_err(tcgetattr(0, &original_mode), "tcgetattr");
  change_mode();

  // Process options
  opt = process_opt(argc, argv);
  
  if (opt == 0)
    no_shell();
  else if (opt == 1)
    shell();
  else{
    reset_mode();
    exit(1);
  }

  // Reset the terminal mode
  reset_mode();
  exit(0);
}

// Used when no option is given
// return ... success 
// exit with 1 ... error
void no_shell(){
  int read_size;
  char buffer[BUF_SIZE];
  
  while(1){ 
    read_size = read(0, &buffer, BUF_SIZE);
    check_err(read_size, "read");
    
    // Read one byte at a time
    int i;
    for (i=0; i<read_size; i++){
      // Detect ^D
      if (buffer[i]=='\004'){
	write(1, &"^D", sizeof(char)*2);
	return;
      }
      // translate <cr> or <lf> into <cr><lf>
      else if (buffer[i]=='\n'||buffer[i]=='\r')
	check_err(write(1, &"\r\n", sizeof(char)*2), "write"); 
      else
	check_err(write(1, &buffer[i], sizeof(char)), "write");
    }//end for
  }//end while
}


// Used with '--shell' option
// return ... success
// exit with 1 ... erro
void shell(){
  int read_size, fd_in;
  char buffer[BUF_SIZE];
  struct pollfd fds[2];
  int ter2sh[2], sh2ter[2];
  int status;
  
  check_err(pipe(ter2sh), "pipe"); // 0=read, 1=write 
  check_err(pipe(sh2ter), "pipe");
  
  fds[0].fd = 0;         
  fds[1].fd = sh2ter[0];
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[1].events = POLLIN | POLLHUP | POLLERR;
    
  // Create a child process
  pid = fork();
  check_err(pid, "fork");

  // Exec a shell in the child process
  if (pid==0){
    // Close unused ends of the pipes
    check_err(close(ter2sh[1]), "close");
    check_err(close(sh2ter[0]), "close");    
 
    check_err(dup2(ter2sh[0],0), "dup2"); // ter2sh[0] -> stdin
    check_err(dup2(sh2ter[1],1), "dup2"); // sh2ter[1] -> stdout
    check_err(dup2(sh2ter[1],2), "dup2"); // sh2ter[1] -> stderr
     
    check_err(close(ter2sh[0]), "close");
    check_err(close(sh2ter[1]), "close");
    
    // Run a shell
    check_err(execlp("/bin/bash", "/bin/bash/", NULL), "execlp"); 
  } 
  
  // Close unused end of the pipes
  check_err_sh(close(sh2ter[1]), "close");
  check_err_sh(close(ter2sh[0]), "close");
 
  // Read and write in the parent process
  while(1){
    int done;
    check_err_sh(poll(fds, 2, 0), "poll");

    int j;
    for (j=0; j<2; j++){
      fd_in = j*sh2ter[0];

      // Input waiting
      if (fds[j].revents & POLLIN){
	read_size = read(fd_in, &buffer, BUF_SIZE);
	check_err_sh(read_size, "read");
  
	// Read one byte at a time
	int i;
	for (i=0; i<read_size; i++){
	  // Detect ^D
	  if (buffer[i]=='\004'){
	    check_err_sh(write(1, &"^D", sizeof(char)*2), "write");
	    check_err_sh(close(ter2sh[1]), "close");
	  }
	  // Detect ^C
	  else if (buffer[i]=='\003'){
	    check_err_sh(write(1, &"^C", sizeof(char)*2), "write");
	    check_err_sh(kill(pid, SIGINT), "kill");
	  }
	  // translate <cr> or <lf> into <cr><lf>
	  else if (buffer[i]=='\n'||buffer[i]=='\r'){
	    check_err_sh(write(1, &"\r\n", sizeof(char)*2), "write");
	    if (!j)
	      check_err_sh(write(ter2sh[1], &"\n", sizeof(char)), "write");
	  }
	  else {
	    check_err_sh(write(1, &buffer[i], sizeof(char)), "write");
	    if (!j)
	      check_err_sh(write(ter2sh[1], &buffer[i], sizeof(char)), "write");
	  }
	}
      }

      // Write end is closed
      else if (fds[j].revents & POLLHUP){
	//fprintf(stderr, "POLLHUP %d \n\r", j);
	done = 1;
	break;
      }
      // Read end is closed
      else if (fds[j].revents & POLLERR){
	//fprintf(stderr, "POLLERR %d \n\r", j);
	done = 1;
	break;
      }
    }//end outer for
    if (done==1)
      break;
  }//end while

  check_err_sh(close(sh2ter[0]), "close");
  
  waitpid(pid, &status, 0);
  int sig, sts;
  if (WIFEXITED(status)){
    // SPEC SAYS OPPOSITE!!!
    sts = WEXITSTATUS(status) & 0x007F;
    sig = WEXITSTATUS(status) & 0xFF00;
  }
  else if (WIFSIGNALED(status)){
    sig = WTERMSIG(status) & 0x007F;
    sts = WTERMSIG(status) & 0xFF00;
  }
  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d",sig,sts);
}


// Resets the terminal mode
void reset_mode(){
  tcsetattr(0, TCSANOW, &original_mode);
}

// Changes the terminal mode
// If either tcgetattr() or tcsetattr() fail, exit with 1
void change_mode(){
  struct termios temp;
  
  // Copy the current terminal mode
  check_err(tcgetattr(0, &temp),"tcgetattr");
  
  // Modify the mode
  temp.c_iflag = ISTRIP; // Input modes: strip off 8 bits
  temp.c_oflag = 0;  // Output modes: no processing
  temp.c_lflag = 0; // Local modes: no processing
  check_err(tcsetattr(0,TCSANOW, &temp), "tcsetattr");
}


// Checks system call errors for no_shell()
// return ... no error
// exit with 1 ... otherwise
void check_err(int rtn, char* call){
  // No error
  if (rtn > -1)
      return;
  // Error
  fprintf(stderr, "system call '%s' failed: %s \r\n", call, strerror(errno));
  reset_mode();
  exit(1);
}


// Checks system call errors for shell()
// returns ... no error
// exit with 1 ... otherwise
void check_err_sh(int rtn, char* call){
  // No error
  if (rtn > -1)
      return;
  // Error
  fprintf(stderr, "system call '%s' failed: %s \r\n", call, strerror(errno));
  kill(pid, SIGINT);
  reset_mode();
  exit(1);
}


// Processes command line arguments
// Return values:
// 0 ... no options
// 1 ... --shell used
// 2 ... invalid option used
int process_opt(int argc, char* argv[]){
  int shell_opt=0, inval_opt=0, opt;

  struct option long_opts[]={
    {"shell", no_argument, NULL, 's'},
    {0, 0, 0, 0}
  };
  
  while (1){
    opt = getopt_long(argc, argv, "", long_opts, NULL);
    if(opt == -1)
      break;
    switch(opt){
    case 's':
      shell_opt = 1;
      break;
    case ':':
    default:
      inval_opt = 1;
      break;
    } //end switch
  }//end while

  // Invalid options -> return 2
  if (inval_opt==1)
    return 2;
  // '--shell' -> return 1
  else if (shell_opt==1)
    return 1;
  // No option -> return 0
  else 
    return 0;
}
