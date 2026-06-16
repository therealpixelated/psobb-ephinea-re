import struct, sys, os

path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ephinea.dll")
d = open(path, 'rb').read()
base = 0x10000000
e_lfanew = struct.unpack_from('<I', d, 0x3C)[0]
coff = e_lfanew + 4
nsec, optsz = struct.unpack_from('<H', d, coff+2)[0], struct.unpack_from('<H', d, coff+16)[0]
opt = coff + 20
sec_off = opt + optsz
secs=[]
for i in range(nsec):
    o = sec_off + i*40
    name = d[o:o+8].rstrip(b'\0')
    vsz,vaddr,rsz,roff = struct.unpack_from('<IIII', d, o+8)
    secs.append((name,vaddr,vsz,roff,rsz))
def r2o(rva):
    for name,vaddr,vsz,roff,rsz in secs:
        if vaddr <= rva < vaddr+max(vsz,rsz): return roff+(rva-vaddr), name
    return None,None

ddir = opt + 0x60
imp_rva, imp_sz = struct.unpack_from('<II', d, ddir+1*8)
exp_rva, exp_sz = struct.unpack_from('<II', d, ddir+0*8)
print(f"IMPORT dir rva=0x{imp_rva:x} size=0x{imp_sz:x}  -> {r2o(imp_rva)[1]}")
print(f"EXPORT dir rva=0x{exp_rva:x} size=0x{exp_sz:x}  -> {r2o(exp_rva)[1]}")

off,_ = r2o(imp_rva)
print("\n-- IMPORT descriptors (raw) --")
for k in range(40):
    desc = d[off+k*20: off+k*20+20]
    if len(desc) < 20: break
    oft, ts, fwd, nameRva, firstThunk = struct.unpack('<IIIII', desc)
    if oft==0 and nameRva==0 and firstThunk==0:
        print(f"  [{k}] NULL terminator"); break
    no,_ = r2o(nameRva)
    nm = b''
    if no is not None and 0 <= no < len(d):
        end = d.find(b'\0', no); nm = d[no:end] if end>0 else d[no:no+32]
    printable = all(32<=c<127 for c in nm) and len(nm)>0
    print(f"  [{k}] nameRva=0x{nameRva:x} -> {nm if printable else b'<<non-ascii/encrypted>>'} (OFT=0x{oft:x} FT=0x{firstThunk:x})")

# entropy sample at import dir vs at the stub
import math
from collections import Counter
def ent(b):
    if not b: return 0
    c=Counter(b); n=len(b); return -sum((v/n)*math.log2(v/n) for v in c.values())
print(f"\nentropy @ import-dir region (512B) = {ent(d[off:off+512]):.3f}")
ep_off=0xc104dc
print(f"entropy @ EP stub region   (512B) = {ent(d[ep_off:ep_off+512]):.3f}")
