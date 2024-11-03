# MySQL服务管理

## 数据库服务启动

```
net start MySQL
```

此行命令在`Windows`的命令提示符(以管理员身份启动)中输入，`MySQL`这个参数并不一定是这个，取决于`MySQL`安装程序在`Windows`系统服务设置的名字。

可以在命令提示符中输入`services.msc`打开服务窗口查看`MySQL`程序的服务名称，比如我这里是`MySQL91`，则启动服务应输入`net start MySQL91`。

```
C:\WINDOWS\system32>net start MySQL91
MySQL91 服务正在启动 .
MySQL91 服务已经启动成功。
```

## 数据库服务停止

```
net stop MySQL
```

此行命令在`Windows`的命令提示符(以管理员身份启动)中输入，`MySQL`这个参数并不一定是这个，取决于`MySQL`安装程序在`Windows`系统服务设置的名字(同上)。

```
C:\WINDOWS\system32>net stop MySQL91
MySQL91 服务正在停止.
MySQL91 服务已成功停止。
```

## 登录数据库

隐藏输入密码：

```sql
mysql -u 用户名 -p
```

显示输入的密码

```
mysql -u 用户名 -p 密码
```

指定登录进入的数据库(隐藏密码)：

```
mysql -u 用户名 -p 数据库名
```

## 退出

```
quit | exit
```



# 创建数据库

```sql
CREATE DATABASE 数据库名称 [可用选项];
```

# 当指定的数据库名称不存在时创建数据库

```sql
CREATE DATABASE IF NOT EXISTS 数据库名称 [可用选项];
```

# 查看数据库

```sql
SHOW DATABASES;
```

# 查看指定数据库的创建信息

```sql
SHOW CREATE DATABASE 数据库名称;
```

输出创建该数据库时的`SQL`语句以及该数据库的默认字符集。

# 选择/切换数据库

```sql
USE 数据库名称;
```

# 删除数据库

```sql
DROP DATABASE 数据库名称;
```

如果该数据库不存在则会报错。

删除前进行数据库存在性检查：

```sql
DROP DATABASE IF EXISTS 数据库名称;
```

# 创建数据表

在进入某个数据库后

```sql
CREATE [TEMPORARY] TABLE [IF NOT EXISTS] 表名
(字段名 字段类型 [字段属性], ...) [表选项];
```

- `TEMPORARY`：可选项，表示临时表，该临时表仅在当前会话可见，在该会话关闭时自动删除。
- `字段名`：数据表列名
- `字段类型`：保存的字段的数据类型
- `字段属性`：可选项，字段的某些特殊约束条件
- `表选项`：设置表的相关特性，如字符集

```sql
mysql> use mydb;
Database changed
mysql> create table goods(
    -> id INT COMMENT '编号',
    -> name VARCHAR(32) COMMENT '商品名',
    -> price INT COMMENT '价格',
    -> description VARCHAR(255) COMMENT '商品描述'
    -> );
Query OK, 0 rows affected (0.03 sec)
```

或是直接向指定的数据库内创建表

```sql
CREATE [TEMPORARY] TABLE [IF NOT EXISTS] 数据库名.表名
(字段名 字段类型 [字段属性]...) [表选项];
```

创建的数据表的数据的字符集默认为`lantil`，如果插入中文数据会出错，可以通过设置支持中文的字符集解决，如`utf-8`：

```sql
CREATE [TEMPORARY] TABLE [IF NOT EXISTS] 表名
(字段名 字段类型 [字段属性], ...) [DEFAULT] {CHARACTER SET | CHARSET} [=] utf8;
```

# 查看数据表

```sql
SHOW TABLES [LIKE 匹配模式];
```

- `匹配模式`：可选，匹配数据表的名称，默认查看当前数据库中所有的数据表。
  - `%`：匹配任意个数字符，`%mydb%`即为名称包含`mydb`的数据表。
  - `_`：匹配一个字符，`_mydb`即为名称为5个字符且后四个字符为`mydb`的数据表。

# 查看数据表的相关信息

```sql
SHOW TABLE STATUS [FROM 数据库名称] [LIKE 匹配模式];
```

可以查看数据表的相关信息，如名称、存储引擎、创建时间等。

# 查看数据表的字段信息

```sql
# 格式1：查看数据表的所有字段的信息
{DESCRIBE | DESC} 数据表名;
# 格式2：查看数据表的指定字段的信息
{DESCRIBE | DESC} 数据表名 字段名 ;
```

