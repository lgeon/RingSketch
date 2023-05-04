#include "cuSketch_new.h"
#include <queue>
#include <cstring>
#define AREA_NUM 4

CUSketch_new::CUSketch_new(int tot_mem_in_bytes, int d, int window_size){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(uint16_t);
    this->window_size = window_size;
    this->sequence = 0;
    this->epoches = new uint16_t[AREA_NUM];
    this->counters = new uint16_t*[AREA_NUM];

    for(int i = 0; i < AREA_NUM; i++){
        counters[i] = new uint16_t[width * d];
        memset(counters[i], 0, sizeof(uint16_t) * width * d); 
    }
    memset(epoches, 0 ,sizeof(uint16_t) * AREA_NUM);
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

}

CUSketch_new::~CUSketch_new(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
    delete[] epoches;
}

void CUSketch_new::clear() { 
    sequence = 0;
}

int CUSketch_new::insert(uint8_t *key, int keylen) {
    uint16_t min = 0x7FFF;
    uint16_t epoch_seq = sequence / window_size;  
    int area_pos = epoch_seq % AREA_NUM; 
    int** val = new int*[d];
    if(epoches[area_pos] != epoch_seq){
        epoches[area_pos] = epoch_seq;
        memset(counters[area_pos], 0, sizeof(uint16_t) * width * d); 
        min = 0;
    }
    for(int i = 0; i < d; ++i){   
        val[i] = new int[2];
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int16_t cur_num = counters[area_pos][pos];
        min = cur_num < min ? cur_num : min;
        val[i][0] = cur_num;
        val[i][1] = pos;
    }

    for(int i = 0; i < d; i++){
        if(val[i][0] == min){
            counters[area_pos][val[i][1]] = min + 1; 
        }
    }
    return ++sequence;  
}

int CUSketch_new::query(uint8_t* key, int keylen){
    return query_epoch(key, keylen, 4);
}


int CUSketch_new::query_epoch(uint8_t *key, int keylen, int time){
    int ret = 0x7FFFFFFF;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int tmp = get_count(pos, time);
        ret = ret > tmp ? tmp : ret;
    }
    return ret;
}


int CUSketch_new::get_count(int pos, int time){
    int res = 0;
    int epoch_seq = sequence / window_size;
    for(int i = 0; i < AREA_NUM; i++){
        if(epoches[i] >= epoch_seq - time)
            res += counters[i][pos];
    }
    return res;
}