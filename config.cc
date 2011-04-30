#include "config.h"

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
void readFile(char * filename, vector<int> &keyList, vector<int> &valueList)
{
   FILE *fp;  

   char c;

   int counter = 0;
   fp = fopen(filename, "r");

   if (fp!=NULL)
   {
     
      while (!feof(fp)) 
      {
         c = fgetc(fp);
         if (isdigit(c))
         {
            counter *= 10;
            counter += atoi(&c);
         } else if (c==' ' || c=='\n')
         {
            if (c== ' ')
            {
               keyList.push_back(counter);
            } else {
               valueList.push_back(counter);
            }         
            counter = 0;
         }
      }
      fclose(fp);
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