# include "hk_node.h"
#include "../BOBHash32.h"


class HeavyKeeper_new
{
    hk_node **counters;
    int width;
    int d;
    BOBHash32 *hash;
    int window_size;
    uint16_t* epoches;
    int sequence;


public:
    HeavyKeeper_new(int tot_mem_in_bytes, int d, int window_size);
    ~HeavyKeeper_new();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    int query_epoch(uint8_t *key, int keylen, int time);
    void clear();


};