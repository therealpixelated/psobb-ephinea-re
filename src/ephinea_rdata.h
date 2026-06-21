/* Verified .rdata constants for the *current* unpacked build (base 0x78480000).
 * Extracted with tools/extract_rdata.py from ephinea_unpacked_base0x78480000.bin.
 *
 * Prior-build Ghidra VAs (0x5317xxxx) are rebased via PRIOR_IMAGE_BASE when noted. */

#ifndef EPHINEA_RDATA_H
#define EPHINEA_RDATA_H

#include <stdint.h>

#define EPHINEA_IMAGE_BASE       0x78480000u
#define PRIOR_IMAGE_BASE         0x523BE040u   /* maps 0x53146240 → res table */

/* Resolution table: 20 × {f32 W, f32 H} */
#define EPH_RVA_RES_TABLE        0x00D88200u

/* Stock geometry (current build RVAs) */
#define EPH_RVA_STOCK_WIDTH      0x00DB8130u   /* 640.0f */
#define EPH_RVA_STOCK_HEIGHT     0x00DB8128u   /* 480.0f */
#define EPH_RVA_ASPECT_4_3       0x00DB7E94u   /* 1.3333334f */
#define EPH_RVA_ONE              0x00DB7E6Cu   /* 1.0f */
#define EPH_RVA_HALF             0x00DB7F34u   /* 2.0f */
#define EPH_RVA_HUNDRED          0x00DB7FC8u   /* 100.0 (f64) */

/* FUN_52dabbd0 breakpoint compares (doubles; prior VA → current RVA) */
#define EPH_RVA_BP0              0x00DB7EA8u   /* prior 0x53175EE8, 0.225 — verify semantics */
#define EPH_RVA_BP1              0x00DB7EC0u   /* prior 0x53175F00, 0.611111 */
#define EPH_RVA_BP2              0x00DB7ED0u   /* prior 0x53175F10, 1.0 */
#define EPH_RVA_BP3              0x00DB7ED8u   /* prior 0x53175F18, 1.01 */
#define EPH_RVA_BP4              0x00DB7EF0u   /* prior 0x53175F30, 1.3 */
#define EPH_RVA_BP5              0x00DB7F00u   /* prior 0x53175F40, 1.4 */
#define EPH_RVA_BP6              0x00DB7F10u   /* prior 0x53175F50, 1.5 */

static inline float eph_read_f32(uint32_t rva)
{
    return *(const float *)(uintptr_t)(EPHINEA_IMAGE_BASE + rva);
}

static inline double eph_read_f64(uint32_t rva)
{
    return *(const double *)(uintptr_t)(EPHINEA_IMAGE_BASE + rva);
}

#endif /* EPHINEA_RDATA_H */
