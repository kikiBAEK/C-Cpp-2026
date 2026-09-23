#include <iostream>
using namespace std;

// 递归函数：把 n 个圆盘从 from 柱移动到 to 柱，借助 aux 柱
void hanoi(int n, char from, char to, char aux) {
    // 递归出口：只剩 1 个圆盘时，直接移动
    if (n == 1) 
    {
        cout << from << " -> " << to << endl;
        return;
    }

    hanoi(n - 1, from, aux, to);

    cout << from << " -> " << to << endl;

    hanoi(n - 1, aux, to, from);
}

int main() {
    int n;
    cout << "输入个数：\n";
    cin >> n;
    cout << "\n移动步骤如下（总共 " << (1LL << n) - 1 << " 步）：\n";
    //pow(2, n) - 1
    hanoi(n, 'A', 'C', 'B');
    return 0;
}