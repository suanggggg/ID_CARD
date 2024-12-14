#include <iostream>
#include <fstream>
#include <sqlite3.h>  // SQLite3 库
#include <codecvt>    // 用于 UTF-8 和宽字符（wchar_t）之间的转换
#include <string>
#include <chrono>     // 用于性能计时
#include <functional> // 用于 std::function

// 转换器，用于将 UTF-8 字符串转换为宽字符（wchar_t）
static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

// 回调函数，用于将查询结果写入 CSV 文件
int callback(void* data, int argc, char** argv, char** azColName) {
    std::ofstream* csv_file = (std::ofstream*)data; // 将回调数据转换为文件流

    // 遍历查询结果的每一列，将每列数据写入 CSV 文件
    for (int i = 0; i < argc; i++) {
        *csv_file << (argv[i] ? argv[i] : "NULL");  // 如果列值存在，写入值，否则写入 "NULL"
        if (i < argc - 1) {
            *csv_file << ",";  // 各列之间用逗号分隔
        }
    }
    *csv_file << "\n"; // 每一行数据结束后换行

    return 0;  // 回调函数返回 0 表示成功
}

// 创建数据库表格，用于存储个人信息
int create_table(sqlite3* db) {
    // SQL 创建表格语句
    const char* create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS person_info (
            id_card_number TEXT PRIMARY KEY,  // 身份证号码作为主键
            name TEXT NOT NULL,               // 姓名字段，不能为空
            gender TEXT,                      // 性别字段
            birth_date TEXT,                  // 出生日期字段
            address TEXT,                     // 地址字段
            phone TEXT,                       // 电话字段
        );
    )";

    char* err_msg = nullptr;  // 错误消息
    int rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);  // 执行 SQL 语句
    if (rc != SQLITE_OK) {  // 如果 SQL 执行失败，打印错误信息
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);  // 释放错误消息
        return rc;
    }
    return SQLITE_OK;  // 表格创建成功
}

// 根据身份证号码查询个人信息
int get_person_info(sqlite3* db, const std::string& id_card_number) {
    sqlite3_stmt* stmt;  // SQL 语句
    const char* sql = "SELECT name, gender, birth_date, address, phone FROM person_info WHERE id_card_number = ?;";  // 查询 SQL 语句

    // 准备 SQL 语句
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // 绑定查询条件（身份证号码）
    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // 如果查询成功，输出查询结果
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* birth_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // 将查询结果转换为宽字符并打印
        std::cout << "Name: " << converter.from_bytes(name).c_str() << std::endl;
        std::cout << "Gender: " << converter.from_bytes(gender).c_str() << std::endl;
        std::cout << "Birth Date: " << converter.from_bytes(birth_date).c_str() << std::endl;
        std::cout << "Address: " << converter.from_bytes(address).c_str() << std::endl;
        std::cout << "Phone: " << converter.from_bytes(phone).c_str() << std::endl;
    }
    else {
        std::cerr << "No record found for ID card number: " << id_card_number << std::endl;
    }

    sqlite3_finalize(stmt);  // 释放 SQL 语句资源

    return rc;
}

// 插入个人信息到数据库
int insert_person_info(sqlite3* db, const std::string& id_card_number, const std::string& name, const std::string& gender,
    const std::string& birth_date, const std::string& address, const std::string& phone) {

    const char* insert_sql = R"(
        INSERT INTO person_info (id_card_number, name, gender, birth_date, address, phone)
        VALUES (?, ?, ?, ?, ?, ?);
    )";  // 插入数据的 SQL 语句

    sqlite3_stmt* stmt;  // SQL 语句
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);  // 准备 SQL 语句
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // 绑定插入数据
    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, gender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, birth_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, phone.c_str(), -1, SQLITE_STATIC);

    // 执行插入操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {  // 如果执行失败，输出错误信息
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);  // 释放 SQL 语句资源
    return SQLITE_OK;  // 插入成功
}

