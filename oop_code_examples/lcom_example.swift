class Helper {
    func h1() { }
    func h2() { }
}

class LCOMGood {
    var a: Int = 0
    var b: Int = 0

    func useA() {
        self.a += 1

    }

    func useB() {
        self.b += 1
    }

    func useBoth() {
        self.a += 1
        self.b += 1
    }
}

class LCOMBad {
    var a: Int = 0
    var b: Int = 0

    func onlyA1() {
        self.a += 1
    }

    func onlyA2() {
        self.a += 2
    }

    func onlyB1() {
        self.b += 1
    }
    
    func helperOnly() {
        Helper().h1()   
    }
}
