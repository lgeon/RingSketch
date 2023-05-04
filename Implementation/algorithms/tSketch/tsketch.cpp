#include <string.h>
#include "tsketch.h"

// TSketch data structure (一个一维数组)
/* counters[width*d]
 * | [0,0] |  ...  | [0,w-1] | [1,0] | ... | [1,w-1] | ...  |
 * |       |       |         |       |     |         |      |
*/


/*
 * d: TSketch深度
 * width: TSketch宽度
*/

TSketch::TSketch(): Algorithm("TSketch") {}

// 函数执行完毕，删除数据
TSketch::~TSketch() {}

// 请求记录
void TSketch::clear()
{
    insertTimes = 0;
    pre_count.clear();
    first_part.clear();
    second_part.clear();
    third_part.clear();
}


// 插入表项
int TSketch::insert(uint8_t *key, int keylen, int timestamp)
{
    // 储存驱逐流的数据
    uint8_t swap_key[KEY_LENGTH_4];
    uint32_t swap_val; //todo : notice, 类型
    int res;

    // 插入pre_count
    res = pre_count.insert(key, timestamp, swap_key, swap_val, 32, 1);
    
    // 插入一级表
    if (res != 0) {
        res = first_part.insert(key, swap_key, swap_val, swap_val);
    } 

    // 插入二级表
    if (res != 0) {
        res = second_part.insert(key, swap_key, swap_val, swap_val);
    }

    // 插入三级表
    if (res != 0) {
        third_part.insert(key, KEY_LENGTH_4, swap_val);
    }
    // return insertTime++;
    return 0;
}


// 查询表项
int TSketch::query(uint8_t *key, int keylen)
{
    // 返回 pre_count 中残留数据 + 三级表中的数据
    int res = pre_count.query(key);
    
    if (first_part.query(key) > 0) {
        res += first_part.query(key);
    } else if (second_part.query(key) > 0) {
        res += second_part.query(key);
    } else {
        res += third_part.query(key);
    }
    return res;
}

void TSketch::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq)
{
    freq.clear();
    uint8_t key[100] = {0};
    for(auto id : flowIDs){
        memcpy(key, id.c_str(), id.size());
        freq[id] = this->query(key, id.size());
    }
}