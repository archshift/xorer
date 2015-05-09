use std::fs::File;
use std::cmp;
use std::io;
use std::io::{Read, Write, Seek, SeekFrom};

static BUF_SIZE: usize = 4 * 1024 * 1024;

pub enum Error {
    IO(io::Error),
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error::IO(err)
    }
}

pub fn xor(buf: &[u8], pad: &[u8]) -> Vec<u8>
{
    let mut out = vec![0; buf.len()];

    for i in 0..buf.len() {
        out[i] = buf[i] ^ pad[i];
    }

    return out;
}

fn verify_file_part_size(file: &File, file_pos: u64, size: u64) -> Result<bool, Error>
{
    let total_file_size = try!(file.metadata()).len();
    Ok((total_file_size > file_pos) && (total_file_size - file_pos >= size))
}

pub fn xor_file(file: &mut File, pad: &mut File,
    file_pos: u64, pad_pos: u64, size: u64) -> Result<(), Error>
{
    if !try!(verify_file_part_size(file, file_pos, size)) {
        panic!();
    }
    if !try!(verify_file_part_size(pad, pad_pos, size)) {
        panic!();
    }

    try!(file.seek(SeekFrom::Start(file_pos)));
    try!(pad.seek(SeekFrom::Start(pad_pos)));

    let mut total_read_size: u64 = 0;
    loop {
        let read_size: usize = cmp::min((size - total_read_size) as usize, BUF_SIZE);
        if read_size == 0 {
            break;
        }
        
        let mut file_buf: Vec<u8> = vec![0;read_size];
        let mut pad_buf: Vec<u8> = vec![0;read_size];

        // TODO: use file.read_all() once that's standardized
        let mut bytes_read = 0;
        while bytes_read < file_buf.len() {
            match file.read(&mut file_buf[bytes_read..]) {
                Ok(n) => bytes_read += n,
                Err(e) => return Err(Error::from(e)),
            };
        }

        // TODO: use file.read_all() once that's standardized
        let mut bytes_read = 0;
        while bytes_read < pad_buf.len() {
            match pad.read(&mut pad_buf[bytes_read..]) {
                Ok(n) => bytes_read += n,
                Err(e) => return Err(Error::from(e)),
            };
        }

        // file.read() seeks automatically, so let's return to the write position
        try!(file.seek(SeekFrom::Start(file_pos + total_read_size)));
        try!(file.write_all(&xor(&file_buf[..], &pad_buf[..])));

        total_read_size += read_size as u64;
    }

    Ok(())
}
