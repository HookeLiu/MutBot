/*
 | Char encode convent
 | 文件名称: enccov.h
 | 文件作用: 编码转换
 | 创建日期: 2020-02-26
 | 更新日期: 2020-02-26
 | 开发人员: JuYan
 +--------------------------
 Copyright (C) JuYan, all rights reserved.
 WINDOWS ONLY
*/
#ifndef _INCLUDE_ENCCOV_H_
#define _INCLUDE_ENCCOV_H_
#define freeString(s)      free(s)              // 使用该宏释放返回的字符串， 务必
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
// 正向转义
extern void escapeCharA(char **p);
extern void escapeCharW(wchar_t **p);
// 反转义
extern void oppositeEscapeCharA(char *r);
extern void oppositeEscapeCharW(wchar_t *r);
#endif // _INCLUDE_ENCCOV_H_


