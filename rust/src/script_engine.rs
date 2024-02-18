use kwui_sys::*;
use std::{ffi::CString, io::Read};

pub struct ScriptEngine;

impl ScriptEngine {
    pub fn load_file(path: &str) {
        let path = CString::new(path).unwrap();
        unsafe {
            kwui_ScriptEngine_loadFile(path.as_ptr())
        }
    }
}
