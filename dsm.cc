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
#include "thread.h"
#include "mutual_exclusion.h"

using namespace std;


// function definitions
int main(int argc, char ** argv);
void  * thread_conn_handler( void * arg );

// NODE variables / properties
int nodeId;
int currentState = TOKEN_FREE;
bool hasToken = false;


vector<int> nodeList, portList, byteList, socketList;		// keep track of node, port , mem_addr, and socket
int server, port;						// keep track of socket and port this nodeId owns

/**
* Main function
*/
int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <nodeId>" << endl;
       return EXIT_FAILURE;
    }

    nodeId = atoi(argv[1]);

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
	 server = setup_server(port,&port);

	 // attempt to setup connection to all nodes
	 for (unsigned int i = 0; i < portList.size(); ++i)
	 {
		if (nodeList[i] != nodeId)	// for each node that is not this node
		{
			int clientSocket = 2;	// 2 means failed to connect
			while (clientSocket == 2)
			{
				clientSocket = setup_client("localhost", portList[i]);	
                sendint(clientSocket, PING);
				if (clientSocket == 2) sleep(1);	// wait for connecting node to be active
			}
			socketList.push_back(clientSocket);	// store socket that we are connecting to 
		} else {
			socketList.push_back(server);		// this node
		}
	 }
    
    mutual_exclusion_init(nodeList, portList, nodeId); 

    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];

	// I'm wondering why we don't need to accept()?? but the socket is connected
	while(true)
	{
		 // accept() socket operation on non socket????
        sin_size = sizeof their_addr;
        int new_fd = accept(server, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) 
			{
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        //cout << "Node " << nodeId << " got connection from " << s << endl;

        // prepare argument for thread
        int * arg = new int(new_fd);
        startDetachedThread(thread_conn_handler, arg);
    }

   return 0;
}

void * thread_conn_handler(void * arg){
    int socket = *(int *)arg;
    free(arg);

    int message = readint( socket );

    if( message == ACQUIRE_LOCK){
        cout << "Node " << nodeId << " got ACQUIRE_LOCK message" << endl;
        lock();
        cout << "Node " << nodeId << " has acquired lock " << endl;
    }
    else if( message == RELEASE_LOCK){
        cout << "Node " << nodeId << " got RELEASE_LOCK message" << endl;
        // do other stuff before actually unlocking, like update variables and such which were modified
        unlock();
    }
    else if( message == DO_WORK){
        cout << "Node " << nodeId << " got DO_WORK message" << endl;
        int totalsize = readint(socket);
        int params[totalsize];			// keep track of nodes given
        for (unsigned int i =0 ; i < totalsize ; ++i)
        {
               params[i] = readint(socket);
        }
        
        int value = params[totalsize-1];
		  // add value to to mem location

    }
    else if( message == PRINT){
        cout << "Node " << nodeId << " got PRINT message" << endl;
        int memLoc = readint(socket);
        

    }
    else if( message == READ){
        cout << "Node " << nodeId << " got READ message" << endl;
        int memLoc = readint(socket);

        // Check if this node has this memory address
        int indexMemAddr = hasMemoryAddr(nodeId, memLoc, byteList);

        if (indexMemAddr != -1)
        {
            // send back value at memaddr How???
				
            //sendint(socket, MAP_VALUE); 
            sendint(socket, byteList[indexMemAddr]);  // Send back to map address
        }
    }
    else if( message == WRITE){
        cout << "Node " << nodeId << " got WRITE message" << endl;
		  int memLoc = readint(socket);
		  
		  int indexMemAddr = hasMemoryAddr(nodeId,memLoc, byteList);

		  if (indexMemAddr != -1)
		  {
				// store the value
				
		  }
			
    }
    else if( message == QUIT){
        cout << "Node " << nodeId << " got QUIT message" << endl;
    }

    else if( message == TOKEN ){
        tokenReceived();
        cout << "Node " << nodeId << " has the token" << endl;
    }



// Proabaly dont' want anything after this line

    else if( message == TOKEN_WANT){
        cout << "Node " << nodeId << " got TOKEN_WANT message" << endl;

        if (hasToken && currentState == TOKEN_FREE)
        {
            // release token
            hasToken = false;
            sendint(socket, TOKEN_FREE);
        }
    }
    else if( message == TOKEN_HELD){
        cout << "Node " << nodeId << " got TOKEN_HELD message" << endl;

        if (currentState == TOKEN_WANT)
        {
            sendint(socket, TOKEN_WANT);
        }
    }
    else if( message == TOKEN_FREE){
        cout << "Node " << nodeId << " got TOKEN_FREE message" << endl;

        if (!hasToken && currentState == TOKEN_WANT)
        {
            currentState = TOKEN_HELD;
            hasToken = true;
        }

    }
    else if( message == PING){
        cout << "Node " << nodeId << " got PING message" << endl;

    }
    else{
        cout << "Unrecognized message" << endl;
    }
    close(socket);
    return NULL;
}
