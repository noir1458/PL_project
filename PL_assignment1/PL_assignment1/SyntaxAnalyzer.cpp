#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

// 전역 변수로 ID, CONST, OP 개수 추적
int idCount = 0;
int constCount = 0;
int opCount = 0;

// 각 카운터 초기화
void resetCounts() {
    idCount = 0;
    constCount = 0;
    opCount = 0;
}

// 트리를 순회하면서 노드 정보를 출력하는 함수
void printCounts() {
    cout << endl;
    cout << "ID: " << idCount << " CONST: " << constCount << " OP: " << opCount << endl;
}

// 심볼 테이블 정의
SymbolTable symTable;

// 파싱 트리 메모리 해제 함수
void freeTree(ParseTreeNode* root) {
    if (!root) return;

    // 모든 자식 노드에 대해 재귀적으로 메모리 해제
    for (ParseTreeNode* child : root->children) {
        freeTree(child);
    }

    delete root; // 현재 노드 메모리 해제
}

// <program> → <statements>
ParseTreeNode* program() {
    ParseTreeNode* node = new ParseTreeNode(0);
    ParseTreeNode* statementsNode = statements();
    node->addChild(statementsNode);
    symTable.print();
    return node;
}

// <statements> → <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = new ParseTreeNode(0);
    ParseTreeNode* statementNode = statement();
    node->addChild(statementNode);
    while (nextToken == SEMI_COLON) {
        lexical();  // 세미콜론 처리
        node->addChild(new ParseTreeNode(SEMI_COLON)); // 세미콜론 노드 추가


        // 중복된 세미콜론 경고 처리
        while (nextToken == SEMI_COLON) {
            node->setStatus(WARNING, "(Warning) \"중복 세미콜론(;) 제거\"");
            lexical();
        }

        statementNode = statement(); // 다음 statement 노드 생성
        node->addChild(statementNode); // STATEMENTS 노드에 다음 statement 추가
    }

    return node;
}

// <statement> → <ident> <assignment_op> <expression>
ParseTreeNode* statement() {
    ParseTreeNode* node = new ParseTreeNode(0);

    resetCounts();

    // IDENT 처리
    if (nextToken == IDENT) {
        idCount++;
        Symbol* symbol = symTable.findSymbol(lexeme);           // 심볼 테이블 lookup
        if (symbol == nullptr) {
            symbol = symTable.createSymbol(lexeme);     // 심볼 테이블에 변수 생성
        }
        ParseTreeNode* identNode = new ParseTreeNode(IDENT);    // IDENT 노드 생성
        node->addChild(identNode);                              // IDENT 노드를 STATEMENT에 추가
        lexical();                                              // IDENT 처리

        // ASSIGN_OP 처리
        if (nextToken == ASSIGN_OP) {
            node->addChild(new ParseTreeNode(ASSIGN_OP)); // ASSIGN_OP 노드 추가
            lexical();

            // 중복된 ASSIGN_OP 경고 처리
            while (nextToken == ASSIGN_OP) {
                node->setStatus(WARNING, "(Warning) \"중복 배정 연산자(:=) 제거\"");
                lexical();
            }

            // 표현식 처리
            ParseTreeNode* exprNode = expression();
            node->addChild(exprNode); // STATEMENT 노드에 expression 추가

            if (exprNode->isDefined) {
                identNode->setValue(exprNode->value);
                symTable.updateSymbol(symbol->name, identNode->value);
            }
        }
        else {
            // ASSIGN_OP 누락 에러 처리
            node->setStatus(ERROR, "(Error) \"배정 연산자(:=)가 필요합니다.\"");
        }
    } 
    else {
        // 식별자 누락 에러 처리
        node->setStatus(ERROR, "(Error) \"식별자가 필요합니다.\"");
    }
    printCounts();
    node->printMessage();
    return node;
}

// <expression> → <term> <term_tail>
ParseTreeNode* expression() {
    ParseTreeNode* node = new ParseTreeNode(0);

    ParseTreeNode* termNode = term();
    node->addChild(termNode);
    ParseTreeNode* termTailNode = term_tail(termNode->value);
    node->addChild(termTailNode);

    if (termNode->isDefined && termTailNode->isDefined) {
        node->setValue(termTailNode->value);
    }

    return node;
}

// <term> → <factor> <factor_tail>
ParseTreeNode* term() {
    ParseTreeNode* node = new ParseTreeNode(0);

    ParseTreeNode* factorNode = factor();
    node->addChild(factorNode);
    ParseTreeNode* factorTailNode = factor_tail(factorNode->value);
    node->addChild(factorTailNode);

    if (factorNode->isDefined && factorTailNode->isDefined) {
        node->setValue(factorTailNode->value);
    }

    return node;
}

