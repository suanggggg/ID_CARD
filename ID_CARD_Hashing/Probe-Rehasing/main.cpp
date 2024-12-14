#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <locale>
#include <chrono>

using namespace std;

// 存储个人信息结构体
struct PersonInfo {
    string name;
    string gender;
    string birthdate;
    string address;
    string phone;

    PersonInfo(const string& n = "", const string& g = "", const string& b = "", const string& a = "", const string& p = "")
        : name(n), gender(g), birthdate(b), address(a), phone(p) {}
};

// 哈希表类
class ExternalHashTable {
private:
    vector<pair<string, PersonInfo>> table; // 存储数据
    vector<bool> occupied; // 标记槽位是否被占用
    size_t table_size; // 当前哈希表大小
    size_t num_elements; // 当前已存储的元素个数
    const double load_factor_threshold = 0.8; // 负载因子阈值

public:
    ExternalHashTable(size_t size = 1024) {
        table_size = size;
        table.resize(table_size, { "", PersonInfo() });
        occupied.resize(table_size, false);
        num_elements = 0;
    }

    // 从CSV文件加载数据
    void loadFromFile(const string& filename) {
        ifstream infile(filename);

        if (!infile.is_open()) {
            cerr << "Failed to open file: " << filename << endl;
            return;
        }

        string line;
        getline(infile, line); // 跳过文件表头

        while (getline(infile, line)) {
            stringstream ss(line);
            string id, name, gender, birthdate, address, phone;

            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, gender, ',');
            getline(ss, birthdate, ',');
            getline(ss, address, ',');
            getline(ss, phone, ',');

            insert(id, name, gender, birthdate, address, phone);
        }
    }

    // 插入数据到哈希表
    void insert(const string& id, const string& name, const string& gender,
        const string& birthdate, const string& address, const string& phone) {
        if ((double)num_elements / table_size > load_factor_threshold) {
            rehash(); // 动态扩容
        }

        size_t index = hashFunction(id);
        size_t original_index = index;
        size_t probe = 1;

        while (occupied[index]) {
            if (table[index].first == id) {
                table[index].second = PersonInfo(name, gender, birthdate, address, phone);
                return;
            }
            index = (original_index + probe * probe) % table_size; // 二次探测
            ++probe;
        }

        table[index] = { id, PersonInfo(name, gender, birthdate, address, phone) };
        occupied[index] = true;
        ++num_elements;
    }

    // 查找身份证号对应的个人信息
    PersonInfo find(const string& id) {
        size_t index = hashFunction(id);
        size_t original_index = index;
        size_t probe = 1;

        while (occupied[index]) {
            if (table[index].first == id) {
                return table[index].second;
            }
            index = (original_index + probe * probe) % table_size; // 二次探测
            ++probe;
        }

        return PersonInfo();
    }

    // 将哈希表保存到文件
    void saveToFile(const string& filename) {
        ofstream outfile(filename);

        if (!outfile.is_open()) {
            cerr << "Failed to open file for writing: " << filename << endl;
            return;
        }

        outfile << "ID,Name,Gender,Birthdate,Address,Phone\n";

        for (size_t i = 0; i < table_size; ++i) {
            if (occupied[i]) {
                outfile << table[i].first << ","
                    << table[i].second.name << ","
                    << table[i].second.gender << ","
                    << table[i].second.birthdate << ","
                    << table[i].second.address << ","
                    << table[i].second.phone << "\n";
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
            hashValue = (hashValue * 31 + c) ^ (hashValue >> 7); // 增加扰动
        }
        return hashValue % table_size;
    }

    // 动态扩容哈希表
    void rehash() {
        size_t new_size = table_size * 2;
        vector<pair<string, PersonInfo>> new_table(new_size, { "", PersonInfo() });
        vector<bool> new_occupied(new_size, false);

        for (size_t i = 0; i < table_size; ++i) {
            if (occupied[i]) {
                size_t index = hashFunction(table[i].first) % new_size;
                size_t original_index = index;
                size_t probe = 1;

                while (new_occupied[index]) {
                    index = (original_index + probe * probe) % new_size;
                    ++probe;
                }

                new_table[index] = table[i];
                new_occupied[index] = true;
            }
        }

        table = move(new_table);
        occupied = move(new_occupied);
        table_size = new_size;

        cout << "Rehashed hash table to size: " << table_size << endl;
    }
};

int main() {
    ExternalHashTable hashTable;
    auto start = std::chrono::high_resolution_clock::now();

    // 从CSV文件中加载数据
    hashTable.loadFromFile("person_info.csv");

    // 查找某个身份证号

    auto insert_start = std::chrono::high_resolution_clock::now();
    hashTable.insert("232303200503177016", "苏航", "Male", "2005-03-17", "华南理工大专", "15124681313");
    auto insert_end = std::chrono::high_resolution_clock::now();

    // 计算并输出程序运行时间
    std::chrono::duration<double> insert_duration = insert_end - insert_start;

    std::cout << "insert_time: " << insert_duration.count() << "s" << std::endl;
    string searchId = "232303200503177016";
    PersonInfo person = hashTable.find(searchId);

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

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start - insert_duration;
    cout << "Execution time: " << duration.count() << " seconds" << endl;

    // 保存哈希表数据到文件
    hashTable.saveToFile("person_info.csv");

    return 0;
}
