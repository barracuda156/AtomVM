/*
 * This file is part of AtomVM.
 *
 * Copyright 2018 Davide Bettio <davide@uninstall.it>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0 OR LGPL-2.1-or-later
 */

#include "atomshashtable.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_SIZE 8

struct HNode
{
    struct HNode *next;
    AtomString key;
    unsigned long value;
};

static unsigned long sdbm_hash(const unsigned char *str, int len)
{
    unsigned long hash = 0;
    int c;

    for (int i = 0; i < len; i++) {
        c = *str++;
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

struct AtomsHashTable *atomshashtable_new()
{
    struct AtomsHashTable *htable = malloc(sizeof(struct AtomsHashTable));
    if (IS_NULL_PTR(htable)) {
        return NULL;
    }
    htable->buckets = calloc(DEFAULT_SIZE, sizeof(struct HNode *));
    if (IS_NULL_PTR(htable->buckets)) {
        free(htable);
        return NULL;
    }

    htable->count = 0;
    htable->capacity = DEFAULT_SIZE;

    return htable;
}

int atomshashtable_insert(struct AtomsHashTable *hash_table, AtomString string, unsigned long value)
{
    int alen = atom_string_len(string);

    unsigned long hash = sdbm_hash(string, alen);
    long index = hash % hash_table->capacity;

    struct HNode *node = hash_table->buckets[index];
    if (node) {
        while (1) {
            if (atom_are_equals(string, node->key)) {
                node->value = value;
                return 1;
            }

            if (node->next) {
                node = node->next;
            } else {
                break;
            }
        }
    }

    struct HNode *new_node = malloc(sizeof(struct HNode));
    if (IS_NULL_PTR(new_node)) {
        return 0;
    }
    new_node->next = NULL;
    new_node->key = string;
    new_node->value = value;

    if (node) {
        node->next = new_node;
    } else {
        hash_table->buckets[index] = new_node;
    }

    hash_table->count++;
    return 1;
}

unsigned long atomshashtable_get_value(const struct AtomsHashTable *hash_table, const AtomString string, unsigned long default_value)
{
    unsigned long hash = sdbm_hash(string, atom_string_len(string));
    long index = hash % hash_table->capacity;

    const struct HNode *node = hash_table->buckets[index];
    while (node) {
        if (atom_are_equals(string, node->key)) {
            return node->value;
        }

        node = node->next;
    }

    return default_value;
}

int atomshashtable_has_key(const struct AtomsHashTable *hash_table, const AtomString string)
{
    unsigned long hash = sdbm_hash(string, atom_string_len(string));
    long index = hash % hash_table->capacity;

    const struct HNode *node = hash_table->buckets[index];
    while (node) {
        if (atom_are_equals(string, node->key)) {
            return 1;
        }

        node = node->next;
    }

    return 0;
}
