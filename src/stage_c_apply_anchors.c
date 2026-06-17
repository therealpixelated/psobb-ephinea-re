/*
 * ephinea.dll — Widescreen Cascade: Stage C
 * Original: FUN_52da7ff0 @ 0x52DA7FF0
 *
 * Recovered from reference/ephinea_decompiled_PRIORBUILD.c:131837-132122
 * Pointer tables from data/stage_c_tables.h (current unpacked build).
 */

#include "cascade_constants.h"
#include "stage_c_tables.h"
#include "engine_write.h"
#include "ephinea_widescreen.h"
#include <math.h>
#include <stdint.h>

void stage_c_92va_apply(void);
void *alloc_engine_object(void);
void  set_detour_status(int enabled);

extern float gameRenderW;
extern float gameRenderH;
extern float displayW;
extern float displayH;
extern float hudScaleFactor;
extern float g_5318f038;
extern float g_5318d21c;
extern float g_5318da7c;
extern float g_5318ee38;
extern float g_5318efa4;
extern int   g_numPlayers;
extern int   g_languageId;

/* Layout scalars (f32 reads from prior .rdata — current build) */
#define C_A4    14.0f
#define C_9C    10.9073f
#define C_104   128.0f
#define C_100   110.0f
#define C_124   210.0f
#define C_B8    21.0f
#define C_D4    44.0f
#define C_FD8   14.0f
#define C_A0    12.0f
#define C_D0    40.0f
#define C_F8    93.0f
#define C_F0    80.0f
#define C_C4    32.0f
#define C_E4    61.0f
#define C_118   157.0f
#define C_128   EPH_STOCK_WIDTH
#define C_048   5.333333f
#define C_150   330.0f
#define C_140   288.0f
#define C_144   300.0f
#define C_190   22937.6f
#define C_B4    20.0f
#define C_110   144.0f
#define C_074   7.0f
#define C_088   8.0f
#define C_104B  128.0f
#define C_E28   1.0f   /* _DAT_53175e28 — stub */
#define C_040   3276.8f

static float eph_norm_scale(float derived)
{
    /* ((_DAT_5318ee38 - ZERO) / ZERO + ONE) pattern — ZERO is identity pivot */
    return ((derived - EPH_ONE) / EPH_ONE + EPH_ONE);
}

static float eph_round_like_dll(float x)
{
    return nearbyintf(x);
}

static void eph_write_tbl_f32(const uint32_t *tbl, int count, float value, int additive)
{
    int i;
    for (i = 0; i < count; i++) {
        uint32_t va = tbl[i];
        if (!eph_is_engine_va(va))
            continue;
        if (additive)
            *eph_engine_f32(va) += value;
        else
            *eph_engine_f32(va) = value;
    }
}

