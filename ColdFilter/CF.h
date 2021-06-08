//autually, CF part also can divided into two part.
//Agg-Evict aggerate and update ,which can reduce insert consume??
//Layer1 record cold item, 
//Layer2 record hot item

#ifndef _CF_H
#define _CF_H

//#include "BOBHash32.h"
#include"../common/param.h"
#include"../common/BOBHash32.h"
#include <immintrin.h> //SSE2 instruction set
#include<algorithm>

//#ifdef UNIX
//#include <x86intrin.h>
//#else
//#include <intrin.h>
//#endif
using namespace std;

#define BUCKET_NUM 16
#define COUNTER_NUM 16
#define AR_COUNTER_SIZE 8//4 bytes for key,4bytes for value,so this setting only suit for 4bytes key
#define LAYER1_RATIO 65
#define LAYER1_COUNTER_SIZE 4//£¨£©bit

//template<int memory_in_bytes, int bucket_num = BUCKET_NUM, int counter_num = COUNTER_NUM, int threshold = 240,int layer1_ratio =  65>
template<int memory_in_byte, int key_len,int d,int single_counter_size =4>
class StreamClassfier
{
	static constexpr int buffer_size = BUCKET_NUM * COUNTER_NUM * AR_COUNTER_SIZE;
	static constexpr int remained = memory_in_byte - buffer_size;
	static constexpr int d1 = d;
	static constexpr int d2 = d;
	static constexpr int m1_in_bytes = int(remained * LAYER1_RATIO / 100.0);
	static constexpr int m2_in_bytes = int(remained*(100 - LAYER1_RATIO) / 100.0);

	uint32_t ID[BUCKET_NUM][COUNTER_NUM] __attribute__((aligned(16)));
	int counter[BUCKET_NUM][COUNTER_NUM];
	int cur_pos[BUCKET_NUM];

	
	static constexpr int w1 = m1_in_bytes * 8 / (LAYER1_COUNTER_SIZE);//layer1: the size of single counter is 4bits;
	static constexpr int w_word = m1_in_bytes * 8 / 4 / 16;
	static constexpr int w2 = m2_in_bytes * 8 /(single_counter_size*8-LAYER1_COUNTER_SIZE);//layer2: the size of counter in layer2 is 16 bits;

	uint64_t L1[m1_in_bytes * 8 / 4 / 16];// layer1 is organized as word ,one word contains 16 counters;
	uint64_t L2[m2_in_bytes * 8 / 16];

	BOBHash32 *bobhash1;//hash in layer1, one hash key is 32bit, 16 bit find location bucket, 
	BOBHash32 *bobhash2[d2];

	int cur_kick;//record eviction location
	int threshold = 1<<30;

	void insert_SC(uint32_t kick_ID, int kick_f)// 
	{
		int v1 = 1 << 30;

		int value[d1];
		int index[d1];
		int offset[d1];
		uint64_t hash_value = (bobhash1->run((const char *)&kick_ID, 4));
		int word_index = hash_value % w_word;//in layer1, find word 

		hash_value >>= 16;//last 16 bits for word position

						  //find d1 positions in word, and get the min value
		uint64_t temp = L1[word_index];
		for (int i = 0; i < d1; i++) {
			offset[i] = (hash_value * 0xF);
			value[i] = (temp >> (offset[i] << 2)) & 0xF;
			v1 = min(v1, value[i]);
			hash_value >>= 4;
		}

		int temp2 = v1 + kick_f;
		//if flow num still can store in Layer1,then add in layer1
		//only addthe smallest value
		if (temp2 <= 15) {
			for (int i = 0; i < d1; i++) {
				int temp3 = ((temp >> (offset[i] << 2)) & 0xF);
				if (temp3 < temp2) {
					temp += ((temp2 - temp3) << (offset[i] << 2));
				}
			}
			L1[word_index] = temp;
			return;
		}

		//flow num exceed counter in Layer1
		//first we modify all position as threshold in layer1
		for (int i = 0; i < d1; i++) {
			temp |= ((uint64_t)0xF << (offset[i] << 2));
		}
		L1[word_index] = temp;

		//second add rest part in layer2
		kick_f -= (15 - v1);//(15-v1) add in layer1, else add in layer2
		int v2 = 1 << 30;
		for (int i = 0; i < d2; i++) {
			index[i] = (bobhash2[i]->run((const char*)&kick_ID, 4)) % w2;
			value[i] = L2[index[i]];
			v2 = min(value[i], v2);
		}

		//if flow not exceed threshold
		temp2 = v2 + kick_f;
		if (temp2 <= threshold) {
			for (int i = 0; i < d2; i++) {
				L2[index[i]] = max(L2[index[i]], (uint64_t)temp2);//i have
														/*---------CM is only add in the smallest value,but max(a,b) for every position is another algorithm!--------*/
			}
		}
		//else flow excedd threshold
		for (int i = 0; i < d2; i++) {
			L2[index[i]] = threshold;
		}

	}

