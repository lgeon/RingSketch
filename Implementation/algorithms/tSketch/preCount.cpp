#include "preCount.h"

/*
	output:
	0: 未逐出
	1: 逐出
*/
template <int bucket_num>
int PreCount<bucket_num>::insert(uint8_t *key, int timestamp, uint8_t *swap_key, uint32_t &swap_val, int keylen, uint32_t f)
{
    uint32_t fp;
    int pos = CalculateFP(key, fp);

    int match = -1, empty = -1;

    // 如果空, 插入
    if (buckets[pos].key[0] == 0) {
        buckets[pos].key[0] = key;
        buckets[pos].timestamp[0] = timestamp;
        buckets[pos].val[0] += f;
        return 0;
    }

    // key 相同，插入
    if (buckets[pos].key[0] == fp) {
        buckets[pos].val[0] += f;
        return 0;
    }

    // key 不同，但占据的流未过期, 放到第二个槽位
    int threshold = 250000; // 阈值 250 000us
    if (timestamp - buckets[pos].timestamp[0] < threshold) {
        // 第二个 bucket 为空, 记录 key 和 timestamp
        if (buckets[pos].key[1] == 0) {
            buckets[pos].key[1] = key;
            buckets[pos].timestamp[1] = timestamp;
        }
        buckets[pos].val[1] += f;
        return 0;
    }

    // key 不同，占据的流过期，驱逐第一个流, 放到第二个槽位
    swap_key = buckets[pos].key[0];
    swap_val = buckets[pos].val[0];
    buckets[pos].key[0] = buckets[pos].key[1];
    buckets[pos].val[0] = buckets[pos].val[1];
    buckets[pos].key[1] = key;
    buckets[pos].timestamp[1] = timestamp;
    buckets[pos].val[1] += f;
    return 1;
}

template <int bucket_num>
int PreCount<bucket_num>::query(uint8_t *key)
{
    uint32_t fp;
    int pos = CalculateFP(key, fp);
    
    if (buckets[pos].key[0] == fp) {
        return buckets[pos].val[0];
    } else if (buckets[pos].key[1] == fp) {
        return buckets[pos].val[1];
    } else {
        return 0;
    }

}

template <int bucket_num>
int PreCount<bucket_num>::CalculateFP(uint8_t *key, uint32_t &fp)
{
    fp = *((uint32_t*)key);
    return CalculateBucketPos(fp) % bucket_num;
}