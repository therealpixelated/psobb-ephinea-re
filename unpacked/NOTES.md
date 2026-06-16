# Unpacked image — `ephinea_unpacked_base0x78480000.bin`

A full **runtime memory dump of the unpacked `ephinea.dll`**, captured from a live
EphineaPSO process *after* the `.banana` packer finished self-decrypting. This is
the plaintext code/data the packed `../ephinea.dll` turns into at load time. It is
a 1:1 unpack of the exact packed DLL in this repo (same build — see
`../SHA256SUMS.txt`). Nothing is carved out, so the statically-linked ffmpeg is
present too — but **ffmpeg is not a target; ignore it.** The target is Ephinea's
own patch code against `PsoBB.exe`.

## Load parameters (important)

| field | value |
|---|---|
| File | `ephinea_unpacked_base0x78480000.bin` |
| Size | `0x031E6000` (52,322,304 bytes) = SizeOfImage |
| Layout | **offset == RVA** (zero-filled where pages weren't committed) |
| **Image base to use** | **`0x78480000`** |
| Bitness | 32-bit x86 |

ASLR put the module at `0x78480000` *this run*, and the packer applied its
relocations/rel32 fixups to that base — so the code's absolute addresses are
baked to `0x78480000`. **Load it as a raw binary with image base `0x78480000`**
(IDA: "binary file" → segment base `0x78480000`; Ghidra: import "Raw Binary",
language `x86:LE:32:default`, base `0x78480000`; Binary Ninja: open as raw, set
base). The DOS/PE header is present at offset 0 if you prefer to carve sections,
but its `ImageBase` field still reads the preferred `0x10000000` — ignore it and
use `0x78480000`.

> A *different* run will relocate to a different base. If you re-dump it yourself
> (`tools/dump_unpacked.py`), use the base that run reports.

## Sanity markers (verified in this dump)

| what | RVA | VA (@base 0x78480000) |
|---|---|---|
| `MZ` / DOS stub `This program cannot…` | `0x000000` / `0x00004e` | `0x78480000` / `0x7848004e` |
| `.banana` section name (in PE header) | `0x0003a8` | `0x784803a8` |
| ffmpeg `libavcodec` | `0xa2dda8` | `0x78eadda8` |
| ffmpeg `avcodec_open2` | `0xa2db39` | `0x78eadb39` |
| ffmpeg `Lavc59.39.100` (= FFmpeg 5.1.x) | `0xa4dadc` | `0x78ecdadc` |
| ffmpeg `binkvideo` | `0xa3b09c` | `0x78ebb09c` |
| msys64 build path | `0xa366a3` | `0x78eb66a3` |
| C++ RTTI `.?AVtype_info` | `0xdd26b8` | `0x792526b8` |

Entropy: 173/400 windows are zeroed (uninitialized `.bss`-style gaps), avg ≈ 6.18,
~45 windows still > 7.0 (compressed ffmpeg tables / leftover packed blobs) — i.e.
genuinely unpacked code+data, not still-encrypted.

## What's inside (high level)

- **THE TARGET:** the Ephinea engine-patch code that hooks `PsoBB.exe` — the
  detour installers, the per-screen layout/scale cascade, HUD/font patches,
  menu/input/camera tuning, local co-op, networking, localization, and the rest of
  the QoL changes. Recovering and naming all of this in clean C is the whole job.
- Statically-linked FFmpeg (libavcodec 59.39.100) — off-the-shelf video library,
  **not a target, ignore it.**
- Standard MSVC CRT + C++ runtime, zlib, etc. — recognizable library code, also
  not the target.

It is **stripped** — no symbols, just `sub_xxxx` / `dword_xxxx`. Recovering meaning
is the point. `../reference/ephinea_decompiled_PRIORBUILD.c` is a Ghidra decompile
of a *slightly older* build (40 KB smaller) — useful as a head start, but
addresses are at a different base and a few functions differ; regenerate from this
bin for exactness.
