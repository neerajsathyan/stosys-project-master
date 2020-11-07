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

#include <cstdlib>

typedef struct{} OpenChannelDevice;

typedef struct {
    size_t device_size;
    size_t min_write_size;
    size_t min_read_size;
    size_t alignment;
} OpenChannelDeviceProperties;

extern "C" {
    OpenChannelDevice *open_ocssd(const char *device_path);

    int64_t read_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer);

    int64_t write_ocssd(OpenChannelDevice *dev, size_t address, size_t size, void *buffer);

    int properties_ocssd(OpenChannelDevice *dev, OpenChannelDeviceProperties *properties);

    int close_ocssd(OpenChannelDevice *dev);
}

class OpenChannelDeviceWrapper {
public:
    explicit OpenChannelDeviceWrapper(const std::string &device_path) {
        this->device = open_ocssd(device_path.c_str());
    };

    ~OpenChannelDeviceWrapper() {
        close_ocssd(this->device);
    };

    int64_t read(size_t address, size_t num_bytes, void *buffer) {
        return read_ocssd(this->device, address, num_bytes, buffer);
    };

    int64_t write(size_t address, size_t num_bytes, void *buffer) {
        return write_ocssd(this->device, address, num_bytes, buffer);
    };

    int get_device_properties(OpenChannelDeviceProperties *properties) {
        return properties_ocssd(this->device, properties);
    };

private:
    OpenChannelDevice *device;
};