// <term_tail> → <add_op> <term> <term_tail> | ε
ParseTreeNode* term_tail(int leftValue) {
    ParseTreeNode* node = new ParseTreeNode(0);
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        opCount++;
        int op = nextToken;
        string opName = lexeme;
        node->addChild(new ParseTreeNode(nextToken));
        lexical();

        while (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            if (op != nextToken) {
                string message = "(Error) \"서로 다른 연산자 " + opName + ", " + lexeme + " 사용\"";
                node->setStatus(ERROR, message);
                break;
            }
            else {
                string message = "(Warning) \"중복 연산자(" + (string) lexeme + ") 제거\"";
                node->setStatus(WARNING, message);
            }

            lexical();
        }

        ParseTreeNode* termNode = term();
        node->addChild(termNode);
        ParseTreeNode* termTailNode = term_tail(termNode->value);
        node->addChild(termTailNode);

        if (termNode->isDefined && termTailNode->isDefined && node->status != -1) {
            if (op == ADD_OP) {
                node->setValue(leftValue + termTailNode->value);
            }
            else {
                node->setValue(leftValue - termTailNode->value);
            }
        }
    }
    else {
        node->setValue(leftValue);
    }
    return node;
}

// <factor> → <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = new ParseTreeNode(0);

    if (nextToken == LEFT_PAREN) {
        node->addChild(new ParseTreeNode(LEFT_PAREN));
        lexical();  // '(' 처리
        ParseTreeNode* expressionNode = expression(); // 괄호 안의 표현식
        node->addChild(expressionNode);

        if (nextToken == RIGHT_PAREN) {
            node->addChild(new ParseTreeNode(RIGHT_PAREN));
            lexical();  // ')' 처리
            if (nextToken == RIGHT_PAREN) {
                node->setStatus(ERROR, "(Error) \"좌괄호 매칭 불가\"");
                return node;
            }
            if (expressionNode->isDefined) {
                node->setValue(expressionNode->value);
            }
        }
        else {
            node->setStatus(ERROR, "(Error) \"우괄호 매칭 불가\"");
            return node;
        }
    }
    else if (nextToken == IDENT) {
        idCount++;
        ParseTreeNode* identNode = new ParseTreeNode(IDENT);
        node->addChild(identNode);

        Symbol* symbol = symTable.findSymbol(lexeme);
        if (symbol == nullptr) {
            symTable.createSymbol(lexeme);
            node->setStatus(ERROR, "(Error) \"정의되지 않은 변수(" + (string) lexeme + ")가 참조됨\"");
        }
        else if (symbol->isDefined) {
            identNode->setValue(symbol->value);
            node->setValue(identNode->value);
        }
        lexical();
    }
    else if (nextToken == INT_LIT) {
        constCount++;
        ParseTreeNode* constNode = new ParseTreeNode(INT_LIT);
        node->addChild(constNode);
        constNode->setValue(atoi(lexeme));
        node->setValue(constNode->value);

        lexical();
    }
    return node;
}

// <factor_tail> → <mult_op> <factor> <factor_tail> | ε
ParseTreeNode* factor_tail(int leftValue) {
    ParseTreeNode* node = new ParseTreeNode(0);

    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        opCount++;
        int op = nextToken;
        string opName = lexeme;
        node->addChild(new ParseTreeNode(nextToken));
        lexical();

        while (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            if (op != nextToken) {
                string message = "(Error) \"서로 다른 연산자 " + opName + ", " + lexeme + " 사용\"";
                node->setStatus(ERROR, message);
                break;
            }
            else {
                string message = "(Warning) \"중복 연산자(" + (string)lexeme + ") 제거\"";
                node->setStatus(WARNING, message);
            }

            lexical();
        }

        ParseTreeNode* factorNode = factor();
        node->addChild(factorNode);
        ParseTreeNode* factorTailNode = factor_tail(factorNode->value);
        node->addChild(factorTailNode);

        if (factorNode->isDefined && factorTailNode->isDefined) {
            if (op == MULT_OP) {
                node->setValue(leftValue * factorTailNode->value);
            }
            else if (factorTailNode->value == 0)
            {
                node->setStatus(ERROR, "(Error) \"Division By Zero\"");
            }
        }
    }
    else {
        node->setValue(leftValue);
    }
    return node;
}