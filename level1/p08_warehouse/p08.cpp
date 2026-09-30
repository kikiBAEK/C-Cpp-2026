#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

struct Goods {
    string model;   // product model
    int quantity;   // stock quantity
};

class Inventory {
private:
    vector<Goods> goodsList;
    const string filename = "inventory.txt";

public:
    // Load data when program starts
    void loadFromFile() {
        ifstream file(filename);
        if (!file.is_open()) {
            return;  // file not exist yet, start with empty inventory
        }

        goodsList.clear();
        string model;
        int qty;
        while (file >> model >> qty) {
            goodsList.push_back({ model, qty });
        }
        file.close();
    }

    // Save data when program exits
    void saveToFile() {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Failed to save file!" << endl;
            return;
        }

        for (const auto& g : goodsList) {
            file << g.model << " " << g.quantity << endl;
        }
        file.close();
        cout << "Inventory data saved to " << filename << endl;
    }

    // Show inventory list
    void showList() {
        system("cls");
        cout << "========== Current Inventory ==========" << endl;
        if (goodsList.empty()) {
            cout << "No items in inventory." << endl;
        }
        else {
            cout << left << setw(20) << "Model" << setw(10) << "Quantity" << endl;
            cout << "--------------------------------------" << endl;
            for (const auto& g : goodsList) {
                cout << left << setw(20) << g.model << setw(10) << g.quantity << endl;
            }
        }
        cout << "======================================" << endl;
        cout << "Press Enter to return to menu...";
        cin.get();
        cin.get();
    }

    // Find goods by model, return index or -1
    int findGoods(const string& model) {
        for (int i = 0; i < (int)goodsList.size(); i++) {
            if (goodsList[i].model == model) {
                return i;
            }
        }
        return -1;
    }

    // Stock In
    void stockIn() {
        system("cls");
        cout << "========== Stock In ==========" << endl;
        string model;
        int qty;

        cout << "Enter product model: ";
        cin >> model;
        cout << "Enter quantity to add: ";
        cin >> qty;

        if (qty <= 0) {
            cout << "Quantity must be greater than 0!" << endl;
            cout << "Press Enter to return...";
            cin.get(); cin.get();
            return;
        }

        int index = findGoods(model);
        if (index != -1) {
            goodsList[index].quantity += qty;
            cout << "Stock in successful! Model [" << model << "] current quantity: "
                << goodsList[index].quantity << endl;
        }
        else {
            goodsList.push_back({ model, qty });
            cout << "New product added! Model [" << model << "] quantity: " << qty << endl;
        }

        cout << "Press Enter to return to menu...";
        cin.get(); cin.get();
    }

    // Stock Out
    void stockOut() {
        system("cls");
        cout << "========== Stock Out ==========" << endl;
        string model;
        int qty;

        cout << "Enter product model: ";
        cin >> model;
        cout << "Enter quantity to remove: ";
        cin >> qty;

        if (qty <= 0) {
            cout << "Quantity must be greater than 0!" << endl;
            cout << "Press Enter to return...";
            cin.get(); cin.get();
            return;
        }

        int index = findGoods(model);
        if (index == -1) {
            cout << "Error: Model [" << model << "] not found in inventory!" << endl;
        }
        else if (goodsList[index].quantity < qty) {
            cout << "Error: Not enough stock! Current quantity: "
                << goodsList[index].quantity << endl;
        }
        else {
            goodsList[index].quantity -= qty;
            cout << "Stock out successful! Model [" << model << "] remaining: "
                << goodsList[index].quantity << endl;
        }

        cout << "Press Enter to return to menu...";
        cin.get(); cin.get();
    }

    // Main menu
    void run() {
        loadFromFile();

        int choice;
        while (true) {
            system("cls");
            cout << "========== Simple Inventory System ==========" << endl;
            cout << "  1. Show Inventory List" << endl;
            cout << "  2. Stock In" << endl;
            cout << "  3. Stock Out" << endl;
            cout << "  4. Exit" << endl;
            cout << "=============================================" << endl;
            cout << "Please select (1-4): ";
            cin >> choice;

            switch (choice) {
            case 1:
                showList();
                break;
            case 2:
                stockIn();
                break;
            case 3:
                stockOut();
                break;
            case 4:
                saveToFile();
                cout << "Thank you for using. Goodbye!" << endl;
                return;
            default:
                cout << "Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.get(); cin.get();
                break;
            }
        }
    }
};

int main() {
    Inventory inv;
    inv.run();
    return 0;
}