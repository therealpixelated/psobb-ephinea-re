import struct, math, sys, os

path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ephinea.dll")
d = open(path, 'rb').read()
print(f"file size = {len(d)} (0x{len(d):x})")

e_lfanew = struct.unpack_from('<I', d, 0x3C)[0]
assert d[e_lfanew:e_lfanew+4] == b'PE\0\0', "not PE"
coff = e_lfanew + 4
machine, nsec, tstamp, psym, nsym, optsz, chars = struct.unpack_from('<HHIIIHH', d, coff)
opt = coff + 20
magic = struct.unpack_from('<H', d, opt)[0]
is64 = magic == 0x20b
print(f"machine=0x{machine:x} sections={nsec} optmagic=0x{magic:x} ({'PE32+' if is64 else 'PE32'})")

aoe   = struct.unpack_from('<I', d, opt+16)[0]   # AddressOfEntryPoint
base  = struct.unpack_from('<I', d, opt+28)[0] if not is64 else struct.unpack_from('<Q', d, opt+24)[0]
salign= struct.unpack_from('<I', d, opt+32)[0]
falign= struct.unpack_from('<I', d, opt+36)[0]
sizeimg = struct.unpack_from('<I', d, opt+56)[0]
ddir_off = opt + (0x60 if not is64 else 0x70)
nddir = struct.unpack_from('<I', d, opt + (0x5C if not is64 else 0x6C))[0]
print(f"ImageBase=0x{base:x} EntryPointRVA=0x{aoe:x} -> VA=0x{base+aoe:x}")
print(f"SizeOfImage=0x{sizeimg:x} SectAlign=0x{salign:x} FileAlign=0x{falign:x} NumDataDirs={nddir}")

dnames = ['EXPORT','IMPORT','RESOURCE','EXCEPTION','SECURITY','BASERELOC','DEBUG','ARCH',
          'GLOBALPTR','TLS','LOAD_CONFIG','BOUND_IMPORT','IAT','DELAY_IMPORT','CLR','RSVD']
print("\n-- Data Directories --")
for i in range(min(nddir,16)):
    rva, sz = struct.unpack_from('<II', d, ddir_off + i*8)
    if rva or sz:
        print(f"  {dnames[i]:14} rva=0x{rva:08x} size=0x{sz:x}")

sec_off = opt + optsz
def entropy(b):
    if not b: return 0.0
    from collections import Counter
    c = Counter(b); n=len(b)
    return -sum((v/n)*math.log2(v/n) for v in c.values())

secs = []
print("\n-- Sections --")
print(f"  {'name':10} {'vaddr':>10} {'vsize':>10} {'raw_off':>10} {'raw_sz':>10} {'chars':>10} {'entropy':>8}")
for i in range(nsec):
    o = sec_off + i*40
    name = d[o:o+8].rstrip(b'\0')
    vsz, vaddr, rsz, roff = struct.unpack_from('<IIII', d, o+8)
    chrs = struct.unpack_from('<I', d, o+36)[0]
    raw = d[roff:roff+rsz]
    ent = entropy(raw[:min(len(raw), 2_000_000)])
    secs.append((name, vaddr, vsz, roff, rsz, chrs))
    try: nm = name.decode()
    except: nm = repr(name)
    print(f"  {nm:10} 0x{vaddr:08x} 0x{vsz:08x} 0x{roff:08x} 0x{rsz:08x} 0x{chrs:08x} {ent:8.3f}")

def rva_to_off(rva):
    for name,vaddr,vsz,roff,rsz,chrs in secs:
        if vaddr <= rva < vaddr + max(vsz,rsz):
            return roff + (rva - vaddr)
    return None

ep_off = rva_to_off(aoe)
print(f"\nEntryPoint RVA 0x{aoe:x} -> file offset {('0x%x'%ep_off) if ep_off else None}")
ep_sec = next((n for (n,va,vs,ro,rs,ch) in secs if va<=aoe<va+max(vs,rs)), b'?')
print(f"EntryPoint section = {ep_sec}")

if ep_off:
    stub = d[ep_off:ep_off+128]
    print("\n-- Entry stub raw bytes (128) --")
    print(stub.hex())
    try:
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64 if is64 else capstone.CS_MODE_32)
        print("\n-- Entry stub disasm --")
        for ins in md.disasm(stub, base+aoe):
            print(f"  0x{ins.address:08x}: {ins.mnemonic:8} {ins.op_str}")
            if len(list(())) : pass
    except ImportError:
        print("\n(capstone not installed -- hex only)")
