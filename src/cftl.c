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

#include "cftl.h"
#include <stddef.h>
#include <errno.h>

/*
 * FIXME milestone 2 and 3 (if using C)
 * Use this function to initialize your FTL.
 * The device_path is something similar to /dev/nvme0n1.
 */
OpenChannelDevice *open_ocssd(const char *device_path) {
    return NULL;
}

int64_t read_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer) {
    return -ENOSYS;
}

int64_t write_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer) {
    return -ENOSYS;
}

/*
 * Return device properties such as device size and minimum write size.
 * They will be used by the key value store and the tests.
 */
int properties_ocssd(OpenChannelDevice *dev, OpenChannelDeviceProperties *properties) {
    return -ENOSYS;
}

/*
 * Use this to cleanup any used resources.
 * Close device handle, join threads, deallocate memory.
 */
int close_ocssd(OpenChannelDevice *dev) {
    return -ENOSYS;
}
