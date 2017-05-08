#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ptr;

extern crate libc;

#[allow(dead_code)]
mod jvmti {
    include!(concat!(env!("OUT_DIR"), "/jvmti_bindings.rs"));
}

#[no_mangle]
#[allow(non_snake_case, unused_variables)]
pub extern fn Agent_OnLoad(vm: *mut jvmti::JavaVM, options: *mut ::std::os::raw::c_char,
                           reserved: *mut ::std::os::raw::c_void) -> jvmti::jint {
    unsafe {
        //        pub GetEnv: ::std::option::Option<unsafe extern "C" fn(vm: *mut JavaVM,
        //                                                               penv:
        //                                                               *mut *mut ::std::os::raw::c_void,
        //                                                               version: jint)
        let mut penv: *mut ::std::os::raw::c_void = ptr::null_mut();
        (**vm).GetEnv.unwrap()(vm, &mut penv, jvmti::JVMTI_VERSION as i32);
    }

    0
}


#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {}
}