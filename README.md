# xorer
[![Build Status](https://travis-ci.org/archshift/xorer.svg?branch=master)](https://travis-ci.org/archshift/xorer)
[![Build Status](https://ci.appveyor.com/api/projects/status/9hrxmka4r8rey9e8?svg=true)](https://ci.appveyor.com/project/archshift/xorer)

A XORPad applier for decrypting 3DS files

### Download

[Nightly builds](http://builds.archshift.com/xorer/nightly) (sort by date)

## Using xorer
Using the program is easy. After it is built, simply run it from the command line with the 3DS application as the first argument, and the xorpad files as the subsequent arguments. Example:
```
xorer someapp.app -e exheader.xorpad -x exefs.xorpad -r romfs.xorpad -o outfile.cxi
```
If the program is a 7.x type program and you have both an `exefs_norm.xorpad` and an `exefs_7x.xorpad` file, specify the 7x variant as well using `-7 exefs_7x.xorpad`. If you've already merged them through some other means, just pass the merged file in with `-x`.

If you want to apply the xorpads in-place (that is, modifying the input file), simply specify the option `--inplace`. 

#### NCSD Options
NCSD files are characterized by either the `.3ds` or `.cci` extension. When you use these files with xorer, you have the options of:
 - Specifying the partition number to decrypt.  
   Use the option `-p num` where `num` is the partition number. The default option is partition 0.

## Dumb xoring
Sometimes it's convenient to just xor one file with another, without anything special going on behind the scenes. To do this, specify the option `--dumb`. Usage will look like this:
```
xorer --dumb somefile.bin xorpad.bin
```

## Building xorer
Building the program is very easy. Simply navigate to the directory and run:
```
cargo build --release
```
(Of course, you'll need to install Rust before doing so.)
