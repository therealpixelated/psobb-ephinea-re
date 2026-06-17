/*
 * ephinea.dll — Widescreen Cascade: Stage B
 * Original: FUN_52dabbd0 @ 0x52DABBD0
 *
 * Recovered from reference/ephinea_decompiled_PRIORBUILD.c:133394-133493
 */

#include "cascade_constants.h"
#include "ephinea_widescreen.h"
#include <math.h>

extern float displayW;
extern float displayH;
extern float gameRenderW;
extern float gameRenderH;
extern float hudScaleFactor;
extern float g_5318f038;
extern float g_5318d21c;
extern float g_5318da7c;
extern float g_5318efa4;
extern float g_5318ee38;

extern int   g_widescreenEnabled; /* _DAT_53857a5c */
extern float hudScalePercent;     /* _DAT_5382fe9c */

extern float g_0097a910;
extern float g_0096e114;
extern float g_006f49fd;
extern float g_006f4a57;
extern float g_0096e168;
extern float g_0096e16c;
extern float g_0096e170;
extern float g_0096e174;
extern float g_0096e178;
extern float g_0096e17c;

/* Bucket payload (f32 reads from current unpacked .rdata) */
#define B_EA0   0.001f
#define B_EA4   0.002734375f
#define B_EB8   0.01875f
#define B_F58   1.55f
#define B_FA8   7.0f
#define B_FC8   9.0f
#define B_FD8   14.0f
#define B_FF0   29.0f
#define B_000   43.0f
#define B_044   3413.334f
#define B_048   5.333333f
#define B_064   6.9765625f
#define B_070   32768.0f
#define B_07C   0.0f
#define B_E8C   EPH_ASPECT_4_3
#define B_E78   1.333333f

static float eph_table_lookup(int idx)
{
    (void)idx;
    return 1.0f; /* DAT_531761e0[] — TODO: dump from current .rdata */
}

static float eph_round_like_dll(float x)
{
    return nearbyintf(x);
}

void stage_b_compute_render_size(void)
{
    float aspect = displayW / displayH;
    float S;
    float aspectMult;
    double dVar3;
    int idx = 0;

    if (aspect >= (float)kAspectBreakpoints[0]) {
        if (aspect >= (float)kAspectBreakpoints[1]) {
            if (aspect >= (float)kAspectBreakpoints[2]) {
                if (aspect >= (float)kAspectBreakpoints[3]) {
                    if (aspect >= (float)kAspectBreakpoints[4]) {
                        if (aspect >= (float)kAspectBreakpoints[5]) {
                            if (aspect >= (float)kAspectBreakpoints[6]) {
                                g_5318d21c = B_064;  g_5318da7c = B_FA8;
                                aspectMult = B_000;  S = B_EA0;
                            } else {
                                g_5318d21c = B_07C;  g_5318da7c = B_044;
                                aspectMult = B_FC8;  S = B_EA0;
                            }
                        } else {
                            g_5318d21c = B_070;  g_5318da7c = B_044;
                            aspectMult = B_F58;  S = B_EA0;
                        }
                    } else {
                        g_5318d21c = B_064;  g_5318da7c = B_044;
                        aspectMult = B_EB8;  S = B_EA0;
                    }
                } else {
                    g_5318d21c = B_064;  g_5318da7c = B_048;
                    aspectMult = B_EA4;  S = B_EA4;
                }
            } else {
                g_5318d21c = B_FA8;  g_5318da7c = EPH_HALF;
                aspectMult = B_EA0;  S = B_EA4;
            }
        } else {
            g_5318d21c = B_FD8;  g_5318da7c = B_FA8;
            aspectMult = B_E8C;  S = B_EA4;
        }
    } else {
        g_5318d21c = B_FF0;  g_5318da7c = B_FD8;
        aspectMult = B_E78;  S = B_EA4;
    }

    g_5318f038 = aspectMult;

    if (!g_widescreenEnabled) {
        S = EPH_ONE;
        dVar3 = (double)(displayH / EPH_STOCK_HEIGHT) * 0.4 *
                ((double)idx + (double)eph_table_lookup(idx));
    } else {
        if (hudScalePercent != 0.0f) {
            S = (hudScalePercent + eph_table_lookup(idx)) / EPH_HUNDRED;
        }
        dVar3 = ((double)displayH / ((double)S * 10.0)) * 0.4 *
                ((double)idx + (double)eph_table_lookup(idx));
    }

    g_0097a910 = (float)(dVar3 / EPH_HUNDRED);
    gameRenderW = (EPH_STOCK_WIDTH * S * aspectMult) / EPH_ASPECT_4_3;
    gameRenderH = EPH_STOCK_HEIGHT * S;

    g_5318efa4 = eph_round_like_dll(EPH_ONE * S);
    g_5318ee38 = eph_round_like_dll((EPH_ONE * gameRenderW) / EPH_STOCK_WIDTH);

    g_0096e114 *= S;
    g_006f49fd *= S;
    g_006f4a57 *= S;
    g_0096e168 *= S;
    g_0096e16c *= S;
    g_0096e170 *= S;
    g_0096e174 *= S;
    g_0096e178 *= S;
    g_0096e17c *= S;

    hudScaleFactor = S;
}
