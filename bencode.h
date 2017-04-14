/* set ts=4 sw=4 enc=utf-8: -*- Mode: c; tab-width: 4; c-basic-offset:4; coding: utf-8 -*- */
/*
 * Copyright 2017 Chul-Woong Yang (cwyang@gmail.com)
 * Licensed under the Apache License, Version 2.0;
 * See LICENSE for details.
 *
 * bencode.h
 * 14 January 2017, cwyang@gmail.com
 *
 * Bittorrent bencode reader and writer module
 *
 */

#ifndef BENCODE_H
#define BENCODE_H

#include "list.h"

typedef struct be_str { // data encoding of bencode can be anything,
    char *buf;          // hence we keep length of the string
    long long int len;
} be_str_t;

typedef struct be_dict {
    list_t link;
    be_str_t key;
    struct be_node *val;
} be_dict_t;

typedef struct be_node {
    list_t link;
    enum be_type { STR, NUM, LIST, DICT } type;
    union {
        be_str_t str;
        long long int num;
        list_t list_head;
        list_t dict_head;
    } x;
} be_node_t;

/** MAIN APIs **/
extern be_node_t *be_decode(const char *inBuf, size_t inBufLen, size_t *readAmount);
extern ssize_t be_encode(const be_node_t *node, char *outBuf, size_t outBufLen);
extern be_node_t *be_alloc(enum be_type type);
extern void be_free(be_node_t *node);
extern void be_dump(be_node_t *node);

/** DICT APIs **/
extern void be_dict_free(be_dict_t *dict);
extern be_node_t *be_dict_lookup(be_node_t *node, const char *key, be_dict_t **dict_entry);
extern long long int be_dict_lookup_num(be_node_t *node, const char *key);
extern char *be_dict_lookup_cstr(be_node_t *node, const char *key);
extern char *be_dict_lookup_cstr_size(be_node_t *node, const char *key, int *size);
extern int be_dict_add(be_node_t *dict, const char *keystr, be_node_t *val);
extern int be_dict_add_str(be_node_t *dict, const char *keystr, char *valstr);
extern int be_dict_add_str_with_len(be_node_t *dict, const char *keystr, char *valstr, int len);
extern int be_dict_add_num(be_node_t *dict, const char *keystr, long long int valnum);

#define BE_MAX_DEPTH 10 // max depth of composite type (list and dict)

#define BE_MALLOC malloc
#define BE_CALLOC calloc
#define BE_FREE(x) do { if (x) free(x); x = NULL; } while (0)
#define BE_STRDUP strdup
#define BE_ASSERT assert
#endif
