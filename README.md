# xorer
![Build Status](https://travis-ci.org/archshift/xorer.svg?branch=master)

A XORPad applier for decrypting 3DS files

## Using xorer
Using the program is easy. After it is built, simply run it from the command line with the 3DS application as the first argument, and the xorpad files as the subsequent arguments. Example:
```
xorer someapp.app -e exheader.xorpad -x exefs.xorpad -r romfs.xorpad
```
If the program is a 7.x type program and you have both an `exefs_norm.xorpad` and an `exefs_7x.xorpad` file, specify the 7x variant as well using `-7 exefs_7x.xorpad`. If you've already merged them through some other means, just pass the merged file in with `-x`.

#### NCSD Options
NCSD files are characterized by either the `.3ds` or `.cci` extension. When you use these files with xorer, you have the options of:
 - Specifying the partition number to decrypt.  
   Use the option `-p num` where `num` is the partition number. The default option is partition 0.
 - Choosing whether to extract the NCCH partition as it decrypts. This will, instead of creating another NCSD file with the xorpads applied, instead create a NCCH file of only the specified partition.  
   Use the option `--extract`.

## Building xorer
Building the program is even easier. Simply navigate to the directory, make a new build directory, and run cmake then make.
```
cd /path/to/xorer
mkdir build && cd build
cmake ..
make
```
Obviously you'll first need to install cmake in order to build xorer.
