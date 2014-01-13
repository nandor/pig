/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2014 Nandor Licker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include "vecmath.h"

void
mat_identity(mat m)
{
  memset(m, 0, sizeof(mat));
  m[0] = 1.0f;
  m[5] = 1.0f;
  m[10] = 1.0f;
  m[15] = 1.0f;
}

void
mat_proj(mat m, float fov, float a, float n, float f)
{
  float t, d;

  t = tan (fov / 360 * PI);
  d = f - n;

  memset(m, 0, sizeof(mat));
  m[0] = 1.0f / (a * t);
  m[5] = 1.0f / t;
  m[10] = -(f + n) / d;
  m[11] = -1.0f;
  m[14] = -2.0f * n * f / d;
}

void
mat_view(mat m, vec * eye, vec * at, vec * up)
{
  vec x, y, z;

  z.x = at->x - eye->x;
  z.y = at->y - eye->y;
  z.z = at->z - eye->z;
  vec_norm(&z);

  vec_cross(&x, &z, up);
  vec_norm(&x);

  vec_cross(&y, &x, &z);

  m[0] = x.x;
  m[1] = y.x;
  m[2] = -z.x;
  m[3] = 0.0f;

  m[4] = x.y;
  m[5] = y.y;
  m[6] = -z.y;
  m[7] = 0.0f;

  m[8] = x.z;
  m[9] = y.z;
  m[10] = -z.z;
  m[11] = 0.0f;

  m[12] = -vec_dot(&x, eye);
  m[13] = -vec_dot(&y, eye);
  m[14] = vec_dot(&z, eye);
  m[15] = 1.0f;
}

void
mat_mul(mat d, mat a, mat b)
{
  __m128 acc, al, bl;
  register int i;

  for (i = 0; i < 16; i += 4)
  {
    al = _mm_loadu_ps(a);
    bl = _mm_set_ps1(b[i]);
    acc = _mm_mul_ps(al, bl);

    al = _mm_loadu_ps(a + 4);
    bl = _mm_set_ps1(b[i + 1]);
    acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

    al = _mm_loadu_ps(a + 8);
    bl = _mm_set_ps1(b[i + 2]);
    acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

    al = _mm_loadu_ps(a + 12);
    bl = _mm_set_ps1(b[i + 3]);
    acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

    _mm_storeu_ps(d + i, acc);
  }
}

void
mat_dump(mat m)
{
  printf("%f %f %f %f\n", m[0], m[4], m[ 8], m[12]);
  printf("%f %f %f %f\n", m[1], m[5], m[ 9], m[13]);
  printf("%f %f %f %f\n", m[2], m[6], m[10], m[14]);
  printf("%f %f %f %f\n", m[3], m[7], m[11], m[15]);
}

void
vec_mul(vec * d, vec * a, mat b)
{
  __m128 al, bl, acc;

  al = _mm_set_ps1(a->x);
  bl = _mm_loadu_ps(b);
  acc = _mm_mul_ps(al, bl);

  al = _mm_set_ps1(a->y);
  bl = _mm_loadu_ps(b + 4);
  acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

  al = _mm_set_ps1(a->z);
  bl = _mm_loadu_ps(b + 8);
  acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

  al = _mm_set_ps1(a->w);
  bl = _mm_loadu_ps(b + 12);
  acc = _mm_add_ps(acc, _mm_mul_ps(al, bl));

  _mm_storeu_ps((float*)d, acc);
}

void
vec_cross(vec * d, vec * a, vec * b)
{
  __m128 al, bl, r;

  al = _mm_loadu_ps((float*)a);
  bl = _mm_loadu_ps((float*)b);

  r =_mm_sub_ps(
    _mm_mul_ps(
      _mm_shuffle_ps(al, al, _MM_SHUFFLE(3, 0, 2, 1)),
      _mm_shuffle_ps(bl, bl, _MM_SHUFFLE(3, 1, 0, 2))
    ),
    _mm_mul_ps(
      _mm_shuffle_ps(al, al, _MM_SHUFFLE(3, 1, 0, 2)),
      _mm_shuffle_ps(bl, bl, _MM_SHUFFLE(3, 0, 2, 1))
    )
  );

  _mm_storeu_ps((float*)d, r);
  d->w = 1.0f;
}

float
vec_dot(vec * a, vec * b)
{
  __m128 al, bl;
  union {
    float v[4];
    __m128 vec;
  } r;

  al = _mm_loadu_ps((float*)a);
  bl = _mm_loadu_ps((float*)b);

  r.vec = _mm_mul_ps(al, bl);
  r.vec = _mm_hadd_ps(r.vec, r.vec);
  r.vec = _mm_hadd_ps(r.vec, r.vec);

  return r.v[0];
}

float vec_len(vec * a)
{
  __m128 al;
  union {
    float v[4];
    __m128 vec;
  } r;

  al = _mm_loadu_ps((float*)a);
  r.vec = _mm_mul_ps(al, al);
  r.vec = _mm_hadd_ps(r.vec, r.vec);
  r.vec = _mm_hadd_ps(r.vec, r.vec);
  r.vec = _mm_sqrt_ps(r.vec);

  return r.v[0];
}

void
vec_norm(vec * a)
{
  __m128 al, tmp;

  al = _mm_loadu_ps((float*)a);
  tmp = _mm_mul_ps(al, al);
  tmp = _mm_hadd_ps(tmp, tmp);
  tmp = _mm_hadd_ps(tmp, tmp);
  tmp = _mm_rsqrt_ps(tmp);

  if (_mm_movemask_ps(_mm_cmpgt_ps(tmp, _mm_setzero_ps())) == 0xF)
  {
    _mm_storeu_ps((float*)a, _mm_mul_ps(al, tmp));
  }
}
