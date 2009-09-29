#ifndef PARSER_H_
#define PARSER_H_

void rule_program           ();
void rule_varDecList        ();
void rule_varDecList2       ();
void rule_varDec            ();
void rule_funDefList        ();
void rule_funDef            ();
void rule_paramList         ();
void rule_instruction       ();
void rule_instructionBlock  ();
void rule_instructionList   ();
void rule_callInstruction   ();
void rule_setInstruction    ();
void rule_ifInstruction     ();
void rule_whileInstruction  ();
void rule_returnInstruction ();
void rule_writeInstruction  ();
void rule_voidInstruction   ();
void rule_expression        ();
void rule_conjunction       ();
void rule_comparison        ();
void rule_arithmeticExpr    ();
void rule_term              ();
void rule_factor            ();
void rule_variable          ();
void rule_funCall           ();
void rule_arguments         ();
void rule_exprList          ();
void rule_exprList2         ();

void entering (char *rule);
void exiting  (char *rule);
void expecting  (char *expected);
void next_token ();

#endif
