#include "bloomFilter_ss.h"
#include <queue>
#include <cstring>

bloomFilter_ss::bloomFilter_ss(int tot_mem_in_bytes, int d, int window_size, int day){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / sizeof(bool);
    this->window_size = window_size;
    this->day = day;
    this->step = width / window_size *  (day - 1); // width  > windowSize
    this->index = 0;
    this->sequence = 0;

    printf("%d", step);
    fflush(stdout);
    counters = new bool*[width];
    for(int i = 0; i < width; i++){
        counters[i] = new bool[day];
        memset(counters[i], 0, sizeof(bool) * day); 
    }

    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

}

bloomFilter_ss::~bloomFilter_ss(){
    delete[] counters;
    delete[] hash;

}

void bloomFilter_ss::clear() {
    for(int i = 0; i < width; i++){
        memset(counters[i], 0, sizeof(bool) * day); 
    }

}

int bloomFilter_ss::insert(uint8_t *key, int keylen) {
    for(int i = 0; i < d; ++i)
    {
        int pos = hash[i].run((const char*)key, keylen) % width;
        counters[pos][0] = true;
    }

    for(int i = 0; i < step; i++){
        bool* bucket = counters[index];
        for(int j = day - 1; j >= 1; j--)
            bucket[j] = bucket[j - 1];
        bucket[0] = false;
        index = (index + 1) % width;
    }

    return ++sequence;  //返回插入次数
}



int bloomFilter_ss::query(uint8_t* key, int keylen){
    int* position = new int[d];
    for(int i = 0; i < d; ++i)
    {
        int pos = hash[i].run((const char*)key, keylen) % width;
        position[i] = pos;
    }

    bool res = false;
    
    for(int i = 0; i < day; i++){
        bool tmp = true;
        for(int j = 0; j < d; j++){
            tmp &= counters[position[j]][i];
        }
        res |= tmp;
    }
    return res ? 1 : 0;
    
}
