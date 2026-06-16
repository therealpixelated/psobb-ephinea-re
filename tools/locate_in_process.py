"""
Walk a live process's address space and report the committed allocations that
contain ephinea / ffmpeg marker strings, so you can find the unpacked
ephinea.dll IMAGE base (the one carrying '.banana' + 'libavcodec' + 'msys64').
Usage:  python tools/locate_in_process.py <PID>
"""
import ctypes, sys
from ctypes import wintypes

PID = int(sys.argv[1]) if len(sys.argv) > 1 else 0
if not PID:
    print(__doc__); sys.exit(1)
k32 = ctypes.WinDLL('kernel32', use_last_error=True)
PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ           = 0x0010
MEM_COMMIT = 0x1000
MEM_IMAGE  = 0x1000000
MEM_MAPPED = 0x40000
MEM_PRIVATE= 0x20000
PAGE_GUARD = 0x100
PAGE_NOACCESS = 0x01

class MBI(ctypes.Structure):
    _fields_ = [("BaseAddress",ctypes.c_void_p),("AllocationBase",ctypes.c_void_p),
                ("AllocationProtect",wintypes.DWORD),("__a",wintypes.DWORD),
                ("RegionSize",ctypes.c_size_t),("State",wintypes.DWORD),
                ("Protect",wintypes.DWORD),("Type",wintypes.DWORD),("__b",wintypes.DWORD)]

k32.OpenProcess.restype=wintypes.HANDLE
k32.OpenProcess.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.DWORD]
k32.VirtualQueryEx.restype=ctypes.c_size_t
k32.VirtualQueryEx.argtypes=[wintypes.HANDLE,ctypes.c_void_p,ctypes.POINTER(MBI),ctypes.c_size_t]
k32.ReadProcessMemory.restype=wintypes.BOOL
k32.ReadProcessMemory.argtypes=[wintypes.HANDLE,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.POINTER(ctypes.c_size_t)]

h=k32.OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,False,PID)
if not h: print("open fail",ctypes.get_last_error()); sys.exit(1)

needles=[b"Lavc",b"libavcodec",b"msys64",b"Ephinea",b"PSOBB",b"Pioneer",b".banana",
         b"This program cannot",b"binkvideo",b"smackvid",b"avcodec_open2",b"PsoBB"]

def readmem(base,size):
    size=min(size, 96*1024*1024)
    buf=(ctypes.c_char*size)()
    got=ctypes.c_size_t(0)
    if k32.ReadProcessMemory(h,ctypes.c_void_p(base),buf,size,ctypes.byref(got)):
        return bytes(buf[:got.value])
    return b""

addr=0; END=0x7fff0000
regions=[]
while addr < END:
    mbi=MBI()
    if k32.VirtualQueryEx(h,ctypes.c_void_p(addr),ctypes.byref(mbi),ctypes.sizeof(mbi))==0:
        addr+=0x1000; continue
    base=mbi.BaseAddress or addr; size=mbi.RegionSize or 0x1000
    if (mbi.State==MEM_COMMIT and not (mbi.Protect & (PAGE_GUARD|PAGE_NOACCESS)) and mbi.Protect!=0):
        regions.append((base,size,mbi.Type,mbi.Protect,mbi.AllocationBase or base))
    nxt=base+size
    addr = nxt if nxt>addr else addr+0x1000

# group by AllocationBase to see whole allocations
from collections import defaultdict
allocs=defaultdict(lambda:[0,0,set()])  # allocbase -> [total, type, protects]
for base,size,typ,prot,ab in regions:
    allocs[ab][0]+=size; allocs[ab][1]=typ; allocs[ab][2].add(prot)

typename={MEM_IMAGE:"IMAGE",MEM_MAPPED:"MAPPED",MEM_PRIVATE:"PRIVATE"}
print("== large committed allocations (>= 256KB) ==")
big=sorted(allocs.items(), key=lambda kv:-kv[1][0])
for ab,(tot,typ,prots) in big:
    if tot>=0x40000:
        print(f"  alloc 0x{ab:08x}  size 0x{tot:08x} ({tot//1024}KB)  {typename.get(typ,hex(typ))}")

print("\n== regions containing marker strings ==")
hits=defaultdict(set)
for base,size,typ,prot,ab in regions:
    data=readmem(base,size)
    if not data: continue
    for nd in needles:
        if nd in data:
            off=data.find(nd)
            hits[(ab,typ)].add(nd.decode('latin1'))
            print(f"  VA 0x{base+off:08x} (alloc 0x{ab:08x} {typename.get(typ,hex(typ))})  {nd.decode('latin1')!r}")

print("\n== summary: allocations with ffmpeg/game markers ==")
for (ab,typ),nds in sorted(hits.items()):
    print(f"  alloc 0x{ab:08x} {typename.get(typ,hex(typ))}: {sorted(nds)}")
