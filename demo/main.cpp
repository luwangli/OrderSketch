#include"../common/param.h"
#include"../common/data_reading.h"
#include "../common/traceRead.h"
#include"../CountMinSketch/cm.h"
#include"../CUSketch/CUSketch2.h"
//#include"../CMHeap/cmheap.h"
#include"../OrderSketch/ordersketch.h"
#include"../CountSketch/countSketch.h"
#include"../PyramidSketch/PCMSketch.h"
#include"../ColdFilter/CF.h"
#include"../SimpleHash/simplehash.h"


#include<fstream>
#include<time.h>
#include<stdlib.h>
//#include"../CountMouse/countmouse.h"
using namespace std;
#define FREE_MEMORY 400*1024 //avaliable memory,()KB
#define HASH_NUM 4//determine column of sketch;
#define SIMPLEHASH_HASH 1
#define SINGLE_COUNTER_SIZE 4//()byte.
#define KEY_LEN 4// return key of hash function.

uint32_t insert_data[MAX_INSERT_PACKET];
unordered_map<uint32_t,int> benchmark_data;

int main()
{
	int packet_num;
//    packet_num = TraceRead("../data/s1.14_10000000.txt",insert_data,benchmark_data);
	packet_num = ReadInTrace("../data/0.dat",insert_data, benchmark_data);
//	ofstream fout1,fout2;9
//	fout1.open("output1.txt");
//	fout2.open("output2.txt");
	timespec start_time, end_time;
	long long timediff;
	double insert_throughout;
	double query_throughout;
	int test_time = 10;
	long long tot_aae;
	long long tot_are;
	int report_val;
	double res_aae,res_are;
	ofstream outFile;
    outFile.open("result.csv",ios::app);
	cout << "************************************************" << endl;
/*******************************************************CountMin sketch*********************************************/
	auto cm = CMSketch<FREE_MEMORY,KEY_LEN,HASH_NUM,SINGLE_COUNTER_SIZE>();
//	auto cm = CMSketch(10*1024,4,3);
	cm.print_basic_info();

	//speed of insert 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for(int i =0;i<packet_num;i++)
	{
		cm.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	//cout<<"Run time: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;	
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;

	printf("throughput of CM (insert): %.6lf Mips\n", insert_throughout);

	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			 cm.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of CM (query): %.6lf Mips\n\n", query_throughout);

	//AAE
//	int tot_ae;
	tot_aae = 0;
	tot_are = 0;
	ofstream fout_cm;
	fout_cm.open("output_cm.txt");
	for(auto itr:benchmark_data)
	{
		report_val = cm.query(itr.first);
		fout_cm<<itr.first<<": "<<"estimation: "<<report_val<<"  actual value  "<<itr.second<<endl;
		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}
	res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"CM"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fCM AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fCM ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout << "************************************************"<<endl;


	/******************************************CU Sketch**********************************************************/
	auto cu2 = CUSketch2<FREE_MEMORY , KEY_LEN, HASH_NUM, SINGLE_COUNTER_SIZE>();
	//	auto cm = CMSketch(10*1024,4,3);
	//cu2.print_basic_info();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i<packet_num; i++)
	{
		cu2.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	//cout << "Run time: " << (double)(end_cu2 - start_cu2) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of CU (insert): %.6lf Mips\n", insert_throughout);


	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			cu2.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of CU (query): %.6lf Mips\n\n", query_throughout);

	
	tot_aae = 0;
	tot_are = 0;
	ofstream fout_cu2;
	fout_cu2.open("output_cu2.txt");
	for (auto itr : benchmark_data)
	{
		report_val = cu2.query(itr.first);
		fout_cu2 << itr.first << ": " << "estimation: " << report_val << "  actual value  " << itr.second << endl;
	//	tot_ae_cu2 += abs(report_val_cu2 - itr.second);
		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}

    res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"CU"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fCU AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fCU ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"***************************************************"<< endl;

	/*************************************OrderSketch Sketch**********************************************/
	auto ordersketch = OrderSketch<FREE_MEMORY , KEY_LEN, HASH_NUM, SINGLE_COUNTER_SIZE>();
	//tcm.print_basic_info();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < packet_num; i++)
	{
		ordersketch.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	//cout << "Run time: " << (double)(end_tcm - start_tcm) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of OrderSketch (insert): %.6lf Mips\n", insert_throughout);

	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			ordersketch.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of OrderSketch (query): %.6lf Mips\n\n", query_throughout);

	tot_aae = 0;
	tot_are = 0;
	ofstream fout_order;
	fout_order.open("output_ordersketch.txt");
	for (auto itr : benchmark_data)
	{
		report_val = ordersketch.query(itr.first);
		fout_order<<itr.first<<": "<<"estimation: "<<report_val<<"  actual value  "<<itr.second<<endl;

		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
//		cout<<"tot_aae:\t"<<tot_aae<<endl;
	}
	res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"OS"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fOrderSketch AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fOrderSketch ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"*****************************************************"<< endl;

	/***************************************Count Sketch****************************/
	auto countSketch = COUNTSketch<FREE_MEMORY , KEY_LEN, HASH_NUM, SINGLE_COUNTER_SIZE>();
//	countSketch.print_basic_info();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < packet_num; i++)
	{
		countSketch.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);

//	cout << "Run time: " << (double)(end_tcm - start_tcm) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of CountSketch (insert): %.6lf Mips\n", insert_throughout);

	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			countSketch.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of CountSketch (query): %.6lf Mips\n\n", query_throughout);

	tot_aae = 0;
	tot_are = 0;
	ofstream fout_cs;
	fout_cs.open("output_count.txt");
	for (auto itr : benchmark_data)
	{
		report_val = countSketch.query(itr.first);
		//report_val_tcm = countSketch,query(itr.first)
		fout_cs << itr.first << ": " << "estimation: " << report_val << "  actual value  " << itr.second << endl;
		tot_aae += abs(report_val- itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}
	res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"Count"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fCountSketch AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fCountSketch ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"***************************************************"<< endl;


	/*******************************Pyramid Sketch**********************/
	auto pSketch = PCMSketch<FREE_MEMORY, KEY_LEN,HASH_NUM,SINGLE_COUNTER_SIZE>();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < packet_num; i++)
	{
		pSketch.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	//	cout << "Run time: " << (double)(end_tcm - start_tcm) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of PyramidSketch (insert): %.6lf Mips\n", insert_throughout);

	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			pSketch.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of PyramidSketch (query): %.6lf Mips\n\n", query_throughout);

	tot_aae = 0;
	tot_are = 0;
	ofstream fout_p;
	fout_p.open("output_pyramid.txt");
	for (auto itr : benchmark_data)
	{
		report_val = pSketch.query(itr.first);
		//report_val_tcm = countSketch,query(itr.first)
		fout_p << itr.first << ": " << "estimation: " << report_val << "  actual value  " << itr.second << endl;
		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}
    res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"Pyramid"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fPyramidSketch AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fPyramidSketch ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"***********************************************************"<< endl;

	/***************************CF-CM********************************/

	auto cf_cu = StreamClassfier<FREE_MEMORY, KEY_LEN, HASH_NUM, SINGLE_COUNTER_SIZE>();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < packet_num; i++)
	{
		cf_cu.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	//	cout << "Run time: " << (double)(end_tcm - start_tcm) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of ColdFilter (insert): %.6lf Mips\n", insert_throughout);

	//speed of query 
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			cf_cu.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of ColdFilter (query): %.6lf Mips\n\n", query_throughout);

	tot_aae = 0;
	tot_are = 0;
	ofstream fout_cfcu;
	fout_cfcu.open("output_cfcu.txt");
	for (auto itr : benchmark_data)
	{
		 report_val = cf_cu.query(itr.first);
		//report_val_tcm = countSketch,query(itr.first)
		fout_cfcu << itr.first << ": " << "estimation: " << report_val << "  actual value  " << itr.second << endl;
		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}
    res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"ColdFilter"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fColdFilter AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fColdFilter ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"**********************************************************"<< endl;


		/***************************SIMPLE HASH********************************/
	auto sh = SimpleHash<FREE_MEMORY, KEY_LEN, SIMPLEHASH_HASH, SINGLE_COUNTER_SIZE>();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < packet_num; i++)
	{
		sh.insert(insert_data[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	//	cout << "Run time: " << (double)(end_tcm - start_tcm) / CLOCKS_PER_SEC << endl;
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	insert_throughout = (double)1000.0*packet_num / timediff;
	printf("throughput of SimpleHash (insert): %.6lf Mips\n", insert_throughout);

	//speed of query
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < test_time; t++)
	{
		for (auto itr : benchmark_data)
		{
			sh.query(itr.first);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
	query_throughout = (double)1000.0*test_time*benchmark_data.size() / timediff;
	printf("throughput of SimpleHash (query): %.6lf Mips\n\n", query_throughout);

	tot_aae = 0;
	tot_are = 0;
	ofstream fout_sh;
	fout_sh.open("output_sh.txt");
	for (auto itr : benchmark_data)
	{
		report_val = sh.query(itr.first);
		fout_sh << itr.first << ": " << "estimation: " << report_val << "  actual value  " << itr.second << endl;
		tot_aae += abs(report_val - itr.second);
		tot_are += abs((report_val - itr.second) / itr.second);
	}
	res_aae = double(tot_aae) / benchmark_data.size();
	res_are = double(tot_are) / benchmark_data.size();
	outFile<<FREE_MEMORY/1024<<","<<"SH"<<","<<res_aae<<","<<res_are<<","<<insert_throughout<<","<<query_throughout<<endl;

	printf("\fSimpleHash AAE:%lf\n", double(tot_aae) / benchmark_data.size());
	printf("\fSimpleHash ARE:%lf\n", double(tot_are) / benchmark_data.size());
	cout <<"**********************************************************"<< endl;

	
}
