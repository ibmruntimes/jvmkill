use ::std::ptr;

pub trait AgentController {
    fn setParameters(&mut self);
}

pub struct agentController {
    jvmti: ::env::JVMTIEnv,
    actions: Vec<Box<Action>>
}

static AGENT_CONTROLLER: Option<agentController> = None;

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

pub trait Action {}

mod heaphistogram;