	void insert_SC2(uint32_t kick_ID, int kick_f)
	{
		int v1 = 1 << 30;
		int MAX_HASH_NUM = 3;
		int value[MAX_HASH_NUM];
		int index[MAX_HASH_NUM];
		int offset[MAX_HASH_NUM];

		uint64_t hash_value = (bobhash1->run((const char *)&kick_ID, 4));
		int word_index = hash_value % w_word;
		hash_value >>= 16;

		uint64_t temp = L1[word_index];
		for (int i = 0; i < d1; i++) {
			offset[i] = (hash_value & 0xF);
			value[i] = (temp >> (offset[i] << 2)) & 0xF;
			v1 = std::min(v1, value[i]);

			hash_value >>= 4;
		}

		int temp2 = v1 + kick_f;
		if (temp2 <= 15) { // maybe optimized use SIMD?
			for (int i = 0; i < d1; i++) {
				int temp3 = ((temp >> (offset[i] << 2)) & 0xF);
				if (temp3 < temp2) {
					temp += ((uint64)(temp2 - temp3) << (offset[i] << 2));
				}
			}
			L1[word_index] = temp;
			return;
		}

		for (int i = 0; i < d1; i++) {
			temp |= ((uint64_t)0xF << (offset[i] << 2));
		}
		L1[word_index] = temp;

		int delta1 = 15 - v1;
		kick_f -= delta1;

		int v2 = 1 << 30;
		for (int i = 0; i < d2; i++) {
			index[i] = (bobhash2[i]->run((const char *)&kick_ID, 4)) % w2;
			value[i] = L2[index[i]];
			v2 = std::min(value[i], v2);
		}

		temp2 = v2 + kick_f;
		if (temp2 <= threshold) {
			for (int i = 0; i < d2; i++) {
				L2[index[i]] = (L2[index[i]] > temp2) ? L2[index[i]] : temp2;
			}
			return;
		}

		for (int i = 0; i < d2; i++) {
			L2[index[i]] = threshold;
		}

//		int delta2 = threshold - v2;
//		kick_f -= delta2;

//		spa->insert(kick_ID, kick_f);
	}
public:
	StreamClassfier()
	{
		bobhash1 = new BOBHash32(500);
		for (int i = 0; i < d2; i++) {
			bobhash2[i] = new BOBHash32(1000 + i);
		}
		cur_kick = 0;
		memset(ID, 0, sizeof(ID));
		memset(counter, 0, sizeof(counter));
		memset(cur_pos, 0, sizeof(cur_pos));
		memset(L1, 0, sizeof(L1));
		memset(L2, 0, sizeof(L2));
	}
	~StreamClassfier()
	{
		delete bobhash1;
		for (int i = 0; i < d2; i++) {
			delete bobhash2[i];
		}
	}

