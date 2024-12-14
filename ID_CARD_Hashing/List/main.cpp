#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <locale>
#include <codecvt>

using namespace std;

// 存储个人信息结构体
struct PersonInfo {
    string name;
    string gender;
    string birthdate;
    string address;
    string phone;

    PersonInfo(const string& n, const string& g, const string& b, const string& a, const string& p)
        : name(n), gender(g), birthdate(b), address(a), phone(p) {}
};

// 哈希表类
class ExternalHashTable {
private:
    vector<list<pair<string, PersonInfo>>> table;

public:
    ExternalHashTable() {
        table.resize(1024);  // 哈希表大小
    }

    // 从CSV文件加载数据
    void loadFromFile(const string& filename) {
        ifstream infile(filename);  // 直接读取GBK编码的文件

        // 判断文件是否成功打开
        if (!infile.is_open()) {
            cerr << "Failed to open file: " << filename << endl;
            return;
        }

        string line;
        getline(infile, line);  // 跳过文件表头

        // 读取每一行数据
        while (getline(infile, line)) {
            stringstream ss(line);
            string id, name, gender, birthdate, address, phone;

            // 按逗号分隔每一行
            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, gender, ',');
            getline(ss, birthdate, ',');
            getline(ss, address, ',');
            getline(ss, phone, ',');

            // 将数据插入哈希表
            insert(id, name, gender, birthdate, address, phone);
        }
    }

    // 插入数据到哈希表
    void insert(const string& id, const string& name, const string& gender,
        const string& birthdate, const string& address, const string& phone) {
        int index = hashFunction(id);  // 根据身份证号计算哈希值
        table[index].emplace_back(id, PersonInfo(name, gender, birthdate, address, phone));
    }

    // 查找身份证号对应的个人信息
    PersonInfo find(const string& id) {
        int index = hashFunction(id);
        for (const auto& kv : table[index]) {
            if (kv.first == id) {
                return kv.second;
            }
        }
        return PersonInfo("", "", "", "", "");
    }

    // 将哈希表保存到文件
    void saveToFile(const string& filename) {
        ofstream outfile(filename);
        if (!outfile.is_open()) {
            cerr << "Failed to open file for writing: " << filename << endl;
            return;
        }

        // 写入表头
        outfile << "ID,Name,Gender,Birthdate,Address,Phone\n";

        // 遍历哈希表中的每一项
        for (const auto& bucket : table) {
            for (const auto& kv : bucket) {
                // 将身份证号和对应的个人信息保存到文件
                outfile << kv.first << ","
                    << kv.second.name << ","
                    << kv.second.gender << ","
                    << kv.second.birthdate << ","
                    << kv.second.address << ","
                    << kv.second.phone << "\n";
            }
        }

        outfile.close();
        cout << "Data saved to file: " << filename << endl;
    }

private:
    // 简单的哈希函数
    unsigned long hashFunction(const string& key) {
        unsigned long hashValue = 0;
        for (char c : key) {
            hashValue = hashValue * 31 + c;
        }
        return hashValue % 1024;  // 返回桶的索引
    }
};

int main() {
    ExternalHashTable hashTable;

    // 从CSV文件中加载数据
    hashTable.loadFromFile("person_info.csv");

    // 假设我们要查找某个身份证号的个人信息
    string searchId = "851187293638236198";
    PersonInfo person = hashTable.find(searchId);

    // 输出查询结果
    if (!person.name.empty()) {
        cout << "Found person:" << endl;
        cout << "姓名: " << person.name << endl;
        cout << "性别: " << person.gender << endl;
        cout << "生日: " << person.birthdate << endl;
        cout << "地址: " << person.address << endl;
        cout << "电话: " << person.phone << endl;
    }
    else {
        cout << "Person not found!" << endl;
    }

    // 将哈希表中的数据保存到文件
    hashTable.saveToFile("output_person_info.csv");

    return 0;
}
