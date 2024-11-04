#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// ���� �м��⿡�� ���Ǵ� �Լ� �� ������ ���� ������ ����
#include "LexicalAnalyzer.h"
#include <cstring>

// �ɺ� ���̺��� ���� ����ü
struct SymbolTable {
    char name[100];
    int value;
    bool isDefined;
};

// ���� ���� ���� (extern ���)
extern SymbolTable symTable[100];   // ���� ����ü �迭 ����
extern int symTableSize;            // �ɺ� ���̺� ũ�� ����

// �ɺ� ���̺� ���� �Լ� ����
SymbolTable* findOrCreateSymbol(const char* name);
void updateSymbolTable(const char* variable, int value);

// �Ľ� Ʈ�� ��� ����ü
struct ParseTreeNode {
    int token;                   // ��ū ����
    char* value;                 // ����� �� (��: �ĺ��� �̸� �Ǵ� ���ͷ� ��)
    ParseTreeNode* child;        // ���� ����� ù ��° �ڽ� (���� �Ʒ�)
    ParseTreeNode* sibling;      // ���� ����� ���� ��� (�ٷ� ������)

    // ������
    ParseTreeNode(int token, const char* valueText)
        : token(token), child(nullptr), sibling(nullptr), value(nullptr) {
        value = new char[strlen(valueText) + 1];
        strcpy(value, valueText);
    }

    // �Ҹ��� (�޸� ����)
    ~ParseTreeNode() {
        delete[] value;
    }
};

// �Ľ� Ʈ�� ���� �Լ� ����
ParseTreeNode* createNode(int token, const char* value);
void addChild(ParseTreeNode* parent, ParseTreeNode* child);
void printTree(ParseTreeNode* root, int level = 0);
void freeTree(ParseTreeNode* root); // �޸� ������ ���� �Լ�

// ���� �м� �Լ� ����
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