```sql
mysql> desc my_goods;
+-------------+--------------+------+-----+---------+-------+
| Field       | Type         | Null | Key | Default | Extra |
+-------------+--------------+------+-----+---------+-------+
| id          | int          | YES  |     | NULL    |       |
| name        | varchar(32)  | YES  |     | NULL    |       |
| price       | int          | YES  |     | NULL    |       |
| description | varchar(255) | YES  |     | NULL    |       |
+-------------+--------------+------+-----+---------+-------+
4 rows in set (0.01 sec)
```

# 查看数据表的创建语句

```sql
SHOW CREATE TABLE 数据表名称;
```

```sql
mysql> show create table my_goods\G
*************************** 1. row ***************************
       Table: my_goods
Create Table: CREATE TABLE `my_goods` (
  `id` int DEFAULT NULL COMMENT '编号',
  `name` varchar(32) DEFAULT NULL COMMENT '商品名',
  `price` int DEFAULT NULL COMMENT '价格',
  `description` varchar(255) DEFAULT NULL COMMENT '商品描述'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
1 row in set (0.00 sec)
```

# 查看数据表结构

```sql
# 格式1
SHOW [FULL] COLUMNS FROM 数据表名 [FROM 数据库名];
# 格式2
SHOW [FULL] COLUMNS FROM 数据库名.数据表名;
```

- `FULL`：可选项，是否显示详细内容，可以额外显示字段权限、`COMMENT`注释信息等

```sql
mysql> show full columns from mydb.my_goods;
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
| Field       | Type         | Collation          | Null | Key | Default | Extra | Privileges                      | Comment  |
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
| id          | int          | NULL               | YES  |     | NULL    |       | select,insert,update,references | 编号     |
| name        | varchar(32)  | utf8mb4_0900_ai_ci | YES  |     | NULL    |       | select,insert,update,references | 商品名   |
| price       | int          | NULL               | YES  |     | NULL    |       | select,insert,update,references | 价格     |
| description | varchar(255) | utf8mb4_0900_ai_ci | YES  |     | NULL    |       | select,insert,update,references | 商品描述 |
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
4 rows in set (0.01 sec)
```

```sql
mysql> show create table my_goods\G
*************************** 1. row ***************************
       Table: my_goods
Create Table: CREATE TABLE `my_goods` (
  `id` int DEFAULT NULL COMMENT '编号',
  `name` varchar(32) DEFAULT NULL COMMENT '商品名',
  `price` int DEFAULT NULL COMMENT '价格',
  `description` varchar(255) DEFAULT NULL COMMENT '商品描述'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
1 row in set (0.00 sec)

mysql> show full columns from mydb.my_goods;
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
| Field       | Type         | Collation          | Null | Key | Default | Extra | Privileges                      | Comment  |
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
| id          | int          | NULL               | YES  |     | NULL    |       | select,insert,update,references | 编号     |
| name        | varchar(32)  | utf8mb4_0900_ai_ci | YES  |     | NULL    |       | select,insert,update,references | 商品名   |
| price       | int          | NULL               | YES  |     | NULL    |       | select,insert,update,references | 价格     |
| description | varchar(255) | utf8mb4_0900_ai_ci | YES  |     | NULL    |       | select,insert,update,references | 商品描述 |
+-------------+--------------+--------------------+------+-----+---------+-------+---------------------------------+----------+
4 rows in set (0.01 sec)

mysql> show full columns from mydb.my_goods\G
*************************** 1. row ***************************
     Field: id
      Type: int
 Collation: NULL
      Null: YES
       Key:
   Default: NULL
     Extra:
Privileges: select,insert,update,references
   Comment: 编号
*************************** 2. row ***************************
     Field: name
      Type: varchar(32)
 Collation: utf8mb4_0900_ai_ci
      Null: YES
       Key:
   Default: NULL
     Extra:
Privileges: select,insert,update,references
   Comment: 商品名
*************************** 3. row ***************************
     Field: price
      Type: int
 Collation: NULL
      Null: YES
       Key:
   Default: NULL
     Extra:
Privileges: select,insert,update,references
   Comment: 价格
*************************** 4. row ***************************
     Field: description
      Type: varchar(255)
 Collation: utf8mb4_0900_ai_ci
      Null: YES
       Key:
   Default: NULL
     Extra:
Privileges: select,insert,update,references
   Comment: 商品描述
4 rows in set (0.00 sec)
```

# 修改数据表名称

