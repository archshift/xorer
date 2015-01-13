use std::fs::File;
use std::mem;
use std::slice;
use std::io;
use std::io::{Read, Seek, SeekFrom};

use crypto::digest::Digest;
use crypto::sha2::Sha256;
use crypto::util;

pub fn verify_hash(file: &mut File, offset: u64, region_size: usize, hash: &[u8]) -> Result<bool, io::Error>
{
	let mut bytes_read = 0;
	let mut buf: Vec<u8> = vec![0;region_size];

	try!(file.seek(SeekFrom::Start(offset)));

	while bytes_read < buf.len() {
		match file.read(&mut buf[bytes_read..]) {
            Ok(n) => bytes_read += n,
            Err(e) => return Err(e),
        };
	}

	let mut hasher = Sha256::new();
	hasher.input(&buf[..]);

	let mut comp_hash: Vec<u8> = vec![0;32];
	hasher.result(&mut comp_hash);
	hasher.reset();

	Ok(util::fixed_time_eq(&comp_hash, hash))
}

pub fn is_zeroed<T>(t: &T) -> bool
{
	let t_ptr: *const T = t;
	let t_ptr: *const u8 = t_ptr as *const u8;
	
    let slice = unsafe { slice::from_raw_parts(t_ptr, mem::size_of::<T>()) };
    for i in 0..slice.len() {
		if slice[i] != 0 {
			return false;
		}
	}
	return true;
}
