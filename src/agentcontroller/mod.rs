pub trait AgentController {
    fn setParameters(&mut self);
}

pub struct agentController<'a> {
    jvmti: ::env::JVMTIEnv,
    actions: &'a [&'a Action]
}

impl<'a> agentController<'a> {
    pub fn new(ti: ::env::JVMTIEnv) -> Result<agentController<'a>, ::jvmti::jint> {
        self::heaphistogram::heapHistogram::new(ti).map(|action|
        agentController{
            jvmti: ti,
            actions: &[&action],
        })
    }
}

trait Action {}

mod heaphistogram;
