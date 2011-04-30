#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "messages.h"
#include "config.h"
#include "socket.h"

using namespace std;

// function definitions
int main(int argc, char ** argv);
pid_t launchDsm(int nodeId);
bool processCommand(stringstream & is);


// Main function
int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <command_file>" << endl;
       return EXIT_FAILURE;
    }

    // char * filename = argv[1];

	 vector<int> idList, portList;
	 string input;				// Command input


    // read membership.conf and launch processes
	 readMembershipConfig(idList, portList);

	 pid_t pid[idList.size()];

	 for (unsigned int i=0; i < idList.size(); ++i)
	 {
    	 pid[i] = launchDsm(idList[i]);
	 }
	 

    // read lines from command file and issue commands
	 bool shouldQuit = false;
	 while( cin && !shouldQuit )
	 {
        stringstream is (stringstream::in | stringstream::out);
        getline(cin, input);
        is << input;
        shouldQuit = processCommand(is);
    }

    return 0;
}

/**
* processCommand received from the command line
* @param is: Streamcommand
* @return True: If no need to process command anymore
*/
bool processCommand(stringstream & is)
{
	printf("Process Command Begins\n");
	return false;

}


/**
* Launch DSM (node)
* @param : nodeId: Identifier
*/
pid_t launchDsm(int nodeId){
    pid_t pid = fork();

    // a 16-bit number will be at most 5 characters
    char nodeIdString[10] = {0};
    sprintf(nodeIdString, "%d", nodeId);

    if( pid == 0 ){
        // launch listener process
        execlp("./dsm", "./dsm", nodeIdString, (char *)0 );
    } 

    return pid;
}