```sql
/*格式1*/
ALTER TABLE 旧表名 RENAME [TO|AS] 新表名;
/*格式2*/
RENAME TABLE 旧表名1 TO 新表名1 [, 旧表名2 TO 新表名2] ...
```

- 格式1可以修改一个数据表的名称
- 格式2则可以同时修改多个数据表的名称

# 修改表选项

```sql
ALTER TABLE 表名 表选项 [=] 值;
```

可以修改数据表的字符集、存储引擎、校对集

# 修改数据表字段名

```sql
ALTER TABLE 数据表名 CHANGE [COLUMN] 旧字段名 新字段名 字段类型 [字段属性];
```

# 修改数据表字段类型

```sql
ALTER TABLE 数据表名 MODIFY [COLUMN] 字段名 新字段类型 [字段属性];
```

# 修改数据表字段位置

```sql
ALTER TABLE 数据表名
MODIFY [COLUMN] 字段名1 数据类型 [字段属性] [FIRST | AFTER 字段名2];
```

- `FIRST`：将`字段名1`调整为数据表的第1项
- `AFTER`：将`字段名1`调整到`字段名2`的后面

# 修改数据表字段的字符集

```sql
ALTER TABLE 数据表名
MODIFY 字段名1 字段类型1 {CHARACTER SET| CHARSET} [=] 字符集,
MODIFY 字段名2 字段类型2 {CHARACTER SET| CHARSET} [=] 字符集,
...;
```

# 新增数据表字段(新增列)

```sql
# 格式1，新增一个字段，指定位置，默认位置为表尾
ALTER TABLE 数据表名
ADD [COLUMN] 新字段名 字段类型 [字段属性] [FIRST | AFTER 字段名];
# 格式2，新增多个字段，新增至表尾
ALTER TABLE 数据表名
ADD [COLUMN] (新字段名1, 字段类型1, 新字段名2, 字段类型2, ...);
```

# 删除字段(删除列)

```sql
ALTER TABLE 数据表名 DROP [COLUMN] 字段名;
```

# 删除数据表

```sql
DROP [TEMPORARY] TABLE [IF EXISTS] 数据表1 [, 数据表2, ...];
```

# 为所有字段插入数据(插入行)

```sql
INSERT [INTO] 数据表名 {VALUES | VALUE} (值1, [值2, ...]);
```

# 为部分字段插入数据

```sql
INSERT [INTO] 数据表名 (字段名1 [, 字段名2] ...) {VALUES | VALUE}
(值1 [, 值2] ...),
...;
```

```sql
INSERT [INTO] 数据表名
SET 字段名1 = 值1 [, 字段名2 = 值2, ...];
```

# 查询数据表中的全部字段数据

```sql
SELECT * FROM 数据表名;
```

# 查询数据表中部分字段数据

```sql
SELECT {字段1, 字段2, ...} FROM 数据表名;
```

# 条件查询

```sql
# 查询全部
SELECT * FROM 数据表名 WHERE 字段名 = 值;
# 查询部分字段
SELECT {字段1, 字段2, 字段3, ...} FROM 数据表名 WHERE 字段名 = 值;
# 查询字段设置别名
SELECT 字段1 AS 别名1, 字段2 AS 别名2, 字段3 AS 别名3, ... FROM 数据表名 WHERE 字段名 = 值;
```

# 数据修改

```sql
UPDATE 数据表名
SET 字段名1 = 值1 [, 字段名2 = 值2, ...]
[WHERE 条件表达式]
```

# 数据删除

```sql
DELETE FROM 数据表名 [WHERE 条件表达式];
```

# 数据类型

`MySQL`中存储的数据的数据类型，包括数字类型、时间日期、字符串类型。

## 数字类型

### 整数类型

|       数据类型       | 字节数 |     取值范围     |
| :------------------: | :----: | :--------------: |
|      `TINYINT`       |   1    |      0~255       |
|      `SMALLINT`      |   2    |     0~65535      |
|     `MEDIUMINT`      |   3    |    0~(2^24)-1    |
|        `INT`         |   4    |    0~(2^32)-1    |
|       `BIGINT`       |   8    |    0~(2^64)-1    |
|  `UNSIGNED TINYINT`  |   1    |     -128~127     |
| `UNSIGNED SMALLINT`  |   2    |   -32768~32767   |
| `UNSIGNED MEDIUMINT` |   3    | -(2^23)~(2^23)-1 |
|    `UNSIGNED INT`    |   4    | -(2^31)~(2^31)-1 |

