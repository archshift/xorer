extern crate getopts;
extern crate crypto;

mod filedata;
mod xor;
mod cli;
mod types;
mod verify;

use std::io;
use std::fs;
use std::fs::{File,OpenOptions};
use std::str;
use types::{ncch,ncsd};

enum Error {
    IO(io::Error),
    CLI(cli::Error),
    XOR(xor::Error),
    HashFailure,
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error::IO(err)
    }
}
impl From<cli::Error> for Error {
    fn from(err: cli::Error) -> Error {
        Error::CLI(err)
    }
}
impl From<xor::Error> for Error {
    fn from(err: xor::Error) -> Error {
        Error::XOR(err)
    }
}

fn decrypt_cxi(ncch: &ncch::NCCH, file: &mut File, file_off: u64, matches: &getopts::Matches)
    -> Result<(), Error>
{
    let mut exheader_file = open_arg_file!("exheader", matches);
    if !try!(ncch.decrypt_exheader(file, &mut exheader_file, file_off)) {
        return Err(Error::HashFailure);
    }

    let mut exefs_file = open_arg_file!("exefs", matches);
    if ncch.flag_extra_keyslot {
        let mut exefs7x_file = open_arg_file!("exefs7x", matches);
        if !try!(ncch.decrypt_exefs7x(file, &mut exefs_file, &mut exefs7x_file, file_off)) {
            return Err(Error::HashFailure);
        }
    } else {
        if !try!(ncch.decrypt_exefs(file, &mut exefs_file, file_off)) {
            return Err(Error::HashFailure);
        }
    }

    if !ncch.flag_noromfs {
        let mut romfs_file = open_arg_file!("romfs", matches);
        if !try!(ncch.decrypt_romfs(file, &mut romfs_file, file_off)) {
            return Err(Error::HashFailure);
        }
    } else {
        extraneous_if!("romfs", matches);
    }

    Ok(())
}

fn decrypt_cfa(ncch: &ncch::NCCH, file: &mut File, file_off: u64, matches: &getopts::Matches)
    -> Result<(), Error>
{
    extraneous_if!("exheader", matches);
    extraneous_if!("exefs", matches);
    extraneous_if!("exefs7x", matches);

    if !ncch.flag_noromfs {
        let mut romfs_file = open_arg_file!("romfs", matches);
        if !try!(ncch.decrypt_romfs(file, &mut romfs_file, file_off)) {
            return Err(Error::HashFailure);
        }
    } else {
        extraneous_if!("romfs", matches);
    }

    Ok(())
}

fn decrypt_ncch(ncch: &ncch::NCCH, file: &mut File, file_off: u64, matches: &getopts::Matches)
    -> Result<(), Error>
{
    match ncch.get_container_type() {
        ncch::ContainerType::CXI => try!(decrypt_cxi(ncch, file, file_off, matches)),
        ncch::ContainerType::CFA => try!(decrypt_cfa(ncch, file, file_off, matches)),
    };

    Ok(())
}

fn decrypt_ncsd(ncsd: &ncsd::NCSD, file: &mut File, file_off: u64,
    matches: &getopts::Matches) -> Result<(), Error>
{
    let partition: usize = match cli::get_arg("partition", matches) {
        Ok(s) => match s.parse::<usize>() {
            Ok(d) => d,
            Err(_) => 0,
        },
        Err(_) => 0,
    };

    let ncch = try!(ncsd.get_ncch(file, file_off, partition));
    try!(decrypt_ncch(&ncch, file, ncsd.get_ncch_offset(partition), matches));

    Ok(())
}

fn decrypt_file(file: &mut File, matches: &getopts::Matches) -> Result<(), Error>
{
    let magic_data = try!(filedata::get_data::<[u8; 4]>(file, 0x100));
    let magic = match str::from_utf8(&magic_data) {
        Ok(a) => a,
        Err(e) => panic!(e),
    };

    match magic {
        "NCCH" => {
            let ncch = try!(ncch::NCCH::new(file, 0));
            try!(decrypt_ncch(&ncch, file, 0, matches));
        },
        "NCSD" => {
            let ncsd = try!(ncsd::NCSD::new(file, 0));
            try!(decrypt_ncsd(&ncsd, file, 0, matches));
        },
        _ => panic!(),
    };

    Ok(())
}

fn padxor_dumb(file: &mut File, pad: &mut File, matches: &getopts::Matches)
    -> Result<(), Error>
{
    let total_file_size = try!(file.metadata()).len();
    try!(xor::xor_file(file, pad, 0, 0, total_file_size));

    Ok(())
}

macro_rules! quit {
    ($expr:expr) => ({
        println!("ERROR: {}", $expr);
        return;
    })
}

fn main()
{
    let optsargs = match cli::parse_args() {
        Ok(args) => args,
        Err(e) => match e {
            cli::Error::ArgAutoexit => return,
            _ => quit!("Could not parse CLI arguments"),
        },
    };

    let opts = optsargs.0;
    let args = optsargs.1;

    if args.free.is_empty() {
        cli::display_help_info(&opts);
        return;
    }

    let mut filename: String;
    if args.opt_present("inplace") {
        filename = args.free[0].clone();
    } else {
        filename = match cli::get_arg("out", &args) {
            Ok(a) => a,
            Err(_) => {
                cli::display_help_info(&opts);
                return;
            },
        };
        // Only copy if output file doesn't exist.
        // Otherwise, will use the current contents.
        if fs::metadata(&filename).is_err() {
            if fs::copy(&args.free[0], &filename).is_err() {
                quit!("Could not copy input file");
            }
        }
    }

    let mut options = OpenOptions::new();
    options.read(true);
    options.write(true);

    let mut file = match options.open(&filename) {
        Ok(in_file) => in_file,
        Err(_) => quit!("Failed to open input file!"),
    };

    if args.opt_present("dumb") {
        let mut pad_file = match options.open(&args.free[1]) {
            Ok(in_file) => in_file,
            Err(_) => quit!("Failed to open input pad!"),
        };
        if padxor_dumb(&mut file, &mut pad_file, &args).is_err() {
            panic!();
        }
    } else {
        match decrypt_file(&mut file, &args) {
            Err(e) => match e {
                Error::IO(_) => quit!("Filesystem IO failed"),
                Error::XOR(_) => quit!("XORing files failed"),
                Error::HashFailure => quit!("Decryption failed - hash mismatch"),
                _ => panic!(e),
            },
            _ => (),
        }
    }
}
