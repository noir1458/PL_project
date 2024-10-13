#define _CRT_SECURE_NO_WARNINGS

#define _CRT_SECURE_NO_WARNINGS

#include "SyntaxAnalyzer.h"
#include <iostream>
#include <cstring>

using namespace std;

// 전역 변수로 ID, CONST, OP 개수 추적
int idCount = 0;
int constCount = 0;
int opCount = 0;

// 전역 변수로 statement 오류 발생 추적
bool statementError;
bool statementWarning;

// 각 카운터 초기화
void resetCounts() {
    idCount = 0;
    constCount = 0;
    opCount = 0;
}

// 심볼 테이블 출력 함수
void printSymbolTable() {
    cout << "Result ==> ";
    for (int i = 0; i < symTableSize; i++) {
        cout << symTable[i].name << ": ";
        if (symTable[i].value == 99) {
            cout << "Unknown";  // UNKNOWN 값 출력
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

// <program> → <statements>
void program() {
    statements();
    printSymbolTable();  // 심볼 테이블 출력
}

// <statements> → <statement> { ; <statement> }
void statements() {
    statement();
    while (nextToken == SEMI_COLON) {
        lexical();  // ';' 처리
        statement();  // 다음 <statement> 처리
    }

}

// <statement> → <ident> <assignment_op> <expression>
void statement() {
    resetCounts();
    statementError = false;
    statementWarning = false;

    if (nextToken == IDENT) {
        idCount++;  // 식별자 개수 증가
        char variable[100];
        strcpy(variable, lexeme);  // 식별자 이름 복사
        lexical();  // 식별자 처리 후 다음 토큰 읽기

        if (nextToken == ASSIGN_OP) {
            lexical();  // 대입 연산자 처리
            int exprValue = expression();  // 표현식 평가 후 값 반환

            // 심볼 테이블에 식별자 값 업데이트
            bool found = false;
            for (int i = 0; i < symTableSize; i++) {
                if (strcmp(symTable[i].name, variable) == 0) {
                    found = true;
                    symTable[i].value = exprValue;  // 계산된 값 저장
                    symTable[i].isDefined = !statementError;
                    break;
                }
            }
            if (!found) {
                strcpy(symTable[symTableSize].name, variable);  // 새 변수 추가
                symTable[symTableSize].value = exprValue;
                symTable[symTableSize].isDefined = !statementError;
                symTableSize++;
            }

            // 구문 출력 및 ID, CONST, OP, (OK) 출력
            cout << "ID: " << idCount << "; CONST: " << constCount << "; OP: " << opCount << ";" << endl;

            // (OK) 또는 (Error) 출력
            if (!statementError && !statementWarning) {
                cout << "(OK)" << endl;
            } 
            
            if (statementWarning) {
                cout << "(Warning) \"중복 연산자 제거\"" << endl;
            }
            if (statementError) {
                cout << "(Error) \"정의되지 않은 변수(" << variable << ")가 참조됨\"" << endl;
            }
        }
    }
}

// <expression> → <term><term_tail>
int expression() {
    int result = term();  // 먼저 term 처리
    result = term_tail(result);  // term_tail에서 추가적인 연산자 처리
    return result;
}

// <term> → <factor><factor_tail>
int term() {
    int result = factor();  // factor 처리
    result = factor_tail(result);  // factor_tail에서 추가적인 연산자 처리
    return result;
}

// <term_tail> → <add_op><term><term_tail> | ε
int term_tail(int leftValue) {
    int result = leftValue;

    // + 또는 - 연산자가 있는 경우에만 처리
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
        opCount++;  // 연산자 개수 증가
        int operatorToken = nextToken;
        lexical();  // 연산자 처리 후 다음 토큰으로 이동

        // 중복된 연산자가 연속으로 오는지 확인
        if (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            statementWarning = true;  // 중복 연산자 경고 설정
            lexical();  // 중복 연산자 건너뜀
        }

        int rightValue = term();  // 다음 term 처리
        if (result != UNKNOWN) {
            if (operatorToken == ADD_OP) {
                result += rightValue;
            }
            else if (operatorToken == SUB_OP) {
                result -= rightValue;
            }
        }
        
        result = term_tail(result);  // 재귀적으로 다음 연산 처리
    }

    return result;
}

// <factor> → <left_paren><expression><right_paren> | <ident> | <const>
int factor() {
    int result = 0;

    if (nextToken == LEFT_PAREN) {
        lexical();  // '(' 처리
        result = expression();  // 괄호 안의 표현식 처리
        if (nextToken == RIGHT_PAREN) {
            lexical();  // ')' 처리
        }
    }
    // 식별자 처리 (변수가 정의되었는지 확인)
    else if (nextToken == IDENT) {
        idCount++;  // 식별자 개수 증가
        bool found = false;
        for (int i = 0; i < symTableSize; i++) {
            if (strcmp(symTable[i].name, lexeme) == 0) {
                found = true;
                if (symTable[i].isDefined) {
                    result = symTable[i].value;  // 정의된 변수의 값 사용
                }
                else {
                    result = UNKNOWN;
                }
                break;
            }
        }
        if (!found) {
            // 변수가 심볼 테이블에 없으면 추가
            strcpy(symTable[symTableSize].name, lexeme);  // 변수 추가
            symTable[symTableSize].value = UNKNOWN;
            symTable[symTableSize].isDefined = false;  // 정의되지 않음으로 설정
            symTableSize++;
            result = UNKNOWN;
            statementError = true;
        }
        lexical();  // 식별자 처리 후 다음 토큰으로 이동
    }
    // 상수 처리
    else if (nextToken == INT_LIT) {
        result = atoi(lexeme);  // 상수 값을 정수로 변환
        constCount++;  // 상수 개수 증가
        lexical();  // 상수 처리 후 다음 토큰으로 이동
    }

    return result;
}

// <factor_tail> → <mult_op><factor><factor_tail> | ε
int factor_tail(int leftValue) {
    int result = leftValue;

    // * 또는 / 연산자가 있는 경우에만 처리
    if (nextToken == MULT_OP || nextToken == DIV_OP) {
        opCount++;

        int operatorToken = nextToken;
        lexical();  // 연산자 처리 후 다음 토큰으로 이동

        if (nextToken == ADD_OP || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP) {
            statementWarning = true;  // 중복 연산자 경고 설정
            lexical();  // 중복 연산자 건너뜀
        }

        int rightValue = factor();  // 다음 factor 값 계산

        if (result != UNKNOWN && rightValue != UNKNOWN) {
            // 연산자에 따른 연산 수행
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
            result = factor_tail(result);  // 재귀적으로 다음 연산 처리
        }
    }

    return result;
}