#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <gperftools/profiler.h>

#define KEY_SIZE 4

void process_opt();
void *thread_routine();
void error();
void create_elements();
void create_heads();
void locks();
void sig_handler();
void random_char();

int opt_yield=0b000;
int opt_sync=0b00;
int num_t=1;
int num_iter=1;
int num_list=1;
long long total_lock_time=0;

pthread_mutex_t *lock_m;
volatile int *lock_s=0;

SortedList_t *heads;
SortedListElement_t *elements;

int main(int argc, char *argv[]){
    struct timespec start, end;
    int ret;

    signal(SIGSEGV,sig_handler);
    process_opt(argc, argv);

    create_elements();
    create_heads();
    locks(1);

    // Note the start time
    ret = clock_gettime(CLOCK_MONOTONIC, &start);
    if (ret<0)
        error("clock_gettime");
    
    // Create num_t many threads and call thread_routine
    pthread_t tid[num_t];
    int idx[num_t];
    int i;
    for (i=0; i<num_t; i++){
        idx[i] = num_iter*i;
        ret = pthread_create(&tid[i], NULL, thread_routine, (void *) &idx[i]);
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

    // Get length of the list
    int total_len=0;
    for (i=0; i<num_list; i++){
        ret = SortedList_length(&heads[i]);
        if (ret ==-1){
            fprintf(stderr, "SortedList_length failed: list currupted\n");
            exit(2);
        }
        total_len += ret;
    }
    if (total_len!=0)
        fprintf(stderr,"list length is not zero\n");
    
    locks(0);

    // Print CSV record
    char *y_tag[8]={"none","i","d","id","l","il","dl","idl"};
    char *s_tag[3]={"none","s","m"};
    long num_op=num_t*num_iter*3;
    long long total_time = (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);
    printf("list-%s-%s,%d,%d,%d,%ld,%lld,%lld,%lld\n",
            y_tag[opt_yield], s_tag[opt_sync], num_t, num_iter, num_list, num_op, total_time, total_time/num_op, total_lock_time/num_op);

    free(elements);
    free(heads);
    exit(0);
}

void process_opt(int argc, char *argv[]){
    int invalid=0, missing=0, opt;

    struct option long_opts[]={
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
  
    while (1){
        opt = getopt_long(argc, argv, ":", long_opts, NULL);
        if(opt == -1)
            break;
        switch(opt){
            case 't':
                num_t = atoi(optarg);
                break;
            case 'i':
                num_iter = atoi(optarg);
                break;
            case 'y':;
                unsigned int i=0;
                for (i=0; i<strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'i':
                            opt_yield=opt_yield|0b001;
                            break;
                        case 'd':
                            opt_yield=opt_yield|0b010;
                            break;
                        case 'l':
                            opt_yield=opt_yield|0b100;
                            break;
                        default:
                            invalid = 1;
                            fprintf(stderr, "error: usage: --yield=[idl]\n");
                            break;
                    }
                }
                break;
            case 's':
                switch(*optarg){
                    case 'm':
                        opt_sync=opt_sync|0b10;
                        break;
                    case 's':
                        opt_sync=opt_sync|0b01;
                        break;
                    default:
                        invalid = 1;
                        fprintf(stderr, "error: usage: --sync=[ms]\n");
                        break;
                }
                break;
            case 'l':
                num_list = atoi(optarg);
                break;
            case ':':
                missing = 1;
                fprintf(stderr, "error: option '%s' requires an argument\n", argv[optind-1]);
                break;
            case '?':
            default:
                invalid = 1;
                fprintf(stderr, "error: option '%s' is not recognized\n", argv[optind-1]);
                break;
        } //end switch
    }//end while

    // Missing arg or invalid opt
    if (missing || invalid){
        exit(1);
    }
}

void error(char* call){
    fprintf(stderr, "call to '%s' failed: %s\n", call, strerror(errno));
    exit(1);
}

void *thread_routine(int *idx){
    struct timespec start, end;
    int ret;
    int head_idx;
    int i;

    //printf("%d: from %d to %d\n",(int)pthread_self(), *idx, *idx+num_iter);

    // NO SYNC
    if (opt_sync==0) {

        // INSERT
        for (i=0; i<num_iter; i++){
            //printf("inserting %s into list%d\n",elements[*idx+i].key,head_idx);
            head_idx = (int) elements[*idx+i].key[0] % num_list;
            SortedList_insert(&heads[head_idx], &elements[*idx+i]);
        }

        // LENGTH
        int total_len=0;
        for (i=0; i<num_list; i++){
            ret = SortedList_length(&heads[i]);
            if (ret==-1){
                fprintf(stderr, "SortedList_length failed: list currupted\n");
                exit(2);
            }
            total_len += ret;
            //printf("list%d length = %d\n", i, ret);
        }
    
        // LOOK UP, DELETE
        for (i=0; i<num_iter; i++){
            head_idx = (int) elements[*idx+i].key[0] % num_list;
            //printf("look list%d for %s\n", head_idx, elements[*idx+i].key);
            SortedListElement_t *del_temp = SortedList_lookup(&heads[head_idx], (const char*) elements[*idx+i].key);
            if (del_temp==NULL){
                fprintf(stderr, "SortedList_lookup failed: previously inserted key not in the list\n");
                exit(2);
            }
            ret = SortedList_delete(del_temp);
            if (ret){
                fprintf(stderr, "SortedList_delete failed: list currupted\n");
                exit(2);
            }
        }
    }
    // SPIN-LOCK
    else if (opt_sync==1){
        long long lock_time=0;

        // INSERT
        for (i=0; i<num_iter; i++){
            head_idx = (int) elements[*idx+i].key[0] % num_list;

            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            while(__sync_lock_test_and_set(&lock_s[head_idx], 1));
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Insert
            SortedList_insert(&heads[head_idx], &elements[*idx+i]);

            // Unlock
            __sync_lock_release(&lock_s[head_idx]);
        }

        // LENGTH
        int total_len=0;
        for (i=0; i<num_list; i++){
            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            while(__sync_lock_test_and_set(&lock_s[i], 1));
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Get length
            ret = SortedList_length(&heads[i]);
            if (ret==-1){
                fprintf(stderr, "SortedList_length failed: list currupted\n");
                exit(2);
            }
            total_len += ret;

            // Unlock
            __sync_lock_release(&lock_s[i]);
        }

        // LOOK UP, DELETE
        for (i=0; i<num_iter; i++){
            head_idx = (int) elements[*idx+i].key[0] % num_list;

            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            while(__sync_lock_test_and_set(&lock_s[head_idx], 1));
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Look up
            SortedListElement_t *del_temp = SortedList_lookup(&heads[head_idx], (const char*) elements[*idx+i].key);
            if (del_temp==NULL){
                fprintf(stderr, "SortedList_lookup failed: previously inserted key not in the list\n");
                exit(2);
            }

            // Delete
            ret = SortedList_delete(del_temp);
            if (ret){
                fprintf(stderr, "SortedList_delete failed: list currupted\n");
                exit(2);
            }

            // Unlock
            __sync_lock_release(&lock_s[head_idx]);
        }
        total_lock_time += lock_time;
    }
    // MUTEX
    else if (opt_sync==2){
        long long lock_time=0;

        // INSERT
        for (i=0; i<num_iter; i++){
            head_idx = (int) elements[*idx+i].key[0] % num_list;

            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            ret = pthread_mutex_lock(&lock_m[head_idx]);
            if (ret!=0)
                error("pthread_mutex_lock");
            //printf("insert: list%d is locked\n",head_idx);
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Insert
            //printf("inserting %s into list%d ... ",elements[*idx+i].key,head_idx);
            SortedList_insert(&heads[head_idx], &elements[*idx+i]);
            //printf("done\n");

            // Unlock
            ret = pthread_mutex_unlock(&lock_m[head_idx]);
            if (ret!=0)
                error("pthread_mutex_unlock");
            //printf("insert: list%d is unlocked\n",head_idx);
        }

        // LENGTH
        int total_len=0;
        for (i=0; i<num_list; i++){
            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            ret = pthread_mutex_lock(&lock_m[i]);
            if (ret!=0)
                error("pthread_mutex_lock");
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Get length
            ret = SortedList_length(&heads[i]);
            if (ret==-1){
                fprintf(stderr, "SortedList_length failed: list currupted\n");
                exit(2);
            }
            total_len += ret;
            //printf("list%d length = %d\n",i, ret);

            // Unlock
            ret = pthread_mutex_unlock(&lock_m[i]);
            if (ret!=0)
                error("pthread_mutex_unlock");

        }

        // LOOK UP, DELETE
        for (i=0; i<num_iter; i++){
            head_idx = (int) elements[*idx+i].key[0] % num_list;

            // Lock
            ret = clock_gettime(CLOCK_MONOTONIC, &start);
            if (ret<0)
                error("clock_gettime");
            ret = pthread_mutex_lock(&lock_m[head_idx]);
            if (ret!=0)
                error("pthread_mutex_lock");
            ret = clock_gettime(CLOCK_MONOTONIC, &end);
            if (ret<0)
                error("clock_gettime");
            lock_time += (end.tv_sec-start.tv_sec)*1000000000 + (end.tv_nsec-start.tv_nsec);

            // Look up
            SortedListElement_t *del_temp = SortedList_lookup(&heads[head_idx], (const char*) elements[*idx+i].key);
            if (del_temp==NULL){
                fprintf(stderr, "SortedList_lookup failed: previously inserted key not in the list\n");
                exit(2);
            }

            // Delete
            ret = SortedList_delete(del_temp);
            if (ret){
                fprintf(stderr, "SortedList_delete failed: list currupted\n");
                exit(2);
            }

            // Unlock
            ret = pthread_mutex_unlock(&lock_m[head_idx]);
            if (ret!=0)
                error("pthread_mutex_unlock");
        }
        total_lock_time += lock_time;
    }

    pthread_exit(NULL);
}


void create_elements(){
    int size = num_t*num_iter;
    elements = malloc(size*sizeof(SortedListElement_t));
    if (elements==NULL)
        error("malloc");

    int i;
    for (i=0; i<size; i++){
        elements[i].key = (char*)malloc(KEY_SIZE*sizeof(char));
        if (elements[i].key==NULL)
            error("malloc");
        random_char((char*)elements[i].key);
        //printf("%s\n",elements[i].key);
    }
}

void create_heads(){
    heads = malloc(num_list*sizeof(SortedList_t));
    if (elements==NULL)
        error("malloc");
}

void locks(int create){
    int i;
    int ret;
    if (create){
        // MUTEX
        if (opt_sync==2){
            lock_m = malloc(num_list*sizeof(pthread_mutex_t));
            for (i=0; i<num_list; i++){
                ret = pthread_mutex_init(&lock_m[i], NULL);
                if (ret!=0)
                    error("pthread_mutex_init");
            }
        }
        // SPIN-LOCK
        else if (opt_sync==1){
             lock_s = malloc(num_list*sizeof(int));
            for (i=0; i<num_list; i++)
                lock_s[i] = 0;      
        }
    }
    else{
        // MUTEX
        if (opt_sync==2){
            for (i=0; i<num_list; i++){
                ret = pthread_mutex_destroy(&lock_m[i]);
                if (ret<0)
                    error("pthread_mutex_destroy");
            }
            free(lock_m);
        }
        // SPIN-LOCK
        else if (opt_sync==1)
            free((void*)lock_s);
    }
}

void sig_handler(){
    fprintf(stderr, "catched segmentation fault\n");
    exit(2);
}

void random_char(char* ptr){
   int i;
   char temp;
   for (i=0; i<KEY_SIZE-1; i++){
      temp = (char)(rand()%93+33);
      *ptr=temp;
      ptr++;
   }
   ptr=NULL;
}
