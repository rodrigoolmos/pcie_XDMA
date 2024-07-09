#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BRAM0_BASE_ADDR 0xC0000000
#define BRAM1_BASE_ADDR 0xC2000000
#define BRAM2_BASE_ADDR 0xC4000000

#define BRAM0_SIZE 0x2000  /* 8K */
#define BRAM1_SIZE 0x1000  /* 4K */
#define BRAM2_SIZE 0x4000  /* 16K */

void write_to_bram(int fd, off_t base_addr, const void *data, size_t size) {
    if (pwrite(fd, data, size, base_addr) == -1) {
        perror("pwrite");
        exit(EXIT_FAILURE);
    }
}

void read_from_bram(int fd, off_t base_addr, void *buffer, size_t size) {
    if (pread(fd, buffer, size, base_addr) == -1) {
        perror("pread");
        exit(EXIT_FAILURE);
    }
}

int main() {
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

    /* Datos para escribir en las BRAM */
    unsigned long int write_data_bram0[BRAM0_SIZE / sizeof(unsigned long int)];
    unsigned long int write_data_bram1[BRAM1_SIZE / sizeof(unsigned long int)];
    unsigned long int write_data_bram2[BRAM2_SIZE / sizeof(unsigned long int)];

        /* Buffers para lectura */
    unsigned long int read_data_bram0[BRAM0_SIZE / sizeof(unsigned long int)];
    unsigned long int read_data_bram1[BRAM1_SIZE / sizeof(unsigned long int)];
    unsigned long int read_data_bram2[BRAM2_SIZE / sizeof(unsigned long int)];

    int i;

    /* Llenar las BRAM con valores consecutivos en hexadecimal */
    for (i = 0; i < BRAM0_SIZE / sizeof(unsigned long int); ++i) {
        write_data_bram0[i] = i;
    }
    for (i = 0; i < BRAM1_SIZE / sizeof(unsigned long int); ++i) {
        write_data_bram1[i] = i + 10;
    }
    for (i = 0; i < BRAM2_SIZE / sizeof(unsigned long int); ++i) {
        write_data_bram2[i] = i + 20;
    }

    /* Variables para medir el tiempo */
    clock_t start_time, end_time;
    double elapsed_time;
    double gbps;

    /* Medir el tiempo de escritura en BRAM0 */
    start_time = clock();
    write_to_bram(fd_h2c, BRAM0_BASE_ADDR, write_data_bram0, sizeof(write_data_bram0));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram0) * 8) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    /* Medir el tiempo de escritura en BRAM1 */
    start_time = clock();
    write_to_bram(fd_h2c, BRAM1_BASE_ADDR, write_data_bram1, sizeof(write_data_bram1));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram1) * 8) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    /* Medir el tiempo de escritura en BRAM2 */
    start_time = clock();
    write_to_bram(fd_h2c, BRAM2_BASE_ADDR, write_data_bram2, sizeof(write_data_bram2));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram2) * 8) / (elapsed_time * 1e9);
    printf("BRAM2 - Elapsed time for writing: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    /* Lectura de las BRAM */
    start_time = clock();
    read_from_bram(fd_c2h, BRAM0_BASE_ADDR, read_data_bram0, sizeof(read_data_bram0));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram0) * 8) / (elapsed_time * 1e9);
    printf("BRAM0 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_from_bram(fd_c2h, BRAM1_BASE_ADDR, read_data_bram1, sizeof(read_data_bram1));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram1) * 8) / (elapsed_time * 1e9);
    printf("BRAM1 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    start_time = clock();
    read_from_bram(fd_c2h, BRAM2_BASE_ADDR, read_data_bram2, sizeof(read_data_bram2));
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    gbps = (sizeof(write_data_bram2) * 8) / (elapsed_time * 1e9);
    printf("BRAM2 - Elapsed time for reading: %f seconds, Write speed: %f Gb/s\n", elapsed_time, gbps);

    /* Mostrar los datos leídos */
    /*printf("Data read from BRAM0: \n");
    for (i = 0; i < BRAM0_SIZE / sizeof(unsigned long int); ++i) {
        printf("%lX ", read_data_bram0[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");

    printf("Data read from BRAM1: \n");
    for (i = 0; i < BRAM1_SIZE / sizeof(unsigned long int); ++i) {
        printf("%lX ", read_data_bram1[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");

    printf("Data read from BRAM2: \n");
    for (i = 0; i < BRAM2_SIZE / sizeof(unsigned long int); ++i) {
        printf("%lX ", read_data_bram2[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");*/

    close(fd_h2c);
    close(fd_c2h);

    return 0;
}