#include "slidingWindow.h"
#include <queue>



SlidingWindow::SlidingWindow(int tot_mem_in_bytes, int d, int window_size): Algorithm("slidingWindow") {
    this->d = d;
    this->width = tot_mem_in_bytes / d / sizeof(uint64_t);
    this->sequence = 0;
    this->area_num = 4;
    this->offsets = new int [this->area_num];
    this->areas = new uint64_t [this->area_num];
    this->window_size = window_size;

    counters = new uint64_t[width * d];
    memset(counters, 0, sizeof(uint64_t) * width * d);  // 为counter分配空间
    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？


    timers = new uint64_t [width * d];
    memset(timers, 0, sizeof(uint64_t) * width * d);
    for(int i = 0; i < area_num; i++){
        uint64_t base = 0xffff;
        offsets[i] = i * (sizeof(uint64_t) / area_num) * 8;
        areas[i] = base << offsets[i];
    }

    //for Count sketch
    s_hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(50 + i);  // 初始化哈希函数？？？

}

SlidingWindow::~SlidingWindow()
{
    delete[] hash;
    delete[] counters;
    delete[] timers;
}

void SlidingWindow::clear() {
    insertTimes = 0;
    sequence = 0;
    memset(counters, 0, sizeof(uint64_t) * width * d);
    memset(timers, 0, sizeof(uint64_t) * width * d);
}

int SlidingWindow::insert(uint8_t *key, int keylen) {
    cm_update(key, keylen);
    this->sequence++;
    return sequence;  //返回插入次数
}

void SlidingWindow::cm_update(uint8_t* key, int keylen){ //CM Sketch
    for(int i = 0; i < d; ++i)  // 遍历d个哈希函数
    {   
        
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        uint64_t epoch_seq = sequence / window_size;  //小周期号
        int area_pos = epoch_seq % area_num; //区域位置
        uint64_t cur = (timers[pos] & areas[area_pos]) >> offsets[area_pos];  


        if(cur != epoch_seq){
            timers[pos] &= ~areas[area_pos];
            timers[pos] |= epoch_seq << offsets[area_pos];
            u_int64_t reset = 1;
            counters[pos] &= ~areas[area_pos];
            counters[pos] |= reset << offsets[area_pos];
        }else{
            uint64_t cur_num = (counters[pos] & areas[area_pos]) >> offsets[area_pos];
            cur_num++;
            counters[pos] &= ~areas[area_pos];
            counters[pos] |= cur_num << offsets[area_pos];
        }
    }
}


void SlidingWindow::cu_update(uint8_t* key, int keylen){ //CU Sketch
    uint64_t min = 0x7FFFFFFF;
    int** val = new int*[d];
    for(int i = 0; i < d; ++i)  // 遍历d个哈希函数
    {   
        val[i] = new int[3];
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        uint64_t epoch_seq = sequence / window_size;  //小周期号
        int area_pos = epoch_seq % area_num; //区域位置
        uint64_t cur = (timers[pos] & areas[area_pos]) >> offsets[area_pos];  

        if(cur != epoch_seq){            
            timers[pos] &= ~areas[area_pos];
            timers[pos] |= epoch_seq << offsets[area_pos];
            u_int64_t reset = 1;
            min = 0;
            counters[pos] &= ~areas[area_pos];
            counters[pos] |= reset << offsets[area_pos];
        }else{
            uint64_t cur_num = (counters[pos] & areas[area_pos]) >> offsets[area_pos];
            min = cur_num < min ? cur_num : min;
            val[i][0] = cur_num;
            val[i][1] = pos;
            val[i][2] = area_pos;
        }

    }

    for(int i = 0; i < d; i++){
        if(val[i][0] == min){
            counters[val[i][1]] &= ~areas[val[i][2]];
            counters[val[i][1]] |= (min + 1) << offsets[val[i][2]];
        }
    }


}

int SlidingWindow::query(uint8_t *key, int keylen){
    return query_epoch(key, keylen, 4);
}

int SlidingWindow::query_epoch(uint8_t *key, int keylen, int time) {
    int ret = 0x7FFFFFFF;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int count = get_count(pos, time);
        ret = ret > count ? count : ret;
    }
    return ret;
}

int SlidingWindow::get_count(int pos, int time) {
    int res = 0;
    int epoch_seq = sequence / window_size;
    for(int i = 0; i < area_num; i++){
        int cur = (timers[pos] & areas[i]) >> offsets[i];
        if(cur >= epoch_seq - time)
            res += (counters[pos] & areas[i]) >> offsets[i];
    }
    return res;
}

void SlidingWindow::get_flowsize_epoch(vector <std::string> &flowIDs, unordered_map<std::string, int> &freq, int time) {
    return;
}

void SlidingWindow::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq){
    return;
}

int SlidingWindow::get_flowquantity() {
    return 0;
}


void SlidingWindow::get_heavy_hitters(int threshold, vector <pair<std::string, int>> &results) {}
