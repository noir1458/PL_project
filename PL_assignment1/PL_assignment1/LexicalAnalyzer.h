#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <cstdio>

extern int charClass;
extern char lexeme[100];
extern char nextChar;
extern int lexLen;
extern int token;
extern int nextToken;
extern FILE* in_fp;

// 문자 유형 상수
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

// 토큰 코드 상수
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26
#define SEMI_COLON 27

// 함수 선언
void addChar();
void getChar();
void getNonBlank();
int lexical();
int lookup(char ch);

#endif // LEXICAL_ANALYZER_H
