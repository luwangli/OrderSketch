#ifndef _COUNTSKETCH_H
#define _COUNTSKETCH_H

#include"../common/BOBHash32.h"
#include"../common/param.h"

using namespace std;

template<int memory_in_byte, int key_len, int d, int single_counter_size =4>
class COUNTSketch
{
private:
	static const int w = memory_in_byte * 8 / (single_counter_size * 8);
	static const int row_size = w / d;
	int counters[d][row_size];
	BOBHash32 *hash[d];
	BOBHash32 *hash_polar[d];

public:
	COUNTSketch()
	{
		memset(counters, 0, sizeof(counters));
		int a;
		for (int i = 0; i < d; i++)
		{
		    srand((unsigned)time(NULL));
		    a =rand()%1200;
//		    cout<<"random \t"<<a<<endl;
			hash[i] = new BOBHash32(i + a);
			hash_polar[i] = new BOBHash32(i * 10 + 100);
		}

	}
	void print_basic_info()
	{
		cout << "Count Sketch" << endl;
		cout << "\tCounters: " << w << endl;
		cout << "\tMemory: " << w*single_counter_size * 8 / 1024 / 8.0 << "KB" << endl;
	}

	void insert(uint32_t key, int f = 1)
	{
		int index;
		int polar;
		for (int i = 0; i < d; i++)
		{
			index = (hash[i]->run((const char*)&key, key_len)) % row_size;
			polar = (hash_polar[i]->run((const char*)&key, key_len)) % row_size;
			counters[i][index] += polar ? 1 : -1;
		}
	}

	int query(uint32_t key)
	{
		int index;
		int query_value = 1 << 30;
		for (int i = 0; i < d; i++)
		{
			index = (hash[i]->run((const char*)&key, key_len)) % row_size;
			query_value = min(query_value, abs(counters[i][index]));
		}
		return query_value;
	}
};
#endif
