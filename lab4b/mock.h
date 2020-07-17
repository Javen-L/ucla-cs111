#ifndef MOCK_H
#define MOCK_H

#define MRAA_GPIO_IN 1
#define MRAA_GPIO_EDGE_RISING 1

typedef int* mraa_aio_context;
typedef int* mraa_gpio_context;

mraa_aio_context mraa_aio_init(int port);
int mraa_aio_read(mraa_aio_context aio);
void mraa_aio_close(mraa_aio_context aio);

int mraa_gpio_dir (mraa_gpio_context btn, int a);	
mraa_gpio_context mraa_gpio_init(int pin);
void mraa_gpio_close(mraa_gpio_context btn);
int mraa_gpio_isr(mraa_gpio_context btn,int a, void(*f)(void *), void* p);


#endif