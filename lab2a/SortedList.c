#include "SortedList.h"
#include <stdio.h>
#include <sched.h>
#include <string.h>

#define MAX_SIZE 1000000

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    // List is empty
    if (list->prev==NULL){
        list->next = element;
        list->prev = element;
        if (opt_yield&INSERT_YIELD)
            sched_yield();
        element->next = list;
        element->prev = list;
        return;
    }
    // Insert in the middle
    SortedListElement_t *temp = list->next;
    while (temp->key!=NULL){
        if(strcmp(element->key,temp->key)<0){
        //if (*element->key < *temp->key){
            element->prev = temp->prev;
            element->next = temp;
            if (opt_yield&INSERT_YIELD)
                sched_yield();
            element->prev->next = element;
            temp->prev = element;
            return;
        }
        temp = temp->next;
    }
    // Insert at the end
    element->prev = list->prev;
    element->next = list;
    if (opt_yield&INSERT_YIELD)
        sched_yield();
    element->prev->next = element;
    list->prev = element;
    return;
}


// Return 0: element deleted successfully, 1: corrtuped prev/next pointers
int SortedList_delete(SortedListElement_t *element){
    if(element->prev!=NULL && element->next!=NULL){
        if(element->next->prev==element && element->prev->next==element){
            element->next->prev = element->prev;
            if (opt_yield&DELETE_YIELD)
                sched_yield();
            element->prev->next = element->next;
            element->prev = NULL;
            element->next = NULL;
            return 0;
        }
    }
    return 1;
}

// Return pointer to matching element, or NULL if none is found
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    // List is empty
    if (list->next==NULL)
        return NULL;
    
    int c=0;
    SortedListElement_t *temp = list->next;

    while(temp->key!=NULL){
        if (*temp->key==*key){
            if (opt_yield&DELETE_YIELD)
                sched_yield();
            return temp;
        }
        temp = temp->next;
        c++;
        if(c>MAX_SIZE)
            return NULL;
    }
    return NULL;
}

// Return list length (excluding head), or -1 if the list is corrupted
int SortedList_length(SortedList_t *list){
    // List is empty
    if (list->next==NULL)
        return 0;

    int count=0;
    SortedListElement_t *temp = list->next;
    while (1){
        if (temp->next==NULL || temp->prev==NULL)
            return -1;
        if (temp->key==NULL)
            break;
        count++;
        temp = temp->next;
        if (count>MAX_SIZE)
            return -1;
    }
    if (opt_yield&LOOKUP_YIELD)
        sched_yield();
    return count;
}

/**
 * variable to enable diagnositc yield calls
 */
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	LOOKUP_YIELD	0x04	// yield in lookup/length critical esction

