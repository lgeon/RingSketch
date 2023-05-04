#include "HashPart.h"

#ifdef USING_SIMD_ACCELERATION
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif // USING_SIMD_ACCELERATION

template<int bucket_num>
HashPart<bucket_num>::HashPart()
{
    this->clear();
}

template<int bucket_num>
HashPart<bucket_num>::~HashPart(){}

template<int bucket_num>
void HashPart<bucket_num>::clear()
{
	memset(key_bucket, 0, sizeof(uint32_t) * bucket_num);
	memset(val_bucket, 0, sizeof(uint32_t) * bucket_num);
}

template<int bucket_num>
int HashPart<bucket_num>::insert(uint8_t *key, uint8_t *swap_key, uint32_t &swap_val, uint32_t f)
{
    uint32_t fp;
	int pos = CalculateFP(key, fp);

	// 0 : 插入成功
	// 1 : 需要插入下一级
	if (key_bucket[pos] == 0) {
		key_bucket[pos] = fp;
		val_bucket[pos] = f;
		return 0;
	} else if (key_bucket[pos] == fp) {
		val_bucket[pos] += f;
		return 0;
	} else {
		return 1;
	}
}


template<int bucket_num>
int HashPart<bucket_num>::query(uint8_t *key)
{
    uint32_t fp;
    int pos = CalculateFP(key, fp);
	if (key_bucket[pos] == fp) {
		return val_bucket[pos];
	} else {
		return 0;
	}
}

template<int bucket_num>
int HashPart<bucket_num>::CalculateFP(uint8_t *key, uint32_t &fp)
{
    fp = *((uint32_t*)key);
    return CalculateBucketPos(fp) % bucket_num;
}