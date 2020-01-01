@echo off
color 0A
title 启动向导
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo            使用说明
echo.
echo 本项目目前分为两个较为独立部分, 前端使用酷Q作为用户接口, 后端为可扩展的应用或应用组, 中间层使用python做预处理和应用选择. 所有部分操作app.db, 通过win32event通信.

:STA
echo   ---------------------------
echo 现在准备运行中间层脚本, 请确保python3环境正常并且已经正确安装pywin32包...
set /p input=准备好后输入Y继续：
if "%input%"=="Y"(goto RUNscript) else (goto STA)

:RUNscript
echo 正在启动脚本...
start python PyDeamon/PyDeamon.py

:confm
echo   ---------------------------
set /p input=是否弹出了新窗口并提示程序开始运行?(Y/N)：
if "%input%"=="Y" (goto OKToexit) else if "%input%"=="N" (goto ErrToexit) else (goto confm)

:OKToexit
echo.
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo 中间层的详细日志保存在debug.log里
echo 接下来请手动启动酷Q并确保前端APP已正常运行, 如果一切正常就可以按任意键关闭此窗口了~
TIMEOUT /T 30
exit

:ErrToexit
echo.
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo 那么再检查运行环境重试吧...程序现在退出...
python -c "import win32event; CQexit = win32event.CreateEvent(None, False, False, 'Global\\CQexit'); win32event.SetEvent(CQexit); win32event.SetEvent(CQexit);"
TIMEOUT /T 10
