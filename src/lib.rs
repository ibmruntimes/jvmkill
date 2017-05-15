#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod env;
mod jvmti;

extern crate libc;

#[no_mangle]
#[allow(unused_variables)]
pub extern fn Agent_OnLoad(vm: *mut jvmti::JavaVM, options: *mut ::std::os::raw::c_char,
                           reserved: *mut ::std::os::raw::c_void) -> jvmti::jint {
    let tienv = env::JVMTIEnv::new(vm);

    0
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {}
}
