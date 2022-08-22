%option noyywrap nodefault yylineno
%{
#include "options.h"
#include "parser.h"
%}

%x COMMENT

%%

"#" {
  if (options.singleLineCommentEnabled) {
    BEGIN COMMENT;
  }
}

<COMMENT>. {
  /* Ignore in comment */
}

<COMMENT>\n {
  BEGIN INITIAL;
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
