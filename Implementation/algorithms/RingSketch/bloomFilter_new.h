#include "../BOBHash32.h"

class BloomFilter_new
{
    bool **counters;
    int width;
    int d;
    BOBHash32 *hash;
    int window_size;
    uint16_t* epoches;
    int sequence;


public:
    BloomFilter_new(int tot_mem_in_bytes, int d, int window_size);
    ~BloomFilter_new();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    int query_epoch(uint8_t *key, int keylen, int time);
    void clear();


};