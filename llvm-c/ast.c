#include <stdlib.h>

#include "ast.h"

/* Global AST Root */
Ast AstRoot = NULL;

/**
 * Constructor for Instruction.
 */
Instruction NewInstruction(InstructionSymbol symbol, int parameter) {
  Instruction instruction = (Instruction)calloc(sizeof(struct _Instruction), 1);
  instruction->symbol = symbol;
  instruction->parameter = parameter;
  return instruction;
}

/**
 * Destructor for Instruction.
 */
void DisposeInstruction(Instruction instruction) {
  if (instruction != NULL) {
    free(instruction);
  }
}

/* Ast */

/**
 * Constructor for Instruction Node.
 */
Ast NewInstructionNode(NodeType type, Instruction instruction, Ast previous) {
  Ast node = (Ast)calloc(sizeof(struct _Ast), 1);
  node->type = type;
  node->instruction = instruction;
  node->previous = previous;
  return node;
}

/**
 * Constructor for Block Node.
 */
Ast NewBlockNode(NodeType type, Ast block, Ast previous) {
  Ast node = (Ast)calloc(sizeof(struct _Ast), 1);
  node->type = type;
  node->block = block;
  node->previous = previous;
  return node;
}

/**
 * Destructor for AST: free whole tree.
 */
void DisposeAst(Ast ast) {
  while (ast != NULL) {
    if (ast->type == InstructionNode) {
      DisposeInstruction(ast->instruction);
    } else {
      DisposeAst(ast->block);
    }
    Ast previous = ast->previous;
    free(ast);
    ast = previous;
  }
}
