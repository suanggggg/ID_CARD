import sqlite3
import csv
from faker import Faker
import random

# 创建一个 Faker 实例
fake = Faker('zh_CN')  # 设置为中文环境

# 连接到 SQLite 数据库（如果文件不存在，会自动创建）
conn = sqlite3.connect('ID_CARD/DataBase/person_info.db')
cursor = conn.cursor()

# 创建表格，去掉 id 列
cursor.execute('''
CREATE TABLE IF NOT EXISTS person_info (
    id_card_number TEXT UNIQUE,
    name TEXT,
    gender TEXT,
    birth_date TEXT,
    address TEXT,
    phone TEXT
)
''')


# 身份证校验码计算函数
def calculate_check_code(id_card_number):
    weight = [7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2]
    check_code_map = ['1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2']

    total = sum(int(id_card_number[i]) * weight[i] for i in range(17))
    check_code = check_code_map[total % 11]
    return check_code


# 生成符合中国11位手机号格式的手机号
def generate_phone_number():
    # 中国手机号一般以 13, 15, 17, 18, 19 开头
    prefix = random.choice(['13', '14', '15', '17', '18', '19'])
    return prefix + ''.join([str(random.randint(0, 9)) for _ in range(9)])


# 将中文数据转换为GBK编码并存储
def convert_to_gbk(text):
    return text.encode('gbk', errors='ignore')  # 编码为GBK，忽略编码错误

# 生成并插入随机数据
def generate_and_insert_data(num_records):
    data_to_insert = []
    data_for_csv = []  # 用于存储 CSV 数据
    for _ in range(num_records):
        # 生成随机个人信息
        id_card_number = str(random.randint(10000000000000000, 99999999999999999))
        check_code = calculate_check_code(id_card_number)
        id_card = id_card_number + check_code
        name = fake.name()  # 生成中文姓名
        last_digit = int(id_card_number[-1])
        if last_digit % 2 == 0:
            gender = "Female"
        else:
            gender = "Male"
        sub_string = id_card[6:14]
        birth_date = f"{sub_string[:4]}-{sub_string[4:6]}-{sub_string[6:]}"
        address = fake.address().replace('\n', ' ')  # 生成地址，并去除换行符
        phone = generate_phone_number()  # 生成符合中国标准的手机号

        # 将数据转换为GBK编码
        name = convert_to_gbk(name)
        address = convert_to_gbk(address)

        # 将数据添加到待插入列表
        data_to_insert.append((id_card, name, gender, birth_date, address, phone))

        # 同时准备 CSV 数据
        data_for_csv.append([id_card, name, gender, birth_date, address, phone])

    # 批量插入数据到数据库
    cursor.executemany('''
    INSERT OR IGNORE INTO person_info (id_card_number, name, gender, birth_date, address, phone)
    VALUES (?, ?, ?, ?, ?, ?)
    ''', data_to_insert)

    with open('ID_CARD/DataBase/person_info.csv', mode='w', newline='', encoding='gbk') as file:
        writer = csv.writer(file)
        writer.writerow(['id_card_number', 'name', 'gender', 'birth_date', 'address', 'phone'])  # 写入表头
        writer.writerows(data_for_csv)  # 写入数据

    # 提交事务
    conn.commit()

    # 写入 CSV 文件（编码为 GBK）

# 生成 100,000 条记录
generate_and_insert_data(120000)

# 完成后关闭数据库连接
conn.close()

print("数据生成完毕，SQLite 数据库和 CSV 文件已创建。")
