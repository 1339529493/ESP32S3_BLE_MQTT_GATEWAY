#include <stdlib.h>
#include <string.h>
#include "memory_pool.h"

/**
 * @brief 初始化内存池
 * 分配一大块连续内存，并将其分割成固定大小的块，链接成空闲链表
 */
int mp_init(memory_pool *pool, size_t block_size, size_t block_count)
{
    // 参数有效性检查
    if (pool == NULL || block_size == 0 || block_count == 0)
        return -1;

    // 确保块大小至少能容纳 mp_block 结构体（用于存储 next 指针）
    pool->block_size = block_size < sizeof(mp_block) ? sizeof(mp_block) : block_size;
    pool->block_count = block_count;
    pool->pool_size = pool->block_size * block_count;
    
    // 分配连续的内存区域作为池底
    pool->pool_base = (char *)malloc(pool->pool_size);
    if (pool->pool_base == NULL)
    {
        return -2;
    }

    // 初始化空闲链表：将所有块串联起来
    pool->free_list = NULL;
    for (size_t i = 0; i < block_count; i++) {
        // 计算第 i 个块的起始地址
        mp_block *block = (mp_block *)(pool->pool_base + i * pool->block_size);
        // 将当前块插入空闲链表头部
        block->next = pool->free_list;
        pool->free_list = block;
    }
    pool->used = 0;
    pool->mutex = xSemaphoreCreateMutex();
    if (pool->mutex == NULL) {
        free(pool->pool_base);
        return -3;  // 锁创建失败
    }
    return 0;
}

/**
 * @brief 分配内存块
 * 优先从空闲链表获取，若链表为空则使用 malloc 动态分配（溢出处理）
 */
void* mp_alloc(memory_pool *pool, size_t size)
{
    if (pool == NULL)
        return NULL;
    
    if (xSemaphoreTake(pool->mutex, portMAX_DELAY) == pdTRUE) 
    {
        // 如果空闲链表为空或数据长度大于块，回退到标准 malloc
        if (pool->free_list == NULL || size > pool->block_size) {
            void *ret = malloc(size);
            xSemaphoreGive(pool->mutex);
            return ret;
        }

        // 从空闲链表头部取出一个块
        mp_block *block = pool->free_list;
        pool->free_list = block->next;
        pool->used++;
        xSemaphoreGive(pool->mutex);
        return (void *)block;
    }
    return NULL;
}

/**
 * @brief 释放内存块
 * 判断指针是否属于池内内存：
 * - 是：归还到空闲链表
 * - 否：调用 free 释放（针对 mp_alloc 中 malloc 分配的块）
 */
void mp_free(memory_pool *pool, void *ptr)
{
    if (pool == NULL || ptr == NULL)
        return;

    if (xSemaphoreTake(pool->mutex, portMAX_DELAY) == pdTRUE) 
    {
        char *p = (char *)ptr;
        // 检查指针是否在内存池管理的地址范围内
        if (p >= pool->pool_base && p < pool->pool_base + pool->pool_size) {
            // 属于池内内存，归还到空闲链表头部
            mp_block *block = (mp_block *)ptr;
            block->next = pool->free_list;
            pool->free_list = block;
            pool->used--;
        } else {
            // 不属于池内内存（可能是溢出时 malloc 的），直接 free
            free(ptr);
        }
        xSemaphoreGive(pool->mutex);
    }
}

/**
 * @brief 销毁内存池
 * 释放底层分配的连续内存，并重置状态
 */
void mp_destroy(memory_pool *pool)
{
    if (pool == NULL)
        return;
    
    // 释放基地址指向的内存
    free(pool->pool_base);
    vSemaphoreDelete(pool->mutex);
    pool->pool_base = NULL;
    pool->free_list = NULL;
    pool->pool_size = 0;
    pool->used = 0;
}