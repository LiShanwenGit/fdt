/*
 * File Name: fdt.c
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

#include "fdt.h"

/**
 * fdt version, it is format of year-month-day
 */
static uint64_t fdt_version = 0;
static uint64_t fdt_consume = 0;

/**
 * @brief inititialize a list.
 *
 * @node: inserted node.
 *
 * @return none
 */
static inline void fdt_list_init(fdt_list_node_t *node) 
{
	node->next = node;
	node->prev = node;
}


/**
 * @brief Add a node to the list tail.
 *
 * @head: head node of the list.
 * @node: inserted node.
 * 
 * @return none
 */
static inline void fdt_list_add_node_at_tail(fdt_list_node_t *head, fdt_list_node_t *node)
{
	fdt_list_node_t *tail = head->prev;

	node->prev = tail;
	node->next = head;
	tail->next = node;
	head->prev = node;
}


/**
 * @brief Add a node to the list front.
 *
 * @head: head node of the list.
 * @node: inserted node.
 * 
 * @return none
 */
static inline void fdt_list_add_node_at_front(fdt_list_node_t *head, fdt_list_node_t *node)
{
	fdt_list_node_t *front = head->next;

	node->prev = head;
	node->next = front;
	front->prev = node;
	head->next = node;
}


/**
 * @brief Delete a tail node from the list.
 *
 * @head: head node of the list.
 * 
 * @return none
 */
static inline void fdt_list_del_tail_node(fdt_list_node_t *head)
{
	fdt_list_node_t *tail_prev = head->prev->prev;

	tail_prev->next = head;
	head->prev = tail_prev;
}


/**
 * @brief  Delete a front node from the list.
 *
 * @head: head node of the list.
 * 
 * @return none
 */
static inline void fdt_list_del_front_node(fdt_list_node_t *head)
{
	fdt_list_node_t *front_next = head->next->next;

	front_next->prev = head;
	head->next = front_next;
}


/**
 * @brief  Delete a node from the list.
 *
 * @node: the node of the list.
 * 
 * @return none
 */
static inline void fdt_list_del_node(fdt_list_node_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}


/**
 * @brief Determine whether the list is empty.
 *
 * @head: head node of the list.
 * 
 * @return  bool: is empty.
 */
static inline bool fdt_list_is_empty(fdt_list_node_t *head)
{
	return head->next == head;
}


/**
 * @brief Return the next node of specific node.
 *
 * @node: specific node.
 *
 * @return list_node*: next node of specific node.
 */
static inline fdt_list_node_t* fdt_list_next_node(fdt_list_node_t *node)
{
	return node->next;
}


/**
 * @brief Return the previous node of specific node.
 *
 * @node: specific node.
 * 
 * @return list_node*: previous node of specific node.
 */
static inline fdt_list_node_t* fdt_list_prev_node(fdt_list_node_t *node)
{
	return node->prev;
}


/**
 * fdt root node instance
 * name: '/'
 */
struct fdt_node fdt_root = {
    .name = "/",
    .parent = &fdt_root,
};


/**
 * @brief init root node
 * 
 * @param none
 * @return none
 */
static void fdt_root_init(void)
{
    fdt_list_init(&fdt_root.child);
    fdt_list_init(&fdt_root.prop);
}


/**
 * @brief get root node
 * 
 * @param none
 * @return fdt_node_t*: root node
 */
fdt_node_t* fdt_get_root_node(void)
{
    return &fdt_root;
}


/**
 * @brief get version of fdt
 * 
 * @param none
 * @return uint64_t: version
 */
uint64_t fdt_get_version(void)
{
    return fdt_version;
}


/**
 * @brief check whether the node is root node
 * 
 * @param node: node
 * @return bool: true or false, root node return true
 */
static bool fdt_node_is_root(fdt_node_t *node)
{
    return node->parent == &fdt_root;
}


/**
 * @brief check whether the node have child node
 * 
 * @param node: node
 * @return bool: true or false, have child node return true
 */
static bool fdt_node_have_child(fdt_node_t *node)
{
    return !fdt_list_is_empty(&node->child);
}


