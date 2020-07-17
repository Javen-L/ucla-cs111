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
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <ulimit.h>
#include "zlib.h"

#define BUF_SIZE 2304

struct termios original_mode;
int sockfd=0, logfd=0, p_num;
int COM_UNIT=0;

void read_write();
void read_write_comp();
void reset_mode();
void change_mode();
void check_err();
int process_opt();
void create_socket();
void keep_log();
void copy_str();
int def();
int inf();
void check_zerr();

int main(int argc, char* argv[]){
  int comp;

  // Save the current terminal mode for restoration
  check_err(tcgetattr(0, &original_mode), "tcgetattr");

  comp = process_opt(argc, argv);
  change_mode();

  if (comp)
    read_write_comp();
  else
    read_write();

  // Close sockfd and log_p
  if (sockfd)
    check_err(close(sockfd), "close");    
  if (logfd)
    close(logfd);
    
  // Reset the terminal mode
  reset_mode();
  exit(0);
}


// Copy stdin -> server, stdout and server -> stdout 
// return ... success
// exit with 1 ... erro
void read_write(){
  int fd_in, read_size;
  char buffer[BUF_SIZE];
  struct pollfd fds[2];

  fds[0].fd = 0;         
  fds[1].fd = sockfd;
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[1].events = POLLIN | POLLHUP | POLLERR;
    
  int polling=1;
  while(polling){
    check_err(poll(fds, 2, 0), "poll");

    int received;
    for (received=0; received<2; received++){
      fd_in = received*sockfd;

      // Read and write
      if (fds[received].revents & POLLIN){
        read_size = read(fd_in, buffer, BUF_SIZE);
        check_err(read_size, "read");
        if (read_size==0){
          polling = 0;
          break;
        }
      
        // Keep log of received data
        if (logfd && received)
          keep_log(received, buffer, read_size);

        // Read one byte at a time
        int i;
        for (i=0; i<read_size; i++){
          
          // Write on stdin
          // Detect ^D
          if (buffer[i]=='\004')
            check_err(write(1, &"^D", sizeof(char)*2), "write");
          // Detect ^C
          else if (buffer[i]=='\003')
            check_err(write(1, &"^C", sizeof(char)*2), "write");
          // translate <cr> or <lf> into <cr><lf>
          else if (buffer[i]=='\n'||buffer[i]=='\r')
            check_err(write(1, &"\r\n", sizeof(char)*2), "write");
          else 
            check_err(write(1, &buffer[i], sizeof(char)), "write");

          // Write on socket
          if(!received){
            check_err(write(sockfd, &buffer[i], sizeof(char)), "write");
            if(logfd)
              keep_log(received, &buffer[i], 1);
          } 
        }//end for
        bzero(buffer, BUF_SIZE);
      }
      // Write/read end is closed
      else if (fds[received].revents & POLLHUP || fds[received].revents & POLLERR){
        polling = 0;
        break;
      }
    }//end for
  }//end while
}

