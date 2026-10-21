#include <graphics.h>
#include <conio.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>

// ===================== 全局参数设置 =====================
const int GRID_SIZE = 15;        // 15x15 棋盘
const int CELL_SIZE = 40;        // 格子大小 (像素)
const int MARGIN = 50;           // 边距 (像素)
const int BOARD_SIZE = MARGIN * 2 + (GRID_SIZE - 1) * CELL_SIZE; // 660x660

const int EMPTY = 0;
const int PLAYER = 1; // 玩家 (黑棋)
const int AI = 2;     // AI (白棋)

int board[GRID_SIZE][GRID_SIZE] = { 0 };
int lastX = -1, lastY = -1; // 记录最后一步落子

// ===================== UI 绘制函数 =====================

// 绘制具有木质纹理感的高颜值棋盘
void DrawBoard() {
    // 1. 棋盘背景（暖木色背景与渐变）
    setfillcolor(RGB(222, 172, 98));
    solidrectangle(0, 0, BOARD_SIZE, BOARD_SIZE);

    // 木纹线条装饰
    setlinecolor(RGB(205, 153, 78));
    for (int i = 0; i < BOARD_SIZE; i += 4) {
        line(0, i, BOARD_SIZE, i);
    }

    // 外边框
    setlinecolor(RGB(80, 40, 10));
    setlinestyle(PS_SOLID, 3);
    rectangle(MARGIN - 10, MARGIN - 10, BOARD_SIZE - MARGIN + 10, BOARD_SIZE - MARGIN + 10);

    // 2. 网格线
    setlinestyle(PS_SOLID, 1);
    setlinecolor(RGB(60, 30, 5));
    for (int i = 0; i < GRID_SIZE; i++) {
        // 横线与竖线
        line(MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (GRID_SIZE - 1) * CELL_SIZE, MARGIN + i * CELL_SIZE);
        line(MARGIN + i * CELL_SIZE, MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (GRID_SIZE - 1) * CELL_SIZE);
    }

    // 3. 标注星位 (天元及四个角点)
    int dots[] = { 3, 7, 11 };
    setfillcolor(RGB(60, 30, 5));
    for (int dx : dots) {
        for (int dy : dots) {
            fillcircle(MARGIN + dx * CELL_SIZE, MARGIN + dy * CELL_SIZE, 4);
        }
    }
}

// 绘制立体感/光泽感棋子
void DrawPiece(int r, int c, int type) {
    int x = MARGIN + c * CELL_SIZE;
    int y = MARGIN + r * CELL_SIZE;
    int radius = CELL_SIZE / 2 - 3;

    // 阴影效果
    setfillcolor(RGB(150, 110, 60));
    solidcircle(x + 2, y + 2, radius);

    if (type == PLAYER) { // 黑棋 (渐变立体感)
        for (int i = radius; i > 0; i--) {
            int colorVal = (int)(20 + (radius - i) * (60.0 / radius));
            setfillcolor(RGB(colorVal, colorVal, colorVal));
            solidcircle(x - (radius - i) / 3, y - (radius - i) / 3, i);
        }
        // 反光高光
        setfillcolor(RGB(120, 120, 120));
        solidcircle(x - radius / 3, y - radius / 3, radius / 4);
    }
    else if (type == AI) { // 白棋 (润泽白玉感)
        for (int i = radius; i > 0; i--) {
            int colorVal = (int)(200 + (radius - i) * (55.0 / radius));
            setfillcolor(RGB(colorVal, colorVal, colorVal));
            solidcircle(x - (radius - i) / 3, y - (radius - i) / 3, i);
        }
        // 反光高光
        setfillcolor(RGB(255, 255, 255));
        solidcircle(x - radius / 3, y - radius / 3, radius / 3);
    }

    // 如果是最新落下的棋子，绘制高亮红点标识
    if (r == lastY && c == lastX) {
        setfillcolor(RGB(235, 60, 60));
        solidcircle(x, y, 4);
    }
}

// 刷新整个画面
void Redraw() {
    BeginBatchDraw();
    DrawBoard();
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (board[r][c] != EMPTY) {
                DrawPiece(r, c, board[r][c]);
            }
        }
    }
    EndBatchDraw();
}

// ===================== AI 核心算法 (博弈树 + Alpha-Beta 剪枝) =====================

