//focus on function about data reading 
#ifndef _DATA_READING_H
#define _DATA_READING_H
#include<iostream>
#include<unordered_map>
using namespace std;

#define MAX_INSERT_PACKET 32000000


int ReadInTrace(char const *filename,uint32_t *insert_data,unordered_map<uint32_t,int> &benchmack_data)
{
	
	
	FILE *Fin = fopen(filename, "rb");

	if (!Fin)
	{
		cerr << "cannot open file:" << filename << endl;
		exit(-1);
	}

	char ip[13];//this was defiend by author , <sip,dip> or five-tuple as key
	int counter = 0;
	while (fread(ip, 1, 13, Fin))
	{
		uint32_t key = *(uint32_t *)ip;
		insert_data[counter] = key;
		benchmack_data[key]++;
		counter++;
		if (counter == MAX_INSERT_PACKET) {
			cout << "reach max packet" << endl;
			break;
		}

	}
	fclose(Fin);
	cout << "Successfully read " << filename << endl;
	cout << "Total stream size: " << counter << endl;
	cout << "Distinct item num: " << benchmack_data.size() << endl;

	int max_freq = 0;
	for (auto itr : benchmack_data) {

		max_freq = std::max(max_freq, itr.second);
	}
	printf("Max frequency = %d\n", max_freq);
	
	return counter;
}


#endif
