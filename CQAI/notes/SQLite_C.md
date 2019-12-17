# SQLite之C接口

**此内容转载自[segmentfault](https://segmentfault.com/a/1190000003850108), 以下内容仅重新排版**   
还有一篇更详细的[sqlite3用法详解 demo](https://blog.csdn.net/qingzhuyuxian/article/details/79959326)值得细看, 不过太长了就没整理

## sqlite之C接口简介  

### sqlite3_open

```c
int sqlite3_open (
    const char *filename,  /* Database filename (UTF-8) */ 
    /* 数据库将采用UTF-8的编码方式，sqlite3_open16采用UTF-16的编码方式 */
    sqlite3 **ppDb         /* OUT: SQLite db handle */ /* 数据库连接对象  */
):
```
打开一个sqlite数据库文件的连接并且返回一个数据库连接对象。  
filename: 数据库名。  
ppDb: 一个数据库连接句柄被返回到这个参数。  
Return: 如果执行成功，则返回SQLITE_OK，否则返回一个错误码。
  
### sqlite3_prepare_v2
```c
int sqlite3_prepare_v2(
  sqlite3 *db,            /* Database handle */ /* 成功打开的数据库句柄 */
  const char *zSql,       /* SQL statement, UTF-8 encoded */ /* UTF8编码的 SQL 语句 */
  int nByte,              /* Maximum length of zSql in bytes. */ /* 参数 sql 的字节数, 包含 '\0' */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */ /* 输出:预编译语句句柄 */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */ /* 输出:指向 sql 语句中未使用的部分 */
);
```

这个函数将sql文本转换成一个准备语句（prepared statement）对象，同时返回这个对象的指针。   
db: 数据库连接句柄。  
zsql: 将要被编译的SQL语句。  
nByte: 如果nByte为负值（-1），则函数取出zSql中从开始到第一个0终止符的内容;若为正值，那么它就是这个函数能从zSql中读取的字节数的最大值;若为0,不会产生SQL语句。有一个更好的做法是:把nbytes的值设为该字符串的长度(包含'0'), 这样可以避免 SQLite 复制该字符串的一份拷贝, 以提高程序的效率。  
ppStmt: 指向一个被编译的语句句柄，它可被sqlite3_step执行。执行错误或者输入的zSql为空字符，*ppStmt为NULL，用sqlite3_finalize来释放它。  
pzTail: 上面提到zSql在遇见终止符或者是达到设定的nByte之后结束，假如zSql还有剩余的内容，那么这些剩余的内容被存放到pZTail中，不包括终止符。如果 pszTail 不为 NULL, 则 pszTail 指向 sql 中第一个被传入的 SQL 语句的结尾. 该函数只编译 sql 的第一个语句, 所以 pszTail 指向的内容则是未被编译的。  
Return: 如果执行成功，则返回SQLITE_OK，否则返回一个错误码。
  
### sqlite3_step
```c
int sqlite3_step(sqlite3_stmt *stmt);
```
Stmt: 调用一次或者多次被编译的语句句柄。  
Return: SQLITE_BUSY，SQLITE_DONE，SQLITE_ROW，SQLITE_ERROR，SQLITE_MISUSE。  
SQLITE_BUSY: 忙碌. 数据库引擎无法锁定数据去完成其工作. 但可以多次尝试.  
SQLITE_DONE: 完成. sql 语句已经被成功地执行.在调用 sqlite_reset() 之前,当前预编译的语句不应该被sqlite3_step()再次调用.  
SQLITE_ROW: 查询时产生了结果. 此时可以通过相关的"数据访问函数(column access functions)"来取得数据. sqlite3_step()可再一次调用将取得下一条查询结果.  
SQLITE_ERROR: 发生了错误. 此时可以通过 sqlite3_errmmsg() 取得相关的错误信息. sqlite3_step() 不能被再次调用.  
SQLITE_MISUSE: 不正确的库的使用. 该函数使用不当.  

### sqlite3_reset
```c
int sqlite3_reset (sqlite3_stmt *stmt);
```
sqlite3_reset用于重置一个准备语句对象到它的初始状态，然后准备被重新执行。所有sql语句变量使用sqlite3_bind_XXX绑定值，使用sqlite3_clear_bindings清除这些绑定。 Sqlite3_reset接口重置准备语句到它代码开始的时候。sqlite3_reset并不改变在准备语句上的任何绑定值，那么这里猜测，可能是语句在被执行的过程中发生了其他的改变，然后这个语句将它重置到绑定值的时候的那个状态。  
Return: SQLITE_BUSY,SQLITE_DONE，SQLITE_ROW。  
SQLITE_BUSY: 暂时无法执行操作.  
SQLITE_DONE: 操作执行完毕.  
SQLITE_ROW: 执行完毕并且有返回（执行select语句时）;需要对查询结果进行处理，SQLITE3提供sqlite3_column_*系列函数.  

### sqlite3_exec
```c
int sqlite3_exec(
    sqlite3*db,                                  /* An open database */
    const char *sql,                           /* SQL to be evaluated */
    int (*callback)(void*,int,char**,char**),  /* Callback function */
    void *arg,                                    /* 1st argument to callback */
    char **errmsg                              /* Error msg written here */
);
```
该函数用来执行多条以“;”分隔的 SQL 语句。封装了 sqlite3_prepare(), sqlte3_step() 和 sqlite3_finalize() 函数。  
db: 数据库连接句柄。  
sql: 将要被编译的SQL语句。  
callback: 回调函数，若该函数非空，则每次SQL语法的结果都会调用该函数，若为空，则将被忽略。  
arg: 回调函数的第一个参数。  
errmsg: 存储错误信息，可为NULL;  
Return: 返回非零，该函数立即中断查询，并不再执行后续的SQL语句和回调函数;返回SQLITE_ABORT结束执行。  
```c
int callback(void *arg, int argc, char **argv, char **azColName);
```
arg: 为sqlite3_exec中的第四个参数，即你所传递的参数。  
argc: 执行SQL语法所产生的列元素个数。  
argv: 执行SQL语法所产生的列元素列表，argv[i]代表第i个元素。  
azColName: 执行SQL语法每行中各列的列名。  

### sqlite3_bind_XXX
```c
int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_int(sqlite3_stmt*, int, int);
int sqlite3_bind_null(sqlite3_stmt*, int);
int sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
```
给参数绑定值。  
stmt: 指向一个被编译的语句句柄，它可被sqlite3_step执行。执行错误或者输入的zSql为空字符，*ppStmt为NULL，用sqlite3_finalize来释放它。  
index: SQL参数(每列元素)的索引，从0开始.  
value: 字符串值。  
len: 字符串长度。  
fun: 一个函数指针，SQLITE3执行完操作后回调此函数，通常用于释放字符串占用的内存。此参数有两个常数，SQLITE_STATIC告诉sqlite3_bind_text函数字符串为常量，可以放心使用；而SQLITE_TRANSIENT会使得sqlite3_bind_text函数对字符串做一份拷贝。  

### sqlite3_column_XXX
```c
double sqlite3_column_double(sqlite3_stmt*, int iCol);
int sqlite3_column_int(sqlite3_stmt*, int iCol);
const unsigned char *sqlite3_column_text(sqlite3_stmt*, int iCol);
int sqlite3_column_type(sqlite3_stmt*, int iCol);
```
这类函数从sqlite3_step()返回的结果集中提取某一列。  
stmt: 指向一个被编译的语句句柄，它可被sqlite3_step执行。执行错误或者输入的zSql为空字符，*ppStmt为NULL，用sqlite3_finalize来释放它。  
index: SQL参数(每列元素)的索引，从1开始.  

## 示例

### 打开数据库
```c
#include <sqlite3.h>
#include <stdio.h>

int main(void)
{
    sqlite3 *db;
        sqlite3_stmt *res;
    
    int rc = sqlite3_open("lab.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
   
    sqlite3_close(db);
        printf ("Succeed to open the database!\n");
    return 0;
}
```
```shell
$ gcc -o open open.c -lsqlite3
$ ./open
Succeed to open the database!
```

### 创建目标数据表
```c
#include <sqlite3.h>
#include <stdio.h>

int main ()
{
    sqlite3    *db;
        sqlite3_stmt *res;
    
    int rc = sqlite3_open("lab.db", &db);
    
    if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);       
                return 1;
        }
        
    char *SQL = "CREATE TABLE IF NOT EXISTS testtable (ID INTEGER PRIMARY KEY, name TEXT, gender TEXT, age REAL, value REAL)";

        if(sqlite3_prepare_v2(db, SQL, -1, &res, 0)) {
                printf("Failed to create table!\n");
                if (res)
                        sqlite3_finalize(res);
                sqlite3_close(db);
                return 1;
        }

        if (sqlite3_step(res) != SQLITE_DONE) {
                sqlite3_finalize(res);
                sqlite3_close(db);
                return 1;
        }
        sqlite3_finalize(res);
        printf ("Succeed to create testtable now.\n");
        sqlite3_close(db);
        
        return 0;
}
```
```shell
$ gcc -o create create.c -lsqlite3
$ ./create
Succeed to create testtable now!
```

### 数据表操作
```c
#include <sqlite3.h>
#include <stdio.h>

int main ()
{
    sqlite3    *db;
        sqlite3_stmt *res;
    
    int rc = sqlite3_open("lab.db", &db);
    
    if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);       
                return 1;
        }
        
        char *err_msg = 0;
    
    /* 想实现哪项功能，就将注释去掉 */
    //char *SQL = "INSERT INTO testtable VALUES(NULL, 'xiao', 'male', 100, 0.1)";//向数据表中插入一条数据
        //char *SQL = "DELETE FROM testtable WHERE gender = males AND age = 26";//删除数据表中的多条数据
        //char *SQL = "DELETE FROM testtable";//删除数据表中所有数据
        //char *SQL = "UPDATE testtable SET gender = female,age=30 WHERE ID = 1";//更新数据表中的多条数据
    //char *SQL = "DROP TABLE IF EXISTS testtable";//删除数据表
        
        int rc2 = sqlite3_exec(db, SQL, 0, 0, &err_msg);

        if(rc2 != SQLITE_OK) {
                fprintf(stderr, "Failed to drop table\n");
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
                sqlite3_close(db);
                return;
        }
        
        sqlite3_close(db);
        printf ("Succeed!\n");
        return 0;
}
```
```shell
$ gcc -o table table.c -lsqlite3
$ ./table
Succeed!
```

### 批量插入数据
```c
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

int main ()
{
    sqlite3    *db;
    
    int rc = sqlite3_open("lab.db", &db);
    
    if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);       
                return 1;
        }

        /* 手动开启事物 */
        sqlite3_stmt* stmtb = NULL;
        const char* beginSQL = "BEGIN TRANSACTION";
        if (sqlite3_prepare_v2(db,beginSQL,strlen(beginSQL),&stmtb,NULL) != SQLITE_OK) {
                if (stmtb)
                        sqlite3_finalize(stmtb);
                sqlite3_close(db);
                return 1;
        }
        if (sqlite3_step(stmtb) != SQLITE_DONE) {
                sqlite3_finalize(stmtb);
                sqlite3_close(db);
                return 1;
        }
        sqlite3_finalize(stmtb);
        /*****************************/
        
        char *err_msg = 0;
        char *sql = "INSERT INTO testtable VALUES(NULL, ?, ?, ?, ?)";
        sqlite3_stmt *stmt = NULL;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
                if (stmt)
                        sqlite3_finalize(stmt);
                sqlite3_close(db);
                return 1;
        }

        char *name = "xiao";
        char *m = "male";
        char *f = "female";
        
        for (int i = 1; i <= 10; i++) {
                sqlite3_bind_null (stmt, 0);
                sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
                if(i % 2 == 0)
                        sqlite3_bind_text(stmt, 2, m, -1, SQLITE_TRANSIENT);
                else
                        sqlite3_bind_text(stmt, 2, f, -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, i + 24);
                double fl = i * 9.9;
                sqlite3_bind_double(stmt, 4, fl);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        return 1;
                }
                sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);

        /* 手动关闭事物 */
        sqlite3_stmt* stmtc = NULL;
        const char *commitSQL = "COMMIT TRANSACTION";
        if (sqlite3_prepare_v2(db,commitSQL,strlen(commitSQL),&stmtc,NULL) != SQLITE_OK) {
                if (stmtc)
                        sqlite3_finalize(stmt);
                sqlite3_close(db);
                return 1;
        }
        if (sqlite3_step(stmtc) != SQLITE_DONE) {
                sqlite3_finalize(stmtc);
                sqlite3_close(db);
                return 1;
        }
        sqlite3_finalize(stmtc);
        /*********************************/
        
        sqlite3_close(db);
        
        return 0;
}
```
```shell
$ gcc -o insert_datas insert_datas.c -lsqlite3
$ ./insert_data
$ sqlite3 lab.db
sqlite3> SELECT * FROM testtable;
1|xiao|female|25.0|9.9
2|xiao|male|26.0|19.8
3|xiao|female|27.0|29.7
4|xiao|male|28.0|39.6
5|xiao|female|29.0|49.5
6|xiao|male|30.0|59.4
7|xiao|female|31.0|69.3
8|xiao|male|32.0|79.2
9|xiao|female|33.0|89.1
10|xiao|male|34.0|99.0
```

### 显示数据表的数据(non call_back)
```c
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

int main ()
{
    sqlite3    *db;
    
    int rc = sqlite3_open("lab.db", &db);
    
    if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);       
                return 1;
        }
        
        const char *sql = "SELECT * FROM testtable";
        sqlite3_stmt *stmt = NULL;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
                if (stmt)
                        sqlite3_finalize(stmt);
                sqlite3_close(db);
                return 1;
        }

        int fieldcout = sqlite3_column_count (stmt);
        do {
                int r = sqlite3_step(stmt);
                if (r == SQLITE_ROW) {
                        int i;
                        for (i = 0; i < fieldcout; ++i) {
                                int vtype = sqlite3_column_type(stmt, i);
                                if (vtype == SQLITE_INTEGER) {
                                        int v = sqlite3_column_int(stmt, i);
                                        printf ("%d\n", v);
                                } else if (vtype == SQLITE_FLOAT) {
                                        double v = sqlite3_column_double(stmt, i);
                                        printf ("%f\n", v);
                                } else if (vtype == SQLITE_TEXT) {
                                        const char *v = (const char *)sqlite3_column_text(stmt, i);
                                        printf("%s\n", v);
                                } else if (vtype == SQLITE_NULL) {
                                        printf ("NULL\n");
                                }
                        }
                } else if (r == SQLITE_DONE) {
                        printf ("Select finish.\n");
                        break;
                } else {
                        printf ("Failed to select.\n");
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        return 1;
                }
        } while (1);
        sqlite3_finalize(stmt);        
        
        sqlite3_close(db);
        
        return 0;
}
```
```shell
$ gcc -o select select.c -lsqlite3
$ ./select
1
xiao
female
25.000000
9.900000
2
xiao
male
26.000000
19.800000
...
...
...
10
xiao
male
34.000000
99.000000
```

### call_back
```c
#include <sqlite3.h>
#include <stdio.h>

int callback(void *, int, char **, char **);

int main(void) {
        sqlite3 *db;
        char *err_msg = 0;

        int rc = sqlite3_open("lab.db", &db);

        if(rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);

                return 1;
        }

        char *sql = "PRAGMA table_info(testtable)";

        rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

        if(rc != SQLITE_OK) {
                fprintf(stderr, "Failed to select data\n");
                fprintf(stderr, "SQL error: %s\n", err_msg);

                sqlite3_free(err_msg);
                sqlite3_close(db);

                return 1;
        }

        sqlite3_close(db);

        return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
        NotUsed = 0;
        int i;
        for (i = 0; i < argc; i++) {
                printf ("%s = %s\n", azColName[i], argv[i]?argv[i]:"NULL");
        }

        printf("\n");

        return 0;
}
```
```shell
$ gcc -o callback callback.c -lsqlite3
$ ./callback
cid = 0
name = ID
type = INTEGER
notnull = 0
dflt_value = NULL
pk = 1
...
...
...
cid = 4
name = value
type = REAL
notnull = 0
dflt_value = NULL
pk = 0
```
