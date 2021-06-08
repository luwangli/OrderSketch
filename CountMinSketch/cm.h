#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
using namespace std;

//#define SINGLE_COUNTER_SIZE 4
//#define SINGLE_COUNTER_SIZE 32

//template<int memory_in_byte,int key_len, int d>//d represent for hash function number,key_len represent the length of return key
template<int memory_in_byte,int key_len,int d,int single_counter_size = 4>
class CMSketch
{
private:
	static const int w = memory_in_byte * 8 /(single_counter_size*8);
	static const int row_size = w/d;
//	static 	const int w;
//	static const int row_size;
//	static const int hash_num;
//	int counters[hash_num][row_size];
	int counters[d][row_size];
	BOBHash32 *hash[d];
	
public:
	
	CMSketch()
	{
		memset(counters,0,sizeof(counters));
		int a;
		for (int i = 0; i < d; i++)
		{
		    srand((unsigned)time(NULL));
		    a =rand()%1200;
//		    cout<<"random \t"<<a<<endl;
			hash[i] = new BOBHash32(i + a);
		}
	}
/*
	CMSketch(int memory_in_byte,int key_len,int d =3):
	w(memory_in_byte*8/SINGLE_COUNTER_SIZE),hash_num(d),row_size(w/d)
	{
//		w =  memory_in_byte * 8 /SINGLE_COUNTER_SIZE;
//		row_size = w/d;
	//	counters = new int[hash_num][row_size];
		memset(counters,0,sizeof(counters));
		for(int i = 0;i<d;i++)
		{
			hash[i] = new BOBHash32(i+750);
		}
	
	}
*/
	~CMSketch()
	{
		for(int i =0;i<d;i++)
		{
			delete hash[i];
//			delete counters[i];
		}
	}

	void print_basic_info()
	{
		cout<<"CM Sketch"<<endl;
		cout<<"\tCounters: "<<w<<endl;
		cout<<"\tMemory: "<< w*single_counter_size*8/1024/8.0<<"KB"<<endl;
//		cout<<"\tMemory: "<<w*SINGLE_COUNTER_SIZE<<endl;
	}
	
	void insert(uint32_t key, int f = 1)
	{
		int index;
		for(int i =0;i<d;i++)
		{
			index = (hash[i]->run((const char*)&key,key_len)) % row_size;
			counters[i][index] += f;
		}
	}

	int query(uint32_t key)//return miminum value
	{
		int index;
		int query_value = 1<<30;
		for(int i=0;i<d;i++)
		{
			index = (hash[i]->run((const char*)&key,key_len))%row_size;
			query_value = min(query_value,counters[i][index]);
		}
		return query_value;
	}
	void decrease(uint32_t key,int f)
	{
		int index;
		for(int i=0;i<d;i++)
		{
			index = (hash[i]->run((const char*)&key,key_len)) % row_size;
			counters[i][index] -= f;
		}
	}
};
#endif
