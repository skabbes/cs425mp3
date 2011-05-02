#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

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
int readByte(int addr);
void writeBye(int addr, int value);



// NODE variables / properties
int nodeId;
int currentState = TOKEN_FREE;
bool hasToken = false;

// a data "cache" for unmodified bytes (memory_address -> value)
map<int, int> unmodified;

// a data "cache" for modified bytes (memory_address -> value)
map<int, int> modified;

// the bytes that I own (memory_address -> value)
map<int, int> myBytes;

// the bytes that others own (memory_address -> port_number)
map<int, int> otherBytes;


vector<int> nodeList, portList, socketList;		// keep track of node, port , and socket
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

     // read membership.conf and determine what port to run on (including all other nodes)
	 readMembershipConfig(nodeList, portList);

	 port = findPort(nodeId, nodeList, portList);	// find the port that this node is running on
    
     // read in
     map<int, int> allBytes = readFileMap("memory_map.conf");
     map<int, int>::iterator it;
     for ( it=allBytes.begin() ; it != allBytes.end(); it++ ){
        int addr = it->first;
        int id = it->second;

        if( id == nodeId ){
            myBytes[addr] = 0;
        } else {
            otherBytes[addr] = findPort(id, nodeList, portList);
        }
     }

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
		 
		 // Update the actual bytes with cache
		/**
		 map<int, int>::iterator it;
     	 for ( it=myBytes.begin() ; it != myBytes.end(); it++ )
		 {
				int memAddr = it->first();
				myBytes[memAddr] = modified[memAddr];
		 }	
**/
        unlock();
    }
    else if( message == DO_WORK){

        cout << "Node " << nodeId << " got DO_WORK message" << endl;
        int totalsize = readint(socket);
        int params[totalsize];			// keep track of nodes given
        for (int i =0 ; i < totalsize ; ++i)
        {
               params[i] = readint(socket);
        }

        int destinationAddr = params[0];
        int value = params[totalsize-1];
        for (int i=1; i<totalsize-1; ++i)
        {
               value += readByte(params[i]);
        }
        writeByte( destinationAddr, value );

    }
    else if( message == PRINT){
        cout << "Node " << nodeId << " got PRINT message" << endl;
        int memLoc = readint(socket);
    }
    else if( message == READ){
        cout << "Node " << nodeId << " got READ message" << endl;
        int memLoc = readint(socket);
        int value = readByte( memLoc );
        sendint(socket, value);
    }
    else if( message == WRITE){
        cout << "Node " << nodeId << " got WRITE message" << endl;

		  int memLoc = readint(socket);	// where to store	(memAddr)
		  int storeValue = readint(socket);	// what to store (value)

		  int indexMemAddr = hasMemoryAddr(nodeId,memLoc, byteList);

			// if this node has this mem addr
		  if (indexMemAddr != -1)
		  {
				// store the value
				modified[memLoc] = storeValue;
		  } else {
				// otherwise, go to the node that has
				
				// *** Not sure if this is necessary ***
		/**
				int otherSocket = setup_client("localhost", otherBytes[memLoc]);
				sendint(otherSocket, WRITE);
				sendint(otherSocket, memLoc);
				sendint(otherSocket, storeValue);
				close(otherSocket);
**/
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

// abstracts writing to a byte, updates the local copies as necessary
void writeBye(int addr, int value){
    map<int, int>::iterator it;

    // do I have it?
    it = myBytes.find(addr);
    if( it != map::end ){
        myBytes[addr] = value;
        return;
    }

    // have I cached it, if so, get it out of the friggin cache, and into modified
    it = unmodified.find(addr);
    if( it != map::end ){
        unmodified.erase( it );
        modified[addr] = value;
        return;
    }


    // otherwise, let's just cache the value for now
    modified[addr] = value;
}

// abstracts reading a byte.  Grabs it from its local copy if possible, or a cached
// version.  As a last resort, it sends a socket connection to read the value
int readByte(int addr){
    map<int, int>::iterator it;

    // do I have it?
    it = myBytes.find(addr);
    if( it != map::end ){
        return it->second;
    }

    // have I cached it?
    it = readCache.find(addr);
    if( it != map::end ){
        return it->second;
    }

    // have I modified it?
    it = modified.find(addr);
    if( it != map::end ){
        return it->second;
    }

    // we have to look it up :(
    it = modified.find(addr);
    if( it != map::end ){
        int port = it->second;
        int socket = setup_client("localhost", port);
        sendint(socket, READ);
        int value = readint(socket);
        unmodified[addr] = value;
        close(socket);
        return value; 
    }
    return -1;
}
