#ifndef __OPCODES_H
#define __OPCODES_H

#include <stdio.h>
#include "shared/types.h"


// instr0 - instructions with 0 operands
enum
{
  RTS = 0,
  NOP,
};

// instr2 - instructions with 2 operands
enum
{
  ADD_IMM_REG = 0,
  SUB_REG_REG,
  EXTSB_REG_REG,
  EXTSW_REG_REG,
  EXTUB_REG_REG,
  EXTUW_REG_REG,
  MOV_IMM_REG,
  MOV_REG_REG,
  MOVB_xREGp_REG,
  MOVW_xREGp_REG,
  MOVL_xREGp_REG,
  MOVB_REG_xmREG,
  MOVW_REG_xmREG,
  MOVL_REG_xmREG,
  SWAPB_REG_REG,
  SWAPW_REG_REG,
  AND_REG_REG,
  OR_REG_REG,
};


void EmitInstr0(int codenum, FILE *outf);
//void EmitInstr2(int codenum, u8 param1, u8 param2, FILE *outf);
void EmitInstr2(int codenum, int param1, int param2, FILE *outf);


#endif /* !__OPCODES_H */
