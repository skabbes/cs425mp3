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


vector<int> idList, portList;		// keep track of contents from membership.confs
vector<string> commandList;

// function definitions
int main(int argc, char ** argv);
pid_t launchDsm(int nodeId);
bool processCommand(stringstream &is);
void processCommand(string command);


// Main function
int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <command_file>" << endl;
       return EXIT_FAILURE;
    }

    // read membership.conf and launch processes
	 readMembershipConfig(idList, portList);
	 pid_t pid[idList.size()];

	 for (unsigned int i=0; i < idList.size(); ++i)
	 {
		pid[i] = launchDsm(idList[i]);
	 }
	 

    // read lines from command file and issue commands
	 char * filename = argv[1];
	 readCommandFile(filename, commandList);

	for (unsigned int i=0; i < commandList.size(); ++i)
	{
		//cout << commandList[i] << endl;

		// Should I wait until the command is completely executed???
		processCommand(commandList[i]);
	}
	// read from command line
	/**
	 string input;				// Command input
	 bool shouldQuit = false;
	 while( cin && !shouldQuit )
	 {
        stringstream is (stringstream::in | stringstream::out);
        getline(cin, input);
        is << input;
        shouldQuit = processCommand(is);
    }**/

	while(1)
   {

   }

    return 0;
}

/**
* Process command (Generic)
* @param: command: command line
*/
void processCommand(string command)
{
	//printf("Process Command from command file\n");

	vector<int> results;

	//interpret command
	interpretCommand(command, results); 
	
	if (results.size() < 3)
	{
		cout << "[Error]:Invalid Command\n";
		return;
	}

	usleep(results[0]);	// sleep for n miliseconds

	int nodeTarget = results[1];	// node to be executed
	int exeComm = results[2];	// executed command
	

	int targetSocket = setup_client("localhost", findPort(nodeTarget, idList, portList));

	// process command
	if (exeComm == ACQUIRE_LOCK)
	{
		// acquire lock if not held by any process	
		sendint(targetSocket, ACQUIRE_LOCK);

	} else if (exeComm == RELEASE_LOCK)
	{
		// release a previously held rock
		sendint(targetSocket, RELEASE_LOCK);

	} else if (exeComm == DO_WORK)
	{
		// Add values stored at shared memory
		sendint(targetSocket, DO_WORK);
		sendint(targetSocket, results.size() - 3);	// send the number of total of parameters
		// send parameters (List of mem. locations)
		for (unsigned int i=3; i < results.size(); ++i)
		{
			sendint(targetSocket, results[i]);	
		}
		
	} else if (exeComm == PRINT)
	{
		// Print the valued stored at the memory location
		int memLoc = results[3];
		sendint(targetSocket, PRINT);	// send PRINT command
		sendint(targetSocket, memLoc);	// send Memory location

	} else {
		cout << "[Error]:Invalid Command\n";
	}
	close(targetSocket);

}

/**
* processCommand received from the command line
* @param is: Streamcommand
* @return True: If no need to process command anymore
*/
bool processCommand(stringstream &is)
{
	printf("Process Command Begins \n");
	string command;
	command = is.str();
	
	processCommand(command);

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

