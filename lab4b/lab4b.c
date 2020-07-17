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
#include <mraa/aio.h>
#include <mraa/gpio.h>

#define B 4275
#define R0 100000
#define BUF_SIZE 256
#define AIO_PIN 1
#define BTN_PIN 60

int log_fd = 0;
int com[] = {1, 1, 1, 1}; // period=1, F=1, start=1, shutdown=0

void error(char* call){
    fprintf(stderr, "error: '%s' failed: %s\n", call, strerror(errno));
    exit(1);
}

void process_opt(int argc, char **argv){
    int invalid = 0;
    int opt;

    struct option long_opts[]={
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
  
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
                log_fd = open(optarg, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (log_fd < 0) error("open");
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

    // Missing arg or invalid opt
    if (invalid) exit(1);
}

void write_log(float temperature){
    time_t local_time;
    struct tm *p_tm;
    int fd = log_fd ? log_fd : 1;

    if(time(&local_time) < 0) error("time");
    p_tm = localtime(&local_time);

    if (com[3]){
        dprintf(fd, "%02d:%02d:%02d %3.1f\n", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, temperature);
        return;
    }
    
    // Shutdown
    dprintf(fd, "%02d:%02d:%02d SHUTDOWN\n", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
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

void *stdin_thread(void *com_){
    int read_size;
    char buffer[BUF_SIZE];
    char temp[BUF_SIZE];
    char leftover[BUF_SIZE];
    struct pollfd poll_fd;
    int *com = (int*)com_;

    poll_fd.fd = 0;   
    poll_fd.events = POLLIN | POLLHUP | POLLERR;;

    while(com[3]){
        if (poll(&poll_fd, 1, 0) < 0) error("poll");
        
        // Read command
        if (poll_fd.revents & POLLIN){
            read_size = read(0, buffer, BUF_SIZE);
	        if (read_size < 0) error("read");

            //printf("buffer: %s\n",buffer);
            int i = 0;
            int j = 0;
            int left = 0;
            while (i < read_size && com[3]){  
                //printf("buffer[%d]: %c\n",i,buffer[i]);
                // '\n' found
                if (buffer[i] == '\n'){
                    if (strlen(leftover)) {
                        strcat(leftover, temp);
                        strcpy(temp, leftover);
                        memset(leftover, 0, BUF_SIZE*sizeof(char)); 
                    }
                    //printf("command found: %s\n",temp);
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

                    if (log_fd) dprintf(log_fd, "%s\n", temp);
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
            
            if (left) strcpy(leftover, temp);
            memset(temp, 0, BUF_SIZE*sizeof(char)); 
        }
        //else if (poll_fd.revents & POLLHUP || poll_fd.revents & POLLERR)
        //    com[3] = 0;
    }
    pthread_exit(NULL);
}

void *sensor_thread(void *com_){
    mraa_aio_context aio;
    mraa_gpio_context btn;
    float temperature;
    const int *com = (int*)com_;
    clock_t clk_s, clk_e, diff;

    // Initialize aio                                                                             
    aio = mraa_aio_init(AIO_PIN);
    if (aio == NULL) {
        fprintf(stderr, "error: 'mraa_aio_init' failed\n");
        exit(1);
    }

    // Initialize gpio                                                                            
    btn = mraa_gpio_init(BTN_PIN);
    if (btn == NULL) {
        fprintf(stderr, "error: 'mraa_gpio_init' failed\n");
        exit(1);
    }

    mraa_gpio_dir(btn, MRAA_GPIO_IN);
    mraa_gpio_isr(btn, MRAA_GPIO_EDGE_RISING, &shutdown_handler, NULL);

    while (com[3]){
        clk_s = clock();
        temperature = get_temperature(aio);
        if (com[2]) write_log(temperature);
        while(diff < com[0]){
            clk_e = clock();
            diff = (double)(clk_e - clk_s) / CLOCKS_PER_SEC;
        }
        diff = 0;
    }
    mraa_aio_close(aio);
    mraa_gpio_close(btn);
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    process_opt(argc, argv);
    pthread_t sensor_t, stdin_t;

    // Create threads to process sensor/stdin input
    if (pthread_create(&sensor_t, NULL, sensor_thread, (void *) com) != 0) error("pthread_create");
    if (pthread_create(&stdin_t, NULL, stdin_thread, (void *) com) != 0) error("pthread_create");

    // Join the threads
    if (pthread_join(sensor_t,0) != 0) error("pthread_join");
    if (pthread_join(stdin_t,0) != 0) error("pthread_join");

    write_log(0);
    exit(0);
}
