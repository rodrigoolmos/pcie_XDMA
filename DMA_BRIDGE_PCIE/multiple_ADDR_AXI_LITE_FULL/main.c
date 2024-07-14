#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define MAP_SIZE 0x4000UL
#define BRAM0_SIZE 0x2000 

#define GPIO_OFFSET 0x0000
#define BRAM_OFFSET 0x2000

int main() {
    uint32_t writeval = 0x5;

    int fd, i;
    void *map_base, *virt_addr;
    uint32_t read_result;

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

    /* Escribir en GPIOS */
    virt_addr = map_base + GPIO_OFFSET;
    *((uint32_t *) virt_addr) = writeval;
    read_result = *((uint32_t *) virt_addr);
    printf("Valor leído de %s: 0x%X\n", resource_path, read_result);

    /* Escribir BRAM completa AXI LITE*/
    for (i = 0; i < BRAM0_SIZE / sizeof(uint32_t); i++){
        virt_addr = map_base + BRAM_OFFSET + sizeof(uint32_t)*i;
        *((uint32_t *) virt_addr) = i;
    }

    /* Leer BRAM completa AXI LITE*/
    for (i = 0; i < BRAM0_SIZE / sizeof(uint32_t); i++){
        virt_addr = map_base + BRAM_OFFSET + sizeof(uint32_t)*i;
        read_result = *((uint32_t *) virt_addr);
        printf("Valor leído de %s: addres 0x%p data %u\n", resource_path, virt_addr, read_result);
    }

    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Error en munmap");
        close(fd);
        exit(1);
    }

    close(fd);


    return 0;
}