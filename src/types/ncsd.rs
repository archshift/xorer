use std::fs::File;
use std::io;

use filedata;
use types::ncch;

pub struct NCSD {
	partition_table: [Partition;8],
}

impl NCSD {
	pub fn new(file: &mut File, offset: u64) -> Result<NCSD, io::Error>
	{
		Ok(NCSD {
			partition_table: try!(filedata::get_data::<[Partition;8]>(file, offset + 0x120)),
		})
	}

	pub fn get_ncch(&self, file: &mut File, ncsd_file_pos: u64, partition: usize)
		-> Result<ncch::NCCH, io::Error>
	{
		let offset: u64 = self.partition_table[partition].offset as u64 * 0x200;
		ncch::NCCH::new(file, ncsd_file_pos + offset)
	}

	pub fn get_ncch_offset(&self, partition: usize) -> u64
	{
		self.partition_table[partition].offset as u64 * 0x200
	}
}

#[repr(C, packed)]
#[derive(Clone,Copy,Debug)]
struct Partition {
	pub offset: u32,
	pub size: u32,
}
