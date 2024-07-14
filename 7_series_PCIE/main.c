#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>

#define MAP_SIZE 1024 /* 1 KB*/

#define BASE_ADDR 0x00000

void wtite_axi_lite_32b(void *addr, uint32_t offset, uint32_t *value, uint32_t vector_elemnts){

    int i;
    void *virt_addr;

    for (i = 0; i < vector_elemnts; i++){
        virt_addr = addr + offset + sizeof(uint32_t)*i;
        *((uint32_t *) virt_addr) = value[i];
    }
}

void read_axi_lite_32b(void *addr, uint32_t offset, uint32_t *value, uint32_t vector_elemnts){

    int i;
    void *virt_addr;

    for (i = 0; i < vector_elemnts; i++){
        virt_addr = addr + offset + sizeof(uint32_t)*i;
        value[i] = *((uint32_t *) virt_addr);
    }

}

int main() {

    int fd;
    uint32_t read_data, error, i, j;
    
    void *map_base[4];
    char *resource_paths[] = {
        "/sys/bus/pci/devices/0000:03:00.0/resource0",
        "/sys/bus/pci/devices/0000:03:00.0/resource1",
        "/sys/bus/pci/devices/0000:03:00.0/resource2",
        "/sys/bus/pci/devices/0000:03:00.0/resource3"
    };

    for (i = 0; i < 4; i++) {
        if ((fd = open(resource_paths[i], O_RDWR | O_SYNC)) == -1) {
            perror("Error abriendo el dispositivo PCIe");
            exit(1);
        }
        map_base[i] = mmap(0, MAP_SIZE * (1 + i), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map_base[i] == (void *) -1) {
            perror("Error en mmap");
            close(fd);
            exit(1);
        }

        close(fd); 
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < (1 + i)*MAP_SIZE / sizeof(uint32_t); j++)
            wtite_axi_lite_32b(map_base[i], BASE_ADDR + sizeof(uint32_t) * j, &j, 1);
    }

    error = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < (1 + i)*MAP_SIZE / sizeof(uint32_t); j++){
            read_axi_lite_32b(map_base[i], BASE_ADDR + sizeof(uint32_t) * j, &read_data, 1);
            if(read_data != j){
                error++;
                printf("Bram error %i expected data %i, data %i\n", i, j, read_data);               
            }
        }
    }

    printf("Num errors %u\n", error);


    for (i = 0; i < 4; i++) {

        if (munmap(map_base[i], MAP_SIZE) == -1) {
            perror("Error en munmap");
        }
    }

    return 0;
}