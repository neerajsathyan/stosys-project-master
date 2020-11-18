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
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <cmath>


pthread_mutex_t  lock ;


#define INVALID_SECTOR_THRESHOLD 10;


void* OpenChannelDevice::startGC(void *arg) {
	struct nvm_ret ret_struct;
	struct nvm_addr address;
    while(true){
        std::cout<<"Created GC thread";
        usleep(5000);

		pthread_mutex_lock(&lock);
		// size_t address = 0;
			//See if the corresponding lpa in pagemap is set to write (valid read state).. check if junk can also be thrown if fresh read without write..
		int count = 0;
		PageMapProp  first = lp2ppMap.front();
		size_t curr_chunk = first.ppa.l.chunk;
		bool flag = false;
		for (auto iter = lp2ppMap.begin(); iter != lp2ppMap.end(); ++iter) {
			size_t chunk = iter->ppa.l.chunk;
			if(curr_chunk != chunk ){
				curr_chunk = chunk;
				count = 0;
			}
			if (iter->flag == 'I') {
					// flag = true;
					// addrs[i] = iter->ppa;
					// break;
				count++;
				//find if chunks of same 
				// iter->lpa% 4096 + 
			}
			if(count > 10) {
				//Write to a free chunk from the free chunk list
				//Update the FTL entries
				flag = true;
				address = iter->ppa;
				break;
				
			}
			
		}
		if(flag) {
			//Write to a free chunk all valid values in this chunk 

			//Update the corresponding FTL entries

			//Erase the current chunk
			nvm_cmd_erase(dev, &address, 1, NULL, 0, &ret_struct);
		}
		pthread_mutex_unlock(&lock ); 	

	}



    //decide when to call GC

    //read the table, and select block to evict
	

    //erase the selected block

}

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
    //this->device_size = 1 * this->geo->l.nsectr * this->geo->l.nbytes;
	//10 Chunks Overprovisioned..
    this->device_size = this->geo->tbytes - (10 * this->geo->l.nsectr * this->geo->l.nbytes);
    //this->device_size = 4 * this->geo->l.nbytes;
    this->current_size_nbytes = 0;

	//Initialise freeChunkList with info abt all the chunks in the device..
	populateFreeChunkList();

    // pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*startGC) (void*), void *arg);
    // void *ret;

  

  pthread_mutex_init(&lock, NULL);
  //maybe pass ftlmap as argument because this is not working directly
  if (pthread_create(&this->thid, NULL, startGC,(void *) "thread 1") != 0) {


}

void OpenChannelDevice::populateFreeChunkList() {\
	//TODO: remove the chunk from free list when writing..
	//Inside each PU Grp:
	for (int i=0; i<geo->l.npugrp; ++i) {
		//Inside each PU Unit..
		for (int j=0; j<geo->l.npunit; ++j) {
			//Inside each chunk.. 
			for (int k=0; k<geo->l.nchunk; ++k) {
				nvm_addr addr;
				addr.l.pugrp = i;
				addr.l.punit = j;
				addr.l.chunk = k;
				addr.l.sectr = 0;
				freeChunkList.push_back(addr);
			}
		}
	}
}

/*
 * Use this to cleanup any used resources.
 * Close device handle, join threads, deallocate memory.
 */
