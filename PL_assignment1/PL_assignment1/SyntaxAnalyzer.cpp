#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

// 심볼 테이블 정의
SymbolTable symTable[100];
int symTableSize = 0;

// 심볼 테이블에서 변수 검색 또는 생성
SymbolTable* findOrCreateSymbol(const char* name) {
    for (int i = 0; i < symTableSize; i++) {
        if (strcmp(symTable[i].name, name) == 0) {
            return &symTable[i];
        }
    }
    // 심볼 테이블에 새 항목 추가
    strcpy(symTable[symTableSize].name, name);
    symTable[symTableSize].value = 0;  // 기본값
    symTable[symTableSize].isDefined = false;
    return &symTable[symTableSize++];
}

// 심볼 테이블에 값 업데이트
void updateSymbolTable(const char* variable, int value) {
    SymbolTable* symbol = findOrCreateSymbol(variable);
    symbol->value = value;
    symbol->isDefined = true;
}

// 트리 노드를 생성하고 반환하는 함수
ParseTreeNode* createNode(int token, const char* valueText) {
    return new ParseTreeNode(token, valueText);
}

// 부모 노드에 자식 노드를 추가하는 함수
void addChild(ParseTreeNode* parent, ParseTreeNode* child) {
    // 부모 노드에 자식이 하나도 없으면 바로 자식노드로 추가
    if (!parent->child) {
        parent->child = child;
    }
    // 부모 노드에 자식이 하나라도 있다면, 마지막 자식 노드(첫 자식 노드의 제일 마지막 sibling)의 sibling으로 추가
    else {
        ParseTreeNode* current = parent->child;
        while (current->sibling) {
            current = current->sibling;
        }
        current->sibling = child;
    }
}

// 트리를 출력하는 함수 (재귀적으로 출력)
void printTree(ParseTreeNode* root, int level) {
    if (!root) return;
    for (int i = 0; i < level; i++) cout << "  ";
    cout << root->value << " (" << root->token << ")" << endl;
    printTree(root->child, level + 1);
    printTree(root->sibling, level);
}

// 파싱 트리 메모리 해제 함수
void freeTree(ParseTreeNode* root) {
    if (!root) return;
    freeTree(root->child);
    freeTree(root->sibling);
    delete root;
}

// <program> → <statements>
ParseTreeNode* program() {
    ParseTreeNode* root = createNode(0, "Program");
    addChild(root, statements());
    return root;
}

// <statements> → <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = createNode(0, "Statements");
    addChild(node, statement());
    while (nextToken == SEMI_COLON) {
        lexical();  // 세미콜론 처리
        addChild(node, statement());  // 다음 statement 추가
    }
    return node;
}

// <statement> → <ident> <assignment_op> <expression>
ParseTreeNode* statement() {
    ParseTreeNode* node = createNode(0, "Statement");

    if (nextToken == IDENT) {
        char* variable = new char[strlen(lexeme) + 1];
        strcpy(variable, lexeme);  // lexeme의 내용을 복사
        ParseTreeNode* varNode = createNode(IDENT, variable);
        addChild(node, varNode);
        lexical();

        if (nextToken == ASSIGN_OP) {
            addChild(node, createNode(ASSIGN_OP, ":="));
            lexical();
            ParseTreeNode* exprNode = expression();
            addChild(node, exprNode);

            // 값 평가 후 심볼 테이블에 저장
            int exprValue = evaluateExpression(exprNode);
            updateSymbolTable(variable, exprValue);
        }

        // 메모리 해제
        delete[] variable;
    }
    return node;
}

// <expression> → <term> <term_tail>
ParseTreeNode* expression() {
    ParseTreeNode* node = term();
    return term_tail(node);
}

// <term> → <factor> <factor_tail>
ParseTreeNode* term() {
    ParseTreeNode* node = factor();
    return factor_tail(node);
}

// <term_tail> → <add_op> <term> <term_tail> | ε
ParseTreeNode* term_tail(ParseTreeNode* leftNode) {
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        ParseTreeNode* operatorNode = createNode(nextToken, lexeme);
        lexical();
        addChild(operatorNode, leftNode);
        addChild(operatorNode, term());
        return term_tail(operatorNode);
    }
    return leftNode;
}

// <factor> → <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = nullptr;

    if (nextToken == LEFT_PAREN) {
        lexical();  // '(' 처리
        node = expression();  // 괄호 안의 표현식
        if (nextToken == RIGHT_PAREN) {
            lexical();  // ')' 처리
        }
    }
    else if (nextToken == IDENT) {
        SymbolTable* symbol = findOrCreateSymbol(lexeme);
        char* identifier = new char[strlen(lexeme) + 1];
        strcpy(identifier, lexeme);
        node = createNode(IDENT, identifier);
        lexical();
    }
    else if (nextToken == INT_LIT) {
        char* intLit = new char[strlen(lexeme) + 1];
        strcpy(intLit, lexeme);
        node = createNode(INT_LIT, intLit);
        lexical();
    }

    return node;
}

// <factor_tail> → <mult_op> <factor> <factor_tail> | ε
ParseTreeNode* factor_tail(ParseTreeNode* leftNode) {
    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        ParseTreeNode* operatorNode = createNode(nextToken, lexeme);
        lexical();
        addChild(operatorNode, leftNode);
        addChild(operatorNode, factor());
        return factor_tail(operatorNode);
    }
    return leftNode;
}

// 트리를 평가하여 값을 계산하는 함수
int evaluateExpression(ParseTreeNode* node) {
    if (!node) return 0;

    if (node->token == INT_LIT) return atoi(node->value);

    if (node->token == IDENT) {
        SymbolTable* symbol = findOrCreateSymbol(node->value);
        if (symbol->isDefined) return symbol->value;
        cout << "Error: Undefined variable " << node->value << endl;
        return 0;
    }

    int leftValue = evaluateExpression(node->child);
    int rightValue = evaluateExpression(node->child->sibling);

    switch (node->token) {
    case ADD_OP: return leftValue + rightValue;
    case SUB_OP: return leftValue - rightValue;
    case MULT_OP: return leftValue * rightValue;
    case DIV_OP:
        if (rightValue == 0) {
            cout << "Error: Division by zero" << endl;
            return 0;
        }
        return leftValue / rightValue;
    default: return 0;
    }
}