# psobb-ephinea-re

**Goal: recover the full C source of `ephinea.dll` and deobfuscate every patch it
applies to the PSOBB client.**

`ephinea.dll` is the engine-patch module shipped with the *Phantasy Star Online:
Blue Burst* private server **Ephinea**. At load time it installs hundreds of
runtime patches/detours into `PsoBB.exe` (the stock game client). This repo exists
to reverse all of that back into clean, readable, named C — every hook, every
patched constant, every behavioral change Ephinea makes to the client.

It ships the custom-packed original plus a fully **unpacked** image so the work can
start immediately.

> ffmpeg is statically linked into the DLL but is **not a target** — it's an
> off-the-shelf library for cutscene video. Ignore it. The target is Ephinea's
> own patch code against `PsoBB.exe`.

## The objective, concretely

1. **Unpack** `ephinea.dll` (custom `.banana` packer). Already done for you in
   `unpacked/`; the bootstrap is documented in [`docs/PACKER.md`](docs/PACKER.md)
   if you want to reproduce it.
2. **Decompile** the unpacked image (Ghidra / IDA / Hex-Rays / Binary Ninja).
3. **Enumerate every patch into the client**: each detour installed into
   `PsoBB.exe`'s `.text`, each overwritten constant in its `.data`, each
   replaced/added function. ephinea.dll installs these via two installer families
   (5-byte `E9/E8` detours + variable-length inline patches) — find all of them.
4. **Deobfuscate / recover clean C** for each patch: name the function, document
   what client behavior it changes (widescreen/scaling, HUD layout, font, menus,
   input, camera, local co-op, networking, localization, QoL), and produce
   compilable/readable source.

The end state is a complete, annotated C reconstruction of what Ephinea does to the
stock client.

## ✅ Status — full hook enumeration done (2026-06-20)

**[`EPHINEA_HOOKS_FULL_RE.md`](EPHINEA_HOOKS_FULL_RE.md)** is the completed objective #3+#4:
a static reverse-engineering of **956 distinct hooks** Ephinea installs into `PsoBB.exe`
(build 59NL / MTethVer12513), each named, classified by subsystem, and cross-referenced
against the stock client. It covers the full 3-stage widescreen cascade + the 4 per-draw
detours, every byte-delta patch, a completeness audit, and adversarial verification of
every widescreen-relevant finding (130 confirmed / 162 revised / 46 refuted).

Supporting data in [`hooks_re/`](hooks_re/):
- [`hook_inventory_table.md`](hooks_re/hook_inventory_table.md) — the full 956-hook table grouped by subsystem (with a `verify` verdict column).
- [`cascade_deepdives.md`](hooks_re/cascade_deepdives.md) — deep-dives of the 8 load-bearing widescreen functions.
- [`data/`](hooks_re/data/) — raw inputs/outputs: the detour list, the stock-vs-Ephinea-vs-ours byte delta CSV, cascade specs, the completeness critic, and the adversarial verdicts.

Built statically from the artifacts in this repo (no live process). Caveat: the `_DAT_5317xxxx`
constant pool lives in the packer-relocated `.rdata`, so formula *structure* is verified but a
handful of literal float values are triangulated, not byte-read — flagged in the doc's §6.

## What's in the repo

```
EPHINEA_HOOKS_FULL_RE.md          # ⭐ the completed hook RE (objective #3/#4)
hooks_re/                         # supporting tables, deep-dives, raw data
ephinea.dll                       # the PACKED original (current build)
unpacked/
  ephinea_unpacked_*.bin          # fully UNPACKED runtime image (base 0x78480000)
  NOTES.md                        # load params + how to import into a disassembler
docs/
  PACKER.md                       # static analysis of the .banana unpacker (step 1)
  PATCHES_OVERVIEW.md             # current map of the patches into PsoBB.exe (work in progress)
  RE_QUALITY_REVIEW.md            # fork merge critique + gap analysis (2026-06-17)
  DETOUR_INSTALLERS.md            # Phase 2: FUN_52dc3240/3290 detour families
  STAGE_C_RE.md                   # Stage C bulk loops + pointer table RE
  STAGE_A_RE.md                   # Stage A registry keys + value-name RE
src/                              # clean C reconstruction (widescreen cascade + installers)
  ephinea_widescreen.h / *.c        # Stages A–C, 92-VA, detour handlers (see RE_QUALITY_REVIEW)
  detour_installer.c              # NOP + E8/E9 patch primitives
  ephinea_rdata.h                 # verified .rdata RVAs for current unpacked build
data/
  detour_installs.csv             # 341 install sites from prior decompile
  detour_installs_current.csv     # + rebased handler VAs (rebase_detours.py)
  registry_strings.csv            # registry value-name map
  stage_c_tables.h                # generated pointer tables (in src/, from extract_stage_c.py)
  cascade_constants.h             # in src/ — stock scalars + breakpoint ladder
reference/
  ephinea_decompiled_PRIORBUILD.c # raw Ghidra C of a slightly older build — the starting point to clean up
tools/
  pe_probe.py / pe_stage2.py / pe_imports.py   # PE + packer inspection
  locate_in_process.py / dump_unpacked.py      # how the unpacked image was produced
SHA256SUMS.txt
```

- The unpacked image is **stripped** (`sub_xxxx` / `dword_xxxx`) — recovering
  meaning is the entire job.
- `reference/ephinea_decompiled_PRIORBUILD.c` is machine-generated C from a near-
  identical earlier build: a head start, not the answer. Regenerate from the
  current `unpacked/` bin for exactness.
- `docs/PATCHES_OVERVIEW.md` is the running inventory of identified patches —
  extend it as you reverse more.

## How the unpacked image was produced

The packer self-decrypts at `DLL_PROCESS_ATTACH`, so the in-memory image is
plaintext once the game runs. The dump is a straight `ReadProcessMemory` of the
module range from a live process (`tools/dump_unpacked.py`); the ASLR base is found
with `tools/locate_in_process.py`. No patching of the running game.

## Integrity

The packed `ephinea.dll` and the unpacked `.bin` are the same build; hashes in
[`SHA256SUMS.txt`](SHA256SUMS.txt).

## Legal / ethics

*Phantasy Star Online* and its assets are © SEGA; the Ephinea client is the
property of the Ephinea project. This repository is for reverse-engineering study
and education only.