	void refresh()
	{
		for (int i = 0; i < BUCKET_NUM; i++)
		{
			for (int j = 0; j < COUNTER_NUM; j++) {
				insert_SC2(ID[i][j], counter[i][j]);
				cout<<ID[i][j]<<":"<<counter[i][j]<<endl;
				ID[i][j] = 0;
				counter[i][j] = 0;
		//	cout<<ID[i][j]<<":"<<counter[i][j]<<endl;
			}
			cur_pos[i] = 0;
		}
	}
	void print_basic_info()
	{
		printf("Cold Filter\n");
		printf("\tSIMD buffer:%d counters,%.4lf MB occupies\n", BUCKET_NUM*COUNTER_NUM, BUCKET_NUM*COUNTER_NUM*8.0 / 1024 / 1024);
		printf("\tL1:%d counters, %.4lf MB occupies\n", w1, w1 * 4.0 / 8 / 1024 / 1024);
		printf("\tL2:%d counters, %.4lf MB occupies\n", w2, w2 * 16.0 / 8 / 1024 / 1024);
	}

	void insert(uint32_t key)
	{
		int bucket_id = key % BUCKET_NUM;//simple hash ...
		const __m128i item = _mm_set1_epi32((int)key);//fill 128 bits register by key

		int matched;

		if (COUNTER_NUM == 16) {//assume counter number is 16 per buckets
			__m128i *keys_p = (__m128i *)ID[bucket_id];//ID is 32bits ,if 16 counter per buckets, we need 4th oompare 4*128 = 32*16

			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);//compare whether it's equal, if equal, return 1 in that position,else 0
			__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
			__m128i c_comp = _mm_cmpeq_epi32(item, keys_p[2]);
			__m128i d_comp = _mm_cmpeq_epi32(item, keys_p[3]);

			a_comp = _mm_packs_epi32(a_comp, b_comp);//compress 32bits to 16 bits 
			c_comp = _mm_packs_epi32(c_comp, d_comp);
			a_comp = _mm_packs_epi32(a_comp, c_comp);

			matched = _mm_movemask_epi8(a_comp);
		}
		else if (COUNTER_NUM == 4) {
			__m128i *keys_p = (__m128i *)ID[bucket_id];
			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
			matched = _mm_movemask_ps(*(__m128 *)&a_comp);
		}
		else {
			throw std::logic_error("Not implemented SIMD.");
		}

		if (matched != 0) {// if matched
			int matched_index = _tzcnt_u32((uint32_t)matched);//_tzcnt_u32 get the position
			++counter[bucket_id][matched_index];
			return;
		}
		// not find in Agg-Evict part
		//if we have empty counter;
		int cur_pos_now = cur_pos[bucket_id];
		if (cur_pos_now != COUNTER_NUM) {//have empty counter
			ID[bucket_id][cur_pos_now] = key;
			++counter[bucket_id][cur_pos_now];
			++cur_pos[bucket_id];
			return;
		}
		// else , we don't have empty counter;
		//randonly choose one counter to kick
		insert_SC2(ID[bucket_id][cur_kick], counter[bucket_id][cur_kick]);
/*********************************************************************************************/
	//	cout<<ID[bucket_id][cur_kick]<<":"<<counter[bucket_id][cur_kick]<<endl;

		ID[bucket_id][cur_kick] = key;
		counter[bucket_id][cur_kick] = 1;
		cur_kick = (cur_kick + 1) % COUNTER_NUM;

	}

	

	int query(uint32_t key)
	{
		int v1 = 1 << 30;
		//query contains two parts, first part in layer1,if layer1 exceed threshold ,we need 
		//query = query_in_layer1 + query_in_layer2
		uint32_t hash_value = (bobhash1->run((const char *)&key, 4));
		int word_index = hash_value%w_word;
		hash_value >>= 16;

		uint64_t temp = L1[word_index];
		for (int i = 0; i < d1; i++) {
			int offset, val;
			offset = (hash_value & 0xF);
			val = temp >> (offset << 2) & 0xF;
			v1 = min(v1, val);
			hash_value >>= 4;
		}

		if (v1 != 15) {
			return v1;
		}

		int v2 = 1 << 30;
		for (int i = 0; i < d2; i++) {
			int index = (bobhash2[i]->run((const char *)&key, 4)) % w2;
			int value = L2[index];
			v2 = min(value, v2);
		}

		return v1 + v2;
	}
};
#endif // !_SC_H
