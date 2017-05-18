#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ptr;
use std::sync::Mutex;

mod env;
mod jvmti;
mod agentcontroller;

#[macro_use]
extern crate lazy_static;
extern crate libc;

lazy_static! {
    static ref STATIC_CONTEXT: Mutex<AgentContext> = Mutex::new(AgentContext::new());
}

pub struct AgentContext {
    ac: Option<agentcontroller::agentController>
}

impl AgentContext {
    pub fn new() -> AgentContext {
        AgentContext {
            ac: None,
        }
    }

    pub fn set(&mut self, a: agentcontroller::agentController) {
        match self.ac {
            Some(_) => panic!("Agent controller has already been set"),
            None => self.ac = Some(a),
        }
    }
}

#[no_mangle]
#[allow(unused_variables)]
pub extern fn Agent_OnLoad(vm: *mut jvmti::JavaVM, options: *mut ::std::os::raw::c_char,
                           reserved: *mut ::std::os::raw::c_void) -> jvmti::jint {
    let ac = env::JVMTIEnv::new(vm).and_then(|ti| agentcontroller::agentController::new(ti));

    match ac {
        Ok(a) =>
            STATIC_CONTEXT.lock().unwrap().set(a),
        Err(e) => {
            return 1;
        }
    }

    0
}
