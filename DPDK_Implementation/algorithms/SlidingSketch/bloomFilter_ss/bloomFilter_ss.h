#include "../BOBHash32.h"


class bloomFilter_ss
{
    bool **counters;
    int width;
    int d;
    int day;
    BOBHash32 *hash;
    int window_size;
    int step;
    int index;
    int sequence;





public:
    bloomFilter_ss(int tot_mem_in_bytes, int d, int window_size, int day);
    ~bloomFilter_ss();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    void clear();


};