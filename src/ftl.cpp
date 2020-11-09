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

#include "ftl.h"
#include <errno.h>
#include <pthread.h>





/* FIXME milestone 2 and 3 (if using C++)
 * Use this function to initialize your FTL.
 * The device_path is something similar to /dev/nvme0n1.
 */
OpenChannelDevice::OpenChannelDevice(const std::string &device_path) {
    this->dev = nvm_dev_open(device_path.c_str());
    if (!this->dev) {
        perror("nvm_dev_open");
        exit(-1);
    }
    nvm_dev_pr(this->dev);

    //Gets geometric values and stored to variables..
    this->geo = nvm_dev_get_geo(this->dev);
    this->sectors_per_chunk = this->geo->l.nsectr;
    this->num_chunks = this->geo->tbytes / (this->geo->l.nsectr * this->geo->l.nbytes);
    this->sector_size = this->geo->l.nbytes;
    this->chunk_size = this->geo->l.nsectr * this->geo->l.nbytes;
    this->device_size = this->geo->tbytes;
    this->current_size_nbytes = 0;

    //TODO: Logic to initialise FTL..
    
}

/*
 * Use this to cleanup any used resources.
 * Close device handle, join threads, deallocate memory.
 */
OpenChannelDevice::~OpenChannelDevice() {
    nvm_dev_close(dev);
    

    //TODO: join threads..
    //TODO: deallocate memory..
    free(&this->geo);
    free(&this->sectors_per_chunk);
    free(&this->num_chunks);
    free(&this->sector_size);
    free(&this->chunk_size);
    free(&this->device_size);
    
}

/*
 * Return device properties such as device size and minimum write size.
 * They will be used by the key value store and the tests.
 */
int OpenChannelDevice::get_device_properties(OpenChannelDeviceProperties *properties) {
    if(this->dev) {
        properties->min_write_size = nvm_dev_get_ws_min(dev) * this->sector_size;
        properties->device_size = this->device_size;
        properties->min_read_size = this->sector_size;
        properties->alignment = this->sector_size;
        return 1;
    }
    return -ENOSYS;
}

int64_t OpenChannelDevice::read(size_t address, size_t num_bytes, void *buffer) {
    return -ENOSYS;
}

