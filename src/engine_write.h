/* Helpers for writing PsoBB.exe globals by absolute VA. */

#ifndef ENGINE_WRITE_H
#define ENGINE_WRITE_H

#include <stdint.h>

static inline float *eph_engine_f32(uint32_t va)
{
    return (float *)(uintptr_t)va;
}

static inline uint32_t *eph_engine_u32(uint32_t va)
{
    return (uint32_t *)(uintptr_t)va;
}

static inline uint16_t *eph_engine_u16(uint32_t va)
{
    return (uint16_t *)(uintptr_t)va;
}

static inline int eph_is_engine_va(uint32_t va)
{
    return va >= 0x00400000u && va <= 0x00FFFFFFu;
}

#endif /* ENGINE_WRITE_H */
