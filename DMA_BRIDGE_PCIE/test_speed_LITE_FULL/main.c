#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>

#define MAP_SIZE 0x200000UL /*AXI LITE SIZE "Mb"*/
#define BRAM0_SIZE 0x100000 /*1Mb*/
#define BRAM1_SIZE 0x2000 /*8Kb*/

#define GPIO_OFFSET 0x102000
#define BRAM0_OFFSET_LITE 0x0000
#define BRAM1_OFFSET_LITE 0x100000

#define BRAM0_OFFSET_FULL 0xC0000000
#define BRAM1_OFFSET_FULL 0xC2000000

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

void write_axi_full(int fd, off_t base_addr, const void *data, size_t nbyte) {
    if (pwrite(fd, data, nbyte, base_addr) == -1) {
        perror("pwrite");
        exit(EXIT_FAILURE);
    }
}

void read_axi_full(int fd, off_t base_addr, void *buffer, size_t nbyte) {
    if (pread(fd, buffer, nbyte, base_addr) == -1) {
        perror("pread");
        exit(EXIT_FAILURE);
    }
}

void test_axi_lite(void *map_base, uint32_t *write_vector, uint32_t *read_vector, 
                   uint32_t bram0_elemenst, uint32_t bram1_elemenst){
    
    uint32_t writeval = 0x1;
    uint32_t read_result, error, i;

    clock_t start_time, end_time;
    double elapsed_time;
    double gbps;
    printf("AXI LITE\n");
/*
    wtite_axi_lite_32b(map_base, GPIO_OFFSET, &writeval, 1);

    read_axi_lite_32b(map_base, GPIO_OFFSET, &read_result, 1);
*/
    start_time = clock();
    wtite_axi_lite_32b(map_base, BRAM0_OFFSET_LITE, write_vector, bram0_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM0_SIZE) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_axi_lite_32b(map_base, BRAM0_OFFSET_LITE, read_vector, bram0_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM0_SIZE) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    error = 0;
    for (i = 0; i < bram0_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

    start_time = clock();
    wtite_axi_lite_32b(map_base, BRAM1_OFFSET_LITE, write_vector, bram1_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM1_SIZE) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_axi_lite_32b(map_base, BRAM1_OFFSET_LITE, read_vector, bram1_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM1_SIZE) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    error = 0;
    for (i = 0; i < bram1_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    } 
}

void test_axi_full(int fd_h2c, int fd_c2h,  uint32_t *write_vector, uint32_t *read_vector,
                                       uint32_t bram0_elemenst, uint32_t bram1_elemenst){

    uint32_t error, i;
    clock_t start_time, end_time;
    double elapsed_time;
    double gbps;
    printf("AXI FULL\n");

    start_time = clock();
    write_axi_full(fd_h2c, BRAM0_OFFSET_FULL, write_vector, bram0_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM0_SIZE) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_axi_full(fd_c2h, BRAM0_OFFSET_FULL, read_vector, bram0_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM0_SIZE) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    error = 0;
    for (i = 0; i < bram0_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

    start_time = clock();
    write_axi_full(fd_h2c, BRAM1_OFFSET_FULL, write_vector, bram1_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM1_SIZE) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_axi_full(fd_c2h, BRAM1_OFFSET_FULL, read_vector, bram1_elemenst);
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (BRAM1_SIZE) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    error = 0;
    for (i = 0; i < bram1_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    } 
}

void test_write_lite_read_full(void *map_base, int fd_h2c, int fd_c2h,  uint32_t *write_vector, uint32_t *read_vector,
                                       uint32_t bram0_elemenst, uint32_t bram1_elemenst){

    uint32_t error, i;

    wtite_axi_lite_32b(map_base, BRAM0_OFFSET_LITE, write_vector, bram0_elemenst);

    read_axi_full(fd_c2h, BRAM0_OFFSET_FULL, read_vector, bram0_elemenst);
    error = 0;
    for (i = 0; i < bram0_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

    wtite_axi_lite_32b(map_base, BRAM1_OFFSET_LITE, write_vector, bram1_elemenst);

    read_axi_full(fd_c2h, BRAM1_OFFSET_FULL, read_vector, bram1_elemenst);
    error = 0;
    for (i = 0; i < bram1_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

}

void test_write_full_read_lite(void *map_base, int fd_h2c, int fd_c2h,  uint32_t *write_vector, uint32_t *read_vector,
                                       uint32_t bram0_elemenst, uint32_t bram1_elemenst){

    uint32_t error, i;

    write_axi_full(fd_h2c, BRAM0_OFFSET_FULL, write_vector, bram0_elemenst);

    read_axi_lite_32b(map_base, BRAM0_OFFSET_LITE, read_vector, bram0_elemenst);
    error = 0;
    for (i = 0; i < bram0_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

    write_axi_full(fd_h2c, BRAM1_OFFSET_FULL, write_vector, bram1_elemenst);

    read_axi_lite_32b(map_base, BRAM1_OFFSET_LITE, read_vector, bram1_elemenst);
    error = 0;
    for (i = 0; i < bram1_elemenst; i++) {
        if (read_vector[i] != i){
            printf("Error in data TX or RX %i\n", i); 
            printf("%i != %i\n", read_vector[i], i); 
        }
    }

}

int main() {


    int fd, i;
    void *map_base;
    uint32_t bram0_elemenst = BRAM0_SIZE / sizeof(uint32_t);
    uint32_t bram1_elemenst = BRAM1_SIZE / sizeof(uint32_t);

    uint32_t *write_vector = (uint32_t *)malloc(BRAM0_SIZE);
    uint32_t *read_vector = (uint32_t *)malloc(BRAM0_SIZE);

    char *resource_path = "/sys/bus/pci/devices/0000:03:00.0/resource0";

    int fd_h2c = open("/dev/xdma0_h2c_0", O_RDWR);
    if (fd_h2c == -1) {
        perror("open /dev/xdma0_h2c_0");
        return EXIT_FAILURE;
    }

    int fd_c2h = open("/dev/xdma0_c2h_0", O_RDWR);
    if (fd_c2h == -1) {
        perror("open /dev/xdma0_c2h_0");
        close(fd_h2c);
        return EXIT_FAILURE;
    }

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

    for (i = 0; i < bram0_elemenst; i++) {
        write_vector[i] = i;
    }

    test_axi_lite(map_base, write_vector, read_vector, bram0_elemenst, bram1_elemenst);
    test_axi_full(fd_h2c, fd_c2h, write_vector, read_vector, bram0_elemenst, bram1_elemenst);
    test_write_full_read_lite(map_base, fd_h2c, fd_c2h, write_vector, read_vector, bram0_elemenst, bram1_elemenst);
    test_write_lite_read_full(map_base, fd_h2c, fd_c2h, write_vector, read_vector, bram0_elemenst, bram1_elemenst);
         
    if (munmap(map_base, MAP_SIZE) == -1) {
        perror("Error en munmap");
        close(fd);
        exit(1);
    }

    close(fd);

    return 0;
}