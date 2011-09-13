/******************************************************************************* 
 *  -- convolve_altivec.cc      Added convolve function optimised for Altivec instruction set
 *
 *  Copyright (C) 2010
 *
 *      Author    TimRex
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/


#include "sid.h"

#if (RESID_USE_ALTIVEC==1)

#include <altivec.h>

float convolve_altivec(const float *a, const float *b, int n)
{
    float sum = 0.0f;
    vector float vsum = { 0.0f, 0.0f, 0.0f, 0.0f };
    union {
        vector float v;
        float a[4];
    } usum;

    vector float *pa = (vector float *)a;
    vector float *pb = (vector float *)b;

    while (n >= 4)
    {
        vsum = vec_madd(*pa, *pb, vsum);
        pa++;
        pb++;
        n -= 4;
    }

    usum.v = vsum;

    sum = usum.a[0] + usum.a[1] + usum.a[2] + usum.a[3];

    a = (float *)pa;
    b = (float *)pb;

    while (n --)
    {
        sum += (*a++ * *b++);
    }

    return sum;
}

#endif
