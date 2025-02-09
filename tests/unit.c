#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../include/bufrw.h"

void test_bfwrite_bfread() {
    FILE *file = fopen("test.bin", "wb+");
    assert(file);
    
    char write_data[] = "Hello, Buffered I/O!";
    size_t data_size = strlen(write_data) + 1;
    
    size_t written = bfwrite(write_data, 1, data_size, 16, file);
    assert(written == data_size);
    
    bfflush(file);
    fseek(file, 0, SEEK_SET);
    
    char read_data[32] = {0};
    size_t read = bfread(read_data, 1, data_size, 16, file);
    assert(read == data_size);
    assert(strcmp(write_data, read_data) == 0);
    
    fclose(file);
    remove("test.bin");
    printf("test_bfwrite_bfread passed.\n");
}

void test_bfseek_bftell() {
    FILE *file = fopen("test.bin", "wb+");
    assert(file);
    
    char data[] = "0123456789";
    bfwrite(data, 1, 10, 16, file);
    bfflush(file);
    
    bfseek(file, 5, SEEK_SET);
    assert(bftell(file) == 5);
    
    bfseek(file, -2, SEEK_CUR);
    assert(bftell(file) == 3);
    
    bfseek(file, 0, SEEK_END);
    assert(bftell(file) == 10);
    
    fclose(file);
    remove("test.bin");
    printf("test_bfseek_bftell passed.\n");
}

#if 0
void bfbestbufsz_print(size_t sz) {
    printf("bestbufsz of %zu: %zu\n", sz, bfbestbufsz(sz));
}
#endif

void test_bfbestbufsz() {
    #if 0
    bfbestbufsz_print(100);
    bfbestbufsz_print(500);
    bfbestbufsz_print(1000);
    bfbestbufsz_print(70000);
    bfbestbufsz_print(0);
    #endif
    assert(bfbestbufsz(100) == 100);
    assert(bfbestbufsz(500) == 500);
    assert(bfbestbufsz(1000) == 512);
    assert(bfbestbufsz(70000) == 65536);
    assert(bfbestbufsz(0) == 512);
    printf("test_bfbestbufsz passed.\n");
}

void test_bfcleanup() {
    bfcleanup();
    printf("test_bfcleanup passed.\n");
}

void rununit(void) {
    test_bfwrite_bfread();
    test_bfseek_bftell();
    test_bfbestbufsz();
    test_bfcleanup();
}