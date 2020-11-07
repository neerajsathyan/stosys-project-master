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

#include "kv.h"

/* ***********************************************************************************
 *
 * This is a simple implementation of a key-value storage app for flash-based devices
 *
 * The user can read, write and delete key-value pairs
 *
 * user API:
 * put [key] [value]
 * get [key]
 *
 * Internally, the kv app needs to access the device through read_ocssd() and write_ocssd()
 *
 *************************************************************************************/



int main(int argc, char ** argv)
{

	printf("----\nusage:\nput key value\nget key\n----\n");

	int command;
	char line[512];
        struct kv_item item;
        int key, success;
	FILE *input, *output;
       	
        if (argc == 2)
        {
                if(strcmp(argv[1], "-i") == 0)
                {
                        input = stdin;
                        output = stdout;
                        printf("starting interactive mode\n");
                }
        }
        else
        {
                printf("starting file test, reading from workload, writing to results\n");
                input = fopen("workload", "r");
                output = fopen("results", "w+");
                if (input == NULL)
                {
                        printf("workload file not found, use -i to start in interactive mode for manual testing\n");
                        exit(EXIT_FAILURE);
                }
                if (output == NULL)
                {
                        printf("error creating output file\n");
                        exit(EXIT_FAILURE);
                }
        }


        while (fgets(line, sizeof line, input))
        {
                command = parse_command(line);
                if(command == -1)
                {
                        printf("command not recognized\n");
                        exit(EXIT_FAILURE);
                }
                if(command == 0)
                {
                        item = parse_put(line);
                        printf("PUT\n");

			put_val(&item);
                }
                if(command == 1)
                {
                        key = parse_get(line);
                        printf("GET\n");

			struct kv_item * res = get_val(key);

			if(!res)
                                printf("key not found\n");
                        else
                        {
                                char *ptr = strtok(res->val, "\n");
                                fprintf(output, "key: %d, value: %s\n", key, ptr);
                                fflush(output);
                        }
                }
        }
	
        exit(EXIT_SUCCESS);
}


int parse_command(char l[512])
{
        if(strncmp(l, "put", 3) == 0)
                return 0;
        if(strncmp(l, "get", 3) == 0)
                return 1;
        if(strncmp(l, "del", 3) == 0)
                return 2;
        return -1;
}


struct kv_item parse_put(char l[512])
{
        struct kv_item i;
        char *ptr = strtok(l, " ");

        ptr = strtok(NULL, " ");
        i.key = atoi(ptr);
        if(i.key == 0)
        {
                printf("error in conversion\n");
        }
        ptr = strtok(NULL, " ");

        i.size = strlen(ptr);
        strncpy(i.val, ptr, strlen(ptr));

        return i;
}


int parse_get(char l[512])
{
        char *ptr = strtok(l, " ");
        ptr = strtok(NULL, " ");

        int key = atoi(ptr);
        return key;
}


int put_val(struct kv_item *i)
{
        printf("put is not implemented\n");

	/* ********************************************************************
	 * TODO milestone 4
	 *
	 * implement put operation through the read_ocssd() and write_ocssd() API
	 *
	 * ********************************************************************/

        return -1;
}


struct kv_item * get_val(int key)
{
        printf("get is not implemented\n");

        /* ********************************************************************
	 * TODO milestone 4
         *
         * implement get operation through the read_ocssd() and write_ocssd() API
         *
         * ********************************************************************/

	return NULL;
}

