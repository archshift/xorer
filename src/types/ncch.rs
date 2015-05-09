use std::fs::File;
use std::str;
use std::io;

use filedata;
use xor;
use verify;

pub enum ContainerType {
	CXI,
	CFA,
}

#[derive(Debug)]
pub struct NCCH {
	pub exheader_size: u64,
    pub exefs_offset:  u64,
    pub exefs_size:    u64,
    pub romfs_offset:  u64,
    pub romfs_size:    u64,
    pub flag_extra_keyslot: bool,
    pub flag_data:     bool,
    pub flag_exec:     bool,
    pub flag_noromfs:  bool,
    pub flag_nocrypto: bool,

    pub exheader_hash: [u8; 0x20],
    pub exefs_hash:    [u8; 0x20],
    pub exefs_hash_region_size: u64,
    pub romfs_hash:    [u8; 0x20],
    pub romfs_hash_region_size: u64,
}

impl NCCH {
	pub fn new(file: &mut File, offset: u64) -> Result<NCCH, io::Error>
	{
		let flags_crypto:       u8 = try!(filedata::get_data::<u8>(file, offset + 0x188 + 3));
		let flags_content_type: u8 = try!(filedata::get_data::<u8>(file, offset + 0x188 + 5));
		let flags_data_format:  u8 = try!(filedata::get_data::<u8>(file, offset + 0x188 + 7));

		Ok(NCCH {
			exheader_size: try!(filedata::get_data::<u32>(file, offset + 0x180)) as u64 * 0x200,
			exefs_offset:  try!(filedata::get_data::<u32>(file, offset + 0x1A0)) as u64 * 0x200,
			exefs_size:    try!(filedata::get_data::<u32>(file, offset + 0x1A4)) as u64 * 0x200,
			romfs_offset:  try!(filedata::get_data::<u32>(file, offset + 0x1B0)) as u64 * 0x200,
			romfs_size:    try!(filedata::get_data::<u32>(file, offset + 0x1B4)) as u64 * 0x200,

			flag_extra_keyslot: flags_crypto != 0,
			flag_data:     (flags_content_type & 1) != 0,
			flag_exec:     (flags_content_type & 2) != 0,
			flag_noromfs:  (flags_data_format & 2)  != 0,
			flag_nocrypto: (flags_data_format & 4)  != 0,

			exheader_hash: try!(filedata::get_data::<[u8; 0x20]>(file, offset + 0x160)),
			exefs_hash:    try!(filedata::get_data::<[u8; 0x20]>(file, offset + 0x1C0)),
			exefs_hash_region_size: try!(filedata::get_data::<u32>(file, offset + 0x1A8)) as u64 * 0x200,
			romfs_hash:    try!(filedata::get_data::<[u8; 0x20]>(file, offset + 0x1E0)),
			romfs_hash_region_size: try!(filedata::get_data::<u32>(file, offset + 0x1B8)) as u64 * 0x200,
		})
	}

	pub fn get_container_type(&self) -> ContainerType
	{
		if self.flag_data && !self.flag_exec {
	        return ContainerType::CFA;
		} else {
	        return ContainerType::CXI;
	    }
	}

	pub fn decrypt_exheader(&self, ncch_file: &mut File, pad: &mut File,
    	ncch_file_pos: u64) -> Result<bool, xor::Error>
	{
		try!(xor::xor_file(ncch_file, pad, ncch_file_pos + 0x200, 0, 0x800));

		Ok(try!(verify::verify_hash(ncch_file, ncch_file_pos + 0x200, 0x400, 
			&self.exheader_hash)))
	}

	pub fn decrypt_exefs(&self, ncch_file: &mut File, pad: &mut File,
    	ncch_file_pos: u64) -> Result<bool, xor::Error>
	{
		let exefs_pos = ncch_file_pos + self.exefs_offset;
		try!(xor::xor_file(ncch_file, pad, exefs_pos, 0, self.exefs_size));
		
		Ok(try!(verify::verify_hash(ncch_file, exefs_pos, 
			self.exefs_hash_region_size as usize, &self.exefs_hash)))
	}