OpenChannelDevice::~OpenChannelDevice() {
	void *exit_status;
	pthread_join( thid ,  &exit_status); 
	nvm_dev_close(dev);
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
	struct nvm_ret ret_struct;
	struct nvm_addr* addrs;
	OpenChannelDeviceProperties properties;
    int read_block_size = 64;
	if (lp2ppMap.empty()) {
		return -2;
	}

	int status = get_device_properties(&properties);
	
	if (num_bytes % properties.alignment == 0 && num_bytes >= properties.min_read_size) {
		int sectors_required = num_bytes/geo->l.nbytes;
		addrs = (nvm_addr* ) calloc(read_block_size, sizeof(*addrs));

    
        int read_units = sectors_required/max_read_sectors;
        if(sectors_required%max_read_sectors!=0) {
            read_units += 1;
        }

		//Merge read units buffers together..

		//TODO: Parallelise this..
		for(auto i=0; i<sectors_required; ++i) {
			bool flag = false;
			for (auto iter = lp2ppMap.begin(); iter != lp2ppMap.end(); ++iter) {
				if (iter->lpa == address + (i*geo->l.nbytes)) {
					addrs[i] = iter->ppa;
					if (iter -> flag == 'W') {
						break;
					}
				}
				int curr_cunk = iter->ppa.l.chunk;
			}
			/*if (!flag) {
				return -3;
			}
			*/
		}

		nvm_addr_prn(addrs, sectors_required, dev);

        //DO Read in multiples of 64 (max_read_sectors)...
		int ret;
		int sector_iter = sectors_required;
		for (auto j=0; j<read_units; ++j) {
			if (sector_iter < max_read_sectors) {
				ret = nvm_cmd_read(dev, addrs + j*sector_iter, sector_iter, buffer+(j*sector_iter*geo->l.nbytes), NULL, 0, &ret_struct);
				sector_iter -= sector_iter;
				if (ret != 0) {
					nvm_ret_pr(&ret_struct);
					return -420;
				}
			} else {
				ret = nvm_cmd_read(dev, addrs + j*max_read_sectors, max_read_sectors, buffer+(j*max_read_sectors*geo->l.nbytes), NULL, 0, &ret_struct);
				sector_iter -= max_read_sectors;
				if (ret != 0) {
					nvm_ret_pr(&ret_struct);
					return -421;
				}
			}
		}
        /*char *read_buffer = calloc(sectors_required*geo->l.nbytes,1);
        for (auto i=0; i<read_units, sectors_required > 0; ++i) {
            int ret;
            if(sectors_required < max_read_sectors) {
                ret = nvm_cmd_read(dev, addrs + addrs_iter, sectors_required, tempBuffer, NULL, 0, &ret_struct);
                sectors_required = 0;
            } else {
                ret = nvm_cmd_read(dev, addrs + addrs_iter, max_read_sectors, tempBuffer, NULL, 0, &ret_struct);
                sectors_required = sectors_required - max_read_sectors;
            } 
                    
		    nvm_ret_pr(&ret_struct);
        }*/

        //int ret = nvm_cmd_read(dev, addrs, 64, buffer, NULL, 0, &ret_struct);
        //nvm_ret_pr(&ret_struct);

	
		if(ret == 0) {
			//Successful read..
			return num_bytes;
		} else {
			return -567; 
		}
	}

	return -ENOSYS;
}

