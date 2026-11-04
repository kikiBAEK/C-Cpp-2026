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
int gameOver = 0;
int viewing = 0;
int lastR = -1, lastC = -1;
int moveCount = 0;
int hoverR = -1, hoverC = -1;
std::vector<std::pair<int, int>> history;

std::mt19937 rng((unsigned)time(0));

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

void RedrawBoard(int showNumbers = 0) {
    BeginBatchDraw();
    cleardevice();
    setbkcolor(RGB(28, 28, 38));
    cleardevice();
    DrawBoard();
    for (int i = 0; i < (int)history.size(); i++) {
        int r = history[i].first, c = history[i].second;
        DrawPiece(r, c, board[r][c], showNumbers ? i + 1 : -1);
    }
    if (!gameOver && hoverR >= 0 && hoverC >= 0 && board[hoverR][hoverC] == EMPTY)
        DrawGhost(hoverR, hoverC, playerColor);

    setbkmode(TRANSPARENT);
    setfillcolor(RGB(70, 70, 80));
    solidroundrect(40, BOARD_PX + 10, 180, BOARD_PX + 50, 8, 8);
    settextcolor(WHITE);
    settextstyle(16, 0, L"微软雅黑");
    DrawCenteredText(110, BOARD_PX + 30, L"撤回");

    solidroundrect(BOARD_PX - 180, BOARD_PX + 10, BOARD_PX - 40, BOARD_PX + 50, 8, 8);
    DrawCenteredText(BOARD_PX - 110, BOARD_PX + 30, L"回到菜单");
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

void AI_Easy(int& outR, int& outC) {
    int best = -1;
    std::vector<std::pair<int, int>> cands;
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (board[r][c]) continue;
            if (IsForbidden(r, c, aiColor)) continue;
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
            if (IsForbidden(mr, mc, role)) continue;
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
    for (r = 0; r < GRID; r++)
        for (c = 0; c < GRID; c++)
            if (!board[r][c] && !IsForbidden(r, c, aiColor)) {
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
            if (board[r][c] || IsForbidden(r, c, aiColor)) continue;
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
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (simBoard[r][c]) continue;
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
    Sleep(400);
    int r = 9, c = 9;
    if (moveCount == 0 && aiColor == STONE_BLACK) {
        r = 9; c = 9;
    }
    else {
        if (difficulty == 1) AI_Easy(r, c);
        else if (difficulty == 2) AI_Medium(r, c);
        else AI_Hard(r, c);
    }
    board[r][c] = aiColor;
    history.push_back(std::make_pair(r, c));
    lastR = r; lastC = c;
    moveCount++;
    hoverR = hoverC = -1;
    RedrawBoard();
    Sleep(300);
    if (IsForbidden(r, c, aiColor)) {
        MessageBox(GetHWnd(), L"黑棋禁手，白棋获胜！", L"游戏结束", MB_OK);
        gameOver = 1; viewing = 1; RedrawBoard(1); return;
    }
    if (CheckFive(r, c, aiColor)) {
        MessageBox(GetHWnd(), L"AI 获胜！", L"游戏结束", MB_OK);
        gameOver = 1; viewing = 1; RedrawBoard(1);
    }
}

void DrawMenu() {
    cleardevice();
    setbkcolor(RGB(28, 28, 38));
    cleardevice();
    setbkmode(TRANSPARENT);
    settextcolor(RGB(245, 225, 190));
    settextstyle(42, 0, L"微软雅黑");
    DrawCenteredText(BOARD_PX / 2, 50, L"五子棋 · 人机对战");

    settextstyle(22, 0, L"微软雅黑");
    settextcolor(RGB(210, 210, 210));
    outtextxy(70, 110, L"难度：");
    outtextxy(70, 220, L"执子：");

    auto drawBtn = [](int x, int y, int w, int h, const wchar_t* text, int sel) {
        setfillcolor(sel ? RGB(50, 130, 210) : RGB(48, 48, 58));
        solidroundrect(x, y, x + w, y + h, 10, 10);
        settextcolor(WHITE);
        settextstyle(20, 0, L"微软雅黑");
        DrawCenteredText(x + w / 2, y + h / 2, text);
        };

    drawBtn(70, 140, 150, 48, L"简单", difficulty == 1);
    drawBtn(250, 140, 150, 48, L"中等", difficulty == 2);
    drawBtn(430, 140, 150, 48, L"困难", difficulty == 3);

    drawBtn(70, 250, 150, 48, L"黑棋", playerColor == STONE_BLACK);
    drawBtn(250, 250, 150, 48, L"白棋", playerColor == STONE_WHITE);
    drawBtn(430, 250, 150, 48, L"随机", playerColor == 0);

    setfillcolor(RGB(48, 48, 58));
    solidroundrect(70, 330, 110, 370, 6, 6);
    if (useForbidden) {
        setlinecolor(RGB(80, 180, 100));
        setlinestyle(PS_SOLID, 3);
        line(80, 350, 95, 365);
        line(95, 365, 105, 335);
    }
    settextcolor(RGB(210, 210, 210));
    settextstyle(20, 0, L"微软雅黑");
    outtextxy(125, 340, L"启用禁手（黑棋三三、四四、长连）");

    setfillcolor(RGB(210, 65, 45));
    solidroundrect(220, 410, 440, 470, 12, 12);
    settextcolor(WHITE);
    settextstyle(26, 0, L"微软雅黑");
    DrawCenteredText(330, 440, L"开 始");

    setfillcolor(RGB(80, 80, 90));
    solidroundrect(220, 490, 440, 540, 10, 10);
    settextstyle(20, 0, L"微软雅黑");
    DrawCenteredText(330, 515, L"退 出");

    settextstyle(15, 0, L"微软雅黑");
    settextcolor(RGB(140, 140, 140));
    DrawCenteredText(BOARD_PX / 2, 580, L"");
}

void RunMenu() {
    difficulty = 1;
    playerColor = STONE_BLACK;
    useForbidden = 0;
    int needRedraw = 1;
    while (1) {
        if (needRedraw) { DrawMenu(); needRedraw = 0; }
        ExMessage msg;
        if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN) {
            int x = msg.x, y = msg.y;
            int changed = 0;
            if (y >= 140 && y <= 188) {
                if (x >= 70 && x <= 220 && difficulty != 1) { difficulty = 1; changed = 1; }
                else if (x >= 250 && x <= 400 && difficulty != 2) { difficulty = 2; changed = 1; }
                else if (x >= 430 && x <= 580 && difficulty != 3) { difficulty = 3; changed = 1; }
            }
            if (y >= 250 && y <= 298) {
                if (x >= 70 && x <= 220 && playerColor != STONE_BLACK) { playerColor = STONE_BLACK; changed = 1; }
                else if (x >= 250 && x <= 400 && playerColor != STONE_WHITE) { playerColor = STONE_WHITE; changed = 1; }
                else if (x >= 430 && x <= 580 && playerColor != 0) { playerColor = 0; changed = 1; }
            }
            if (x >= 70 && x <= 110 && y >= 330 && y <= 370) {
                useForbidden = !useForbidden;
                changed = 1;
            }
            if (x >= 220 && x <= 440 && y >= 410 && y <= 470) {
                if (playerColor == 0) playerColor = (rng() % 2 == 0) ? STONE_BLACK : STONE_WHITE;
                aiColor = 3 - playerColor;
                return;
            }
            if (x >= 220 && x <= 440 && y >= 490 && y <= 540) { closegraph(); exit(0); }
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
    RedrawBoard();
}

void RunGame() {
    memset(board, 0, sizeof(board));
    history.clear();
    lastR = lastC = -1;
    moveCount = 0;
    gameOver = 0;
    viewing = 0;
    hoverR = hoverC = -1;
    RedrawBoard();
    if (playerColor == STONE_WHITE) { Sleep(500); DoAIMove(); }
    ExMessage msg;
    int lastHoverTime = 0;
    while (1) {
        if (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_MOUSEMOVE && !gameOver) {
                int now = GetTickCount();
                if (now - lastHoverTime < 30) continue; // 限制刷新频率
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
                    if (x >= BOARD_PX - 180 && x <= BOARD_PX - 40) return;
                }
                if (gameOver) continue;
                int c = (x - MARGIN + CELL / 2) / CELL;
                int r = (y - MARGIN + CELL / 2) / CELL;
                if (r < 0 || r >= GRID || c < 0 || c >= GRID || board[r][c] != EMPTY) continue;
                if (moveCount == 0 && playerColor == STONE_BLACK && (r != 9 || c != 9)) {
                    MessageBox(GetHWnd(), L"黑棋第一手必须下在天元！", L"提示", MB_OK);
                    continue;
                }
                board[r][c] = playerColor;
                if (IsForbidden(r, c, playerColor)) {
                    board[r][c] = EMPTY;
                    MessageBox(GetHWnd(), L"禁手！你输了", L"游戏结束", MB_OK);
                    gameOver = 1; viewing = 1; RedrawBoard(1); continue;
                }
                history.push_back(std::make_pair(r, c));
                lastR = r; lastC = c;
                moveCount++;
                hoverR = hoverC = -1;
                RedrawBoard();
                if (CheckFive(r, c, playerColor)) {
                    MessageBox(GetHWnd(), L"恭喜你获胜！", L"游戏结束", MB_OK);
                    gameOver = 1; viewing = 1; RedrawBoard(1); continue;
                }
                DoAIMove();
            }
        }
        Sleep(5);
    }
}

int main() {
    initgraph(BOARD_PX, WIN_H);
    SetWindowText(GetHWnd(), L"五子棋 · 人机对战");
    while (1) { RunMenu(); RunGame(); }
    closegraph();
    return 0;
}