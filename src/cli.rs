#![macro_use]

extern crate getopts;

use std::env;

pub enum Error {
    ArgAutoexit,
    ExtraneousArg(String),
    MissingArg(String),
    OptParse(getopts::Fail),
}

impl From<getopts::Fail> for Error {
    fn from(err: getopts::Fail) -> Error {
        Error::OptParse(err)
    }
}

pub fn display_help_info(opts: &getopts::Options)
{
    println!("{}", opts.usage("xorer: Apply xorpads to encrypted 3DS files"));
}

pub fn parse_args() -> Result<(getopts::Options, getopts::Matches), Error>
{
    let args: Vec<String> = env::args().collect();

    let mut opts = getopts::Options::new();
    opts.optflag("h", "help", "Display this help and exit");
    opts.optflag("", "dumb", "Indiscriminate xor of any filetype");
    opts.optflag("", "inplace", "Modify original file");
    opts.optopt("o", "out", "Output file", "file");
    opts.optopt("e", "exheader", "NCCH exheader xorpad", "xorpad");
    opts.optopt("x", "exefs", "NCCH exefs (normal) xorpad", "xorpad");
    opts.optopt("7", "exefs7x", "NCCH exefs (7.x) xorpad", "xorpad");
    opts.optopt("r", "romfs", "NCCH romfs xorpad", "xorpad");
    opts.optopt("p", "partition", "NCSD partition number", "#");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => {
            display_help_info(&opts);
            return Err(Error::from(f));
        }
    };

    if matches.opt_present("help") {
        display_help_info(&opts);
        return Err(Error::ArgAutoexit);
    }

    Ok((opts, matches))
}

pub fn get_arg(string: &str, matches: &getopts::Matches) -> Result<String, Error>
{
    let arg_str = match matches.opt_str(string) {
        Some(s) => s,
        None => return Err(Error::MissingArg(String::from(string))),
    };
    Ok(arg_str)
}

#[macro_export]
macro_rules! open_arg_file {
    ($string:expr, $matches:expr) => ({
        try!(File::open(try!(::cli::get_arg($string, $matches))))
    })
}

#[macro_export]
macro_rules! extraneous_if {
    ($string:expr, $matches:expr) => ({
        if $matches.opt_present($string) {
            return Err(From::from(::cli::Error::ExtraneousArg(String::from($string))));
        }
    })
}
