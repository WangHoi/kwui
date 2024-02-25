use std::ffi::CStr;

use kwui_sys::*;

pub enum ScriptValue {
    Ref(*mut kwui_ScriptValue),
    Own(*mut kwui_ScriptValue),
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
    pub(crate) fn inner(&self) -> *mut kwui_ScriptValue {
        match self {
            ScriptValue::Ref(inner) => *inner,
            ScriptValue::Own(inner) => *inner,
        }
    }
    pub(crate) fn into_inner(self) -> *mut kwui_ScriptValue {
        let inner = match self {
            ScriptValue::Ref(inner) => inner,
            ScriptValue::Own(inner) => inner,
        };
        std::mem::forget(self);
        inner
    }
}

impl Drop for ScriptValue {
    fn drop(&mut self) {
        if let ScriptValue::Own(inner) = self {
            unsafe {
                kwui_ScriptValue_delete(*inner);
            }
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
        Self::Own(unsafe { kwui_ScriptValue_newNull() })
    }
}
impl FromScriptValue for () {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        Ok(())
    }
}

impl IntoScriptValue for () {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        Ok(ScriptValue::Own(unsafe { kwui_ScriptValue_newNull() }))
    }
}
impl FromScriptValue for bool {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        unsafe { Ok(kwui_ScriptValue_to_bool(value.inner())) }
    }
}

impl IntoScriptValue for bool {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        Ok(ScriptValue::Own(unsafe { kwui_ScriptValue_newBool(self) }))
    }
}

impl IntoScriptValue for i32 {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        Ok(ScriptValue::Own(unsafe {
            kwui_ScriptValue_newDouble(self as _)
        }))
    }
}

impl IntoScriptValue for f32 {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        Ok(ScriptValue::Own(unsafe {
            kwui_ScriptValue_newDouble(self as _)
        }))
    }
}

impl FromScriptValue for String {
    fn from_script_value(value: &ScriptValue) -> Result<Self, ()> {
        unsafe {
            let mut len = 0;
            let data = kwui_ScriptValue_to_string(value.inner(), &mut len);
            let slice = std::slice::from_raw_parts(data as *const u8, len);
            Ok(String::from_utf8_lossy(slice).to_string())
        }
    }
}

impl IntoScriptValue for &str {
    fn into_script_value(self) -> Result<ScriptValue, ()> {
        Ok(ScriptValue::Own(unsafe {
            kwui_ScriptValue_newString(self.as_ptr() as _, self.len())
        }))
    }
}