/**
 * @brief find node by name
 * 
 * @param node: node, if the value is NULL, meaning find node from root node
 * @param name: node name
 * @return fdt_node_t*: node
 */
fdt_node_t* fdt_find_node_by_name(fdt_node_t *node, const char *name)
{
    fdt_node_t *child = NULL;
    if(node == NULL) {
        node = &fdt_root;
    }

    fdt_list_for_each_entry(child, &node->child, fdt_node_t, entry) {
        if(child == NULL) {
            continue;
        }

        if(fdt_strcmp(child->name, name) == 0) {
            return child;
        }
    }

    return NULL;
}


/**
 * @brief find node by path
 * 
 * @param path: node path
 * @return fdt_node_t*: node
 */
fdt_node_t* fdt_find_node_by_path(const char *path)
{
    char node_name[512] = {0};
    int i = 0;
    fdt_node_t* parent = &fdt_root;
    fdt_node_t* node = NULL;

    while(*path) {
        if(*path == ' ') {
            path ++;
            continue;
        }
        else if(*path == '/' && i > 0) {
            node_name[i] = 0; i = 0;

            node = fdt_find_node_by_name(parent, node_name);
            if(node == NULL) {
                return NULL;
            }
            
            parent = node;
        }
        else if(*path != '/'){
            node_name[i++] = *path;
        }

        path ++;
    }

    if(i) {
        node_name[i+1] = 0;
        return fdt_find_node_by_name(parent, node_name);
    }

    return node;
}


/**
 * @brief find property by name
 * 
 * @param node: node
 * @param name: property name
 * @return fdt_prop_t*: property
 */
fdt_prop_t* fdt_find_prop_by_name(fdt_node_t *node, const char *name)
{
    fdt_prop_t *child = NULL;

    fdt_list_for_each_entry(child, &node->prop, fdt_prop_t, node) {
        if(child == NULL) {
            continue;
        }

        if(fdt_strcmp(child->name, name) == 0) {
            return child;
        }
    }

    return NULL;
}


/**
 * @brief find property by path
 *
 * @param path: the path of property
 * @return fdt_prop_t*: property
 */
fdt_prop_t* fdt_find_prop_by_path(const char *path)
{
    fdt_node_t* node = NULL;
    char node_path[512] = {0};
    fdt_memcpy(node_path, path, fdt_strlen(path));

    char *p = (char*)&node_path[fdt_strlen(node_path) - 1];
    char *prop_name = NULL;

    while(*p != '/' && *p) {
        p --;
    }

    *p = 0;
    prop_name = (p + 1);

    node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return NULL;
    }

    return fdt_find_prop_by_name(node, prop_name);
}


/**
 * @brief read string property
 * 
 * @param node: node
 * @param name: property name
 * @return const char*: property value
 */
const char* fdt_read_prop_string(fdt_node_t *node, const char *name)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return NULL;
    }

    char *str = (char*)prop->offset;
    return (str+1);
}


