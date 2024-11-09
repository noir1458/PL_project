#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

int parseStatus = OK;
const char* parseStatusMessage = "(OK)";

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
    cout << "ID: " << idCount << " CONST: " << constCount << " OP: " << opCount << endl;
}

void printStatus(const char* message) {
    cout << message << endl;
}

// 심볼 테이블 정의
SymbolTable symTable[100];
int symTableSize = 0;

// 심볼 테이블에 변수 생성
SymbolTable* createSymbol(const char* name) {
    // 심볼 테이블에 새 항목 추가
    strcpy(symTable[symTableSize].name, name);
    symTable[symTableSize].value = 0;  // 기본값
    symTable[symTableSize].isDefined = false;
    // cout << name << endl;
    return &symTable[symTableSize++];
}

// 심볼 테이블에서 변수 검색
SymbolTable* findSymbol(const char* name) {
    for (int i = 0; i < symTableSize; i++) {
        if (strcmp(symTable[i].name, name) == 0) {
            return &symTable[i];
        }
    }
    return nullptr;
}

// 심볼 테이블에 값 업데이트
void updateSymbolTable(const char* variable, int value) {
    SymbolTable* symbol = findSymbol(variable);
    symbol->value = value;
    symbol->isDefined = true;
}

// 심볼 테이블에서 정의된 변수 값을 가져오거나, 미정의 시 Unknown으로 표시
void printSymbolTable() {
    cout << "Result ==> ";
    for (int i = 0; i < symTableSize; i++) {
        cout << symTable[i].name << ": ";
        if (symTable[i].isDefined && symTable[i].value != UNKNOWN) {
            cout << symTable[i].value;
        }
        else {
            cout << "Unknown";
        }
        if (i < symTableSize - 1) cout << "; ";
    }
    cout << ";" << endl;
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

// 파싱 트리 메모리 해제 함수
void freeTree(ParseTreeNode* root) {
    if (!root) return;
    freeTree(root->child);
    freeTree(root->sibling);
    delete root;
}

// <program> → <statements>
ParseTreeNode* program() {
    ParseTreeNode* root = createNode(0, "PROGRAM");
    ParseTreeNode* statementsNode = statements();
    addChild(root, statementsNode);
    return root;
}

// <statements> → <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = createNode(0, "STATEMENTS");
    ParseTreeNode* StatementNode = statement();
    addChild(node, StatementNode);
    while (nextToken == SEMI_COLON) {
        lexical();  // 세미콜론 처리
        addChild(node, createNode(SEMI_COLON, ";"));  // 세미콜론 노드 추가
        while (nextToken == SEMI_COLON) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"중복 세미콜론 제거\"";
            }
            lexical();
        }
        StatementNode = statement();
        addChild(node, StatementNode);  // 다음 Statement 추가
    }
    if (nextToken == EOF) {
        addChild(node, createNode(EOF, ""));
    }
    return node;
}

// <statement> → <ident> <assignment_op> <expression>
ParseTreeNode* statement() {
    ParseTreeNode* node = createNode(0, "STATEMENT");

    if (nextToken == IDENT) {
        SymbolTable* symbol = createSymbol(lexeme);
        ParseTreeNode* identNode = createNode(IDENT, lexeme);
        addChild(node, identNode);
        lexical();

        if (nextToken == ASSIGN_OP) {
            addChild(node, createNode(ASSIGN_OP, ":="));
            lexical();
            while (nextToken == ASSIGN_OP) {
                if (parseStatus == OK) {
                    parseStatus = WARNING;
                    parseStatusMessage = "Warning: \"중복 배정문 제거\"";
                }
                lexical();
            }
            ParseTreeNode* exprNode = expression();
            addChild(node, exprNode);
        }
        // else 에러
    }
    // else 에러
    return node;
}

// <expression> → <term> <term_tail>
ParseTreeNode* expression() {
    ParseTreeNode* node = createNode(0, "EXPRESSION");
    ParseTreeNode* termNode = term();
    addChild(node, termNode);
    ParseTreeNode* termTailNode = term_tail();
    addChild(node, termTailNode);
    return node;
}

// <term> → <factor> <factor_tail>
ParseTreeNode* term() {
    ParseTreeNode* node = createNode(0, "TERM");
    ParseTreeNode* factorNode = factor();
    addChild(node, factorNode);
    ParseTreeNode* factorTailNode = factor_tail();
    addChild(node, factorTailNode);
    return node;
}

