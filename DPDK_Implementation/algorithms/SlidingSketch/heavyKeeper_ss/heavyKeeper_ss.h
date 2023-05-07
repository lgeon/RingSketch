#include "../BOBHash32.h"
#include "ss_node.h"


class heavyKeeper_ss
{
    ss_node **counters;
    int width;
    int d;
    int day;
    BOBHash32 *hash;
    int window_size;
    int step;
    int index;
    int sequence;
    int cur;




public:
    heavyKeeper_ss(int tot_mem_in_bytes, int d, int window_size, int day);
    ~heavyKeeper_ss();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    void clear();


};