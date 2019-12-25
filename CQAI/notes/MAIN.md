# ��Q SDK ���ο����ʼǱ�����˵��

## ��Qbase64��Ϣ����(������)

ע:**��һ����ֱ�ӳ�����, ����[��Q��̳](https://cqp.cc/t/28730)**

��������Ķ��������ᵽ��base64�������Ϣ������Ҫ����base64���룬���涼�����ᣬ���������ݶ��ǽ���֮��ġ���ش�����Բο�[��Q��̳](https://cqp.cc/t/26287)

* İ������Ϣ

��**CQ_getStrangerInfo**���ص���Ϣ���Ѿ���ӹ����ѵ�Ҳһ����  
> ǰ8���ֽڣ���һ��Int64_t���ȣ�QQ�ţ�  
> ������2���ֽڣ���һ��short���ȣ��ǳƳ��ȣ�  
> �������ǳƳ��ȸ��ֽڣ��ǳ��ı���  
> ������4���ֽڣ���һ��int���ȣ��Ա�0��1Ů��  
> ������4���ֽڣ���һ��int���ȣ����䣬QQ�ﲻ��ֱ���޸����䣬�Գ�����Ϊ׼��

�ṹ�ο���

```C++
struct CQ_TYPE_QQ
{
        int64_t        QQID;       //QQ��
        std::string    nick;       //�ǳ�
        int            sex;        //�Ա�
        int            age;        //����
};
```

* Ⱥ��Ա��Ϣ

��**CQ_getGroupMemberInfoV2**���ص���Ϣ

> ǰ8���ֽڣ���һ��Int64_t���ȣ�QQȺ�ţ�  
> ������8���ֽڣ���һ��Int64_t���ȣ�QQ�ţ�  
> ������2���ֽڣ���һ��short���ȣ��ǳƳ��ȣ�  
> �������ǳƳ��ȸ��ֽڣ��ǳ��ı���  
> ������2���ֽڣ���һ��short���ȣ�Ⱥ��Ƭ���ȣ�  
> ������Ⱥ��Ƭ���ȸ��ֽڣ�Ⱥ��Ƭ�ı���  
> ������4���ֽڣ���һ��int���ȣ��Ա�0��1Ů��  
> ������4���ֽڣ���һ��int���ȣ����䣬QQ�ﲻ��ֱ���޸����䣬�Գ�����Ϊ׼��  
> ������2���ֽڣ���һ��short���ȣ��������ȣ�  
> �������������ȸ��ֽڣ������ı���  
> ������4���ֽڣ���һ��int���ȣ���Ⱥʱ�����  
> ������4���ֽڣ���һ��int���ȣ������ʱ�����  
> ������2���ֽڣ���һ��short���ȣ�Ⱥ�ȼ����ȣ�  
> ������Ⱥ�ȼ����ȸ��ֽڣ�Ⱥ�ȼ��ı���  
> ������4���ֽڣ���һ��int���ȣ�����Ȩ�ޣ�1��Ա��2����Ա��3Ⱥ����  
> ������4���ֽڣ���һ��int���ȣ�0����֪����ʲô�������ǲ�����¼��Ա��  
> ������2���ֽڣ���һ��short���ȣ�ר��ͷ�γ��ȣ�  
> ������ר��ͷ�γ��ȳ��ȸ��ֽڣ�ר��ͷ�γ����ı���  
> ������4���ֽڣ���һ��int���ȣ�ר��ͷ�ι���ʱ�����  
> ������4���ֽڣ���һ��int���ȣ������޸���Ƭ��1�����²�0�ǲ�����  
�ṹ�ο���

```C++
struct CQ_Type_GroupMember
{
        int64_t        GroupID;            // Ⱥ��
        int64_t        QQID;               // QQ��
        std::string    nick;               // QQ�ǳ�
        std::string    card;               // Ⱥ��Ƭ
        int            sex;                // �Ա� 0/�� 1/Ů
        int            age;                // ����
        std::string    area;               // ����
        int            jointime;           // ��Ⱥʱ��
        int            lastsent ;          // �ϴη���ʱ��
        std::string    level_name;         // ͷ������
        int            permission;         // Ȩ�޵ȼ� 1/��Ա 2/����Ա 3/Ⱥ��
        bool           unfriendly;         // ������Ա��¼
        std::string    title;              // �Զ���ͷ��
        int            titleExpiretime;    // ͷ�ι���ʱ��
        bool           cardcanchange;      // ����Ա�Ƿ���Э������
};
```

* ������Ϣ

��**__eventGroupMsg**��**fromAnonymous**����  
> ǰ8���ֽڣ���һ��Int64_t���ȣ�������ʶ�ţ�  
> ������2���ֽڣ���һ��short���ȣ��������Ƴ��ȣ�  
> �������������Ƴ��ȸ��ֽڣ����������ı���  
> ������2���ֽڣ���һ��short���ȣ�token���ȣ�Ŀǰ������40�ֽڣ�  
> ������token���ȸ��ֽڣ�token���ݣ�  

## SQLite ���Ӵ��������

```C
#define SQLITE_OK           0   /* �ɹ� | Successful result */
/* �����뿪ʼ */
#define SQLITE_ERROR        1   /* SQL���� �� ��ʧ���ݿ� | SQL error or missing database */
#define SQLITE_INTERNAL     2   /* SQLite �ڲ��߼����� | Internal logic error in SQLite */
#define SQLITE_PERM         3   /* �ܾ����� | Access permission denied */
#define SQLITE_ABORT        4   /* �ص���������ȡ������ | Callback routine requested an abort */
#define SQLITE_BUSY         5   /* ���ݿ��ļ������� | The database file is locked */
#define SQLITE_LOCKED       6   /* ���ݿ��е�һ�������� | A table in the database is locked */
#define SQLITE_NOMEM        7   /* ĳ�� malloc() ��������ʧ�� | A malloc() failed */
#define SQLITE_READONLY     8   /* ����д��һ��ֻ�����ݿ� | Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* ������ sqlite3_interupt() �����ж� | Operation terminated by sqlite3_interrupt() */
#define SQLITE_IOERR       10   /* ����ĳЩ���� I/O ���� | Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* ���ݿ����ӳ����ȷ | The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* sqlite3_file_control() �г���δ֪������ | Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* ��Ϊ���ݿ������²���ʧ�� | Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* �޷������ݿ��ļ� | Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* ���ݿ�����Э����� | Database lock protocol error */
#define SQLITE_EMPTY       16   /* ���ݿ�Ϊ�� | Database is empty */
#define SQLITE_SCHEMA      17   /* ���ݽṹ�����ı� | The database schema changed */
#define SQLITE_TOOBIG      18   /* �ַ�������������ݳ�����С���� | String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* ����Լ��Υ����ȡ�� | Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* �������Ͳ�ƥ�� | Data type mismatch */
#define SQLITE_MISUSE      21   /* ����ȷ�Ŀ�ʹ�� | Library used incorrectly */
#define SQLITE_NOLFS       22   /* ʹ���˲���ϵͳ��֧�ֵĹ��� | Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* ��Ȩʧ�� | Authorization denied */
#define SQLITE_FORMAT      24   /* �������ݿ��ʽ���� | Auxiliary database format error */
#define SQLITE_RANGE       25   /* ���ݸ�sqlite3_bind()�ĵڶ�������������Χ | 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* ���򿪵��ļ�����һ�����ݿ��ļ� | File opened that is not a database file */
#define SQLITE_ROW         100  /* sqlite3_step() �Ѿ�����һ���н�� | sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() ���ִ�в��� | sqlite3_step() has finished executing */
/* ��������� */
```
## ~~C++ʵ�ּ��׶�ʱ��(������)~~

~~ͷ�ļ�"timer.hpp"����[CSDN](https://blog.csdn.net/u012234115/article/details/89857431)~~  

~~ʾ���÷�:~~

����: 20 Dec. 2019  **ȡ����"C++ʵ�ּ��׶�ʱ��"**, ������win32event, ʹ��"Global\"���ε��¼��������ڶ�����̼�ʹ��, ���㴥������. (��Ҳ��һ������Ҳ��δ���: �����WaitForSingleObject�ڼ��������˳�, ���򽫻����²����߳��޷���ȫ�˳�. Ŀǰ�Ľ��������Ӧ���������˳�ʱ�������SetEvent, ������ʵ������ʱ�򲢲���Ч, Ҳ��ֻ�ܵȳ�ʱ֮���Լ��˳���.)  

## ���ݿ����

Ŀǰ�ƻ�����һ��db, ����2�����3������:`event`��`relationship`; `Activ`, `power`, `priority`.

* `event`:

```sql
CREATE TABLE `event` (
    `EID`     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `TYPE`    INTEGER NOT NULL,
    `TIME`    NUMERIC NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `LINK`    INTEGER,
    `CONT`    TEXT,
    `NOTE`    TEXT,
    `STATUS`  INTEGER
);
/*
type���¼�������: 10xxϵ��Ԥ������Q��APP���л����������; 5xxxΪ�Զ����¼�, ����ʱ��ʱ/�м���̵�, ϸ�����; 6xxxΪ��Q��Ӧ����Ϣ��QQ�¼�;  
link����Ӧ��msgId: ����¼���QQ��Ϣ�й�, �Ǿ�ȥ��Q�Լ�����־�����msgId��, �൱�ڸ�����õ����.  
cont����������: ��Ҫ��������ں�˴��ָ����ǰ�˶�, ��˴���֮ǰ�ͷ�ԭʼ��Ϣ���ݵ�Ƭ��(Ŀǰ�����128�ַ�)�Ա����.  
note����ע/����: ��ŵ�����Ϣ������־.  
status��״̬��: ��ֵԽ��Խ��Ҫ. 0-�Ѿ������; 200-���������账��; 233-��˴������; 266-�����ȴ�, ���綨ʱ����֮���; 300-������Ⱥ��Ϣ; 301-������˽����Ϣ; 302-����������; 401-��ת��������Ա; 500-������  
*/
```

* `relationship`:

```sql
CREATE TABLE `relationship` (
    `QQ`        INTEGER NOT NULL UNIQUE,
    `Nickname`  TEXT NOT NULL,
    `level`     INTEGER NOT NULL,
    `amity`     INTEGER NOT NULL,
    `from`      TEXT,
    `note`      TEXT,
    `lastActiv` NUMERIC DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(`QQ`)
);
/*
QQ��QQ�û���: Ψһ����, ÿ�μӺ��ѵ�ʱ��ӿ�Q API��ȡ¼��.  
Nickname����ע��: Ĭ��¼��Ӻ���ʱ�����, ��˳��ԴӸ�������ȡ�˼ʹ�ϵ��ע���ɹ���Աȷ�Ϻ����.  
level��Ȩ�޵ȼ�: 0Ϊ����Ա, 250Ϊ�¼Ӻ���, 254Ϊ������. ��ֵԽ��Ȩ�޺����ȼ�Խ��.  
amity���Ѻö�: -250Ϊ���, 10Ϊ�¼Ӻ���, 250Ϊ����.  
from����Դ��ע: Ĭ���ǿ�Q API�ṩ����Դ, ��˳��ԴӸ�������ȡ��Դ��ע���ɹ���Աȷ�Ϻ����.  
note����ע: Ĭ�Ͽ�, �ɺ��ά��.  
lastActiv�����һ�λ�Ծʱ��: ���ȴ��������Ծ���ѵ�����.  
*/
```

* ����:

```sql
CREATE INDEX `Activ` ON `relationship` ( `lastActiv` DESC );
CREATE INDEX `power` ON `relationship` ( `level` ASC );
CREATE INDEX `priority` ON `event` ( `STATUS` DESC );
```

���ȴ�����Ҫ��/��Ȩ�޵�/���µ���Ϣ

## Ӧ������������

����: 22 Dec. 2019 �ǳ���л[���](https://github.com/JuYanYan)�İ���, ��һ���ֽ����������ƵĽ�����, �������ϸ��Ҳ���ع�.

-----

��˳����Լ��������Ա����Q API������Ӧ�þ����ܾ���/�߿ɶ���/�ʵ��ݴ���/�������ֻ�������.  
���㰴����ԭ����һ���򵥵������Զ�����ʵ�ָ����ʽ������.  
�۲�CQP.dll�е�����Q API�����б�CQP.def, ��Q API�ṩ�Ķ��������ܷ�Ϊ"����", "����", "��ȡ" ������; ���������ܷ�Ϊ"��Ϣ", "�û�", "����". :  

1. ����:

> _sendPrivateMsg        ����˽����Ϣ, �ɹ�������ϢID  
> _sendGroupMsg          ����Ⱥ��Ϣ, �ɹ�������ϢID  
> _sendLike              �����ֻ���  

2. ����:

> _deleteMsg             ������Ϣ  
> _setGroupKick          �Ƴ�ȺԱ  
> _setGroupBan           ���ý���  
> _setGroupAdmin         ����Ⱥ����Ա  
> _setGroupWholeBan      ����ȫԱ����  
> _setGroupAnonymousBan  ��������ȺԱ  
> _setGroupAnonymous     ����Ⱥ��������  
> _setGroupCard          ����ȺԱ���  
> _setGroupLeave         ��Ⱥ  
> _setGroupSpecialTitle  ����ר��ͷ��  
> _setFriendAddRequest   ���ú����������ͬ���ܾ�  
> _setGroupAddRequestV2  ���ü�Ⱥ����ͬ���ܾ�  
> _addLog                ��ӿ�Q�������־  
> _setFatal              ������������(ֹͣ����)  

3. ��ȡ:

> _getGroupMemberInfoV2  ȡȺ��Ա��Ϣ  
> _getStrangerInfo       ȡİ������Ϣ  
> _getLoginQQ            ȡ��¼QQ  
> _getAppDirectory       ȡӦ��(��CQ��data�������Ŀ¼, ����Ӧ���ļ������)Ŀ¼  
> _getLoginNick          ȡ��¼QQ�ǳ�  
> _getRecord             ��������  
> _getGroupMemberList    ȡȺ��Ա��Ϣ�б�  
> _getGroupList          ȡȺ�б�  

�ɴ�, ��Ƶĸ�ʽ�����Դ�����ʽ:  
"��Ӧ���� �������� ������ϵ �������� ����ID �������� [�޶�����]"  

һ��ֻ����һ�����, �����з�Ϊ�Ͼ��, �������ַ�����?��Χ�����з������ǿ�ѡ, Ĭ�����ѡ��  
ʾ��:

```e
���� ��Ϣ �� ���� 616471607 ?ʾ��1?      // ������ `send msg to 616471607 ?example1?`  
send msg to grp 483537882 ?example2?  // ������ `�� ��Ϣ �� Ⱥ 483537882 ?ʾ��2?`  
�� �ֻ��� �� ���� 616471607 10           // `�� �� �� 616471607`  

���� ��Ϣ ���� Ⱥ 483537882 ?��������Ϣ? ������=616471607  
set �û� �Ƴ� Ⱥ 2139223150 ��Ⱥ=483537882  
���� �û� ���� ȺԱ 2139223150 233 ����Ⱥ=2139223150 // ��������������, 233��. ���û����������, Ĭ�ϲ������б������Ⱥ  
���� �û� ���� ȺԱ 2139223150 �� ����Ⱥ=483537882 // ��/�� 1/0 ��/α  
���� �û� ���� Ⱥ 483537882 233  
���� �û� ���� ȺԱ ?��������? 233 ����Ⱥ=483537882  
���� �û� ���� Ⱥ 483537882 1  
���� �û� ��Ƭ ȺԱ 2139223150  
���� usr �Ƴ� (�Լ�) ��Ⱥ=483537882  
���� �û� ͷ�� ȺԱ 2139223150 ?��~?  
���� �û� ���� ���� 616471607 ͬ��  
���� �û� ���� ȺԱ 616471607 ͬ�� ����Ⱥ=483537882  
���� ���� ��־ ���� ?����?  
���� ���� ��־ ���� ��������  

��ȡ �û� ��Ϣ ȺԱ 2139223150 ��Ⱥ=483537882  
��ȡ �û� ��Ϣ İ���� 616471607  
��ȡ �û� ��Ϣ �Լ� ?QQ?  
��ȡ �û� ��Ϣ �Լ� ?�ǳ�?  
��ȡ ��Ϣ ���� ?�����ļ���?  
��ȡ �û� �б� Ⱥ 483537882  
��ȡ ���� Ⱥ�б�  
��ȡ ���� Ŀ¼  

blackList 1816533856 // ��Qû���ṩ���ڵ�API, Ϊ�˾���ɧ�ź͹��ں���Ϣ, �Լ���һ��������.  
```

## ���ֿ�Q API����ֵ

ע:**�ⲿ�����Լ����Թ۲������, ��Ϊû�ҵ��ٷ��ĵ���˵��**

```note
0   ��Ƭ�޷��ͳɹ�(���ƺ��ǲ���ȷ����û�е��)
0   ������Ϣ�ɹ�
-38 ����ʧ�ܣ��������ʺŴ�����ʺŲ��ڸ�Ⱥ���ڣ�
-42 ����ʧ�ܣ�Ȩ�޲��㣩
```
