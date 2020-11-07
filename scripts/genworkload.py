'''
MIT License

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

'''


import sys
import hashlib
import random
import string
from subprocess import Popen, PIPE
from time import sleep


def gen_value(length):
    letters = string.ascii_lowercase
    result_str = ''.join(random.choice(letters) for i in range(length))
    return result_str

def gen_key(maxkey):
    val = random.randint(0, maxkey)
    return val


if __name__ == "__main__":
    

    if (len(sys.argv) == 2) and (sys.argv[1] == "full"):
        iters = 150000
        maxkey = 100 # a low maxkey value will generate a lot of collisions and consequent updates
                     # without creating too many kv pairs to fit in storage
                     # this workload is in total approximately 70KB for the user
                     # but it generates 70MB of update pressure on the FTL and GC
        valuelen = 10
    elif (len(sys.argv) != 4):
        print("-------------------------------------------------------\n\
                Usage: \n\
                OPTION 1: genworkload.py full\n\
                this generates enough operations to exhaust storage\n\
                \n\
                OPTION 2: genworkload.py [n pairs] [max key] [value len]\n\
                The generated custom workload is saved in the workload file\n\
                -----------------------------------------------------------")
        sys.exit(0)
    else:
        iters = int(sys.argv[1])
        maxkey = int(sys.argv[2])
        valuelen = int(sys.argv[3])
        print("\nparams:\niterating with %d kv pairs\nmax key value %d\nvalue len %d\n" % (iters, maxkey, valuelen))

    hashes = {}

    print("----------------------------------------\n")

    print("|\tgenerating workload with %d kv pairs" % iters)

    with open("workload", "w+") as f:
        keys = []
        for iter in range(0, iters):
            key = gen_key(maxkey)
            value = gen_value(valuelen)

            hashval = hashlib.md5(value.encode())
            #print(hashval.hexdigest())

            if key not in keys:
                keys.append(key)

            hashes[key] = [value, hashval.hexdigest()]
            f.write("put %d %s\n" % (key, value))
            
        for i in range(0, len(keys)):
            f.write("get %d\n" % keys[i])
            
    print("|\tworkload was saved on file")
    print("|\tOK\n")
    sleep(0.5)

    print("----------------------------------------\n")

 
    print("|\tstarting kv app")
    p = Popen(["../bin/./kv"], stdin=PIPE, stdout=PIPE, encoding='utf8')
    print("|\tOK\n")

    print("----------------------------------------\n")


    print("|\twaiting for process to terminate")
    (output, error) = p.communicate()
    p_status = p.wait()
    print("|\tprocess terminated with exit code %d" % p_status)
    print("|\tOK\n")


    print("----------------------------------------\n")

    print("|\tBeginning checksum verification")

    matches = 0
    errors = 0
    mismatch = []

    with open("results", "r") as f:
        while True:
            line = f.readline()

            if not line:
                break
            line = line.replace("\n", "")
            keyval = line.split(", ")
            
            key = keyval[0].replace("key: ", "")
            key = int(key)

            value = keyval[1].replace("value: ", "")
            
            if key in hashes:
                hashval = hashlib.md5(value.encode())

                if hashval.hexdigest() == hashes[key][1]:
                    matches = matches + 1
                else:
                    errors = errors + 1
                    mismatch.append([key, hashes[key][0], value])

    print("|\tOK\n")
    print("----------------------------------------\n")

    print("\nCorrect hashes: %d" % matches)
    print("Mismatched hashed: %d" % errors)

    print("\nerrors will be written to the errors file\n")
    
    with open("errors", "w+") as f:
        for i in range(0, len(mismatch)):
            f.write("key: %d, expected: %s, retrieved: %s\n" % (mismatch[i][0], mismatch[i][1], mismatch[i][2]))

    print("DONE\n")
    print("----------------------------------------\n")

    sys.exit(0)