/**
 * @brief read int property
 * 
 * @param node: node
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_int(fdt_node_t *node, const char *name, size_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *len = (uint8_t*)prop->offset;
    size_t  ret = 0;

    for(int i = *len; i > 0; i--) {
        ret = (ret << 8  | (*(len + i)));
    }

    *value = ret;
    return 0;
}


/**
 * @brief read u8 property
 * 
 * @param node: node
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u8(fdt_node_t *node, const char *name, uint8_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *len = (uint8_t*)prop->offset;

    *value = *(len + 1);
    return 0;
}


/**
 * @brief read u16 property
 * 
 * @param node: node
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u16(fdt_node_t *node, const char *name, uint16_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *len = (uint8_t*)prop->offset;

    *value = *(uint16_t*)(len + 1);
    return 0;
}


/**
 * @brief read u32 property
 * 
 * @param node: node
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u32(fdt_node_t *node, const char *name, uint32_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *len = (uint8_t*)prop->offset;

    *value = *(uint32_t*)(len + 1);
    return 0;
}


/**
 * @brief read u64 property
 * 
 * @param node: node
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u64(fdt_node_t *node, const char *name, uint64_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *len = (uint8_t*)prop->offset;

    *value = *(uint64_t*)(len + 1);
    return 0;
}


/**
 * @brief read array property
 * 
 * @param node: node
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array(fdt_node_t *node, const char *name, uint8_t index, size_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *pos = (uint8_t*)prop->offset;
    uint8_t cell_size = *pos - 32;
    pos ++;
    uint8_t len = *pos;

    if(index >= len) {
        return -1;
    }

    uint64_t ret = 0;
    for(int j = cell_size; j > 0; j--) {
        ret = (ret << 8  | (*(pos + j + index * cell_size)));
    }

    *value = ret;
    return 0;
}


/**
 * @brief read array property for u8 type
 * 
 * @param node: node
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u8(fdt_node_t *node, const char *name, uint8_t index, uint8_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *pos = (uint8_t*)prop->offset;
    uint8_t cell_size = *pos - 32;
    pos ++;
    uint8_t len = *pos;

    if(index >= len) {
        return -1;
    }

    *value = * ((uint8_t*)(pos + index * cell_size + 1));
    return 0;
}


/**
 * @brief read array property for u16 type
 * 
 * @param node: node
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u16(fdt_node_t *node, const char *name, uint8_t index, uint16_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *pos = (uint8_t*)prop->offset;
    uint8_t cell_size = *pos - 32;
    pos ++;
    uint8_t len = *pos;

    if(index >= len) {
        return -1;
    }

    *value = * ((uint16_t*)(pos + index * cell_size + 1));
    return 0;
}


/**
 * @brief read array property for u32 type
 * 
 * @param node: node
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u32(fdt_node_t *node, const char *name, uint8_t index, uint32_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *pos = (uint8_t*)prop->offset;
    uint8_t cell_size = *pos - 32;
    pos ++;
    uint8_t len = *pos;

    if(index >= len) {
        return -1;
    }

    *value = * ((uint32_t*)(pos + index * cell_size + 1));
    return 0;
}


/**
 * @brief read array property for u64 type
 * 
 * @param node: node
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u64(fdt_node_t *node, const char *name, uint8_t index, uint64_t *value)
{
    fdt_prop_t *prop = fdt_find_prop_by_name(node, name);
    if(prop == NULL) {
        return -1;
    }

    uint8_t *pos = (uint8_t*)prop->offset;
    uint8_t cell_size = *pos - 32;
    pos ++;
    uint8_t len = *pos;

    if(index >= len) {
        return -1;
    }

    *value = * ((uint64_t*)(pos + index * cell_size + 1));
    return 0;
}


/**
 * @brief read string property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @return const char*: property value
 */
const char* fdt_read_prop_string_by_path(const char *node_path, const char *name)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return NULL;
    }

    return fdt_read_prop_string(node, name);
}


/**
 * @brief read int property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_int_by_path(const char *node_path, const char *name, size_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_int(node, name, value);
}


/**
 * @brief read u8 property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u8_by_path(const char *node_path, const char *name, uint8_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_u8(node, name, value);
}


/**
 * @brief read u16 property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u16_by_path(const char *node_path, const char *name, uint16_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_u16(node, name, value);
}


/**
 * @brief read u32 property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u32_by_path(const char *node_path, const char *name, uint32_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_u32(node, name, value);
}


/**
 * @brief read u64 property by node path
 * 
 * @param node_path: node path
 * @param name: property name
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_u64_by_path(const char *node_path, const char *name, uint64_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_u64(node, name, value);
}


/**
 * @brief read array property for u8 type
 * 
 * @param node_path: node path
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u8_by_path(const char *node_path, const char *name, uint8_t index, uint8_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_array_u8(node, name, index, value);
}


/**
 * @brief read array property for u16 type
 * 
 * @param node_path: node path
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u16_by_path(const char *node_path, const char *name, uint8_t index, uint16_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_array_u16(node, name, index, value);
}


/**
 * @brief read array property for u32 type
 * 
 * @param node_path: node path
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u32_by_path(const char *node_path, const char *name, uint8_t index, uint32_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_array_u32(node, name, index, value);
}


/**
 * @brief read array property for u64 type
 * 
 * @param node_path: node path
 * @param name: property name
 * @param index: index
 * @param value: property value
 * @return int: 0: success, -1: fail
 */
