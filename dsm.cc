#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>


#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>



#include "messages.h"
#include "config.h"
#include "socket.h"

using namespace std;

// Declaration of variables
int currentState = TOKEN_FREE;
bool hasToken = false;

// function definitions
int main(int argc, char ** argv);
pthread_t startDetachedThread( void * (*functor)(void *), void * arg );
pthread_t startThread( void * (*functor)(void *), void * arg );
int lock();
int unlock();
void receiveToken();
void releaseToken();
void passTokenAlong();


/**
* Acquire lock (Before enter critical section)
* @return - 1 if successfully acquire lock, otherwise, 0 is returned!
*/
int lock()
{
	currentState = TOKEN_WANT;
	
	if (hasToken)
	{
		// if we have token
		// set state to "held" and then return
		currentState = TOKEN_HELD;
		return 1;
	}
	return 0;
}

/**
* Release lock (Exit critical section)
* @return - 0 if lock is release
*/
int unlock()
{
	// Update variables() we modified
	// mutex_lock();

	// passToken along


	releaseToken();
	// set state to free
	currentState = TOKEN_FREE;
	return 0;
}

void passTokenAlong()
{

}


void receiveToken()
{
	hasToken = true;
}

void releaseToken()
{
	hasToken = false;
}





int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <nodeId>" << endl;
       return EXIT_FAILURE;
    }

    int nodeId = atoi(argv[1]);


	 vector<int> nodeList, portList, byteList, socketList;		// keep track of node, port , mem_addr, and socket
	 int socket, port;						// keep track of socket and port this nodeId owns

    cout << "Started DSM with id=" << nodeId << endl;

	 // read memory_map.conf, set up variables that this node owns
	 readMemoryMapConfig(byteList, nodeId);

    // read membership.conf and determine what port to run on (including all other nodes)
	 readMembershipConfig(nodeList, portList);

	 port = findPort(nodeId, nodeList, portList);	// find the port that this node is running on

	 if (port == -99)
	 {
		printf("[Error]: No port found for id:%i .\n",nodeId);
		return EXIT_FAILURE;
	 }


	 // setup a server connection to run on that port
	 socket = setup_server(port,&port);

	 // attempt to setup connection to all nodes
	 for (unsigned int i = 0; i < portList.size(); ++i)
	 {
		if (nodeList[i] != nodeId)	// for each node that is not this node
		{
			int clientSocket = 2;	// 2 means failed to connect
			while (clientSocket == 2)
			{
				clientSocket = setup_client("localhost", portList[i]);	
				if (clientSocket == 2) sleep(1);	// wait for connecting node to be active
			}
			socketList.push_back(clientSocket);	// store socket that we are connecting to 
		} else {
			socketList.push_back(socket);		// this node
		}
	 }

	// I'm wondering why we don't need to accept()?? but the socket is connected
	while(true)
	{
    	// get connections, launch a thread for each connection
        cout << "getting connections!" << endl;
        sleep(1);
   }

   return 0;
}

/**
* Stop threads and free resources
*
*/
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

/**
* Start thread
* 
*/
pthread_t startThread( void * (*functor)(void *), void * arg ){
    pthread_t handler;
    if( pthread_create(&handler, NULL, functor, arg) ){
        free(arg);
        perror("pthread_create");
    }
    return handler;
}
