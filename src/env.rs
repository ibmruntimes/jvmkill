use ::std::io::Write;
use ::std::ptr;
use ::jvmti::jvmtiEnv;

pub trait JVMTI {}

#[derive(Clone, Copy)]
pub struct JVMTIEnv {
    jvmti: *mut jvmtiEnv
}

macro_rules! errln (
    ($($arg:tt)*) => { {
        writeln!(&mut ::std::io::stderr(), $($arg)*).unwrap();
    } }
);

impl JVMTIEnv {
    pub fn new(vm: *mut ::jvmti::JavaVM) -> Result<JVMTIEnv, ::jvmti::jint> {
        let mut penv: *mut ::std::os::raw::c_void = ptr::null_mut();
        let rc;
        unsafe {
            rc = (**vm).GetEnv.unwrap()(vm, &mut penv, ::jvmti::JVMTI_VERSION as i32);
        }
        if rc as u32 != ::jvmti::JNI_OK {
            errln!("ERROR: GetEnv failed: {}", rc);
            return Err(::jvmti::JNI_ERR);
        }
        Ok(JVMTIEnv { jvmti: penv as *mut jvmtiEnv })
    }
}

impl JVMTI for JVMTIEnv {}
