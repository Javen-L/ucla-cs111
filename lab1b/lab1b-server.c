#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "zlib.h"

#define BUF_SIZE 2304

pid_t pid;
int sockfd=0, new_sockfd=0;

void read_write();
void read_write_comp();
void check_err();
int process_opt();
void create_socket();
int def();
int inf();
void check_zerr();

int main(int argc, char* argv[]){

  // --compress used
  if (process_opt(argc, argv))
    read_write_comp();
  // --compress not used
  else
    read_write();

  // Close sockfd and new_sockfd
  if (sockfd)
    check_err(close(sockfd), "close");
  if (new_sockfd)
    check_err(close(new_sockfd), "close");
  exit(0);
}


// Default read_write function
// return ... success
// exit with 1 ... error
void read_write(){
  struct pollfd fds[2];
  int ter2sh[2], sh2ter[2];

  check_err(pipe(ter2sh), "pipe"); // 0=read, 1=write 
  check_err(pipe(sh2ter), "pipe");
  
  fds[0].fd = new_sockfd;         
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
    check_err(close(new_sockfd), "close");
    check_err(close(sockfd), "close");    
 
    check_err(dup2(ter2sh[0],0), "dup2"); // ter2sh[0] -> stdin
    check_err(dup2(sh2ter[1],1), "dup2"); // sh2ter[1] -> stdout
    check_err(dup2(sh2ter[1],2), "dup2"); // sh2ter[1] -> stderr
     
    check_err(close(ter2sh[0]), "close");
    check_err(close(sh2ter[1]), "close");
    
    // Run a shell
    check_err(execlp("/bin/bash", "/bin/bash/", NULL), "execlp"); 
  } 
  
  char buffer[BUF_SIZE];
  int read_size, status;
  int fd_in[2]={new_sockfd, sh2ter[0]};
  int fd_out[2]={ter2sh[1], new_sockfd};

  // Close unused end of the pipes
  check_err(close(sh2ter[1]), "close");
  check_err(close(ter2sh[0]), "close");
 
  // Read and write in the parent process
  int polling = 1;
  while(polling){
    check_err(poll(fds, 2, 0), "poll");

    int shell;
    for (shell=0; shell<2; shell++){

      // Input waiting
      if (fds[shell].revents & POLLIN){
      	read_size = read(fd_in[shell], buffer, BUF_SIZE);
        check_err(read_size, "read");
        if (read_size==0){
          polling = 0;
          break;
        }

        // shell -> client
        int i;
        if (shell){
          //check_err(write(fd_out[shell], buffer, read_size), "write");
          for (i=0; i<read_size; i++)
            check_err(write(fd_out[shell], &buffer[i], sizeof(char)), "write");
        }
	      // client -> shell
        else{
          for (i=0; i<read_size; i++){
            // Detect ^D from the socket
            if (buffer[i]=='\004')
              check_err(close(fd_out[shell]), "close");
            // Detect ^C from the socket
            else if (buffer[i]=='\003')
              check_err(kill(pid, SIGINT), "kill");
            else if (buffer[i]=='\n'||buffer[i]=='\r')
              check_err(write(fd_out[shell], &"\n", sizeof(char)), "write");
            else	
              check_err(write(fd_out[shell], &buffer[i], sizeof(char)), "write");
          }
        }//end if POLLIN
        bzero(buffer, BUF_SIZE);
      }
      // Write/read end is closed
      else if (fds[shell].revents & POLLHUP || fds[shell].revents & POLLERR){
        polling = 0;
        break;
      }
    }//end for        
  }//end while

  check_err(close(sh2ter[0]), "close");
  
  // Report exit status of the shell
  check_err(waitpid(pid, &status, 0), "waitpid");
  int sig, sts;
  if (WIFEXITED(status)){
    sts = WEXITSTATUS(status) & 0x007F;
    sig = WEXITSTATUS(status) & 0xFF00;
  }
  else if (WIFSIGNALED(status)){
    sig = WTERMSIG(status) & 0x007F;
    sts = WTERMSIG(status) & 0xFF00;
  }
  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",sig,sts);
}


