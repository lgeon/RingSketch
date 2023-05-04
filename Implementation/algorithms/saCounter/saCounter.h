#ifndef _SA_COUNTER_H_
#define _SA_COUNTER_H_

#include <map>
#include "../BOBHash32.h"
#include "../algorithms.h"

using std::unordered_map;

class saCounter: public Algorithm
{
    uint8_t *counters;
    uint32_t *recoverCounters;
    uint32_t *tmpRover; // 中间态counter, 只还原，不减去大流
    unordered_map<string, int> elephant_flow;
    int width;
    int d;
    BOBHash32 *hash;
public:
    saCounter(int tot_mem_in_bytes, int d);
    ~saCounter();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    void recover();
    void clear();
    void get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq);
    void insert_elephant(uint8_t* key, int size, int keylen);
    int get_flowquantity();
    void get_heavy_hitters(int threshold, vector<pair<string, int>> & results);
};


#endif //_SA_COUNTER_H_