int64_t OpenChannelDevice::write(size_t address, size_t num_bytes, void *buffer) {
	if(current_size_nbytes >= device_size) {
		//Disk Full..
		return -1;
	}

	struct nvm_ret ret_struct;
	struct nvm_addr *addrs;
	int ret;
	//Check if min_write_size is satisfied..
	OpenChannelDeviceProperties local_properties;

	int status = get_device_properties(&local_properties);

	int min_write_size = nvm_dev_get_ws_min(dev); //Min no of sectors to be written..

	if ((num_bytes%local_properties.alignment == 0) && (num_bytes >= local_properties.min_write_size)) {
		int sectors_required = num_bytes/geo->l.nbytes;
		addrs = (nvm_addr* ) calloc(sectors_required, sizeof(*addrs));
		for (auto i=0; i<sectors_required; ++i) {
			if (!lp2ppMap.empty()) {
				for (auto iter = lp2ppMap.begin(); iter != lp2ppMap.end(); ++iter) {
					if((iter->lpa) == address+(i*geo->l.nbytes) && (iter->flag == 'W')) {
						//Invalidate those mappings.. Overwrite happening in this sector..
						iter->flag = 'I';
						current_size_nbytes -= sector_size;
						//Add to invalidated Chunk List if its already not present..
						addInvalidatedChunkList(iter->ppa);
						break;
					}
				}
			}

			//Check if sequential writes filled up the device_size addresses..
			//Then Invoke GC and store it in free chunk from the free chunk list..
			if (filled_sequential_write()) {
				//Get a free chunk.. or if working with the same chunk, use that..
				//Get from fchunk iter which is updated by GC upon invocation..
				//if(fchunk_iter.l)

			}
			//Sequential write in the beginning..
			addrs[i].l.pugrp = curr_physical_group;
			addrs[i].l.punit = curr_physical_pu;
			addrs[i].l.chunk = curr_physical_chunk;
			addrs[i].l.sectr = curr_physical_sector;
			//Updating Page Map..
			PageMapProp *pagemap = new PageMapProp();
			pagemap->flag = 'W';
			pagemap->lpa = address+(i*geo->l.nbytes);
			pagemap->ppa = addrs[i];
			pagemap->num_bytes = num_bytes;
			pagemap->start_address = address;
			//Adding the page map to the table..
			lp2ppMap.push_back(*pagemap);
			update_genericaddress();

			//Remove from free chunk..
			removeFromFreeChunkList(addrs, sectors_required);		

		}
	assert(current_size_nbytes+num_bytes <= device_size);
	nvm_addr_prn(addrs, sectors_required, dev);

	ret = nvm_cmd_write(dev, addrs, sectors_required, buffer, NULL, 0, &ret_struct);

	if(ret == 0) {
		//Successful write..
        printf("Successful Write!!\n");
		current_size_nbytes += num_bytes;
		return num_bytes;
	}
	return ret;

	}


	//0x2
	return -ENOSYS;
}


nvm_addr* OpenChannelDevice::getMinWriteSizeFreeSectors() {
	
	nvm_addr *addrs = (nvm_addr* ) calloc(nvm_dev_get_ws_min(dev) * this->sector_size, sizeof(*addrs));
	//TODO:
	// if (found in current chunk) {
	// 	//Check in the current chunk..
	// }
	// //Check for min write size sectors in invalidation list
	// else if(found in invalidation list) {

	// } else {
	// 	//Get it from a free chunk list and update freeChunkList..
	// }

	return addrs;
}

bool OpenChannelDevice::filled_sequential_write() {
	if ((curr_physical_group+1)*(curr_physical_pu+1)*(curr_physical_chunk+1)*(curr_physical_sector+1)*geo->l.nbytes > device_size) {
		return true;
	}
	return false;
}

void OpenChannelDevice::addInvalidatedChunkList(nvm_addr addr) {
	//Check if the respective chunk is already invalidated..
	//If yes then update the free sector availability, else add chunk to the list..
	InvalidatedChunkProp *inv = new InvalidatedChunkProp();
	nvm_addr ichunk_addr = addr;
	ichunk_addr.l.sectr = 0;
	inv->ppa = ichunk_addr;
	if (invalidatedChunkList.empty()) {
		inv->live_data_sectors = geo->l.nsectr - 1;
		invalidatedChunkList.push_back(*inv);
	} else {
		bool equality_flag = false;
		for (auto i = invalidatedChunkList.begin(); i != invalidatedChunkList.end(); i++) {
			if (check_nvm_addr_equality(ichunk_addr, i->ppa)) {
				// Chunk already has invalidated sectors..
				i->live_data_sectors -= 1;
				equality_flag = true;
				break;
			}
		}
		if (!equality_flag) {
			//Chunk was not present.. add to the list..
			inv->live_data_sectors = geo->l.nsectr - 1;
			invalidatedChunkList.push_back(*inv);
		}
	}
}

void OpenChannelDevice::removeFromFreeChunkList(nvm_addr *addr, int addr_size) {
	for (int i=0; i<addr_size; ++i) {
		if (addr[i].l.sectr == 0) {
			for (auto j=freeChunkList.begin(); j!=freeChunkList.end(); ++j) {
				if(check_nvm_addr_equality(addr[i], *j)) {
					freeChunkList.erase(j);
					j--;
				}
			}
		}
	}
}

