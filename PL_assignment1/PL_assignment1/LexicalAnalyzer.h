#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <cstdio>

// 심볼 테이블을 위한 구조체
struct SymbolTable {
    char name[100];
    int value;
    bool isDefined;
};

// 전역 변수 선언 (extern 사용)
extern SymbolTable symTable[100];   // 전역 구조체 배열 선언
extern int symTableSize;            // 심볼 테이블 크기 선언
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
