"""
Dump the UNPACKED ephinea.dll image out of a live EphineaPSO process via
ReadProcessMemory. The packer self-decrypts at DLL_PROCESS_ATTACH, so once the
game is running the in-memory image is plaintext.

Usage:  python tools/dump_unpacked.py <PID> <BASE_HEX> [out.bin]
  PID       = pid of the running PsoBB.exe (Ephinea)
  BASE_HEX  = base address ephinea.dll loaded at (ASLR-randomized each run).
              Find it with tools/locate_in_process.py <PID>, which reports the
              IMAGE allocation that contains '.banana' + 'libavcodec'.
Example:    python tools/dump_unpacked.py 32688 0x78480000 ephinea_unpacked.bin

The bin is written zero-filled at offset==RVA, size = SizeOfImage (0x31e6000).
"""
import ctypes, sys, math
from ctypes import wintypes
from collections import Counter

PID  = int(sys.argv[1]) if len(sys.argv) > 1 else 0
BASE = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x78480000
SIZE = 0x031E6000           # SizeOfImage
OUT  = sys.argv[3] if len(sys.argv) > 3 else "ephinea_unpacked.bin"
if not PID:
    print(__doc__); sys.exit(1)

k32 = ctypes.WinDLL('kernel32', use_last_error=True)

PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ           = 0x0010

class MBI(ctypes.Structure):
    _fields_ = [
        ("BaseAddress",       ctypes.c_void_p),
        ("AllocationBase",    ctypes.c_void_p),
        ("AllocationProtect", wintypes.DWORD),
        ("__align",           wintypes.DWORD),
        ("RegionSize",        ctypes.c_size_t),
        ("State",             wintypes.DWORD),
        ("Protect",           wintypes.DWORD),
        ("Type",              wintypes.DWORD),
        ("__align2",          wintypes.DWORD),
    ]

k32.OpenProcess.restype = wintypes.HANDLE
k32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
k32.VirtualQueryEx.restype = ctypes.c_size_t
k32.VirtualQueryEx.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.POINTER(MBI), ctypes.c_size_t]
k32.ReadProcessMemory.restype = wintypes.BOOL
k32.ReadProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]

h = k32.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, False, PID)
if not h:
    print("OpenProcess failed", ctypes.get_last_error()); sys.exit(1)

img = bytearray(SIZE)            # zero-filled; committed pages overlaid -> offset == RVA
committed = 0
addr = BASE
end  = BASE + SIZE
MEM_COMMIT = 0x1000
while addr < end:
    mbi = MBI()
    if k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi)) == 0:
        addr += 0x1000; continue
    rbase = mbi.BaseAddress or addr
    rsize = mbi.RegionSize or 0x1000
    nxt = rbase + rsize
    if mbi.State == MEM_COMMIT:
        # clamp region to [BASE,end)
        rs = max(rbase, BASE); re = min(nxt, end)
        buf = (ctypes.c_char * (re-rs))()
        got = ctypes.c_size_t(0)
        if k32.ReadProcessMemory(h, ctypes.c_void_p(rs), buf, re-rs, ctypes.byref(got)):
            n = got.value
            img[rs-BASE:rs-BASE+n] = bytes(buf[:n])
            committed += n
    addr = nxt if nxt > addr else addr + 0x1000

open(OUT,'wb').write(img)
print(f"wrote {OUT}")
print(f"image span [0x{BASE:08x},0x{end:08x}) size 0x{SIZE:x} ({SIZE} bytes)")
print(f"committed+read = {committed} bytes ({committed/SIZE*100:.1f}% of image)")

def ent(b):
    if not b: return 0
    c=Counter(b); n=len(b); return -sum((v/n)*math.log2(v/n) for v in c.values())

# MZ/PE header sanity + section names
print("\nheader:", img[:2], "e_lfanew=0x%x"%int.from_bytes(img[0x3c:0x40],'little'))

# find ffmpeg + sanity markers, report as RVA
for needle in [b"Lavc59.39.100", b"libavcodec", b"msys64", b".banana", b"binkvideo",
               b"avcodec_open2", b"This program cannot", b".?AVtype_info"]:
    i = img.find(needle)
    print(f"  {needle!r:22} @ {('RVA 0x%06x (VA 0x%08x)'%(i, BASE+i)) if i>=0 else 'NOT FOUND'}")

# compact entropy summary
W=0x20000; zero=0; vals=[]
for off in range(0, SIZE, W):
    chunk = img[off:off+W]
    if any(chunk): vals.append(ent(chunk))
    else: zero+=1
print(f"\nentropy windows (0x{W:x}): nonzero={len(vals)} zero={zero} "
      f"min={min(vals):.2f} max={max(vals):.2f} avg={sum(vals)/len(vals):.2f}")
print("high-entropy (>7.0) windows = likely still-packed/compressed remnants:",
      sum(1 for v in vals if v>7.0))
