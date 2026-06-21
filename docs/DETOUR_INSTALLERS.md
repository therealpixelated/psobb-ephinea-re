# Detour installer families — Phase 2 RE

**Status:** mechanism verified from prior Ghidra decompile; install inventory scraped to
`data/detour_installs.csv`.

## The three primitives

| Function | Opcode | Effect |
|---|---|---|
| `FUN_52dc3460` | `0x90` × N | NOP-sled: erase `N` bytes at engine site before patching |
| `FUN_52dc3240` | `E8 rel32` | 5-byte **CALL** detour to ephinea handler |
| `FUN_52dc3290` | `E9 rel32` | 5-byte **JMP** detour to ephinea handler |

Both installers share the same prologue:

```c
if (patch_len != 0)
    FUN_52dc3460(site, patch_len);   // NOP out variable-length original insn(s)
*site = 0xE8 or 0xE9;
*(int32_t*)(site + 1) = handler - site - 5;
```

Clean C lives in `src/detour_installer.c`.

## Inventory (prior build decompile)

| Metric | Count |
|---|---|
| `FUN_52dc3240` (E8 CALL) | 155 |
| `FUN_52dc3290` (E9 JMP) | 186 |
| **Total install calls** | **341** |
| `patch_len == 5` | 195 |
| `patch_len == 6` | 39 |
| `patch_len == 7` | 30 |
| `patch_len == 10` | 20 |
| `patch_len == 11` | 6 |
| longest (`0x49` = 73) | 1 |

The CSV columns are: `installer`, `opcode`, `handler`, `engine_va`, `patch_len`.

### What this is NOT

- The 341 lines are **installer call sites** in ephinea.dll init — not the same as the
  2451-row engine byte delta (`docs/PATCHES_OVERVIEW.md` §3). Delta rows include data
  patches, reloc noise, and inline immediates never installed via these functions.
- The older "120 hooks file" is a **subset** of the E8/E9 layer (trampoline handlers at
  `0x52D8–0x52DE`), not the full surface.

## Next RE steps

1. Rebase handler addresses from prior decompile (`0x52Fxxxxx`) to current unpacked RVAs.
2. Bucket handlers by shared target (high fan-out localization family, HUD quad block, etc.).
3. Recover the init function that sequences all 341 installs (search for dense
   `FUN_52dc3240`/`3290` cluster in current unpacked `.text`).
4. For `patch_len > 5`, disassemble the NOP'd span at each engine VA to name the
   replaced instruction(s).
