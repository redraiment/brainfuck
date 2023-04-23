#ifndef __AST_H_
#define __AST_H_

typedef enum {
  UpdateInstruction = 0,
  MoveInstruction,
  InputInstruction,
  OutputInstruction
} InstructionSymbol;

typedef struct _Instruction {
  InstructionSymbol symbol;
  int parameter;
} *Instruction;

typedef enum {
  InstructionNode = 0,
  BlockNode
} NodeType;

typedef struct _Ast {
  NodeType type;
  union {
    struct _Instruction* instruction;
    struct _Ast* block;
  };
  struct _Ast* previous;
} *Ast;

extern Ast AstRoot;

Instruction NewInstruction(InstructionSymbol, int);
void DisposeInstruction(Instruction);

Ast NewInstructionNode(NodeType, Instruction, Ast);
Ast NewBlockNode(NodeType, Ast, Ast);
void DisposeAst(Ast);

void OptimizeAst(Ast);

#endif
