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
  d = n - f;

  memset(m, 0, sizeof(mat));
  m[0] = 1.0f / (a * t);
  m[5] = 1.0f / t;
  m[10] = (f + n) / d;
  m[11] = -1.0f;
  m[14] = 2.0f * n * f / d;
}

void
mat_view(mat m, vec * eye, vec * at, vec * up)
{
  vec x, y, z;

  z.x = at->x - eye->x;
  z.y = at->y - eye->y;
  z.z = at->z - eye->z;
  vec_norm(&z);

  vec_cross(&x, up, &z);
  vec_norm(&x);

  vec_cross(&y, &z, &x);
  vec_norm(&y);

  m[0] = x.x;
  m[1] = y.x;
  m[2] = z.x;
  m[3] = 0.0f;

  m[4] = x.y;
  m[5] = y.y;
  m[6] = z.y;
  m[7] = 0.0f;

  m[8] = x.z;
  m[9] = y.z;
  m[10] = z.z;
  m[11] = 0.0f;

  m[12] = -vec_dot(&x, eye);
  m[13] = -vec_dot(&y, eye);
  m[14] = -vec_dot(&z, eye);
  m[15] = 1.0f;
}

void
mat_mul(mat x, mat y, mat z)
{
  x[ 0] = y[0] * z[ 0] + y[4] * z[ 1] + y[ 8] * z[ 2] + y[12] * z[ 3];
  x[ 1] = y[0] * z[ 4] + y[4] * z[ 5] + y[ 8] * z[ 6] + y[12] * z[ 7];
  x[ 2] = y[0] * z[ 8] + y[4] * z[ 9] + y[ 8] * z[10] + y[12] * z[11];
  x[ 3] = y[0] * z[12] + y[4] * z[13] + y[ 8] * z[14] + y[12] * z[15];

  x[ 4] = y[1] * z[ 0] + y[5] * z[ 1] + y[ 9] * z[ 2] + y[13] * z[ 3];
  x[ 5] = y[1] * z[ 4] + y[5] * z[ 5] + y[ 9] * z[ 6] + y[13] * z[ 7];
  x[ 6] = y[1] * z[ 8] + y[5] * z[ 9] + y[ 9] * z[10] + y[13] * z[11];
  x[ 7] = y[1] * z[12] + y[5] * z[13] + y[ 9] * z[14] + y[13] * z[15];

  x[ 8] = y[2] * z[ 0] + y[6] * z[ 1] + y[10] * z[ 2] + y[14] * z[ 3];
  x[ 9] = y[2] * z[ 4] + y[6] * z[ 5] + y[10] * z[ 6] + y[14] * z[ 7];
  x[10] = y[2] * z[ 8] + y[6] * z[ 9] + y[10] * z[10] + y[14] * z[11];
  x[11] = y[2] * z[12] + y[6] * z[13] + y[10] * z[14] + y[14] * z[15];

  x[12] = y[3] * z[ 0] + y[7] * z[ 1] + y[11] * z[ 2] + y[15] * z[ 3];
  x[13] = y[3] * z[ 4] + y[7] * z[ 5] + y[11] * z[ 6] + y[15] * z[ 7];
  x[14] = y[3] * z[ 8] + y[7] * z[ 9] + y[11] * z[10] + y[15] * z[11];
  x[15] = y[3] * z[12] + y[7] * z[13] + y[11] * z[14] + y[15] * z[15];
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
vec_mul(vec * dest, vec * a, mat b)
{
  dest->x = b[ 0] * a->x + b[ 1] * a->y + b[ 2] * a->z + b[ 3] * a->w;
  dest->y = b[ 4] * a->x + b[ 5] * a->y + b[ 6] * a->z + b[ 7] * a->w;
  dest->z = b[ 8] * a->x + b[ 9] * a->y + b[10] * a->z + b[11] * a->w;
  dest->w = b[12] * a->x + b[13] * a->y + b[14] * a->z + b[15] * a->w;
}

void
vec_cross(vec * dest, vec * a, vec * b)
{
  dest->x = a->y * b->z - a->z * b->y;
  dest->y = a->z * b->x - a->x * b->z;
  dest->z = a->x * b->y - a->y * b->x;
  dest->w = 1.0;
}

float
vec_dot(vec * a, vec * b)
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vec_len(vec * a)
{
  return sqrt(vec_dot(a, a));
}

void
vec_norm(vec * a)
{
  float l;

  l = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
  if (fabsf(l) >= 0.001)
  {
    a->x /= l;
    a->y /= l;
    a->z /= l;
  }
}
