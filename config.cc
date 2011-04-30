#include "config.h"
#include <iostream>
#include <fstream>
using namespace std;

// read membership.config file and parse data to idList and portList
// @param idList: List of nodeID
// @param portList: List of port corresponding to each id
void readMembershipConfig(vector<int> &idList, vector<int> &portList)
{
	readFile("membership.conf",idList, portList);
}

// read memorymap.config file and parse data to byteList and nodeList
// @param byteList : memory address associated with nodeId
// @param id = Identifier
void readMemoryMapConfig(vector<int> &byteList, int id)
{
	vector<int> nodeList, indexList, byteTemp;
	readFile("memory_map.conf", byteTemp, nodeList);

	// find indices in the nodeList that has this 'id'
	for (unsigned int i=0; i < byteTemp.size(); ++i)
	{
		if (id == nodeList[i])
		{
			indexList.push_back(i);
		}
	}

	// append all information that belongs to this 'id'
	for (unsigned int i=0; i < indexList.size(); ++i)
	{
		byteList.push_back(byteTemp[indexList[i]]);
	}

}

// find port
// @param: id nodeId to find the port for
// @param: nodeList: list of node
// @param: portList: list of port
// @return return port number if found, otherwise, -99 is returned.
int findPort(int id, vector<int> nodeList, vector<int> portList)
{
	for (unsigned int i=0; i < nodeList.size(); ++i)
	{
		if (nodeList[i] == id)
		{
			return portList[i];
		}
	}
	return -99;
}



// Read file and parse content
// @param filename
// @param keyList: a list of stored key (i.e. ID)
// @param valueList: a list of stored value corresponding to the key
void readFile(string filename, vector<int> &keyList, vector<int> &valueList)
{
    ifstream file(filename.c_str());
    int key, value;

    if (file.is_open()){
        while ( file.good() )
        {
            file >> key;
            file >> value;

            if( file.good() ){
                keyList.push_back( key );
                valueList.push_back( value );
            }
        }
        file.close();
    }
}

// Read command file
// @param filename : filename
// @param command: stored list of commands
void readCommandFile(char* filename, vector<string> &command)
{
	 ifstream file(filename);
	 string line;
    if (file.is_open()){
        while ( file.good() )
        {
            file >> line;

            if( file.good() )
				{
                command.push_back(line);
            }
        }
        file.close();
    }
	
}

// Printing content from Vectors
// @param keyList: a list of stored key (i.e. ID)
// @param valueList: a list of stored value corresponding to the key
void printContent(vector<int> idList, vector<int> valueList)
{
   for (unsigned int i=0; i < idList.size(); ++i)
   {
      printf("Id: %i : %i\n", idList[i], valueList[i]);
   }
}

/**
* Spliting string into vector of string
* @param str : Full string
* @param delim : Delimiter (spliter)
* @param results: Vector result (pass by ref)
*/
void splitString(string str, string delim, vector<string> &results)
{
	int cutAt;
	while( (cutAt = str.find_first_of(delim)) != (int) str.npos )
	{
		if(cutAt > 0)
		{
			results.push_back(str.substr(0,cutAt));
		}
		str = str.substr(cutAt+1);
		
	}

	if(str.length() > 0)
	{
		results.push_back(str);
	}
}

/**
* Interpret the string command
* @param str: string command
* @param commandResult: stored vector int
*/
void interpretCommand(string str, vector<int> &commandResult)
{
	vector<string> results;
	splitString(str, ":", results);
	for (unsigned int i = 0; i < results.size(); ++i)
	{

		char* pch = (char*)malloc( sizeof( char ) *(results[i].length() +1) );
		strcpy(pch, results[i].c_str() );
		commandResult.push_back(atoi(pch));
		free(pch);
	}
}