	pub fn decrypt_exefs7x(&self, ncch_file: &mut File, pad: &mut File, pad_7x: &mut File, 
		ncch_file_pos: u64) -> Result<bool, xor::Error>
	{
		let exefs_pos = ncch_file_pos + self.exefs_offset;
		try!(xor::xor_file(ncch_file, pad, exefs_pos, 0, 0x200));

		let exefs = try!(EXEFS::new(ncch_file, exefs_pos));
		let code_hdr = match exefs.get_code_header() {
			Some(hd) => hd,
			None => panic!(),
		};

    	let code_end_off: u64 = (0x200 + code_hdr.file_off + code_hdr.file_size) as u64;

		// Everything up to the start of the code section
	    // (excluding the header, which is already decrypted)
	    let sect_start: u64 = 0x200;
	    let sect_size:  u64 = code_hdr.file_off as u64;
    	try!(xor::xor_file(ncch_file, pad, exefs_pos + sect_start, sect_start, sect_size));

    	// Everything in the code section
		let sect_start: u64 = sect_start + sect_size;
	    let sect_size:  u64 = code_hdr.file_size as u64;
    	try!(xor::xor_file(ncch_file, pad_7x, exefs_pos + sect_start, sect_start, sect_size));

    	// Everything after the end of the code section
    	let sect_start: u64 = sect_start + sect_size;
	    let sect_size:  u64 = self.exefs_size - code_end_off;
    	try!(xor::xor_file(ncch_file, pad, exefs_pos + sect_start, sect_start, sect_size));

		Ok(try!(verify::verify_hash(ncch_file, exefs_pos, 
				self.exefs_hash_region_size as usize, &self.exefs_hash)) 
			&& try!(exefs.verify_hashes(ncch_file, exefs_pos)))
	}

	pub fn decrypt_romfs(&self, ncch_file: &mut File, pad: &mut File,
    	ncch_file_pos: u64) -> Result<bool, xor::Error>
	{
		let romfs_pos = ncch_file_pos + self.romfs_offset;
		try!(xor::xor_file(ncch_file, pad, romfs_pos, 0, self.romfs_size));
		
		Ok(try!(verify::verify_hash(ncch_file, romfs_pos, 
			self.romfs_hash_region_size as usize, &self.romfs_hash)))
	}
}

#[repr(C, packed)]
#[derive(Clone,Copy,Debug)]
pub struct FileHeader {
	pub filename: [u8; 8],
	pub file_off: u32,
	pub file_size: u32,
}

pub type FileHash = [u8; 32];

#[derive(Debug)]
pub struct EXEFS {
	pub headers: [FileHeader; 10],
	pub hashes: [FileHash; 10],
}

impl EXEFS {
	pub fn new(file: &mut File, offset: u64) -> Result<EXEFS, io::Error>
	{
		let mut exefs = EXEFS {
			headers: try!(filedata::get_data::<[FileHeader; 10]>(file, offset + 0)),
			hashes:  try!(filedata::get_data::<[FileHash; 10]>(file, offset + 0xC0)),
		};

		return Ok(exefs);
	}

	pub fn get_code_header(&self) -> Option<FileHeader>
	{
		for i in 0..10 {
            let header_filename = match str::from_utf8(&self.headers[i].filename[0..5]) {
            	Ok(a) => a,
            	Err(_) => return None
            };
            if header_filename == ".code" {
                return Some(self.headers[i]);
            }
        }
        None
	}

	pub fn verify_hashes(&self, file: &mut File, exefs_offset: u64) -> Result<bool, io::Error>
	{

		for i in 0..10 {            
            // If the header and the hash are both zero-initialized, there's no code to verify
            if verify::is_zeroed(&self.headers[i]) && verify::is_zeroed(&self.hashes[9-i]) {
                return Ok(true);
            }

            // For some reason, the hashes are in reverse order...
            if !try!(verify::verify_hash(file, exefs_offset + 0x200 + self.headers[i].file_off as u64, 
				self.headers[i].file_size as usize, &self.hashes[9-i])) {
            	return Ok(false);
            } 
        }
        Ok(true)
	}
}
