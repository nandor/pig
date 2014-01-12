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
#include "pig.h"

int main()
{
  pig * p;
  mat model, view, proj, vp;
  vec pos, at, up;

  float data[] =
  {
    0.0f, 0.0f, 2.0f, 1.0f,
    0.0f, 3.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f
  };

  if (!(p = pig_init(800, 600)))
  {
    fprintf(stderr, "Cannot init PIG");
    return -1;
  }

  pos.x = 0.0f; pos.y = 0.0f; pos.z = 6.0f;
  at.x = 0.0f; at.y = 0.0f; at.z = 0.0f;
  up.x = 0.0f; up.y = 1.0f; up.z = 0.0f;

  mat_identity(model);
  mat_view(view, &pos, &at, &up);
  mat_proj(proj, 45.0f, 800.0f / 600.0f, 0.1f, 200.0f);

  mat_mul(vp, proj, view);
  mat_mul(p->m_mvp, vp, model);

  pig_triangle(p, data, 1);

  pig_show(p);
  pig_free(p);
  return 0;
}
