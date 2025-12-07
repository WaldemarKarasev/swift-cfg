class LCOMGood {
    var a: Int = 0
    var b: Int = 0

    func useA() {
        a += 1
    }

    func useB() {
        b += 1
    }

    func useBoth() {
        a += b
    }
}

class LCOMBad {
    var a: Int = 0
    var b: Int = 0

    func onlyA1() {
        a += 1
    }

    func onlyA2() {
        a += 2
    }

    func onlyB1() {
        b += 1
    }
}
