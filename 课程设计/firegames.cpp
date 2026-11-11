#include <graphics.h>
#include <conio.h>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstring>
#include <windows.h>
#include <string>
#include <cmath>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

const int GRID = 19;
const int CELL = 32;
const int MARGIN = 40;
const int BOARD_PX = MARGIN * 2 + (GRID - 1) * CELL;
const int WIN_H = BOARD_PX + 70;

const int EMPTY = 0;
const int STONE_BLACK = 1;
const int STONE_WHITE = 2;

int board[GRID][GRID] = { 0 };
int playerColor = STONE_BLACK;
int aiColor = STONE_WHITE;
int difficulty = 1;
int useForbidden = 0;
int enableBGM = 1;
int enableSFX = 1;
int timerLevel = 4;
int dualMode = 0;
int gameOver = 0;
int viewing = 0;
int lastR = -1, lastC = -1;
int moveCount = 0;
int hoverR = -1, hoverC = -1;
int pausedGame = 0;
int playerHasMoved = 0;
std::vector<std::pair<int, int>> history;

std::mt19937 rng((unsigned)time(0));

wchar_t exeDir[MAX_PATH] = { 0 };

int timerSeconds[] = { 5, 10, 15, 30, 0 };
int remainTime = 0;
DWORD lastTick = 0;
int isPlayerTurn = 1;
int currentTurn = STONE_BLACK;

void GetExeDir() {
    GetModuleFileNameW(NULL, exeDir, MAX_PATH);
    wchar_t* p = wcsrchr(exeDir, L'\\');
    if (p) *(p + 1) = L'\0';
}

void PlayBGM() {
    if (!enableBGM) return;
    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%sbgm.mp3", exeDir);
    wchar_t cmd[MAX_PATH + 64];
    mciSendString(L"close bgm", NULL, 0, NULL);
    swprintf_s(cmd, L"open \"%s\" alias bgm", path);
    mciSendString(cmd, NULL, 0, NULL);
    mciSendString(L"play bgm repeat", NULL, 0, NULL);
}

void StopBGM() {
    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"close bgm", NULL, 0, NULL);
    mciSendString(L"close drop", NULL, 0, NULL);
}

void PlayDropSound() {
    if (!enableSFX) return;
    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%sdrop.mp3", exeDir);
    wchar_t cmd[MAX_PATH + 64];
    mciSendString(L"close drop", NULL, 0, NULL);
    swprintf_s(cmd, L"open \"%s\" alias drop", path);
    mciSendString(cmd, NULL, 0, NULL);
    mciSendString(L"setaudio drop volume to 900", NULL, 0, NULL);
    mciSendString(L"play drop", NULL, 0, NULL);
}

void ResetPlayerTimer() {
    if (timerLevel >= 4) return;
    remainTime = timerSeconds[timerLevel];
    lastTick = GetTickCount();
}

void DrawBoard() {
    setfillcolor(RGB(222, 172, 98));
    solidrectangle(0, 0, BOARD_PX, BOARD_PX);
    setlinecolor(RGB(200, 145, 70));
    for (int i = 0; i < BOARD_PX; i += 5) line(0, i, BOARD_PX, i);
    setlinecolor(RGB(70, 35, 8));
    setlinestyle(PS_SOLID, 3);
    rectangle(MARGIN - 10, MARGIN - 10, BOARD_PX - MARGIN + 10, BOARD_PX - MARGIN + 10);
    setlinestyle(PS_SOLID, 1);
    setlinecolor(RGB(50, 25, 5));
    for (int i = 0; i < GRID; i++) {
        line(MARGIN, MARGIN + i * CELL, MARGIN + (GRID - 1) * CELL, MARGIN + i * CELL);
        line(MARGIN + i * CELL, MARGIN, MARGIN + i * CELL, MARGIN + (GRID - 1) * CELL);
    }
    int dots[] = { 3, 9, 15 };
    setfillcolor(RGB(50, 25, 5));
    for (int a = 0; a < 3; a++)
        for (int b = 0; b < 3; b++)
            fillcircle(MARGIN + dots[a] * CELL, MARGIN + dots[b] * CELL, 3);
}

