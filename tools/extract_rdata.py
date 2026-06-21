#!/usr/bin/env python3
"""Extract widescreen cascade .rdata constants from the unpacked ephinea image.

Reads unpacked/ephinea_unpacked_base0x78480000.bin (offset == RVA) and emits
human-verified constants for the current build. Uses the resolution table as
an anchor to map prior-build Ghidra VAs when useful.
"""

from __future__ import annotations

import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BIN = ROOT / "unpacked" / "ephinea_unpacked_base0x78480000.bin"
IMAGE_BASE = 0x78480000

# Prior-build VA of resolution table row 0 (640x480), verified identical layout.
PRIOR_RES_TABLE = 0x53146240
CUR_RES_TABLE_RVA = 0x00D88200
PRIOR_BASE = PRIOR_RES_TABLE - CUR_RES_TABLE_RVA


def read_f32(data: bytes, rva: int) -> float:
    return struct.unpack_from("<f", data, rva)[0]


def read_f64(data: bytes, rva: int) -> float:
    return struct.unpack_from("<d", data, rva)[0]


def prior_to_rva(prior_va: int) -> int:
    return prior_va - PRIOR_BASE


def main() -> None:
    data = BIN.read_bytes()

    print(f"# ephinea.dll rdata extract (image base 0x{IMAGE_BASE:08X})")
    print(f"# PRIOR_BASE (for Ghidra prior-build VAs) = 0x{PRIOR_BASE:08X}\n")

    # Resolution table — pairs of f32 {W, H}
    print("## Resolution table @ RVA 0x{:08X}".format(CUR_RES_TABLE_RVA))
    for i in range(20):
        w = read_f32(data, CUR_RES_TABLE_RVA + i * 8)
        h = read_f32(data, CUR_RES_TABLE_RVA + i * 8 + 4)
        print(f"  [{i:2d}] {w:.0f}x{h:.0f}")

    # Stock geometry scalars (current-build RVAs found by float search)
    print("\n## Stock scalars (current build)")
    stock = {
        "STOCK_WIDTH": 0x00DB8130,
        "STOCK_HEIGHT": 0x00DB8128,
        "ASPECT_4_3": 0x00DB7E94,
        "ONE": 0x00DB7E6C,
        "HALF": 0x00DB7F34,
        "HUNDRED": 0x00DB7FC8,  # stored as f64=100 at this qword
    }
    for name, rva in stock.items():
        val = read_f64(data, rva) if name == "HUNDRED" else read_f32(data, rva)
        print(f"  {name:14s} RVA 0x{rva:08X} = {val:.8g}")

    # Breakpoint ladder — doubles at prior Ghidra addresses (structure verified)
    print("\n## Aspect breakpoint ladder (doubles @ prior FUN_52dabbd0 compares)")
    bp_names = ["bp0", "bp1", "bp2", "bp3", "bp4", "bp5", "bp6"]
    bp_prior = [
        0x53175EE8,
        0x53175F00,
        0x53175F10,
        0x53175F18,
        0x53175F30,
        0x53175F40,
        0x53175F50,
    ]
    for name, prior in zip(bp_names, bp_prior):
        rva = prior_to_rva(prior)
        print(f"  {name} prior 0x{prior:08X} rva 0x{rva:08X} = {read_f64(data, rva):.8g}")

    # Parallel float ladder (visually matches doc breakpoints 1.25..1.7)
    print("\n## Float aspect ladder (candidate doc values, contiguous block)")
    for rva in range(0x00DB7E80, 0x00DB7F28, 4):
        f = read_f32(data, rva)
        if 1.2 <= f <= 2.0:
            print(f"  RVA 0x{rva:08X} = {f:.6g}")


if __name__ == "__main__":
    main()