bool OpenChannelDevice::check_nvm_addr_equality(nvm_addr &addr1, nvm_addr &addr2) {
	if((addr1.l.chunk == addr2.l.chunk)&&(addr1.l.pugrp == addr2.l.pugrp)
	&&(addr1.l.punit == addr2.l.punit) && (addr1.l.sectr == addr2.l.sectr))
		return true;
	return false;
}

void OpenChannelDevice::update_genericaddress() {
	curr_physical_sector = (curr_physical_sector + 1)%sectors_per_chunk;
	if (curr_physical_sector == 0)
		curr_physical_chunk = (curr_physical_chunk + 1)%geo->l.nchunk;
	if (curr_physical_chunk == 0 && curr_physical_sector == 0)
		curr_physical_pu = (curr_physical_pu + 1)%geo->l.npunit;
	if (curr_physical_pu == 0 && curr_physical_chunk == 0 && curr_physical_sector == 0)
		curr_physical_group = (curr_physical_group + 1)%geo->l.npugrp;
	assert(curr_physical_sector <= sectors_per_chunk);
	assert(curr_physical_chunk <= geo->l.nchunk);
	assert(curr_physical_pu <= geo->l.npunit);
	assert(curr_physical_group <= geo->l.npugrp);
	//TODO: implement size over logic..
}

std::vector <PageMapProp> OpenChannelDevice::getMap() {
	return lp2ppMap;
}

void OpenChannelDevice::setMap(std::vector <PageMapProp> mapper) {
	lp2ppMap = mapper;
}

int64_t OpenChannelDevice::eraseAll() {
	//Erase entire disk..
	for (int i = 0; i < geo->l.npugrp; ++i) {
		for (int j = 0; j < geo->l.npunit; ++j) {
			for (int k = 0;k < geo->l.nchunk; ++k) {
				struct nvm_ret ret;
				struct nvm_addr addrs;
				addrs.l.punit = i;
				addrs.l.pugrp = j;
				addrs.l.chunk = k;
				int status = nvm_cmd_erase(dev, &addrs, 1, NULL, 0, &ret);
				if (status == -1) {
					printf("Erase Error status:");
					printf(ret.status + "");
				}
			}
		}
	}
	printf("Entire disk erase!");
	return 0;
}

void OpenChannelDevice::gc_func() {
	printf("Invoked GC!");

}



int main(int argc, char **argv) {
	OpenChannelDevice *device = new OpenChannelDevice("/dev/nvme0n1");
	OpenChannelDeviceProperties properties;
	device->get_device_properties(&properties);
	auto disk_size = properties.device_size;
	auto min_write_size = properties.min_write_size;
	auto num_waves = disk_size / min_write_size;
	// std::vector<uint8_t> write_vec(min_write_size);
	// std::vector<uint8_t> all_data_written(0);
	// all_data_written.reserve(disk_size);

	// for (auto i=0; i<num_waves; i++) {
	// 	std::generate(write_vec.begin(), write_vec.end(), std::rand);
	// 	all_data_written.insert(all_data_written.begin() + (i * min_write_size), write_vec.begin(), write_vec.end());
	// 	std::cout<<"\nLogical Address: "<<(i*min_write_size)<<"\n";
	// 	auto bytes_written = device->write(i * min_write_size, min_write_size, reinterpret_cast<void *>(write_vec.data()));
	// 	if (bytes_written < 0) {
	// 		std::cout<<"Error in writing!\n";
	// 		break;
	// 	}
	// }

	// std::cout<<"\nREADING: \n";
	// std::vector<uint8_t> read_vec(disk_size);
	// auto bytes_read = device->read(0, disk_size, reinterpret_cast<void *>(read_vec.data()));
	// std::cout<<"Read Status: "<<bytes_read<<"\n";

	// std::cout<<"Checking READ and WRITE Buffers\n";
	// if(read_vec == all_data_written) {
	// 	std::cout<<"Equal!\n";
	// } else {
	// 	std::cout<<"Unequal\n";
	// }
	
	device->eraseAll();
	device->~OpenChannelDevice();
	return 0;
}

