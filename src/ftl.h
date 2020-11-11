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

#pragma once

#include <string>
#include <liblightnvm.h>
#include <vector>
#include <unordered_map>

struct PageMapProp {
    size_t lpa;
    struct nvm_addr ppa;
    char flag;
    size_t num_bytes;
    size_t start_address;
    //explicit PageMapProp(char flag, size_t lpa, nvm_addr ppa);
};

// group (2) -> PU (4) -> Chunk (6) -> block/sector (256)  ... 4096 bytes sector   50331648 total bytes
typedef struct {
    size_t device_size;
    size_t min_write_size;
    size_t min_read_size;
    size_t alignment;
} OpenChannelDeviceProperties;

typedef struct {
    // size_t physical_addr;
    nvm_addr logical_addr;
    short flag;  //-1 -> invalid, 0-> erase, 1->valid
} TableField;

class OpenChannelDevice {
    
    struct nvm_dev *dev;
    
    const struct nvm_geo *geo;

    volatile int curr_ptr = 0; //starting address of the device (in la)
    
    volatile int curr_physical_group = 0;
    volatile int curr_physical_pu = 0;
    volatile size_t curr_physical_sector = 0;
    volatile size_t curr_physical_chunk = 0;
    
    size_t sectors_per_chunk;
    size_t num_chunks;
    size_t sector_size;
    size_t chunk_size;
    size_t device_size;

    size_t current_size_nbytes;

    // FTL Map Table (Page Mapped)
    std::vector <PageMapProp> lp2ppMap;
    // std::unordered_map<size_t, PageMapProp> table;

public:
    explicit OpenChannelDevice(const std::string &device_path);
    ~OpenChannelDevice();
    int64_t read(size_t address, size_t num_bytes, void *buffer);
    int64_t write(size_t address, size_t num_bytes, void *buffer);
    int get_device_properties(OpenChannelDeviceProperties *properties);
    void update_genericaddress();
    std::vector <PageMapProp> getMap();
    void setMap(std::vector <PageMapProp> mapper);
    //bool check_lp_or_emptymap(std::unordered_map<size_t, PageMapProp> table, size_t address);
};

extern "C" OpenChannelDevice *open_ocssd(const char *device_path);

extern "C" size_t read_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer);

extern "C" size_t write_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer);

extern "C" int properties_ocssd(OpenChannelDevice *dev, OpenChannelDeviceProperties *properties);

extern "C" int close_ocssd(OpenChannelDevice *dev);
