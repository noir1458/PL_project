#include "LexicalAnalyzer.h"
#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;

int charClass = 0;
char lexeme[100];
char nextChar = ' ';
int lexLen = 0;
int token = 0;
int nextToken = 0;
FILE* in_fp = NULL;

// lookup - �����ڿ� ��ȣ�� �����Ͽ� �� ��ū�� ��ȯ�ϴ� �Լ�
int lookup(char ch) {
    switch (ch) {
    case '(':
        addChar();
        nextToken = LEFT_PAREN;
        break;
    case ')':
        addChar();
        nextToken = RIGHT_PAREN;
        break;
    case '+':
        addChar();
        nextToken = ADD_OP;
        break;
    case '-':
        addChar();
        nextToken = SUB_OP;
        break;
    case '*':
        addChar();
        nextToken = MULT_OP;
        break;
    case '/':
        addChar();
        nextToken = DIV_OP;
        break;
    case ':':  // ':' ���ڸ� ���� ó��
        addChar();
        getChar();  // ���� ���ڰ� '='���� Ȯ��
        if (nextChar == '=') {
            addChar();
            nextToken = ASSIGN_OP;  // := ���� ������ ó��
        }
        break;
    case ';':
        addChar();
        nextToken = SEMI_COLON;
        break;
    default:
        addChar();
        nextToken = EOF;
        break;
    }
    return nextToken;
}

// addchar - nextChar�� lexeme�� �߰��ϴ� �Լ�
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
    else {
        cout << "Error - lexeme is too long" << endl;
    }
}

// getChar - �Է����κ��� ���� ���ڸ� �����ͼ� �� ���� ������ �����ϴ� �Լ�
void getChar() {
    if ((nextChar = getc(in_fp)) != EOF) {
        if (isalpha(nextChar)) {
            charClass = LETTER;
        }
        else if (isdigit(nextChar)) {
            charClass = DIGIT;
        }
        else {
            charClass = UNKNOWN;
        }
    }
    else {
        charClass = EOF;
    }
}

// getNonBlank - �� ���� ���ڸ� ��ȯ�� ������ getChar�� ȣ���ϴ� �Լ�
void getNonBlank() {
    while ((isspace(nextChar) || nextChar <= 32) && nextChar != EOF) {  // EOF�� üũ�Ͽ� ���� ���� ����
        getChar();  // ������ �Ѿ �� getChar ȣ��
    }
}

// lexical - ���� �м���
int lexical() {
    lexLen = 0;
    getNonBlank();  // ���� ó��
    switch (charClass) {
        case LETTER:  // �ĺ��� �Ľ�
            addChar();
            getChar();
            while (charClass == LETTER || charClass == DIGIT) {
                addChar();
                getChar();
            }
            nextToken = IDENT;
            break;

        case DIGIT:  // ���� ���ͷ� �Ľ�
            addChar();
            getChar();
            while (charClass == DIGIT) {
                addChar();
                getChar();
            }
            nextToken = INT_LIT;
            break;

        case UNKNOWN:  // �����ڿ� ��ȣ �Ľ�
            lookup(nextChar);
            getChar();
            break;

        case EOF:
            nextToken = EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = '\0';
            break;
    }

    return nextToken;
}