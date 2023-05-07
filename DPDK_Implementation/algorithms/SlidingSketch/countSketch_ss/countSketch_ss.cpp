#include "countSketch_ss.h"
#include <queue>

countSketch_ss::countSketch_ss(int tot_mem_in_bytes, int d, int window_size, int day) : Algorithm("countSketch_ss"){
    this->d = d; //depth
    this->width = tot_mem_in_bytes / d / sizeof(uint64_t);
    this->window_size = window_size;
    this->day = day;
    this->step = width * d / window_size *  (day - 1); // width  > windowSize
    this->index = 0;
    this->cur = 0;
    printf("%d\n", step);
    fflush(stdout);
    counters = new int64_t*[width * d];
    for(int i = 0; i < width * d; i++){
        counters[i] = new int64_t[day];
        memset(counters[i], 0, sizeof(int64_t) * day); 
    }

    hash = new BOBHash32[d];  // 生成d个hash函数
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i);  // 初始化哈希函数？？？

    s_hash = new BOBHash32[d];
    for(int i = 0; i < d; i++)
        s_hash[i].initialize(50 + i);

}

countSketch_ss::~countSketch_ss(){
    for(int i = 0; i < width * d; i++)
        delete[] counters[i];
    delete[] hash;
    delete[] s_hash;

}

void countSketch_ss::clear() {
    insertTimes = 0;
    for(int i = 0; i < width * d; i++){
        memset(counters[i], 0, sizeof(int64_t) * day); 
    }

}

int countSketch_ss::insert(uint8_t *key, int keylen) {
    c_update(key, keylen);
    for(int i = 0; i < step; i++){
        int64_t* bucket = counters[index];
        for(int j = day - 1; j >= 1; j--)
            bucket[j] = bucket[j - 1];
        bucket[0] = 0;
        index = (index + 1) % (width * d);
    }

    return ++insertTimes;  //返回插入次数
}



void countSketch_ss::c_update(uint8_t* key, int keylen){
    for(int i = 0; i < d; i++){
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int s = s_hash[i].run((const char*)key, keylen) % 2;
        s = s == 0 ? -1 : s;
        counters[pos][0] += s;
    }
}


int countSketch_ss::query(uint8_t* key, int keylen){
    priority_queue<int, vector<int>, less<int>> queMin;
    priority_queue<int, vector<int>, greater<int>> queMax;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        int s = s_hash[i].run((const char*)key, keylen) % 2;
        s = s == 0 ? -1 : s;
        int count = get_count(pos) * s;
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

int countSketch_ss::get_count(int pos){
    int res = 0;
    
    for(int i = 0; i < day; i++)
        res += counters[pos][i];
    // res = counters[pos][0];
    return res;
}

void countSketch_ss::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq){}

int countSketch_ss::get_flowquantity(){return 0;}

void countSketch_ss::get_heavy_hitters(int threshold, vector<pair<string, int>> & results){}