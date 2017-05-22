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
    static ref RAW_MONITOR_ID: Mutex<env::RawMonitorID> = Mutex::new(env::RawMonitorID::new());
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
    let mut jvmti_env = env::JVMTIEnv::new(vm);
    let ac = jvmti_env.and_then(|ti| agentcontroller::agentController::new(ti));

    match ac {
        Ok(a) =>
            STATIC_CONTEXT.lock().unwrap().set(a),
        Err(e) => {
            return 1;
        }
    }

    // register resource exhaustion callback
    let id = RAW_MONITOR_ID.lock().unwrap();
    jvmti_env.CreateRawMonitor("jvmkill", id).unwrap();

    0
}

/*
int setCallbacks(jvmtiEnv *jvmti) {
   jvmtiError err;

   err = jvmti->CreateRawMonitor("jvmkillMonitor", &monitorID);
   if (err != JVMTI_ERROR_NONE) {
      std::cerr << "ERROR: CreateRawMonitor failed: " << err << std::endl;
      return JNI_ERR;
   }

   jvmtiEventCallbacks callbacks;
   memset(&callbacks, 0, sizeof(callbacks));

   callbacks.ResourceExhausted = &resourceExhausted;

   err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
   if (err != JVMTI_ERROR_NONE) {
      std::cerr << "ERROR: SetEventCallbacks failed: " << err << std::endl;
      return JNI_ERR;
   }

   err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_RESOURCE_EXHAUSTED, NULL);
   if (err != JVMTI_ERROR_NONE) {
      std::cerr << "ERROR: SetEventNotificationMode failed: %d" << err << std::endl;
      return JNI_ERR;
   }

   return JNI_OK;
}

*/