void DrawPiece(int r, int c, int type, int order = -1) {
    int x = MARGIN + c * CELL;
    int y = MARGIN + r * CELL;
    int rad = CELL / 2 - 2;
    setfillcolor(RGB(140, 100, 50));
    solidcircle(x + 1, y + 1, rad);
    if (type == STONE_BLACK) {
        for (int i = rad; i > 0; --i) {
            int v = 15 + (rad - i) * 50 / rad;
            setfillcolor(RGB(v, v, v));
            solidcircle(x - (rad - i) / 4, y - (rad - i) / 4, i);
        }
        setfillcolor(RGB(100, 100, 100));
        solidcircle(x - rad / 3, y - rad / 3, rad / 5);
    }
    else {
        for (int i = rad; i > 0; --i) {
            int v = 190 + (rad - i) * 60 / rad;
            setfillcolor(RGB(v, v, v));
            solidcircle(x - (rad - i) / 4, y - (rad - i) / 4, i);
        }
        setfillcolor(RGB(255, 255, 255));
        solidcircle(x - rad / 3, y - rad / 3, rad / 4);
    }
    if (r == lastR && c == lastC && !viewing) {
        setfillcolor(RGB(220, 50, 50));
        solidcircle(x, y, 3);
    }
    if (viewing && order > 0) {
        setbkmode(TRANSPARENT);
        settextcolor(type == STONE_BLACK ? RGB(220, 220, 220) : RGB(40, 40, 40));
        settextstyle(12, 0, L"Arial");
        wchar_t buf[8];
        swprintf_s(buf, L"%d", order);
        int tw = textwidth(buf);
        outtextxy(x - tw / 2, y - 6, buf);
    }
}

void DrawGhost(int r, int c, int type) {
    int x = MARGIN + c * CELL;
    int y = MARGIN + r * CELL;
    int rad = CELL / 2 - 2;
    if (type == STONE_BLACK)
        setfillcolor(RGB(60, 60, 60));
    else
        setfillcolor(RGB(200, 200, 200));
    solidcircle(x, y, rad);
    setlinecolor(RGB(180, 180, 180));
    setlinestyle(PS_SOLID, 1);
    circle(x, y, rad);
}

void DrawCenteredText(int cx, int cy, const wchar_t* text) {
    setbkmode(TRANSPARENT);
    int w = textwidth(text), h = textheight(text);
    outtextxy(cx - w / 2, cy - h / 2, text);
}

void DrawTimer() {
    if (timerLevel >= 4 || gameOver) return;

    int cx = BOARD_PX / 2;
    int cy = BOARD_PX + 30;
    int radius = 18;

    setlinecolor(RGB(80, 80, 90));
    setlinestyle(PS_SOLID, 3);
    circle(cx, cy, radius);

    settextcolor(WHITE);
    settextstyle(16, 0, L"微软雅黑");
    wchar_t buf[8];
    swprintf_s(buf, L"%d", remainTime);
    DrawCenteredText(cx, cy, buf);
}

void RedrawBoard(int showNumbers = 0) {
    BeginBatchDraw();
    setbkcolor(RGB(28, 28, 38));
    cleardevice();
    DrawBoard();
    for (int i = 0; i < (int)history.size(); i++) {
        int r = history[i].first, c = history[i].second;
        DrawPiece(r, c, board[r][c], showNumbers ? i + 1 : -1);
    }
    if (!gameOver && hoverR >= 0 && hoverC >= 0 && board[hoverR][hoverC] == EMPTY)
        DrawGhost(hoverR, hoverC, dualMode ? currentTurn : playerColor);

    setbkmode(TRANSPARENT);
    setfillcolor(RGB(70, 70, 80));
    solidroundrect(40, BOARD_PX + 10, 180, BOARD_PX + 50, 8, 8);
    settextcolor(WHITE);
    settextstyle(16, 0, L"微软雅黑");
    DrawCenteredText(110, BOARD_PX + 30, L"撤回");

    solidroundrect(BOARD_PX - 180, BOARD_PX + 10, BOARD_PX - 40, BOARD_PX + 50, 8, 8);
    const wchar_t* btnText = (gameOver || !playerHasMoved) ? L"回到桌面" : L"暂离";
    DrawCenteredText(BOARD_PX - 110, BOARD_PX + 30, btnText);

    DrawTimer();
    EndBatchDraw();
}

int CheckFive(int r, int c, int role) {
    const int d[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int cnt = 1;
        for (int s = 1; s < 5; s++) {
            int nr = r + d[k][0] * s, nc = c + d[k][1] * s;
            if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID || board[nr][nc] != role) break;
            cnt++;
        }
        for (int s = 1; s < 5; s++) {
            int nr = r - d[k][0] * s, nc = c - d[k][1] * s;
            if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID || board[nr][nc] != role) break;
            cnt++;
        }
        if (cnt >= 5) return 1;
    }
    return 0;
}

int CountDirection(int r, int c, int dr, int dc, int role, int& open) {
    int cnt = 1; open = 0;
    for (int s = 1; s < 6; s++) {
        int nr = r + dr * s, nc = c + dc * s;
        if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID) break;
        if (board[nr][nc] == role) cnt++;
        else { if (board[nr][nc] == EMPTY) open++; break; }
    }
    for (int s = 1; s < 6; s++) {
        int nr = r - dr * s, nc = c - dc * s;
        if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID) break;
        if (board[nr][nc] == role) cnt++;
        else { if (board[nr][nc] == EMPTY) open++; break; }
    }
    return cnt;
}

