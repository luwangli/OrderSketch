#ifndef _CUSKETCH_H
#define _CUSKETCH_H

#include"../common/param.h"
#include"../common/BOBHash32.h"
using namespace std;

template <int memory_int_bytes,int key_len, int d,int single_counter_size = 4>
class CUSketch
{
private:
	static const int w = memory_int_bytes * 8 / (single_counter_size * 8);
	int counters[w];
	BOBHash32 *hash[d];

public:
	CUSketch()
	{
		memset(counters,0,sizeof(counters));
		for (int i = 0; i< d;i++){
			hash[i] = new BOBHash32(i + 750);
		}
	}

	virtual ~CUSketch()
	{
		for (int i=0;i<d; i++)
			delete hash[i];
	}

	void print_basic_info()
	{
		cout << "CU Sketch" << endl;
		cout << "\tCounters: " << w << endl;
		cout << "\tMemory: " << w*single_counter_size * 8 / 1024 / 8.0 << "KB" << endl;
	}

	void insert(uint32_t key, int f = 1)
	{
		int index[d];
		int value[d];
		int min_val = 1 << 30;		

		for(int i = 0; i< d;i++){
			index[i] = (hash[i]->run((const char *)&key,key_len)) % w;
			value[i] = counters[index[i]];
			min_val = min(min_val,value[i]);
		}
		
		int temp = min_val+ f;
		for(int i = 0; i< d;i++){
			counters[index[i]] = max(counters[index[i]], temp);
		}
	}

	int query(uint32_t key)
	{
		int ret = 1 << 30;
		for (int i =0; i<d; i++){
			int tmp = counters[(hash[i]->run((const char *)&key,key_len))%w];
			ret = min(ret, tmp);
		}
		return ret;
	}	
};

#endif 	
