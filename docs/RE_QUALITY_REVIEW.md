# RE Quality Review — llama-bob fork merge + gap analysis

**Date:** 2026-06-17  
**Scope:** Merged `llama-bob/psobb-ephinea-re` commit `9535385` into `cursor/merge-llama-bob-fork-981f`, cross-checked against `reference/ephinea_decompiled_PRIORBUILD.c`, `docs/PATCHES_OVERVIEW.md`, and a fresh static pass on `unpacked/ephinea_unpacked_base0x78480000.bin`.

---

## Executive verdict

The fork is a **strong scaffold**, not finished RE. It correctly encodes the cascade *architecture* and fixes several doc errors (L3=19, L4=20, Stage C ≠ 92-VA). But **only one file is production-grade** (`92va_apply.c`); the rest are outline/pseudocode with placeholders, wrong types, and would not compile. The session note claiming Phase 1 widescreen cascade **"100% complete"** overstates the work by a wide margin.

| Artifact | Honest completeness | Notes |
|---|---|---|
| `92va_apply.c` | **~95%** | VA list and factor classes match decompile; op order on dual-listed VAs is correct |
| `stage_b_render_size.c` | **~40%** | Ladder shape OK; bucket assignments diverge from decompile; many `/* placeholder */` |
| `stage_a_config_load.c` | **~25%** | Registry path pattern OK; ~50 values stubbed; resolution index remap incomplete |
| `stage_c_apply_anchors.c` | **~15%** | Six loop *counts* correct; body is mostly comments + undefined symbols |
| Detour handlers (4 files) | **~30%** | Right idea; missing includes, `undefined4`, incomplete call-through |
| `ephinea_widescreen.h` | **~50%** | Good extern map; `static` helper in header; `undefined4` undeclared |
| Detour installers | **0% → started here** | Now in `src/detour_installer.c` + `data/detour_installs.csv` |

---

## What the fork got right

### 1. Architecture matches verified static analysis

The three-stage cascade + per-draw detour layer described in `PATCHES_OVERVIEW.md` is reflected correctly:

- **Stage A** → registry/resolution → tail-calls **Stage B**
- **Stage B** → 7-level aspect ladder → `gameRenderW` / `gameRenderH` + 10 `*= S` extras
- **Stage C** → six bulk loops (122/90/35/28/235/12) → calls **92-VA** → more writes
- **Runtime detours** for vertex transform, HUD quads, UI edge, deanchor pin

### 2. Doc corrections are valid

Cross-checking `FUN_52da9280` in the prior decompile (lines 132130–132293):

- **L3 = 19** VAs (`local_100[0x10]` + `local_bc` + `local_b8`) — fork is correct
- **L4 = 20** VAs — fork is correct
- **L3/L4 divide by 2** (`_DAT_53175f20`) — fork is correct
- **L5 is raw** `(H-480)` — fork is correct
- **Inline pass runs after L3/L4** and re-touches `0x008FA1D8`, `0x008FA1D0`, `0x008FA1AC` — fork preserves this (required for parity)

### 3. `92va_apply.c` VA inventory matches decompile

All 93 distinct engine VAs from `PATCHES_OVERVIEW.md` §4 are present. The L1 array order in the fork differs from the decompile's stack layout but that does not affect semantics (independent `*=` ops).

---

## Critical quality issues

### 1. Stage C is not implemented — it is annotated pseudocode

`stage_c_apply_anchors.c` references symbols that do not exist anywhere in the repo:

- `anchor_table_6`, `anchor_table_6b`, `ANCHOR_ASPECTTABLE`, `ANCHOR_PLAYERSCALE`
- `g_009a3840` … `g_009a38d8` (24 allocator writes collapsed to a comment)
- `round_func`, `float_to_ushort`, `d3d9_enumfunc`, `match_string_in_list`, etc.

The six bulk loops are the right iteration counts but use a **simplified addressing model**. The decompile indexes tables of **pointers-to-engine-floats** at `0x5318F040` etc.; the fork treats those addresses as if they were contiguous float arrays. That may be wrong — each slot is likely a `float*` into PsoBB `.data`, not a dense `float[]`.

**Impact:** A port using only `92va_apply.c` will still be visibly wrong (already flagged in `PATCHES_OVERVIEW.md`). A port using this Stage C stub would not compile and would still be incomplete even if fixed naively.

### 2. Stage B bucket ladder does not match the decompile

The fork's `stage_b_render_size.c` invents bucket constant assignments (e.g. bucket 3 uses `bucketDelta[4]` for multiple fields). The prior decompile at `FUN_52dabbd0` (lines 133403–133462) assigns **specific** globals per branch:

| Bucket | `_DAT_5318d21c` | `_DAT_5318da7c` | `_DAT_5318f038` (aspectMult) | `local_8` (S) |
|---|---|---|---|---|
| 6 | `_DAT_53176064` | `_DAT_53175fa8` | `_DAT_53176000` | `_DAT_53175ea0` |
| 5 | `_DAT_5317607c` | `_DAT_53176044` | `_DAT_53175fc8` | `_DAT_53175ea0` |
| … | *(see decompile)* | | | |