// 查询个人信息（与 get_person_info 函数相似）
int query_person_info(sqlite3* db, const std::string& id_card_number) {
    const char* select_sql = R"(
        SELECT * FROM person_info WHERE id_card_number = ?;
    )";  // 查询 SQL 语句

    sqlite3_stmt* stmt;  // SQL 语句
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);  // 准备 SQL 语句
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);  // 绑定查询条件

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // 如果查询成功，输出查询结果
        std::cout << "ID Card Number: " << sqlite3_column_text(stmt, 0) << std::endl;
        std::cout << "Name: " << sqlite3_column_text(stmt, 1) << std::endl;
        std::cout << "Gender: " << sqlite3_column_text(stmt, 2) << std::endl;
        std::cout << "Birth Date: " << sqlite3_column_text(stmt, 3) << std::endl;
        std::cout << "Address: " << sqlite3_column_text(stmt, 4) << std::endl;
        std::cout << "Phone: " << sqlite3_column_text(stmt, 5) << std::endl;
    }
    else {
        std::cout << "No record found with this ID card number!" << std::endl;
    }

    sqlite3_finalize(stmt);  // 释放 SQL 语句资源
    return SQLITE_OK;  // 查询成功
}

// 更新个人信息
int update_person_info(sqlite3* db, const std::string& id_card_number, const std::string& name, const std::string& phone) {
    const char* update_sql = R"(
        UPDATE person_info SET name = ?, phone = ?, update_time = CURRENT_TIMESTAMP WHERE id_card_number = ?;
    )";  // 更新 SQL 语句

    sqlite3_stmt* stmt;  // SQL 语句
    int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, 0);  // 准备 SQL 语句
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // 绑定更新数据
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, id_card_number.c_str(), -1, SQLITE_STATIC);

    // 执行更新操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {  // 如果执行失败，输出错误信息
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);  // 释放 SQL 语句资源
    return SQLITE_OK;  // 更新成功
}

// 删除个人信息
int delete_person_info(sqlite3* db, const std::string& id_card_number) {
    const char* delete_sql = R"(
        DELETE FROM person_info WHERE id_card_number = ?;
    )";  // 删除 SQL 语句

    sqlite3_stmt* stmt;  // SQL 语句
    int rc = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0);  // 准备 SQL 语句
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);  // 绑定删除条件

    // 执行删除操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {  // 如果执行失败，输出错误信息
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);  // 释放 SQL 语句资源
    return SQLITE_OK;  // 删除成功
}

// 测量某个操作的执行时间
void measureExecutionTime(const std::string& operation, const std::function<void()>& func) {
    auto start = std::chrono::high_resolution_clock::now();  // 获取当前时间
    func();  // 执行传入的操作
    auto end = std::chrono::high_resolution_clock::now();  // 获取操作结束时间
    std::chrono::duration<double> duration = end - start;  // 计算时间差
    std::cout << operation << " time: " << duration.count() << " s" << std::endl;  // 输出执行时间
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();  // 记录程序开始时间

    sqlite3* db;  // SQLite3 数据库对象
    char* errMsg = nullptr;  // 错误消息
    int rc = sqlite3_open("DataBase/person_info.db", &db);  // 打开数据库
    if (rc) {  // 如果打开失败，输出错误信息
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return(0);
    }

    if (rc != SQLITE_OK) {
        std::cerr << "SQL 执行失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);  // 释放错误消息
    }

    std::ofstream csv_file("DataBase/person_info.csv", std::ios::out | std::ios::binary);  // 打开 CSV 文件
    if (!csv_file.is_open()) {  // 如果文件打开失败，输出错误信息
        std::cerr << "无法创建 CSV 文件" << std::endl;
        sqlite3_close(db);  // 关闭数据库
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();  // 获取结束时间
    std::chrono::duration<double> duration = end - start;  // 计算数据库打开时间
    std::cout << "open db time: " << duration.count() << " s" << std::endl;

    // 插入数据并测量执行时间
    measureExecutionTime("insert", [&]() { insert_person_info(db, "232303200503177016", "苏航", "Male", "2005-03-17", "华南理工大学", "123456"); });

    // 查询数据并测量执行时间
    measureExecutionTime("find", [&]() { query_person_info(db, "917529489550438371"); });

    // 向 CSV 文件写入 UTF-8 BOM（字节顺序标记）
    unsigned char utf8_bom[3] = { 0xEF, 0xBB, 0xBF };
    csv_file.write(reinterpret_cast<char*>(utf8_bom), 3);  // 写入 BOM

    // 查询数据库并将结果写入 CSV 文件
    const char* sql = "SELECT * FROM person_info";  // 要导出的 SQL 语句
    rc = sqlite3_exec(db, sql, callback, (void*)&csv_file, &errMsg);  // 执行 SQL 查询并将结果写入 CSV 文件

    sqlite3_close(db);  // 关闭数据库连接
    csv_file.close();   // 关闭 CSV 文件

    return 0;  // 程序执行完毕
}
