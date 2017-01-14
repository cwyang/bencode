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

extern be_node_t *be_decode(const char *inBuf, size_t inBufLen, size_t *readAmount);
extern ssize_t be_encode(const be_node_t *node, char *outBuf, size_t outBufLen);
extern void be_free(be_node_t *node);
extern void be_dump(be_node_t *node);

#define BE_MAX_DEPTH 10 // max depth of composite type (list and dict)

#define BE_MALLOC malloc
#define BE_CALLOC calloc
#define BE_FREE free
#define BE_ASSERT assert
#endif
