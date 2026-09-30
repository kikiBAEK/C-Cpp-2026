#include <iostream>
#include <vector>
#include <string>
using namespace std;

// ============================================================
// 基类：Animal
// ============================================================
class Animal {
protected:
    string name;

public:
    Animal(const string& n) : name(n) {}
    virtual ~Animal() {}                    // 虚析构函数，保证正确释放

    // 纯虚函数：每种动物必须实现自己的叫声
    virtual void speak() const = 0;

    string getName() const { return name; }
};

// ============================================================
// 具体动物：Dog、Cat（功能要求一）
// ============================================================
class Dog : public Animal {
public:
    Dog(const string& n) : Animal(n) {}

    void speak() const override {
        cout << name << " (Dog) says: Woof! Woof!" << endl;
    }
};

class Cat : public Animal {
public:
    Cat(const string& n) : Animal(n) {}

    void speak() const override {
        cout << name << " (Cat) says: Meow~ Meow~" << endl;
    }
};

// ============================================================
// 功能要求（二）新增：Bird 和 WolfDog
// 注意：完全不需要修改 Zoo 类的任何代码
// ============================================================
class Bird : public Animal {
public:
    Bird(const string& n) : Animal(n) {}

    void speak() const override {
        cout << name << " (Bird) says: Tweet! Tweet!" << endl;
    }
};

class WolfDog : public Dog {                // 继承自 Dog
public:
    WolfDog(const string& n) : Dog(n) {}

    // 重写叫声，更像狼嚎
    void speak() const override {
        cout << name << " (WolfDog) says: Awoooooo~ Howl!" << endl;
    }
};

// ============================================================
// Zoo 类（功能要求一 + 二都不需要修改）
// ============================================================
class Zoo {
private:
    vector<Animal*> animals;                // 用基类指针存储任意动物

public:
    // 添加动物
    void addAnimal(Animal* animal) {
        if (animal != nullptr) {
            animals.push_back(animal);
        }
    }

    // 让动物园里所有动物依次叫一次
    void makeAllSpeak() const {
        cout << "===== Animals in the Zoo are speaking =====" << endl;
        if (animals.empty()) {
            cout << "The zoo is empty." << endl;
            return;
        }
        for (const auto& animal : animals) {
            animal->speak();                // 多态调用
        }
        cout << "===========================================" << endl;
    }

    // 析构时释放所有动物
    ~Zoo() {
        for (auto animal : animals) {
            delete animal;
        }
        animals.clear();
    }

    // 禁止拷贝（简单处理）
    Zoo(const Zoo&) = delete;
    Zoo& operator=(const Zoo&) = delete;

    Zoo() = default;
};

// ============================================================
// 主函数演示
// ============================================================
int main() {
    Zoo zoo;

    // ---------- 功能要求（一）----------
    cout << "【功能要求一】添加 Dog 和 Cat" << endl;
    zoo.addAnimal(new Dog("Buddy"));
    zoo.addAnimal(new Cat("Kitty"));
    zoo.addAnimal(new Dog("Max"));
    zoo.makeAllSpeak();
    cout << endl;

    // ---------- 功能要求（二）----------
    // 注意：下面新增代码完全不需要改 Zoo 类
    cout << "【功能要求二】增加 Bird 和 WolfDog（无需修改 Zoo 类）" << endl;
    zoo.addAnimal(new Bird("Tweety"));
    zoo.addAnimal(new WolfDog("Alpha"));
    zoo.addAnimal(new Bird("Rio"));
    zoo.makeAllSpeak();

    return 0;
}