int IsOverline(int r, int c, int role) {
    if (!useForbidden || role != STONE_BLACK) return 0;
    const int d[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int open = 0;
        if (CountDirection(r, c, d[k][0], d[k][1], role, open) >= 6) return 1;
    }
    return 0;
}

int IsDoubleThree(int r, int c, int role) {
    if (!useForbidden || role != STONE_BLACK) return 0;
    int live = 0;
    const int d[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int open = 0;
        if (CountDirection(r, c, d[k][0], d[k][1], role, open) == 3 && open == 2) live++;
    }
    return live >= 2;
}

int IsDoubleFour(int r, int c, int role) {
    if (!useForbidden || role != STONE_BLACK) return 0;
    int fours = 0;
    const int d[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int open = 0;
        if (CountDirection(r, c, d[k][0], d[k][1], role, open) == 4 && open >= 1) fours++;
    }
    return fours >= 2;
}

int IsForbidden(int r, int c, int role) {
    if (!useForbidden || role != STONE_BLACK) return 0;
    if (CheckFive(r, c, role)) return 0;
    return IsOverline(r, c, role) || IsDoubleFour(r, c, role) || IsDoubleThree(r, c, role);
}

int ScoreLine(int r, int c, int dr, int dc, int role) {
    int open = 0;
    int cnt = CountDirection(r, c, dr, dc, role, open);
    if (cnt >= 5) return 2000000;
    if (cnt == 4 && open == 2) return 300000;
    if (cnt == 4 && open == 1) return 40000;
    if (cnt == 3 && open == 2) return 25000;
    if (cnt == 3 && open == 1) return 2500;
    if (cnt == 2 && open == 2) return 600;
    if (cnt == 2 && open == 1) return 60;
    return 0;
}

int EvaluatePoint(int r, int c, int role) {
    const int d[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    int s = 0;
    for (int k = 0; k < 4; k++) s += ScoreLine(r, c, d[k][0], d[k][1], role);
    return s;
}

int EvaluateBoard() {
    int score = 0;
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (board[r][c] == aiColor) score += EvaluatePoint(r, c, aiColor);
            else if (board[r][c] == playerColor) score -= EvaluatePoint(r, c, playerColor) * 13 / 10;
        }
    return score;
}

int NeedAvoidForbidden() {
    return useForbidden && aiColor == STONE_BLACK;
}

void AI_Easy(int& outR, int& outC) {
    int best = -1;
    std::vector<std::pair<int, int>> cands;
    int avoid = NeedAvoidForbidden();
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (board[r][c]) continue;
            if (avoid && IsForbidden(r, c, aiColor)) continue;
            int s = EvaluatePoint(r, c, aiColor) + EvaluatePoint(r, c, playerColor) * 9 / 10;
            if (s > best) { best = s; cands.clear(); cands.push_back(std::make_pair(r, c)); }
            else if (s == best) cands.push_back(std::make_pair(r, c));
        }
    if (cands.empty()) { outR = 9; outC = 9; return; }
    std::pair<int, int> p = cands[rng() % cands.size()];
    outR = p.first; outC = p.second;
}

struct MoveScore {
    int r, c, score;
};

bool cmpMove(const MoveScore& a, const MoveScore& b) { return a.score > b.score; }

