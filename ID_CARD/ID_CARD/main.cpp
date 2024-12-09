#include <iostream>
#include <fstream>
#include <sqlite3.h>
#include <codecvt>
#include <string>

static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

// 回调函数，用于将查询结果写入 CSV 文件
int callback(void* data, int argc, char** argv, char** azColName) {
    std::ofstream* csv_file = (std::ofstream*)data;

    for (int i = 0; i < argc; i++) {
        *csv_file << (argv[i] ? argv[i] : "NULL");
        if (i < argc - 1) {
            *csv_file << ",";
        }
    }
    *csv_file << "\n";

    return 0;
}

int create_table(sqlite3* db) {
    const char* create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS person_info (
            id_card_number TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            gender TEXT,
            birth_date TEXT,
            address TEXT,
            phone TEXT,
        );
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return rc;
    }
    return SQLITE_OK;
}

int get_person_info(sqlite3* db, const std::string& id_card_number) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name, gender, birth_date, address, phone FROM person_info WHERE id_card_number = ?;";

    // 准备 SQL 语句
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // 绑定查询参数（身份证号码）
    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // 获取查询结果
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* birth_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // 输出查询结果
        std::cout << "Name: " << converter.from_bytes(name).c_str() << std::endl;
        std::cout << "Gender: " << converter.from_bytes(gender).c_str() << std::endl;
        std::cout << "Birth Date: " << converter.from_bytes(birth_date).c_str() << std::endl;
        std::cout << "Address: " << converter.from_bytes(address).c_str() << std::endl;
        std::cout << "Phone: " << converter.from_bytes(phone).c_str() << std::endl;
    }
    else {
        std::cerr << "No record found for ID card number: " << id_card_number << std::endl;
    }

    // 释放 SQL 语句资源
    sqlite3_finalize(stmt);

    return rc;
}

int insert_person_info(sqlite3* db, const std::string& id_card_number, const std::string& name, const std::string& gender,
    const std::string& birth_date, const std::string& address, const std::string& phone) {
    const char* insert_sql = R"(
        INSERT INTO person_info (id_card_number, name, gender, birth_date, address, phone)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, gender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, birth_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, phone.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

int query_person_info(sqlite3* db, const std::string& id_card_number) {
    const char* select_sql = R"(
        SELECT * FROM person_info WHERE id_card_number = ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
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

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

int update_person_info(sqlite3* db, const std::string& id_card_number, const std::string& name, const std::string& phone) {
    const char* update_sql = R"(
        UPDATE person_info SET name = ?, phone = ?, update_time = CURRENT_TIMESTAMP WHERE id_card_number = ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, id_card_number.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

int delete_person_info(sqlite3* db, const std::string& id_card_number) {
    const char* delete_sql = R"(
        DELETE FROM person_info WHERE id_card_number = ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, id_card_number.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

int main() {
    sqlite3 *db;
    char* errMsg = nullptr;
    int rc = sqlite3_open("DataBase/person_info.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return(0);
    }

    if (rc != SQLITE_OK) {
        std::cerr << "SQL 执行失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    std::ofstream csv_file("DataBase/person_info.csv", std::ios::out | std::ios::binary);
    if (!csv_file.is_open()) {
        std::cerr << "无法创建 CSV 文件" << std::endl;
        sqlite3_close(db);
        return 1;
    }

    //插入数据

    /*std::wstring id_card_number, name, gender, birth_date, address, phone;
    std::wcin >> id_card_number >> name >> gender >> birth_date >> address >> phone;

    insert_person_info(db, converter.to_bytes(id_card_number), converter.to_bytes(name),
        converter.to_bytes(gender), converter.to_bytes(birth_date), converter.to_bytes(address), converter.to_bytes(phone));*/

    // 查询数据
    // insert_person_info(db, "232303200503177016","苏航", "Male", "2005-03-17", "华南理工大学", "123456");
    query_person_info(db, "873557964055422531");


    // 写入 UTF-8 BOM（字节顺序标记）
    unsigned char utf8_bom[3] = { 0xEF, 0xBB, 0xBF };
    csv_file.write(reinterpret_cast<char*>(utf8_bom), 3);

    // 查询数据库
    const char* sql = "SELECT * FROM person_info";  // 你需要导出的表的名字
    rc = sqlite3_exec(db, sql, callback, (void*)&csv_file, &errMsg);

    sqlite3_close(db);
    csv_file.close();

    return 0;
}
