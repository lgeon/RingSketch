/*
    预处理流
*/
#ifndef _PRE_COUNT_H_
#define _PRE_COUNT_H_

#include "param.h"

template <int bucket_num>
class PreCount
{
    alignas(64) Bucket buckets[bucket_num];  // ??? alignas
public:
    PreCount();
    ~PreCount();

    void clear();

    int insert(uint8_t *key, int timestamp, uint8_t *swap_key, uint32_t &swap_val, int keylen, uint32_t f = 1);
    int quick_insert(uint8_t *key, uint32_t f = 1);

    int query(uint8_t *key);

    int get_memory_usage();
    int get_bucket_num();
private:
    int CalculateFP(uint8_t *key, uint32_t &fp);
};

#endif //_PRE_COUNT_H_