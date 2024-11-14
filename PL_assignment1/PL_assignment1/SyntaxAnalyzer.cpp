#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

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
    cout << endl;
    cout << "ID: " << idCount << " CONST: " << constCount << " OP: " << opCount << endl;
}

// �ɺ� ���̺� ����
SymbolTable symTable;

// �Ľ� Ʈ�� �޸� ���� �Լ�
void freeTree(ParseTreeNode* root) {
    if (!root) return;

    // ��� �ڽ� ��忡 ���� ��������� �޸� ����
    for (ParseTreeNode* child : root->children) {
        freeTree(child);
    }

    delete root; // ���� ��� �޸� ����
}

// <program> �� <statements>
ParseTreeNode* program() {
    ParseTreeNode* node = new ParseTreeNode(0);
    ParseTreeNode* statementsNode = statements();
    node->addChild(statementsNode);
    symTable.print();
    return node;
}

// <statements> �� <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = new ParseTreeNode(0);
    ParseTreeNode* statementNode = statement();
    node->addChild(statementNode);
    while (nextToken == SEMI_COLON) {
        lexical();  // �����ݷ� ó��
        node->addChild(new ParseTreeNode(SEMI_COLON)); // �����ݷ� ��� �߰�


        // �ߺ��� �����ݷ� ��� ó��
        while (nextToken == SEMI_COLON) {
            node->setStatus(WARNING, "(Warning) \"�ߺ� �����ݷ�(;) ����\"");
            lexical();
        }

        statementNode = statement(); // ���� statement ��� ����
        node->addChild(statementNode); // STATEMENTS ��忡 ���� statement �߰�
    }

    return node;
}

// <statement> �� <ident> <assignment_op> <expression>
ParseTreeNode* statement() {
    ParseTreeNode* node = new ParseTreeNode(0);

    resetCounts();

    // IDENT ó��
    if (nextToken == IDENT) {
        idCount++;
        Symbol* symbol = symTable.findSymbol(lexeme);           // �ɺ� ���̺� lookup
        if (symbol == nullptr) {
            symbol = symTable.createSymbol(lexeme);     // �ɺ� ���̺� ���� ����
        }
        ParseTreeNode* identNode = new ParseTreeNode(IDENT);    // IDENT ��� ����
        node->addChild(identNode);                              // IDENT ��带 STATEMENT�� �߰�
        lexical();                                              // IDENT ó��

        // ASSIGN_OP ó��
        if (nextToken == ASSIGN_OP) {
            node->addChild(new ParseTreeNode(ASSIGN_OP)); // ASSIGN_OP ��� �߰�
            lexical();

            // �ߺ��� ASSIGN_OP ��� ó��
            while (nextToken == ASSIGN_OP) {
                node->setStatus(WARNING, "(Warning) \"�ߺ� ���� ������(:=) ����\"");
                lexical();
            }

            // ǥ���� ó��
            ParseTreeNode* exprNode = expression();
            node->addChild(exprNode); // STATEMENT ��忡 expression �߰�

            if (exprNode->isDefined) {
                identNode->setValue(exprNode->value);
                symTable.updateSymbol(symbol->name, identNode->value);
            }
        }
        else {
            // ASSIGN_OP ���� ���� ó��
            node->setStatus(ERROR, "(Error) \"���� ������(:=)�� �ʿ��մϴ�.\"");
        }
    } 
    else {
        // �ĺ��� ���� ���� ó��
        node->setStatus(ERROR, "(Error) \"�ĺ��ڰ� �ʿ��մϴ�.\"");
    }
    printCounts();
    node->printMessage();
    return node;
}

// <expression> �� <term> <term_tail>
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

// <term> �� <factor> <factor_tail>
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

// <term_tail> �� <add_op> <term> <term_tail> | ��
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
                string message = "(Error) \"���� �ٸ� ������ " + opName + ", " + lexeme + " ���\"";
                node->setStatus(ERROR, message);
                break;
            }
            else {
                string message = "(Warning) \"�ߺ� ������(" + (string) lexeme + ") ����\"";
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

// <factor> �� <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = new ParseTreeNode(0);

    if (nextToken == LEFT_PAREN) {
        node->addChild(new ParseTreeNode(LEFT_PAREN));
        lexical();  // '(' ó��
        ParseTreeNode* expressionNode = expression(); // ��ȣ ���� ǥ����
        node->addChild(expressionNode);

        if (nextToken == RIGHT_PAREN) {
            node->addChild(new ParseTreeNode(RIGHT_PAREN));
            lexical();  // ')' ó��
            if (nextToken == RIGHT_PAREN) {
                node->setStatus(ERROR, "(Error) \"�°�ȣ ��Ī �Ұ�\"");
                return node;
            }
            if (expressionNode->isDefined) {
                node->setValue(expressionNode->value);
            }
        }
        else {
            node->setStatus(ERROR, "(Error) \"���ȣ ��Ī �Ұ�\"");
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
            node->setStatus(ERROR, "(Error) \"���ǵ��� ���� ����(" + (string) lexeme + ")�� ������\"");
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

// <factor_tail> �� <mult_op> <factor> <factor_tail> | ��
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
                string message = "(Error) \"���� �ٸ� ������ " + opName + ", " + lexeme + " ���\"";
                node->setStatus(ERROR, message);
                break;
            }
            else {
                string message = "(Warning) \"�ߺ� ������(" + (string)lexeme + ") ����\"";
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