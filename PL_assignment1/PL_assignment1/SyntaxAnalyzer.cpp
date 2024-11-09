#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

int parseStatus = OK;
const char* parseStatusMessage = "(OK)";

// ���� ������ ID, CONST, OP ���� ����
int idCount = 0;
int constCount = 0;
int opCount = 0;

// �� ī���� �ʱ�ȭ
void resetCounts() {
    idCount = 0;
    constCount = 0;
    opCount = 0;
}

// Ʈ���� ��ȸ�ϸ鼭 ��� ������ ����ϴ� �Լ�
void printCounts() {
    cout << "ID: " << idCount << " CONST: " << constCount << " OP: " << opCount << endl;
}

void printStatus(const char* message) {
    cout << message << endl;
}

// �ɺ� ���̺� ����
SymbolTable symTable[100];
int symTableSize = 0;

// �ɺ� ���̺� ���� ����
SymbolTable* createSymbol(const char* name) {
    // �ɺ� ���̺� �� �׸� �߰�
    strcpy(symTable[symTableSize].name, name);
    symTable[symTableSize].value = 0;  // �⺻��
    symTable[symTableSize].isDefined = false;
    // cout << name << endl;
    return &symTable[symTableSize++];
}

// �ɺ� ���̺��� ���� �˻�
SymbolTable* findSymbol(const char* name) {
    for (int i = 0; i < symTableSize; i++) {
        if (strcmp(symTable[i].name, name) == 0) {
            return &symTable[i];
        }
    }
    return nullptr;
}

// �ɺ� ���̺� �� ������Ʈ
void updateSymbolTable(const char* variable, int value) {
    SymbolTable* symbol = findSymbol(variable);
    symbol->value = value;
    symbol->isDefined = true;
}

// �ɺ� ���̺��� ���ǵ� ���� ���� �������ų�, ������ �� Unknown���� ǥ��
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

// Ʈ�� ��带 �����ϰ� ��ȯ�ϴ� �Լ�
ParseTreeNode* createNode(int token, const char* valueText) {
    return new ParseTreeNode(token, valueText);
}

// �θ� ��忡 �ڽ� ��带 �߰��ϴ� �Լ�
void addChild(ParseTreeNode* parent, ParseTreeNode* child) {
    // �θ� ��忡 �ڽ��� �ϳ��� ������ �ٷ� �ڽĳ��� �߰�
    if (!parent->child) {
        parent->child = child;
    }
    // �θ� ��忡 �ڽ��� �ϳ��� �ִٸ�, ������ �ڽ� ���(ù �ڽ� ����� ���� ������ sibling)�� sibling���� �߰�
    else {
        ParseTreeNode* current = parent->child;
        while (current->sibling) {
            current = current->sibling;
        }
        current->sibling = child;
    }
}

// �Ľ� Ʈ�� �޸� ���� �Լ�
void freeTree(ParseTreeNode* root) {
    if (!root) return;
    freeTree(root->child);
    freeTree(root->sibling);
    delete root;
}

// <program> �� <statements>
ParseTreeNode* program() {
    ParseTreeNode* root = createNode(0, "PROGRAM");
    ParseTreeNode* statementsNode = statements();
    addChild(root, statementsNode);
    return root;
}

// <statements> �� <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = createNode(0, "STATEMENTS");
    ParseTreeNode* StatementNode = statement();
    addChild(node, StatementNode);
    while (nextToken == SEMI_COLON) {
        lexical();  // �����ݷ� ó��
        addChild(node, createNode(SEMI_COLON, ";"));  // �����ݷ� ��� �߰�
        while (nextToken == SEMI_COLON) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"�ߺ� �����ݷ� ����\"";
            }
            lexical();
        }
        StatementNode = statement();
        addChild(node, StatementNode);  // ���� Statement �߰�
    }
    if (nextToken == EOF) {
        addChild(node, createNode(EOF, ""));
    }
    return node;
}

// <statement> �� <ident> <assignment_op> <expression>
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
                    parseStatusMessage = "Warning: \"�ߺ� ������ ����\"";
                }
                lexical();
            }
            ParseTreeNode* exprNode = expression();
            addChild(node, exprNode);
        }
        // else ����
    }
    // else ����
    return node;
}

