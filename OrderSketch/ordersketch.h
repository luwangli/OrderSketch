#ifndef _ORDERSKETCH_H
#define _ORDERSKETCH_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
using namespace std;

template<int memory_in_byte, int key_len, int d, int single_counter_size = 4>
class OrderSketch
{
private:
	static const int w = memory_in_byte * 8 / (single_counter_size * 8);
	static const int row_size = w / d;
	int counters[d][row_size];
	BOBHash32 *hash[d];
	int order = 0;
	int noise[d] ;

public:
	OrderSketch()
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

	~OrderSketch()
	{
		for (int i = 0; i < d; i++)
		{
			delete hash[i];
		}
	}

	void print_basic_info()
	{
		cout << "TCM Sketch" << endl;
		cout << "\tCounters: " << w << endl;
		cout << "\tMemory: " << w*single_counter_size * 8 / 1024 / 8.0 << "KB" << endl;

	}

	void insert(uint32_t key, int f = 1)
	{
		int index;
		index = (hash[order]->run((const char*)&key, key_len))%row_size;
		counters[order][index] += f;
		noise[order]++;
		order = (order + 1) % d;

	}
/*
	int query(uint32_t key)
	{
	    int index;
	    int tmp = 0;
	    int res;
	    for(int i=0;i<d;i++)
	    {
	        index = (hash[i]->run((const char*)&key, key_len)) % row_size;
	        tmp+=counters[i][index] - (noise[i]-counters[i][index])/(w-1);
	    }
	    res = tmp/d;
	    return res;

	}
*/


	int query(uint32_t key)
	{

	    int index;
	    int myarray[d];
	    int tmp;
	    for(int i =0; i <d;i++)
	    {
	        index = (hash[i]->run((const char*)&key, key_len)) % row_size;
	        myarray[i] = counters[i][index] - (noise[i]-counters[i][index])/(w-1);
	    }
	    for(int i = d-1;i>=0;i--)
	    {
	        for(int j=0;j<i;j++){
	            if(myarray[j]>myarray[j+1]){
	                tmp = myarray[j];
	                myarray[j] = myarray[j+1];
	                myarray[j+1] = tmp;
	            }

	        }
	    }

	    if(d&1)
	        return myarray[d/2];
	    else
	        return (myarray[d/2]+myarray[d/2 -1])/2;

	}
	

};

#endif
