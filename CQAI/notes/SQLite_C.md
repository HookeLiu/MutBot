# SQLite֮C�ӿ�

**������ת����[segmentfault](https://segmentfault.com/a/1190000003850108), �������ݽ������Ű�**   
����һƪ����ϸ��[sqlite3�÷���� demo](https://blog.csdn.net/qingzhuyuxian/article/details/79959326)ֵ��ϸ��, ����̫���˾�û����

## sqlite֮C�ӿڼ��  

### sqlite3_open

```c
int sqlite3_open (
    const char *filename,  /* Database filename (UTF-8) */ 
    /* ���ݿ⽫����UTF-8�ı��뷽ʽ��sqlite3_open16����UTF-16�ı��뷽ʽ */
    sqlite3 **ppDb         /* OUT: SQLite db handle */ /* ���ݿ����Ӷ���  */
):
```
��һ��sqlite���ݿ��ļ������Ӳ��ҷ���һ�����ݿ����Ӷ���  
filename: ���ݿ�����  
ppDb: һ�����ݿ����Ӿ�������ص����������  
Return: ���ִ�гɹ����򷵻�SQLITE_OK�����򷵻�һ�������롣
  
### sqlite3_prepare_v2
```c
int sqlite3_prepare_v2(
  sqlite3 *db,            /* Database handle */ /* �ɹ��򿪵����ݿ��� */
  const char *zSql,       /* SQL statement, UTF-8 encoded */ /* UTF8����� SQL ��� */
  int nByte,              /* Maximum length of zSql in bytes. */ /* ���� sql ���ֽ���, ���� '\0' */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */ /* ���:Ԥ��������� */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */ /* ���:ָ�� sql �����δʹ�õĲ��� */
);
```

���������sql�ı�ת����һ��׼����䣨prepared statement������ͬʱ������������ָ�롣   
db: ���ݿ����Ӿ����  
zsql: ��Ҫ�������SQL��䡣  
nByte: ���nByteΪ��ֵ��-1��������ȡ��zSql�дӿ�ʼ����һ��0��ֹ��������;��Ϊ��ֵ����ô��������������ܴ�zSql�ж�ȡ���ֽ��������ֵ;��Ϊ0,�������SQL��䡣��һ�����õ�������:��nbytes��ֵ��Ϊ���ַ����ĳ���(����'0'), �������Ա��� SQLite ���Ƹ��ַ�����һ�ݿ���, ����߳����Ч�ʡ�  
ppStmt: ָ��һ�������������������ɱ�sqlite3_stepִ�С�ִ�д�����������zSqlΪ���ַ���*ppStmtΪNULL����sqlite3_finalize���ͷ�����  
pzTail: �����ᵽzSql��������ֹ�������Ǵﵽ�趨��nByte֮�����������zSql����ʣ������ݣ���ô��Щʣ������ݱ���ŵ�pZTail�У���������ֹ������� pszTail ��Ϊ NULL, �� pszTail ָ�� sql �е�һ��������� SQL ���Ľ�β. �ú���ֻ���� sql �ĵ�һ�����, ���� pszTail ָ�����������δ������ġ�  
Return: ���ִ�гɹ����򷵻�SQLITE_OK�����򷵻�һ�������롣
  
### sqlite3_step
```c
int sqlite3_step(sqlite3_stmt *stmt);
```
Stmt: ����һ�λ��߶�α�������������  
Return: SQLITE_BUSY��SQLITE_DONE��SQLITE_ROW��SQLITE_ERROR��SQLITE_MISUSE��  
SQLITE_BUSY: æµ. ���ݿ������޷���������ȥ����乤��. �����Զ�γ���.  
SQLITE_DONE: ���. sql ����Ѿ����ɹ���ִ��.�ڵ��� sqlite_reset() ֮ǰ,��ǰԤ�������䲻Ӧ�ñ�sqlite3_step()�ٴε���.  
SQLITE_ROW: ��ѯʱ�����˽��. ��ʱ����ͨ����ص�"���ݷ��ʺ���(column access functions)"��ȡ������. sqlite3_step()����һ�ε��ý�ȡ����һ����ѯ���.  
SQLITE_ERROR: �����˴���. ��ʱ����ͨ�� sqlite3_errmmsg() ȡ����صĴ�����Ϣ. sqlite3_step() ���ܱ��ٴε���.  
SQLITE_MISUSE: ����ȷ�Ŀ��ʹ��. �ú���ʹ�ò���.  

### sqlite3_reset
```c
int sqlite3_reset (sqlite3_stmt *stmt);
```
sqlite3_reset��������һ��׼�����������ĳ�ʼ״̬��Ȼ��׼��������ִ�С�����sql������ʹ��sqlite3_bind_XXX��ֵ��ʹ��sqlite3_clear_bindings�����Щ�󶨡� Sqlite3_reset�ӿ�����׼����䵽�����뿪ʼ��ʱ��sqlite3_reset�����ı���׼������ϵ��κΰ�ֵ����ô����²⣬����������ڱ�ִ�еĹ����з����������ĸı䣬Ȼ�������佫�����õ���ֵ��ʱ����Ǹ�״̬��  
Return: SQLITE_BUSY,SQLITE_DONE��SQLITE_ROW��  
SQLITE_BUSY: ��ʱ�޷�ִ�в���.  
SQLITE_DONE: ����ִ�����.  
SQLITE_ROW: ִ����ϲ����з��أ�ִ��select���ʱ��;��Ҫ�Բ�ѯ������д���SQLITE3�ṩsqlite3_column_*ϵ�к���.  

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
�ú�������ִ�ж����ԡ�;���ָ��� SQL ��䡣��װ�� sqlite3_prepare(), sqlte3_step() �� sqlite3_finalize() ������  
db: ���ݿ����Ӿ����  
sql: ��Ҫ�������SQL��䡣  
callback: �ص����������ú����ǿգ���ÿ��SQL�﷨�Ľ��������øú�������Ϊ�գ��򽫱����ԡ�  
arg: �ص������ĵ�һ��������  
errmsg: �洢������Ϣ����ΪNULL;  
Return: ���ط��㣬�ú��������жϲ�ѯ��������ִ�к�����SQL���ͻص�����;����SQLITE_ABORT����ִ�С�  
```c
int callback(void *arg, int argc, char **argv, char **azColName);
```
arg: Ϊsqlite3_exec�еĵ��ĸ����������������ݵĲ�����  
argc: ִ��SQL�﷨����������Ԫ�ظ�����  
argv: ִ��SQL�﷨����������Ԫ���б�argv[i]�����i��Ԫ�ء�  
azColName: ִ��SQL�﷨ÿ���и��е�������  

### sqlite3_bind_XXX
```c
int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_int(sqlite3_stmt*, int, int);
int sqlite3_bind_null(sqlite3_stmt*, int);
int sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
```
��������ֵ��  
stmt: ָ��һ�������������������ɱ�sqlite3_stepִ�С�ִ�д�����������zSqlΪ���ַ���*ppStmtΪNULL����sqlite3_finalize���ͷ�����  
index: SQL����(ÿ��Ԫ��)����������0��ʼ.  
value: �ַ���ֵ��  
len: �ַ������ȡ�  
fun: һ������ָ�룬SQLITE3ִ���������ص��˺�����ͨ�������ͷ��ַ���ռ�õ��ڴ档�˲���������������SQLITE_STATIC����sqlite3_bind_text�����ַ���Ϊ���������Է���ʹ�ã���SQLITE_TRANSIENT��ʹ��sqlite3_bind_text�������ַ�����һ�ݿ�����  

### sqlite3_column_XXX
```c
double sqlite3_column_double(sqlite3_stmt*, int iCol);
int sqlite3_column_int(sqlite3_stmt*, int iCol);
const unsigned char *sqlite3_column_text(sqlite3_stmt*, int iCol);
int sqlite3_column_type(sqlite3_stmt*, int iCol);
```
���ຯ����sqlite3_step()���صĽ��������ȡĳһ�С�  
stmt: ָ��һ�������������������ɱ�sqlite3_stepִ�С�ִ�д�����������zSqlΪ���ַ���*ppStmtΪNULL����sqlite3_finalize���ͷ�����  
index: SQL����(ÿ��Ԫ��)����������1��ʼ.  

## ʾ��

### �����ݿ�
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

### ����Ŀ�����ݱ�
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

### ���ݱ����
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
    
    /* ��ʵ������ܣ��ͽ�ע��ȥ�� */
    //char *SQL = "INSERT INTO testtable VALUES(NULL, 'xiao', 'male', 100, 0.1)";//�����ݱ��в���һ������
        //char *SQL = "DELETE FROM testtable WHERE gender = males AND age = 26";//ɾ�����ݱ��еĶ�������
        //char *SQL = "DELETE FROM testtable";//ɾ�����ݱ�����������
        //char *SQL = "UPDATE testtable SET gender = female,age=30 WHERE ID = 1";//�������ݱ��еĶ�������
    //char *SQL = "DROP TABLE IF EXISTS testtable";//ɾ�����ݱ�
        
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

### ������������
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

        /* �ֶ��������� */
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

        /* �ֶ��ر����� */
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

### ��ʾ���ݱ������(non call_back)
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
