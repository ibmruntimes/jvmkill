pub struct heapHistogram {
    jvmti: ::env::JVMTIEnv,
}

impl heapHistogram {
    pub fn new(jvmti: ::env::JVMTIEnv) -> Result<heapHistogram, ::jvmti::jint> {
        Ok(heapHistogram {
            jvmti: jvmti
        })
    }
}

impl super::Action for heapHistogram {}