### 浮点数类型

| 数据类型 | 字节数 | 负数取值范围 | 整数取值范围 |
| :------: | :----: | :----------: | :----------: |
| `FLOAT`  |   4    |              |              |
| `DOUBLE` |   8    |              |              |

### 定点数类型

`DECIMAL(M, D)`，`M`代表定点数数字的总位数(不包括'.'和'-')，最大值为65，默认值为10，`D`表示小数点后的位数，最大值为30，默认值为0。

### 位(BIT)类型

`BIT(M)`，`M`表示位数，范围为1~64。

## 时间日期类型

|  数据类型   |                取值范围                 |      日期格式       |        零值         |
| :---------: | :-------------------------------------: | :-----------------: | :-----------------: |
|   `YEAR`    |                1901~2155                |        YYYY         |        0000         |
|   `DATE`    |          1000-01-01~9999-12-3           |     YYYY-MM-DD      |     0000-00-00      |
|   `TIME`    |          -838:59:59~838:59:59           |      HH:MM:SS       |      00:00:00       |
| `DATETIME`  | 1000-01-01 00:00:00~9999-12-31 23:59:59 | YYYY-MM-DD HH:MM:SS | 0000-00-00 00:00:00 |
| `TIMESTAMP` | 1907-01-01 00:00:01~2038-01-19 03:14:07 | YYYY-MM-DD HH:MM:SS | 0000-00-00 00:00:00 |

## 字符串类型

|  数据类型   |      说明      |
| :---------: | :------------: |
|   `CHAR`    |   定长字符串   |
|  `VARCHAR`  |   变长字符串   |
|   `TEXT`    |   大文本数据   |
|   `ENUM`    |    枚举类型    |
|    `SET`    |   字符串对象   |
|  `BINARY`   | 定长二进制数据 |
| `VARBINARY` | 变长二进制数据 |
|   `BLOB`    |  二进制大对象  |

### `CHAR`和`VARCHAR`

定义方式

```sql
CHAR(M)
VARCHAR(M)
```

`M`代表字符串的最大长度，`CHAR`无论存储的数据有多长，都会占用M字节，而`VARCHAR`则占用存储字符串的实际长度+1个字节，最多不超过M+1个字节。

### `TEXT`

保存大文本数据，较长的文本数据，具体可分为4种类型。

|   数据类型   | 存储大小 |
| :----------: | :------: |
|  `TINYTEXT`  | 0~2^8-1  |
|    `TEXT`    | 0~2^16-1 |
| `MEDIUMTEXT` | 0~2^24-1 |
|  `LONGTEXT`  | 0~2^32-1 |

存储大小是指字节数，并不是字符数。

### `ENUM`

```sql
ENUM('值1', '值2', '值3', ...)
```

### `SET`

```sql
SET('值1', '值2', '值3', ...)
```

`SET`类型最多可以有64个值，与`ENUM`不同，可以同时选择多个值。

### `BINARY`和`VARBINARY`

```sql
BINARY(M)
VARBINARY(M)
```

### `BLOB`

用于保存数据量很大的二进制数据

|   数据类型   | 存储大小 |
| :----------: | :------: |
|  `TINYBLOB`  | 0~2^8-1  |
|    `BLOB`    | 0~2^16-1 |
| `MEDIUMBLOB` | 0~2^24-1 |
|  `LONGBLOB`  | 0~2^32-1 |

# 约束

## 默认约束

默认约束用于为数据表中的字段指定默认值，默认值通过关键字`DEFAULT`定义。

```sql
字段名 数据类型 DEFAULT 默认值;
```

注：`BLOB`、`TEXT`不支持默认约束。

还可以通过`ALTER TABLE`添加或删除默认约束。

