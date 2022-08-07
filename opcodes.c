#include <stdio.h>
#include <assert.h>
#include "opcodes.h"


// instructions with no operand (e.g. "rts")
typedef struct
{
  u16   (*func)(void);
  char  *mnemonic;  // optional for debug output
} instr0_t;


// instructions with 2 operands (e.g. "add #127, r0")
typedef struct
{
  u16   (*func)(u8, u8);
  char  *mnemonic;
} instr2_t;


// instr0 functions
static u16 f_RTS(void);
static u16 f_NOP(void);


// instr2 functions
static u16 f_ADD_IMM_REG(u8, u8);
static u16 f_SUB_REG_REG(u8, u8);
static u16 f_EXTSB_REG_REG(u8, u8);
static u16 f_EXTSW_REG_REG(u8, u8);
static u16 f_EXTUB_REG_REG(u8, u8);
static u16 f_EXTUW_REG_REG(u8, u8);
static u16 f_MOV_IMM_REG(u8, u8);
static u16 f_MOV_REG_REG(u8, u8);
static u16 f_MOVB_xREGp_REG(u8, u8);
static u16 f_MOVW_xREGp_REG(u8, u8);
static u16 f_MOVL_xREGp_REG(u8, u8);
static u16 f_MOVB_REG_xmREG(u8, u8);
static u16 f_MOVW_REG_xmREG(u8, u8);
static u16 f_MOVL_REG_xmREG(u8, u8);
static u16 f_SWAPB_REG_REG(u8, u8);
static u16 f_SWAPW_REG_REG(u8, u8);
static u16 f_AND_REG_REG(u8, u8);
static u16 f_OR_REG_REG(u8, u8);


static instr0_t our_instr0tab[] =
{
  { f_RTS, "rts" },     // RTS
  { f_NOP, "nop" },     // NOP
};

static instr2_t our_instr2tab[] =
{
  { f_ADD_IMM_REG,      "add #%i, r%i" },       // ADD_IMM_REG
  { f_SUB_REG_REG,      "sub r%i, r%i" },       // SUB_REG_REG
  { f_EXTSB_REG_REG,    "exts.b r%i, r%i" },    // EXTSB_REG_REG
  { f_EXTSW_REG_REG,    "exts.w r%i, r%i" },    // EXTSW_REG_REG
  { f_EXTUB_REG_REG,    "extu.b r%i, r%i" },    // EXTUB_REG_REG
  { f_EXTUW_REG_REG,    "extu.w r%i, r%i" },    // EXTUW_REG_REG
  { f_MOV_IMM_REG,      "mov #%i, r%i" },       // MOV_IMM_REG
  { f_MOV_REG_REG,      "mov r%i, r%i" },       // MOV_REG_REG
  { f_MOVB_xREGp_REG,   "mov.b @r%i+, r%i" },   // MOVB_xREGp_REG
  { f_MOVW_xREGp_REG,   "mov.w @r%i+, r%i" },   // MOVW_xREGp_REG
  { f_MOVL_xREGp_REG,   "mov.l @r%i+, r%i" },   // MOVL_xREGp_REG
  { f_MOVB_REG_xmREG,   "mov.b r%i, @-r%i" },   // MOVB_REG_xmREG
  { f_MOVW_REG_xmREG,   "mov.w r%i, @-r%i" },   // MOVW_REG_xmREG
  { f_MOVL_REG_xmREG,   "mov.l r%i, @-r%i" },   // MOVL_REG_xmREG
  { f_SWAPB_REG_REG,    "swap.b r%i, r%i" },    // SWAPB_REG_REG
  { f_SWAPW_REG_REG,    "swap.w r%i, r%i" },    // SWAPw_REG_REG
  { f_AND_REG_REG,      "and r%i, r%i" },       // AND_REG_REG
  { f_OR_REG_REG,       "or r%i, r%i" },        // OR_REG_REG
};


//============================================================================

#define ARRAYSIZE(a)    (sizeof(a) / sizeof((a)[0]))


// in spc.c - prints the mnemonic
int src_printf(const char *, ...);



//---------------------------
// EmitInstr0
//---------------------------
void EmitInstr0(int codenum, FILE *outf)
{
  u16   code;

  assert(codenum < ARRAYSIZE(our_instr0tab));

  code = (our_instr0tab[codenum].func)();
  fputc(code & 0x0F, outf);
  fputc(code >> 8, outf);
  src_printf(our_instr0tab[codenum].mnemonic);
  src_printf("\n");
}


