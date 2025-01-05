/*
 * File Name: fdt.h
 *
 * Copyright 2024-, lishanwen (1477153217@qq.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FDT_H__
#define __FDT_H__


#ifdef x86_64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdbool.h>
#include <stdint.h>


/**
 * @prev: previous node of the list.
 * @next: next node of the list.
 */
typedef struct fdt_list_node {
	struct fdt_list_node *prev;
	struct fdt_list_node *next;

}fdt_list_node_t;


/**
 * @brief Property type.
 * @FDT_PROP_STRING : string type.
 * @FDT_PROP_INT    : integer type.
 * @FDT_PROP_ARRAY  : array type.
 */
typedef enum {
    FDT_PROP_STRING = 0, // string offset
    FDT_PROP_INT = 1,    // int offset,
    FDT_PROP_ARRAY = 32, // array offset  
    FDT_PROP_INVALID = 256

}fdt_prop_type_t;


/** 
 * @brief Property node.
 * @node: next node.
 * @name: property name.
 * @offset: offset of property value.
 */
typedef struct fdt_prop {
    fdt_list_node_t node;
    const char* name;
    const void* offset;

}fdt_prop_t;


/**
 * @brief fdt node struct.
 * @parent: parent node.
 * @entry: next node.
 * @child: child of node
 * @name: node name.
 * @prop: property list head of node.
 */
typedef struct fdt_node {
    struct fdt_node *parent;
    fdt_list_node_t entry;
    fdt_list_node_t child;
    const char *name;
    fdt_list_node_t prop;

}fdt_node_t;


/**
 * @brief Get the offset of internal members of the structure
 * 
 * @struct_t: structure typedef 
 * @member: member in structure
 * 
 * @return offset of member in the structure
*/
#define  fdt_offsetof(struct_t, member)      \
               ((size_t)&((struct_t*)0)->member)


/**
 * @brief Get the address of the structure instance.
 *
 * @ptr: address of the structure member.
 * @type: type of the structure.
 * @member: member name of the ptr in structure.
 *   
 * @return pointer to address of structure 
 */
#define fdt_container_of(ptr, type, member)      ({ \
               (type *)((char *)ptr - fdt_offsetof(type, member)); })



#define FDT_LIST_HEAD(name) \
	fdt_list_node_t name = {.prev = &(name), .next = &(name)}


/**
 * @brief foreach the list.
 *
 * @pos: the &struct list_head to use as a loop cursor.
 * @list_head: the head for your list.
 * 
 * @return none
 */
#define fdt_list_for_each(pos, list_head) \
	for (pos = (list_head)->next; pos != (list_head); pos = pos->next)


/**
 * @brief Return the next entry of specific node.
 *
 * @entry: specific entry.
 *
 * @return entry_type: next entry of specific entry.
 */
#define fdt_list_next_entry(entry, entry_type, list_node_member) \
	fdt_container_of(entry->list_node_member.next, entry_type, list_node_member)


/**
 * @brief Return the previous entry of specific node.
 *
 * @entry: specific entry.
 *
 * @return entry_type: previous entry of specific entry.
 */
#define fdt_list_prev_entry(entry, entry_type, list_node_member) \
	fdt_container_of(entry->list_node_member.prev, entry_type, list_node_member)


/**
 * @brief foreach the list inserted in a structure.
 *
 * @pos: the &struct list_head to use as a loop cursor.
 * @list_head: the head for your list.
 * @entry_type: type of the struct.
 * @list_node_member: member name of the list_node in structure.
 *
 * @return none
 */
#define fdt_list_for_each_entry(pos, list_head, entry_type, list_node_member) \
	for (pos = fdt_container_of((list_head)->next, entry_type, list_node_member); \
	     &pos->list_node_member != (list_head); \
	     pos = fdt_container_of(pos->list_node_member.next, entry_type, list_node_member))


/**
 * @brief FDT magic number. meaning "fdt"
 */
#define FDT_MAGIC                   0x746466


#ifdef x86_64
#define FDT_LOG(...)                printf(__VA_ARGS__)
#define FDT_LOG_ERROR(...)          printf("[ERROR]"__VA_ARGS__)
#define FDT_LOG_INFO(...)           printf("[INFO]"__VA_ARGS__)
#else
#define FDT_LOG(...)
#define FDT_LOG_ERROR(...)
#define FDT_LOG_INFO(...)
#endif


#define  fdt_malloc(size)           malloc(size)
#define  fdt_free(ptr)              free(ptr)

#define  fdt_strcmp(a, b)           strcmp(a, b)
#define  fdt_strlen(s)              strlen(s)

#define  fdt_memset(buf, val, len)  memset(buf, val, len)
#define  fdt_memcpy(dst, src, len)  memcpy(dst, src, len)



#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief load fdt blob.
 * @param dtb: fdt blob.
 * @param dtb_size: fdt blob size.
 * @return 0 if success, or -1.
 * @note: fdt_load() must be called before any other functions.
 */
int fdt_load(const void *dtb, const uint64_t dtb_size);


/**
 * @brief Get root node of fdt.
 * @param none
 * @return root node.
 */
fdt_node_t* fdt_get_root_node(void);


/**
 * @brief Get version of fdt.
 * @param none
 * @return version.
 * @node version: year-month-day.
 */
