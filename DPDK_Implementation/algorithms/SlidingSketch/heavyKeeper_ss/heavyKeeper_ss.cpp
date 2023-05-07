#include <queue>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "heavyKeeper_ss.h"

heavyKeeper_ss::heavyKeeper_ss(int tot_mem_in_bytes, int d, int window_size, int day){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(ss_node);
    this->window_size = window_size;
    this->sequence = 0;
    this->counters = new ss_node*[width * d];
    this->step = window_size / width / d / (day - 1); //windowsize > width
    this->index = 0;
    this->cur = 0;
    this->day = day;

    printf("%d\n", step);
    fflush(stdout);
    for(int i = 0; i < width * d; i++)
        counters[i] = new ss_node[day];
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

     std::srand(std::time(nullptr));
}

heavyKeeper_ss::~heavyKeeper_ss(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
}

void heavyKeeper_ss::clear() { 
    sequence = 0;
}

int heavyKeeper_ss::insert(uint8_t *key, int keylen) {
    uint8_t* f = new uint8_t[keylen / 8];
    memcpy(f, key, keylen / 8);

    for(int i = 0; i < d; ++i){   
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        ss_node* cur = &counters[pos][0];
        if(cur->count != 0){
            if(memcmp(f, cur->flowkey, keylen / 8) == 0)
                cur->count++;
            else{
                double probability = static_cast<double>(std::rand()) / RAND_MAX;
                if(probability <= std::pow(1.08, -1 * cur->count))
                    cur->count--;
            }
        }
        if(cur->count == 0){
            cur->flowkey = f;
            cur->count = 1;
        }
    }

    cur++;
    if(cur == step){
        cur = 0;
        for(int i = 0; i < step; i++){
            ss_node* bucket = counters[index];
            for(int j = day - 1; j >= 1; j--)
                bucket[j] = bucket[j - 1];
            memset(&bucket[0], 0, sizeof(ss_node));
            
        }
        index = (index + 1) % (width * d);   
    }
    // printf("cur: %d index: %d\n", cur, index);
    // fflush(stdout);
    return ++sequence;  //返回插入次数
}

int heavyKeeper_ss::query(uint8_t* key, int keylen){
    uint8_t* f = new uint8_t[keylen / 8];
    memcpy(f, key, keylen / 8);
    int res = 0;
    for(int i = 0; i < d; ++i)
    {   
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int tmp = 0;
        for(int j = 0; j < day; j++){
            ss_node cur = counters[pos][j];
            if(cur.count != 0){
                if(memcmp(f, cur.flowkey, keylen / 8) == 0){
                    tmp += cur.count;
                }
            }
                    

        }   

        res = tmp > res ? tmp : res;
    }
    return res;
}


