#include <iostream>
using namespace std;

// 判断一个正整数是否为素数
bool isPrime(int n) {
    if (n <= 1) return false;          // 1 和负数不是素数
    if (n == 2) return true;           // 2 是素数
    if (n % 2 == 0) return false;      // 偶数不是素数

    // 只需要判断到平方根
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

int main() {
    int n;

    cout << "请输入一个正整数 n: ";
    cin >> n;

    if (n <= 0) {
        cout << "请输入正整数！" << endl;
        return 0;
    }

    if (isPrime(n)) {
        cout << n << " 是素数。" << endl;
    }
    else {
        cout << n << " 不是素数。" << endl;
    }

    return 0;
}