```sql
mysql> desc my_goods;
+-------------+--------------+------+-----+---------+-------+
| Field       | Type         | Null | Key | Default | Extra |
+-------------+--------------+------+-----+---------+-------+
| id          | int          | YES  |     | NULL    |       |
| name        | varchar(32)  | YES  |     | NULL    |       |
| price       | int          | YES  |     | NULL    |       |
| description | varchar(255) | YES  |     | NULL    |       |
+-------------+--------------+------+-----+---------+-------+
4 rows in set (0.01 sec)

# 添加默认约束
mysql> alter table my_goods MODIFY price int DEFAULT 1;
Query OK, 0 rows affected (0.02 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> desc my_goods;
+-------------+--------------+------+-----+---------+-------+
| Field       | Type         | Null | Key | Default | Extra |
+-------------+--------------+------+-----+---------+-------+
| id          | int          | YES  |     | NULL    |       |
| name        | varchar(32)  | YES  |     | NULL    |       |
| price       | int          | YES  |     | 1       |       |
| description | varchar(255) | YES  |     | NULL    |       |
+-------------+--------------+------+-----+---------+-------+
4 rows in set (0.00 sec)

# 解除默认约束
mysql> alter table my_goods MODIFY price int;
Query OK, 0 rows affected (0.01 sec)
Records: 0  Duplicates: 0  Warnings: 0

mysql> desc my_goods;
+-------------+--------------+------+-----+---------+-------+
| Field       | Type         | Null | Key | Default | Extra |
+-------------+--------------+------+-----+---------+-------+
| id          | int          | YES  |     | NULL    |       |
| name        | varchar(32)  | YES  |     | NULL    |       |
| price       | int          | YES  |     | NULL    |       |
| description | varchar(255) | YES  |     | NULL    |       |
+-------------+--------------+------+-----+---------+-------+
4 rows in set (0.00 sec)
```

## 非空约束

指定字段不可为`NULL`，通过关键字`NOT NULL`进行约束。

```sql
字段名 数据类型 NOT NULL;
```

## 唯一约束

保证数据表中字段的唯一性，表中字段的值不可重复，通过关键字`UNIQUE`进行约束。

```sql
# 列级约束
字段名 数据类型 UNIQUE;
# 表级约束
UNIQUE(字段名1, 字段名2, ...);
```

## 主键约束

唯一标识表中的一条数据的字段，通过关键字`PRIMARY KEY`定义，同时代表了唯一约束`UNIQUE`以及非空约束`NOT NULL`，每个数据表最多同时含有一个主键。

```sql
# 列级约束
字段名 数据类型 PRIMARY KEY
# 表级约束
PRIMARY KEY (字段名1, 字段名2, ...)
```

```sql
# 添加主键约束
ALTER TABLE 数据表名 ADD PRIMARY KEY (字段名);
# 删除主键约束
ALTER TABLE 数据表名 DROP PRIMARY KEY;
```

# 数据操作

## 复制表的结构

```sql
CREATE [TEMPORYRY] TABLE [IF NOT EXISTS] 数据表名
{LIKE 旧表名 | (LIKE 旧表名)}
```

从旧表名中复制一份相同的表结构，但不包含数据。

## 复制表的数据

```sql
INSERT [INTO] 数据表名1 [(字段列表)] SELECT [(字段列表)] FROM 数据表名2;
```

两个数据表名可以相同也可以不同，复制数据要考虑主键冲突问题。

## 主键冲突解决

### 更新

利用更新操作解决插入数据时的主键冲突。

```sql
INSERT [INTO] 数据表名 [(字段列表)] {VALUES | VALUE} (值列表)
ON DUPLICATE KEY UPDATE 字段名1 = 新值1 [, 字段名2=新值2] ...;
```

### 替换

替换会删除原有的发生主键冲突的数据，再插入新的数据。

```sql
REPLACE [INTO] 数据表名 [(字段列表)]
{VALUES | VALUE} (值列表) [, (值列表)] ...;
```

## 清空数据

```sql
TRUNCATE [TABLE] 数据表名;
```

`TRUNCATE`会先执行删除`DROP`操作删除数据表，再根据表结构文件`.frm`重建数据表。

## 查询结果去除重复数据

```sql
SELECT DISTINCT 字段列表 FROM 数据表;
```

`select选项`默认为`ALL`，选用`DISTINCT`会将`字段列表`数据全相同的数据合并为一条查询结果。

## 排序

### 单字段排序

```sql
SELECT * | {字段列表} FROM 数据表名
ORDER BY 字段名 [ASC | DESC];
```

按照`字段名`对查询出的数据结果进行排序。`ASC`为升序排序，`DESC`为降序排序，默认为`ASC`。

### 多字段排序

```sql
SELECT * | {字段列表} FROM 数据表名
ORDER BY 字段名1 [ASC | DESC] [, 字段名2 [ASC | DESC]] ...;
```

多字段排序会首先按照`字段名1`排序，`字段名1`相同的再按照`字段名2`排序，以此类推。

## 限量

