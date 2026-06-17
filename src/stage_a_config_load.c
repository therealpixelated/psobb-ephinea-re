/*
 * ephinea.dll — Widescreen Cascade: Stage A
 * Original: FUN_52da9930 @ 0x52DA9930
 *
 * Loads Ephinea config from HKLM\\Software\\SonicTeam\\PSOBB\\Ephinea,
 * resolves display size from NEW_RES / resolution table, applies SSAA,
 * then tail-calls Stage B.
 *
 * Recovered from reference/ephinea_decompiled_PRIORBUILD.c:132304-133352
 */

#include "cascade_constants.h"
#include "ephinea_widescreen.h"
#include "registry_strings.h"
#include <windows.h>

extern float displayW;
extern float displayH;
extern int   g_widescreenEnabled;   /* _DAT_538785a4 — LOW_PERF gate */
extern int   g_hudScaleIndex;       /* _DAT_5388f9c0 — RES_HUD 0-4 */
extern int   g_displayMode;         /* _DAT_5318e940 */
extern int   g_vsyncEnabled;        /* _DAT_5318eed4 */
extern int   g_frameLimit;          /* _DAT_5318f240 */
extern int   g_aaQuality;           /* _DAT_5318f028 */
extern int   g_textureFilter;       /* _DAT_5318dcfc */
extern int   g_mipmapEnabled;       /* _DAT_53835c5c */
extern int   g_resolutionIndex;     /* _DAT_53a9895c */
extern int   g_ssaaFactor;          /* _DAT_5387b938 */
extern int   g_renderPath;          /* _DAT_53870578 */
extern int   g_numPlayers;          /* _DAT_53a9027c */
extern int   g_languageId;          /* _DAT_5388ed90 */
extern float hudScalePercent;       /* _DAT_5382fe9c */
extern int   widescreenEnabled;     /* _DAT_53857a5c */

static int g_resolutionPick;        /* _DAT_5388f988 */

static DWORD read_dword_reg(HKEY hKey, LPCWSTR name, DWORD defaultVal)
{
    DWORD value = defaultVal;
    DWORD cb = sizeof(value);
    if (RegQueryValueExW(hKey, name, NULL, NULL, (LPBYTE)&value, &cb) != ERROR_SUCCESS)
        return defaultVal;
    return value;
}

static void apply_resolution_index(DWORD idx)
{
    /* decompile:132497-132515 clamp + remap */
    if (idx > 0x13) idx = 0x13;
    if (idx < 2) idx = 0;
    g_resolutionIndex = (int)idx;
    if (g_resolutionIndex == 3) g_resolutionIndex = 2;
    if (g_resolutionIndex == 6) g_resolutionIndex = 5;

    /* SSAA switch when index > 4 (decompile:132516-132583) */
    g_ssaaFactor = 0;
    if (g_resolutionIndex > 4) {
        int bucket = g_resolutionIndex - 5;
        switch (bucket) {
        case 0: case 1: g_ssaaFactor = 2; g_resolutionIndex = 0; break;
        case 2:         g_ssaaFactor = 4; g_resolutionIndex = 0; break;
        default:        g_resolutionIndex = 0; break;
        }
    }
}

static void resolve_display_size(void)
{
    const float *res = (const float *)(uintptr_t)(0x78480000u + 0x00D88200u);

    g_resolutionPick = g_resolutionIndex;
    if (g_resolutionPick > 0x19) g_resolutionPick = 0;

    displayW = res[g_resolutionPick * 2];
    displayH = res[g_resolutionPick * 2 + 1];

    if (g_ssaaFactor != 0) {
        displayW *= (float)g_ssaaFactor;
        displayH *= (float)g_ssaaFactor;
    }
}

int stage_a_load_config(void)
{
    HKEY hKey = NULL;
    DWORD v;
    int result = 0;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KEY_EPHINEA, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return 1;
    }

    v = read_dword_reg(hKey, VAL_LOW_PERF, 0);
    g_widescreenEnabled = (v > 0) ? 1 : 0;

    v = read_dword_reg(hKey, VAL_RES_HUD, 0);
    g_hudScaleIndex = (v > 4) ? 0 : v;

    v = read_dword_reg(hKey, VAL_DGV_PRESENTATION, 0);
    g_displayMode = (v > 7) ? 0 : (int)v;

    v = read_dword_reg(hKey, VAL_VSYNC, 0);
    g_vsyncEnabled = (v > 0) ? 1 : 0;

    v = read_dword_reg(hKey, VAL_FRAME_LIMIT, 0);
    g_frameLimit = (v > 3) ? 0 : (int)v;

    v = read_dword_reg(hKey, VAL_MSAA, 0);
    g_aaQuality = (v > 0) ? 1 : 0;

    v = read_dword_reg(hKey, VAL_TEXTURE_FILTER, 1);
    g_textureFilter = (v > 0) ? 1 : 0;

    v = read_dword_reg(hKey, VAL_DISABLE_MIPMAPS, 0);
    g_mipmapEnabled = (v > 0) ? 0 : 1; /* inverted semantics */

    v = read_dword_reg(hKey, VAL_RENDER_MODE, 0);
    g_renderPath = (v > 0) ? 1 : 0;

    v = read_dword_reg(hKey, VAL_NEW_RES, 0);
    apply_resolution_index(v);

    /* HUD scale percent 100..450 (decompile:132687-132695) */
    v = read_dword_reg(hKey, VAL_HUD_SCALE, 0);
    if (v != 0) {
        if (v < 100) v = 100;
        if (v > 0x1C2) v = 0x1C2;
        hudScalePercent = (float)v;
    }

    v = read_dword_reg(hKey, VAL_WIDESCREEN, 0);
    widescreenEnabled = (v > 1) ? 1 : (int)v;
    if (displayW < 800.0f) /* _DAT_53176138 threshold — stub */
        widescreenEnabled = 0;

    v = read_dword_reg(hKey, VAL_LANG, 0);
    g_languageId = (v > 1) ? 1 : (int)v;

    RegCloseKey(hKey);

    /* Secondary HKLM key for player blob (decompile:133322-133345) */
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KEY_PSOBB, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD players[9] = {0};
        DWORD cb = sizeof(players);
        if (RegQueryValueExW(hKey, L"Players", NULL, NULL, (LPBYTE)players, &cb) == ERROR_SUCCESS) {
            g_numPlayers = (int)players[5];
            if (g_numPlayers > 10) g_numPlayers = 10;
        }
        RegCloseKey(hKey);
    } else {
        result |= 0x10;
    }

    resolve_display_size();
    stage_b_compute_render_size();
    return result;
}
