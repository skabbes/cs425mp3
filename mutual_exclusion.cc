#include "mutual_exclusion.h"

#include <vector>
#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include "thread.h"
#include "messages.h"
#include "socket.h"

using namespace std;

pthread_mutex_t token = PTHREAD_MUTEX_INITIALIZER; // token
int nextPort;


/**
 * Initialize the distributed mutual exclusion code
 * This should only be called after all servers are up and running
 * @param : nodes: List of nodes
 * @param : ports: List of ports corresponding to nodes
 * @param : id : Node identifier
 */
void mutual_exclusion_init( vector<int> const & nodes, vector<int> const & ports, int id ){

     // find minimum node (to determine which node holds token first)
     int minNode = nodes[0];
     int nextIndex = 0;
     for(unsigned int i=0;i < nodes.size();i++){
        if( nodes[i] < minNode){
            minNode = nodes[i];
        }

        // find the next port to connect to
        if( nodes[i] > id && (nodes[nextIndex] <= id || nodes[i] < nodes[nextIndex]) ){
            nextIndex = i;
        }
     }

     // actually, we just care about the next port
     nextPort = ports[nextIndex];
     cout << "Node " << id << "'s next node is " << nodes[nextIndex] << endl;

     if( minNode == id ){
         // only the minimum node will be unlocked
         pthread_mutex_unlock( &token );
         cout << "Node id = " << id << " has the token" << endl;
         tokenReceived();
     } else {
         // only the minimum node will be unlocked
         pthread_mutex_lock( &token );
     }
}

/**
* Upon receiving a token
*/
void tokenReceived(){
    pthread_mutex_unlock( &token );

    // potentially wait some time?
    sleep(1);

    // I think mutex maintain a queue for first come first serve ordering
    // but need to double check on that
    pthread_mutex_lock( &token );

    // send token along to next guy
    int socket = setup_client("localhost", nextPort);
    sendint(socket,  TOKEN);
    close( socket );
}

void mutual_exclusion_destroy(){
    pthread_mutex_unlock( &token );
}

/**
* Acquire lock (Before enter critical section)
*/
void lock()
{
    // this mutex will be available once we possess a token
    pthread_mutex_lock( &token );
}

/**
* Release lock 
*/
void unlock()
{
    pthread_mutex_unlock( &token );
}
