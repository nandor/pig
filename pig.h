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
#ifndef __PIG_PIG_H__
#define __PIG_PIG_H__

#include "types.h"
#include "vecmath.h"

/* Renderer settings */
typedef enum
{
  /* Fill only with vertex color */
  RM_COLOR = (1 << 0),
  /* Fill only with texture data */
  RM_TEXTURE = (1 << 1),
  /* Lambert Shading */
  RM_LAMBERT = (1 << 2),
  /* Phong Shading */
  RM_PHON = (1 << 3)
} rendermode_t;

/* Vertex data */
typedef struct
{
  /* Vertex position */
  float x, y, z;
  /* Color */
  float r, g, b;
  /* Texture coordinate */
  float u, v;
} vertex_t;

/* Framebuffer pixel */
typedef struct
{
  puint8_t r;
  puint8_t g;
  puint8_t b;
  puint8_t a;
  float depth;
} __attribute__ ((__packed__)) pixel_t;

/* Renderer state */
typedef struct
{
  /* Viewport width */
  puint16_t width;
  /* Viewport height */
  puint16_t height;
  /* Framebuffer */
  pixel_t * fbuffer;
  /* MPV matrix */
  mat m_mvp;
  /* Current texture data */
  puint8_t * tex_data;
  /* Current texture width */
  puint16_t tex_width;
  /* Current texture height */
  puint16_t tex_height;
  /* Texture rendering mode */
  puint32_t mode;
} pig_t;

pig_t * pig_init(puint16_t, puint16_t);
void pig_triangle(pig_t *, vertex_t *, puint32_t);
void pig_show(pig_t *);
void pig_free(pig_t *);

#endif /*__PIG_PIG_H__*/