int Minimax(int depth, int alpha, int beta, int maximizing, int role) {
    if (depth == 0) return EvaluateBoard();
    std::vector<MoveScore> moves;
    int r, c, dr, dc, nr, nc;
    int hasNear;
    int avoid = (useForbidden && role == STONE_BLACK);
    for (r = 0; r < GRID; r++)
        for (c = 0; c < GRID; c++)
            if (!board[r][c]) {
                hasNear = 0;
                for (dr = -2; dr <= 2 && !hasNear; dr++)
                    for (dc = -2; dc <= 2; dc++) {
                        nr = r + dr; nc = c + dc;
                        if (nr >= 0 && nr < GRID && nc >= 0 && nc < GRID && board[nr][nc]) hasNear = 1;
                    }
                if (hasNear || history.empty()) {
                    if (avoid && IsForbidden(r, c, role)) continue;
                    int sc = EvaluatePoint(r, c, role) + EvaluatePoint(r, c, 3 - role);
                    moves.push_back({ r, c, sc });
                }
            }
    if (moves.empty()) return 0;
    std::sort(moves.begin(), moves.end(), cmpMove);

    int best, v;
    size_t i;
    if (maximizing) {
        best = -999999999;
        for (i = 0; i < moves.size(); i++) {
            int mr = moves[i].r, mc = moves[i].c;
            board[mr][mc] = role;
            if (CheckFive(mr, mc, role)) { board[mr][mc] = EMPTY; return 2000000; }
            v = Minimax(depth - 1, alpha, beta, 0, 3 - role);
            board[mr][mc] = EMPTY;
            if (v > best) best = v;
            if (v > alpha) alpha = v;
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        best = 999999999;
        for (i = 0; i < moves.size(); i++) {
            int mr = moves[i].r, mc = moves[i].c;
            board[mr][mc] = role;
            if (CheckFive(mr, mc, role)) { board[mr][mc] = EMPTY; return -2000000; }
            v = Minimax(depth - 1, alpha, beta, 1, 3 - role);
            board[mr][mc] = EMPTY;
            if (v < best) best = v;
            if (v < beta) beta = v;
            if (beta <= alpha) break;
        }
        return best;
    }
}

void FindBest(int& outR, int& outC, int depth) {
    int r, c, dr, dc, nr, nc, s, best;
    int hasNear;
    int avoid = NeedAvoidForbidden();

    for (r = 0; r < GRID; r++)
        for (c = 0; c < GRID; c++)
            if (!board[r][c] && !(avoid && IsForbidden(r, c, aiColor))) {
                board[r][c] = aiColor;
                if (CheckFive(r, c, aiColor)) { board[r][c] = EMPTY; outR = r; outC = c; return; }
                board[r][c] = EMPTY;
            }
    for (r = 0; r < GRID; r++)
        for (c = 0; c < GRID; c++)
            if (!board[r][c]) {
                board[r][c] = playerColor;
                if (CheckFive(r, c, playerColor)) { board[r][c] = EMPTY; outR = r; outC = c; return; }
                board[r][c] = EMPTY;
            }
    best = -999999999;
    outR = 9; outC = 9;
    for (r = 0; r < GRID; r++)
        for (c = 0; c < GRID; c++) {
            if (board[r][c]) continue;
            if (avoid && IsForbidden(r, c, aiColor)) continue;
            hasNear = (history.empty());
            if (!hasNear) {
                for (dr = -2; dr <= 2 && !hasNear; dr++)
                    for (dc = -2; dc <= 2; dc++) {
                        nr = r + dr; nc = c + dc;
                        if (nr >= 0 && nr < GRID && nc >= 0 && nc < GRID && board[nr][nc]) hasNear = 1;
                    }
            }
            if (!hasNear) continue;
            board[r][c] = aiColor;
            s = Minimax(depth, -999999999, 999999999, 0, playerColor);
            board[r][c] = EMPTY;
            if (s > best) { best = s; outR = r; outC = c; }
        }
}

struct MCTSNode {
    int r, c;
    int visits;
    double wins;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int player;
};

void DeleteTree(MCTSNode* node) {
    if (!node) return;
    for (size_t i = 0; i < node->children.size(); i++)
        DeleteTree(node->children[i]);
    delete node;
}

std::vector<std::pair<int, int>> GetCandidates(int simBoard[GRID][GRID], int role) {
    std::vector<MoveScore> scored;
    int avoid = (useForbidden && role == STONE_BLACK);
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (simBoard[r][c]) continue;
            if (avoid && IsForbidden(r, c, role)) continue;
            int hasNear = 0;
            for (int dr = -2; dr <= 2 && !hasNear; dr++)
                for (int dc = -2; dc <= 2; dc++) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < GRID && nc >= 0 && nc < GRID && simBoard[nr][nc]) hasNear = 1;
                }
            if (!hasNear && history.size() > 0) continue;
            int sc = EvaluatePoint(r, c, role) * 2 + EvaluatePoint(r, c, 3 - role);
            scored.push_back({ r, c, sc });
        }
    std::sort(scored.begin(), scored.end(), cmpMove);
    std::vector<std::pair<int, int>> res;
    int limit = 18;
    if ((int)scored.size() < limit) limit = (int)scored.size();
    for (int i = 0; i < limit; i++)
        res.push_back(std::make_pair(scored[i].r, scored[i].c));
    return res;
}

int Simulate(int simBoard[GRID][GRID], int turn) {
    int localMove = 0;
    while (localMove < 40) {
        auto cands = GetCandidates(simBoard, turn);
        if (cands.empty()) return 0;
        int topN = 6;
        if ((int)cands.size() < topN) topN = (int)cands.size();
        int idx = (rng() % 5 == 0) ? (rng() % cands.size()) : (rng() % topN);
        auto mv = cands[idx];
        simBoard[mv.first][mv.second] = turn;
        if (CheckFive(mv.first, mv.second, turn)) return (turn == aiColor) ? 1 : -1;
        turn = 3 - turn;
        localMove++;
    }
    return 0;
}

