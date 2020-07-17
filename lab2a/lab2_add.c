#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

#define SYNC_M 0b0100
#define SYNC_S 0b0010
#define SYNC_C 0b0001
#define YIELD 0b1000

void add();
void process_opt();
void *thread_routine();
void copy_str();
void error();
int opt_num();

pthread_mutex_t lock_m;
volatile int lock_s = 0;
int opt_bits=0b0000;

struct t_arg {
    long long *counter;
    int num_iter;
}args;

int main(int argc, char *argv[]){
    int num_t=1, num_iter=1;
    int ret;
    long long counter=0;
    struct timespec start, end;
    pthread_t tid[num_t];

    process_opt(argc, argv, &num_t, &num_iter);
    struct t_arg argt={&counter, num_iter};

    // --sync=m
    if (opt_bits&SYNC_M){
        ret=pthread_mutex_init(&lock_m, NULL);
        if (ret!=0){ 
            error("pthread_mutex_init");
        }
    }

    // Note the start time
    ret = clock_gettime(CLOCK_MONOTONIC, &start);
    if (ret<0)
        error("clock_gettime");
    
    // Create num_t many threads and call thread_routine
    int i;
    for (i=0; i<num_t; i++){
        ret=pthread_create(&tid[i], NULL, thread_routine, (void *) &argt);
        if (ret!=0)
            error("pthread_create");
    }
    for (i=0; i<num_t; i++){
        ret=pthread_join(tid[i],0);
        if (ret!=0)
            error("pthread_join");
    }

    // Note the end time
    ret = clock_gettime(CLOCK_MONOTONIC, &end);
    if (ret<0)
        error("clock_gettime");

    // --sync=m
    if (opt_bits&SYNC_M){
        ret=pthread_mutex_destroy(&lock_m);
        if (ret<0)
            error("clock_gettime");
    }

    // Print CSV record
    long long num_op=num_t*num_iter*2;
    long total_time_sec=end.tv_sec-start.tv_sec;
    long total_time_nsec=end.tv_nsec-start.tv_nsec;
    long long total_time = total_time_sec*1000000000 + total_time_nsec;
    char *tag[8]={"add-none","add-m","add-s","add-c","add-yield-none","add-yield-m","add-yield-s","add-yield-c"};
    printf("%s,%d,%d,%lld,%lld,%lld,%lld\n",
            tag[opt_num(opt_bits)], num_t, num_iter, num_op, total_time, total_time/num_op, counter);
    exit(0);
}

void add(long long *pointer, long long value) {
    int ret;
    long long oldval=0, sum=0;

    // --sync=m
    if (opt_bits&SYNC_M){
        ret=pthread_mutex_lock(&lock_m);
        if (ret!=0)
            error("pthread_mutex_lock");
    }

    // --sync=s
    if (opt_bits&SYNC_S)
        while(__sync_lock_test_and_set(&lock_s, 1));

    // --sync=c
    if (opt_bits&SYNC_C)
        oldval = *pointer;

    sum = opt_bits&SYNC_C ? oldval + value : *pointer + value;

    if (opt_bits&YIELD)
        ret = sched_yield();  

    if (opt_bits&SYNC_C){
        while (__sync_val_compare_and_swap (pointer, oldval, sum)!=oldval){
            oldval = *pointer;
            sum = oldval + value;
            if (opt_bits&YIELD)
                ret = sched_yield();
        }
    }
    else 
        *pointer = sum;

    // --sync=m
    if (opt_bits&SYNC_M){
        ret=pthread_mutex_unlock(&lock_m);
        if (ret!=0)
            error("pthread_mutex_unlock");
    }
    // --sync=s
    if (opt_bits&SYNC_S)
        __sync_lock_release(&lock_s);
}

void process_opt(int argc, char *argv[], int *num_t, int *num_iter){
    int invalid=0, missing=0, opt;

    struct option long_opts[]={
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
  
    while (1){
        opt = getopt_long(argc, argv, ":", long_opts, NULL);
        if(opt == -1)
            break;
        switch(opt){
            case 't':
                *num_t = atoi(optarg);
                break;
            case 'i':
                *num_iter = atoi(optarg);
                break;
            case 'y':
                opt_bits=opt_bits|0b1000;
                break;
            case 's':
                switch(*optarg){
                    case 'm':
                        opt_bits=opt_bits|0b0100;
                        break;
                    case 's':
                        opt_bits=opt_bits|0b0010;
                        break;
                    case 'c':
                        opt_bits=opt_bits|0b0001;
                        break;
                    default:
                        invalid = 1;
                        fprintf(stderr, "error: usage: --sync=[msc]\n");
                        break;
                }
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

    // Missing arg or invalid opt
    if (missing || invalid){
        exit(1);
    }
}

void error(char* call){
    fprintf(stderr, "call to '%s' failed: %s \r\n", call, strerror(errno));
    exit(1);
}

void *thread_routine(void *argt){
    struct t_arg *temp = (struct t_arg *) argt;
    int i; 
    for (i=0; i<temp->num_iter; i++)
        add(temp->counter, 1);
    for (i=0; i<temp->num_iter; i++)
        add(temp->counter, -1);
    pthread_exit(NULL);
}

int opt_num(){
    int num;
    switch(opt_bits){
        case 0b0000:
            num=0;
            break;
        case 0b0100:
            num=1;
            break;
        case 0b0010:
            num=2;
            break;
        case 0b0001:
            num=3;
            break;
        case 0b1000:
            num=4;
            break;
        case 0b1100:
            num=5;
            break;
        case 0b1010:
            num=6;
            break;
        case 0b1001:
            num=7;
            break;
    }
    return num;
}
