import struct, capstone, sys, os

path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ephinea.dll")
d = bytearray(open(path, 'rb').read())

base = 0x10000000
ep_rva = 0x31e22dc
ep_off = 0xc104dc            # file offset of EP (from _pe_probe)
ep_va  = base + ep_rva       # 0x131e22dc

# Layer-1 params recovered from the stub:
xor_start_va = ep_va + 0x63
xor_len      = 0x5c1
xor_key      = 0xc7          # dl from mov edx,0x7bf760c7
xor_off      = ep_off + 0x63

dec = bytearray(d[xor_off:xor_off+xor_len])
for i in range(len(dec)):
    dec[i] ^= xor_key

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = False

# entry into decrypted code is EP+0x67  -> offset 4 into the decrypted buffer
entry_into = 0x67 - 0x63
print(f"== Stage-2 (XOR 0x{xor_key:02x}), {xor_len} bytes @ VA 0x{xor_start_va:08x}; entry @ +0x{entry_into:x} ==\n")
print("decrypted head hex:", dec[entry_into:entry_into+64].hex(), "\n")

cnt = 0
for ins in md.disasm(bytes(dec[entry_into:]), xor_start_va + entry_into):
    print(f"  0x{ins.address:08x}: {ins.bytes.hex():<16} {ins.mnemonic:8} {ins.op_str}")
    cnt += 1
    if cnt >= 80:
        break