void AI_MCTS(int& outR, int& outC) {
    const int SIMULATIONS = 600;
    MCTSNode* root = new MCTSNode{ -1, -1, 0, 0.0, nullptr, {}, aiColor };

    for (int sim = 0; sim < SIMULATIONS; sim++) {
        int simBoard[GRID][GRID];
        memcpy(simBoard, board, sizeof(board));
        MCTSNode* node = root;
        int turn = aiColor;
        int depth = 0;

        while (depth < 15) {
            if (node->children.empty()) {
                auto cands = GetCandidates(simBoard, turn);
                for (size_t i = 0; i < cands.size(); i++) {
                    MCTSNode* child = new MCTSNode{ cands[i].first, cands[i].second, 0, 0.0, node, {}, turn };
                    node->children.push_back(child);
                }
                if (node->children.empty()) break;
                node = node->children[0];
                simBoard[node->r][node->c] = turn;
                turn = 3 - turn;
                break;
            }
            double bestUCT = -1e9;
            MCTSNode* bestChild = nullptr;
            for (size_t i = 0; i < node->children.size(); i++) {
                MCTSNode* ch = node->children[i];
                double uct = (ch->visits == 0) ? 1e9 : (ch->wins / ch->visits) + 1.2 * sqrt(log(node->visits + 1.0) / ch->visits);
                if (uct > bestUCT) { bestUCT = uct; bestChild = ch; }
            }
            if (!bestChild) break;
            node = bestChild;
            simBoard[node->r][node->c] = node->player;
            turn = 3 - node->player;
            depth++;
        }

        int result = Simulate(simBoard, turn);

        while (node) {
            node->visits++;
            if (node->player == aiColor) node->wins += (result == 1) ? 1.0 : (result == -1 ? 0.0 : 0.5);
            else node->wins += (result == -1) ? 1.0 : (result == 1 ? 0.0 : 0.5);
            node = node->parent;
        }
    }

    double bestRate = -1;
    outR = 9; outC = 9;
    for (size_t i = 0; i < root->children.size(); i++) {
        MCTSNode* ch = root->children[i];
        if (ch->visits > 0) {
            double rate = ch->wins / ch->visits;
            if (rate > bestRate) { bestRate = rate; outR = ch->r; outC = ch->c; }
        }
    }
    DeleteTree(root);
}

void AI_Medium(int& outR, int& outC) { AI_MCTS(outR, outC); }
void AI_Hard(int& outR, int& outC) { FindBest(outR, outC, 4); }

void DoAIMove() {
    if (dualMode) return;
    isPlayerTurn = 0;
    ResetPlayerTimer();
    RedrawBoard();
    Sleep(350);
    int r = 9, c = 9;
    if (moveCount == 0 && aiColor == STONE_BLACK) {
        r = 9; c = 9;
    }
    else {
        if (difficulty == 1) AI_Easy(r, c);
        else if (difficulty == 2) AI_Medium(r, c);
        else AI_Hard(r, c);
    }
    if (NeedAvoidForbidden() && IsForbidden(r, c, aiColor)) {
        int found = 0;
        for (int rr = 0; rr < GRID && !found; rr++)
            for (int cc = 0; cc < GRID && !found; cc++)
                if (!board[rr][cc] && !IsForbidden(rr, cc, aiColor)) {
                    r = rr; c = cc; found = 1;
                }
    }
    board[r][c] = aiColor;
    history.push_back(std::make_pair(r, c));
    lastR = r; lastC = c;
    moveCount++;
    hoverR = hoverC = -1;
    PlayDropSound();
    RedrawBoard();
    Sleep(200);
    if (IsForbidden(r, c, aiColor)) {
        MessageBox(GetHWnd(), L"黑棋禁手，白棋获胜！", L"游戏结束", MB_OK);
        gameOver = 1; viewing = 1; RedrawBoard(1); return;
    }
    if (CheckFive(r, c, aiColor)) {
        MessageBox(GetHWnd(), dualMode ? L"白棋获胜！" : L"AI 获胜！", L"游戏结束", MB_OK);
        gameOver = 1; viewing = 1; RedrawBoard(1);
    }
    isPlayerTurn = 1;
    ResetPlayerTimer();
}

