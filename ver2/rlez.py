#!/usr/bin/env python3

ESC_CHAR = 0xaa

def rlez_decode(src):
    dst = bytearray()
    l = len(src)
    idx = 0
    while idx < l:
        b = src[idx]
        if b == ESC_CHAR:
            idx += 1
            b = src[idx]
            if b == 0:
                dst.append(ESC_CHAR)
            else:
                dst+= bytes([0] * b)
        else:
            dst.append(b)
        idx += 1

    return dst;


def rlez_encode(src):
    dst = bytearray()
    zcount = 0

    for b in src:
        if zcount:
            if b == 0:
                zcount += 1
                if zcount == 255:
                    dst.append(zcount)
                    zcount = 0
            else:
                dst.append(zcount)
                zcount = 0
                if b == ESC_CHAR:
                    dst += bytes([ESC_CHAR, 0])
                else: 
                    dst.append(b)
        else:
            if b == ESC_CHAR:
                dst += bytes([ESC_CHAR, 0])
            elif b == 0:
                zcount = 1
                dst.append(ESC_CHAR)
            else:
                dst.append(b)

    if zcount:
        dst.append(zcount)

    return dst


if __name__ == '__main__':
    import binascii

    with open('nv_comp.dat','rb') as ifh:
        comp = ifh.read()
    with open('nv.txt.dat', 'rb') as ifh:
        orig = ifh.read()

    decomp = rlez_decode(comp) 
    if decomp != orig: 
        print('Uh-oh')

    our_comp = rlez_encode(orig)
    if our_comp != comp:
        print('Uh-oh 2')

