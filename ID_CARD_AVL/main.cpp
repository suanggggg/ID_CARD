#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <algorithm>

using namespace std;

// 人员信息结构
struct PersonInfo {
    string id_card;
    string name;
    string gender;
    string birth_date;
    string address;
    string phone;

    PersonInfo(string id_card, string name, string gender, string birth_date, string address, string phone)
        : id_card(move(id_card)), name(move(name)), gender(move(gender)),
        birth_date(move(birth_date)), address(move(address)), phone(move(phone)) {}
};

// AVL树节点结构
struct AVLNode {
    string key;
    PersonInfo value;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(string k, PersonInfo val)
        : key(move(k)), value(move(val)), height(1), left(nullptr), right(nullptr) {}
};

// AVL树类
class AVLTree {
public:
    AVLTree() : root(nullptr) {}

    void insert(const string& key, const PersonInfo& value) {
        root = insert(root, key, value);
    }

    PersonInfo* search(const string& key) {
        AVLNode* node = search(root, key);
        return node ? &node->value : nullptr;
    }

    void deleteNode(const string& key) {
        root = deleteNode(root, key);
    }

private:
    AVLNode* root;

    int height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int getBalance(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        x->right = y->left;
        y->left = x;

        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;

        return y;
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        y->left = x->right;
        x->right = y;

        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;

        return x;
    }

    AVLNode* insert(AVLNode* node, const string& key, const PersonInfo& value) {
        if (!node) return new AVLNode(key, value);

        if (key < node->key) {
            node->left = insert(node->left, key, value);
        }
        else if (key > node->key) {
            node->right = insert(node->right, key, value);
        }
        else {
            return node;
        }

        node->height = 1 + max(height(node->left), height(node->right));
        return balanceNode(node, key);
    }

    AVLNode* balanceNode(AVLNode* node, const string& key) {
        int balance = getBalance(node);

        if (balance > 1 && key < node->left->key)
            return rightRotate(node);

        if (balance < -1 && key > node->right->key)
            return leftRotate(node);

        if (balance > 1 && key > node->left->key) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && key < node->right->key) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    AVLNode* search(AVLNode* node, const string& key) {
        if (!node || node->key == key)
            return node;

        return key < node->key ? search(node->left, key) : search(node->right, key);
    }

    AVLNode* findMin(AVLNode* node) {
        while (node && node->left)
            node = node->left;
        return node;
    }

    AVLNode* deleteNode(AVLNode* node, const string& key) {
        if (!node) return nullptr;

        if (key < node->key) {
            node->left = deleteNode(node->left, key);
        }
        else if (key > node->key) {
            node->right = deleteNode(node->right, key);
        }
        else {
            if (!node->left || !node->right) {
                AVLNode* temp = node->left ? node->left : node->right;
                delete node;
                return temp;
            }

            AVLNode* temp = findMin(node->right);
            node->key = temp->key;
            node->value = temp->value;
            node->right = deleteNode(node->right, temp->key);
        }

        node->height = 1 + max(height(node->left), height(node->right));
        return balanceNode(node, key);
    }
};

void loadDataFromCSV(const string& filename, AVLTree& tree) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string id_card, name, gender, birth_date, address, phone;

        getline(ss, id_card, ',');
        getline(ss, name, ',');
        getline(ss, gender, ',');
        getline(ss, birth_date, ',');
        getline(ss, address, ',');
        getline(ss, phone, ',');

        PersonInfo person(id_card, name, gender, birth_date, address, phone);
        tree.insert(id_card, person);
    }
}

void measureExecutionTime(const string& operation, const function<void()>& func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << operation << " time: " << duration.count() << " s" << endl;
}

int main() {
    AVLTree tree;

    string csv_filename = "DataBase/person_info.csv";
    measureExecutionTime("Load data", [&]() { loadDataFromCSV(csv_filename, tree); });

    PersonInfo person("232303200503177016", "哈哈哈", "Male", "1990-01-01", "Some Address", "12345678900");
    measureExecutionTime("Insert", [&]() { tree.insert(person.id_card, person); });

    string search_id = "233110563766882389";
    measureExecutionTime("Search", [&]() {
        PersonInfo* foundPerson = tree.search(search_id);
        if (foundPerson) {
            cout << "Found person: " << foundPerson->name << endl;
        }
        else {
            cout << "Person not found." << endl;
        }
        });

    return 0;
}
