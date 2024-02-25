use kwui_sys::*;
use log;
use std::{ffi::CString, io::Read};

use crate::script_value::{FromScriptValue, IntoScriptValue, ScriptValue};

pub struct ScriptEngine;
pub struct ScriptEventHandler {
    event: CString,
    inner: *mut std::os::raw::c_void,
}

impl Drop for ScriptEventHandler {
    fn drop(&mut self) {
        ScriptEngine::remove_event_listener(self);
    }
}

impl ScriptEngine {
    pub fn load_file(path: &str) {
        let path = CString::new(path).unwrap();
        unsafe { kwui_ScriptEngine_loadFile(path.as_ptr()) }
    }
    pub fn call_global_function(name: &str, args: &[ScriptValue]) -> ScriptValue {
        let name = CString::new(name).unwrap();
        let mut args = args
            .iter()
            .map(|s| s.inner())
            .collect::<Box<[*mut kwui_ScriptValue]>>();
        let inner = unsafe {
            kwui_ScriptEngine_callGlobalFunction(name.as_ptr(), args.len() as _, args.as_mut_ptr())
        };
        ScriptValue::Own(inner)
    }
    pub fn add_global_function<R, Fun, A>(name: &str, func: Fun)
    where
        R: IntoScriptValue,
        Fun: Fn(A) -> R + 'static,
        A: ScriptFuntionParams,
    {
        let name = CString::new(name).unwrap();
        let closure: Box<Callback> =
            Box::new(Box::new(move |args| -> Result<ScriptValue, ()> {
                func(A::from_params(args)?).into_script_value()
            }) as Callback);
        unsafe {
            kwui_ScriptEngine_addGlobalFunction(
                name.as_ptr(),
                Some(invoke_closure),
                Box::into_raw(closure) as _,
            );
        }
    }
    pub fn remove_global_function(name: &str) {
        let name = CString::new(name).unwrap();
        unsafe {
            kwui_ScriptEngine_removeGlobalFunction(name.as_ptr());
        }
    }
    pub fn add_event_listener<R, Fun, A>(event: &str, func: Fun) -> ScriptEventHandler
    where
        R: IntoScriptValue,
        Fun: Fn(A) -> Result<R, ()> + 'static,
        A: ScriptFuntionParams,
    {
        todo!()
        /*
        let c_event = CString::new(event).unwrap();
        let closure = Box::new(func);
        let inner = Box::into_raw(closure) as _;
        unsafe {
            kwui_ScriptEngine_addEventListener(c_event.as_ptr(), Some(invoke_closure), inner);
        }
        ScriptEventHandler {
            event: c_event,
            inner,
        }
        */
    }
    pub fn remove_event_listener(handler: &mut ScriptEventHandler) {
        unsafe {
            kwui_ScriptEngine_removeEventListener(
                handler.event.as_ptr(),
                Some(invoke_closure),
                handler.inner,
            );
        }
    }
    pub fn post_event(event: &str, data: Option<impl IntoScriptValue>) {
        let c_event = CString::new(event).unwrap();
        unsafe {
            match data {
                Some(data) => {
                    let data = data.into_script_value();
                    if let Ok(data) = data {
                        kwui_ScriptEngine_postEvent1(c_event.as_ptr(), data.inner());
                    } else {
                        log::warn!("ScriptEngine::post_event '{}' failed", event);
                    }
                }
                None => {
                    kwui_ScriptEngine_postEvent0(c_event.as_ptr());
                }
            }
        }
    }
}

macro_rules! make_args {
	() => { { let args : [$crate::value::Value; 0] = []; args } };

	( $($s:expr),* ) => {
		{
			let args = [
			$(
				$crate::script_value::ScriptValue::from($s)
			 ),*
			];
			args
		}
	};
}

type Callback<'a> = Box<dyn Fn(&'a [ScriptValue]) -> Result<ScriptValue, ()> + 'a>;

pub trait ScriptFuntionParams: Sized {
    fn from_params(params: &[ScriptValue]) -> Result<Self, ()>;
}

impl ScriptFuntionParams for () {
    fn from_params(params: &[ScriptValue]) -> Result<Self, ()> {
        Ok(())
    }
}

impl<A1> ScriptFuntionParams for (A1,)
where
    A1: FromScriptValue,
{
    fn from_params(param: &[ScriptValue]) -> Result<Self, ()> {
        let a1 = A1::from_script_value(&param[0])?;
        Ok((a1, ))
    }
}

/*
impl<P> ScriptFunction for dyn IntoScriptFunction<P> {
    fn call(&self, args: &[ScriptValue]) -> Result<ScriptValue, ()> {
        (self as &dyn IntoScriptFunction<P>).call(args)
    }
}

impl<R, Fun> IntoScriptFunction<()> for Fun
where
    R: IntoScriptValue,
    Fun: Fn() -> R,
{
    fn call(&self, args: &[ScriptValue]) -> Result<ScriptValue, ()> {
        self().into_script_value()
    }
}
impl<R, Fun, A1> IntoScriptFunction<(A1)> for Fun
where
    R: IntoScriptValue,
    Fun: Fn(A1) -> R,
    A1: FromScriptValue,
{
    fn call(&self, args: &[ScriptValue]) -> Result<ScriptValue, ()> {
        let a1 = A1::from_script_value(args[0])?;
        self(a1).into_script_value()
    }
}
*/
unsafe extern "C" fn invoke_closure(
    argc: ::std::os::raw::c_int,
    argv: *mut *mut kwui_ScriptValue,
    udata: *mut ::std::os::raw::c_void,
) -> *mut kwui_ScriptValue {
    let closure = (udata as *mut Callback).as_mut().unwrap();
    let args: Vec<ScriptValue> = std::slice::from_raw_parts_mut(argv, argc as _)
        .into_iter()
        .map(|inner| ScriptValue::Ref(*inner) )
        .collect();
    match closure(&args) {
        Ok(ret) => ret.into_inner(),
        Err(_) => kwui_ScriptValue_newNull(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Application;

    fn f0(_: ()) {
        eprintln!("f0 called");
    }

    fn f1((a, ): (String, )) {
        // eprintln!("f1 called with {}", a);
    }

    #[test]
    fn global_func() {
        let app = Application::new();
        // ScriptEngine::add_global_function("f0", f0);
        ScriptEngine::add_global_function("f1", f1);
        // ScriptEngine::call_global_function("f0", &[]);
        for _ in 0..1000_000 {
        ScriptEngine::call_global_function("f1", &make_args!("abc"));
        }
    }
}
