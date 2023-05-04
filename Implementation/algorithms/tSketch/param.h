#ifndef _PARAM_H_
#define _PARAM_H_

#include <string.h>
#include <stdint.h>

#define KEY_LENGTH_4 4
#define KEY_LENGTH_13 13

#define BUCKET_PER_COUNTER 2
#define CONSTANT_NUMBER 2654435761u

#define CalculateBucketPos(fp) (((fp) * CONSTANT_NUMBER) >> 15)

struct Bucket
{
	uint32_t key[BUCKET_PER_COUNTER]; // flow ID
	uint16_t val[BUCKET_PER_COUNTER]; // cnt
    uint16_t timestamp[BUCKET_PER_COUNTER]; // 首次插入的时间戳
};

#endif