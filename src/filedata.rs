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

	let ntype: &T = unsafe { mem::transmute(buf.as_ptr()) };
	Ok(ntype.clone())
}
