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
#include <string.h>
#include <errno.h>
#include <sys/types.h>

// how to compile this code
// gcc ocssd_main.c -llightnvm -laio -fopenmp

struct PageMapProp {
    size_t lpa;
    struct nvm_addr ppa;
    char flag;
    size_t num_bytes;
    size_t start_address;
};

void update_genericaddress(int group, int pu, size_t chunk, size_t sector);

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

    int curr_physical_group = 0;
    int curr_physical_pu = 0;
    size_t curr_physical_sector = 0;
    size_t curr_physical_chunk = 0;
    
    /*New code*/
    size_t sectors_per_chunk = dev_geo->l.nsectr;
    printf("Sectors per chunk is %lu sectors \n",sectors_per_chunk);
    size_t num_chunks = dev_geo->tbytes / (dev_geo->l.nsectr * dev_geo->l.nbytes);
    printf("Number of chunks %lu \n", num_chunks);
    size_t sector_size = dev_geo->l.nbytes;
    printf("Sector size from l -> %lu \n",sector_size);
    size_t chunk_size = dev_geo->l.nsectr * dev_geo->l.nbytes;
    printf("Chunk Size is %lu bytes \n",chunk_size);
    size_t device_size = dev_geo->tbytes;
    printf("Device Size %lu bytes\n",device_size);
    size_t current_size_nbytes = 0;
    
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
    
    size_t alignment = dev_geo->l.nbytes;

    if ((write_size_in_bytes%alignment == 0) && (write_size_in_bytes >= min_write_size*sector_size)) {
        size_t sectors_required = write_size_in_bytes/dev_geo->l.nbytes;
        for(size_t i=0; i < sectors_required; i++){
            // rest is already initialized to zero, we just need to increase the sector number
            // we are using the generic addressing mode
            addrs[i].l.pugrp = curr_physical_group;
            addrs[i].l.punit = curr_physical_pu;
            addrs[i].l.chunk = curr_physical_chunk;
            addrs[i].l.sectr = curr_physical_sector;
            update_genericaddress(&curr_physical_group, &curr_physical_pu, &curr_physical_chunk, &curr_physical_sector);
        }
    } else {

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

void update_genericaddress(int curr_physical_group, int curr_physical_pu , size_t curr_physical_chunk, size_t curr_physical_sector) {
        //Do updates to this like if 
    curr_physical_sector = (curr_physical_sector + 1)%sectors_per_chunk;
    if (curr_physical_sector == 0)
        curr_physical_chunk = (curr_physical_chunk + 1)%geo->l.nchunk;
    if (curr_physical_chunk == 0)
        curr_physical_pu = (curr_physical_pu + 1)%geo->l.npunit;
    //TODO: Implement R/W Parallelism here later..
    if (curr_physical_pu == 0)
        curr_physical_group = (curr_physical_group + 1)%geo->l.npugrp;
}