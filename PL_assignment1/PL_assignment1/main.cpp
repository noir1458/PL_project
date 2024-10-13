#define _CRT_SECURE_NO_WARNINGS

#include "LexicalAnalyzer.h"  // 어휘 분석기 함수 및 전역 변수 포함
#include "SyntaxAnalyzer.h"   // 구문 분석기 함수 및 전역 변수 포함
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    // 입력 파일이 제공되었는지 확인
    if (argc > 1) {
        in_fp = fopen(argv[1], "r");  // 입력 파일 열기
        if (in_fp == NULL) {
            cout << "ERROR - cannot open " << argv[1] << endl;
            return 1;
        }
        else {
            getChar();  // 어휘 분석기의 첫 번째 문자를 읽음
            lexical();      // 첫 번째 토큰을 가져옴
            program();  // 구문 분석을 시작 (program() 함수 호출)
        }
        fclose(in_fp);  // 입력 파일 닫기
    }
    else {
        cout << "파일 이름이 필요합니다." << endl;
        return 1;
    }

    return 0;
}
