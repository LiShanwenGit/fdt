#include "fdt.h"
#include <stdio.h>

extern const void *fdt_dts_blob;
extern const unsigned long long fdt_dts_size;

static int case_count = 1;


void ut_case(int result, const char *msg)
{
    if(result) {
        printf("%2d. %-35s: \033[1;32mOK\033[0m\n", case_count, msg);
    } else {
        printf("%2d. %-35s: \033[1;31mFAIL\033[0m\n", case_count, msg);
    }

    case_count ++;
}


int main(void)
{
    int ret = -1;
    //调用fdt_load接口，读取设备树二进制文件
    ret = fdt_load(fdt_dts_blob, fdt_dts_size);
    if(ret) {
        FDT_LOG_ERROR("fdt load failed\n");
        return -1;
    }

    //打印设备树的版本信息
    printf("version: %"PRIx64"\n", fdt_get_version());

    printf("================== UNIT TEST BEGIN ================\n");

    fdt_node_t *node_root = fdt_get_root_node();

    fdt_node_t *node1 = fdt_find_node_by_name(node_root, "node1");
    ut_case(node1 && strcmp(node1->name, "node1") == 0, "fdt_find_node_by_name");

    fdt_node_t *subnode1 = fdt_find_node_by_path("/node1/subnode1");
    ut_case(subnode1 && strcmp(subnode1->name, "subnode1") == 0, "fdt_find_node_by_path");

    fdt_prop_t *string = fdt_find_prop_by_name(node1, "string");
    ut_case(string && strcmp(string->name, "string") == 0, "fdt_find_prop_by_name");

    string = fdt_find_prop_by_path("/node1/subnode1/string");
    ut_case(string && strcmp(string->name, "string") == 0, "fdt_find_prop_by_path");


    /* read property */
    const char *string_val = fdt_read_prop_string(node1, "string");
    ut_case(string_val && strcmp(string_val, "test_string") == 0, "fdt_read_prop_string");

    size_t int_val = 0;
    ret = fdt_read_prop_int(node1, "int", &int_val);
    ut_case(ret == 0 && int_val == 95, "fdt_read_prop_int");

    size_t int_val_index = 0;
    ret = fdt_read_prop_int_index(node1, "array", 1, &int_val_index);
    ut_case(ret == 0 && int_val_index == 0x787de, "fdt_read_prop_int_index");


    /* read property by path */
    const char *string_val_path = fdt_read_prop_string_by_path("/node1", "string");
    ut_case(string_val_path && strcmp(string_val_path, "test_string") == 0, "fdt_read_prop_string_by_path");

    size_t int_val_path = 0;
    ret = fdt_read_prop_int_by_path("/node1", "int", &int_val_path);
    ut_case(ret == 0 && int_val_path == 95, "fdt_read_prop_int_by_path");

    size_t int_val_index_path = 0;
    ret = fdt_read_prop_int_index_by_path("/node1", "array", 1, &int_val_index_path);
    ut_case(ret == 0 && int_val_index_path == 0x787de, "fdt_read_prop_int_index_by_path");


    /* get property int size */
    int int_size = 0;
    int_size = fdt_get_prop_int_size(node1, "array16");
    ut_case(ret == 0 && int_size == 4, "fdt_get_prop_int_size");

    int int_size_path = 0;
    int_size_path = fdt_get_prop_int_size_by_path("/node1", "array16");
    ut_case(ret == 0 && int_size_path == 4, "fdt_get_prop_int_size_by_path");


    /* get property type */
    fdt_prop_type_t type = fdt_get_prop_type(node1, "string");
    ut_case(type == FDT_PROP_STRING, "fdt_get_prop_type string");

    fdt_prop_type_t type_by_path = fdt_get_prop_type_by_path("/node1", "string");
    ut_case(type_by_path == FDT_PROP_STRING, "fdt_get_prop_type_by_path string");

    type = fdt_get_prop_type(node1, "int");
    ut_case(type == FDT_PROP_INT, "fdt_get_prop_type int");

    type_by_path = fdt_get_prop_type_by_path("/node1", "int");
    ut_case(type_by_path == FDT_PROP_INT, "fdt_get_prop_type_by_path int");

    type = fdt_get_prop_type(node1, "array");
    ut_case(type == FDT_PROP_ARRAY, "fdt_get_prop_type array");

    type_by_path = fdt_get_prop_type_by_path("/node1", "array");
    ut_case(type_by_path == FDT_PROP_ARRAY, "fdt_get_prop_type_by_path array");

    printf("================== UNIT TEST END ================\n");
    return 0;
}
