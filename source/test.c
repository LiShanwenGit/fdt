#include "fdt.h"
#include <stdio.h>

extern const void *fdt_dts_blob;
extern const unsigned long long fdt_dts_size;


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
    printf("version: %lx\n", fdt_get_version());

    //从根节点查找uart2节点
    fdt_node_t *uart2_node = fdt_find_node_by_name(NULL, "uart2");
    if(uart2_node) {
        printf("uart2_node name = %s\n", uart2_node->name);
    }

    //通过路径查找uart2/st7789v节点
    fdt_node_t *st7789v = fdt_find_node_by_path("/spi1/st7789v/");
    if(st7789v) {
        printf("st7789v node name = %s\n", st7789v->name);
    }

    //通过路径直接获取/spi1/st7789v/compatible属性的字符串
    const char *str = fdt_read_prop_string_by_path("/spi1/st7789v/", "compatible");
    if(str) {
        printf("/spi1/st7789v/compatible = %s\n", str);
    }

    fdt_node_t *child = NULL;
    fdt_node_t *root =  fdt_get_root_node();
    //遍历根节点的所有子节点
    fdt_for_each_node_child(root, child) {
        printf("child node name = %s\n", child->name);
    }

    printf("===%ld==\n", fdt_debug_get_consume_bytes());

    return 0;
}
