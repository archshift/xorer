use std::fs::File;
use std::io;
use std::io::{Read, Seek, SeekFrom};
use std::mem;
use std::slice;

pub fn get_data<T: Copy>(file: &mut File, off: u64) -> Result<T, io::Error>
{
    unsafe {
        let mut t: T = mem::zeroed();
        let t_bytes = slice::from_raw_parts_mut((&mut t as *mut T) as *mut u8, mem::size_of::<T>());
        try!(file.seek(SeekFrom::Start(off)));
        try!(file.read_exact(t_bytes));

        Ok(t)
    }
}
