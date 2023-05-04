#ifndef _T_SKETCH_H_
#define _T_SKETCH_H_

#include "../BOBHash32.h"
#include "../algorithms.h"
#include "./preCount.h"
#include "./hashPart.h"
#include "./sketchPart.h"

class TSketch: public Algorithm
{
    static constexpr int pre_mem = 150;
    static constexpr int mem1 = 150;
    static constexpr int mem2 = 150;
    static constexpr int mem3 = 150;

    PreCount<pre_mem> pre_count; // 预处理器
    HashPart<mem1> first_part;  // 一级哈希表
    HashPart<mem2> second_part;  // 二级哈希表
    SketchPart<mem3> third_part;  // 三级cm-sketch

public:
    TSketch();
    ~TSketch();

    int insert(uint8_t *key, int keylen, int timestamp);
    int query(uint8_t *key, int keylen);
    void clear();
    void get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq);
};


#endif //_T_SKETCH_H_