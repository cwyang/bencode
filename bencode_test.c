/* set ts=4 sw=4 enc=utf-8: -*- Mode: c; tab-width: 4; c-basic-offset:4; coding: utf-8 -*- */
/*
 * Copyright 2017 Chul-Woong Yang (cwyang@gmail.com)
 * Licensed under the Apache License, Version 2.0;
 * See LICENSE for details.
 *
 * bencode_test.c
 * 15 January 2017, cwyang@gmail.com
 *
 * Bittorrent bencode reader and writer module
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bencode.h"

const char *sample="d8:announce35:udp://tracker.openbittorrent.com:8013:creation datei1327049827e4:infod6:lengthi20e4:name10:sample.txt12:piece lengthi65536e6:pieces20:..R....x...d.......17:privatei1eee";

int main(void) 
{
    be_node_t *node;
    size_t len = strlen(sample), rx;
    ssize_t n, n2;
    char *buf;

    node = be_decode(sample, len, &rx);
    BE_ASSERT(len == rx); // should consume all

    n = be_encode(node, NULL, 0);
    BE_ASSERT(n == len);  // should correctly calc
    
    buf = BE_MALLOC(n+1);
    n2 = be_encode(node, buf, n);
    buf[n] = '\0';

    BE_ASSERT(strncmp(sample, buf, len) == 0);
    BE_ASSERT(n == n2);    // should be same

    BE_FREE(buf);
    
    be_dump(node);
    be_free(node);

    printf("All tests passed!\n");

    return 0;
}
