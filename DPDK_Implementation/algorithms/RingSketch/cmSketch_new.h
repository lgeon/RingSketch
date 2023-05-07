#include "../BOBHash32.h"

class CMSketch_new
{
    uint16_t **counters;
    int width;
    int d;
    BOBHash32 *hash;
    int window_size;
    uint16_t* epoches;
    int sequence;


public:
    CMSketch_new(int tot_mem_in_bytes, int d, int window_size);
    ~CMSketch_new();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    int query_epoch(uint8_t *key, int keylen, int time);
    void clear();

private:
    int get_count(int pos, int time);

};