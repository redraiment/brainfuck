%{
#include "scanner.h"
#include "parser.h"
#include "ast.h"

void yyerror(char*);
%}

%union {
  struct _Ast* ast;
  struct _Instruction* instruction;
  yytoken_kind_t token;
}

%token <token> INCREMENT DECREMENT FORWARD BACKWARD INPUT OUTPUT WHILE WEND

%type <ast> commands command
%type <instruction> form

%%

brainfuck: commands { AstRoot = $1; }
  ;

commands: %empty { $$ = NULL; }
  | commands command {
    $2->previous = $1;
    $$ = $2;
  }
  ;

command: form { $$ = NewInstructionNode(InstructionNode, $1, NULL); }
  | WHILE commands WEND { $$ = NewBlockNode(BlockNode, $2, NULL); }
  ;

form: INCREMENT { $$ = NewInstruction(UpdateInstruction, 1); }
  | DECREMENT { $$ = NewInstruction(UpdateInstruction, -1); }
  | FORWARD { $$ = NewInstruction(MoveInstruction, 1); }
  | BACKWARD { $$ = NewInstruction(MoveInstruction, -1); }
  | INPUT { $$ = NewInstruction(InputInstruction, 1); }
  | OUTPUT { $$ = NewInstruction(OutputInstruction, 1); }
  ;

%%

/**
 * Global error handler.
 */
void yyerror(char* message) {
  fprintf(stderr, "Error at line %d: %s\n", yylineno, message);
}
