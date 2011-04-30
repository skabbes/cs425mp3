#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>

pthread_t startDetachedThread( void * (*functor)(void *), void * arg );
pthread_t startThread( void * (*functor)(void *), void * arg );
#endif
