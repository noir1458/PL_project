#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// 어휘 분석기에서 사용되는 함수 및 변수에 대한 참조를 포함
#include "LexicalAnalyzer.h"

// 전역 변수로 ID, CONST, OP 개수 추적
extern int idCount;
extern int constCount;
extern int opCount;
extern bool statementError;
extern bool statementWarning;

// 함수 선언
void program();
void statements();
void statement();
int expression();
int term();
int factor();
int term_tail(int);
int factor_tail(int);

#endif // SYNTAX_ANALYZER_H