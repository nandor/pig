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
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "pig.h"
#include "rasterizer.h"

pig *
pig_init(puint16_t width, puint16_t height)
{
  pig * p;

  if (!(p = (pig*)malloc(sizeof(pig))))
  {
    return NULL;
  }

  p->width = width;
  p->height = height;
  p->cbuffer = (puint8_t*)malloc(p->width * p->height * sizeof(puint8_t) * 4);
  p->dbuffer = (float*)malloc(p->width * p->height * sizeof(float));
  memset(p->cbuffer, 0, p->width * p->height * sizeof(puint8_t) * 4);
  memset(p->dbuffer, 0, p->width * p->height * sizeof(float));

  return p;
}

void
pig_free(pig * p)
{
  if (!p) {
    return;
  }

  if (p->cbuffer) {
    free(p->cbuffer);
    p->cbuffer = NULL;
  }

  if (p->dbuffer) {
    free(p->dbuffer);
    p->dbuffer = NULL;
  }
}

void
pig_triangle(pig * p, float * verts, puint32_t count)
{
  vec * arr;

  arr = (vec*)verts;
  for (puint32_t i = 0; i < count; ++i) {
    pig_raster_triangle(p, &arr[i * 3], &arr[i * 3 + 1], &arr[i * 3 + 2]);
  }
}

void
pig_show(pig * p)
{
  FILE * fout;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep row;

  if (!(fout = fopen("pig.png", "wb")))
  {
    return;
  }

  if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                          NULL, NULL, NULL)))
  {
    fclose(fout);
    return;
  }

  if (!(info_ptr = png_create_info_struct(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, NULL);
    fclose(fout);
    return;
  }

  png_init_io(png_ptr, fout);
  png_set_IHDR(png_ptr, info_ptr, p->width, p->height,
               8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);

  row = (png_bytep)malloc(sizeof(png_byte) * p->width * 3);
  for (puint16_t i = 0; i < p->height; ++i)
  {
    for (puint16_t j = 0; j < p->width; ++j)
    {
      row[j * 3 + 0] = p->cbuffer[((i * p->width + j) << 2) + 0] * 255.0f;
      row[j * 3 + 1] = p->cbuffer[((i * p->width + j) << 2) + 1] * 255.0f;
      row[j * 3 + 2] = p->cbuffer[((i * p->width + j) << 2) + 2] * 255.0f;
    }

    png_write_row(png_ptr, row);
  }

  png_write_end(png_ptr, NULL);

  fclose(fout);
  free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
}
