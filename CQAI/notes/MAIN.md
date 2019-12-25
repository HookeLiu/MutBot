# 酷Q SDK 二次开发笔记备忘和说明

## 酷Qbase64信息解析(抄来的)

注:**这一节是直接抄来的, 来自[酷Q论坛](https://cqp.cc/t/28730)**

这里解析的都是上面提到的base64编码的信息，首先要经过base64解码，下面都不再提，分析的数据都是解码之后的。相关代码可以参考[酷Q论坛](https://cqp.cc/t/26287)

* 陌生人信息

即**CQ_getStrangerInfo**返回的信息，已经添加过好友的也一样。  
> 前8个字节，即一个Int64_t长度，QQ号；  
> 接下来2个字节，即一个short长度，昵称长度；  
> 接下来昵称长度个字节，昵称文本；  
> 接下来4个字节，即一个int长度，性别，0男1女；  
> 接下来4个字节，即一个int长度，年龄，QQ里不能直接修改年龄，以出生年为准；

结构参考：

```C++
struct CQ_TYPE_QQ
{
        int64_t        QQID;       //QQ号
        std::string    nick;       //昵称
        int            sex;        //性别
        int            age;        //年龄
};
```

* 群成员信息

即**CQ_getGroupMemberInfoV2**返回的信息

> 前8个字节，即一个Int64_t长度，QQ群号；  
> 接下来8个字节，即一个Int64_t长度，QQ号；  
> 接下来2个字节，即一个short长度，昵称长度；  
> 接下来昵称长度个字节，昵称文本；  
> 接下来2个字节，即一个short长度，群名片长度；  
> 接下来群名片长度个字节，群名片文本；  
> 接下来4个字节，即一个int长度，性别，0男1女；  
> 接下来4个字节，即一个int长度，年龄，QQ里不能直接修改年龄，以出生年为准；  
> 接下来2个字节，即一个short长度，地区长度；  
> 接下来地区长度个字节，地区文本；  
> 接下来4个字节，即一个int长度，入群时间戳；  
> 接下来4个字节，即一个int长度，最后发言时间戳；  
> 接下来2个字节，即一个short长度，群等级长度；  
> 接下来群等级长度个字节，群等级文本；  
> 接下来4个字节，即一个int长度，管理权限，1成员，2管理员，3群主；  
> 接下来4个字节，即一个int长度，0，不知道是什么，可能是不良记录成员；  
> 接下来2个字节，即一个short长度，专属头衔长度；  
> 接下来专属头衔长度长度个字节，专属头衔长度文本；  
> 接下来4个字节，即一个int长度，专属头衔过期时间戳；  
> 接下来4个字节，即一个int长度，允许修改名片，1允许，猜测0是不允许；  
结构参考：

```C++
struct CQ_Type_GroupMember
{
        int64_t        GroupID;            // 群号
        int64_t        QQID;               // QQ号
        std::string    nick;               // QQ昵称
        std::string    card;               // 群名片
        int            sex;                // 性别 0/男 1/女
        int            age;                // 年龄
        std::string    area;               // 地区
        int            jointime;           // 入群时间
        int            lastsent ;          // 上次发言时间
        std::string    level_name;         // 头衔名字
        int            permission;         // 权限等级 1/成员 2/管理员 3/群主
        bool           unfriendly;         // 不良成员记录
        std::string    title;              // 自定义头衔
        int            titleExpiretime;    // 头衔过期时间
        bool           cardcanchange;      // 管理员是否能协助改名
};
```

* 匿名信息

即**__eventGroupMsg**的**fromAnonymous**参数  
> 前8个字节，即一个Int64_t长度，匿名标识号；  
> 接下来2个字节，即一个short长度，匿名名称长度；  
> 接下来匿名名称长度个字节，匿名名称文本；  
> 接下来2个字节，即一个short长度，token长度，目前看都是40字节；  
> 接下来token长度个字节，token内容；  

## SQLite 连接错误码对照

```C
#define SQLITE_OK           0   /* 成功 | Successful result */
/* 错误码开始 */
#define SQLITE_ERROR        1   /* SQL错误 或 丢失数据库 | SQL error or missing database */
#define SQLITE_INTERNAL     2   /* SQLite 内部逻辑错误 | Internal logic error in SQLite */
#define SQLITE_PERM         3   /* 拒绝访问 | Access permission denied */
#define SQLITE_ABORT        4   /* 回调函数请求取消操作 | Callback routine requested an abort */
#define SQLITE_BUSY         5   /* 数据库文件被锁定 | The database file is locked */
#define SQLITE_LOCKED       6   /* 数据库中的一个表被锁定 | A table in the database is locked */
#define SQLITE_NOMEM        7   /* 某次 malloc() 函数调用失败 | A malloc() failed */
#define SQLITE_READONLY     8   /* 尝试写入一个只读数据库 | Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* 操作被 sqlite3_interupt() 函数中断 | Operation terminated by sqlite3_interrupt() */
#define SQLITE_IOERR       10   /* 发生某些磁盘 I/O 错误 | Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* 数据库磁盘映像不正确 | The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* sqlite3_file_control() 中出现未知操作数 | Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* 因为数据库满导致插入失败 | Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* 无法打开数据库文件 | Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* 数据库锁定协议错误 | Database lock protocol error */
#define SQLITE_EMPTY       16   /* 数据库为空 | Database is empty */
#define SQLITE_SCHEMA      17   /* 数据结构发生改变 | The database schema changed */
#define SQLITE_TOOBIG      18   /* 字符串或二进制数据超过大小限制 | String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* 由于约束违例而取消 | Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* 数据类型不匹配 | Data type mismatch */
#define SQLITE_MISUSE      21   /* 不正确的库使用 | Library used incorrectly */
#define SQLITE_NOLFS       22   /* 使用了操作系统不支持的功能 | Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* 授权失败 | Authorization denied */
#define SQLITE_FORMAT      24   /* 附加数据库格式错误 | Auxiliary database format error */
#define SQLITE_RANGE       25   /* 传递给sqlite3_bind()的第二个参数超出范围 | 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* 被打开的文件不是一个数据库文件 | File opened that is not a database file */
#define SQLITE_ROW         100  /* sqlite3_step() 已经产生一个行结果 | sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() 完成执行操作 | sqlite3_step() has finished executing */
/* 错误码结束 */
```
## ~~C++实现简易定时器(抄来的)~~

~~头文件"timer.hpp"来自[CSDN](https://blog.csdn.net/u012234115/article/details/89857431)~~  

~~示例用法:~~

更新: 20 Dec. 2019  **取消了"C++实现简易定时器"**, 改用了win32event, 使用"Global\"修饰的事件名可以在多个进程间使用, 方便触发操作. (但也有一个问题也仍未解决: 如果在WaitForSingleObject期间主程序退出, 程序将会留下残余线程无法完全退出. 目前的解决方案是应用启动和退出时多给几次SetEvent, 但发现实际上有时候并不起效, 也就只能等超时之后自己退出了.)  

## 数据库设计

目前计划是用一个db, 包含2个表和3个索引:`event`和`relationship`; `Activ`, `power`, `priority`.

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
type→事件的类型: 10xx系列预留给酷Q及APP运行环境相关内容; 5xxx为自定义事件, 如临时计时/中间过程等, 细则待定; 6xxx为酷Q对应的消息和QQ事件;  
link→对应的msgId: 如果事件和QQ消息有关, 那就去酷Q自己的日志库根据msgId找, 相当于给后端用的外键.  
cont→交换内容: 主要设计是用于后端存放指令让前端读, 后端处理之前就放原始消息内容的片段(目前设计是128字符)以便调试.  
note→备注/调试: 存放调试信息或者日志.  
status→状态码: 数值越大越重要. 0-已经处理过; 200-正常且无需处理; 233-后端处理完成; 266-继续等待, 比如定时提醒之类的; 300-待处理群消息; 301-待处理私聊消息; 302-待处理请求; 401-待转发给管理员; 500-待调试  
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
QQ→QQ用户号: 唯一主键, 每次加好友的时候从酷Q API获取录入.  
Nickname→备注名: 默认录入加好友时的马甲, 后端尝试从附言中提取人际关系备注并由管理员确认后更新.  
level→权限等级: 0为管理员, 250为新加好友, 254为黑名单. 数值越大权限和优先级越低.  
amity→友好度: -250为厌恶, 10为新加好友, 250为亲密.  
from→来源备注: 默认是酷Q API提供的来源, 后端尝试从附言中提取来源备注并由管理员确认后更新.  
note→备注: 默认空, 由后端维护.  
lastActiv→最后一次活跃时间: 优先处理最近活跃好友的内容.  
*/
```

* 索引:

```sql
CREATE INDEX `Activ` ON `relationship` ( `lastActiv` DESC );
CREATE INDEX `power` ON `relationship` ( `level` ASC );
CREATE INDEX `priority` ON `event` ( `STATUS` DESC );
```

优先处理重要的/高权限的/较新的消息

## 应答命令规则设计

更新: 22 Dec. 2019 非常感谢[燕儿](https://github.com/JuYanYan)的帮助, 这一部分将会做成完善的解释器, 部分设计细节也会重构.

-----

后端程序以及人类管理员给酷Q API的命令应该尽可能精简/高可读性/适当容错性/方便在手机上输入.  
打算按编译原理做一个简单的有限自动机来实现个表达式解释器.  
观察CQP.dll中导出酷Q API函数列表CQP.def, 酷Q API提供的动作大致能分为"发送", "设置", "获取" 三大类; 按内容由能分为"消息", "用户", "环境". :  

1. 发送:

> _sendPrivateMsg        发送私聊消息, 成功返回消息ID  
> _sendGroupMsg          发送群消息, 成功返回消息ID  
> _sendLike              发送手机赞  

2. 设置:

> _deleteMsg             撤回消息  
> _setGroupKick          移除群员  
> _setGroupBan           设置禁言  
> _setGroupAdmin         设置群管理员  
> _setGroupWholeBan      设置全员禁言  
> _setGroupAnonymousBan  禁言匿名群员  
> _setGroupAnonymous     设置群匿名开关  
> _setGroupCard          设置群员马甲  
> _setGroupLeave         退群  
> _setGroupSpecialTitle  设置专属头衔  
> _setFriendAddRequest   设置好友添加请求同意或拒绝  
> _setGroupAddRequestV2  设置加群请求同意或拒绝  
> _addLog                添加酷Q自身的日志  
> _setFatal              设置致命错误(停止处理)  

3. 获取:

> _getGroupMemberInfoV2  取群成员信息  
> _getStrangerInfo       取陌生人信息  
> _getLoginQQ            取登录QQ  
> _getAppDirectory       取应用(在CQ的data里的数据目录, 不是应用文件本身的)目录  
> _getLoginNick          取登录QQ昵称  
> _getRecord             接收语音  
> _getGroupMemberList    取群成员信息列表  
> _getGroupList          取群列表  

由此, 设计的格式化语言大致形式:  
"响应类型 内容类型 操作关系 操作对象 对象ID 操作内容 [限定条件]"  

一次只接受一条语句, 即换行符为断句符, 变量及字符串用?包围。其中方括号是可选, 默认最大选择。  
示例:

```e
发送 消息 给 好友 616471607 ?示例1?      // 或者是 `send msg to 616471607 ?example1?`  
send msg to grp 483537882 ?example2?  // 或者是 `发 消息 给 群 483537882 ?示例2?`  
发 手机赞 给 好友 616471607 10           // `发 赞 给 616471607`  

设置 消息 撤回 群 483537882 ?包含的消息? 发送者=616471607  
set 用户 移除 群 2139223150 从群=483537882  
设置 用户 禁言 群员 2139223150 233 禁言群=2139223150 // 操作内容是秒数, 233秒. 如果没有限制条件, 默认操作所有被管理的群  
设置 用户 管理 群员 2139223150 真 设置群=483537882 // 是/否 1/0 真/伪  
设置 用户 禁言 群 483537882 233  
设置 用户 禁言 群员 ?匿名名称? 233 被控群=483537882  
设置 用户 匿名 群 483537882 1  
设置 用户 名片 群员 2139223150  
设置 usr 移除 (自己) 从群=483537882  
设置 用户 头衔 群员 2139223150 ?喵~?  
设置 用户 请求 好友 616471607 同意  
设置 用户 请求 群员 616471607 同意 设置群=483537882  
设置 环境 日志 调试 ?测试?  
设置 环境 日志 错误 致命错误  

获取 用户 信息 群员 2139223150 从群=483537882  
获取 用户 信息 陌生人 616471607  
获取 用户 信息 自己 ?QQ?  
获取 用户 信息 自己 ?昵称?  
获取 消息 语音 ?语音文件号?  
获取 用户 列表 群 483537882  
获取 环境 群列表  
获取 环境 目录  

blackList 1816533856 // 酷Q没有提供拉黑的API, 为了拒收骚扰和公众号消息, 自己做一个黑名单.  
```

## 部分酷Q API返回值

注:**这部分是自己测试观察整理的, 因为没找到官方文档和说明**

```note
0   名片赞发送成功(但似乎是不能确定有没有点成)
0   撤回消息成功
-38 发送失败（接收者帐号错误或帐号不在该群组内）
-42 撤回失败（权限不足）
```
