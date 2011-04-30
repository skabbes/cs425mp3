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
#include "mutual_exclusion.h"

using namespace std;

// function definitions
int main(int argc, char ** argv);

// NODE variables / properties
int nodeId;

int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <nodeId>" << endl;
       return EXIT_FAILURE;
    }

    nodeId = atoi(argv[1]);


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