// Used with '--compress' 
// return ... success
// exit with 1 ... erro
void read_write_comp(){
  struct pollfd fds[2];
  int ter2sh[2], sh2ter[2];

  check_err(pipe(ter2sh), "pipe"); // 0=read, 1=write 
  check_err(pipe(sh2ter), "pipe");
  
  fds[0].fd = new_sockfd;         
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
    check_err(close(new_sockfd), "close");
    check_err(close(sockfd), "close");    
 
    check_err(dup2(ter2sh[0],0), "dup2"); // ter2sh[0] -> stdin
    check_err(dup2(sh2ter[1],1), "dup2"); // sh2ter[1] -> stdout
    check_err(dup2(sh2ter[1],2), "dup2"); // sh2ter[1] -> stderr
     
    check_err(close(ter2sh[0]), "close");
    check_err(close(sh2ter[1]), "close");
    
    // Run a shell
    check_err(execlp("/bin/bash", "/bin/bash/", NULL), "execlp"); 
  } 
  
  char buffer[BUF_SIZE];
  char temp[BUF_SIZE];
  int read_size, temp_size; 
  int status;
  int fd_in[2]={new_sockfd, sh2ter[0]};
  int fd_out[2]={ter2sh[1], new_sockfd};

  // Close unused end of the pipes
  check_err(close(sh2ter[1]), "close");
  check_err(close(ter2sh[0]), "close");
 
  // Read and write in the parent process
  int polling = 1;
  while(polling){
    check_err(poll(fds, 2, 0), "poll");

    int shell;
    for (shell=0; shell<2; shell++){

      // Input waiting
      if (fds[shell].revents & POLLIN){
      	temp_size = read(fd_in[shell], temp, BUF_SIZE);
      	check_err(temp_size, "read");
        
        // Compress 1 byte at a time: shell -> client
        int i;
        if(shell){
          read_size = def(temp, buffer, Z_DEFAULT_COMPRESSION, temp_size);
          check_err(write(fd_out[shell], buffer, read_size), "write");
          /*
          for (i=0; i<temp_size; i++){
            read_size = def(&temp[i], buffer, Z_DEFAULT_COMPRESSION, 1);
            check_err(write(fd_out[shell], buffer, read_size), "write");
          }
          */
        }

        // Decompress all at once: client -> shell
        else{
          read_size = inf(temp, buffer, temp_size);

          for (i=0; i<read_size; i++){
            // Detect ^D from the socket
            if (buffer[i]=='\004')
              check_err(close(fd_out[shell]), "close");
            // Detect ^C from the socket
            else if (buffer[i]=='\003')
              check_err(kill(pid, SIGINT), "kill");
            else if(buffer[i]=='\n' || buffer[i]=='\r')
              check_err(write(fd_out[shell], &"\n", sizeof(char)), "write");
            else
              check_err(write(fd_out[shell], &buffer[i], sizeof(char)), "write");
          }
        }
        bzero(buffer, BUF_SIZE);
        bzero(temp, BUF_SIZE);
      }
      // Write/read end is closed
      else if (fds[shell].revents & POLLHUP || fds[shell].revents & POLLERR){
	      polling = 0;
	      break;
      }
    }//end for
  }//end while

  check_err(close(sh2ter[0]), "close");
  
  // Report exit status of the shell
  check_err(waitpid(pid, &status, 0), "waitpid");
  int sig, sts;
  if (WIFEXITED(status)){
    sts = WEXITSTATUS(status) & 0x007F;
    sig = WEXITSTATUS(status) & 0xFF00;
  }
  else if (WIFSIGNALED(status)){
    sig = WTERMSIG(status) & 0x007F;
    sts = WTERMSIG(status) & 0xFF00;
  }
  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",sig,sts);
}


