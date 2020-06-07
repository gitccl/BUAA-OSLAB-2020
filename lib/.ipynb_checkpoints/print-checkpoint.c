/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include	<print.h>

/* macros */
#define		IsDigit(x)	( ((x) >= '0') && ((x) <= '9') )
#define		Ctod(x)		( (x) - '0')

/* forward declaration */
extern int PrintChar(char *, char, int, int);
extern int PrintString(char *, char *, int, int);
extern int PrintNum(char *, unsigned long, int, int, int, int, char, int);

/* private variable */
static const char theFatalMsg[] = "fatal error in lp_Print!";

/* -*-
 * A low level printf() function.
 */
void
lp_Print(void (*output)(void *, char *, int), void * arg, char *fmt,  va_list ap)
{

#define 	OUTPUT(arg, s, l)  \
  { if (((l) < 0) || ((l) > LP_MAX_BUF)) { \
       (*output)(arg, (char*)theFatalMsg, sizeof(theFatalMsg)-1); for(;;); \
    } else { \
      (*output)(arg, s, l); \
    } \
  }
    
    char buf[LP_MAX_BUF];

    char c;
    char *s;
    long int num;

    int longFlag; //标记是否为long型
    int negFlag; //标记是否为负数
    int width; //标记输出宽度
    int prec; //标记小数精度
    int ladjust; // 标记是否左对齐
    char padc; //填充多余位置所用的字符

    int length;

    for(;;) {
	
        { 
            /* scan for the next '%' */
            while((*fmt) && *fmt != '%') {
                OUTPUT(arg, fmt, 1);
                fmt ++ ;
            }  
            /* flush the string found so far */
            
            /* are we hitting the end? */
            if((*fmt) == 0 ) break;
        }
        /*init the variable */
        longFlag = 0;
        negFlag = 0;
        width = 0;
        ladjust = 0;
        prec = 0;
        padc = ' ';

        /* we found a '%' */
        fmt ++ ;
        /* check for long */
        // if( *fmt == 'l' ) {
        //     longFlag = 1;
        //     fmt ++ ;
        // } 
        /* check for other prefixes */
        /*check for flag */
        if(*fmt == '-') ladjust = 1, fmt ++;
        else if(*fmt == '0') padc = '0' , fmt ++;

        /*check for width */
        while( IsDigit(*fmt) ) {
            width = width * 10 + Ctod(*fmt) ;
            fmt ++ ;
        }

        /*check for precision */
        if(*fmt == '.') 
        {
            fmt ++ ;
            while(IsDigit(*fmt))
            {
                prec = prec * 10 + Ctod(*fmt);
                fmt ++ ;
            }
        }
        /* check for long */
        if( *fmt == 'l' ) {
            longFlag = 1;
            fmt ++ ;
        } 
        
        /* check format flag */
        negFlag = 0;
        switch (*fmt) {
        case 'b':
            if (longFlag) { 
                num = va_arg(ap, long int); 
            } else { 
                num = va_arg(ap, int);
            }
            length = PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
            OUTPUT(arg, buf, length);
            break;

        case 'd':
        case 'D':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            if (num < 0) {
                num = - num;
                negFlag = 1;
            }
            length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0);
            OUTPUT(arg, buf, length);
            break;

        case 'o':
        case 'O':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
            OUTPUT(arg, buf, length);
            break;

        case 'u':
        case 'U':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
            OUTPUT(arg, buf, length);
            break;
            
        case 'x':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
            OUTPUT(arg, buf, length);
            break;

        case 'X':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
            OUTPUT(arg, buf, length);
            break;

        case 'c':
            c = (char)va_arg(ap, int);
            length = PrintChar(buf, c, width, ladjust);
            OUTPUT(arg, buf, length);
            break;

        case 's':
            s = (char*)va_arg(ap, char *);
            length = PrintString(buf, s, width, ladjust);
            OUTPUT(arg, buf, length);
            break;

        case '\0':
            fmt --;
            break;

        default:
            /* output this char as it is */
            OUTPUT(arg, fmt, 1);
        }	/* switch (*fmt) */

        fmt ++;
    }		/* for(;;) */

    /* special termination call */
    OUTPUT(arg, "\0", 1);
}


/* --------------- local help functions --------------------- */
int
PrintChar(char * buf, char c, int length, int ladjust)
{
    int i;
    
    if (length < 1) length = 1;
    if (ladjust) {
	    *buf = c;
	    for (i = 1; i < length; i ++) buf[i] = ' ';
    } else {
        for (i=0; i < length - 1; i ++) buf[i] = ' ';
        buf[length - 1] = c;
    }
    return length;
}

int
PrintString(char * buf, char* s, int length, int ladjust)
{
    int i;
    int len=0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    if (ladjust) {
        for (i=0; i< len; i++) buf[i] = s[i];
        for (i=len; i< length; i++) buf[i] = ' ';
    } else {
        for (i=0; i< length-len; i++) buf[i] = ' ';
        for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    }
    return length;
}

int
PrintNum(char * buf, unsigned long u, int base, int negFlag, 
	 int length, int ladjust, char padc, int upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int actualLength =0;
    char *p = buf;
    int i;

    do {
        int tmp = u %base;
        if (tmp <= 9) {
            *p++ = '0' + tmp;
        } else if (upcase) {
            *p++ = 'A' + tmp - 10;
        } else {
            *p++ = 'a' + tmp - 10;
        }
        u /= base;
    } while (u != 0);

    if (negFlag) {
	    *p++ = '-';
    }
    //最终整个字符串要翻转
    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    if (ladjust) {
	    padc = ' ';//左对齐就不能填充0
    }
    if (negFlag && !ladjust && (padc == '0')) {
        /*右对齐，同时要填充前导0，那就填充0(把字符'-'覆盖) */
        for (i = actualLength-1; i < length-1; i++) buf[i] = padc; 
        buf[length -1] = '-';//第一位填'-'
    } else {
	    for (i = actualLength; i < length; i++) buf[i] = padc;
    }
	    

    /* prepare to reverse the string */
    {
        int begin = 0;
        int end;
        if (ladjust) {
            end = actualLength - 1;//如果是左对齐，只需要翻转数字部分(包括负号)
        } else {
            end = length -1;
        }

        while (end > begin) {
            char tmp = buf[begin];
            buf[begin] = buf[end];
            buf[end] = tmp;
            begin ++;
            end --;
        }
    }

    /* adjust the string pointer */
    return length;
}
