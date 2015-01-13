use std::fs::File;
use std::io;
use std::io::{Read, Seek, SeekFrom};
use std::mem;
use std::ptr;
use std::vec::Vec;

pub fn get_data<T: Clone>(file: &mut File, off: u64) -> Result<T, io::Error>
{
	let mut bytes_read = 0;
	let mut buf = vec![0;mem::size_of::<T>()];

	try!(file.seek(SeekFrom::Start(off)));

	while bytes_read < buf.len() {
		match file.read(&mut buf[bytes_read..]) {
            Ok(n) => bytes_read += n,
            Err(e) => return Err(e),
        };
	}

	// Ugly way to create a type without initializing it,
	// then copy the buffer into its memory.
	let mut ntype = Vec::<T>::with_capacity(1);
	unsafe { 
		ntype.set_len(1);
		ptr::copy(buf.as_ptr(), ntype.as_mut_ptr() as *mut u8, mem::size_of::<T>());
	};
	Ok(ntype[0].clone())
}
