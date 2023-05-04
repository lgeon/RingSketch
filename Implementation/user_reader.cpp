#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>     /* open */
#include <unistd.h>    /* exit */
#include <sys/ioctl.h> /* ioctl */
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "util.h"
#include "tuple.h"
#include "ringbuffer.h"
#include "./mySketch/algorithms/slidingWindow/slidingWindow.cpp"
#include "./mySketch/algorithms/BOBHash32.cpp"
#include "./mySketch/algorithms/algorithms.cpp"
#include "./mySketch/algorithms/slidingSketch/slidingSketch.cpp"
#include "./mySketch/algorithms/cmSketch/cmsketch.h"
#include "./mySketch/algorithms/cmSketch/cmsketch.cpp"
#include "./mySketch/algorithms/countSketch_sw/countSketch_sw.cpp"
#include "./mySketch/algorithms/countSketch_ss/countSketch_ss.cpp"
#include "./mySketch/algorithms/bloomFilter_ss/bloomFilter_ss.cpp"
#include "./mySketch/algorithms/new_reslove/cmSketch_new.cpp"
#include "./mySketch/algorithms/new_reslove/cuSketch_new.cpp"
#include "./mySketch/algorithms/new_reslove/countSketch_new.cpp"
#include "./mySketch/algorithms/new_reslove/bloomFilter_new.cpp"
#include "./mySketch/algorithms/new_reslove/heavyKeeper_new.cpp"
#include "./mySketch/algorithms/heavyKeeper_ss/heavyKeeper_ss.cpp"
 
// The number of ringbuffer
// **Must** be (# pmd threads + 1) 
#define MAX_RINGBUFFER_NUM 1 
#define DEPTH 4   
#define TOT_MEM_IN_BYTES DEPTH      
#define WINDOW_SIZE 10000

//3000 6000 9000 12000
// SlidingSketch *ss = new SlidingSketch(10 * 12000 * 8, 10, 3 * WINDOW_SIZE, 8); 
countSketch_ss *ss = new countSketch_ss(10 * 3000 * 8, 10, 3 * WINDOW_SIZE, 2);

//20000 40000 60000 80000
// CMSketch_new *ns = new CMSketch_new(3 * 80000 * 2, 3, WINDOW_SIZE); 
// CUSketch_new *ns = new CUSketch_new(3 * 80000 * 2, 3, WINDOW_SIZE); 
CountSketch_new *ns = new CountSketch_new(3 * 20000 * 2, 3, WINDOW_SIZE);
// bloomFilter_ss *ss = new bloomFilter_ss(450000 * sizeof(bool), 10, 3 * WINDOW_SIZE, 2);
// BloomFilter_new *ns = new BloomFilter_new(225000 * sizeof(bool), 3, WINDOW_SIZE);
// HeavyKeeper_new *ns = new HeavyKeeper_new(3 * 316 * 16, 3, WINDOW_SIZE);
// heavyKeeper_ss *ss = new heavyKeeper_ss(10 * 200 * 16, 10, 3 * WINDOW_SIZE, 2); //29 byte per counter
  
static inline char *ip2a(uint32_t ip, char *addr)  
{
    sprintf(addr, "%d.%d.%d.%d", (ip & 0xff), ((ip >> 8) & 0xff), ((ip >> 16) & 0xff), ((ip >> 24) & 0xff)); 
    return addr;    
} 
 
void print_tuple(FILE *f, tuple_t *t)  
{  
    char ip1[30], ip2[30]; 

    fprintf(f, "%s(%u) <-> %s(%u) %u %ld\n",
            ip2a(t->key.src_ip, ip1), t->key.src_port,      
            ip2a(t->key.dst_ip, ip2), t->key.dst_port,
            t->key.proto, t->size);
}   

int counter = 0;    
int query = 0;   
bool KEEP_RUNNING;   
ofstream f;
ofstream f2;
ringbuffer_t *rbs[MAX_RINGBUFFER_NUM];
 
void handler(int sig)  
{
    KEEP_RUNNING = false;
    ss->clear(); 
    ns->clear();
    f.close(); 
    f2.close();
    printf("total insert: % d\n", counter); 
    printf("total query: % d\n", query); 
 
}
  
int main(int argc, char *argv[])
{ 
    
    LOG_MSG("Initialize the ringbuffer.\n");
    for (int i = 0; i < MAX_RINGBUFFER_NUM; ++i) 
    {
        char name[30];
        sprintf(name, "/rb_%d", i);
        rbs[i] = connect_ringbuffer_shm(name, sizeof(tuple_t));
    }
    printf("connected.\n");       
    signal(SIGINT, handler); 
    KEEP_RUNNING = true; 
    //insert
    int idx = 0;
    while (KEEP_RUNNING) 
    {   
        tuple_t t;
        while (read_ringbuffer(rbs[(idx) % MAX_RINGBUFFER_NUM], &t) < 0 && KEEP_RUNNING)
        {
            idx = (idx + 1) % MAX_RINGBUFFER_NUM; 
        }
        if(t.key.src_ip == 0 && t.key.dst_ip == 0){
            printf("Insert end.\n");
            break;
        } 
        counter++;
        
        ss->insert((uint8_t *)&t, 13); 
        ns->insert((uint8_t *)&t, 13);   
        printf("total insert: %d\n", counter);
        
    }   

 
    //query    
    f.open("result_sw.txt", std::ios::out);     
    f2.open("result_ss.txt", std::ios::out); 
    while (KEEP_RUNNING)
    {    
        tuple_t t;
        while (read_ringbuffer(rbs[(idx) % MAX_RINGBUFFER_NUM], &t) < 0 && KEEP_RUNNING)
        {  
            idx = (idx + 1) % MAX_RINGBUFFER_NUM; 
        }
        if(t.key.src_ip == 0 && t.key.dst_ip == 0)
            continue;
        // print_tuple(stdout, &t);
        // printf("\n");
        int count = ns->query((uint8_t *)&t, 13);
        int count2 = ss ->query((uint8_t *)&t, 13);  
        f << count << "\n";   
        f2 << count2 << "\n";       
        query++;   

        printf("total query: %d\n", query);
    } 
    return 0;     
}               
      