// Copy stdin -> server, stdout and server -> stdout
// Data sent/received compressed
// return ... success
// exit with 1 ... erro
void read_write_comp(){
  int fd_in;
  int read_size, comp_size, temp_size;
  char buffer[BUF_SIZE] = {'\0'};
  char buffer_comp[BUF_SIZE] = {'\0'};
  char temp[BUF_SIZE] = {'\0'};
  struct pollfd fds[2];

  fds[0].fd = 0;         
  fds[1].fd = sockfd;
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[1].events = POLLIN | POLLHUP | POLLERR;
    
  int polling=1;
  while(polling){
    check_err(poll(fds, 2, 0), "poll");

    int received;
    for (received=0; received<2; received++){
      fd_in = received*sockfd;

      // Read and write
      if (fds[received].revents & POLLIN){
      	temp_size = read(fd_in, temp, BUF_SIZE);
	      check_err(temp_size, "read");
	      if (temp_size==0){
          polling = 0;
          break;
        }
        read_size = temp_size;
 
        // Decompress all data received
        if (received){
          // Keep log of received data
          if(logfd)
            keep_log(received, temp, temp_size);
          read_size = inf(temp, buffer, temp_size);
          // Decompress one block at a time
          /*
          int k;
          for (k=0; k<temp_size/COM_UNIT; k++)
            read_size = inf(&temp[k*COM_UNIT], &buffer[k], (k+1)*COM_UNIT);
          read_size = temp_size/COM_UNIT;
          */
        }

        int i;
        for (i=0; i<read_size; i++){
          // Compress 1 byte at a time: stdin -> server
          if (!received){
            comp_size = def(temp, buffer_comp, Z_DEFAULT_COMPRESSION, temp_size);
            copy_str(temp, buffer);
            // Define compression unit: 1 byte --(compress)--> COM_UNIT bytes
            //if (COM_UNIT == 0)
            //  COM_UNIT = comp_size / temp_size;
          }

          // Write on stdout
          // Detect ^D
          if (buffer[i]=='\004')
            check_err(write(1, &"^D", sizeof(char)*2), "write");
          // Detect ^C
          else if (buffer[i]=='\003')
            check_err(write(1, &"^C", sizeof(char)*2), "write");
          // translate <cr> or <lf> into <cr><lf>
          else if (buffer[i]=='\n'||buffer[i]=='\r')
            check_err(write(1, &"\r\n", sizeof(char)*2), "write");
          else
            check_err(write(1, &buffer[i], sizeof(char)), "write");
        
          // Write on sockfd
          if(!received){
            check_err(write(sockfd, buffer_comp, comp_size), "write");
            if(logfd)
              keep_log(received, buffer_comp, comp_size);
          } 
        }//end for
        bzero(buffer, BUF_SIZE);
        bzero(buffer_comp, BUF_SIZE);
        bzero(temp, BUF_SIZE);
      }
      // Write/read end is closed
      else if (fds[received].revents & POLLHUP || fds[received].revents & POLLERR){
	      polling = 0;
      	break;
      }
    }//end for
  }//end while
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
  temp.c_iflag = ISTRIP;
  temp.c_oflag = 0;
  temp.c_lflag = 0;
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
  fprintf(stderr, "error: '%s' failed: %s \r\n", call, strerror(errno));
  if(logfd>0)
    close(logfd);
  if(sockfd>0)
    close(sockfd);
  reset_mode();
  exit(1);
}


// Processes command line arguments
// Exit with 1 if missing arg, invalid option, port not specified
// Returns 1 if '--compress' used
int process_opt(int argc, char* argv[]){
  int invalid=0, missing=0, comp=0, port=0, opt;

  struct option long_opts[]={
    {"port", required_argument, NULL, 'p'},
    {"log", required_argument, NULL, 'l'},
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
    case 'l':
      check_err(ulimit(UL_SETFSIZE,10000), "ulimit");
      logfd=open(optarg, O_WRONLY | O_TRUNC | O_CREAT, 0644);
      check_err(logfd, "open");
      break;
    case 'c':
      comp = 1;
      break;
    case ':':
      missing = 1;
      fprintf(stderr, "error: option '%s' requires an argument \r\n", argv[optind-1]);
      break;
    case '?':
    default:
      invalid = 1;
      fprintf(stderr, "error: option '%s' is not recognized \r\n", argv[optind-1]);
      break;
    } //end switch
  }//end while

  // Missing arg or invalid op 
  if (missing || invalid)
    exit(1);
  
  // Port not specified
  if (!port){
    fprintf(stderr, "error: port number not specified \r\n"); 
    exit(1);
  }
  return comp;
}


// Creates a socket
void create_socket(int p_num){
  struct sockaddr_in serv_addr;
  socklen_t serv_addrlen;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  check_err(sockfd, "socket");  

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(p_num);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  serv_addrlen = sizeof(serv_addr);
  check_err(connect(sockfd,(struct sockaddr*) &serv_addr, serv_addrlen), "connect");
}


// Copy string -- str1 must not contain '\0'
void copy_str(char* original, char* copy){
  while(*original != '\0'){
    *copy = *original;    
    copy++;
    original++;
  }
}


// Keeps log in logfd
void keep_log(int received, char* buffer, int read_size){
  if (received)
    check_err(write(logfd, &"RECEIVED ", 9), "write");
  else
    check_err(write(logfd, &"SENT ", 5), "write");
  
  char temp[BUF_SIZE];
  sprintf(temp, "%d", read_size);
  check_err(write(logfd, temp, strlen(temp)), "write");
  check_err(write(logfd, &" bytes: ", 8), "write");
  check_err(write(logfd, buffer, read_size), "write");
  check_err(write(logfd, "\n", 1), "write");
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
    int ret = inflate(&strm, Z_NO_FLUSH);
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
  if(logfd)
    close(logfd);
  if(sockfd)
    close(sockfd);
  reset_mode();
  exit(1);
}
