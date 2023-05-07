#include "slidingSketch.h"
#include <queue>

SlidingSketch::SlidingSketch(int tot_mem_in_bytes, int d, int window_size, int day) : Algorithm("slidingSketch"){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(uint64_t);
    this->window_size = window_size;
    this->day = day;
    this->step = width * d / window_size * (day - 1); // width  > windowSize
    this->index = 0;
    this->cur = 0;
    printf("step: %d\n", step);
    fflush(stdout);

    counters = new uint64_t*[width * d];
    for(int i = 0; i < width * d; i++){
        counters[i] = new uint64_t[day];
        memset(counters[i], 0, sizeof(uint64_t) * day); 
    }

    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

    s_hash = new BOBHash32[d];
    for(int i = 0; i < d; i++)
        s_hash[i].initialize(50 + i);

}

SlidingSketch::~SlidingSketch(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
    delete[] s_hash;
}

void SlidingSketch::clear() {
    insertTimes = 0;
    for(int i = 0; i < width * d; i++){
        memset(counters[i], 0, sizeof(uint64_t) * day); 
    }

}

int SlidingSketch::insert(uint8_t *key, int keylen) {
   
    for(int i = 0; i < step; i++){
        uint64_t* bucket = counters[index];
        for(int j = day - 1; j >= 1; j--)
            bucket[j] = bucket[j - 1];
        bucket[0] = 0;
        index = (index + 1) % (width * d);
    }
    cu_update(key, keylen);
    return ++insertTimes;  //返回插入次数
}

void SlidingSketch::cm_update(uint8_t* key, int keylen){
    for(int i = 0; i < d; ++i){   
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        counters[pos][0]++;
    }
}
void SlidingSketch::cu_update(uint8_t* key, int keylen){
    uint64_t min = 0x7fffffff;
    uint64_t** vals = new uint64_t*[d];
    for(int i = 0; i < d; ++i)
    {
        vals[i] = new uint64_t[2];
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        uint64_t count = counters[pos][0];
        vals[i][0] = count;
        vals[i][1] = pos;
        min = count < min ? count : min; 
    }

    for(int i = 0; i < d; i++){
        if(vals[i][0] == min)
            counters[vals[i][1]][0] = min + 1;
    }
}


int SlidingSketch::query(uint8_t *key, int keylen){
    int ret = 0x7FFFFFFF;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int tmp = get_count(pos);
        ret = ret > tmp ? tmp : ret;
    }
    return ret;
}


int SlidingSketch::get_count(int pos){
    int res = 0;
    
    for(int i = 0; i < day; i++)
    {
        res += counters[pos][i];
    }
    return res;
}

void SlidingSketch::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq){}

int SlidingSketch::get_flowquantity(){return 0;}

void SlidingSketch::get_heavy_hitters(int threshold, vector<pair<string, int>> & results){}