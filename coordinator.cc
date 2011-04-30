#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "messages.h"
#include "config.h"

using namespace std;

// function definitions
int main(int argc, char ** argv);
pid_t launchDsm(int nodeId);

int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <command_file>" << endl;
       return EXIT_FAILURE;
    }

    // char * filename = argv[1];

	 vector<int> idList, portList;

    // read membership.conf and launch processes
	 readMembershipConfig(idList, portList);

	 pid_t pid[idList.size()];

	 for (unsigned int i=0; i < idList.size(); ++i)
	 {
    	 pid[i] = launchDsm(idList[i]);
	 }
	 

    // read lines from command file and issue commands
	 
    return 0;
}

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

