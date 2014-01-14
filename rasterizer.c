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
#include <emmintrin.h>
#include "rasterizer.h"

typedef struct
{
  /* Window coordinates */
  pint16_t x, y;
  /* Depth in [-1, 1] range */
  float z;
  /* Vertex color */
  float r, g, b;
  /* Texture coordinates */
  float u, v;
} frag_t;

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

/**
 * Texture lookup
 */
static inline void
texel_fetch(pig_t * p, frag_t * f, puint8_t * r, puint8_t * g, puint8_t * b)
{
  float u, v;
  puint16_t x, y;
  puint8_t * px;

  u = f->u - floor(f->u);
  v = f->v - floor(f->v);
  x = (puint16_t)(u * p->tex_width) % p->tex_width;
  y = (puint16_t)(v * p->tex_height) % p->tex_height;
  px = p->tex_data + (((y * p->tex_width) + x) << 2);

  *r = px[0];
  *g = px[1];
  *b = px[2];
}

/**
 * Emits a single fragment
 */
static inline void
emit_fragment(pig_t * p, frag_t * f)
{
  pixel_t * px;

  // Make sure the fragment is in the viewport
  if (f->x < 0 || p->width < f->x ||
      f->y < 0 || p->height < f->y ||
      f->z < 0.0f || 1.0f < f->z)
  {
    return;
  }

  // Depth test
  px = p->fbuffer + ((f->y * p->width) + f->x);
  if (px->depth < f->z) {
    return;
  }

  puint8_t r, g, b;

  // Lookup texture
  if (p->mode == RM_TEXTURE)
  {
    texel_fetch(p, f, &r, &g, &b);
  }
  else
  {
    r = 0;
    g = 0;
    b = 0;
  }

  // Write the fragment
  px->r = r;
  px->g = g;
  px->b = b;
  px->depth = f->z;
}

/**
 * Bresenham's line algorithm
 */
static void
emit_line(pig_t * p, frag_t * p0, frag_t * p1)
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
emit_triangle(pig_t * p, frag_t * a, frag_t * b, frag_t * c)
{
  int minx = max(min(a->x, min(b->x, c->x)), 0);
  int miny = max(min(a->y, min(b->y, c->y)), 0);
  int maxx = min(max(a->x, max(b->x, c->x)), p->width - 1);
  int maxy = min(max(a->y, max(b->y, c->y)), p->height - 1);
  float det = (a->x - c->x) * (b->y - c->y) - (b->x - c->x) * (a->y - c->y);
  float bcx = b->x - c->x, bcy = b->y - c->y;
  float acx = a->x - c->x, acy = a->y - c->y;
  float dx, dy, dz;
  frag_t f;

  f.r = 1.0f;
  f.g = 1.0f;
  f.b = 1.0f;

  for (f.y = miny; f.y <= maxy; ++f.y)
  {
    for (f.x = minx; f.x <= maxx; ++f.x)
    {
      /* Check whether the pixel is in the triangle or not */
      int w0 = orient(b, c, &f);
      int w1 = orient(c, a, &f);
      int w2 = orient(a, b, &f);

      if (w0 >= 0 && w1 >= 0 && w2 >= 0)
      {
        /* Compute weights */
        dx = (f.x - c->x) * bcy - (f.y - c->y) * bcx;
        dy = acx * (f.y - c->y) - acy * (f.x - c->x);
        dz = (a->x - f.x) * (b->y - f.y) - (a->y - f.y) * (b->x - f.x);
        dx /= det; dy /= det; dz /= det;

        /* Interpolate attributes */
        f.z = a->z * dx + b->z * dy + c->z * dz;
        f.u = a->u * dx + b->u * dy + c->u * dz;
        f.v = a->v * dx + b->v * dy + c->v * dz;
        f.r = a->r * dx + b->r * dy + c->r * dz;
        f.g = a->g * dx + b->g * dy + c->g * dz;
        f.b = a->b * dx + b->b * dy + c->b * dz;

        /* Render the fragment */
        emit_fragment(p, &f);
      }
    }
  }
}

/**
 * Computes the window-space coordinate of a vertex
 * Returns a non-zero value if the vertex is not clipped
 */
static inline int
transform_vertex(pig_t * p, frag_t * f, vertex_t * a)
{
  vec x, tmp;
  int clipped;

  /* Apply transformations */
  x.x = a->x; x.y = a->y; x.z = a->z; x.w = 1.0f;
  vec_mul(&tmp, &x, p->m_mvp);

  /* Clipping, return 0 if the vertex is outside the view volume */
  clipped = -tmp.w <= tmp.x && tmp.x <= tmp.w &&
            -tmp.w <= tmp.y && tmp.y <= tmp.w &&
            -tmp.w <= tmp.z && tmp.z <= tmp.w &&
            0 < tmp.w;

  /* Get the normalized device coordinates */
  tmp.x /= tmp.w;
  tmp.y /= tmp.w;
  tmp.z /= tmp.w;

  /* Window coordinates */
  f->x = p->width * (tmp.x + 1.0f) / 2;
  f->y = p->height * (tmp.y + 1.0f) / 2;
  f->z = tmp.z;

  /* Copy vertex attributes */
  f->r = a->r;
  f->g = a->g;
  f->b = a->b;
  f->u = a->u;
  f->v = a->v;

  return clipped;
}

void
pig_raster_triangle(pig_t * p, vertex_t * a, vertex_t * b, vertex_t * c)
{
  frag_t f[3];
  int clipA, clipB, clipC;

  clipA = transform_vertex(p, f + 0, a);
  clipB = transform_vertex(p, f + 1, b);
  clipC = transform_vertex(p, f + 2, c);

  /* The triangle is only rasterized if at least one vertex is visible */
  if (clipA || clipB || clipC)
  {
    emit_triangle(p, f + 0, f + 1, f + 2);
    return;
  }
}
