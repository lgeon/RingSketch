#include "../BOBHash32.h"
#include "../algorithms.h"

class SlidingWindow: public Algorithm
{
    uint64_t *counters;
    uint64_t *timers;
    int width;
    int d;
    BOBHash32 *hash;
    BOBHash32 *s_hash;
    int sequence;
    uint64_t *areas;
    int* offsets;
    int area_num;
    int window_size;



public:
    SlidingWindow(int tot_mem_in_bytes, int d, int window_size);
    ~SlidingWindow();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    int query_epoch(uint8_t *key, int keylen, int time);
    void clear();
    void get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq);
    void get_flowsize_epoch(vector<string> &flowIDs, unordered_map<string,int> &freq, int time);
    int get_flowquantity();
    void get_heavy_hitters(int threshold, vector<pair<string, int>> & results);


private:
    int get_count(int pos, int time);
    void cm_update(uint8_t* key, int key_len);
    void cu_update(uint8_t* key, int key_len);
};