The fork collapses these into generic `bucketDelta[i]` / `bucketScale[i]` array indexing with comments like `/* see bucketScale[5] variant */` — **not recovered**.

The `dVar3` / table-lookup path for `_DAT_0097a910` (lines 133465–133475) is entirely placeholder (`1.0f`).

### 3. Would not compile as-is

Examples:

| File | Issue |
|---|---|
| `ephinea_widescreen.h` | `undefined4`, `HKEY`, `LPCWSTR` without includes; `static read_dword_reg` in header |
| `vertex_aspect_correct.c` | Uses `gameRenderW`, `ASPECT_4_3`, `undefined4` — no includes |
| `92va_apply.c` | Calls `FUN_52dc3460` — no declaration |
| `deanchor_pin.c` | `_DAT_53175e64` — Ghidra name, not defined |
| `ui_edge_reanchor.c` | `*_DAT_5318d108` — invalid C syntax |

These files are **RE notes shaped like C**, not a buildable translation unit.

### 4. Address base confusion

Comments mix three conventions without marking which build they apply to:

1. **Engine VAs** (`0x008Fxxxx`, `0x0040xxxx`) — PsoBB.exe absolute VAs ✓ correct in 92-VA
2. **Prior ephinea.dll VAs** (`0x5317xxxx`, `0x52DAxxxx`) — from older dump / Ghidra
3. **Current unpacked base** (`0x78480000` + RVA) — per `unpacked/NOTES.md`

The prior-build → current-build rebase for `.rdata` is **`PRIOR_IMAGE_BASE = 0x523BE040`** (anchored on resolution table `0x53146240` → RVA `0x00D88200`). This mapping reproduces the 20-entry resolution table exactly but **does not land 640/480 at prior `0x53176128`** — those Ghidra symbols moved or overlap other tables in the current build. Use `src/ephinea_rdata.h` for verified current-build RVAs.

### 5. Breakpoint constants — semantics still partially unknown

Fresh extract from current unpacked image (`tools/extract_rdata.py`):

| Prior VA | Current RVA | f64 value |
|---|---|---|
| `0x53175EE8` | `0x00DB7EA8` | 0.225 |
| `0x53175F00` | `0x00DB7EC0` | 0.611111 |
| `0x53175F30` | `0x00DB7EF0` | 1.3 |
| `0x53175F40` | `0x00DB7F00` | 1.4 |
| `0x53175F50` | `0x00DB7F10` | 1.5 |

A contiguous **float** ladder at `0x00DB7E80` looks like documented aspect breakpoints (1.25, 1.3, 1.396, 1.4, 1.45, 1.5, 1.55, 1.7…) but the **doubles actually compared** in `FUN_52dabbd0` at the Ghidra-named addresses do not all match those doc values (first two are 0.225 and 0.611). This needs a focused re-decompile of Stage B on the **current** unpacked bin — do not trust the fork's placeholder `aspectBreakpoint[]` initializers.

---

## New work added in this merge follow-up

| Path | Purpose |
|---|---|
| `tools/extract_rdata.py` | Pull resolution table + stock scalars + breakpoint bytes from unpacked bin |
| `src/ephinea_rdata.h` | Current-build RVAs for 640/480/4:3/2/1/100 and breakpoint qwords |
| `src/detour_installer.c` | Clean C for `FUN_52dc3240` / `3290` / `3460` |
| `data/detour_installs.csv` | **341** install calls scraped from prior decompile |
| `docs/DETOUR_INSTALLERS.md` | Phase 2 RE notes + patch_len histogram |

---

## Recommended priority queue

1. **Re-decompile Stage B + Stage C on current `unpacked/*.bin`** at base `0x78480000` — prior build is 40 KB smaller; addresses drift.
2. **Finish Stage C loop bodies** — dump the six pointer tables at `0x5318F040` etc. from live ephinea `.data` and resolve each entry to an engine VA.
3. **Extract breakpoint / aspectMult tables** with semantics proof (match ladder branches to stored constants).
4. **Phase 2 detours** — rebase 341 handlers; bucket by fan-out (localization `FUN_52db4540` ×10 sites first).
5. **Font block** (`0x0097DB60..8C`) — atlas width 256→592 without texture swap is a known parity risk (`PATCHES_OVERVIEW.md` §3B).
6. **Do not chase** ffmpeg (confirmed out of scope) or `.text differ` relocation noise.

---

## Bottom line

Merging the fork was worthwhile: it centralizes the corrected 92-VA recipe and names the cascade cleanly. Treat it as **annotated pseudocode + one solid reference function**, not a completed Phase 1. The highest-risk gap remains **Stage C** (~700 writes), not the 92-VA layer the fork emphasizes.
