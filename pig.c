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

pig_t *
pig_init(puint16_t width, puint16_t height)
{
  pig_t * p;
  pixel_t * px;
  size_t buffer_size;

  if (!(p = (pig_t*)malloc(sizeof(pig_t))))
  {
    return NULL;
  }

  /* Initialise settings */
  p->width = width;
  p->height = height;
  p->mode = RM_COLOR;
  p->tex_data = NULL;
  p->tex_width = 0;
  p->tex_height = 0;

  /* Initialise the framebuffer */
  buffer_size = p->width * p->height * sizeof(pixel_t);
  p->fbuffer = (pixel_t*)malloc(buffer_size);
  for (px = p->fbuffer; px - p->fbuffer < p->width * p->height; ++px) {
    px->r = px->g = px->b = px->a = 0;
    px->depth = 1.0f;
  }

  return p;
}

void
pig_free(pig_t * p)
{
  if (!p) {
    return;
  }

  if (p->fbuffer) {
    free(p->fbuffer);
    p->fbuffer = NULL;
  }
}

void
pig_triangle(pig_t * p, vertex_t * v, puint32_t count)
{
  puint32_t i;
  for (i = 0; i < count; ++i) {
    pig_raster_triangle(p, &v[i * 3 + 0], &v[i * 3 + 1], &v[i * 3 + 2]);
  }
}

void
pig_show(pig_t * p)
{
  FILE * fout;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep row;
  puint16_t i, j;
  pixel_t * pix;

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
  for (i = 0; i < p->height; ++i)
  {
    for (j = 0; j < p->width; ++j)
    {
      pix = p->fbuffer + ((p->height - i - 1) * p->width + j);
      row[j * 3 + 0] = pix->r;
      row[j * 3 + 1] = pix->g;
      row[j * 3 + 2] = pix->b;
    }

    png_write_row(png_ptr, row);
  }

  png_write_end(png_ptr, NULL);

  fclose(fout);
  free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
}
