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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <numeric>
#include "doctest.h"
#ifdef cppftl
#include "ftl.h"
#else
#include "cppwrapper.h"
#endif

class FtlTestsFixture {
protected:
#ifdef cppftl
    OpenChannelDevice ocd;
#else
    OpenChannelDeviceWrapper ocd;
#endif
public:
    FtlTestsFixture() : ocd("/dev/nvme0n1") {}
};

TEST_CASE_FIXTURE(FtlTestsFixture, "Hammer that sector!") {
    OpenChannelDeviceProperties properties;
    ocd.get_device_properties(&properties);
    auto disk_size = properties.device_size;
    auto min_write_size = properties.min_write_size;
    auto min_read_size = properties.min_read_size;
    auto num_waves = disk_size / min_write_size;
    auto times_to_fill = 2;
    std::vector<uint8_t> write_vec(min_write_size);

    auto last_sector_id = disk_size / min_read_size;
    auto sector_to_hammer = std::rand() % last_sector_id;

    for (auto attempt=0; attempt<times_to_fill; attempt++) {
        for (auto i = 0; i < num_waves; i++) {
            std::generate(write_vec.begin(), write_vec.end(), std::rand);

            auto bytes_written = ocd.write(sector_to_hammer * min_read_size, min_write_size,
                                           reinterpret_cast<void *>(write_vec.data()));
            REQUIRE(bytes_written == min_write_size);
        }
    }

    std::vector<uint8_t> read_vec(min_write_size);
    auto bytes_read = ocd.read(sector_to_hammer * min_read_size, min_write_size, reinterpret_cast<void *>(read_vec.data()));
    CHECK(bytes_read == min_write_size);

    CHECK(read_vec == write_vec);
}

TEST_CASE_FIXTURE(FtlTestsFixture, "Double the data with GC!") {
    OpenChannelDeviceProperties properties;
    ocd.get_device_properties(&properties);
    auto disk_size = properties.device_size;
    auto min_write_size = properties.min_write_size;
    auto num_waves = disk_size / min_write_size;
    auto times_to_fill = 2;
    std::vector<uint8_t> write_vec(min_write_size);
    std::vector<uint8_t> all_data_written(0);
    all_data_written.reserve(disk_size);

    for (auto attempt=0; attempt<times_to_fill; attempt++) {
        all_data_written.clear();
        for (auto i = 0; i < num_waves; i++) {
            std::generate(write_vec.begin(), write_vec.end(), std::rand);

            all_data_written.insert(all_data_written.begin() + (i * min_write_size), write_vec.begin(),
                                    write_vec.end());

            auto bytes_written = ocd.write(i * min_write_size, min_write_size,
                                           reinterpret_cast<void *>(write_vec.data()));

            REQUIRE(bytes_written == min_write_size);
        }
    }

    std::vector<uint8_t> read_vec(disk_size);
    auto bytes_read = ocd.read(0, disk_size, reinterpret_cast<void *>(read_vec.data()));
    CHECK(bytes_read == disk_size);

    CHECK(read_vec == all_data_written);
}
