#ifndef _CUSKETCH2_H
#define _CUSKETCH2_H

#include"../common/param.h"
#include"../common/BOBHash32.h"
using namespace std;

template<int memory_in_bytes,int key_len,int d,int single_counter_size =4>
class CUSketch2
{
private:
	static const int w = memory_in_bytes * 8 / (single_counter_size*8);
	static const int row_size = w / d;
	BOBHash32 *hash[d];
	int counters[d][row_size];	

public:
	CUSketch2()
	{
		memset(counters, 0, sizeof(counters));
		int a;
		for (int i = 0; i < d; i++)
		{
		    srand((unsigned)time(NULL));
		    a =rand()%1200;
//		    cout<<"random \t"<<a<<endl;
			hash[i] = new BOBHash32(i + a);
		}
	}
	virtual ~CUSketch2()
	{
		for (int i = 0; i < d; i++)
		{
			delete hash[i];
		}
	}
	void print_basic_info()
	{
		cout << "CU2 Sketch" << endl;
		cout << "\tCounters: " << w<<endl;
		cout << "\tMemory: " << w*single_counter_size * 8 / 1024 / 8.0 << "KB" << endl;
	}
	void insert(uint32_t key, int f = 1)
	{
		int row = 0;
		int column = 0;
		int position;
		int value;
		
		int min_val = 1 << 30;//for cu, only add the min val

		for (int i = 0; i < d; i++)//get min value and it's location
		{
			position = (hash[i]->run((const char*)&key, key_len)) % row_size;
			value = counters[i][position];
			if (value < min_val)
			{
				min_val = value;
				row = i;
				column = position;
			}
			
		}
		counters[row][column] += f;

		

	}
	int query(uint32_t key)
	{
		int index;
		int ret = 1 << 30;
		for (int i = 0; i < d; i++)
		{
			index = hash[i]->run((const char*)&key,key_len)%row_size;
			
			ret = min(ret, counters[i][index]);
		}
		return ret;
	}
};

#endif
