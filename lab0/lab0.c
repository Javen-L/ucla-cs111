#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void seg_fault(){
  int* p = NULL;
  *p = 1;
}

void sig_handler(){
  fprintf(stderr, "catched segmentation fault: exiting the program... \n");
  exit(4);
}

void usage(){
  char* msg =
    "usage: ./lab0 [OPTIONS...] \n"
    "--input=filename    use the specified file as standard input \n"
    "--output=filename   use the specified file as standard output \n"
    "--segfault          force a segmentation fault \n"
    "--catch             use signal(2) to register a SIGSEGV handler \n";
  fprintf(stderr, "%s", msg);
}

int main(int argc, char *argv[]){

  const int BUF_SIZE = 256;
  int input_fd, output_fd;
  int input_byte;
  char buffer[BUF_SIZE];
  int opt, input_op = 0, output_op = 0, seg_op = 0, catch_op = 0, missing_arg = 0, inval_opt = 0;
  char *input_file, *output_file; 
  int argv_inval[argc];
  int curr_ind = 0;

  // Process all arguments
  struct option long_options[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"segfault", no_argument, NULL, 's'},
    {"catch", no_argument, NULL, 'c'},
    {0, 0, 0, 0}
  };

  while (1){
    curr_ind = optind;
    opt = getopt_long(argc, argv, ":", long_options, NULL);
    if(opt == -1)
      break;
    switch(opt){
    case 'i':
      input_op = 1;
      input_file = optarg;
      break;
    case 'o':
      output_op = 1;
      output_file = optarg;
      break;
    case 's':
      seg_op = 1;
      break;
    case 'c':
      catch_op = 1;
      break;
    case ':':
      // argv_inval[.]=1 -> missing argument
      argv_inval[curr_ind] = 1;
      missing_arg = 1;
      break;
    case '?':
    default:
      // argv_inval[.]=2 -> invalid argument
      argv_inval[curr_ind] = 2;
      inval_opt = 1;      
      break;
    } //end switch
  }//end while

  // ACTION 0: Handle missing arguments/invalid options
  // Missing argument
  if (missing_arg == 1){
    int i;
    for (i=1; i<argc; i++){
      if(argv_inval[i]==1)
	fprintf(stderr, "missing an argument %s: option '%s' requires an argument \n", argv[0], argv[i]);    
    }
    usage();
    exit(1);
  }
  // Invalid option(s)
  if (inval_opt == 1){
    int i;
    for (i=1; i<argc; i++){
      if (argv_inval[i]==2)
	fprintf(stderr, "invalid option %s: option '%s' is not recognized \n", argv[0], argv[i]);
    }
    usage();
    exit(1);
  }

  // ACTION 1: File Redirections 
  // Set input file as stdin (file discriptor = 0)
  if (input_op == 1){
    input_fd = open(input_file, O_RDONLY);
    dup2(input_fd, 0);                                      
    if (input_fd==-1){
      fprintf(stderr, "'--input' error: cannot open file '%s': %s \n", input_file, strerror(errno));
      exit(2);
    }
  }
  // Set output file as stdout (i.e. file discriptor = 1)
  if (output_op==1){
    output_fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    dup2(output_fd, 1);
    if (output_fd == -1){
      fprintf(stderr, "--output error: cannot open/create '%s': %s \n", output_file, strerror(errno));
      exit(3);
    }
  }

  // ACTION 2: Register a signal handler
  if (catch_op == 1)
    signal(SIGSEGV,sig_handler);

  // ACTION 3: Cause a segmentation fault
  if (seg_op == 1)
    seg_fault();

  // ACTION 4: Copy stdin to stdout
  while (1){
    input_byte = read(0, &buffer, BUF_SIZE);
    // End of input file
    if (input_byte == 0)
      break;
    write(1, &buffer, input_byte);    
  } //end while

  close(input_fd);
  close(output_fd);
  return 0;
}

