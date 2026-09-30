#include <iostream>
using namespace std;

// 链表节点
struct ListNode {
    int value;
    ListNode* next;

    ListNode(int val) : value(val), next(nullptr) {}
};

// ==================== 功能函数 ====================

// 1. 创建链表（这里用数组初始化，方便测试）
ListNode* createList(const int arr[], int n) {
    if (n <= 0) return nullptr;

    ListNode* head = new ListNode(arr[0]);
    ListNode* curr = head;

    for (int i = 1; i < n; i++) {
        curr->next = new ListNode(arr[i]);
        curr = curr->next;
    }
    return head;
}

// 2. 遍历链表，依次显示各节点的 value
void printList(ListNode* head) {
    ListNode* curr = head;
    cout << "链表内容: ";
    while (curr != nullptr) {
        cout << curr->value;
        if (curr->next != nullptr) cout << " -> ";
        curr = curr->next;
    }
    cout << " -> nullptr" << endl;
}

// 3. 反转链表（经典三指针法）
ListNode* reverseList(ListNode* head) {
    ListNode* prev = nullptr;
    ListNode* curr = head;
    ListNode* next = nullptr;

    while (curr != nullptr) {
        next = curr->next;   // 先保存下一个节点
        curr->next = prev;   // 反转指针
        prev = curr;         // prev 前进
        curr = next;         // curr 前进
    }
    return prev;             // prev 成为新的头节点
}

// 4 & 5. 查找值为 target 的节点
// 返回该节点的序号（从 1 开始），找不到返回 -1
// 参数 startFrom：从这个节点之后开始找（用于找“下一个”）
int findValue(ListNode* head, int target, ListNode* startFrom = nullptr) {
    ListNode* curr = (startFrom == nullptr) ? head : startFrom->next;
    int index = 1;

    // 如果指定了 startFrom，需要先计算出它的序号
    if (startFrom != nullptr) {
        ListNode* temp = head;
        index = 1;
        while (temp != startFrom && temp != nullptr) {
            temp = temp->next;
            index++;
        }
        index++; // 从 startFrom 的下一个开始
        curr = startFrom->next;
    }

    while (curr != nullptr) {
        if (curr->value == target) {
            return index;
        }
        curr = curr->next;
        index++;
    }
    return -1;
}

// 更清晰的查找版本：返回节点指针 + 序号
// 找到第一个值为 5 的节点
pair<ListNode*, int> findFirst(ListNode* head, int target) {
    ListNode* curr = head;
    int index = 1;
    while (curr != nullptr) {
        if (curr->value == target) {
            return { curr, index };
        }
        curr = curr->next;
        index++;
    }
    return { nullptr, -1 };
}

// 从某个节点之后继续找下一个值为 target 的节点
pair<ListNode*, int> findNext(ListNode* head, ListNode* from, int target) {
    if (from == nullptr) return { nullptr, -1 };

    // 先算出 from 的序号
    int index = 1;
    ListNode* temp = head;
    while (temp != from && temp != nullptr) {
        temp = temp->next;
        index++;
    }

    // 从 from 的下一个开始找
    ListNode* curr = from->next;
    index++;

    while (curr != nullptr) {
        if (curr->value == target) {
            return { curr, index };
        }
        curr = curr->next;
        index++;
    }
    return { nullptr, -1 };
}

// 释放链表内存
void deleteList(ListNode* head) {
    while (head != nullptr) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// ==================== main 函数 ====================
int main() {
    // 1. 在 main 中创建一个单向链表
    // 示例数据：1 -> 5 -> 3 -> 5 -> 7 -> 5 -> 9
    int arr[] = { 1, 5, 3, 5, 7, 5, 9 };
    int n = sizeof(arr) / sizeof(arr[0]);

    ListNode* head = createList(arr, n);

    cout << "【1】创建链表成功" << endl;
    printList(head);
    cout << endl;

    // 2. 遍历显示（上面已经做了）
    cout << "【2】遍历链表完成" << endl << endl;

    // 3. 反转链表
    head = reverseList(head);
    cout << "【3】反转后的链表:" << endl;
    printList(head);
    cout << endl;

    // 为了后面查找方便，我们再反转回来（也可以直接在反转后的链表上查找）
    // 这里为了演示“查找第一个5”和“下一个5”，我们重新创建原链表
    deleteList(head);
    head = createList(arr, n);

    cout << "重新创建原链表用于查找演示:" << endl;
    printList(head);
    cout << endl;

    // 4. 查找第一个值为 5 的节点
    auto [firstNode, firstIndex] = findFirst(head, 5);
    cout << "【4】查找第一个值为 5 的节点: ";
    if (firstIndex != -1) {
        cout << "找到，序号 = " << firstIndex << endl;
    }
    else {
        cout << "未找到，返回 -1" << endl;
    }

    // 5. 查找下一个值为 5 的节点
    auto [nextNode, nextIndex] = findNext(head, firstNode, 5);
    cout << "【5】查找下一个值为 5 的节点: ";
    if (nextIndex != -1) {
        cout << "找到，序号 = " << nextIndex << endl;
    }
    else {
        cout << "未找到，返回 -1" << endl;
    }

    // 再找一次下一个
    auto [nextNextNode, nextNextIndex] = findNext(head, nextNode, 5);
    cout << "继续查找再下一个值为 5 的节点: ";
    if (nextNextIndex != -1) {
        cout << "找到，序号 = " << nextNextIndex << endl;
    }
    else {
        cout << "未找到，返回 -1" << endl;
    }

    // 释放内存
    deleteList(head);

    cout << "\n程序结束，内存已释放。" << endl;
    return 0;
}