void DrawMenu() {
    cleardevice();
    setbkcolor(RGB(28, 28, 38));
    cleardevice();
    setbkmode(TRANSPARENT);

    settextstyle(42, 0, L"微软雅黑");
    settextcolor(RGB(245, 225, 190));
    int titleW = textwidth(L"五子棋");
    int modeW = 140;
    int gap = 20;
    int totalW = titleW + gap + modeW;
    int startX = (BOARD_PX - totalW) / 2;

    outtextxy(startX, 40, L"五子棋");

    COLORREF modeColor = pausedGame ? RGB(90, 90, 90) : (dualMode ? RGB(50, 160, 70) : RGB(50, 130, 210));
    setfillcolor(modeColor);
    solidroundrect(startX + titleW + gap, 38, startX + titleW + gap + modeW, 82, 10, 10);
    settextcolor(pausedGame ? RGB(160, 160, 160) : WHITE);
    settextstyle(18, 0, L"微软雅黑");
    DrawCenteredText(startX + titleW + gap + modeW / 2, 60, dualMode ? L"双人对战" : L"人机对战");

    settextstyle(22, 0, L"微软雅黑");
    settextcolor(RGB(210, 210, 210));
    outtextxy(70, 120, L"难度：");
    outtextxy(70, 230, L"执子：");

    auto drawBtn = [](int x, int y, int w, int h, const wchar_t* text, int sel) {
        setfillcolor(sel ? RGB(50, 130, 210) : RGB(48, 48, 58));
        solidroundrect(x, y, x + w, y + h, 10, 10);
        settextcolor(WHITE);
        settextstyle(20, 0, L"微软雅黑");
        DrawCenteredText(x + w / 2, y + h / 2, text);
        };

    drawBtn(70, 150, 150, 48, L"简单", difficulty == 1);
    drawBtn(250, 150, 150, 48, L"中等", difficulty == 2);
    drawBtn(430, 150, 150, 48, L"困难", difficulty == 3);

    drawBtn(70, 260, 150, 48, L"黑棋", playerColor == STONE_BLACK);
    drawBtn(250, 260, 150, 48, L"白棋", playerColor == STONE_WHITE);
    drawBtn(430, 260, 150, 48, L"随机", playerColor == 0);

    COLORREF forbidColor = pausedGame ? RGB(90, 90, 90) : RGB(48, 48, 58);
    setfillcolor(forbidColor);
    solidroundrect(70, 330, 110, 370, 6, 6);
    if (useForbidden && !pausedGame) {
        setlinecolor(RGB(80, 180, 100));
        setlinestyle(PS_SOLID, 3);
        line(80, 350, 95, 365);
        line(95, 365, 105, 335);
    }
    settextcolor(pausedGame ? RGB(140, 140, 140) : RGB(210, 210, 210));
    settextstyle(20, 0, L"微软雅黑");
    outtextxy(125, 340, L"启用禁手");

    solidroundrect(250, 330, 290, 370, 6, 6);
    if (enableBGM) {
        setlinecolor(RGB(80, 180, 100));
        setlinestyle(PS_SOLID, 3);
        line(260, 350, 275, 365);
        line(275, 365, 285, 335);
    }
    settextcolor(RGB(210, 210, 210));
    outtextxy(305, 340, L"音乐");

    solidroundrect(400, 330, 440, 370, 6, 6);
    if (enableSFX) {
        setlinecolor(RGB(80, 180, 100));
        setlinestyle(PS_SOLID, 3);
        line(410, 350, 425, 365);
        line(425, 365, 435, 335);
    }
    outtextxy(455, 340, L"音效");

    settextcolor(pausedGame ? RGB(140, 140, 140) : RGB(210, 210, 210));
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(70, 400, L"计时器：");
    const wchar_t* labels[] = { L"5秒", L"10秒", L"15秒", L"30秒", L"无限制" };
    int baseX = 160;
    for (int i = 0; i < 5; i++) {
        int x = baseX + i * 90;
        COLORREF btnColor = pausedGame ? RGB(90, 90, 90) : (timerLevel == i ? RGB(50, 130, 210) : RGB(48, 48, 58));
        setfillcolor(btnColor);
        solidroundrect(x, 390, x + 70, 430, 8, 8);
        settextcolor(pausedGame ? RGB(160, 160, 160) : WHITE);
        settextstyle(16, 0, L"微软雅黑");
        DrawCenteredText(x + 35, 410, labels[i]);
    }

    if (pausedGame) {
        // 两个按钮整体居中
        int btnW = 180;
        int btnH = 60;
        int gap = 40;
        int totalBtnW = btnW * 2 + gap;
        int btnStartX = (BOARD_PX - totalBtnW) / 2;

        setfillcolor(RGB(50, 160, 70));
        solidroundrect(btnStartX, 490, btnStartX + btnW, 550, 12, 12);
        settextcolor(WHITE);
        settextstyle(22, 0, L"微软雅黑");
        DrawCenteredText(btnStartX + btnW / 2, 520, L"继续游戏");

        setfillcolor(RGB(210, 65, 45));
        solidroundrect(btnStartX + btnW + gap, 490, btnStartX + btnW * 2 + gap, 550, 12, 12);
        DrawCenteredText(btnStartX + btnW + gap + btnW / 2, 520, L"重制游戏");
    }
    else {
        setfillcolor(RGB(210, 65, 45));
        solidroundrect(220, 490, 440, 550, 12, 12);
        settextcolor(WHITE);
        settextstyle(26, 0, L"微软雅黑");
        DrawCenteredText(330, 520, L"开 始");
    }

    setfillcolor(RGB(80, 80, 90));
    solidroundrect(220, 580, 440, 630, 10, 10);
    settextstyle(20, 0, L"微软雅黑");
    DrawCenteredText(330, 605, L"退 出");
}

