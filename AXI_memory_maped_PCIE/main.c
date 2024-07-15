#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>

#define MAP_SIZE 0x4000UL

#define GPIO_OFFSET 0

void wtite_axi_lite_32b(void *addr, uint32_t offset, uint32_t *value, uint32_t data_elemnts){

    int i;
    void *virt_addr;

    for (i = 0; i < data_elemnts; i++){
        virt_addr = addr + offset + sizeof(uint32_t)*i;
        *((uint32_t *) virt_addr) = value[i];
    }
}

void read_axi_lite_32b(void *addr, uint32_t offset, uint32_t *value, uint32_t data_elemnts){

    int i;
    void *virt_addr;

    for (i = 0; i < data_elemnts; i++){
        virt_addr = addr + offset + sizeof(uint32_t)*i;
        value[i] = *((uint32_t *) virt_addr);
    }

}

int main() {

    int fd, i;
    void *map_base;

    uint32_t read_data;
    char *resource_path = "/sys/bus/pci/devices/0000:03:00.0/resource0";

    if ((fd = open(resource_path, O_RDWR | O_SYNC)) == -1) {
        perror("Error abriendo el dispositivo PCIe");
        exit(1);
    }

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_base == (void *) -1) {
        perror("Error en mmap");
        close(fd);
        exit(1);
    }

    for (i = 0; i < 16; i++) {
        wtite_axi_lite_32b(map_base, GPIO_OFFSET, &i, 1);
        sleep(1);
    }

         
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Error en munmap");
        close(fd);
        exit(1);
    }

    close(fd);

    return 0;
}