int fdt_read_prop_array_u64_by_path(const char *node_path, const char *name, uint8_t index, uint64_t *value)
{
    fdt_node_t* node = fdt_find_node_by_path(node_path);
    if(node == NULL) {
        return -1;
    }

    return fdt_read_prop_array_u64(node, name, index, value);
}


/**
 * @brief create a property
 * 
 * @param name: property name
 * @param value: data value
 * @return fdt_prop_t*: property
 */
static fdt_prop_t* fdt_prop_create(const char *name, const void *value)
{
    fdt_prop_t *prop = fdt_malloc(sizeof(fdt_prop_t));
    if(prop == NULL) {
        return NULL;
    }

    prop->name = name;
    prop->offset = value;

    fdt_consume += sizeof(fdt_prop_t);

    return prop;
}


/**
 * @brief create a node
 * 
 * @param name: node name
 * @return fdt_node_t*: node
 */
static fdt_node_t* fdt_node_create(const char *name)
{
    fdt_node_t *node = fdt_malloc(sizeof(fdt_node_t));
    if(node == NULL) {
        return NULL;
    }

    node->name = name;
    node->parent = NULL;

    fdt_list_init(&node->prop);
    fdt_list_init(&node->child);

    fdt_consume += sizeof(fdt_node_t);

    return node;
}


/**
 * @brief add property to node
 * 
 * @param node: node
 * @param prop: property
 * @return none
 */
static void fdt_node_append_prop(fdt_node_t *node, fdt_prop_t *prop)
{
    fdt_list_add_node_at_tail(&node->prop, &prop->node);
}


/**
 * @brief add child node to node
 * 
 * @param node: node
 * @param child: child node
 * @return none
 */
static void fdt_node_add_child(fdt_node_t *node, fdt_node_t *child)
{
    child->parent = node;
    fdt_list_add_node_at_tail(&node->child, &child->entry);
}


/**
 * @brief debug print all node and property
 * 
 * @param node: node
 * @level: level of node, for example, the root node is 0, the '/uart' node is 1, etc.
 * @return none
 * @note only used for debug
 */
static void fdt_debug_put_node_prop(fdt_node_t *node, int level)
{
    fdt_prop_t *prop = NULL;
    char _level[1024] = {0};
    fdt_memset(_level, ' ', level *4);
    uint8_t *type = NULL;

    FDT_LOG("%snode = %s\n", _level, node->name);

    fdt_list_for_each_entry(prop, &node->prop, fdt_prop_t, node) {
        if(prop == NULL) {
            continue;
        }

        FDT_LOG("%s    prop: %s = ", _level, prop->name);
        type = (uint8_t*)prop->offset;

        if(*type == FDT_PROP_STRING) {
            FDT_LOG("%s\n", (char*)(type + 1));
        }
        else if(*type > FDT_PROP_STRING && *type < FDT_PROP_ARRAY) {
            uint8_t len = *type;
            uint64_t value = 0;
            for(int i = len; i > 0; i--) {
                value = (value << 8  | (*(type + i)));
            }

            FDT_LOG("0x%lx\n", value);
        }
        else if(*type > FDT_PROP_ARRAY) {
            uint8_t cell_size = *type - 32;
            type ++;
            uint8_t array_len = *((uint8_t*)type);
            
            for(int i = 0; i < array_len; i++) {
                uint64_t value = 0;
                for(int j = cell_size; j > 0; j--) {
                    value = (value << 8  | (*(type + j + i * cell_size)));
                }
                FDT_LOG("0x%lx ", value);
            }
            FDT_LOG("\n");
        }
    }
}


/**
 * @brief debug print all node and property
 * 
 * @param node: node
 * @return none
 * @note only used for debug
 */
void fdt_debug_put_node_info(fdt_node_t *node)
{
    static int level = 0;
    fdt_node_t *child = NULL;

    fdt_debug_put_node_prop(node, level);
    
    if(fdt_node_have_child(node)) {
        level ++;
        fdt_list_for_each_entry(child, &node->child, fdt_node_t, entry) {
            fdt_debug_put_node_info(child);
        }
        level --;
    }
}


