#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// 어휘 분석기에서 사용되는 함수 및 변수에 대한 참조를 포함
#include "LexicalAnalyzer.h"
#include <cstring>
#include <vector>
#include <iostream>

using namespace std;

#define OK 1
#define WARNING 0
#define ERROR -1

// 전역 변수로 ID, CONST, OP 개수 추적
extern int idCount;
extern int constCount;
extern int opCount;

// 심볼 구조체
struct Symbol {
    string name;
    int value;
    bool isDefined;
};

// 심볼 테이블 구조체
struct SymbolTable {
    Symbol symbols[100];
    int size;

    // 생성자 (크기를 0으로 초기화)
    SymbolTable() : size(0) {
        for (int i = 0; i < 100; i++) {
            symbols[i] = Symbol();  // 각 Symbol을 기본값으로 초기화
        }
    }

    // 심볼 생성 함수
    Symbol* createSymbol(const string& name) {
        symbols[size].name = name;
        symbols[size].value = 0;          // 기본값
        symbols[size].isDefined = false;
        return &symbols[size++];          // 새 심볼 반환 후 크기 증가
    }

    // 심볼 검색 함수
    Symbol* findSymbol(const string& name) {
        for (int i = 0; i < size; i++) {
            if (symbols[i].name == name) {
                return &symbols[i];
            }
        }
        return nullptr;
    }

    // 심볼 업데이트 함수
    void updateSymbol(const string& variable, int value) {
        Symbol* symbol = findSymbol(variable);
        if (symbol) {
            symbol->value = value;
            symbol->isDefined = true;
        }
    }

    // 심볼 테이블 출력 함수
    void print() const {
        cout << "Result ==> ";
        for (int i = 0; i < size; i++) {
            cout << symbols[i].name << ": ";
            if (symbols[i].isDefined) {
                cout << symbols[i].value;
            }
            else {
                cout << "Unknown";
            }
            if (i < size - 1) cout << "; ";
        }
        cout << ";" << endl;
    }
};

struct ParseTreeNode {
    int token;                   // 토큰 유형
    int value;                   // 노드의 값
    bool isDefined;              // 노드의 값이 초기화 되었는지 확인
    int status;                  // 노드의 상태 (OK, WARNING, ERROR 등)
    string message;              // 경고 및 오류 메시지 배열
    vector<ParseTreeNode*> children;  // 모든 자식 노드를 관리하는 벡터

    // 생성자
    ParseTreeNode(int token)
        : token(token), value(0), isDefined(false), status(OK), message("(OK)") {}

    // 자식 노드 추가 함수
    void addChild(ParseTreeNode* childNode) {
        children.push_back(childNode);
        // 상태 및 메시지 업데이트
        setStatus(childNode->status, childNode->message);
    }

    // 값 설정 함수
    void setValue(int newValue) {
        value = newValue;
        isDefined = true;
    }

    // 상태 및 메시지 설정 함수
    void setStatus(int newStatus, const string& newMessage) {
        if (status > newStatus) {
            status = newStatus;
            message = newMessage;
        }
    }

    // 메시지 출력
    void printMessage() const {
        if (!message.empty()) {
            cout << message << endl;
        }
    }
};

// 각 카운터 초기화
void resetCounts();

// 파싱 트리 관련 함수 선언
void freeTree(ParseTreeNode* root); // 메모리 해제를 위한 함수

// 구문 분석 함수 선언
ParseTreeNode* program();
ParseTreeNode* statements();
ParseTreeNode* statement();
ParseTreeNode* expression();
ParseTreeNode* term();
ParseTreeNode* factor();
ParseTreeNode* term_tail(int leftValue);
ParseTreeNode* factor_tail(int leftValue);

#endif // SYNTAX_ANALYZER_H