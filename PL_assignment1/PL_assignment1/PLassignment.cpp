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

// 함수 선언들
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(char ch);

// 문자 유형들
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

//토큰 코드들
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
	// 입력 파일 이름
	const char* inputFileName;

	if (argc > 1) {
		inputFileName = argv[1];
	}
	else {
		cout << "명령줄 인자 제공 필요" << endl;
		return 1;
	}

	cout << "입력 파일 : " << inputFileName << endl;

	//입력 데이터 내용을 열고 그 내용을 처리
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

//lookup - 연산자와 괄호를 조사하여 그 토큰을 반환하는 함수
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
		nextToken = ASSIGN_OP; // = 문자 처리
		break;
	default:
		addChar();
		nextToken = EOF;
		break;
	}
	return nextToken;
}

// addchar - nextChar를 lexeme에 추가하는 함수
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

//getChar - 입력으로부터 다음 문자를 가져와서 그 문자 유형을 결정하는 함수
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

// getNonBlank - 비 공백 문자를 반환할때까지 getChar를 호출하는 함수
void getNonBlank() {
	while (isspace(nextChar))
		getChar();
}


//lex - 산술식을 위한 단순 어휘 분석기
int lex() {
	lexLen = 0;
	getNonBlank();
	switch (charClass) {
		// 식별자를 파싱한다
	case LETTER:
		addChar();
		getChar();
		while (charClass == LETTER || charClass == DIGIT) {
			addChar();
			getChar();
		}
		nextToken = IDENT;
		break;

		// 정수 리터럴을 파싱한다
	case DIGIT:
		addChar();
		getChar();
		while (charClass == DIGIT) {
			addChar();
			getChar();
		}
		nextToken = INT_LIT;
		break;

		// 괄호와 연산자
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
	} //switch의 끝

	cout << "Next token is : " << nextToken << ", Next lexeme is : " << lexeme << endl;
	return nextToken;
}