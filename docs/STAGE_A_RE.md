# Stage A registry loader — RE notes

**Function:** `FUN_52da9930` @ prior `0x52DA9930`  
**Implementation:** `src/stage_a_config_load.c`

## Registry keys (verified UTF-16 in unpacked .rdata)

| Macro | RVA | String |
|---|---|---|
| `KEY_EPHINEA` | `0x00D81208` | `Software\SonicTeam\PSOBB\Ephinea` |
| `KEY_PSOBB` | `0x00D81FE4` | `Software\SonicTeam\PSOBB` |

Value-name strings are generated into `src/registry_strings.h` by `tools/extract_registry.py`.

## Mapped values (first pass — 15 HKLM reads)

| Value macro | Decompile global | Notes |
|---|---|---|
| `LOW_PERF` | `_DAT_538785a4` | boolean gate |
| `RES_HUD` | `_DAT_5388f9c0` | HUD preset index 0–4 |
| `DGV_PRESENTATION` | `_DAT_5318e940` | display mode 0–7 |
| `DGV_VRAM` | `_DAT_5318eed4` | vsync |
| `CLASSIC_INTRO` | `_DAT_5318f240` | frame limit 0–3 |
| `MSAA_QUALITY` | `_DAT_5318f028` | AA on/off |
| `USE_BLOOM` | `_DAT_5318dcfc` | texture filter (name uncertain) |
| `DISABLE_MIPMAPS` | `_DAT_53835c5c` | inverted mipmap flag |
| `USE_D3D9` | `_DAT_53870578` | render path |
| `NEW_RES` | `_DAT_53a9895c` | resolution index + SSAA switch |
| `HUD_SCALE` | `_DAT_5382fe9c` | percent 100–450 |
| `CLASSIC_FULLSCREEN` | `_DAT_53857a5c` | widescreen on/off |
| `LANG` | `_DAT_5388ed90` | language id |

## Resolution table + SSAA

- Table at RVA `0x00D88200` (20 × `{f32 W, f32 H}` pairs)
- Index remaps per decompile: `<2→0`, `3→2`, `6→5`
- Indices `>4` trigger SSAA factor 2 or 4 and reset index

## `DAT_531761e0` int→double bias

Extracted to `cascade_constants.h` as `eph_i32_to_f64()`. Used in Stage B `dVar3` path and multiplayer scaling in Stage C.

## Remaining (~55 registry reads)

The decompile lists **70** `RegQueryValueEx` calls under the Ephinea key. Camera, input, audio, and QoL knobs live in the `0x00D819xx–0x00D81Fxx` string pool. Extend `data/registry_strings.csv` by correlating each `0x5313fxxx` pointer with its target global from the decompile.

## Regenerate

```bash
python3 tools/extract_registry.py
```