限定查询结果的数量，或指定查询从哪里开始。

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式] [ORDER BY 字段 ASC | DESC]
LIMIT [OFFSET,] 记录数;
```

`记录数`限定最大查询结果数量，`OFFSET`指定从总的查询结果中第1条结果的偏移量，如果从第3条结果开始展示则`OFFSET`设置为3。

# 分组与聚合

## 分组统计

查询数据时为`WHERE`添加`GROUP BY`根据指定字段进行分组。

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式] GROUP BY 字段名;
```

## 分组排序

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式]
GROUP BY 字段名 [ASC | DESC];
```

## 多分组统计

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式]
GROUP BY 字段名1 [ASC | DESC], [, 字段名2 [ASC | DESC]]...;
```

## 回溯统计

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式]
GROUP BY 字段名1 [ASC | DESC], [, 字段名2 [ASC | DESC]]... WITH ROLLUP;
```

回溯统计会在指定字段分组统计结果的最后添加一个对上面结果的一个总的数量统计，其分组字段为`NULL`。

## 统计筛选

```sql
SELECT [select选项] 字段列表 FROM 数据表名
[WHERE 条件表达式]
GROUP BY 字段名1 [ASC | DESC], [, 字段名2 [ASC | DESC]]... [WITH ROLLUP]
HAVING 条件表达式;
```

`HAVING`后的条件表达式可以使用聚合函数，而`WHERE`后的则不能。

## 聚合函数

|       函数名       |                     作用                     |
| :----------------: | :------------------------------------------: |
|     `COUNT()`      |     返回参数字段的数量，`NULL`字段不统计     |
|      `SUM()`       |               返回参数字段之和               |
|      `AVG()`       |              返回参数字段平均值              |
|      `MAX()`       |              返回参数字段最大值              |
|      `MIN()`       |              返回参数字段最小值              |
|  `GROUP_CONCAT()`  |     返回符合条件的参数字段值的连接字符串     |
| `JSON_ARRAYAGG()`  | 将符合条件的参数字段值作为单个`JSON`数组返回 |
| `JSON_OBJECTAGG()` | 将符合条件的参数字段值作为单个`JSON`对象返回 |

# 运算符

## 算术运算符

用于数值类型数据，通常应用在`SELECT`查询结果的字段中。

| 运算符 |   作用   |    示例     |
| :----: | :------: | :---------: |
|   +    |  加运算  | SELECT 4+2; |
|   -    |  减运算  | SELECT 4-2; |
|   *    |  乘运算  | SELECT 4*2; |
|   /    |  除运算  | SELECT 4/2; |
|   %    | 取模运算 | SELECT 4%2; |

# 多表查询

## 联合查询

```sql
SELECT ...
UNION [ALL | DISTINCT] SELECT ...
[UNION [ALL | DISTINCT] SELECT ...];
```

联合查询中`SELECT`的查询字段数量必须相同，且查询结果只保留第一个`SELECT`的查询字段。

## 连接查询

### 交叉连接

```sql
SELECT 查询字段 FROM 表1 CROSS JOIN 表2;
```

返回两个表中所有数据行的笛卡尔积。

### 内连接

```sql
SELECT 查询字段 FROM 表1
[INNER] JOIN 表2 ON 匹配条件;
```

返回`表1`与`表2`中符合`匹配条件`的所有`查询字段`。

### 左外连接

```sql
SELECT 查询字段
FROM 表1 LEFT [OUTER] JOIN 表2 ON 匹配条件;
```

左表`表1`为主表，右表`表2`为从表，查询所有主表中的数据，以及从表中符合匹配条件的记录，主表中的记录若没有查询的从表中的字段数据，则设置为`NULL`。

### 右外连接

```sql
SELECT 查询字段
FROM 表1 RIGHT [OUTER] JOIN 表2 ON 匹配条件;
```

左表`表1`为从表，右表`表2`为主表，查询所有主表中的数据，以及从表中符合匹配条件的记录，主表中的记录若没有查询的从表中的字段数据，则设置为`NULL`。

## 子查询

# 外键约束

## 添加外键约束

```sql
[CONSTRAINT name] FOREIGN KEY [index_name] (index_col_name, ...)
REFERENCES tbl_name (index_col_name, ...)
[ON DELETE {RESTRICT | CASCADE | SET NULL | NO ACTION | SET DEFAULT}]
[ON UPDATE {RESTRICT | CASCADE | SET NULL | NO ACTION | SET DEFAULT}]
```

## 删除外键约束

```sql
ALTER TABLE 表名 DROP FOREIGN KEY 外键名;
```

