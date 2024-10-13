#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include<fstream>
#include <cstring>
#include <cctype>
#include <cstdio>

using namespace std;

/*global declarations*/
/*Variables*/
int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
int nextToken;
FILE* in_fp;

// �Լ� �����
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(char ch);

// ���� ������
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

//��ū �ڵ��
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26

//*********** 
int main(int argc, char* argv[]) {
	// �Է� ���� �̸�
	const char* inputFileName;

	if (argc > 1) {
		inputFileName = argv[1];
	}
	else {
		cout << "����� ���� ���� �ʿ�" << endl;
		return 1;
	}

	cout << "�Է� ���� : " << inputFileName << endl;

	//�Է� ������ ������ ���� �� ������ ó��
	if ((in_fp = fopen(inputFileName, "r")) == NULL)
		cout << "ERROR - cannot open" << inputFileName << endl;
	else {
		getChar();
		do {
			lex();
		} while (nextToken != EOF);
	}
	return 0;
}

//lookup - �����ڿ� ��ȣ�� �����Ͽ� �� ��ū�� ��ȯ�ϴ� �Լ�
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
	case '=':
		addChar();
		nextToken = ASSIGN_OP; // = ���� ó��
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
	else
	{
		cout << "Error - lexeme is too long" <<endl;
	}
}

//getChar - �Է����κ��� ���� ���ڸ� �����ͼ� �� ���� ������ �����ϴ� �Լ�
void getChar() {
	if ((nextChar = getc(in_fp)) != EOF) {
		if (isalpha(nextChar))
			charClass = LETTER;
		else if (isdigit(nextChar))
			charClass = DIGIT;
		else
			charClass = UNKNOWN;
	}
	else {
		charClass = EOF;
	}
}

// getNonBlank - �� ���� ���ڸ� ��ȯ�Ҷ����� getChar�� ȣ���ϴ� �Լ�
void getNonBlank() {
	while (isspace(nextChar))
		getChar();
}


//lex - ������� ���� �ܼ� ���� �м���
int lex() {
	lexLen = 0;
	getNonBlank();
	switch (charClass) {
		// �ĺ��ڸ� �Ľ��Ѵ�
	case LETTER:
		addChar();
		getChar();
		while (charClass == LETTER || charClass == DIGIT) {
			addChar();
			getChar();
		}
		nextToken = IDENT;
		break;

		// ���� ���ͷ��� �Ľ��Ѵ�
	case DIGIT:
		addChar();
		getChar();
		while (charClass == DIGIT) {
			addChar();
			getChar();
		}
		nextToken = INT_LIT;
		break;

		// ��ȣ�� ������
	case UNKNOWN:
		lookup(nextChar);
		getChar();
		break;

		//EOF
	case EOF:
		nextToken = EOF;
		lexeme[0] = 'E';
		lexeme[1] = 'O';
		lexeme[2] = 'F';
		lexeme[3] = '\0';
		break;
	} //switch�� ��

	cout << "Next token is : " << nextToken << ", Next lexeme is : " << lexeme << endl;
	return nextToken;
}