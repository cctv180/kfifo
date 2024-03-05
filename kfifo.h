#ifndef KFIFO_H
#define KFIFO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct KFIFO_Handle
{
    volatile uint32_t in;
    volatile uint32_t out;
    uint32_t mask;
    void *pool;
} KFIFO_Handle_t;

/**
 * @brief 使用给定的内存池和大小初始化循环缓冲区。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param pool 指向内存池的指针。
 * @param size 内存池的大小。
 * @return 如果初始化成功则返回true，否则返回false。
 */
bool KFIFO_Init(KFIFO_Handle_t *rb, void *pool, uint32_t size);


/**
 * @brief 向循环缓冲区写入一个字节。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param byte 要写入的字节。
 * @return 如果写入操作成功则返回true，否则返回false。
 */
bool KFIFO_WriteByte(KFIFO_Handle_t *rb, uint8_t byte);

/**
 * @brief 从循环缓冲区读取一个字节。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param byte 用于存储读取字节的指针。
 * @return 如果读取操作成功则返回true，否则返回false。
 */
bool KFIFO_ReadByte(KFIFO_Handle_t *rb, uint8_t *byte);

/**
 * @brief 向循环缓冲区写入一个数据缓冲区。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param buffer 指向数据缓冲区的指针。
 * @param size 数据缓冲区的大小。
 * @return 如果写入操作成功则返回true，否则返回false。
 */
bool KFIFO_Write_Buffer(KFIFO_Handle_t *rb, const void *buffer, uint32_t size);

/**
 * @brief 从循环缓冲区读取一个数据缓冲区。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param buffer 用于存储读取数据的缓冲区的指针。
 * @param size 缓冲区的大小。
 * @return 如果读取操作成功则返回true，否则返回false。
 */
bool KFIFO_Read_Buffer(KFIFO_Handle_t *rb, void *buffer, uint32_t size);

/**
 * @brief 向KFIFO的线性缓冲区写入数据。
 *
 * 这个函数向KFIFO的线性缓冲区写入数据。
 *
 * @param rb 指向KFIFO句柄的指针。
 * @return 指向可以写入数据的线性缓冲区的指针。
 */
void *KFIFO_Linear_Write(KFIFO_Handle_t *rb);

/**
 * @brief 完成对KFIFO线性缓冲区的写操作。
 *
 * 这个函数完成对KFIFO线性缓冲区的写操作。
 *
 * @param rb 指向KFIFO句柄的指针。
 * @param size 写入线性缓冲区的字节数。
 */
void KFIFO_Linear_Write_Finish(KFIFO_Handle_t *rb, uint32_t size);

/**
 * @brief 从KFIFO的线性缓冲区读取数据。
 *
 * 这个函数从KFIFO的线性缓冲区读取数据。
 *
 * @param rb 指向KFIFO句柄的指针。
 * @return 指向可以从中读取数据的线性缓冲区的指针。
 */
void *KFIFO_Linear_Read(KFIFO_Handle_t *rb);

/**
 * @brief 完成对KFIFO线性缓冲区的读操作。
 *
 * 这个函数完成对KFIFO线性缓冲区的读操作。
 *
 * @param rb 指向KFIFO句柄的指针。
 * @param size 从线性缓冲区读取的字节数。
 */
void KFIFO_Linear_Read_Finish(KFIFO_Handle_t *rb, uint32_t size);

/**
 * 获取缓冲区中的数据长度。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @return 缓冲区中的数据长度。
 */
uint32_t KFIFO_GetLen(KFIFO_Handle_t *rb);

/**
 * 预览一个数据。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 * @param byte 用于存储预览数据的指针。
 * @return 如果预览操作成功则返回true，否则返回false。
 */
bool KFIFO_Peek(KFIFO_Handle_t *rb, uint8_t *byte);

/**
 * 重置环形缓冲区。
 *
 * @param rb 指向KFIFO_Handle_t结构的指针。
 */
void KFIFO_Reset(KFIFO_Handle_t *rb);


#endif // KFIFO_H

// #define RB_SIZE (rb->mask + 1)        // 缓冲区大小
// #define RB_USED (rb->in - rb->out)    // 已使用大小
// #define RB_UNUSED (RB_SIZE - RB_USED) // 空闲大小 #define RB_USED ((rb->in<rb->out)?RB_SIZE-(rb->out-rb->in):(rb->in-rb->out))
// #define RB_OFFSET_IN (rb->in & rb->mask)   // 计算写入位置
// #define RB_OFFSET_OUT (rb->out & rb->mask) // 计算读取位置

// #define RB_REMAIN_IN (RB_SIZE - RB_OFFSET_IN)   // 计算从写入位置到环形缓冲区末尾的剩余空间
// #define RB_REMAIN_OUT (RB_SIZE - RB_OFFSET_OUT) // 计算从读取位置到环形缓冲区末尾的剩余空间