// <term_tail> → <add_op> <term> <term_tail> | ε
ParseTreeNode* term_tail() {
    ParseTreeNode* node = createNode(0, "TERM TAIL");
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        int op = nextToken;
        addChild(node, createNode(nextToken, lexeme));
        lexical();

        while (nextToken == op) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"중복 연산자(+|-) 제거\"";
            }
            lexical();
        }

        ParseTreeNode* termNode = term();
        addChild(node, termNode);
        ParseTreeNode* termTailNode = term_tail();
        addChild(node, termTailNode);
    }
    // else면 테일 종료이므로 에러 아님
    return node;
}

// <factor> → <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = createNode(0, "FACTOR");

    if (nextToken == LEFT_PAREN) {
        addChild(node, createNode(LEFT_PAREN, "("));
        lexical();  // '(' 처리
        ParseTreeNode* expressionNode = expression(); // 괄호 안의 표현식
        addChild(node, expressionNode);
        if (nextToken == RIGHT_PAREN) {
            addChild(node, createNode(RIGHT_PAREN, ")"));
            lexical();  // ')' 처리
            if (nextToken == RIGHT_PAREN) {
                if (parseStatus != ERROR) {
                    parseStatus = ERROR;
                    parseStatusMessage = "Error: \"좌괄호 매칭 불가\"";
                }
            }
        }
        else {
            if (parseStatus != ERROR) {
                parseStatus = ERROR;
                parseStatusMessage = "Error: \"우괄호 매칭 불가\"";
            }
        }
    }
    else if (nextToken == IDENT) {
        SymbolTable* symbol = findSymbol(lexeme);
        if (symbol == nullptr) {
            createSymbol(lexeme);
            if (parseStatus != ERROR) {
                parseStatus = ERROR;
                parseStatusMessage = "Error: \"정의되지 않은 변수가 참조됨\"";
            }
        }
        ParseTreeNode* identNode = createNode(IDENT, lexeme);
        addChild(node, identNode);
        lexical();
    }
    else if (nextToken == INT_LIT) {
        ParseTreeNode* constNode = createNode(INT_LIT, lexeme);
        addChild(node, constNode);
        lexical();
    }

    return node;
}

// <factor_tail> → <mult_op> <factor> <factor_tail> | ε
ParseTreeNode* factor_tail() {
    ParseTreeNode* node = createNode(0, "FACTOR TAIL");

    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        int op = nextToken;
        addChild(node, createNode(nextToken, lexeme));
        lexical();

        while (nextToken == op) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"중복 연산자(*|/) 제거\"";
            }
            lexical();
        }

        ParseTreeNode* factorNode = factor();
        addChild(node, factorNode);
        ParseTreeNode* factorTailNode = factor_tail();
        addChild(node, factorTailNode);
    }
    // else면 테일 종료이므로 에러 아님
    return node;
}

