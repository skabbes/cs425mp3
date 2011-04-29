#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "messages.h"

using namespace std;

// function definitions
int main(int argc, char ** argv);

int main(int argc, char ** argv){
    if( argc != 2){
       cerr << "Usage " << argv[0] << " <nodeId>" << endl;
       return EXIT_FAILURE;
    }

    int nodeId = atoi(argv[1]);

    cout << "Started DSM with id=" << nodeId << endl;
    // read membership.conf and determine what port to run on
    // setup a server connection to run on that port

    return 0;
}
