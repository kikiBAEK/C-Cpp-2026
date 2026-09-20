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

int main() 
{
    //尺寸
    system("mode con cols=80 lines=25");

    string text = "Hello";
    int pos = 0;
    int direction = 1;
    int speed = 50;
    int width = 80;
    int textWidth = text.length();

    hideCursor();
    system("cls");
    //开动
    while (true) {
        gotoxy(0, 12);
        cout << string(width, ' ');

        gotoxy(pos, 12);
        cout << text << flush;

        pos += direction;

        if (pos <= 0) {
            pos = 0;
            direction = 1;
        }
        else if (pos + textWidth >= width) {
            pos = width - textWidth;
            direction = -1;
        }

        Sleep(speed);

        if (_kbhit()) {
            break;
        }
    }

    return 0;
}