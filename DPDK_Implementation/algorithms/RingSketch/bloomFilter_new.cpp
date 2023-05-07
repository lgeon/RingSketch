#include "bloomFilter_new.h"
#include <queue>
#include <cstring>
#define AREA_NUM 4

BloomFilter_new::BloomFilter_new(int tot_mem_in_bytes, int d, int window_size){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / sizeof(bool);
    this->window_size = window_size;
    this->sequence = 0;
    this->epoches = new uint16_t[AREA_NUM];
    this->counters = new bool*[AREA_NUM];

    for(int i = 0; i < AREA_NUM; i++){
        counters[i] = new bool[width];
        memset(counters[i], 0, sizeof(bool) * width); 
    }
    memset(epoches, 0 ,sizeof(uint16_t) * AREA_NUM);
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

}

BloomFilter_new::~BloomFilter_new(){
    delete[] counters;
    delete[] hash;
    delete[] epoches;
}

void BloomFilter_new::clear() { 
    sequence = 0;
}

int BloomFilter_new::insert(uint8_t *key, int keylen) {
    uint16_t epoch_seq = sequence / window_size;  
    int area_pos = epoch_seq % AREA_NUM; 
    if(epoches[area_pos] != epoch_seq){
        epoches[area_pos] = epoch_seq;
        memset(counters[area_pos], 0, sizeof(bool) * width); 
    }
    for(int i = 0; i < d; ++i){   
        int pos = hash[i].run((const char*)key, keylen) % width;
        counters[area_pos][pos] = true;
    }
    return ++sequence;  //返回插入次数
}

int BloomFilter_new::query(uint8_t* key, int keylen){
    return query_epoch(key, keylen, 4);
}


int BloomFilter_new::query_epoch(uint8_t *key, int keylen, int time){
    int* position = new int[d];
    for(int i = 0; i < d; ++i)
    {
        int pos = hash[i].run((const char*)key, keylen) % width;
        position[i] = pos;
    }

    bool res = false;
    uint16_t epoch_seq = sequence / window_size;  
    for(int i = 0; i < AREA_NUM; i++){
        if(epoches[i] >= epoch_seq - time){
            bool tmp = true;
            for(int j = 0; j < d; j++)
                tmp &= counters[i][position[j]];
            res |= tmp;
        }
    }
    return res ? 1 : 0;
}












