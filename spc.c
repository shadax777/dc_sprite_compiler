// sprite compiler  (c) 2001/2002 Christian Werle aka "Shadax"
//
// Compile() generates .sh4 files (which contain pure SH4 code)

/*
entry:
  r4: source address
  r5: destination address end(!)
  r6: destination width (bytes per line, not pixels!)

run-time:
  r6: remaining bytes in destination to next line from last pixel in cur. line
  r0: temporary pixel storage

exit:
  r0: trashed
  r4: trashed
  r5: trashed
  r6: trashed
*/


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>


#define ALPHA   0       /* RGB alpha value */


static int          our_printsrc = 0;   // print asm source to stdout
static unsigned int our_instrcount;     // num of emitted instructions per bmp


#include "shared/types.h"
#include "shared/bitmap.h"
#include "shared/sample_data.h"
#include "opcodes.h"



#define EMIT_INSTR0(code, f)            \
        (EmitInstr0(code, f), our_instrcount++)

#define EMIT_INSTR2(code, p1, p2, f)    \
        (EmitInstr2(code, p1, p2, f), our_instrcount++)



int src_printf(const char *fmt, ...)
{
  if(our_printsrc)
  {
    va_list ap;
    char    text[512];

    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    printf("%s", text);
    return strlen(text);
  }
  return 0;
}


//---------------------------------------
// _OutputTranslucentPixels()
//---------------------------------------
static void _OutputTranslucentPixels(int num_alphas, int bpp, FILE *f)
{
  int   remaining;

  if(!num_alphas)
    return;

  // bytes in source
  remaining = num_alphas * bpp;
  while(remaining > 127)
  {
    EMIT_INSTR2(ADD_IMM_REG, 127, 4, f);
    remaining -= 127;
  }
  EMIT_INSTR2(ADD_IMM_REG, remaining, 4, f);

  // bytes in dest
  remaining = num_alphas * bpp;
  while(remaining > 128)
  {
    EMIT_INSTR2(ADD_IMM_REG, -128, 5, f);
    remaining -= 128;
  }
  EMIT_INSTR2(ADD_IMM_REG, -remaining, 5, f);
  src_printf("\n");
} // _OutputTranslucentPixels()


//---------------------------
// Compile()
//---------------------------
void Compile(Bitmap *bm, FILE *outf)
{
  int       x, y;
  int       bpp = sizeof(Pixel);    // bytes per pixel
  int       remaining;


  assert(bpp == 1
      || bpp == 2
      || bpp == 4);


  our_instrcount = 0;

  src_printf(".text\n");
  src_printf("_Blit__%s:\n\n", bm->name);

  // R6 will hold the number of remaining bytes until start of next line
  src_printf("! r6 will hold the number of remaining bytes until start of next line\n");
  remaining = bm->w * bpp;
  while(remaining > 128)
  {
    EMIT_INSTR2(ADD_IMM_REG, -128, 6, outf);
    remaining -= 128;
  }
  EMIT_INSTR2(ADD_IMM_REG, -remaining, 6, outf);
  src_printf("\n");

  // compile line by line
  for(y = 0; y < bm->h; y++)
  {
    int num_alphas = 0;

    // pixels in this line
    src_printf("! line %i\n", y);
    for(x = 0; x < bm->w; x++)
    {
      Pixel pixel = bm->sh4ready_pixels[y * bm->w + x];

      // collect subsequent translucent pixels
      if(pixel == ALPHA)
      {
        num_alphas++;
      }
      else
      {
        // flush subsequent translucent pixels
        if(num_alphas > 0)
        {
          _OutputTranslucentPixels(num_alphas, bpp, outf);
          num_alphas = 0;
        }

        // save the pixel in R0
        switch(bpp)
        {
          case 1: EMIT_INSTR2(MOVB_xREGp_REG, 4, 0, outf); break;
          case 2: EMIT_INSTR2(MOVW_xREGp_REG, 4, 0, outf); break;
          case 4: EMIT_INSTR2(MOVL_xREGp_REG, 4, 0, outf); break;
        }

        // roll back the sign-extension by masking pixel
        // whose bit #15 (or #7) is set
        if(bpp != 4
        && (pixel & (bpp==2 ? 0x8000:0x80)) != 0)
        {
          // zero bits #15..31 (or #7..31) in R0
          switch(bpp)
          {
            case 1: EMIT_INSTR2(EXTUB_REG_REG, 0, 0, outf); break;
            case 2: EMIT_INSTR2(EXTUW_REG_REG, 0, 0, outf); break;
            // 4 bpp needs no bit-masking
          }
        }

        // copy the pixel from R0 to its destination
        // -> "mov.{b,w,l} R0, @-R5"
        switch(bpp)
        {
          case 1: EMIT_INSTR2(MOVB_REG_xmREG, 0, 5, outf); break;
          case 2: EMIT_INSTR2(MOVW_REG_xmREG, 0, 5, outf); break;
          case 4: EMIT_INSTR2(MOVL_REG_xmREG, 0, 5, outf); break;
        }
        src_printf("\n");
      }

      // if we are on the last pixel in this line
      // flush all remaining subsequent translucent pixels
      if(x == bm->w-1 && num_alphas)
      {
        _OutputTranslucentPixels(num_alphas, bpp, outf);
      }
    }

    // jump to next line of pixels
    src_printf("! line break\n");
    EMIT_INSTR2(SUB_REG_REG, 6, 5, outf);
    src_printf("\n\n");
  }

  EMIT_INSTR0(RTS, outf);
  EMIT_INSTR0(NOP, outf);

  fprintf(stderr, "%i instructions (%i*%i)\n", our_instrcount, bm->w, bm->h);
} // Compile




//------------------------------------
// _ParseArgv()
//------------------------------------
static void _ParseArgv(char *argv)
{
  const static char *options[] =
  {
    "-v",
    "--verbose",
    NULL
  };
  int   i;

  for(i = 0; options[i]; i++)
  {
    if(!strcmp(argv, options[i]))
    {
      switch(i)
      {
        case 0:     // -v
        case 1:     // --verbose
          our_printsrc = 1;
          break;
      }
    }
  }
}


//------------------------------------
// main()
//------------------------------------
int main(int argc, char **argv)
{
  char      fname[128];
  FILE      *f;
  int       i;


  for(i = 1; i < argc; i++)
  {
    _ParseArgv(argv[i]);
  }

  // build dyna-code of the bitmaps
  for(i = 0; i < NUM_SAMPLEBITMAPS; i++)
  {
    BM_PatchBitmap(&our_bitmaps[i]);
    sprintf(fname, "Code__%s.sh4", our_bitmaps[i].name);
    if((f = fopen(fname, "wb")))
    {
      Compile(&our_bitmaps[i], f);
      fclose(f);
    }
    else
    {
      fprintf(stderr, "could not write %s\n", fname);
      return 1;
    }
  }
  return 0;
}
