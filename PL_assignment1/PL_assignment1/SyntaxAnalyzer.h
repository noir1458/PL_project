#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// 어휘 분석기에서 사용되는 함수 및 변수에 대한 참조를 포함
#include "LexicalAnalyzer.h"
#include <cstring>

// 심볼 테이블을 위한 구조체
struct SymbolTable {
    char name[100];
    int value;
    bool isDefined;
};

// 전역 변수 선언 (extern 사용)
extern SymbolTable symTable[100];   // 전역 구조체 배열 선언
extern int symTableSize;            // 심볼 테이블 크기 선언

// 심볼 테이블 관련 함수 선언
SymbolTable* findOrCreateSymbol(const char* name);
void updateSymbolTable(const char* variable, int value);

// 파싱 트리 노드 구조체
struct ParseTreeNode {
    int token;                   // 토큰 유형
    char* value;                 // 노드의 값 (예: 식별자 이름 또는 리터럴 값)
    ParseTreeNode* child;        // 현재 노드의 첫 번째 자식 (왼쪽 아래)
    ParseTreeNode* sibling;      // 가장 가까운 형제 노드 (바로 오른쪽)

    // 생성자
    ParseTreeNode(int token, const char* valueText)
        : token(token), child(nullptr), sibling(nullptr), value(nullptr) {
        value = new char[strlen(valueText) + 1];
        strcpy(value, valueText);
    }

    // 소멸자 (메모리 해제)
    ~ParseTreeNode() {
        delete[] value;
    }
};

// 파싱 트리 관련 함수 선언
ParseTreeNode* createNode(int token, const char* value);
void addChild(ParseTreeNode* parent, ParseTreeNode* child);
void printTree(ParseTreeNode* root, int level = 0);
void freeTree(ParseTreeNode* root); // 메모리 해제를 위한 함수

// 구문 분석 함수 선언
ParseTreeNode* program();
ParseTreeNode* statements();
ParseTreeNode* statement();
ParseTreeNode* expression();
ParseTreeNode* term();
ParseTreeNode* factor();
ParseTreeNode* term_tail(ParseTreeNode* leftNode);
ParseTreeNode* factor_tail(ParseTreeNode* leftNode);
int evaluateExpression(ParseTreeNode* node);

#endif // SYNTAX_ANALYZER_H