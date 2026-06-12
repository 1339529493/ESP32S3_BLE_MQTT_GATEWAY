#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stddef.h>
#include "freertos/FreeRTOS.h"

/**
 * @brief 内存块节点结构
 * 用于构建空闲链表，每个内存块的起始位置存储指向下一个空闲块的指针
 */
typedef struct mp_block {
    struct mp_block *next; /**< 指向下一个空闲内存块的指针 */
} mp_block;

/**
 * @brief 内存池结构体
 * 管理一块预分配的连续内存，将其分割为固定大小的块
 */
typedef struct {
    size_t block_size;   /**< 每个内存块的大小（字节），至少为 sizeof(mp_block) */
    size_t block_count;  /**< 内存池中块的总数量 */
    char *pool_base;     /**< 内存池基地址，指向 malloc 分配的连续内存区域 */
    size_t pool_size;    /**< 内存池总大小（block_size * block_count） */
    mp_block *free_list; /**< 空闲块链表的头指针 */
    size_t used;         /**< 当前已分配的块数量 */
    SemaphoreHandle_t mutex;    /**< 互斥锁，用于保护内存池的并发访问 */
} memory_pool;

/**
 * @brief 初始化内存池
 * 
 * @param pool 指向要初始化的内存池结构体的指针
 * @param block_size 每个块的大小
 * @param block_count 块的数量
 * @return int 0 表示成功，-1 表示失败（参数错误或内存分配失败）
 */
int mp_init(memory_pool *pool, size_t block_size, size_t block_count);

/**
 * @brief 从内存池中分配一个块
 * 
 * @param pool 指向内存池结构体的指针
 * @param size 要分配的块的大小
 * @return void* 指向分配到的内存块的指针，如果池为空或大小超过块则回退到 malloc，失败返回 NULL
 */
void* mp_alloc(memory_pool *pool, size_t size);

/**
 * @brief 释放一个块回内存池
 * 
 * @param pool 指向内存池结构体的指针
 * @param ptr 指向要释放的内存块的指针
 */
void mp_free(memory_pool *pool, void *ptr);

/**
 * @brief 销毁内存池，释放底层分配的内存
 * 
 * @param pool 指向要销毁的内存池结构体的指针
 */
void mp_destroy(memory_pool *pool);

#endif