#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

// ============================================================
// 功能要求（一）：基本 int 安全数组（用 get / set）
// ============================================================
class SafeArrayBasic {
private:
    int* data;
    int size;

public:
    SafeArrayBasic(int n) {
        if (n <= 0) {
            cout << "[Basic] Error: size must be positive!" << endl;
            exit(1);
        }
        size = n;
        data = new int[size] {};
    }

    ~SafeArrayBasic() {
        delete[] data;
    }

    SafeArrayBasic(const SafeArrayBasic&) = delete;
    SafeArrayBasic& operator=(const SafeArrayBasic&) = delete;

    int get(int index) const {
        if (index < 0 || index >= size) {
            cout << "[Basic] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        return data[index];
    }

    void set(int index, int value) {
        if (index < 0 || index >= size) {
            cout << "[Basic] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        data[index] = value;
    }

    int getSize() const { return size; }

    void print() const {
        cout << "[ ";
        for (int i = 0; i < size; i++) {
            cout << data[i];
            if (i < size - 1) cout << ", ";
        }
        cout << " ]" << endl;
    }
};

// ============================================================
// 功能要求（二）：运算符重载版（支持 arr[i]）
// ============================================================
class SafeArrayOp {
private:
    int* data;
    int size;

public:
    SafeArrayOp(int n) {
        if (n <= 0) {
            cout << "[Op] Error: size must be positive!" << endl;
            exit(1);
        }
        size = n;
        data = new int[size] {};
    }

    ~SafeArrayOp() {
        delete[] data;
    }

    SafeArrayOp(const SafeArrayOp&) = delete;
    SafeArrayOp& operator=(const SafeArrayOp&) = delete;

    // 可写版本
    int& operator[](int index) {
        if (index < 0 || index >= size) {
            cout << "[Op] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        return data[index];
    }

    // 只读版本
    const int& operator[](int index) const {
        if (index < 0 || index >= size) {
            cout << "[Op] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        return data[index];
    }

    int getSize() const { return size; }

    void print() const {
        cout << "[ ";
        for (int i = 0; i < size; i++) {
            cout << data[i];
            if (i < size - 1) cout << ", ";
        }
        cout << " ]" << endl;
    }
};

// ============================================================
// 功能要求（三）：模板版（类型无关）
// ============================================================
template <typename T>
class SafeArray {
private:
    T* data;
    int size;

public:
    SafeArray(int n) {
        if (n <= 0) {
            cout << "[Template] Error: size must be positive!" << endl;
            exit(1);
        }
        size = n;
        data = new T[size]{};
    }

    ~SafeArray() {
        delete[] data;
    }

    SafeArray(const SafeArray&) = delete;
    SafeArray& operator=(const SafeArray&) = delete;

    T& operator[](int index) {
        if (index < 0 || index >= size) {
            cout << "[Template] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        return data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= size) {
            cout << "[Template] Error: index " << index
                << " out of range [0, " << size - 1 << "]!" << endl;
            exit(1);
        }
        return data[index];
    }

    int getSize() const { return size; }

    void print() const {
        cout << "[ ";
        for (int i = 0; i < size; i++) {
            cout << data[i];
            if (i < size - 1) cout << ", ";
        }
        cout << " ]" << endl;
    }
};

// ============================================================
// 测试主函数
// ============================================================
int main() {
    cout << "========== 1. Basic Version (get/set) ==========" << endl;
    SafeArrayBasic arr1(5);
    for (int i = 0; i < 5; i++) {
        arr1.set(i, (i + 1) * 10);
    }
    cout << "Content: ";
    arr1.print();
    cout << "arr1.get(2) = " << arr1.get(2) << endl;
    cout << endl;

    cout << "========== 2. Operator[] Version ==========" << endl;
    SafeArrayOp arr2(5);
    for (int i = 0; i < 5; i++) {
        arr2[i] = (i + 1) * 100;
    }
    cout << "Content: ";
    arr2.print();
    cout << "arr2[3] = " << arr2[3] << endl;
    cout << endl;

    cout << "========== 3. Template Version ==========" << endl;

    // int
    SafeArray<int> arrInt(5);
    for (int i = 0; i < 5; i++) {
        arrInt[i] = (i + 1) * 10;
    }
    cout << "Int array: ";
    arrInt.print();

    // double
    SafeArray<double> arrDouble(4);
    arrDouble[0] = 3.14;
    arrDouble[1] = 2.718;
    arrDouble[2] = 1.414;
    arrDouble[3] = 1.732;
    cout << "Double array: ";
    arrDouble.print();

    // string
    SafeArray<string> arrStr(3);
    arrStr[0] = "Hello";
    arrStr[1] = "Safe";
    arrStr[2] = "Array";
    cout << "String array: ";
    arrStr.print();
    cout << endl;

    // 越界测试（任选一个取消注释即可看到报错）
    // cout << "Testing out-of-bounds access..." << endl;
    // cout << arr1.get(10) << endl;        // Basic 版越界
    // cout << arr2[10] << endl;            // Operator 版越界
    // cout << arrInt[10] << endl;          // Template 版越界

    cout << "All tests finished successfully (no out-of-bounds accessed)." << endl;
    return 0;
}