int64_t OpenChannelDevice::write(size_t address, size_t num_bytes, void *buffer) {

    if(current_size_nbytes >= device_size) {
        //Disk Full...
        return -1;
    }
    struct nvm_ret ret_struct;
    struct nvm_addr *addrs;
    int ret;
    //Check if min_write_size is satisfied..
    OpenChannelDeviceProperties *local_properties;
    int status = get_device_properties(local_properties);
    int min_write_size = nvm_dev_get_ws_min(dev); //Min no of sector to be written..
    if ((num_bytes%local_properties->alignment == 0) && (num_bytes >= local_properties->min_write_size)) {
        //check if the logical address is not present or if the map is empty..
        int sectors_required = num_bytes/geo->l.nbytes;
        if(check_lp_or_emptymap(lp2ppMap, address)) {
            // Do write and mapping..
            ssize_t err = 0;
            // for (auto i=0; i< num_bytes/local_properties->min_write_size; ++i) {
            //     //For each 4 sector writes of the buffer..
            //     //allocate the array
            //     addrs = calloc()
            // }
            addrs = (nvm_addr* ) calloc(num_bytes, sizeof(*addrs));
            //Update Group, PU, chunk, sector based on current values..
            //TODO: for now assert is given in update_genericaddress if device size is over.. implement
            //a return error code here later..
            //How much sectors required for this file..
            
            //sector_required should be aligned.. 
            for(auto i=0; i<sectors_required; ++i) {
                addrs[i].l.pugrp = curr_physical_group;
                addrs[i].l.punit = curr_physical_pu;
                addrs[i].l.chunk = curr_physical_chunk;
                addrs[i].l.sectr = curr_physical_sector;
                //Updating page map..
                PageMapProp *pagemap = new PageMapProp();
                pagemap->flag = 'W';
                pagemap->lpa = address+(i * geo->l.nbytes);
                pagemap->ppa = addrs[i];
                pagemap->num_bytes = num_bytes;
                pagemap->start_address = address;
                //Adding the page map to the table..
                //lp2ppMap.push_back(*new PageMapProp('W',(address+(i * geo->l.nbytes)),addrs[i]));
                lp2ppMap.push_back(*pagemap);
                //Update generic address..
                update_genericaddress();
            }

        } else {
            
            //LPA-PPA mapping was foud, hence invalidate the current LPA table.. Required for GC..
            int sector_invalidation_counter = 0;
            for (auto iter = lp2ppMap.begin(); iter != lp2ppMap.end(), iter->flag == 'W'; ++iter) {
                // 0, num_bytes-8KB.. 0, 4..
                //addr, addr+4086, addr+(2*4086), addr+(n*4086)  -->  (n-1)*4086 = num_bytes .. addr+num_bytes
                if (iter->lpa < iter->start_address+iter->num_bytes) {
                    iter->flag = 'I';
                    sector_invalidation_counter++;
                }
            }

            assert(sector_invalidation_counter == sectors_required);

            //Now write the data with updated ppa to the lpa..
            for(auto i=0; i<sectors_required; ++i) {
                addrs[i].l.pugrp = curr_physical_group;
                addrs[i].l.punit = curr_physical_pu;
                addrs[i].l.chunk = curr_physical_chunk;
                addrs[i].l.sectr = curr_physical_sector;
                //Updating page map..
                PageMapProp *pagemap = new PageMapProp();
                pagemap->flag = 'W';
                pagemap->lpa = address+(i * geo->l.nbytes);
                pagemap->ppa = addrs[i];
                pagemap->num_bytes = num_bytes;
                pagemap->start_address = address;
                //Adding the page map to the table..
                //lp2ppMap.push_back(*new PageMapProp('W',(address+(i * geo->l.nbytes)),addrs[i]));
                lp2ppMap.push_back(*pagemap);
                //Update generic address..
                update_genericaddress();
            }

            
        }

        ret = nvm_cmd_write(dev, addrs, sectors_required, buffer, NULL, 0, &ret_struct);
        //TODO: Find a way to return status error codes.. rather than -1 and 0;
        //&ret_struct.status;
        if(ret == 0) {
            //Successful write..
            current_size_nbytes += num_bytes;
            return num_bytes;  
        }
        return ret;

    } else {
        //0x2
        return -1;
    }

}

void OpenChannelDevice::update_genericaddress() {
    //Do updates to this like if 
    curr_physical_sector = (curr_physical_sector + 1)%sectors_per_chunk;
    if (curr_physical_sector == 0)
        curr_physical_chunk = (curr_physical_chunk + 1)%geo->l.nchunk;
    if (curr_physical_chunk == 0)
        curr_physical_pu = (curr_physical_pu + 1)%geo->l.npunit;
    //TODO: Implement R/W Parallelism here later..
    if (curr_physical_pu == 0)
        curr_physical_group = (curr_physical_group + 1)%geo->l.npugrp;
    assert(curr_physical_sector <= sectors_per_chunk);
    assert(curr_physical_chunk <= geo->l.nchunk);
    assert(curr_physical_pu <= geo->l.npunit);
    assert(curr_physical_group <= geo->l.npugrp);
}



bool OpenChannelDevice::check_lp_or_emptymap(std::vector <PageMapProp> lp2ppMap, size_t address) {
    if (lp2ppMap.empty()) {
        return true;
    } else {
        //Iterate to see if lp already exists..
        for (auto iter = lp2ppMap.begin(); iter != lp2ppMap.end(); ++iter) {
            if (*(&iter->lpa) == address) {
                return false;
            }
        }
        return true;
    }
}

/*
 * Below are wrapper functions if to use your C++ functions from C, if required.
 * For example, if you write your FTL in C++ and your key value store in C.
 */
extern "C" OpenChannelDevice *open_ocssd(const char *device_path) {
    auto ocd = new OpenChannelDevice(device_path);

    return ocd;
}

extern "C" size_t read_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer) {
    return dev->read(address, size, buffer);
}

extern "C" size_t write_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer) {
    return dev->write(address, size, buffer);
}

extern "C" int properties_ocssd(OpenChannelDevice *dev, OpenChannelDeviceProperties *properties) {
    return dev->get_device_properties(properties);
}

extern "C" int close_ocssd(OpenChannelDevice *dev) {
    dev->~OpenChannelDevice();
    return 0;
}
