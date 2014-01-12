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
#include <stdio.h>
#include <math.h>
#include "rasterizer.h"

typedef struct
{
  pint16_t x;
  pint16_t y;
  float z;
} frag_t;

static inline int
abs(int a)
{
  return a > 0 ? a : (-a);
}

static inline int
min(int a, int b)
{
  return a < b ? a : b;
}

static inline int
max(int a, int b)
{
  return a > b ? a : b;
}

static inline void
emit_fragment(pig * p, frag_t * f)
{
  puint8_t * px;

  if (f->x < 0 || p->width < f->x ||
      f->y < 0 || p->height < f->y ||
      f->z < 0.0f || 1.0f < f->z)
  {
    return;
  }

  px = p->cbuffer + ((f->y * p->width) + f->x) * 4;
  px[0] = 128;
  px[1] = 128;
  px[2] = 128;
}

/**
 * Bresenham's line algorithm
 */
static void
emit_line(pig * p, frag_t * p0, frag_t * p1)
{
  int dx, dy, sx, sy, err, e;
  frag_t f;

  dx = abs(p1->x - p0->x);
  dy = abs(p1->y - p0->y);
  sx = p1->x < p0->x ? -1 : 1;
  sy = p1->y < p0->y ? -1 : 1;
  err = dx - dy;
  f = *p0;

  while (1)
  {
    emit_fragment(p, &f);
    if (f.x == p1->x && f.y == p1->y)
    {
      break;
    }

    e = 2 * err;
    if (e > -dy) {
      err -= dy;
      f.x += sx;
    }

    if (f.x == p1->x && f.y == p1->y)
    {
      emit_fragment(p, &f);
      break;
    }

    if (e < dx) {
      err += dx;
      f.y += sy;
    }
  }
}

static inline int
orient(frag_t * a, frag_t * b, frag_t * c)
{
  return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

/**
 * Triangle rasterization
 */
static void
emit_triangle(pig * p, frag_t * a, frag_t * b, frag_t * c)
{
  int minx = max(min(a->x, min(b->x, c->x)), 0);
  int miny = max(min(a->y, min(b->y, c->y)), 0);
  int maxx = min(max(a->x, max(b->x, c->x)), p->width - 1);
  int maxy = min(max(a->y, max(b->y, c->y)), p->height - 1);
  frag_t f;

  for (f.y = miny; f.y <= maxy; ++f.y)
  {
    for (f.x = minx; f.x <= maxx; ++f.x)
    {
      int w0 = orient(b, c, &f);
      int w1 = orient(c, a, &f);
      int w2 = orient(a, b, &f);

      if (w0 >= 0 && w1 >= 0 && w2 >= 0)
      {
          emit_fragment(p, &f);
      }
    }
  }
}

void
pig_raster_triangle(pig * p, vec * a, vec * b, vec * c)
{
  vec x[3];
  frag_t f[3];

  vec_mul(x + 0, a, p->m_mvp);
  vec_mul(x + 1, b, p->m_mvp);
  vec_mul(x + 2, c, p->m_mvp);

  for (register int i = 0; i < 3; ++i) {
    x[i].x /= x[i].w;
    x[i].y /= x[i].w;
    x[i].z /= x[i].w;

    f[i].x = p->width * (x[i].x + 1.0f) / 2;
    f[i].y = p->height * (x[i].y + 1.0f) / 2;
    f[i].z = (x[i].z + 1.0) / 2.0;
  }

  emit_triangle(p, f + 0, f + 1, f + 2);
}
