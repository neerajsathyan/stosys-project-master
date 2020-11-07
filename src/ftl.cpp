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

/* FIXME milestone 2 and 3 (if using C++)
 * Use this function to initialize your FTL.
 * The device_path is something similar to /dev/nvme0n1.
 */
OpenChannelDevice::OpenChannelDevice(const std::string &device_path) {
    dev = nvm_dev_open(device_path.c_str());
    if (!dev) {
        perror("nvm_dev_open");
        exit(0);
    }
    nvm_dev_pr(dev);

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
}

/*
 * Return device properties such as device size and minimum write size.
 * They will be used by the key value store and the tests.
 */
int OpenChannelDevice::get_device_properties(OpenChannelDeviceProperties *properties) {
    if(dev) {
        struct nvm_ret *ret;
        const struct nvm_geo *geo = nvm_dev_get_geo(dev);
        properties->min_write_size = nvm_dev_get_ws_min(dev) * geo->l.nbytes;
        properties->device_size = geo->tbytes;
        properties->min_read_size = geo->l.nbytes;
        properties->alignment = geo->l.nbytes;
        
        return 1;
    }
    return -ENOSYS;
}

int64_t OpenChannelDevice::read(size_t address, size_t num_bytes, void *buffer) {
    return -ENOSYS;
}

int64_t OpenChannelDevice::write(size_t address, size_t num_bytes, void *buffer) {
    return -ENOSYS;
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