int evaluateParseTree(ParseTreeNode* node, int leftValue) {
    if (!node) {
        return 0;  // 기본 반환 값
    }

    // Nonterminal 노드: 자식 노드로 재귀
    if (strcmp(node->value, "PROGRAM") == 0) {
        return evaluateParseTree(node->child);  // STATEMENTS 노드 평가
    }

    if (strcmp(node->value, "STATEMENTS") == 0) {
        ParseTreeNode* statementNode = node->child;
        int lastValue = 0;

        // 각 STATEMENT 노드를 평가, EOF가 나타나면 종료
        while (statementNode) {
            lastValue = evaluateParseTree(statementNode);  // 각 STATEMENT 노드 평가
            statementNode = statementNode->sibling;  // 다음 STATEMENT로 이동 (형제 노드)
        }
        // cout << "STATEMENTS: " << lastValue << endl;
        cout << endl;
        return lastValue;  // 마지막 구문 결과 반환, 필요 없지만 int 반환해야 함
    }

    if (strcmp(node->value, "STATEMENT") == 0) {
        if (node->child->token == IDENT && node->child->sibling && node->child->sibling->token == ASSIGN_OP) {
            ParseTreeNode* identNode = node->child;
            evaluateParseTree(identNode);

            int assignValue = evaluateParseTree(node->child->sibling);  // EXPRESSION 노드 평가 후 배정

            updateSymbolTable(identNode->value, assignValue);
            // cout << "STATEMENT: " << exprValue << endl;

            return assignValue;           // 필요 없지만 int 반환해야 함
        }
        return 0;
    }

    if (strcmp(node->value, "EXPRESSION") == 0) {
        int termValue = evaluateParseTree(node->child);                             // TERM 노드 평가
        int termTailValue = evaluateParseTree(node->child->sibling, termValue);     // TERM TAIL 노드 평가
        if (termValue == UNKNOWN || termTailValue == UNKNOWN) {
            return UNKNOWN;
        }
        // cout << "EXPRESSION " << termTailValue << endl;
        return termTailValue;
    }

    if (strcmp(node->value, "TERM") == 0) {
        int factorValue = evaluateParseTree(node->child);                           // FACTOR 노드 평가
        int factorTailValue = evaluateParseTree(node->child->sibling, factorValue); // FACTOR TAIL 노드 평가
        if (factorValue == UNKNOWN || factorTailValue == UNKNOWN) {
            return UNKNOWN;
        }
        // cout << "TERM: " << factorTailValue << endl;
        return factorTailValue;
    }

    if (strcmp(node->value, "TERM TAIL") == 0) {
        if (node->child && (node->child->token == ADD_OP || node->child->token == SUB_OP)) {
            int termValue = evaluateParseTree(node->child, leftValue);          // leftValue (+|-) TERM 노드 평가
            int termTailValue = evaluateParseTree(node->child->sibling->sibling, termValue);
            // cout << "TERM TAIL OP: " << termTailValue << endl;
            return termTailValue;                                                // TERM TAIL 재귀 평가
        }
        // cout << "TERM TAIL: " << leftValue << endl;
        return leftValue;
    }

    if (strcmp(node->value, "FACTOR") == 0) {
        if (node->child->token == LEFT_PAREN) {  // 괄호 표현식 처리
            int expressionValue = evaluateParseTree(node->child);  // EXPRESSION 평가
            // cout << "FACTOR PAREN: " << expressionValue << endl;
            return expressionValue;
        }
        else {
            int value = evaluateParseTree(node->child);  // IDENT 또는 INT_LIT 평가
            // cout << "FACTOR IDENT | INT: " << value << endl;
            return value;
        }
    }

    if (strcmp(node->value, "FACTOR TAIL") == 0) {
        if (node->child && (node->child->token == MULT_OP || node->child->token == DIV_OP)) {
            int factorValue = evaluateParseTree(node->child, leftValue);  // leftValue (*|/) FACTOR 평가
            int factorTailValue = evaluateParseTree(node->child->sibling->sibling, factorValue);  // FACTOR TAIL 재귀 평가
            // cout << "FACTOR TAIL OP: " << factorTailValue << endl;
            return factorTailValue;
        }
        // cout << "FACTOR TAIL: " << leftValue << endl;
        return leftValue;
    }

    // Terminal 노드: 형제 노드로 재귀 이동
    if (node->token == INT_LIT) {
        constCount++;
        cout << atoi(node->value) << " ";
        return atoi(node->value);  // 상수 리터럴 값 반환
    }

    if (node->token == IDENT) {
        idCount++;
        cout << node->value << " ";
        SymbolTable* symbol = findSymbol(node->value);
        if (symbol->isDefined) {
            return symbol->value;  // 정의된 변수 값을 반환
        }
        return UNKNOWN;
    }

    if (node->token == ADD_OP || node->token == SUB_OP) {
        opCount++;
        cout << node->value << " ";
        int rightValue = evaluateParseTree(node->sibling); // 오른쪽 피연산자
        if (rightValue == UNKNOWN) {
            return UNKNOWN;
        }
        if (node->token == ADD_OP) {
            return leftValue + rightValue;
        }
        else {
            return leftValue - rightValue;
        }
    }

    if (node->token == MULT_OP || node->token == DIV_OP) {
        opCount++;
        cout << node->value << " ";
        int rightValue = evaluateParseTree(node->sibling); // 오른쪽 피연산자
        if (rightValue == UNKNOWN) {
            return UNKNOWN;
        }
        if (node->token == DIV_OP && rightValue == 0) {
            cout << "Error: Division by zero" << endl;
            return 0;
        }
        if (node->token == MULT_OP) {
            return leftValue * rightValue;
        }
        else {
            return leftValue / rightValue;
        }
    }

    if (node->token == ASSIGN_OP) {
        cout << node->value << " ";
        int rightValue = evaluateParseTree(node->sibling); // 오른쪽 피연산자

        return rightValue;
    }

    if (node->token == SEMI_COLON || node->token == EOF) {
        cout << "\b" << node->value << endl;
        printCounts();
        resetCounts();
    }

    if (node->token == LEFT_PAREN) {
        cout << node->value;
        int rightValue = evaluateParseTree(node->sibling);
        evaluateParseTree(node->sibling->sibling);
        return rightValue;
    }

    if (node->token == RIGHT_PAREN) {
        cout << "\b" << node->value << " ";
    }


    return 0;  // 기본 반환 값
}
