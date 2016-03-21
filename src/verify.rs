use std::fs::File;
use std::mem;
use std::slice;
use std::io;
use std::io::{Read, Seek, SeekFrom};

use crypto::digest::Digest;
use crypto::sha2::Sha256;

pub fn verify_hash(file: &mut File, offset: u64, region_size: usize, hash: &[u8]) -> Result<bool, io::Error>
{
	let mut buf: Vec<u8> = vec![0;region_size];

	try!(file.seek(SeekFrom::Start(offset)));
    try!(file.read_exact(&mut buf[..]));

	let mut hasher = Sha256::new();
	hasher.input(&buf[..]);

	let mut comp_hash: Vec<u8> = vec![0;32];
	hasher.result(&mut comp_hash);
	hasher.reset();

	Ok(is_buf_same(&comp_hash, hash))
}

pub fn is_buf_zeroed(t: &[u8]) -> bool
{
	for i in 0..t.len() {
		if t[i] != 0 {
			return false;
		}
	}
	return true;
}

pub fn is_zeroed<T: Copy>(t: &T) -> bool
{
	let t_ptr = (t as *const T) as *const u8;
    let slice = unsafe { slice::from_raw_parts(t_ptr, mem::size_of::<T>()) };
    return is_buf_zeroed(&slice);
}

pub fn is_buf_same(t: &[u8], r: &[u8]) -> bool
{
	if t.len() != r.len() {
		return false;
	}

	for i in 0..t.len() {
		if t[i] != r[i] {
			return false;
		}
	}
	return true;
}

pub fn is_same<T: Copy, R: Copy>(t: &T, r: &R) -> bool
{
	if mem::size_of::<T>() != mem::size_of::<R>() {
		return false;
	}

	let t_ptr = (t as *const T) as *const u8;
	let r_ptr = (r as *const R) as *const u8;

    let slice_t = unsafe { slice::from_raw_parts(t_ptr, mem::size_of::<T>()) };
    let slice_r = unsafe { slice::from_raw_parts(r_ptr, mem::size_of::<R>()) };

    return is_buf_same(&slice_t, &slice_r);
}
