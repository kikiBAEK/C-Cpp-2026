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

int main() {
    // 用 mode 命令固定窗口大小（更可靠）
    system("mode con cols=80 lines=25");

    string text = "Hello";
    int pos = 0;
    int direction = 1;
    int speed = 50;
    int width = 80;
    int textWidth = text.length();   // Hello 是 5

    hideCursor();
    system("cls");

    while (true) {
        // 清除当前行
        gotoxy(0, 12);
        cout << string(width, ' ');

        // 显示文字
        gotoxy(pos, 12);
        cout << text << flush;

        // 更新位置
        pos += direction;

        // 左边界
        if (pos <= 0) {
            pos = 0;
            direction = 1;
        }
        // 右边界
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