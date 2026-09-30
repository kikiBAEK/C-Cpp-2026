#include <iostream>
#include <conio.h>   // Windows下用_getch()和方向键
#include <windows.h> // 用于清屏和光标控制
using namespace std;

const int HEIGHT = 10;
const int WIDTH = 15;

// 迷宫地图（1 = 墙，0 = 空地，2 = 出口）
int maze[HEIGHT][WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,1,1,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,2,1},  // 2 是出口
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int playerX = 1;  // 玩家起始位置
int playerY = 1;

void clearScreen() {
    system("cls");  // Windows 清屏
}

void drawMaze() {
    clearScreen();
    cout << "=== 迷宫小游戏 ===" << endl;
    cout << "使用方向键移动，到达 E 即可获胜！" << endl << endl;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == playerX && y == playerY) {
                cout << "@ ";          // 玩家
            }
            else if (maze[y][x] == 1) {
                cout << "# ";          // 墙
            }
            else if (maze[y][x] == 2) {
                cout << "E ";          // 出口
            }
            else {
                cout << ". ";          // 空地
            }
        }
        cout << endl;
    }
    cout << endl << "当前位置: (" << playerX << ", " << playerY << ")" << endl;
}

bool canMove(int newX, int newY) {
    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT)
        return false;
    return maze[newY][newX] != 1;  // 不是墙就能走
}

void movePlayer(int dx, int dy) {
    int newX = playerX + dx;
    int newY = playerY + dy;

    if (canMove(newX, newY)) {
        playerX = newX;
        playerY = newY;
    }
}

int main() {
    // 设置控制台编码（可选，避免中文乱码）
    SetConsoleOutputCP(65001);

    cout << "按任意键开始游戏..." << endl;
    _getch();

    while (true) {
        drawMaze();

        // 检查是否到达出口
        if (maze[playerY][playerX] == 2) {
            cout << "\n★★★ 恭喜你！成功走出迷宫！★★★" << endl;
            cout << "按任意键退出..." << endl;
            _getch();
            break;
        }

        // 读取方向键
        int key = _getch();
        if (key == 224) {          // 方向键的前缀
            key = _getch();
            switch (key) {
            case 72: movePlayer(0, -1); break;  // 上
            case 80: movePlayer(0, 1); break;  // 下
            case 75: movePlayer(-1, 0); break;  // 左
            case 77: movePlayer(1, 0); break;  // 右
            }
        }
        else if (key == 27) {      // ESC 退出
            cout << "游戏已退出。" << endl;
            break;
        }
    }

    return 0;
}