#include "heavyKeeper_v3.h"
#include <queue>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cmath>
#define AREA_NUM 4

HeavyKeeper_v3::HeavyKeeper_v3(int tot_mem_in_bytes, int d, int window_size){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(hk_node);
    this->window_size = window_size;
    this->sequence = 0;
    this->epoches = new uint16_t[AREA_NUM];
    this->counters = new hk_node*[AREA_NUM];
    this->k = 8;

    for(int i = 0; i < AREA_NUM; i++){
        counters[i] = new hk_node[width * d];
        memset(counters[i], 0, sizeof(hk_node) * width * d); 
    }
    memset(epoches, 0 ,sizeof(uint16_t) * AREA_NUM);
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数


     std::srand(std::time(nullptr));
}

HeavyKeeper_v3::~HeavyKeeper_v3(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
    delete[] epoches;
}

void HeavyKeeper_v3::clear() { 
    sequence = 0;
}

int HeavyKeeper_v3::insert(uint8_t *key, int keylen) {
    uint8_t* f = new uint8_t[keylen];
    memcpy(f, key, keylen);
    uint16_t epoch_seq = sequence / window_size;  
    int area_pos = epoch_seq % AREA_NUM; 
    if(epoches[area_pos] != epoch_seq){
        epoches[area_pos] = epoch_seq;
        memset(counters[area_pos], 0, sizeof(hk_node) * width * d); 
    }
    for(int i = 0; i < d; ++i){   
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        hk_node* cur = &counters[area_pos][pos];
        if(cur->count != 0){
            if(memcmp(f, cur->flowkey, keylen) == 0){
                if(cur->count < 255)
                    cur->count++;
                else{
                    double probability = static_cast<double>(std::rand()) / RAND_MAX;
                    if(probability <= std::pow(2, -1 * k))
                        cur->count++;
                }
            }else{
                double probability = static_cast<double>(std::rand()) / RAND_MAX;
                if(probability <= std::pow(1.08, -1 * cur->count)){
                    if(cur->count < 255)
                        cur->count--;
                    else{
                        double probability = static_cast<double>(std::rand()) / RAND_MAX;
                        if(probability <= std::pow(2, -1 * k))
                            cur->count--;
                    }

                }
            }
        }
        if(cur->count == 0){
            cur->flowkey = f;
            cur->count = 1;
        }
    }

    return ++sequence;  //返回插入次数
}

int HeavyKeeper_v3::query(uint8_t* key, int keylen){
    return query_epoch(key, keylen, 4);
}


int HeavyKeeper_v3::query_epoch(uint8_t *key, int keylen, int time){
    uint8_t* f = new uint8_t[keylen];
    memcpy(f, key, keylen);
    int epoch_seq = sequence / window_size;
    int res = 0;
    int* position = new int[d];
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        position[i] = pos;
    }
    for(int i = 0; i < AREA_NUM; ++i)
    {   
        if(epoches[i] >= epoch_seq - time){
            int tmp = 0;
            for(int j = 0; j < d; j++){
                hk_node cur = counters[i][position[j]];
                if(cur.count != 0){
                    if(memcmp(f, cur.flowkey, keylen) == 0)
                        tmp = cur.count > tmp ? cur.count : tmp;
                }
            }
            if(tmp > 255)
                tmp = 255 + (tmp - 255) * static_cast<int>(std::pow(2, k));
            res += tmp;
        }
    }
    return res;
}


