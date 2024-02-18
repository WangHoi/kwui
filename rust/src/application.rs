use kwui_sys::*;
use std::ffi::CString;

pub struct Application {
    inner: *mut kwui_Application,
}

impl Application {
    pub fn new() -> Self {
        let args = std::env::args()
            .into_iter()
            .map(|a| CString::new(a).unwrap())
            .collect::<Vec<_>>();
        let mut args = args
            .iter()
            .map(|s| s.as_ptr() as *mut i8)
            .collect::<Vec<_>>();
        let argc = args.len() as _;
        let inner = unsafe { kwui_Application_new(argc, args.as_mut_ptr()) };
        Self { inner }
    }
    pub fn exec(&self) -> i32 {
        unsafe {
            kwui_Application_exec(self.inner)
        }
    }
}

impl Drop for Application {
    fn drop(&mut self) {
        unsafe {
            kwui_Application_delete(self.inner)
        }
    }
}