// <expression> �� <term> <term_tail>
ParseTreeNode* expression() {
    ParseTreeNode* node = createNode(0, "EXPRESSION");
    ParseTreeNode* termNode = term();
    addChild(node, termNode);
    ParseTreeNode* termTailNode = term_tail();
    addChild(node, termTailNode);
    return node;
}

// <term> �� <factor> <factor_tail>
ParseTreeNode* term() {
    ParseTreeNode* node = createNode(0, "TERM");
    ParseTreeNode* factorNode = factor();
    addChild(node, factorNode);
    ParseTreeNode* factorTailNode = factor_tail();
    addChild(node, factorTailNode);
    return node;
}

// <term_tail> �� <add_op> <term> <term_tail> | ��
ParseTreeNode* term_tail() {
    ParseTreeNode* node = createNode(0, "TERM TAIL");
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        int op = nextToken;
        addChild(node, createNode(nextToken, lexeme));
        lexical();

        while (nextToken == op) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"�ߺ� ������(+|-) ����\"";
            }
            lexical();
        }

        ParseTreeNode* termNode = term();
        addChild(node, termNode);
        ParseTreeNode* termTailNode = term_tail();
        addChild(node, termTailNode);
    }
    // else�� ���� �����̹Ƿ� ���� �ƴ�
    return node;
}