// Checks system call errors
// returns ... no error
// exit with 1 ... otherwise
void check_err(int rtn, char* call){
  // No error
  if (rtn > -1)
      return;
  // Error
  fprintf(stderr, "system call '%s' failed: %s \r\n", call, strerror(errno));
  if (pid>0)
    kill(pid, SIGINT);
  if (sockfd>0)
    close(sockfd);
  if (new_sockfd>0)
    close(new_sockfd);
  exit(1);
}


// Processes command line arguments
// Returns 1 if '--compress' used, 0 otherwise
int process_opt(int argc, char* argv[]){
  int invalid=0, missing=0, comp=0, port=0, opt;

  struct option long_opts[]={
    {"port", required_argument, NULL, 'p'},
    {"compress", no_argument, NULL, 'c'},
    {0, 0, 0, 0}
  };
  
  while (1){
    opt = getopt_long(argc, argv, ":", long_opts, NULL);
    if(opt == -1)
      break;
    switch(opt){
    case 'p':
      port=1;
      create_socket(atoi(optarg));
      break;
    case 'c':
      comp = 1;
      break;
    case ':':
      missing = 1;
      fprintf(stderr, "error: option '%s' requires an argument \r\n",
	      argv[optind-1]);
      break;
    case '?':
    default:
      invalid = 1;
      fprintf(stderr, "error: option '%s' is not recognized \r\n",
	      argv[optind-1]);
      break;
    } //end switch
  }//end while

  // Missing arg or invalid op 
  if (missing || invalid){
    exit(1);
  }
  // Port not specified
  if (!port){
    fprintf(stderr, "error: port number not specified \r\n");
    exit(1);
  }
  return comp;
}


// Creates a socket
void create_socket(int p_num){
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t serv_addrlen, cli_addrlen;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  check_err(sockfd, "socket");
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(p_num);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
 
  serv_addrlen = sizeof(serv_addr);
  check_err(bind(sockfd, (struct sockaddr*) &serv_addr, serv_addrlen), "bind");
  check_err(listen(sockfd, 5), "listen");

  cli_addrlen = sizeof(cli_addr);
  new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addrlen);
  check_err(new_sockfd, "accept");
}


// Compress
int def(unsigned char* src, unsigned char* dst, int level, int read_size){
  z_stream strm;
  int total;

  // Allocate deflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  check_zerr(deflateInit(&strm, level), "deflateInit");

  strm.avail_in = read_size;
  strm.next_in = src;

  // Use deflate()
  do {
    strm.avail_out = BUF_SIZE;
    strm.next_out = dst;
    deflate(&strm, Z_FINISH);
    total = BUF_SIZE - strm.avail_out;
  } while (strm.avail_out == 0);

  // Clearn up
  (void)deflateEnd(&strm);
  return total;
}


// Decompress
int inf(Bytef* src, Bytef* dst, int read_size){
  z_stream strm;
  int total;

  // Allocate inflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  check_zerr(inflateInit(&strm), "inflateInit");

  strm.avail_in = read_size;
  strm.next_in = src;
  // Use inflate()
  do {
    strm.avail_out = BUF_SIZE;
    strm.next_out = dst;
    int ret = inflate(&strm, Z_FINISH);
    if (ret < 1){
      (void)inflateEnd(&strm);
      check_zerr(ret, "inflate");
    }
    if(ret == 2)
      check_err(ret, "inflate");
    total = BUF_SIZE - strm.avail_out;
  } while (strm.avail_out == 0);
  (void)inflateEnd(&strm);
  return total;
}


// Check errors for zlib functions
void check_zerr(int ret, char* func){
  // No error
  if (ret==0)
    return;
  // Error
  fprintf(stderr, "error: zlib function '%s' failed: %d \r\n", func, ret);
  if (pid>0)
    kill(pid, SIGINT);
  if (sockfd>0)
    close(sockfd);
  if (new_sockfd>0)
    close(new_sockfd);
  exit(1); 
}
