//------------------------------------------------------------------------------
// File: vdi_osal.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include "rtos_types.h"
#include "malloc.h"
#include "vpuconfig.h"
#include "vputypes.h"
#include "vdi_osal.h"
//------------------------------------------------------------------------------
// math related api
//------------------------------------------------------------------------------
#ifndef I64
typedef long long I64;
#endif

// 32 bit / 16 bit ==> 32-n bit remainder, n bit quotient
static int fixDivRq(int a, int b, int n)
{
    I64 c;
    I64 a_36bit;
    I64 mask, signBit, signExt;
    int i;

    // DIVS emulation for BPU accumulator size
    // For SunOS build
    mask      = 0x0F;
    mask    <<= 32;
    mask     |= 0x00FFFFFFFF; // mask = 0x0FFFFFFFFF;
    signBit   = 0x08;
    signBit <<= 32;           // signBit = 0x0800000000;
    signExt   = 0xFFFFFFF0;
    signExt <<= 32;           // signExt = 0xFFFFFFF000000000;

    a_36bit = (I64)a;

    for (i = 0; i < n; i++)
    {
        c = a_36bit - (b << 15);
        if (c >= 0)
            a_36bit = (c << 1) + 1;
        else
            a_36bit = a_36bit << 1;

        a_36bit = a_36bit & mask;
        if (a_36bit & signBit)
            a_36bit |= signExt;
    }

    a = (int)a_36bit;
    return a; // R = [31:n], Q = [n-1:0]
}

int math_div(int number, int denom)
{
    int c;
    c = fixDivRq(number, denom, 17); // R = [31:17], Q = [16:0]
    c = c & 0xFFFF;
    c = (c + 1) >> 1;                // round
    return (c & 0xFFFF);
}
