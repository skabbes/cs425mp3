#include "thread.h"
#include <cstdlib>
#include <cstdio>
#include <errno.h>

pthread_t startDetachedThread( void * (*functor)(void *), void * arg ){
    pthread_attr_t DetachedAttr;
    pthread_attr_init(&DetachedAttr);
    pthread_attr_setdetachstate(&DetachedAttr, PTHREAD_CREATE_DETACHED);

    pthread_t handler;
    if( pthread_create(&handler, &DetachedAttr, functor, arg) ){
        free(arg);
        perror("pthread_create");
    }
    pthread_detach(handler);

    // free resources for detached attribute
    pthread_attr_destroy(&DetachedAttr);

    return handler;
}

pthread_t startThread( void * (*functor)(void *), void * arg ){
    pthread_t handler;
    if( pthread_create(&handler, NULL, functor, arg) ){
        free(arg);
        perror("pthread_create");
    }
    return handler;
}

