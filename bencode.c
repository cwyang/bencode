/* set ts=4 sw=4 enc=utf-8: -*- Mode: c; tab-width: 4; c-basic-offset:4; coding: utf-8 -*- */
/*
 * Copyright 2017 Chul-Woong Yang (cwyang@gmail.com)
 * Licensed under the Apache License, Version 2.0;
 * See LICENSE for details.
 *
 * bencode.c
 * 14 January 2017, cwyang@gmail.com
 *
 * Bittorrent bencode reader and writer module
 *
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.h"

#define EAT(BUF,LEN) (BUF)++,(LEN)--
#define EAT_N(BUF,LEN,N) (BUF)+=(N),(LEN)-=(N)

//#define BE_DEBUG

static be_node_t *be_alloc(enum be_type type) {
    be_node_t *ret = BE_CALLOC(1, sizeof(be_node_t));
    if (ret) {
        ret->type = type;
        init_list_head(&ret->link);
        if (type == LIST)
            init_list_head(&ret->x.list_head);
        else if (type == DICT)
            init_list_head(&ret->x.dict_head);
    }
    return ret;
}

void be_free(be_node_t *node) {
    list_t *l, *tmp;

    if (node == NULL)
        return;
    
    list_del(&node->link);
    switch (node->type) {
    case STR:
        BE_FREE(node->x.str.buf);
        break;
    case NUM:
        break;
    case LIST:
        list_for_each_safe(l, tmp, &node->x.list_head) {
            be_node_t *entry = list_entry(l, be_node_t, link);
            be_free(entry);
        }
        break;
    case DICT:
        list_for_each_safe(l, tmp, &node->x.list_head) {
            be_dict_t *entry = list_entry(l, be_dict_t, link);
            BE_FREE(entry->key.buf);
            be_free(entry->val);
            BE_FREE(entry);
        }
        break;
    default:
        assert(0);
        break;
    }
    BE_FREE(node);
}

/* Parse until non-digit marker occurs.
   'e' = '-e' = '00e' = ':' = '-:' = '00:'= 0
   '999999999999999999999999999999999999999999999999999999999e' = LLONG_MAX
   '-99999999999999999999999999999999999999999999999999999999e' = LLONG_MIN
   '--e' = error (leaves '-e')
   '123' = error (consumes all)
*/
static long long int be_decode_int(const char *buf, size_t len, size_t *rx) {
    size_t orglen = len;
    long long int ret = 0;
    int sign = 1, overflowed = 0;

#ifdef BE_DEBUG
    printf("%s: %s %d\n", __FUNCTION__, buf, (int) len);
#endif

    if (*buf == '-') {
        sign = -1;
        EAT(buf,len);
    }
    
    while (len > 0) {
        if (!isdigit(*buf))
            break;
        if (!overflowed) {
            long long int tmp = ret;
            ret = ret * 10;
            ret += (*buf - '0') * sign;
            if (sign == 1 && ret < tmp) { // overflow
                overflowed = 1;
                ret = LLONG_MAX;
            } else if (sign == -1 && ret > tmp) { // underflow
                overflowed = 1;
                ret = LLONG_MIN;
            }
        } 
        EAT(buf,len);
    }

    *rx = orglen - len;
    return ret;
}

static be_str_t be_decode_str(const char *buf, size_t len, size_t *rx) {
    char *ret;
    size_t orglen = len, n;
    be_str_t str = { .buf = NULL, .len = 0 };

#ifdef BE_DEBUG
    printf("%s: %s %d\n", __FUNCTION__, buf, (int) len);
#endif

    long long int slen = be_decode_int(buf, len, &n);

    EAT_N(buf,len,n);
    
    if ((len == 0) ||
        (slen < 0 || slen > len - 1) ||
        (*buf != ':'))
        goto out;

    if ((ret = BE_MALLOC(slen + 1)) == NULL)
        goto out;

    str.len = slen;
    str.buf = ret;
    memcpy(ret, buf + 1, slen);
    ret[slen] = '\0';
    EAT_N(buf,len,slen+1);
    
out:
    *rx = orglen - len;
    return str;
}

