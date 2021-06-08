#ifndef _SimpleHash_H
#define _SimpleHash_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
using namespace std;

template<int memory_in_byte, int key_len, int d, int single_counter_size = 4>
class SimpleHash
{
private:
	static const int w = memory_in_byte * 8 / (single_counter_size * 8);
	static const int row_size = w / d;
	int counters[d][row_size];
	BOBHash32 *hash[d];
	int order = 0;
	int noise;

public:
	SimpleHash()
	{
		memset(counters, 0, sizeof(counters));
		for (int i = 0; i < d; i++)
		{
			hash[i] = new BOBHash32(i + 750);
		}

	}

	~SimpleHash()
	{
		for (int i = 0; i < d; i++)
		{
			delete hash[i];
		}
	}

	void print_basic_info()
	{
		cout << "Simple Hash" << endl;
		cout << "\tCounters: " << w << endl;
		cout << "\tMemory: " << w*single_counter_size * 8 / 1024 / 8.0 << "KB" << endl;

	}

	void insert(uint32_t key, int f = 1)
	{
		int index;
		index = (hash[order]->run((const char*)&key, key_len))%row_size;
		counters[order][index] += f;
		order = (order + 1) % d;
		noise++;
	}

	int query(uint32_t key)
	{
		int index;
		index = (hash[0]->run((const char*)&key, key_len)) % row_size;
		return counters[0][index];
		/***
		int query_value = 1<<30;
		int modify_value = 0;
		for (int i = 0; i < d; i++)
		{
			index = (hash[i]->run((const char*)&key, key_len)) % row_size;
//			modify_value = abs(counters[i][index]-(noise-counters[i][index])/(w-1));
			modify_value = counters[i][index];
//			query_value += counters[i][index];
			query_value = min(query_value,modify_value);
		}
		return query_value*d;//correct result
		***/
	}

};

#endif
