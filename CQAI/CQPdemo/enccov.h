/*
 | Char encode convent
 | �ļ�����: enccov.h
 | �ļ�����: ����ת��
 | ��������: 2020-02-26
 | ��������: 2020-02-26
 | ������Ա: JuYan
 +--------------------------
 Copyright (C) JuYan, all rights reserved.
 WINDOWS ONLY
*/
#ifndef _INCLUDE_ENCCOV_H_
#define _INCLUDE_ENCCOV_H_
#define freeString(s)      free(s)              // ʹ�øú��ͷŷ��ص��ַ����� ���
// GB2312 => UTF-8
extern char * acpToUTF8(const char *acp);
// UTF-8 => GB2312
extern char * utf8ToACP(const char *utf8);
// Unicode => GB2312
extern char * utf16ToACP(const wchar_t * utf16);
// Unicode => utf-8
extern char * utf16ToUTF8(const wchar_t * utf16);
// utf-8 => Unicode
extern wchar_t * utf8ToUTF16(const char* utf8);
// GB2312 => Unicode
extern wchar_t * acpToUTF16(const char* acp);
// ����ת��
extern void escapeCharA(char **p);
extern void escapeCharW(wchar_t **p);
// ��ת��
extern void oppositeEscapeCharA(char *r);
extern void oppositeEscapeCharW(wchar_t *r);
#endif // _INCLUDE_ENCCOV_H_


