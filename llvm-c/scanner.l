%option noyywrap nodefault yylineno
%{
#include "parser.h"
%}
%%

"#".* {
  /* Single line comment */
}

">" {
  return FORWARD;
}

"<" {
  return BACKWARD;
}

"+" {
  return INCREMENT;
}

"-" {
  return DECREMENT;
}

"," {
  return INPUT;
}

"." {
  return OUTPUT;
}

"[" {
  return WHILE;
}

"]" {
  return WEND;
}

.|\n {
  /* Ignore comments */
}

%%
