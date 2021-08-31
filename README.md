# Simple Pub/Sub example 
### First, install OpenDDS
To install OpenDDS-3.14, download tar from https://opendds.org/downloads.html and 
follow instructions in [INSTALL.md](https://github.com/objectcomputing/OpenDDS/blob/master/INSTALL.md). For quick compile and install:
   1. tar xvzf OpenDDS-3.14.tar.gz
   2. cd OpenDDS-3.14
   3. To configure, type:
           ./configure --prefix=<path/to/preferred/location>
           For example, ./configure --prefix=/opt/OpenDDS-3.14
   4. make 
   5. sudo make install

To _compile_ and _run_ OpenDDS application, always export OpenDDS environment:
   source /path/to/opendds/setenv.sh 
See INSTALL.md for more info.

# Build and Run example
Export OpenDDS-3.14 environment - see above (source /path/to/opendds/setenv.sh)

To compile:
```sh
 $ mkdir build && cd build
 $ cmake ..
 $ make 
```
Start discover server (Terminal 1):
```sh
$ DCPSInfoRepo -ORBListenEndpoints iiop://127.0.0.1:12345
```

Publish (Terminal 2):
```sh
$ ./build/my_example_pub -DCPSConfigFile my_example_conf.ini
```
Subscriber (Terminal 3):
```sh
$ ./build/my_example_sub -DCPSConfigFile my_example_conf.ini
```
