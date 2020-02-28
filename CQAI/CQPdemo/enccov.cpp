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
#include "stdafx.h"
// UTF-8 => GB2312
extern char * utf8ToACP(const char *utf8)
{
    char *r;
    wchar_t *p;
    p = utf8ToUTF16(utf8);
    r = utf16ToACP(p);
    freeString(p);
    return r;
}
// GB2312 => UTF-8
extern char * acpToUTF8(const char *acp)
{
    char *r;
    wchar_t *p;
    p = acpToUTF16(acp);
    r = utf16ToUTF8(p);
    freeString(p);
    return r;
}
// Unicode => GB2312
extern char * utf16ToACP(const wchar_t * utf16)
{
    int len;
    char *astr;
    len = WideCharToMultiByte(CP_ACP, 0, utf16, -1, NULL, 0, NULL, NULL);
    astr = (char*)malloc(sizeof(char) * (len + 1));
    assert(astr);
    WideCharToMultiByte(CP_ACP, 0, utf16, -1, astr, len, NULL, NULL);
    astr[len] = '\0';
    return astr;
}
// Unicode => utf-8
extern char * utf16ToUTF8(const wchar_t * utf16)
{
    int len;
    char *astr;
    len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, NULL, NULL);
    astr = (char*)malloc(sizeof(char) * (len + 1));
    assert(astr);
    WideCharToMultiByte(CP_UTF8, 0, utf16, -1, astr, len, NULL, NULL);
    astr[len] = '\0';
    return astr;
}
// utf-8 => Unicode
extern wchar_t * utf8ToUTF16(const char* utf8)
{
    int len;
    wchar_t* wstr;
    len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wstr = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    assert(wstr);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    wstr[len] = L'\0';
    return wstr;
}
// GB2312 => Unicode
extern wchar_t * acpToUTF16(const char* acp)
{
    int len;
    wchar_t* wstr;
    len = MultiByteToWideChar(CP_ACP, 0, acp, -1, NULL, 0);
    wstr = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    assert(wstr);
    MultiByteToWideChar(CP_ACP, 0, acp, -1, wstr, len);
    wstr[len] = L'\0';
    return wstr;
}
// 字符转义 - 因为多写太讨厌了所以就template了, 请保证输入有效
// &→&amp;  ]→&#93;  [→&#91;  %→%%  \→\\  @→&#64;  '→'' // ?→&que;
template<typename chr> static void escapeChar(chr **r, const chr *src)
{
    int   sz = 0;
    chr  *res = NULL, *pres;
    const chr *pstr = src;
    while (*pstr != 0)                                  // 首先计算大小
    {
        if (*pstr == '&' || *pstr == '[' || *pstr == ']')
        {
            sz += 5;                                    // 转义输出的大小是5
        }
        else {
            sz += 1;
        }
        pstr++;
    }
    assert(sz > 0);
    res = (chr*)malloc(sizeof(chr) * (sz + 1));
    assert(res);
    pres = res;
    pstr = src;
    while (*pstr != 0)                                  // 转义字符
    {
        switch (*pstr)
        {
        case '&':
            *pres++ = '&';
            *pres++ = 'a';
            *pres++ = 'm';
            *pres++ = 'p';
            *pres++ = ';';
            break;
        case '[':
            *pres++ = '&';
            *pres++ = '#';
            *pres++ = '9';
            *pres++ = '1';
            *pres++ = ';';
            break;
        case ']':
            *pres++ = '&';
            *pres++ = '#';
            *pres++ = '9';
            *pres++ = '3';
            *pres++ = ';';
            break;
        case '%':
            *pres++ = '%';
            *pres++ = '%';
            break;
        case '@':
            *pres++ = '&';
            *pres++ = '#';
            *pres++ = '6';
            *pres++ = '4';
            *pres++ = ';';
            break;
//        case '?':
//            *pres++ = '&';
//            *pres++ = 'q';
//            *pres++ = 'u';
//            *pres++ = 'e';
//            *pres++ = ';';
//            break;
        case '\'':
            //if ((pstr > src&&* (pstr - 1) != ' ') && *pstr != '\0' && *(pstr + 1) != ',') {
            //    *pres++ = '\'';
            //    *pres++ = '\'';
            //}
            //else {
            //    *pres++ = *pstr;
            //}
            *pres++ = '\'';
            *pres++ = '\'';
            break;
        default:
            *pres++ = *pstr;
            break;
        }
        pstr++;
    }
    *pres = 0;
    *r = res;
}
// char和wchar_t版本的字符转义
extern void escapeCharA(char **p)
{
    char *r;
    escapeChar<char>(&r, *p);
    freeString(*p);
    *p = r;
}
extern void escapeCharW(wchar_t **p)
{
    wchar_t *r;
    escapeChar<wchar_t>(&r, *p);
    freeString(*p);
    *p = r;
}
// 反转义 - 因为多写太讨厌了所以就template了, 请保证输入有效
//  \\→\  &#64;→@  ''→' // &que;→?
template<typename chr> static void oppositeEscapeChar(chr *r)
{
    chr  *read = r;
    chr  *write = r;
    while (*read != 0)
    {
        if (read[0] == '&' && read[1] == '#' && read[2] == '6' && read[3] == '4' && read[4] == ';')
        {
            *write++ = '@';
            read += 5;
        }
        else if (read[0] == '\'' && read[1] == '\'')
        {
            *write++ = '\'';
            read += 2;
        }
        else if (read[0] == '%' && read[1] == '%')
        {
            *write++ = '%';
            read += 2;
        }
//        else if (read[0] == '&' && read[1] == 'q' && read[2] == 'u' && read[3] == 'e' && read[4] == ';')
//        {
//            *write++ = '?';
//            read += 5;
//        }

        else {
            *write = *read;
            read++;
            write++;
        }
    }
    *write = 0;
}
// char和wchar_t版本的字符反转义
extern void oppositeEscapeCharA(char *r)
{
    oppositeEscapeChar<char>(r);
}
extern void oppositeEscapeCharW(wchar_t *r)
{
    oppositeEscapeChar<wchar_t>(r);
}