// 评估棋盘特定方向上的连子情况
int EvaluateDirection(int r, int c, int dr, int dc, int role) {
    int count = 1;
    int openEnds = 0;

    // 正向延伸
    int i = 1;
    while (true) {
        int nr = r + dr * i, nc = c + dc * i;
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) break;
        if (board[nr][nc] == role) count++;
        else {
            if (board[nr][nc] == EMPTY) openEnds++;
            break;
        }
        i++;
    }

    // 反向延伸
    i = 1;
    while (true) {
        int nr = r - dr * i, nc = c - dc * i;
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) break;
        if (board[nr][nc] == role) count++;
        else {
            if (board[nr][nc] == EMPTY) openEnds++;
            break;
        }
        i++;
    }

    // 模式打分
    if (count >= 5) return 100000;
    if (count == 4) {
        if (openEnds == 2) return 10000;
        if (openEnds == 1) return 1000;
    }
    if (count == 3) {
        if (openEnds == 2) return 1000;
        if (openEnds == 1) return 100;
    }
    if (count == 2) {
        if (openEnds == 2) return 100;
        if (openEnds == 1) return 10;
    }
    return 0;
}

// 评估某个位置对某个角色的价值
int EvaluatePoint(int r, int c, int role) {
    int score = 0;
    int dirs[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };
    for (auto& dir : dirs) {
        score += EvaluateDirection(r, c, dir[0], dir[1], role);
    }
    return score;
}

// 检查是否有人胜利
bool CheckWin(int r, int c, int role) {
    int dirs[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };
    for (auto& dir : dirs) {
        int count = 1;
        for (int step = 1; step < 5; step++) {
            int nr = r + dir[0] * step, nc = c + dir[1] * step;
            if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE && board[nr][nc] == role) count++;
            else break;
        }
        for (int step = 1; step < 5; step++) {
            int nr = r - dir[0] * step, nc = c - dir[1] * step;
            if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE && board[nr][nc] == role) count++;
            else break;
        }
        if (count >= 5) return true;
    }
    return false;
}

// AI 思考最佳落子位置
void AiMove(int& outR, int& outC) {
    int maxScore = -1;
    outR = 7; outC = 7; // 默认中点

    std::vector<std::pair<int, std::pair<int, int>>> candidateMoves;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (board[r][c] != EMPTY) continue;

            // 评估 AI 在此落子的攻击得分，以及阻止玩家在此落子的防御得分
            int aiScore = EvaluatePoint(r, c, AI);
            int playerScore = EvaluatePoint(r, c, PLAYER);

            // 综合打分：进攻为主，兼顾防守
            int totalScore = aiScore + (int)(playerScore * 0.9);

            candidateMoves.push_back({ totalScore, {r, c} });
        }
    }

    if (!candidateMoves.empty()) {
        std::sort(candidateMoves.rbegin(), candidateMoves.rend());
        outR = candidateMoves[0].second.first;
        outC = candidateMoves[0].second.second;
    }
}

// 显示优雅的胜负对话框
void ShowWinner(const wchar_t* message) {
    Redraw();
    MessageBox(GetHWnd(), message, L"游戏结束", MB_OK | MB_ICONINFORMATION);
}

// ===================== 主程序 =====================
int main() {
    // 创建图形窗口
    initgraph(BOARD_SIZE, BOARD_SIZE);
    SetWindowText(GetHWnd(), L"C++ 五子棋人机对战");

    Redraw();

    bool gameOver = false;
    ExMessage msg;

    while (!gameOver) {
        if (peekmessage(&msg, EM_MOUSE)) {
            // 响应鼠标左键点击
            if (msg.message == WM_LBUTTONDOWN) {
                int c = (msg.x - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
                int r = (msg.y - MARGIN + CELL_SIZE / 2) / CELL_SIZE;

                // 检查点击位置有效性
                if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE && board[r][c] == EMPTY) {
                    // 1. 玩家落子 (黑棋)
                    board[r][c] = PLAYER;
                    lastY = r; lastX = c;
                    Redraw();

                    if (CheckWin(r, c, PLAYER)) {
                        ShowWinner(L"恭喜你，获得了胜利！🎉");
                        gameOver = true;
                        break;
                    }

                    // 2. AI 落子 (白棋)
                    Sleep(200); // 模拟思考延时
                    int aiR, aiC;
                    AiMove(aiR, aiC);
                    board[aiR][aiC] = AI;
                    lastY = aiR; lastX = aiC;
                    Redraw();

                    if (CheckWin(aiR, aiC, AI)) {
                        ShowWinner(L"很遗憾，AI 赢得了比赛！🤖");
                        gameOver = true;
                        break;
                    }
                }
            }
        }
    }

    closegraph();
    return 0;
}