#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>
#include <mraa/aio.h>
//#include "mock.h"

#define B 4275
#define R0 100000
#define BUF_SIZE 256
#define AIO_PIN 1

int logfd = 0;
int sockfd = 0;
int com[] = {1, 1, 0, 1}; // period=1, F=1, start=0, shutdown=0
int port_num = -1;
char id[32];
char host[BUF_SIZE];


void error_msg(char* call){
    fprintf(stderr, "error: '%s' failed: %s\n", call, strerror(errno));
}

// 0: success
// 1: invalid command line arg
// 2: other failrures
int process_opt(int argc, char **argv){
    int invalid = 0;
    int requirement = 1;
    int opt;

    struct option long_opts[]={
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    // Process other parameters
    while (1){
        opt = getopt_long(argc, argv, ":", long_opts, NULL);
        if(opt == -1)
            break;
        switch(opt){
            case 'p':
                com[0] = atoi(optarg);
                break;
            case 's':
                if (*optarg == 'F')
                    com[1] = 1;
                else if (*optarg == 'C')
                    com[1] = 0;
                else {
                    invalid = 1;
                    fprintf(stderr, "error: invlaid argument: try 'scale=F' or 'scale=C'\n");
                }
                break;
            case 'l':
                logfd = open(optarg, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (logfd < 0){
                    error_msg("open");
                    return 1;
                }
                requirement++;
                break;
            case 'i':
                strcpy(id, optarg);
                if (strlen(id) != 9){
                    fprintf(stderr, "error: id must be a 9-digit-number\n");
                    return 1;
                } 
                requirement++;
                break;
            case 'h':
                strcpy(host, optarg);
                requirement++;
                break;
            case ':':
                invalid = 1;
                fprintf(stderr, "error: option '%s' requires an argument\n", argv[optind-1]);
                break;
            case '?':
            default:
                invalid = 1;
                fprintf(stderr, "error: option '%s' is not recognized\n", argv[optind-1]);
                break;
        }
    }

    // Get port number
    int i;
    int port_found = 0;
    for (i=0; i<argc; i++){
        if(i == 0 || argv[i][0] == '-')
            continue;
        
        int j;
        for (j=0; j<(int)strlen(argv[i]); j++){
            if (argv[i][j] > 47 && argv[i][j] < 58){
                port_found = 1;
                continue;
            }
            port_found = 0;
            break;
        }

        if(port_found)
            port_num = atoi(argv[i]);
    }

    // Missing arg or invalid opt
    if (invalid)
        return 1;

    // Missing port number
    if(port_num < 0){
        fprintf(stderr, "error: missing port number\n");
        return 1;
    }

    // Missing required parameters
    if (requirement < 4){
        fprintf(stderr, "error: missing required parameters\n");
        return 1;
    }
    return 0;
}

void write_log(float temperature){
    time_t local_time;
    struct tm *p_tm;
    char write_format[BUF_SIZE];
    int retval;

    if (time(&local_time) < 0){
        error_msg("time");
        exit(2);
    }

    p_tm = localtime(&local_time);

    if (com[3]){
        sprintf(write_format, "%02d:%02d:%02d %3.1f\n", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, temperature);
        retval = write(sockfd, write_format, strlen(write_format));
        retval = 1;
        if (retval < 0){
            error_msg("write");
            exit(2);
        }
        retval = write(logfd, write_format, strlen(write_format));
        if (retval < 0){
            error_msg("write");
            exit(2);
        }
        memset(write_format, 0, BUF_SIZE*sizeof(char));
        return;
    }
    
    // Shutdown
    sprintf(write_format, "%02d:%02d:%02d SHUTDOWN\n", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
    retval = write(sockfd, write_format, strlen(write_format));
    if (retval < 0){
        error_msg("write");
        exit(2);
    }
    retval = write(logfd, write_format, strlen(write_format));
    if (retval < 0){
        error_msg("write");
        exit(2);
    }
    memset(write_format, 0, BUF_SIZE*sizeof(char));
}

void shutdown_handler(){
    com[3] = 0;
}

float get_temperature(mraa_aio_context aio){
    int aio_value = 0;
    float temperature = 0;
                           
    aio_value = mraa_aio_read(aio);
    if (aio_value < 0) {
        fprintf(stderr, "error: 'mraa_aio_read_float' failed\n");
        exit(1);
    }

    // Convert aio value to temperature                                                           
    float R = 1023.0/aio_value - 1.0;
    R = R0 * R;
    temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;

    // SCALE=C
    if (com[1] == 0)
        return temperature;
    else
        return temperature*9/5 + 32;
}

int get_command(char *command){
    if (strcmp("SCALE=F",command) == 0) return 1;
    if (strcmp("SCALE=C",command) == 0) return 2;
    if (strcmp("STOP",command) == 0) return 4;
    if (strcmp("START",command) == 0) return 5;
    if (strcmp("OFF",command) == 0) return 7;

    // Check if PERIOD=second
    char *temp = command;
    const char *PERIOD = "PERIOD=";
    int i;
    for (i=0; i<(int)strlen(PERIOD); i++) {
        if (PERIOD[i] != command[i]) break;
        // PERIOD= matched. Check if second starts with int
        if (PERIOD[i] == '='){
            // second does not starts with int
            temp++;
            if (atoi(temp) == 0) return 0;
            // If second starts with int, consider it valid
            return 3;
        }
        temp++;
    }

    // Check if LOG
    const char *LOG = "LOG";
    for (i=0; i<(int)strlen(LOG); i++) {
        if (LOG[i] != command[i]) break;
        if (LOG[i] == 'G') return 6;
    }
    return 0;
}

int create_socket(int port_num){
    struct hostent *server;
    struct sockaddr_in serv_addr;
    socklen_t serv_addrlen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error_msg("socket");  
        return 2;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"error: no such host exists\n");
        return 1;
    }

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    serv_addrlen = sizeof(serv_addr);
    int retval = connect(sockfd,(struct sockaddr*) &serv_addr, serv_addrlen);
    if (retval < 0){
        error_msg("connect");
        return 1;
    }
    return 0;
}


void *server_thread(void *com_){
    int read_size;
    char buffer[BUF_SIZE];
    char write_format[BUF_SIZE];
    char temp[BUF_SIZE];
    char leftover[BUF_SIZE];
    struct pollfd poll_fd;
    int retval;
    int *com = (int*)com_;

    // Open server
    retval = create_socket(port_num);
    if (retval > 0)
        exit(retval);

    // Write ID
    sprintf(write_format, "ID=%s\n",id);
    retval = write(sockfd, write_format, strlen(write_format));
    if (retval < 0){
        error_msg("write");
        exit(2);
    }
    retval = write(logfd, write_format, strlen(write_format));
    if (retval < 0){
        error_msg("write");
        exit(2);
    }
    memset(write_format, 0, BUF_SIZE*sizeof(char));
    com[2] = 1;

    // Close stdin and stdout
    retval = close(0);
    if (retval < 0){
        error_msg("close");
        exit(2);
    }
    retval = close(1);
    if (retval < 0){
        error_msg("close");
        exit(2);
    }

    poll_fd.fd = sockfd;   
    poll_fd.events = POLLIN | POLLHUP | POLLERR;;

    while(com[3]){
        if (poll(&poll_fd, 1, 0) < 0){
            error_msg("poll");
            exit(2);
        }
        // Read command
        if (poll_fd.revents & POLLIN){
            read_size = read(poll_fd.fd, buffer, BUF_SIZE);
	        if (read_size < 0){
                error_msg("read");
                exit(2);
            }

            int i = 0;
            int j = 0;
            int left = 0;
            while (i < read_size && com[3]){  

                // '\n' found
                if (buffer[i] == '\n'){
                    if (strlen(leftover)) {
                        strcat(leftover, temp);
                        strcpy(temp, leftover);
                        memset(leftover, 0, BUF_SIZE*sizeof(char)); 
                    }
                    int command = get_command(temp);
                    switch(command){
                        case 1: // SCALE=P
                            com[1] = 1;
                            break;
                        case 2: // SCALE=C
                            com[1] = 0;
                            break;
                        case 3: // PERIOD=second
                            com[0] = atoi(temp+7);
                            break;
                        case 4: // STOP
                            com[2] = 0;
                            break;
                        case 5: // START
                            com[2] = 1;
                            break;
                        case 6: // LOG
                            break;
                        case 7: // OFF
                            com[3] = 0;                            
                            break;
                        default:
                            break;
                    }
                    
                    // Log command in logfd
                    sprintf(write_format, "%s\n", temp);
                    retval = write(logfd, write_format, strlen(write_format));
                    if (retval < 0){
                        error_msg("write");
                        exit(2);
                    }
                    memset(write_format, 0, BUF_SIZE*sizeof(char));

                    left = 0;
                    j = -1;
                    memset(temp, 0, BUF_SIZE*sizeof(char));
                }
                else {
                    temp[j] = buffer[i];
                    left = 1;
                }
                i++;
                j++;
            }
            
            if (left)
                strcpy(leftover, temp);
            memset(temp, 0, BUF_SIZE*sizeof(char)); 
        }
        else if (poll_fd.revents & POLLHUP || poll_fd.revents & POLLERR){
            fprintf(stderr, "error: server closed\n");
            exit(2);
        }
    }
    pthread_exit(NULL);
}

void *sensor_thread(void *com_){
    mraa_aio_context aio;
    float temperature;
    const int *com = (int*)com_;
    clock_t clk_s, clk_e, diff;

    // Initialize aio                                                                             
    aio = mraa_aio_init(AIO_PIN);
    if (aio == NULL) {
        fprintf(stderr, "error: 'mraa_aio_init' failed\n");
        exit(1);
    }

    while (com[3]){
        clk_s = clock();
        temperature = get_temperature(aio);
        if (com[2])
            write_log(temperature);
        while(diff < (int) com[0]){
            clk_e = clock();
            diff = (double)(clk_e - clk_s) / CLOCKS_PER_SEC;
        }
        diff = 0;
    }
    mraa_aio_close(aio);
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    int retval;
    retval = process_opt(argc, argv);
    if (retval > 0)
        exit(retval);

    pthread_t sensor_t, server_t;

    // Create threads to process sensor/stdin input
    if (pthread_create(&sensor_t, NULL, sensor_thread, (void *) com) != 0){
        error_msg("pthread_create");
        exit(2);
    }
    if (pthread_create(&server_t, NULL, server_thread, (void *) com) != 0){
        error_msg("pthread_create");
        exit(2);
    }
    // Join the threads
    if (pthread_join(sensor_t,0) != 0){
        error_msg("pthread_join");
        exit(2);
    }
    if (pthread_join(server_t,0) != 0){
        error_msg("pthread_join");
        exit(2);
    }
    
    write_log(0);
    retval = close(sockfd);
    if (retval < 0){
        error_msg("close");
        exit(2);
    }
    exit(0);
}
