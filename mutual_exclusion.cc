#include "mutual_exclusion.h"

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <pthread.h>

using namespace std;

pthread_mutex_t token = PTHREAD_MUTEX_INITIALIZER;
pthread_t tokenThread;

void mutual_exclusion_init( vector<int> const & nodes, vector<int> const & ports, int id ){

     // find minimum node (to determine which node holds token first)
     int minNode = nodes[0];
     for(unsigned int i=0;i < nodes.size();i++){
        if( nodes[i] < minNode){
            minNode = nodes[i];
        }
     }

     if( minNode == id ){
         // only the minimum node will be unlocked
         pthread_mutex_unlock( &token );
         cout << "Node id = " << id << " has the token" << endl;
     } else {
         // only the minimum node will be unlocked
         pthread_mutex_lock( &token );
     }
}

void mutual_exclusion_destroy(){
    pthread_mutex_unlock( &token );
    pthread_cancel( tokenThread );
}

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

void * tokenPasser( void * arg ){
    //int nextPort = *(int*)arg;
    free(arg);
    return NULL;
}

/**
* Acquire lock (Before enter critical section)
* @return - 1 if successfully acquire lock, otherwise, 0 is returned!
*/
void lock()
{
    pthread_mutex_lock( &token );
}

/**
* Release lock (Exit critical section)
* @return - 0 if lock is release
*/
void unlock()
{
	// Update variables() we modified
	// mutex_lock();

	// passToken along


	//releaseToken();
	// set state to free
	//currentState = TOKEN_FREE;
}
