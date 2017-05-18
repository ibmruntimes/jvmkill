use ::std::ptr;

pub trait AgentController {
    fn setParameters(&mut self);
}

pub struct agentController {
    jvmti: ::env::JVMTIEnv,
    actions: Vec<Box<Action>>
}

impl agentController {
    pub fn new(ti: ::env::JVMTIEnv) -> Result<agentController, ::jvmti::jint> {
        self::heaphistogram::heapHistogram::new(ti).map(|action| {
            agentController {
                jvmti: ti,
                actions: vec![Box::new(action)],
            }
        })
    }
}

// TODO: use a raw monitor to provide the necessary synchronisation
unsafe impl Send for agentController {}
unsafe impl Sync for agentController {}

pub trait Action {}

mod heaphistogram;
