#include "countSketch_new.h"
#include <queue>
#include <cstring>
#define AREA_NUM 4

CountSketch_new::CountSketch_new(int tot_mem_in_bytes, int d, int window_size){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(uint16_t);
    this->window_size = window_size;
    this->sequence = 0;
    this->epoches = new uint16_t[AREA_NUM];
    this->counters = new int16_t*[AREA_NUM];

    for(int i = 0; i < AREA_NUM; i++){
        counters[i] = new int16_t[width * d];
        memset(counters[i], 0, sizeof(uint16_t) * width * d); 
    }
    memset(epoches, 0 ,sizeof(uint16_t) * AREA_NUM);
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

    s_hash = new BOBHash32[d];
    for(int i = 0; i < d; ++i)
        s_hash[i].initialize(50 + i);

}

CountSketch_new::~CountSketch_new(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
    delete[] epoches;
    delete[] s_hash;
}

void CountSketch_new::clear() { 
    sequence = 0;
}

int CountSketch_new::insert(uint8_t *key, int keylen) {
    uint16_t epoch_seq = sequence / window_size;  
    int area_pos = epoch_seq % AREA_NUM; 
    if(epoches[area_pos] != epoch_seq){
        epoches[area_pos] = epoch_seq;
        memset(counters[area_pos], 0, sizeof(uint16_t) * width * d); 
    }
    for(int i = 0; i < d; ++i){   
        int s = s_hash[i].run((const char*)key, keylen) % 2;
        s = s == 0 ? -1 : s;
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        counters[area_pos][pos] += s;
    }
    return ++sequence;  
}

int CountSketch_new::query(uint8_t* key, int keylen){
    return query_epoch(key, keylen, 4);
}


int CountSketch_new::query_epoch(uint8_t *key, int keylen, int time){
    priority_queue<int, vector<int>, less<int>> queMin;
    priority_queue<int, vector<int>, greater<int>> queMax;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
         int s = s_hash[i].run((const char*)key, keylen) % 2;
        s = s == 0 ? -1 : s;
        int count = get_count(pos, time) * s;
        if(queMin.empty() || count <= queMin.top()){
            queMin.push(count);
            if(queMax.size() + 1 < queMin.size()){
                queMax.push(queMin.top());
                queMin.pop();
            }
        }else{
            queMax.push(count);
            if(queMin.size() < queMax.size()){
                queMin.push(queMax.top());
                queMax.pop();
            }
        }
    }
    return queMin.size() == queMax.size() ? (queMin.top() + queMax.top()) / 2 : queMin.top();
}


int CountSketch_new::get_count(int pos, int time){
    int res = 0;
    int epoch_seq = sequence / window_size;
    for(int i = 0; i < AREA_NUM; i++){
        if(epoches[i] >= epoch_seq - time)
            res += counters[i][pos];
    }
    return res;
}