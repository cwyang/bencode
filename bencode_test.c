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

const char *sample="d4:testl4:teste8:announce35:udp://tracker.openbittorrent.com:8013:creation datei1327049827e4:infod6:lengthi20e4:name10:sample.txt12:piece lengthi65536e6:pieces20:..R....x...d.......17:privatei1eee";

const char *valid_samples[] = {
    /* from specs */
    "4:spam",
    "0:",
    "i3e",
    "i-3e",
    "i0e",
    "l4:spam4:eggse",
    "le",
    "d3:cow3:moo4:spam4:eggse",
    "d4:spaml1:a1:bee",
    "d9:publisher3:bob17:publisher-webpage15:www.example.com18:publisher.location4:homee",
    "de",
    /* other cases */
    "llllllll4:testeeeeeeee",
    NULL
};

/* spec does not allow, but we parse them for robustness */
const char *loose_samples[] = {
    "i9999999999999999999999999999999999999999999999999e", /* parse to LLONG_MAX */
    "i03e",
    "i-0e",
    NULL
};
    
const char *invalid_samples[] = {
    "1234",
    "i1234",
    "12:abc",
    "i--0e",
    "llllllllllll4:testeeeeeeeeeeee", // too many depth
    NULL
};

enum check_type { STRICT, LOOSE };

static void should_pass(const char *c, enum check_type type) 
{
    be_node_t *node;
    size_t len = strlen(c), rx;
    ssize_t n, n2;
    char *buf;

    printf("<%s>\n", c);
    
    node = be_decode(c, len, &rx);
    BE_ASSERT(node != NULL);
    if (type == STRICT)
        BE_ASSERT(len == rx); // should consume all

    n = be_encode(node, NULL, 0);
    if (type == STRICT)
        BE_ASSERT(n == len);  // should correctly calc
    
    buf = BE_MALLOC(n+1);
    n2 = be_encode(node, buf, n);
    buf[n] = '\0';

    BE_ASSERT(n == n2);    // should be same
    if (type == STRICT)
        BE_ASSERT(strncmp(c, buf, len) == 0);

    BE_FREE(buf);
    be_free(node);
}

static void should_fail(const char *c) 
{
    be_node_t *node;
    size_t len = strlen(c), rx;
    
    printf("<%s>\n", c);

    node = be_decode(c, len, &rx);
    BE_ASSERT(node == NULL);
}

int main(void) 
{
    be_node_t *node;
    size_t len = strlen(sample), rx;
    ssize_t n, n2;
    char *buf;
    const char **c;

    printf("* should pass - strict\n");
    for (c = &valid_samples[0]; *c != NULL; c++)
        should_pass(*c, STRICT);

    printf("\n* should pass - loose\n");
    for (c = &loose_samples[0]; *c != NULL; c++)
        should_pass(*c, LOOSE);

    printf("\n* should fail\n");
    for (c = &invalid_samples[0]; *c != NULL; c++)
        should_fail(*c);
    
    printf("\n* composite pattern\n");
    printf("<%s>\n", sample);
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
    
    printf("\n* dumping above data\n");
    be_dump(node);
    be_free(node);

    printf("\nAll tests passed!\n");

    return 0;
}
