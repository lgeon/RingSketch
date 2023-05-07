// #ifndef MYSKETCH_SLIDINGWINDOW_H
// #define MYSKETCH_SLIDINGWINDOW_H
#include "../BOBHash32.h"
#include "../algorithms.h"

class countSketch_ss: public Algorithm
{
    int64_t **counters;
    int width;
    int d;
    int day;
    BOBHash32 *hash;
    BOBHash32 *s_hash;
    int window_size;
    int step;
    int index;
    int cur;




public:
    countSketch_ss(int tot_mem_in_bytes, int d, int window_size, int day);
    ~countSketch_ss();

    int insert(uint8_t *key, int keylen);
    int query(uint8_t *key, int keylen);
    void clear();
    void get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq);
    int get_flowquantity();
    void get_heavy_hitters(int threshold, vector<pair<string, int>> & results);

private:
    int get_count(int pos);
    void c_update(uint8_t* key, int ket_len);

};