/**
 *  模块名称 : KFIFO模块
 *  文件名称 : kfifo.c
 *  版   本 : V0.02
 *  说   明 : 仿KFIFO
 *  修改记录 :
 *  版本号      日期        作者    说明
 *  V0.02   2024-03-05  cctv180  增加截尾写入 KFIFO_Write_Buffer_Truncated
 *  V0.01   2024-02-02   xzyswr  KFIFO https://www.armbbs.cn/forum.php?mod=viewthread&tid=122967
 *  参考资料 : STM32 HAL 库实现乒乓缓存加空闲中断的串口 DMA 收发机制，轻松跑上 2M 波特率
 *           https://blog.csdn.net/wanglei_11/article/details/131576165
 *
 */

#include <string.h>
#include "kfifo.h"

/* 缓冲区大小: mask + 1 */
static inline uint32_t RB_SIZE(const KFIFO_Handle_t *rb)
{
    return rb->mask + 1; // mask是缓冲区大小减1，所以加1就是原始大小
}

/* 已使用大小 (in - out) & mask */
static inline uint32_t RB_USED(const KFIFO_Handle_t *rb)
{
    return (rb->in - rb->out) & rb->mask; // in是写入位置，out是读取位置，二者之差就是已使用的大小
}

/* 空闲大小 */
static inline uint32_t RB_UNUSED(const KFIFO_Handle_t *rb)
{
    return RB_SIZE(rb) - RB_USED(rb); // 总大小减去已使用的大小就是未使用的大小
}

/* 辅助函数 写入位置偏移量 */
static inline uint32_t RB_OFFSET_IN(const KFIFO_Handle_t *rb)
{
    return rb->in & rb->mask; // 使用位运算获取写入位置在缓冲区中的偏移量
}

/* 辅助函数 读取位置偏移量 */
static inline uint32_t RB_OFFSET_OUT(const KFIFO_Handle_t *rb)
{
    return rb->out & rb->mask; // 使用位运算获取读取位置在缓冲区中的偏移量
}

/* 辅助函数 写入位置到缓冲区末尾的空间大小 */
static inline uint32_t RB_REMAIN_IN(const KFIFO_Handle_t *rb)
{
    return RB_SIZE(rb) - RB_OFFSET_IN(rb); // 总大小减去写入位置的偏移量就是剩余空间
}

/* 辅助函数 读取位置到缓冲区末尾的空间大小 */
static inline uint32_t RB_REMAIN_OUT(const KFIFO_Handle_t *rb)
{
    return RB_SIZE(rb) - RB_OFFSET_OUT(rb); // 总大小减去读取位置的偏移量就是剩余空间
}

/* 缓冲区是否已满 */
static inline bool KFIFO_IsFull(const KFIFO_Handle_t *rb)
{
    return (RB_UNUSED(rb) == 0); // 如果未使用的大小为0，那么缓冲区就是满的
}

/* 缓冲区是否为空 */
static inline bool KFIFO_IsEmpty(const KFIFO_Handle_t *rb)
{
    return (RB_USED(rb) == 0); // 如果已使用的大小为0，那么缓冲区就是空的
}

bool KFIFO_Init(KFIFO_Handle_t *rb, void *pool, uint32_t size)
{
    if (rb == NULL || pool == NULL || (size & (size - 1)))
    {
        return false;
    }
    rb->in = 0;
    rb->out = 0;
    rb->mask = size - 1;
    rb->pool = pool;
    return true;
}

bool KFIFO_WriteByte(KFIFO_Handle_t *rb, uint8_t byte)
{
    if (KFIFO_IsFull(rb))
    {
        return false;
    }

    ((uint8_t *)rb->pool)[RB_OFFSET_IN(rb)] = byte;
    rb->in++;

    return true;
}

bool KFIFO_ReadByte(KFIFO_Handle_t *rb, uint8_t *byte)
{
    if (KFIFO_IsEmpty(rb))
    {
        return false;
    }

    *byte = ((uint8_t *)rb->pool)[RB_OFFSET_OUT(rb)];
    rb->out++;

    return true;
}