void RunMenu() {
    int needRedraw = 1;
    if (enableBGM) PlayBGM();
    while (1) {
        if (needRedraw) { DrawMenu(); needRedraw = 0; }
        ExMessage msg;
        if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN) {
            int x = msg.x, y = msg.y;
            int changed = 0;

            settextstyle(42, 0, L"微软雅黑");
            int titleW = textwidth(L"五子棋");
            int modeW = 140;
            int gap = 20;
            int totalW = titleW + gap + modeW;
            int startX = (BOARD_PX - totalW) / 2;
            int modeX1 = startX + titleW + gap;
            int modeX2 = modeX1 + modeW;

            if (y >= 150 && y <= 198) {
                if (x >= 70 && x <= 220 && difficulty != 1) { difficulty = 1; changed = 1; }
                else if (x >= 250 && x <= 400 && difficulty != 2) { difficulty = 2; changed = 1; }
                else if (x >= 430 && x <= 580 && difficulty != 3) { difficulty = 3; changed = 1; }
            }
            if (y >= 260 && y <= 308) {
                if (x >= 70 && x <= 220 && playerColor != STONE_BLACK) { playerColor = STONE_BLACK; changed = 1; }
                else if (x >= 250 && x <= 400 && playerColor != STONE_WHITE) { playerColor = STONE_WHITE; changed = 1; }
                else if (x >= 430 && x <= 580 && playerColor != 0) { playerColor = 0; changed = 1; }
            }
            if (!pausedGame && x >= 70 && x <= 110 && y >= 330 && y <= 370) {
                useForbidden = !useForbidden;
                changed = 1;
            }
            if (x >= 250 && x <= 290 && y >= 330 && y <= 370) {
                enableBGM = !enableBGM;
                if (enableBGM) PlayBGM();
                else StopBGM();
                changed = 1;
            }
            if (x >= 400 && x <= 440 && y >= 330 && y <= 370) {
                enableSFX = !enableSFX;
                changed = 1;
            }
            if (!pausedGame && y >= 390 && y <= 430) {
                for (int i = 0; i < 5; i++) {
                    int bx = 160 + i * 90;
                    if (x >= bx && x <= bx + 70 && timerLevel != i) {
                        timerLevel = i;
                        changed = 1;
                        break;
                    }
                }
            }
            if (!pausedGame && x >= modeX1 && x <= modeX2 && y >= 38 && y <= 82) {
                dualMode = !dualMode;
                changed = 1;
            }

            if (pausedGame) {
                int btnW = 180;
                int gapBtn = 40;
                int totalBtnW = btnW * 2 + gapBtn;
                int btnStartX = (BOARD_PX - totalBtnW) / 2;

                if (x >= btnStartX && x <= btnStartX + btnW && y >= 490 && y <= 550) {
                    return;
                }
                if (x >= btnStartX + btnW + gapBtn && x <= btnStartX + btnW * 2 + gapBtn && y >= 490 && y <= 550) {
                    pausedGame = 0;
                    needRedraw = 1;
                    continue;
                }
            }
            else {
                if (x >= 220 && x <= 440 && y >= 490 && y <= 550) {
                    if (playerColor == 0) playerColor = (rng() % 2 == 0) ? STONE_BLACK : STONE_WHITE;
                    aiColor = 3 - playerColor;
                    return;
                }
            }

            if (x >= 220 && x <= 440 && y >= 580 && y <= 630) {
                StopBGM();
                closegraph();
                exit(0);
            }
            if (changed) needRedraw = 1;
        }
        Sleep(10);
    }
}

void UndoToPlayer() {
    if (history.size() < 2 || gameOver) return;
    std::pair<int, int> ai = history.back(); history.pop_back(); board[ai.first][ai.second] = EMPTY; moveCount--;
    std::pair<int, int> pl = history.back(); history.pop_back(); board[pl.first][pl.second] = EMPTY; moveCount--;
    if (!history.empty()) { lastR = history.back().first; lastC = history.back().second; }
    else lastR = lastC = -1;
    int playerMoves = 0;
    for (size_t i = 0; i < history.size(); i++) {
        if ((i % 2 == 0 && playerColor == STONE_BLACK) || (i % 2 == 1 && playerColor == STONE_WHITE))
            playerMoves++;
    }
    playerHasMoved = (playerMoves > 0);
    isPlayerTurn = 1;
    if (dualMode) currentTurn = (history.size() % 2 == 0) ? STONE_BLACK : STONE_WHITE;
    ResetPlayerTimer();
    RedrawBoard();
}