//---------------------------
// EmitInstr2
//---------------------------
//void EmitInstr2(int codenum, u8 param1, u8 param2, FILE *outf)
void EmitInstr2(int codenum, int param1, int param2, FILE *outf)
{
  u16   code;

  assert(codenum < ARRAYSIZE(our_instr2tab));

  code = (our_instr2tab[codenum].func)(param1, param2);
  fputc(code & 0xFF, outf);
  fputc((code >> 8) & 0xFF, outf);
  src_printf(our_instr2tab[codenum].mnemonic, param1, param2);
  src_printf("\n");
}


//============================================================================


//--------------------------------------------------------------------------
//                  arithmetic operations
//--------------------------------------------------------------------------

// add #imm, Rn
static u16 f_ADD_IMM_REG(u8 imm, u8 n)
{
  u16   code = 0x7000;  // 0100nnnniiiiiiii

  assert(n <= 15);
  code |= n << 8;
  code |= imm;
  return code;
}


// sub Rm, Rn
static u16 f_SUB_REG_REG(u8 m, u8 n)
{
  u16   code = 0x3008;  // 0011nnnnmmmm1000

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// exts.b Rm, Rn
static u16 f_EXTSB_REG_REG(u8 m, u8 n)
{
  u16   code = 0x600E;  // 0110nnnnmmmm1110

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// exts.w Rm, Rn
static u16 f_EXTSW_REG_REG(u8 m, u8 n)
{
  u16   code = 0x600F;  // 0110nnnnmmmm1111

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// extu.b Rm, Rn
static u16 f_EXTUB_REG_REG(u8 m, u8 n)
{
  u16   code = 0x600C;  // 0110nnnnmmmm1100

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// extu.w Rm, Rn
static u16 f_EXTUW_REG_REG(u8 m, u8 n)
{
  u16   code = 0x600D;  // 0110nnnnmmmm1101

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


//--------------------------------------------------------------------------
//                  fixed-point transfers
//--------------------------------------------------------------------------

// mov #imm, Rn
static u16 f_MOV_IMM_REG(u8 imm, u8 n)
{
  u16   code = 0xE000;  // 1110nnnniiiiiiii

  assert(n <= 15);
  code |= n << 8;
  code |= imm;
  return code;
}


// mov Rm, Rn
static u16 f_MOV_REG_REG(u8 m, u8 n)
{
  u16   code = 0x6003;  // 0110nnnnmmmm0011

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.b @Rm+, Rn
static u16 f_MOVB_xREGp_REG(u8 m, u8 n)
{
  u16   code = 0x6004;  // 0110nnnnmmmm0100

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.w @Rm+, Rn
static u16 f_MOVW_xREGp_REG(u8 m, u8 n)
{
  u16   code = 0x6005;  // 0110nnnnmmmm0101

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.l @Rm+, Rn
static u16 f_MOVL_xREGp_REG(u8 m, u8 n)
{
  u16   code = 0x6006;  // 0110nnnnmmmm0110

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.b Rm, @-Rn
static u16 f_MOVB_REG_xmREG(u8 m, u8 n)
{
  u16   code = 0x2004;  // 0010nnnnmmmm0100

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.w Rm, @-Rn
static u16 f_MOVW_REG_xmREG(u8 m, u8 n)
{
  u16   code = 0x2005;  // 0010nnnnmmmm0101

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// mov.l Rm, @-Rn
static u16 f_MOVL_REG_xmREG(u8 m, u8 n)
{
  u16   code = 0x2006;  // 0010nnnnmmmm0110

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// swap.b Rm, Rn
static u16 f_SWAPB_REG_REG(u8 m, u8 n)
{
  u16   code = 0x6008;  // 0110nnnnmmmm1000

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// swap.w Rm, Rn
static u16 f_SWAPW_REG_REG(u8 m, u8 n)
{
  u16   code = 0x6009;  // 0110nnnnmmmm1001

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


//--------------------------------------------------------------------------
//                  logic operations
//--------------------------------------------------------------------------

// and Rm, Rn
static u16 f_AND_REG_REG(u8 m, u8 n)
{
  u16   code = 0x2009;  // 0010nnnnmmmm1001

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


// or Rm, Rn
static u16 f_OR_REG_REG(u8 m, u8 n)
{
  u16   code = 0x200B;  // 0010nnnnmmmm1011

  assert(m <= 15);
  assert(n <= 15);
  code |= m << 4;
  code |= n << 8;
  return code;
}


//--------------------------------------------------------------------------
//                  branches
//--------------------------------------------------------------------------

// rts
static u16 f_RTS(void)
{
  return 0x000B;
}


//--------------------------------------------------------------------------
//                  system control
//--------------------------------------------------------------------------

// nop
static u16 f_NOP(void)
{
  return 0x0009;
}