uint64_t fdt_get_version(void);


/**
 * @brief for each child of node.
 * @param parent_node: parent node.
 * @param child_node: child node.
 * @note it is a for each loop.
 */
#define fdt_for_each_node_child(parent_node, child_node)   fdt_list_for_each_entry(child_node, &parent_node->child, fdt_node_t, entry)


/**
 * @brief for each property of node.
 * @param parent_node: parent node.
 * @param node_prop: property of node.
 * @note it is a for each loop.
 */
#define fdt_for_each_node_prop(parent_node, node_prop)     fdt_list_for_each_entry(node_prop, &parent_node->prop, fdt_prop_t, node)


/**
 * @brief Find node by name.
 * @param node: node.
 * @param name: node name.
 * @return node of found node, or NULL.
 */
fdt_node_t* fdt_find_node_by_name(fdt_node_t *node, const char *name);


/**
 * @brief Find node by path.
 * @param path: node path.
 * @return node of found node, or NULL.
 */
fdt_node_t* fdt_find_node_by_path(const char *path);


/**
 * @brief Find property by name.
 * @param node: node.
 * @param name: property name.
 * @return property of found property, or NULL.
 */
fdt_prop_t* fdt_find_prop_by_name(fdt_node_t *node, const char *name);


/**
 * @brief Find property by path.
 * @param path: property path.
 * @return property of found property, or NULL.
 */
fdt_prop_t* fdt_find_prop_by_path(const char *path);


/**
 * @brief Read property value for string type.
 * @param node: node.
 * @param name: property name.
 * @return property string value.
 */
const char* fdt_read_prop_string(fdt_node_t *node, const char *name);


/**
 * @brief Read property value for integer type.
 * @param node: node.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_int(fdt_node_t *node, const char *name, size_t *value);


/**
 * @brief Read property value for u8 type.
 * @param node: node.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u8(fdt_node_t *node, const char *name, uint8_t *value);


/**
 * @brief Read property value for u16 type.
 * @param node: node.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u16(fdt_node_t *node, const char *name, uint16_t *value);


/**
 * @brief Read property value for u32 type.
 * @param node: node.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u32(fdt_node_t *node, const char *name, uint32_t *value);


/**
 * @brief Read property value for u64 type.
 * @param node: node.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u64(fdt_node_t *node, const char *name, uint64_t *value);


/**
 * @brief Read property value for integer array type.
 * @param node: node.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array(fdt_node_t *node, const char *name, uint8_t index, size_t *value);


/**
 * @brief Read property value for u8 array type.
 * @param node: node.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u8(fdt_node_t *node, const char *name, uint8_t index, uint8_t *value);


/**
 * @brief Read property value for u16 array type.
 * @param node: node.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u16(fdt_node_t *node, const char *name, uint8_t index, uint16_t *value);


/**
 * @brief Read property value for u32 array type.
 * @param node: node.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u32(fdt_node_t *node, const char *name, uint8_t index, uint32_t *value);


/**
 * @brief Read property value for u64 array type.
 * @param node: node.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u64(fdt_node_t *node, const char *name, uint8_t index, uint64_t *value);


/**
 * @brief Read property string value by path.
 * @param node_path: node path.
 * @param name: property name.
 * @return property string value, or NULL.
 */
const char* fdt_read_prop_string_by_path(const char *node_path, const char *name);


/**
 * @brief Read property value for integer type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_int_by_path(const char *node_path, const char *name, size_t *value);


/**
 * @brief Read property value for u8 type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u8_by_path(const char *node_path, const char *name, uint8_t *value);


/**
 * @brief Read property value for u16 type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u16_by_path(const char *node_path, const char *name, uint16_t *value);


/**
 * @brief Read property value for u32 type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u32_by_path(const char *node_path, const char *name, uint32_t *value);


/**
 * @brief Read property value for u64 type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_u64_by_path(const char *node_path, const char *name, uint64_t *value);


/**
 * @brief Read property value for integer array type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_by_path(const char *node_path, const char *name, uint8_t index, size_t *value);


/**
 * @brief Read property value for u8 array type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u8_by_path(const char *node_path, const char *name, uint8_t index, uint8_t *value);


/**
 * @brief Read property value for u16 array type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u16_by_path(const char *node_path, const char *name, uint8_t index, uint16_t *value);


/**
 * @brief Read property value for u32 array type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u32_by_path(const char *node_path, const char *name, uint8_t index, uint32_t *value);


/**
 * @brief Read property value for u64 array type by path.
 * @param node_path: node path.
 * @param name: property name.
 * @param index: array index.
 * @param value: property value.
 * @return 0 if success, or -1.
 */
int fdt_read_prop_array_u64_by_path(const char *node_path, const char *name, uint8_t index, uint64_t *value);


/**
 * @brief Debug print node info.
 * @param node: node.
 * @return void.
 * @note you should call it in debug mode.
 */
void fdt_debug_put_node_info(fdt_node_t *node);


/**
 * @brief Debug to get fdt number of bytes consumed
 * @param none
 * @return uint64_t number of bytes consumed
 */
uint64_t fdt_debug_get_consume_bytes(void);


#ifdef __cplusplus
}
#endif


#endif // !__FDT_H__
