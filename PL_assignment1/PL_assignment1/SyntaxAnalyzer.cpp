#define _CRT_SECURE_NO_WARNINGS

#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

// ���� ������ ID, CONST, OP ���� ����
int idCount = 0;
int constCount = 0;
int opCount = 0;

// ���� ������ statement ���� �߻� ����
bool statementError;
bool statementWarning;

// �� ī���� �ʱ�ȭ
void resetCounts() {
    idCount = 0;
    constCount = 0;
    opCount = 0;
}

// �ɺ� ���̺� ��� �Լ�
void printSymbolTable() {
    cout << "Result ==> ";
    for (int i = 0; i < symTableSize; i++) {
        cout << symTable[i].name << ": ";
        if (symTable[i].value == 99) {
            cout << "Unknown";  // UNKNOWN �� ���
        }
        else {
            cout << symTable[i].value;
        }
        if (i < symTableSize - 1) {
            cout << "; ";
        }
    }
    cout << ";" << endl;
}

// <program> �� <statements>
void program() {
    statements();
    printSymbolTable();  // �ɺ� ���̺� ���
}

// <statements> �� <statement> { ; <statement> }
void statements() {
    statement();
    while (nextToken == SEMI_COLON) {
        lexical();  // ';' ó��
        statement();  // ���� <statement> ó��
    }

}

// <statement> �� <ident> <assignment_op> <expression>
void statement() {
    resetCounts();
    statementError = false;
    statementWarning = false;

    if (nextToken == IDENT) {
        idCount++;  // �ĺ��� ���� ����
        char variable[100];
        strcpy(variable, lexeme);  // �ĺ��� �̸� ����
        lexical();  // �ĺ��� ó�� �� ���� ��ū �б�

        if (nextToken == ASSIGN_OP) {
            lexical();  // ���� ������ ó��
            int exprValue = expression();  // ǥ���� �� �� �� ��ȯ

            // �ɺ� ���̺� �ĺ��� �� ������Ʈ
            bool found = false;
            for (int i = 0; i < symTableSize; i++) {
                if (strcmp(symTable[i].name, variable) == 0) {
                    found = true;
                    symTable[i].value = exprValue;  // ���� �� ����
                    symTable[i].isDefined = !statementError;
                    break;
                }
            }
            if (!found) {
                strcpy(symTable[symTableSize].name, variable);  // �� ���� �߰�
                symTable[symTableSize].value = exprValue;
                symTable[symTableSize].isDefined = !statementError;
                symTableSize++;
            }

            // ���� ��� �� ID, CONST, OP, (OK) ���
            cout << "ID: " << idCount << "; CONST: " << constCount << "; OP: " << opCount << ";" << endl;

            // (OK) �Ǵ� (Error) ���
            if (!statementError && !statementWarning) {
                cout << "(OK)" << endl;
            } 
            
            if (statementWarning) {
                cout << "(Warning) \"�ߺ� ������ ����\"" << endl;
            }
            if (statementError) {
                cout << "(Error) \"���ǵ��� ���� ����(" << variable << ")�� ������\"" << endl;
            }
        }
    }
}

// <expression> �� <term><term_tail>
int expression() {
    int result = term();  // ���� term ó��
    result = term_tail(result);  // term_tail���� �߰����� ������ ó��
    return result;
}

// <term> �� <factor><factor_tail>
int term() {
    int result = factor();  // factor ó��
    result = factor_tail(result);  // factor_tail���� �߰����� ������ ó��
    return result;
}

// <term_tail> �� <add_op><term><term_tail> | ��
int term_tail(int leftValue) {
    int result = leftValue;

    // + �Ǵ� - �����ڰ� �ִ� ��쿡�� ó��
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        opCount++;  // ������ ���� ����
        int operatorToken = nextToken;
        lexical();  // ������ ó�� �� ���� ��ū���� �̵�

        // �ߺ��� �����ڰ� �������� ������ Ȯ��
        if (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            statementWarning = true;  // �ߺ� ������ ��� ����
            lexical();  // �ߺ� ������ �ǳʶ�
        }

        int rightValue = term();  // ���� term ó��
        if (result != UNKNOWN) {
            if (operatorToken == ADD_OP) {
                result += rightValue;
            }
            else if (operatorToken == SUB_OP) {
                result -= rightValue;
            }
        }
        
        result = term_tail(result);  // ��������� ���� ���� ó��
    }

    return result;
}

// <factor> �� <left_paren><expression><right_paren> | <ident> | <const>
int factor() {
    int result = 0;

    if (nextToken == LEFT_PAREN) {
        lexical();  // '(' ó��
        result = expression();  // ��ȣ ���� ǥ���� ó��
        if (nextToken == RIGHT_PAREN) {
            lexical();  // ')' ó��
        }
    }
    // �ĺ��� ó�� (������ ���ǵǾ����� Ȯ��)
    else if (nextToken == IDENT) {
        idCount++;  // �ĺ��� ���� ����
        bool found = false;
        for (int i = 0; i < symTableSize; i++) {
            if (strcmp(symTable[i].name, lexeme) == 0) {
                found = true;
                if (symTable[i].isDefined) {
                    result = symTable[i].value;  // ���ǵ� ������ �� ���
                }
                else {
                    result = UNKNOWN;
                }
                break;
            }
        }
        if (!found) {
            // ������ �ɺ� ���̺� ������ �߰�
            strcpy(symTable[symTableSize].name, lexeme);  // ���� �߰�
            symTable[symTableSize].value = UNKNOWN;
            symTable[symTableSize].isDefined = false;  // ���ǵ��� �������� ����
            symTableSize++;
            result = UNKNOWN;
            statementError = true;
        }
        lexical();  // �ĺ��� ó�� �� ���� ��ū���� �̵�
    }
    // ��� ó��
    else if (nextToken == INT_LIT) {
        result = atoi(lexeme);  // ��� ���� ������ ��ȯ
        constCount++;  // ��� ���� ����
        lexical();  // ��� ó�� �� ���� ��ū���� �̵�
    }

    return result;
}

// <factor_tail> �� <mult_op><factor><factor_tail> | ��
int factor_tail(int leftValue) {
    int result = leftValue;

    // * �Ǵ� / �����ڰ� �ִ� ��쿡�� ó��
    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        opCount++;

        int operatorToken = nextToken;
        lexical();  // ������ ó�� �� ���� ��ū���� �̵�

        if (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            statementWarning = true;  // �ߺ� ������ ��� ����
            lexical();  // �ߺ� ������ �ǳʶ�
        }

        int rightValue = factor();  // ���� factor �� ���

        if (result != UNKNOWN && rightValue != UNKNOWN) {
            // �����ڿ� ���� ���� ����
            if (operatorToken == MULT_OP) {
                result *= rightValue;
            }
            else if (operatorToken == DIV_OP) {
                if (rightValue == 0) {
                    result = 0;
                }
                else {
                    result /= rightValue;
                }
            }
            result = factor_tail(result);  // ��������� ���� ���� ó��
        }
    }

    return result;
}