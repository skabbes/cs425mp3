#ifndef CONFIG_H
#define CONFIG_H

#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

void readMembershipConfig(vector<int> &idList, vector<int> &portList);
void readMemoryMapConfig(vector<int> &byteList, int id);
void readFile(string filename, vector<int> &keyList, vector<int> &valueList);
void readCommandFile(char * filename, vector<string> &command);
void printContent(vector<int> idList, vector<int> valueList);
int findPort(int id, vector<int> nodeList, vector<int> portList);
int hasMemoryAddr(int id, int memAddr, vector<int> byteList);
void splitString(string str, string delim, vector<string> &results);
void interpretCommand(string str, vector<int> &commandResult);
#endif