#define DO_ERR(CODE) do { errno = CODE; goto out; } while (0)
#define ALLOC(T) do {                           \
        ret = be_alloc(T);                      \
        if (ret == NULL) DO_ERR(ENOMEM);        \
    } while (0)
#define CHECK2(COND, CODE) do {                 \
        if (COND) {                             \
            be_free(ret);                       \
            ret = NULL;                         \
            DO_ERR(CODE);                       \
        }                                       \
    } while (0)
#define CHECK(COND) CHECK2(COND,EINVAL)
#define EAT_CHECK(BUF,LEN) do {                 \
        EAT(BUF,LEN);                           \
        CHECK(((LEN) == 0));                    \
    } while (0)

static be_node_t *be_decode1(const char *buf, size_t len, size_t *rx, int depth) {
    size_t orglen = len, n;
    be_node_t *ret = NULL, *entry;

#ifdef BE_DEBUG
    printf("%s: %s %d %d\n", __FUNCTION__, buf, (int) len, depth);
#endif
    
    if (depth > BE_MAX_DEPTH) {   // recursion threshold exceeded
        errno = ELOOP;
        goto out;
    }
    
    if (len == 0) {
        errno = EINVAL;
        goto out;
    }

    switch (*buf) {
    case 'i':
        ALLOC(NUM);
        EAT_CHECK(buf, len);
        ret->x.num = be_decode_int(buf, len, &n); // we parse "i-0e" as 0
        EAT_N(buf,len,n);
        CHECK((*buf != 'e'));
        EAT(buf, len);
        break;
    case '0'...'9':
        ALLOC(STR);
        ret->x.str = be_decode_str(buf, len, &n); // "0" should be return ""
        EAT_N(buf,len,n);
        CHECK((ret->x.str.buf == NULL));
        break;
    case 'l':
        ALLOC(LIST);
        EAT_CHECK(buf,len);
        while (*buf != 'e') { // "le" return empty list
            entry = be_decode1(buf, len, &n, depth+1);
            EAT_N(buf,len,n);
            CHECK((entry == NULL));
            list_add_tail(&entry->link, &ret->x.list_head);
            CHECK((len == 0));
        }
        EAT(buf,len);
        break;
    case 'd':
        ALLOC(DICT);
        EAT_CHECK(buf,len);
        while (*buf != 'e') { // "de" return empty dictionary
            be_dict_t *dict_entry = BE_CALLOC(1, sizeof(be_dict_t));
            CHECK2((dict_entry == NULL), ENOMEM);
            init_list_head(&dict_entry->link);
            list_add_tail(&dict_entry->link, &ret->x.dict_head); // early link 

            dict_entry->key = be_decode_str(buf, len, &n);
            EAT_N(buf,len,n);
            CHECK((dict_entry->key.buf == NULL));
            dict_entry->val = be_decode1(buf, len, &n, depth+1);
            EAT_N(buf,len,n);
            CHECK((dict_entry->val == NULL));
            CHECK((len == 0));
        }
        EAT(buf,len);
        break;
    default: // error
        errno = EINVAL;
        goto out;
    }

out:
    *rx = orglen - len;
    return ret;
}

/* when error, errno is set:
   ENOMEM: malloc failed
   ELOOP:  recursion threshold met
   EINVAL: bad format bencode file
*/
be_node_t *be_decode(const char *inBuf, size_t inBufLen, size_t *readAmount) {
    return be_decode1(inBuf, inBufLen, readAmount, 1);
}

