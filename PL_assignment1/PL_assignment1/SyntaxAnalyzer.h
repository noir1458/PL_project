#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

// ���� �м��⿡�� ���Ǵ� �Լ� �� ������ ���� ������ ����
#include "LexicalAnalyzer.h"
#include <cstring>
#include <iostream>

using namespace std;

#define OK 1
#define WARNING 0
#define ERROR -1

// ���� ������ ID, CONST, OP ���� ����
extern int idCount;
extern int constCount;
extern int opCount;

// �ɺ� ����ü
struct Symbol {
    string name;
    int value;
    bool isDefined;
};

// �ɺ� ���̺� ����ü
struct SymbolTable {
    Symbol symbols[100];
    int size;

    // ������ (ũ�⸦ 0���� �ʱ�ȭ)
    SymbolTable() : size(0) {
        for (int i = 0; i < 100; i++) {
            symbols[i] = Symbol();  // �� Symbol�� �⺻������ �ʱ�ȭ
        }
    }

    // �ɺ� ���� �Լ�
    Symbol* createSymbol(const string& name) {
        symbols[size].name = name;
        symbols[size].value = 0;          // �⺻��
        symbols[size].isDefined = false;
        return &symbols[size++];          // �� �ɺ� ��ȯ �� ũ�� ����
    }

    // �ɺ� �˻� �Լ�
    Symbol* findSymbol(const string& name) {
        for (int i = 0; i < size; i++) {
            if (symbols[i].name == name) {
                return &symbols[i];
            }
        }
        return nullptr;
    }

    // �ɺ� ������Ʈ �Լ�
    void updateSymbol(const string& variable, int value) {
        Symbol* symbol = findSymbol(variable);
        if (symbol) {
            symbol->value = value;
            symbol->isDefined = true;
        }
    }

    // �ɺ� ���̺� ��� �Լ�
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

// �Ľ� Ʈ�� ��� ����ü
struct ParseTreeNode {
    int token;                  // ��ū ����
    int value;                  // ����� ��
    bool isDefined;             // ����� ���� �ʱ�ȭ �Ǿ����� Ȯ��
    int status;                 // ����� ���� (OK, WARNING, ERROR ��)
    string message;              // ��� �� ���� �޽��� �迭
    ParseTreeNode* child;       // ���� ����� ù ��° �ڽ� (���� �Ʒ�)
    ParseTreeNode* sibling;     // ���� ����� ���� ��� (�ٷ� ������)

    // ������
    ParseTreeNode(int token)
        : token(token), value(0), isDefined(false), child(nullptr), sibling(nullptr), status(OK), message("(OK)") {}

    // �ڽ� ��� �߰� �Լ�
    void addChild(ParseTreeNode* childNode) {
        if (!child) {
            child = childNode;  // �ڽ��� ������ �ٷ� �߰�
        }
        else {
            ParseTreeNode* current = child;
            while (current->sibling) {
                current = current->sibling;
            }
            current->sibling = childNode;  // ������ �ڽ��� sibling���� �߰�
        }

        setStatus(childNode->status, childNode->message); // �̹��� �߰��� �ڽ� ����� ���� �켱������ �� ������ ����
    }

    // �� ���� �Լ�
    void setValue(int newValue) {
        value = newValue;
        isDefined = true;  // ���� �����Ǿ����Ƿ� ���ǵ� ������ ǥ��
    }

    // ���� �� �޽��� ���� �Լ�
    void setStatus(int newStatus, const string& newMessage) {
        if (status > newStatus) {
            status = newStatus;
            message = newMessage;
        }
    }

    // �޽��� ���
    void printMessage() const {
        if (!message.empty()) {
            cout << message << endl;
        }
    }
};

// �� ī���� �ʱ�ȭ
void resetCounts();

// �Ľ� Ʈ�� ���� �Լ� ����
void freeTree(ParseTreeNode* root); // �޸� ������ ���� �Լ�

// ���� �м� �Լ� ����
ParseTreeNode* program();
ParseTreeNode* statements();
ParseTreeNode* statement();
ParseTreeNode* expression();
ParseTreeNode* term();
ParseTreeNode* factor();
ParseTreeNode* term_tail(int leftValue);
ParseTreeNode* factor_tail(int leftValue);

#endif // SYNTAX_ANALYZER_H