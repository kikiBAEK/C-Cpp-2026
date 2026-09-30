#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
using namespace std;

struct Pos {
    int x, y;
    Pos(int x = 0, int y = 0) : x(x), y(y) {}
};

class Sokoban {
private:
    vector<string> map;
    vector<string> originalMap;
    Pos player;
    int steps;
    int currentLevel;
    int maxLevel;
    vector<int> bestScores;

    const char WALL = '#';
    const char FLOOR = ' ';
    const char PLAYER = '@';
    const char BOX = '$';
    const char TARGET = '.';
    const char BOX_ON_TARGET = '*';
    const char PLAYER_ON_TARGET = '+';

    // ==================== 三个关卡数据（全部内置） ====================
    vector<vector<string>> levels = {
        // Level 1
        {
            "########",
            "#      #",
            "#  $   #",
            "# @.$  #",
            "#  .   #",
            "########"
        },
        // Level 2
        {
            "#########",
            "#       #",
            "#  $ $  #",
            "#  .@.  #",
            "#  $ $  #",
            "#  . .  #",
            "#########"
        },
        // Level 3
        {
            "##########",
            "#        #",
            "#  $  $  #",
            "#  .@.   #",
            "#  $  $  #",
            "#  .  .  #",
            "#        #",
            "##########"
        }
    };

public:
    Sokoban() : steps(0), currentLevel(1), maxLevel(3) {
        loadScores();
    }

    bool loadLevel(int level) {
        if (level < 1 || level > maxLevel) {
            return false;
        }

        map = levels[level - 1];
        originalMap = levels[level - 1];

        player = Pos(-1, -1);
        for (int y = 0; y < (int)map.size(); y++) {
            for (int x = 0; x < (int)map[y].size(); x++) {
                char c = map[y][x];
                if (c == PLAYER || c == PLAYER_ON_TARGET) {
                    player = Pos(x, y);
                }
            }
        }

        if (player.x == -1) {
            cout << "[错误] 关卡中没有找到玩家 (@)！" << endl;
            return false;
        }

        steps = 0;
        currentLevel = level;
        return true;
    }

    void draw() {
        system("cls");
        cout << "========== Sokoban ==========" << endl;
        cout << "Level: " << currentLevel << " / " << maxLevel
            << "    Steps: " << steps << endl;

        if (currentLevel <= (int)bestScores.size() && bestScores[currentLevel - 1] > 0) {
            cout << "Best: " << bestScores[currentLevel - 1] << " steps" << endl;
        }

        cout << "Arrow keys: Move | R: Restart | ESC: Exit" << endl;
        cout << "=============================" << endl << endl;

        for (size_t i = 0; i < map.size(); i++) {
            cout << map[i] << endl;
        }
        cout << endl;
        cout << "Legend: # Wall  @ Player  $ Box  . Target  * BoxOnTarget  + PlayerOnTarget" << endl;
    }

    bool isTarget(int x, int y) {
        if (y < 0 || y >= (int)originalMap.size() || x < 0 || x >= (int)originalMap[y].size())
            return false;
        char c = originalMap[y][x];
        return c == TARGET || c == BOX_ON_TARGET || c == PLAYER_ON_TARGET;
    }

    char getChar(int x, int y) {
        if (y < 0 || y >= (int)map.size() || x < 0 || x >= (int)map[y].size())
            return WALL;
        return map[y][x];
    }

    void setChar(int x, int y, char c) {
        if (y >= 0 && y < (int)map.size() && x >= 0 && x < (int)map[y].size()) {
            map[y][x] = c;
        }
    }

    bool tryMove(int dx, int dy) {
        int nx = player.x + dx;
        int ny = player.y + dy;
        char next = getChar(nx, ny);

        if (next == WALL) return false;

        // 推箱子
        if (next == BOX || next == BOX_ON_TARGET) {
            int bx = nx + dx;
            int by = ny + dy;
            char behind = getChar(bx, by);

            if (behind == WALL || behind == BOX || behind == BOX_ON_TARGET)
                return false;

            bool boxToTarget = isTarget(bx, by);
            setChar(bx, by, boxToTarget ? BOX_ON_TARGET : BOX);

            bool wasTarget = isTarget(nx, ny);
            setChar(nx, ny, wasTarget ? TARGET : FLOOR);
        }

        // 移动玩家
        bool playerToTarget = isTarget(nx, ny);
        setChar(nx, ny, playerToTarget ? PLAYER_ON_TARGET : PLAYER);

        bool wasTarget = isTarget(player.x, player.y);
        setChar(player.x, player.y, wasTarget ? TARGET : FLOOR);

        player.x = nx;
        player.y = ny;
        steps++;
        return true;
    }

    bool isWin() {
        for (size_t i = 0; i < map.size(); i++) {
            for (size_t j = 0; j < map[i].size(); j++) {
                if (map[i][j] == BOX) return false;
            }
        }
        return true;
    }

    void restart() {
        loadLevel(currentLevel);
    }

    void loadScores() {
        bestScores.assign(maxLevel, 0);
        ifstream file("scores.txt");
        if (file.is_open()) {
            int level, score;
            while (file >> level >> score) {
                if (level >= 1 && level <= maxLevel) {
                    bestScores[level - 1] = score;
                }
            }
            file.close();
        }
    }

    void saveScore() {
        if (currentLevel > (int)bestScores.size()) {
            bestScores.resize(currentLevel, 0);
        }
        if (bestScores[currentLevel - 1] == 0 || steps < bestScores[currentLevel - 1]) {
            bestScores[currentLevel - 1] = steps;
        }

        ofstream file("scores.txt");
        if (file.is_open()) {
            for (int i = 0; i < (int)bestScores.size(); i++) {
                if (bestScores[i] > 0) {
                    file << (i + 1) << " " << bestScores[i] << endl;
                }
            }
            file.close();
        }
    }

    void run() {
        cout << "Sokoban Game" << endl;
        cout << "Press any key to start..." << endl;
        _getch();

        while (currentLevel <= maxLevel) {
            if (!loadLevel(currentLevel)) {
                cout << "Load level failed!" << endl;
                cout << "Press any key to exit..." << endl;
                _getch();
                return;
            }

            bool levelCleared = false;
            while (!levelCleared) {
                draw();

                if (isWin()) {
                    cout << "\n*** Level Clear! Steps: " << steps << " ***" << endl;
                    saveScore();
                    cout << "Press any key for next level..." << endl;
                    _getch();
                    levelCleared = true;
                    currentLevel++;
                    break;
                }

                int key = _getch();
                if (key == 224) {               // 方向键
                    key = _getch();
                    switch (key) {
                    case 72: tryMove(0, -1); break;  // 上
                    case 80: tryMove(0, 1); break;  // 下
                    case 75: tryMove(-1, 0); break;  // 左
                    case 77: tryMove(1, 0); break;  // 右
                    }
                }
                else if (key == 'r' || key == 'R') {
                    restart();
                }
                else if (key == 27) {           // ESC
                    cout << "Game Exit." << endl;
                    return;
                }
            }
        }

        // 全部通关
        system("cls");
        cout << "========== All Levels Cleared! ==========" << endl;
        cout << "Best Scores:" << endl;
        for (int i = 0; i < (int)bestScores.size(); i++) {
            if (bestScores[i] > 0) {
                cout << "Level " << (i + 1) << ": " << bestScores[i] << " steps" << endl;
            }
        }
        cout << "=========================================" << endl;
        cout << "Press any key to exit..." << endl;
        _getch();
    }
};

int main() {
    Sokoban game;
    game.run();
    return 0;
}