void stage_c_apply_anchors(void)
{
    int i;
    float halfW = gameRenderW / EPH_HALF;
    float halfH = gameRenderH / EPH_HALF;
    float wDeltaHalf = (gameRenderW - EPH_STOCK_WIDTH) / EPH_HALF;
    float wDeltaFull = (gameRenderW - EPH_STOCK_WIDTH);
    float hDeltaFull = (gameRenderH - EPH_STOCK_HEIGHT);
    float hDeltaHalf = halfH - C_E4; /* renderH/2 - _DAT_531760e4 */

    /* Allocator batches — results stored in dll-side pointer tables (skipped here) */
    for (i = 0; i < 6; i++)
        (void)alloc_engine_object();
    for (i = 0; i < 6; i++)
        (void)alloc_engine_object();
    for (i = 0; i < 24; i++)
        (void)alloc_engine_object();

    /* Scalar geometry derivations (decompile lines 131912-131953) */
    *eph_engine_f32(0x006F4CF2) = eph_norm_scale(g_5318ee38) * C_A4;
    *eph_engine_f32(0x006F4CFA) = eph_norm_scale(g_5318efa4) * C_9C;
    *eph_engine_f32(0x006F4D02) = eph_norm_scale(g_5318ee38) * C_104B;
    *eph_engine_f32(0x006F4D0A) = eph_norm_scale(g_5318efa4) * C_100;
    *eph_engine_f32(0x006F4D4E) = eph_norm_scale(g_5318ee38) * C_104B;
    *eph_engine_f32(0x006F4D56) = eph_norm_scale(g_5318efa4) * C_9C;
    *eph_engine_f32(0x006F4D5E) = eph_norm_scale(g_5318ee38) * C_124;
    *eph_engine_f32(0x006F4D66) = eph_norm_scale(g_5318efa4) * C_100;

    *eph_engine_f32(0x009D0040) = halfW - EPH_ONE;
    *eph_engine_f32(0x00761CCC) = halfW - C_B8;
    *eph_engine_f32(0x00761CDE) = halfW - C_B8;
    *eph_engine_f32(0x0076236C) = halfW - C_D4;
    *eph_engine_f32(0x007625F8) = halfW - C_FD8;
    *eph_engine_f32(0x007627CF) = halfW - C_FD8;
    *eph_engine_f32(0x008F87D4) = gameRenderW - C_A0;
    *eph_engine_f32(0x008F87E4) = gameRenderW - C_A0;
    *eph_engine_f32(0x009F0ADC) = gameRenderW - C_D0;
    *eph_engine_f32(0x009F0AF4) = gameRenderW - C_F8;
    *eph_engine_f32(0x009F0B0C) = gameRenderW - C_D0;
    *eph_engine_f32(0x009F0B24) = gameRenderW - C_F8;
    *eph_engine_f32(0x009F0B3C) = gameRenderW - EPH_ONE;
    *eph_engine_f32(0x0070D350) = gameRenderW - C_118;
    *eph_engine_f32(0x009F0B5C) = gameRenderW - EPH_ONE;
    *eph_engine_f32(0x0070D2F6) = gameRenderW - C_118;
    *eph_engine_f32(0x009FF080) = gameRenderW - C_C4;
    *eph_engine_f32(0x009FF098) = gameRenderW - C_C4;
    *eph_engine_f32(0x009FF0B0) = gameRenderW - EPH_ONE;
    *eph_engine_f32(0x009FF0C8) = gameRenderW - EPH_ONE;
    *eph_engine_f32(0x009FF0E0) = gameRenderW - C_F0;
    *eph_engine_f32(0x009FF0F8) = gameRenderW - C_F0;
    *eph_engine_f32(0x009FF110) = gameRenderW - C_C4;
    *eph_engine_f32(0x009FF128) = gameRenderW - C_C4;
    *eph_engine_f32(0x009FF1A8) = gameRenderW - C_C4;
    *eph_engine_f32(0x006F4922) = halfW;
    *eph_engine_f32(0x006F4936) = halfH;
    *eph_engine_f32(0x0096F2B8) = gameRenderW - C_E4;
    *eph_engine_f32(0x007584EE) = gameRenderW + C_048;
    *eph_engine_f32(0x0075851F) = gameRenderW + C_048;
    *eph_engine_f32(0x00978454) = gameRenderW - EPH_STOCK_WIDTH;
    *eph_engine_f32(0x00978458) = (gameRenderW - EPH_STOCK_WIDTH) + (-9.53675e-07f);
    *eph_engine_f32(0x009712EC) = gameRenderH;
    *eph_engine_f32(0x007165B8) = gameRenderW - EPH_ONE;

    /* 12× aspectMult pointer writes */
    for (i = 0; i < kAspectMultPtr_COUNT; i++) {
        uint32_t va = kAspectMultPtr[i];
        if (eph_is_engine_va(va))
            *eph_engine_f32(va) = g_5318f038;
    }

    *eph_engine_f32(0x0098A320) = g_5318d21c;
    *eph_engine_f32(0x0098A324) = g_5318da7c;

    /* Six bulk loops */
    eph_write_tbl_f32(kAnchorW, kAnchorW_COUNT, gameRenderW, 0);
    eph_write_tbl_f32(kAnchorH, kAnchorH_COUNT, gameRenderH, 0);
    eph_write_tbl_f32(kAnchorWaddHalf, kAnchorWaddHalf_COUNT, wDeltaHalf, 1);
    eph_write_tbl_f32(kAnchorWaddFull, kAnchorWaddFull_COUNT, wDeltaFull, 1);
    eph_write_tbl_f32(kAnchorHaddFull, kAnchorHaddFull_COUNT, hDeltaFull, 1);
    eph_write_tbl_f32(kAnchorHaddHalf, kAnchorHaddHalf_COUNT, hDeltaHalf, 1);

    for (i = 0; i < kScaleW_COUNT; i++) {
        uint32_t va = kScaleW[i];
        if (!eph_is_engine_va(va)) continue;
        float ratio = *eph_engine_f32(va) / EPH_STOCK_WIDTH;
        *eph_engine_f32(va) = eph_round_like_dll(ratio * gameRenderW);
    }
    for (i = 0; i < kScaleH_COUNT; i++) {
        uint32_t va = kScaleH[i];
        if (!eph_is_engine_va(va)) continue;
        float ratio = *eph_engine_f32(va) / EPH_STOCK_HEIGHT;
        *eph_engine_f32(va) = eph_round_like_dll(ratio * gameRenderH);
    }

    /* ushort H-scale loop — engine ushort targets only */
    for (i = 0; i < kUshortH_COUNT; i++) {
        uint32_t va = kUshortH[i];
        if (!eph_is_engine_va(va)) continue;
        float scaled = (float)*eph_engine_u16(va) * C_E28 * gameRenderH;
        *eph_engine_u16(va) = (uint16_t)eph_round_like_dll(scaled);
    }

    *eph_engine_u32(0x009B8DB4) = 0xFF181818u;
    *eph_engine_u32(0x009B8DC0) = 0xFF181818u;
    *eph_engine_u32(0x009B8DB8) = 0xFA181818u;
    *eph_engine_u32(0x009B8DBC) = 0xFA181818u;
    *eph_engine_u32(0x009B8DE8) = 0xFA181818u;
    *eph_engine_u32(0x009B8DEC) = 0xFA181818u;

    /* Geometric recomputation (6 writes) */
    *eph_engine_f32(0x009CA53C) = halfW - (C_104 - *eph_engine_f32(0x009CA53C)) * eph_norm_scale(g_5318efa4);
    *eph_engine_f32(0x009CA544) = halfW - (C_104 - *eph_engine_f32(0x009CA544)) * eph_norm_scale(g_5318efa4);
    *eph_engine_f32(0x009CA54C) = halfW - (C_104 - *eph_engine_f32(0x009CA54C)) * eph_norm_scale(g_5318efa4);
    *eph_engine_f32(0x009CA554) = halfW - (C_104 - *eph_engine_f32(0x009CA554)) * eph_norm_scale(g_5318efa4);
    *eph_engine_f32(0x0093B8AC) = halfH - (C_E4 - *eph_engine_f32(0x0093B8AC)) * eph_norm_scale(g_5318efa4);
    *eph_engine_f32(0x0093B8A8) = halfH - (C_E4 - *eph_engine_f32(0x0093B8A8)) * eph_norm_scale(g_5318efa4);

    if (g_numPlayers > 1)
        *eph_engine_f32(0x00927318) = C_150;
    if (g_numPlayers > 2) {
        float factor = C_E28;
        for (i = 0; i < g_numPlayers - 2; i++)
            factor = ((float)i / C_040 + EPH_ONE) * factor;
        factor = (float)(g_numPlayers - 2) * factor + EPH_ONE;
        for (i = 0; i < kPlayerScale_COUNT; i++) {
            uint32_t va = kPlayerScale[i];
            if (eph_is_engine_va(va))
                *eph_engine_f32(va) *= factor;
        }
        *eph_engine_f32(0x0097F1BC) += (float)(g_numPlayers - 2);
        *eph_engine_f32(0x0090235C) -= (float)(g_numPlayers - 2);
        *eph_engine_f32(0x00902358) += (float)(g_numPlayers - 2);
    }

    *eph_engine_f32(0x008FB114) = halfW;
    *eph_engine_f32(0x008FB110) = halfH;
    *eph_engine_f32(0x008FB10C) /= hudScaleFactor;

    /* Language / display-mode block stubbed — see decompile 132057-132083 */
    (void)g_languageId;
    set_detour_status(1);

    if (C_140 <= C_144 / hudScaleFactor)
        *eph_engine_f32(0x00A11464) = C_140;
    else
        *eph_engine_f32(0x00A11464) = C_144 / hudScaleFactor;

    stage_c_92va_apply();

    /* Post-92-VA writes (decompile 132093-132121) */
    *eph_engine_f32(0x0077F2D1) = C_190;
    *eph_engine_f32(0x0077F32C) = C_190;
    *eph_engine_f32(0x0077F362) = C_190;
    *eph_engine_f32(0x0077F3C7) = C_190;
    *eph_engine_f32(0x0077F2ED) = gameRenderW + C_074;
    *eph_engine_f32(0x0077F30D) = gameRenderW + C_074;
    *eph_engine_f32(0x0077F386) = gameRenderW + C_074;
    *eph_engine_f32(0x0077F3A7) = gameRenderW + C_074;
    *eph_engine_f32(0x0077F3B2) = gameRenderH + C_074;
    *eph_engine_f32(0x0077F3D2) = gameRenderH + C_074;
    *eph_engine_f32(0x0091FF30) = displayW + C_B4;
    *eph_engine_f32(0x0091FF2C) = displayH + C_B4;
    *eph_engine_f32(0x0091FF00) = displayW + C_B4;
    *eph_engine_f32(0x0091FEFC) = displayH + C_B4;
    *eph_engine_f32(0x00973138) = C_190;
    *eph_engine_f32(0x00973134) = C_190;
    *eph_engine_f32(0x00973130) += C_074;
    *eph_engine_f32(0x0097312C) += C_074;
    *eph_engine_f32(0x009F4654) = C_110;

    {
        float *cols[6] = {
            eph_engine_f32(0x009F4564), eph_engine_f32(0x009F4594),
            eph_engine_f32(0x009F45C4), eph_engine_f32(0x009F45F4),
            eph_engine_f32(0x009F4624), eph_engine_f32(0x009F4654),
        };
        for (i = 0; i < 6; i++)
            *cols[i] = (float)i * C_A4 + C_074;
    }

    *eph_engine_f32(0x0096FFDC) += C_088;
}
