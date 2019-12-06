> 推荐 C++ 开发者选用以下 SDK，事件及 Api 的实现较本项目更为完善，更新更为及时：
>
> * [richardchien/coolq-cpp-sdk](https://github.com/richardchien/coolq-cpp-sdk)
> * [MikuPy2001/CQSDK](https://github.com/MikuPy2001/CQSDK)
>
> 本项目将保持一定程度的维护，留作参考之用。
>
> 其它语言的 SDK 请参考：[酷Q 应用开发 V9 SDK](https://cqp.cc/t/15124)

# cqsdk-vc [![Build status](https://ci.appveyor.com/api/projects/status/b45ik9dass1rnrnj?svg=true)](https://ci.appveyor.com/project/Coxxs/cqsdk-vc)

使用 Visual C++ 编写 酷Q V9 应用。

[下载](https://github.com/CoolQ/cqsdk-vc/archive/master.zip)

文件说明
--------
`CQPdemo.sln` - 示例项目，可以直接在此基础上编写应用

您可以编译为 `app.dll`，与 `CQPdemo/app.json` 一起放置在 酷Q 的 `dev/com.example.democ/` 目录下测试

`CQPdemo/app.json` - 样例应用的对应信息文件，包含应用的基础信息、事件列表等，请放置在 `dev/com.example.democ/` 目录下（无需使用的事件、菜单、权限请在此删除）

`CQPdemo/cqp.h` - 酷Q SDK 头文件，通常无需修改

`CQPdemo/CQP.lib` - CQP.dll 的动态连接 .lib 文件，便于 C、C++ 等调用 酷Q 的方法。

关于内存释放
----------
酷Q V9 应用机制中包含了一部分涉及字符串 `const char*` 的参数及返回值。酷Q V9 中均要求内存申请方负责释放。

具体规则如下：

* 对于 Event 的字符串参数，酷Q 将负责在事件函数返回后立即释放。
  * 在部分场景下，参数可能传入空指针 `NULL`，应用应判断该情况，避免读取空指针发生致命错误。
* 对于 Api 的字符串**参数**，应用负责在调用完后进行释放。
* 对于 Api 的字符串**返回值**，酷Q 将负责在合适的时机进行释放。
  * 应用不应对字符串**返回值**进行释放，否则一段时间后，酷Q 尝试释放该内存时将发生致命错误。
  * 应用应在调用 Api 后之后立即使用返回的指针，避免 酷Q 释放内存后，应用使用该指针时发生致命错误。
  * 如果 Api 调用失败，将返回空指针 `NULL`，应用应判断该情况，避免读取空指针发生致命错误。

官方网站
--------
主站：https://cqp.cc

文库：https://docs.cqp.im
