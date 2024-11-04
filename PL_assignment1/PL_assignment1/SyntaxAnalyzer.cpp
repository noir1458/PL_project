#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

// �ɺ� ���̺� ����
SymbolTable symTable[100];
int symTableSize = 0;

// �ɺ� ���̺��� ���� �˻� �Ǵ� ����
SymbolTable* findOrCreateSymbol(const char* name) {
    for (int i = 0; i < symTableSize; i++) {
        if (strcmp(symTable[i].name, name) == 0) {
            return &symTable[i];
        }
    }
    // �ɺ� ���̺� �� �׸� �߰�
    strcpy(symTable[symTableSize].name, name);
    symTable[symTableSize].value = 0;  // �⺻��
    symTable[symTableSize].isDefined = false;
    return &symTable[symTableSize++];
}

// �ɺ� ���̺� �� ������Ʈ
void updateSymbolTable(const char* variable, int value) {
    SymbolTable* symbol = findOrCreateSymbol(variable);
    symbol->value = value;
    symbol->isDefined = true;
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

// Ʈ���� ����ϴ� �Լ� (��������� ���)
void printTree(ParseTreeNode* root, int level) {
    if (!root) return;
    for (int i = 0; i < level; i++) cout << "  ";
    cout << root->value << " (" << root->token << ")" << endl;
    printTree(root->child, level + 1);
    printTree(root->sibling, level);
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
    ParseTreeNode* root = createNode(0, "Program");
    addChild(root, statements());
    return root;
}

// <statements> �� <statement> { ; <statement> }
ParseTreeNode* statements() {
    ParseTreeNode* node = createNode(0, "Statements");
    addChild(node, statement());
    while (nextToken == SEMI_COLON) {
        lexical();  // �����ݷ� ó��
        addChild(node, statement());  // ���� statement �߰�
    }
    return node;
}

// <statement> �� <ident> <assignment_op> <expression>
ParseTreeNode* statement() {
    ParseTreeNode* node = createNode(0, "Statement");

    if (nextToken == IDENT) {
        char* variable = new char[strlen(lexeme) + 1];
        strcpy(variable, lexeme);  // lexeme�� ������ ����
        ParseTreeNode* varNode = createNode(IDENT, variable);
        addChild(node, varNode);
        lexical();

        if (nextToken == ASSIGN_OP) {
            addChild(node, createNode(ASSIGN_OP, ":="));
            lexical();
            ParseTreeNode* exprNode = expression();
            addChild(node, exprNode);

            // �� �� �� �ɺ� ���̺� ����
            int exprValue = evaluateExpression(exprNode);
            updateSymbolTable(variable, exprValue);
        }

        // �޸� ����
        delete[] variable;
    }
    return node;
}

// <expression> �� <term> <term_tail>
ParseTreeNode* expression() {
    ParseTreeNode* node = term();
    return term_tail(node);
}

// <term> �� <factor> <factor_tail>
ParseTreeNode* term() {
    ParseTreeNode* node = factor();
    return factor_tail(node);
}

// <term_tail> �� <add_op> <term> <term_tail> | ��
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

// <factor> �� <left_paren> <expression> <right_paren> | <ident> | <const>
ParseTreeNode* factor() {
    ParseTreeNode* node = nullptr;

    if (nextToken == LEFT_PAREN) {
        lexical();  // '(' ó��
        node = expression();  // ��ȣ ���� ǥ����
        if (nextToken == RIGHT_PAREN) {
            lexical();  // ')' ó��
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

// <factor_tail> �� <mult_op> <factor> <factor_tail> | ��
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

// Ʈ���� ���Ͽ� ���� ����ϴ� �Լ�
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