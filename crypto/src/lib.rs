// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#![cfg_attr(feature = "with-bench", feature(test))]

extern crate rustc_serialize as serialize;

#[cfg(all(test, feature = "with-bench"))]
extern crate test;

pub mod buffer;
mod cryptoutil;
pub mod digest;
pub mod sha2;
mod simd;
pub mod symmetriccipher;
