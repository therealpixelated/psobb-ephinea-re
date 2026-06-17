#!/usr/bin/env python3
"""Add current-build handler RVAs to detour_installs.csv."""

from __future__ import annotations

import csv
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "data" / "detour_installs.csv"
OUT = ROOT / "data" / "detour_installs_current.csv"
PRIOR_BASE = 0x523BE040
IMAGE_BASE = 0x78480000


def parse_handler(handler: str) -> int | None:
    m = re.match(r"(?:FUN_|&LAB_)([0-9A-Fa-f]+)", handler)
    if not m:
        return None
    return int(m.group(1), 16)


def main() -> None:
    rows = []
    with SRC.open(newline="") as f:
        for row in csv.DictReader(f):
            prior = parse_handler(row["handler"])
            if prior is None:
                rva = ""
                current = ""
            else:
                rva = prior - PRIOR_BASE
                current = f"0x{IMAGE_BASE + rva:08X}"
            row["handler_prior_va"] = f"0x{prior:08X}" if prior else ""
            row["handler_rva"] = f"0x{rva:08X}" if prior else ""
            row["handler_current_va"] = current
            rows.append(row)

    fields = list(rows[0].keys()) if rows else []
    with OUT.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)
    print(f"Wrote {len(rows)} rows to {OUT}")


if __name__ == "__main__":
    main()
