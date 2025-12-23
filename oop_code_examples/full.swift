class Helper {
    func h1() { }
    func h2() { }
}

class Repo {
    func save() { }
}

class BaseController {
    var id: Int = 0

    func baseReset() {
        self.id = 0
    }
}

class FullMetricsExample: BaseController {
    var value: Int = 0
    var label: String = ""

    // WMC + LCOM (value)
    func inc() {
        self.value += 1
    }

    func resetValue() {
        self.value = 0
    }

    // LCOM (label)
    func setLabelOk() {
        self.label = "ok"
    }

    // RFC + CBO 
    func work() -> Repo {
        self.inc()          

        Helper().h1()       
        let r = Repo()
        r.save()            
        return r
    }

    // RFC + CBO
    func moreWork() {
        Helper().h2()
    }

    // RFC (super) + LCOM (value)
    func resetAll() {
        super.baseReset()  
        self.resetValue()
    }
}

// NOC (BaseController)
class ChildA: BaseController { }
class ChildB: BaseController { }
