#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"

Node * rule_program           ();
Node * rule_varDecList        ();
Node * rule_varDecList2       ();
Node * rule_varDec            ();
Node * rule_funDefList        ();
Node * rule_funDef            ();
Node * rule_paramList         ();
Node * rule_instruction       ();
Node * rule_instructionBlock  ();
Node * rule_instructionList   ();
Node * rule_callInstruction   ();
Node * rule_setInstruction    ();
Node * rule_ifInstruction     ();
Node * rule_whileInstruction  ();
Node * rule_returnInstruction ();
Node * rule_writeInstruction  ();
Node * rule_voidInstruction   ();
Node * rule_expression        ();
Node * rule_conjunction       ();
Node * rule_comparison        ();
Node * rule_arithmeticExpr    ();
Node * rule_term              ();
Node * rule_factor            ();
Node * rule_variable          ();
Node * rule_funCall           ();
Node * rule_funCallExp        ();
Node * rule_arguments         ();
Node * rule_exprList          ();
Node * rule_exprList2         ();

void expecting  (char *expected);
void next_token ();

#endif
