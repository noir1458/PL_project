#define _CRT_SECURE_NO_WARNINGS

#include "LexicalAnalyzer.h"  // ���� �м��� �Լ� �� ���� ���� ����
#include "SyntaxAnalyzer.h"   // ���� �м��� �Լ� �� ���� ���� ����
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    // �Է� ������ �����Ǿ����� Ȯ��
    if (argc > 1) {
        in_fp = fopen(argv[1], "r");  // �Է� ���� ����
        if (in_fp == NULL) {
            cout << "ERROR - cannot open " << argv[1] << endl;
            return 1;
        }
        else {
            getChar();  // ���� �м����� ù ��° ���ڸ� ����
            lexical();      // ù ��° ��ū�� ������
            program();  // ���� �м��� ���� (program() �Լ� ȣ��)
        }
        fclose(in_fp);  // �Է� ���� �ݱ�
    }
    else {
        cout << "���� �̸��� �ʿ��մϴ�." << endl;
        return 1;
    }

    return 0;
}
