@echo off
color 0A
title ������
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo            ʹ��˵��
echo.
echo ����ĿĿǰ��Ϊ������Ϊ��������, ǰ��ʹ�ÿ�Q��Ϊ�û��ӿ�, ���Ϊ����չ��Ӧ�û�Ӧ����, �м��ʹ��python��Ԥ�����Ӧ��ѡ��. ���в��ֲ���app.db, ͨ��win32eventͨ��.

:STA
echo   ---------------------------
echo ����׼�������м��ű�, ��ȷ��python3�������������Ѿ���ȷ��װpywin32��...
set /p input=׼���ú�����Y������
if "%input%"=="Y"(goto RUNscript) else (goto STA)

:RUNscript
echo ���������ű�...
start python PyDeamon/PyDeamon.py

:confm
echo   ---------------------------
set /p input=�Ƿ񵯳����´��ڲ���ʾ����ʼ����?(Y/N)��
if "%input%"=="Y" (goto OKToexit) else if "%input%"=="N" (goto ErrToexit) else (goto confm)

:OKToexit
echo.
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo �м�����ϸ��־������debug.log��
echo ���������ֶ�������Q��ȷ��ǰ��APP����������, ���һ�������Ϳ��԰�������رմ˴�����~
TIMEOUT /T 30
exit

:ErrToexit
echo.
echo   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo ��ô�ټ�����л������԰�...���������˳�...
python -c "import win32event; CQexit = win32event.CreateEvent(None, False, False, 'Global\\CQexit'); win32event.SetEvent(CQexit); win32event.SetEvent(CQexit);"
TIMEOUT /T 10
