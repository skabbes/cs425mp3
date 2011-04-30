#include <cstdlib>
#include <vector>



vector<int> idList;
vector<int> portList;

vector<int> idByteList;
vector<int> byteList;
/**
* Read file and extract contents from the file
* then parse them to vector
*/

void readFile(char * filename, vector<int> key, vector<int> value)
{
	FILE *fp;
	char c;
	int sameId = 0;
	int counter = 0;
	int memConfFile = 0;


	if (filename == "")
	{
		memConfFile = 1;
	} 

	fp = fopen(filename, "r");

	if (fp!=NULL)
	{
		while(!feof(fp))
		{
			c =  fgetc(fp);
			if (isdigit(c))
			{
				counter *= 10;
				counter += atoi(&c);
			} else if (c ==' ' || c=='\n')
			{
				if (c==' ')
				{
					if (counter == id)
					{
						sameId = 1;
					}

					key.push_back(counter);
				} else {
					if (sameId == 1)
					{
						port = counter;
					}
					value.push_back(counter);
					sameId = 0;
				}
				counter = 0;
			}
		}

	}
	fclose(fp);
}
