#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#define KEY_SIZE 3

void process_opt();
void *thread_routine();
void error();
void create_elements();
void free_elements();
void sig_handler();
void random_char();
void lock();
void unlock();

int opt_yield=0b000;
int opt_sync=0b00;
pthread_mutex_t lock_m;
volatile int lock_s = 0;
int num_t=1, num_iter=1;

struct t_arg {
    int elements_idx;
    SortedList_t *head;
    SortedListElement_t **elements;
}args;

int main(int argc, char *argv[]){
    int ret;
    struct timespec start, end;
    signal(SIGSEGV,sig_handler);

    process_opt(argc, argv, &num_t, &num_iter);
       
    SortedList_t head={NULL,NULL,NULL};
    SortedListElement_t *elements[num_t*num_iter];

    create_elements(elements, num_iter*num_t);

    // --sync=m
    if (opt_sync==2){
        ret=pthread_mutex_init(&lock_m, NULL);
        if (ret!=0)
            error("pthread_mutex_init");
    }

    // Note the start time
    ret = clock_gettime(CLOCK_MONOTONIC, &start);
    if (ret<0)
        error("clock_gettime");
    // Create num_t many threads and call thread_routine
    pthread_t tid[num_t];
    struct t_arg argt[num_t];
    int i;
    for (i=0; i<num_t; i++){
        argt[i].elements_idx = num_iter*i;
        argt[i].head = &head;
        argt[i].elements = elements;
        ret=pthread_create(&tid[i], NULL, thread_routine, (void *) &argt[i]);
        if (ret!=0)
            error("pthread_create");
 
    }
    //printf("pass creating threads\n");
    for (i=0; i<num_t; i++){
        ret=pthread_join(tid[i],0);
        if (ret!=0)
            error("pthread_join");
    }
    //printf("pass joining threads\n");

    // Note the end time
    ret = clock_gettime(CLOCK_MONOTONIC, &end);
    if (ret<0)
        error("clock_gettime");

    ret = SortedList_length(&head);
    if (ret!=0)
        fprintf(stderr,"list length is not zero\n");
    else if (ret<0)
        fprintf(stderr,"list currupted\n");

    // --sync=m
    if (opt_sync==2){
        ret=pthread_mutex_destroy(&lock_m);
        if (ret<0)
            error("clock_gettime");
    }

    // Print CSV record
    char *y_tag[8]={"none","i","d","id","l","il","dl","idl"};
    char *s_tag[3]={"none","s","m"};
    long num_op=num_t*num_iter*3;
    long total_time_sec=end.tv_sec-start.tv_sec;
    long total_time_nsec=end.tv_nsec-start.tv_nsec;
    long long total_time = total_time_sec*1000000000 + total_time_nsec;
    printf("list-%s-%s,%d,%d,1,%ld,%lld,%lld\n",
            y_tag[opt_yield], s_tag[opt_sync], num_t, num_iter, num_op, total_time, total_time/num_op);

    free_elements(elements, num_iter*num_t);
    exit(0);
}

void process_opt(int argc, char *argv[], int *num_t, int *num_iter){
    int invalid=0, missing=0, opt;
    unsigned int i;

    struct option long_opts[]={
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
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
    int ret;
    struct t_arg *temp = (struct t_arg *) argt;

    if (opt_sync==2){
        ret=pthread_mutex_lock(&lock_m);
        if (ret!=0)
            error("pthread_mutex_lock");
    }
    // --sync=s
    if (opt_sync==1)
        while(__sync_lock_test_and_set(&lock_s, 1));

    // Insert elements into the list
    int i;
    for (i=0; i<num_iter; i++){
        SortedList_insert(temp->head, temp->elements[temp->elements_idx+i]);
    }

    // Get length of the list
    int len = SortedList_length(temp->head);
    if (len==-1){
        fprintf(stderr, "SortedList_length failed: list currupted\n");
        exit(2);
    }

    // Look up a key and deletes it
    for (i=0; i<len; i++){

        SortedListElement_t *del_temp = SortedList_lookup(temp->head, (const char*) temp->elements[temp->elements_idx+i]->key);
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

    // --sync=m
    if (opt_sync==2){
        ret=pthread_mutex_unlock(&lock_m);
        if (ret!=0)
            error("pthread_mutex_unlock");
    }
    // --sync=s
    if (opt_sync==1)
        __sync_lock_release(&lock_s);

    pthread_exit(NULL);
}

void create_elements(SortedListElement_t *elements[], int size){
    int i;
    for (i=0; i<size; i++){
        elements[i] = (SortedListElement_t*)malloc(sizeof(SortedListElement_t*));
        if (elements==NULL)
            error("malloc");
        elements[i]->key = (char*)malloc(KEY_SIZE*sizeof(char));
        if (elements[i]->key==NULL)
            error("malloc");
        //*(char *)elements[i]->key=(char)(rand()%93+33);
        random_char((char *)elements[i]->key);
    }
}

void free_elements(SortedListElement_t *elements[], int size){
    int i;
    for (i=0; i<size; i++){
        if (elements[i]!=NULL)
            free(elements[i]);
    }
}

void sig_handler(){
    fprintf(stderr, "catched segmentation fault\n");
    exit(2);
}

void random_char(char* ptr){
   int i;
   char temp;
   for (i=0; i<KEY_SIZE; i++){
      temp = (char)(rand()%93+33);
      *ptr=temp;
      ptr++;
   }
}
