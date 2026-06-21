# Stage C bulk apply — RE notes

**Function:** `FUN_52da7ff0` @ prior `0x52DA7FF0`  
**Status:** structure recovered; pointer tables dumped from current unpacked build.

## Six bulk loops (verified iteration counts)

| Loop | Prior table | Count | Operation |
|---|---|---|---|
| W assign | `0x5318F040` | 122 (`0x7A`) | `*ptr = gameRenderW` |
| H assign | `0x5318E6E0` | 90 (`0x5A`) | `*ptr = gameRenderH` |
| W += half | `0x5318E650` | 35 (`0x23`) | `*ptr += (W-640)/2` |
| W += full | `0x5318F2A0` | 28 (`0x1C`) | `*ptr += (W-640)` |
| H += full | `0x5318E9C8` | 235 (`0xEB`) | `*ptr += (H-480)` |
| H += half | `0x5318EF00` | 12 (`0xC`) | `*ptr += (H/2 - offset)` |

Each table slot is a **`float*` stored in ephinea.dll .data** pointing at a PsoBB.exe `.data` global. The generated arrays in `data/stage_c_tables.h` list the pointer values from the static unpacked image.

### Table coverage (current build dump)

| Table | Engine-range pointers |
|---|---|
| `kAnchorW` | 110 / 122 |
| `kAnchorH` | 89 / 90 |
| `kAnchorWaddHalf` | 24 / 35 |
| `kAnchorWaddFull` | 22 / 28 |
| `kAnchorHaddFull` | 233 / 235 |
| `kAnchorHaddHalf` | 6 / 12 |

Non-engine slots are dll/runtime pointers (uninitialized at dump time or indirect) — `stage_c_apply_anchors.c` skips them via `eph_is_engine_va()`.

## Regenerate tables

```bash
python3 tools/extract_stage_c.py
```

Produces:

- `data/stage_c_tables.h` — all pointer tables
- `data/cascade_constants.h` — stock scalars + breakpoint ladder

## Implementation

`src/stage_c_apply_anchors.c` now mirrors the decompile call order:

1. ~40 scalar writes (lines 131912–131953)
2. 12× aspectMult
3. Six bulk loops (table-driven)
4. W/H proportional scale + ushort scale
5. Color constants, geometry recompute, multiplayer scaling
6. `stage_c_92va_apply()` — 97 arithmetic writes
7. ~25 post-92-VA edge writes

Remaining gaps:

- `DAT_531761e0` lookup table for Stage B `dVar3` / `_DAT_0097a910`
- D3D9 display-mode enumeration block (language fallback)
- 36 allocator writes (`FUN_52d46df0`) — side effects only, not layout math
