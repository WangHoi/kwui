use std::ffi::CStr;

use kwui_sys::*;

pub struct ScriptValue {
    inner: *mut kwui_ScriptValue,
}
unsafe impl Send for ScriptValue {}
unsafe impl Sync for ScriptValue {}

impl ScriptValue {
    pub fn from(v: impl IntoScriptValue) -> Self {
        match v.into_script_value() {
            Ok(v) => v,
            Err(_) => Self::default(),
        }
    }
    pub(crate) fn from_inner(inner: *mut kwui_ScriptValue) -> Self {
        // eprintln!("from_inner {:?}", inner);
        Self {
            inner
        }
    }
    pub(crate) fn inner(&self) -> *mut kwui_ScriptValue {
        self.inner
    }
    pub(crate) fn leak(self) -> *mut kwui_ScriptValue {
        // eprintln!("leak {:?}", self.inner);
        let inner = self.inner;
        std::mem::forget(self);
        inner
    }
}

impl Drop for ScriptValue {
    fn drop(&mut self) {
        // eprintln!("drop {:?}", self.inner);
        unsafe {
            kwui_ScriptValue_delete(self.inner);
        }
    }
}

pub trait FromScriptValue: Sized {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()>;
}

pub trait IntoScriptValue {
    fn into_script_value(self) -> Result<ScriptValue, ()>;
}

impl Default for ScriptValue {
    fn default() -> Self {
        let inner = unsafe { kwui_ScriptValue_newNull() };
        Self { inner }
    }
}
impl FromScriptValue for () {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        Ok(())
    }
}

impl IntoScriptValue for () {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe { kwui_ScriptValue_newNull() };
        Ok(ScriptValue::from_inner(inner))
    }
}
impl FromScriptValue for bool {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        unsafe { Ok(kwui_ScriptValue_to_bool(value.inner)) }
    }
}

impl IntoScriptValue for bool {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe { kwui_ScriptValue_newBool(self) };
        Ok(ScriptValue::from_inner(inner))
    }
}

impl FromScriptValue for i32 {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        unsafe { Ok(kwui_ScriptValue_to_double(value.inner) as _) }
    }
}

impl IntoScriptValue for i32 {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe { kwui_ScriptValue_newDouble(self as _) };
        Ok(ScriptValue::from_inner(inner))
    }
}

impl IntoScriptValue for f32 {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe { kwui_ScriptValue_newDouble(self as _) };
        Ok(ScriptValue::from_inner(inner))
    }
}

impl FromScriptValue for String {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        unsafe {
            let mut len = 0;
            let data = kwui_ScriptValue_to_string(value.inner, &mut len);
            let slice = std::slice::from_raw_parts(data as *const u8, len);
            Ok(String::from_utf8_lossy(slice).to_string())
        }
    }
}

impl IntoScriptValue for String {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe {
            kwui_ScriptValue_newString(self.as_ptr() as _, self.len())
        };
        Ok(ScriptValue::from_inner(inner))
    }
}

impl IntoScriptValue for &str {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        let inner = unsafe {
            kwui_ScriptValue_newString(self.as_ptr() as _, self.len())
        };
        Ok(ScriptValue::from_inner(inner))
    }
}
