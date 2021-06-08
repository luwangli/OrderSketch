//read data stream, return arrary

#ifndef _TRACEREAD_H
#define _TRACEREAD_H
#include "param.h"

using namespace std;

#define MAX_PACKET 32000000
typedef pair<uint32_t, int> KV;

uint32_t str_to_dec(string str)
{
    uint32_t res = 0;
    for (uint32_t i=0; i <str.length(); i++)
    {
        if(str[i]>='0' && str[i]<='9')
            res = res *10 + str[i] - '0';
    }
    return res;
}


int TraceRead(char const *filename, uint32_t *datastream, unordered_map<uint32_t, int> &benchmark)
{
    FILE *Fin = fopen(filename,"rb");
    if(!Fin)
    {
        cerr << "cannot open file:" << filename <<endl;
        exit(-1);
    }

    ifstream fin(filename);
    string line;
    uint32_t res_dec;
    int count = 0;
    while (getline(fin, line)){
 //       cout << line <<endl;
        res_dec = str_to_dec(line);
 //       cout << res_dec <<endl;
        datastream[count] = res_dec;
        benchmark[res_dec]++;
        count++;
        if (count == MAX_PACKET ){
            cout << "reach max packet number" <<endl;
            break;
        }
    }
    fin.clear();
    fin.close();
    int flow_num = benchmark.size();
    cout << "Successfully read " << filename << endl;
    cout<<"******************basic information********************"<<endl;
    cout<<"flow number: "<<flow_num<<"\t packet number: "<<count<<endl;
    return count;

}

#endif
