/*
 * MIT License

Copyright (c) 2020-2021

Authors: Sacheendra Talluri, Giulia Frascaria, and, Animesh Trivedi
This code is part of the Storage System Course at VU Amsterdam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <liblightnvm.h>

// how to compile this code
// gcc ocssd_main.c -llightnvm -laio -fopenmp

int main(int argc, char **argv) {
    struct nvm_ret ret_struct;
    struct nvm_addr *addrs;
    const struct nvm_geo *dev_geo;
    char *write_buffer, *read_buffer;
    size_t i, write_size_in_bytes;
    int min_write_size, ret;

    struct nvm_dev *dev = nvm_dev_open("/dev/nvme0n1");
    if (!dev) {
        perror("nvm_dev_open");
        return 1;
    }
    nvm_dev_pr(dev);
    printf("\n\n\n -- ** -- \n");
    // we have the device geometry
    dev_geo = nvm_dev_get_geo(dev);
    // we have other custom features
    min_write_size = nvm_dev_get_ws_min(dev);
    printf("The size of the sector is %lu bytes \n", dev_geo->sector_nbytes);
    printf("The minimum write sector count is %d \n", min_write_size);

    write_size_in_bytes = min_write_size * dev_geo->sector_nbytes;
    printf("Test write size is %lu \n", write_size_in_bytes);
    write_buffer = calloc(1, write_size_in_bytes);
    // write a pattern
    for(size_t i = 0; i < write_size_in_bytes; i++){
        write_buffer[i] = rand() & 0xFF;
    }
    //allocate the array
    addrs = calloc(min_write_size, sizeof(*addrs));
    // we will pick up first write_size_in_bytes addresses
    // TODO: notice we are not checking if the maximum number of blocks in a chunk is reached or not, you should
    for(i=0; i < min_write_size; i++){
        // rest is already initialized to zero, we just need to increase the sector number
        // we are using the generic addressing mode
        addrs[i].l.sectr = i;
    }

    ret = nvm_cmd_write(dev, addrs, min_write_size, write_buffer, NULL, 0, &ret_struct);
    printf("The return code for the write operation is %d \n", ret);
    nvm_ret_pr(&ret_struct);

    read_buffer = calloc(1, write_size_in_bytes);
    ret = nvm_cmd_read(dev, addrs, min_write_size, read_buffer, NULL, 0, &ret_struct);
    printf("The return code for the read operation is %d \n", ret);
    nvm_ret_pr(&ret_struct);
    printf("Matching the pattern ...\n");
    for(size_t i = 0; i < write_size_in_bytes; i++){
        assert(read_buffer[i] == write_buffer[i]);
    }
    printf("Pattern matches OK");
    // then erase, only passing the first will do, there is already the min unit of erase set to the chunk size
    ret = nvm_cmd_erase(dev, addrs, 1, NULL, 0, &ret_struct);
    printf("The return code for the erase operation is %d \n", ret);
    nvm_ret_pr(&ret_struct);

    nvm_dev_close(dev);
    printf("All done, good bye cruel world!\n");
    return 0;
}