bool KFIFO_Write_Buffer(KFIFO_Handle_t *rb, const void *buffer, uint32_t size)
{
    if (RB_UNUSED(rb) < size)
    {
        return false;
    }

    if (RB_REMAIN_IN(rb) < size)
    {
        memcpy((uint8_t *)rb->pool + RB_OFFSET_IN(rb), buffer, RB_REMAIN_IN(rb));
        memcpy(rb->pool, (uint8_t *)buffer + RB_REMAIN_IN(rb), size - RB_REMAIN_IN(rb));
    }
    else
    {
        memcpy((uint8_t *)rb->pool + RB_OFFSET_IN(rb), buffer, size);
    }
    rb->in += size;

    return true;
}

uint32_t KFIFO_Write_Buffer_Truncated(KFIFO_Handle_t *rb, const void *buffer, uint32_t size)
{
    // 计算可用空间
    uint32_t space = RB_UNUSED(rb);

    // 如果待写入的数据超过了可用空间，进行截尾
    if (size > space)
    {
        size = space;
    }
    if (size == 0)
    {
        return 0;
    }

    // 分两步写入数据，首先判断是否需要环绕
    if (RB_REMAIN_IN(rb) < size)
    {
        // 第一部分：从当前写入位置到buffer末尾
        memcpy((uint8_t *)rb->pool + RB_OFFSET_IN(rb), buffer, RB_REMAIN_IN(rb));
        // 第二部分：从buffer开始到需要环绕到的位置
        memcpy(rb->pool, (uint8_t *)buffer + RB_REMAIN_IN(rb), size - RB_REMAIN_IN(rb));
    }
    else
    {
        // 如果不需要环绕，一次性完成写入
        memcpy((uint8_t *)rb->pool + RB_OFFSET_IN(rb), buffer, size);
    }
    rb->in += size;

    return size; // 返回实际写入的字节数
}

bool KFIFO_Read_Buffer(KFIFO_Handle_t *rb, void *buffer, uint32_t size)
{
    if (RB_USED(rb) < size)
    {
        return false;
    }

    if (RB_REMAIN_OUT(rb) < size)
    {
        memcpy(buffer, (uint8_t *)rb->pool + RB_OFFSET_OUT(rb), RB_REMAIN_OUT(rb));
        memcpy((uint8_t *)buffer + RB_REMAIN_OUT(rb), rb->pool, size - RB_REMAIN_OUT(rb));
    }
    else
    {
        memcpy(buffer, (uint8_t *)rb->pool + RB_OFFSET_OUT(rb), size);
    }
    rb->out += size;

    return true;
}

inline uint32_t KFIFO_Linear_Write_Rem(const KFIFO_Handle_t *rb)
{
    return RB_REMAIN_IN(rb); // 返回从当前写入位置到缓冲区末尾的剩余空间大小
}

void *KFIFO_Linear_Write(KFIFO_Handle_t *rb)
{
    return (uint8_t *)rb->pool + RB_OFFSET_IN(rb);
}

void KFIFO_Linear_Write_Finish(KFIFO_Handle_t *rb, uint32_t size)
{
    rb->in += size;
}

inline uint32_t KFIFO_Linear_Read_Rem(const KFIFO_Handle_t *rb)
{
    return RB_REMAIN_OUT(rb); // 返回从当前读取位置到缓冲区末尾的剩余数据量
}

void *KFIFO_Linear_Read(KFIFO_Handle_t *rb)
{
    return (uint8_t *)rb->pool + RB_OFFSET_OUT(rb);
}
void KFIFO_Linear_Read_Finish(KFIFO_Handle_t *rb, uint32_t size)
{
    rb->out += size;
}

uint32_t KFIFO_GetLen(KFIFO_Handle_t *rb)
{
    return RB_USED(rb);
}

bool KFIFO_Peek(KFIFO_Handle_t *rb, uint8_t *byte)
{
    if (KFIFO_IsEmpty(rb))
    {
        return false;
    }

    *byte = ((uint8_t *)rb->pool)[RB_OFFSET_OUT(rb)];
    return true;
}

void KFIFO_Reset(KFIFO_Handle_t *rb)
{
    rb->in = 0;
    rb->out = 0;
}
