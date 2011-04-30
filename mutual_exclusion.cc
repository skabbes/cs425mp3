#include "mutual_exclusion.h"

#include <vector>
#include <iostream>
#include <pthread.h>
#include <cstdlib>

using namespace std;

pthread_mutex_t token = PTHREAD_MUTEX_INITIALIZER;
pthread_t tokenThread;

/**
 * Initialize the distributed mutual exclusion code
 * This should only be called after all servers are up and running
 */
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

void tokenReceived(){
    pthread_mutex_unlock( &token );
    // I think mutex maintain a queue for first come first serve ordering
    // but need to double check on that
    pthread_mutex_lock( &token );

    // send the token to the next guy
}

/**
 * When you quit, make sure to destroy the token passer
 */
void mutual_exclusion_destroy(){
    pthread_cancel( tokenThread );
    pthread_mutex_unlock( &token );
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
