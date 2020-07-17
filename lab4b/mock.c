#include "mock.h"
#include <stdlib.h>

mraa_aio_context mraa_aio_init(int port){
    port = 1;
    mraa_aio_context p = malloc(sizeof(mraa_aio_context));
    return p;
}

int mraa_aio_read(mraa_aio_context aio){
    *aio = 1;
    return 500;
}


void mraa_aio_close(mraa_aio_context aio){
    free(aio);
}

mraa_gpio_context mraa_gpio_init(int pin){
    pin = 1;
    mraa_aio_context p = malloc(sizeof(mraa_aio_context));
    return p;
}

void mraa_gpio_close(mraa_gpio_context btn){
    free(btn);
}

int mraa_gpio_dir (mraa_gpio_context btn, int a){
    *btn = 1;
    a = 1;
    return 1;
}

int mraa_gpio_isr(mraa_gpio_context btn,int a, void(*f)(void *), void* p){
    *btn = 1;
    a = 1;
    //f(NULL);
    p = NULL;
    return 0;
}
