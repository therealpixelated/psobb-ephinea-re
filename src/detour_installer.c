/*
 * ephinea.dll — Detour installer family (Phase 2 RE)
 *
 * Recovered from reference/ephinea_decompiled_PRIORBUILD.c:
 *   FUN_52dc3460 @ 0x52DC3460 — NOP-sled overwrite (0x90 fill)
 *   FUN_52dc3240 @ 0x52DC3240 — 5-byte E8 rel32 CALL detour
 *   FUN_52dc3290 @ 0x52DC3290 — 5-byte E9 rel32 JMP detour
 *
 * Both installers optionally call FUN_52dc3460 first to NOP out `patch_len`
 * bytes at the engine site (when patch_len != 0), then write a 5-byte
 * relative branch to `handler` in PsoBB.exe .text.
 *
 * Full install inventory: data/detour_installs.csv (341 call sites in prior
 * decompile; docs/PATCHES_OVERVIEW.md cites ~350 total).
 */

#include <stdint.h>

typedef void (*eph_patch_nop_fn)(uint8_t *site, int len);
typedef void (*eph_install_call_fn)(void *handler, uint8_t *site, int patch_len);
typedef void (*eph_install_jmp_fn)(void *handler, uint8_t *site, int patch_len);

/* FUN_52dc3460 — fill [site, site+len) with 0x90 NOPs */
void eph_patch_nop_sled(uint8_t *site, int len)
{
    for (; len != 0; len--) {
        *site++ = 0x90;
    }
}

/* FUN_52dc3240 — E8 rel32 call detour (used for "call hook" sites) */
void eph_install_call_detour(void *handler, uint8_t *site, int patch_len)
{
    if (patch_len != 0) {
        eph_patch_nop_sled(site, patch_len);
    }
    *site = 0xE8;
    *(int32_t *)(site + 1) = (int32_t)((uintptr_t)handler - (uintptr_t)site - 5);
}

/* FUN_52dc3290 — E9 rel32 jmp detour (used for "replace entry" sites) */
void eph_install_jmp_detour(void *handler, uint8_t *site, int patch_len)
{
    if (patch_len != 0) {
        eph_patch_nop_sled(site, patch_len);
    }
    *site = 0xE9;
    *(int32_t *)(site + 1) = (int32_t)((uintptr_t)handler - (uintptr_t)site - 5);
}

/*
 * Install statistics (prior-build decompile scrape):
 *   FUN_52dc3240 (E8): 155 sites
 *   FUN_52dc3290 (E9): 186 sites
 *   patch_len histogram: 195×5, 39×6, 30×7, 20×10, 6×11, plus long patches to 73
 *
 * Engine VAs are 0x0040xxxx–0x0083xxxx (PsoBB.exe). Handlers live in ephinea.dll
 * 0x52D8xxxx–0x52F8xxxx (prior base) — regenerate handler RVAs from current
 * unpacked image before wiring a live patcher.
 */
