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
void writeByte(int addr, int value);



// NODE variables / properties
int nodeId;
pthread_mutex_t waitForToken = PTHREAD_MUTEX_INITIALIZER; // token

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

    pthread_mutex_lock( &waitForToken );

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
        pthread_mutex_unlock( &waitForToken );
        lock();
        cout << "Node " << nodeId << " has acquired lock " << endl;
    }
    else if( message == RELEASE_LOCK){
        cout << "Node " << nodeId << " got RELEASE_LOCK message" << endl;

		 // Update the actual bytes with our cache by sending messages
		 map<int, int>::iterator it;
     	 for ( it=modified.begin() ; it != modified.end(); it++ )
		 {
				int memAddr = it->first;
				int value = it->second;
                int port = otherBytes[memAddr];
                int socket = setup_client("localhost", port);
                sendint(socket, WRITE);
                sendint(socket, value);
                close(socket);
		 }	

         // clear "cache" maps
         modified.erase( modified.begin(), modified.end() );

         // we don't need to send messages to modify the actual unmodifed bytes
         unmodified.erase( unmodified.begin(), unmodified.end() );

         pthread_mutex_lock( &waitForToken );
         unlock();
    }
    else if( message == DO_WORK){

        cout << "Node " << nodeId << " got DO_WORK message" << endl;

        pthread_mutex_lock( &waitForToken );
        int totalsize = readint(socket);
        int params[totalsize];
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
        pthread_mutex_unlock( &waitForToken );

    }
    else if( message == PRINT){

        pthread_mutex_lock( &waitForToken );
        cout << "Node " << nodeId << " got PRINT message" << endl;
        int memLoc = readint(socket);
        int value = readByte(memLoc);
		// wait until get token TODO

		// print things out
		cout << "Value " << value << " at mem.location " << memLoc << endl;
        pthread_mutex_unlock( &waitForToken );

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
		  writeByte(memLoc, storeValue);

			
    }
    else if( message == QUIT){
        cout << "Node " << nodeId << " got QUIT message" << endl;
    }

    else if( message == TOKEN ){
        tokenReceived();
        cout << "Node " << nodeId << " has the token" << endl;
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
void writeByte(int addr, int value){
    map<int, int>::iterator it;

    // do I have it?
    it = myBytes.find(addr);
    if( it != myBytes.end() ){
        myBytes[addr] = value;
        return;
    }

    // have I cached it, if so, get it out of the friggin cache, and into modified
    it = unmodified.find(addr);
    if( it != unmodified.end() ){
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
    if( it != myBytes.end() ){
        return it->second;
    }

    // have I cached it?
    it = unmodified.find(addr);
    if( it != unmodified.end() ){
        return it->second;
    }

    // have I modified it?
    it = modified.find(addr);
    if( it != modified.end() ){
        return it->second;
    }

    // we have to look it up :(
    it = otherBytes.find(addr);
    if( it != otherBytes.end() ){
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