/**
 * @brief Debug to get fdt number of bytes consumed
 * @param none
 * @return uint64_t number of bytes consumed
 */
uint64_t fdt_debug_get_consume_bytes(void)
{
    return fdt_consume;
}


/**
 * @brief get magic of dtb file
 * 
 * @param token: input token position of dtb file
 * @return uint64_t: magic
 */
static uint64_t get_magic(uint8_t *token)
{
    uint64_t magic = 0;

    magic = *(token ++);
    magic |= (*(token ++) << 8);
    magic |= (*(token ++) << 16);

    return magic & 0xffffff;
}



/**
 * @brief get version of dtb file
 * 
 * @param token: input token position of dtb file
 * @return uint64_t: version number
 */
static uint64_t get_version(uint8_t *token)
{
    uint64_t version = 0;

    version = *(token ++);
    version |= (*(token ++) << 8);
    version |= (*(token ++) << 16);

    return version & 0xffffff;
}


/**
 * @brief load blob data of dtb file
 * 
 * @param dtb: dtb file
 * @param dtb_size: dtb file size
 * @return int: 0: success, -1: fail
 */
int fdt_load(const void *dtb, const uint64_t dtb_size)
{
    uint64_t pos = 0;
    uint8_t *token = (uint8_t*)dtb;
    uint8_t node_level = 0;
    fdt_node_t *parent_node = &fdt_root;
    fdt_node_t *curr_node = &fdt_root;

    fdt_root_init();

    if(get_magic(token) != FDT_MAGIC) {
        FDT_LOG_ERROR("magic error: invalid dtb file\n");
        return -1;
    }

    fdt_version = get_version(token + 3);
    token += 6;

    if(*token != 0 || *(token + 1) != '/') {
        FDT_LOG_ERROR("invalid dtb file\n");
        return -1;
    }

    token += 3; pos += 9; //skip magic and version and root node name '/'

    while(pos < dtb_size) {
        if(*token == 0xff) {
            // property
            token ++;

            char *prop_name = (char*)token;
            int prop_name_len = fdt_strlen(prop_name) + 1;
            token += prop_name_len;
            pos += prop_name_len + 1;

            void* value = (void*)token;

            fdt_prop_t *prop = fdt_prop_create(prop_name, value);
            if(prop == NULL) {
                FDT_LOG_ERROR("create string prop failed");
                return -1;
            }
            fdt_node_append_prop(curr_node, prop);

            uint8_t prop_type = *token;
            
            if(prop_type == FDT_PROP_STRING) {
                char *prop_str = (char*)(token + 1);
                int prop_str_len = fdt_strlen(prop_str) + 1;
                token ++;
                
                token += prop_str_len;
                pos += (prop_str_len + 1);
            }
            else if(prop_type >= FDT_PROP_INT && prop_type < FDT_PROP_ARRAY){
                uint8_t value_bytes = prop_type;
                token += (value_bytes + 1); pos += (value_bytes + 1);
            }
            else if(prop_type > FDT_PROP_ARRAY) {
                uint8_t cell_bytes = prop_type - 32;
                uint8_t array_len = *(token + 1);

                int array_size = cell_bytes * array_len + 2;

                token += array_size;
                pos += array_size;
            }
        }
        else {
            // node begin
            if(*token > node_level) {
                parent_node = curr_node;
            }
            else if(*token < node_level) {
                int diff = node_level - *token;
                for(int i = 0; i < diff; i ++) {
                    parent_node = parent_node->parent;
                }
            }
            node_level  = *token;
            
            token ++;
            char *node_name = (char*)token;
            int node_name_len = fdt_strlen(node_name) + 1;

            curr_node = fdt_node_create(node_name);
            if(curr_node == NULL) {
                FDT_LOG_ERROR("create node failed\n");
                return -1;
            }

            fdt_node_add_child(parent_node, curr_node);

            token += node_name_len;
            pos += (node_name_len + 1);
        }
    }

    return 0;
}