static void newline(int indent) {
    int i;
    putchar('\n');
    for (i = 0; i < indent; i++)
        putchar(' ');
}
static int be_str_dump(be_str_t *str) 
{
    int i;
    printf("\"");
    for (i = 0; i < str->len; i++) {
        if (isprint(str->buf[i]))
            printf("%c", str->buf[i]);
        else
            printf(".");
    }
    printf("\"");
    return 2 + str->len;
}
static void be_dump1(be_node_t *node, int indent) {
    int sz, first = 1;
    list_t *l;
    
    switch (node->type) {
    case NUM:
        printf("%lld", node->x.num);
        break;
    case STR:
        be_str_dump(&node->x.str);
        break;
    case LIST:
        list_for_each(l, &node->x.list_head) {
            be_node_t *entry = list_entry(l, be_node_t, link);
            if (first) {
                printf("[ ");
                first = 0;
            } else {
                newline(indent);
                printf (", ");
            }
            be_dump1(entry, indent+2);
        }
        printf("]");
        break;
    case DICT:
        list_for_each(l, &node->x.list_head) {
            be_dict_t *entry = list_entry(l, be_dict_t, link);
            if (first) {
                printf("{ ");
                first = 0;
            } else {
                newline(indent);
                printf (", ");
            }
            sz = be_str_dump(&entry->key);
            printf(": ");
            be_dump1(entry->val, indent+sz+4);
        }
        printf("}");
    }
}

void be_dump(be_node_t *node) {
    be_dump1(node, 0);
    printf("\n");
}

#define TMPBUFLEN 32    /* enough to hold LLONG_MAX and some chars */

static ssize_t be_encode_str(const be_str_t *str, char *outBuf, size_t outBufLen) 
{
    char tmpBuf[TMPBUFLEN];
    int sz = snprintf(tmpBuf, TMPBUFLEN, "%lld:", str->len);

    if (outBuf == NULL)
        return sz + str->len;

    if (sz + str->len > outBufLen)
        return -1;
    
    memcpy(outBuf, tmpBuf, sz);
    memcpy(outBuf+sz, str->buf, str->len);
    return sz + str->len;
}

/*
  when outBuf == NULL, be_encode returns outBufLen needed 
*/
ssize_t be_encode(const be_node_t *node, char *outBuf, size_t outBufLen) {
    char tmpBuf[TMPBUFLEN];
    ssize_t r;
    int sz = 1;
    list_t *l;

    if (outBuf && outBufLen == 0)
        return -1;
    
    switch (node->type) {
    case NUM:
        sz = snprintf(tmpBuf, TMPBUFLEN, "i%llde", node->x.num);
        if (outBuf != NULL) {
            if (sz > outBufLen)
                return -1;
            strncpy(outBuf, tmpBuf, sz);
        }
        break;
    case STR:
        sz = be_encode_str(&node->x.str, outBuf, outBufLen);
        if (sz < 0)
            return -1;
        break;
    case LIST:
        if (outBuf) {
            *outBuf = 'l';
            EAT(outBuf, outBufLen);
        }
        list_for_each(l, &node->x.list_head) {
            be_node_t *entry = list_entry(l, be_node_t, link);
            r = be_encode(entry, outBuf, outBufLen);
#define CHECK_AND_UPDATE do {                               \
                if (r < 0) return -1;                       \
                if (outBuf) EAT_N(outBuf, outBufLen, r);    \
                sz += r;                                    \
            } while (0)
            CHECK_AND_UPDATE;
        }
        
        if (outBuf) {
            if (outBufLen == 0)
                return -1;
            *outBuf = 'e';
        }
        sz++;
        break;
    case DICT:
        if (outBuf) {
            *outBuf = 'd';
            EAT(outBuf, outBufLen);
        }
        list_for_each(l, &node->x.list_head) {
            be_dict_t *entry = list_entry(l, be_dict_t, link);

            r = be_encode_str(&entry->key, outBuf, outBufLen);
            CHECK_AND_UPDATE;
            
            r = be_encode(entry->val, outBuf, outBufLen);
            CHECK_AND_UPDATE;
        }
        if (outBuf) {
            if (outBufLen == 0)
                return -1;
            *outBuf = 'e';
        }
        sz++;
        break;
    }
    return sz;
}