// <factor> �� <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = createNode(0, "FACTOR");

    if (nextToken == LEFT_PAREN) {
        addChild(node, createNode(LEFT_PAREN, "("));
        lexical();  // '(' ó��
        ParseTreeNode* expressionNode = expression(); // ��ȣ ���� ǥ����
        addChild(node, expressionNode);
        if (nextToken == RIGHT_PAREN) {
            addChild(node, createNode(RIGHT_PAREN, ")"));
            lexical();  // ')' ó��
            if (nextToken == RIGHT_PAREN) {
                if (parseStatus != ERROR) {
                    parseStatus = ERROR;
                    parseStatusMessage = "Error: \"�°�ȣ ��Ī �Ұ�\"";
                }
            }
        }
        else {
            if (parseStatus != ERROR) {
                parseStatus = ERROR;
                parseStatusMessage = "Error: \"���ȣ ��Ī �Ұ�\"";
            }
        }
    }
    else if (nextToken == IDENT) {
        SymbolTable* symbol = findSymbol(lexeme);
        if (symbol == nullptr) {
            createSymbol(lexeme);
            if (parseStatus != ERROR) {
                parseStatus = ERROR;
                parseStatusMessage = "Error: \"���ǵ��� ���� ������ ������\"";
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

// <factor_tail> �� <mult_op> <factor> <factor_tail> | ��
ParseTreeNode* factor_tail() {
    ParseTreeNode* node = createNode(0, "FACTOR TAIL");

    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        int op = nextToken;
        addChild(node, createNode(nextToken, lexeme));
        lexical();

        while (nextToken == op) {
            if (parseStatus == OK) {
                parseStatus = WARNING;
                parseStatusMessage = "Warning: \"�ߺ� ������(*|/) ����\"";
            }
            lexical();
        }

        ParseTreeNode* factorNode = factor();
        addChild(node, factorNode);
        ParseTreeNode* factorTailNode = factor_tail();
        addChild(node, factorTailNode);
    }
    // else�� ���� �����̹Ƿ� ���� �ƴ�
    return node;
}

int evaluateParseTree(ParseTreeNode* node, int leftValue) {
    if (!node) {
        return 0;  // �⺻ ��ȯ ��
    }

    // Nonterminal ���: �ڽ� ���� ���
    if (strcmp(node->value, "PROGRAM") == 0) {
        return evaluateParseTree(node->child);  // STATEMENTS ��� ��
    }

    if (strcmp(node->value, "STATEMENTS") == 0) {
        ParseTreeNode* statementNode = node->child;
        int lastValue = 0;

        // �� STATEMENT ��带 ��, EOF�� ��Ÿ���� ����
        while (statementNode) {
            lastValue = evaluateParseTree(statementNode);  // �� STATEMENT ��� ��
            statementNode = statementNode->sibling;  // ���� STATEMENT�� �̵� (���� ���)
        }
        // cout << "STATEMENTS: " << lastValue << endl;
        cout << endl;
        return lastValue;  // ������ ���� ��� ��ȯ, �ʿ� ������ int ��ȯ�ؾ� ��
    }

    if (strcmp(node->value, "STATEMENT") == 0) {
        if (node->child->token == IDENT && node->child->sibling && node->child->sibling->token == ASSIGN_OP) {
            ParseTreeNode* identNode = node->child;
            evaluateParseTree(identNode);

            int assignValue = evaluateParseTree(node->child->sibling);  // EXPRESSION ��� �� �� ����

            updateSymbolTable(identNode->value, assignValue);
            // cout << "STATEMENT: " << exprValue << endl;

            return assignValue;           // �ʿ� ������ int ��ȯ�ؾ� ��
        }
        return 0;
    }

    if (strcmp(node->value, "EXPRESSION") == 0) {
        int termValue = evaluateParseTree(node->child);                             // TERM ��� ��
        int termTailValue = evaluateParseTree(node->child->sibling, termValue);     // TERM TAIL ��� ��
        if (termValue == UNKNOWN || termTailValue == UNKNOWN) {
            return UNKNOWN;
        }
        // cout << "EXPRESSION " << termTailValue << endl;
        return termTailValue;
    }

    if (strcmp(node->value, "TERM") == 0) {
        int factorValue = evaluateParseTree(node->child);                           // FACTOR ��� ��
        int factorTailValue = evaluateParseTree(node->child->sibling, factorValue); // FACTOR TAIL ��� ��
        if (factorValue == UNKNOWN || factorTailValue == UNKNOWN) {
            return UNKNOWN;
        }
        // cout << "TERM: " << factorTailValue << endl;
        return factorTailValue;
    }

    if (strcmp(node->value, "TERM TAIL") == 0) {
        if (node->child && (node->child->token == ADD_OP || node->child->token == SUB_OP)) {
            int termValue = evaluateParseTree(node->child, leftValue);          // leftValue (+|-) TERM ��� ��
            int termTailValue = evaluateParseTree(node->child->sibling->sibling, termValue);
            // cout << "TERM TAIL OP: " << termTailValue << endl;
            return termTailValue;                                                // TERM TAIL ��� ��
        }
        // cout << "TERM TAIL: " << leftValue << endl;
        return leftValue;
    }

    if (strcmp(node->value, "FACTOR") == 0) {
        if (node->child->token == LEFT_PAREN) {  // ��ȣ ǥ���� ó��
            int expressionValue = evaluateParseTree(node->child);  // EXPRESSION ��
            // cout << "FACTOR PAREN: " << expressionValue << endl;
            return expressionValue;
        }
        else {
            int value = evaluateParseTree(node->child);  // IDENT �Ǵ� INT_LIT ��
            // cout << "FACTOR IDENT | INT: " << value << endl;
            return value;
        }
    }

    if (strcmp(node->value, "FACTOR TAIL") == 0) {
        if (node->child && (node->child->token == MULT_OP || node->child->token == DIV_OP)) {
            int factorValue = evaluateParseTree(node->child, leftValue);  // leftValue (*|/) FACTOR ��
            int factorTailValue = evaluateParseTree(node->child->sibling->sibling, factorValue);  // FACTOR TAIL ��� ��
            // cout << "FACTOR TAIL OP: " << factorTailValue << endl;
            return factorTailValue;
        }
        // cout << "FACTOR TAIL: " << leftValue << endl;
        return leftValue;
    }

    // Terminal ���: ���� ���� ��� �̵�
    if (node->token == INT_LIT) {
        constCount++;
        cout << atoi(node->value) << " ";
        return atoi(node->value);  // ��� ���ͷ� �� ��ȯ
    }

    if (node->token == IDENT) {
        idCount++;
        cout << node->value << " ";
        SymbolTable* symbol = findSymbol(node->value);
        if (symbol->isDefined) {
            return symbol->value;  // ���ǵ� ���� ���� ��ȯ
        }
        return UNKNOWN;
    }

    if (node->token == ADD_OP || node->token == SUB_OP) {
        opCount++;
        cout << node->value << " ";
        int rightValue = evaluateParseTree(node->sibling); // ������ �ǿ�����
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
        int rightValue = evaluateParseTree(node->sibling); // ������ �ǿ�����
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
        int rightValue = evaluateParseTree(node->sibling); // ������ �ǿ�����

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


    return 0;  // �⺻ ��ȯ ��
}
