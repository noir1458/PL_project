#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// ���� �м��⿡�� ���Ǵ� �Լ� �� ������ ���� ������ ����
#include "LexicalAnalyzer.h"

// ���� ������ ID, CONST, OP ���� ����
extern int idCount;
extern int constCount;
extern int opCount;
extern bool statementError;
extern bool statementWarning;

// �Լ� ����
void program();
void statements();
void statement();
int expression();
int term();
int factor();
int term_tail(int);
int factor_tail(int);

#endif // SYNTAX_ANALYZER_H