void RunGame() {
    if (!pausedGame) {
        memset(board, 0, sizeof(board));
        history.clear();
        lastR = lastC = -1;
        moveCount = 0;
        gameOver = 0;
        viewing = 0;
        playerHasMoved = 0;
        currentTurn = STONE_BLACK;
    }
    pausedGame = 0;
    hoverR = hoverC = -1;
    isPlayerTurn = 1;
    ResetPlayerTimer();
    RedrawBoard();
    if (!dualMode && playerColor == STONE_WHITE && moveCount == 0) { Sleep(500); DoAIMove(); }
    ExMessage msg;
    int lastHoverTime = 0;
    while (1) {
        if (!gameOver && timerLevel < 4) {
            DWORD now = GetTickCount();
            if (now - lastTick >= 1000) {
                remainTime--;
                lastTick = now;
                RedrawBoard();
                if (remainTime <= 0) {
                    const wchar_t* msgText = dualMode ?
                        (currentTurn == STONE_BLACK ? L"黑棋超时，白棋获胜！" : L"白棋超时，黑棋获胜！") :
                        (isPlayerTurn ? L"时间到，你输了！" : L"时间到，AI 输了！");
                    MessageBox(GetHWnd(), msgText, L"游戏结束", MB_OK);
                    gameOver = 1; viewing = 1; RedrawBoard(1);
                    continue;
                }
            }
        }

        if (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_MOUSEMOVE && !gameOver) {
                int now = GetTickCount();
                if (now - lastHoverTime < 30) continue;
                lastHoverTime = now;

                int c = (msg.x - MARGIN + CELL / 2) / CELL;
                int r = (msg.y - MARGIN + CELL / 2) / CELL;
                if (r >= 0 && r < GRID && c >= 0 && c < GRID && board[r][c] == EMPTY) {
                    if (r != hoverR || c != hoverC) {
                        hoverR = r; hoverC = c;
                        RedrawBoard();
                    }
                }
                else if (hoverR != -1) {
                    hoverR = hoverC = -1;
                    RedrawBoard();
                }
            }
            if (msg.message == WM_LBUTTONDOWN) {
                int x = msg.x, y = msg.y;
                if (y >= BOARD_PX + 10 && y <= BOARD_PX + 50) {
                    if (x >= 40 && x <= 180) { UndoToPlayer(); continue; }
                    if (x >= BOARD_PX - 180 && x <= BOARD_PX - 40) {
                        if (!gameOver && playerHasMoved) pausedGame = 1;
                        return;
                    }
                }
                if (gameOver) continue;
                int c = (x - MARGIN + CELL / 2) / CELL;
                int r = (y - MARGIN + CELL / 2) / CELL;
                if (r < 0 || r >= GRID || c < 0 || c >= GRID || board[r][c] != EMPTY) continue;

                int placeColor = dualMode ? currentTurn : playerColor;
                if (moveCount == 0 && placeColor == STONE_BLACK && (r != 9 || c != 9)) {
                    MessageBox(GetHWnd(), L"黑棋第一手必须下在天元！", L"提示", MB_OK);
                    continue;
                }
                board[r][c] = placeColor;
                if (IsForbidden(r, c, placeColor)) {
                    board[r][c] = EMPTY;
                    MessageBox(GetHWnd(), L"禁手！你输了", L"游戏结束", MB_OK);
                    gameOver = 1; viewing = 1; RedrawBoard(1); continue;
                }
                history.push_back(std::make_pair(r, c));
                lastR = r; lastC = c;
                moveCount++;
                playerHasMoved = 1;
                hoverR = hoverC = -1;
                PlayDropSound();
                RedrawBoard();

                if (CheckFive(r, c, placeColor)) {
                    const wchar_t* winMsg = dualMode ?
                        (placeColor == STONE_BLACK ? L"黑棋获胜！" : L"白棋获胜！") :
                        L"恭喜你获胜！";
                    MessageBox(GetHWnd(), winMsg, L"游戏结束", MB_OK);
                    gameOver = 1; viewing = 1; RedrawBoard(1); continue;
                }

                if (dualMode) {
                    currentTurn = 3 - currentTurn;
                    ResetPlayerTimer();
                    RedrawBoard();
                }
                else {
                    DoAIMove();
                }
            }
        }
        Sleep(5);
    }
}

int main() {
    GetExeDir();
    initgraph(BOARD_PX, WIN_H);
    SetWindowText(GetHWnd(), L"五子棋");
    if (enableBGM) PlayBGM();
    while (1) { RunMenu(); RunGame(); }
    StopBGM();
    closegraph();
    return 0;
}