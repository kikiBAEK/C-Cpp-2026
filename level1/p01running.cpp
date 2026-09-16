#include <iostream>
#include <string>
#include <windows.h>
#include <conio.h>

using namespace std;

void gotoxy(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursor_info = { 1, 0 };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

// 固定控制台窗口大小
void setConsoleSize(int width, int height) {
    // 设置缓冲区大小
    COORD bufferSize = { (SHORT)width, (SHORT)height };
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), bufferSize);

    // 设置窗口大小
    SMALL_RECT windowSize = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
}

int main() {
    // 设置控制台支持中文
    SetConsoleOutputCP(65001);
    system("chcp 65001 > nul");

    // 固定窗口为 80列 × 25行
    setConsoleSize(160, 25);

    string text = "Hello";   // 可以改成 "A" 或任意文字
    int pos = 0;
    int direction = 1;
    int speed = 60;               // 速度，越小越快
    int width = 100;               // 固定宽度
    int textWidth = 10;           // "奔跑的字母" 大约占10列

    hideCursor();
    system("cls");

    while (true) {
        // 清除当前行
        gotoxy(0, 10);
        cout << string(width, ' ');

        // 显示文字
        gotoxy(pos, 10);
        cout << text << flush;

        // 更新位置
        pos += direction;

        // 左边界反弹
        if (pos <= 0) {
            pos = 0;
            direction = 1;
        }
        // 右边界反弹
        else if (pos + textWidth >= width) {
            pos = width - textWidth;
            direction = -1;
        }

        Sleep(speed);

        // 按任意键退出
        if (_kbhit()) {
            break;
        }
    }

    return 0;
}