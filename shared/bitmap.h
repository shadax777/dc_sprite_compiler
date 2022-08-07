#ifndef __BITMAP_H
#define __BITMAP_H

#include "types.h"


typedef struct Bitmap
{
  const char *name;     /* name for identification in editor            */
  Pixel *pixels;        /* pointer to bitmap's actual pixels            */
  u16   w, h;           /* width and height                             */
  u16   flags;          /* internal flags for drawing                   */

#warning sh4ready_pixels[] should be dynamically allocated
  Pixel sh4ready_pixels[131072];    // these pixels are used by dyna-code

} Bitmap;


/*
the Bitmap struct should be extended:

Bitmap
{
  //...
  void (*code)(struct Bitmap *dest, const struct Bitmap *self);
}
Compile() should set "code" to an address with the freshly
created asm-code

- code should get generated at runtime or red from a .sh4 file
*/

//--------------------------------------------------------
// BM_PatchBitmap: converts the pixels to DC RGB format and
//                 creates pixels ready for the sh4 code
//--------------------------------------------------------
#include "palette.h"
// this is the RGB code:
// #define RGB(r, g, b) (((r)>>3) << 11) | (((g)>>2) << 5)| (((b)>>3) << 0)
void BM_PatchBitmap(Bitmap *bm)
{
  int           x, y;
  int           i, k, bmsize;

  // convert all pixels to RGB
  for(y = 0; y < bm->h; y++)
  {
    for(x = 0; x < bm->w; x++)
    {
      Pixel *pixel = &bm->pixels[y * bm->w + x];
      u32   r, g, b;

    #if 0
      // this gives us too dark colors
      r = (palette256[*pixel*3+0] >> 3) << 11;
      g = (palette256[*pixel*3+1] >> 2) << 5;
      b = (palette256[*pixel*3+2] >> 3) << 0;
    #else
      // these colors fit exactly :)
      r = (palette256[*pixel*3+0] >> 1) << 11;
      g = (palette256[*pixel*3+1] >> 0) << 5;
      b = (palette256[*pixel*3+2] >> 1) << 0;
    #endif

      *pixel = r | g | b;
    }
  }

  // flip pixels for dyna-code
  bmsize = bm->w * bm->h;
  //bm->sh4ready_pixels = (Pixel*)malloc(bmsize * sizeof(Pixel));
  for(i = 0, k = bmsize-1; i < bmsize; i++, k--)
  {
    bm->sh4ready_pixels[k] = bm->pixels[i];
  }
} // BM_PatchBitmap


#endif  /* __BITMAP_H */
