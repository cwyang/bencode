/* set ts=4 sw=4 enc=utf-8: -*- Mode: c; tab-width: 4; c-basic-offset:4; coding: utf-8 -*- */
/*
 * Copyright 2017 Chul-Woong Yang (cwyang@gmail.com)
 * Licensed under the Apache License, Version 2.0;
 * See LICENSE for details.
 *
 * list.h
 * 14 January 2017, cwyang@gmail.com
 *
 * Linux compatible doubly linked list.
 *
 */

#ifndef SIMPLE_LIST_H
#define SIMPLE_LIST_H

typedef struct list_head {
    struct list_head *next, *prev;
} list_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) list_t name = LIST_HEAD_INIT(name)

static inline list_t* init_list_head(list_t *head) {
    head->next = head->prev = head;
    return head;
}

static inline void list_add(list_t *new, list_t *head) {
    new->prev = head;
    new->next = head->next;
    head->next->prev = new;
    head->next = new;
}

static inline void list_add_tail(list_t *new, list_t *head) {
    new->prev = head->prev;
    new->next = head;
    head->prev->next = new;
    head->prev = new;
}

static inline void list_del(list_t *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

static inline int list_empty(list_t *head) {
    return head->next == head;
}

#ifdef container_of
#define list_entry container_of
#else
#define list_entry(ptr, type, member)                               \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#endif

#define list_for_each(POS, HEAD)                                        \
    for ((POS) = (HEAD)->next; (POS) != (HEAD); (POS) = (POS)->next)
            
#define list_for_each_safe(POS, TEMP, HEAD)                    \
    for ((POS) = (HEAD)->next, (TEMP) = (POS)->next;           \
         (POS) != (HEAD);                                      \
         (POS) = (TEMP), (TEMP) = (POS)->next)

#endif // SIMPLE_LIST_H
