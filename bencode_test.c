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
#include <netinet/in.h>
#include <arpa/inet.h>

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
    "d3:cowi123ee",    
    "d4:spaml1:a1:bee",
    "d9:publisher3:bob17:publisher-webpage15:www.example.com18:publisher.location4:homee",
    "de",
    /* other cases */
    "llllllll4:testeeeeeeee",
    "d8:intervali1800e5:peers6:" "\x0a\x02\x14\x0b\x1a\xe2" "e",
    NULL
};

/* spec does not allow, but we parse them for robustness */
const char *loose_samples[] = {
    "i99999999999999999999999999999999999999999999999999e", /* parse to LLONG_MAX */
    "i-9999999999999999999999999999999999999999999999999e", /* parse to LLONG_MIN */
    "i03e",
    "i-0e",
    NULL
};
    
const char *invalid_samples[] = {
    "",
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

static void gen_dict_bt_resp() 
{
    int i;
    be_node_t *node, *dict, *peers;
    char buf[INET6_ADDRSTRLEN], *c;
    size_t n;
    uint32_t v4;
    uint8_t v6[16];

    peers = be_decode("le", 2, &n);
    BE_ASSERT(peers != NULL);

    for (i = 0; i < 16; i++)
        v6[i] = i;
    v4=htonl(0x0a000001);
    
#define NUM_PEERS 50
    for (i = 0; i < NUM_PEERS; i++) {
        dict = be_alloc(DICT);
        BE_ASSERT(dict != NULL);
        if (0) {
            inet_ntop(AF_INET6, v6, buf, INET6_ADDRSTRLEN);
        } else {
            inet_ntop(AF_INET, &v4, buf, INET_ADDRSTRLEN);
        }
        be_dict_add_str(dict, "ip", buf);
        be_dict_add_str(dict, "id", "12345678901234567890");
        be_dict_add_num(dict, "port", ntohs(53764));
        list_add_tail(&dict->link, &peers->x.list_head);
    }
    node = be_alloc(DICT);
    BE_ASSERT(node != NULL);
    be_dict_add(node, "peers", peers);

//    be_dump(node);
    n = be_encode(node, NULL, 0);
    c = BE_MALLOC(n+1);
    BE_ASSERT(n > 0 && c != NULL);
    n = be_encode(node, c, n);
    c[n] = '\0';
    
    BE_FREE(c);
    
    be_free(node);
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

    gen_dict_bt_resp();
    
    printf("\nAll tests passed!\n");

    return 0;
}
