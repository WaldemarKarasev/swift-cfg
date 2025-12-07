class Helper {
    func h1() { }
    func h2() { }
}

class RFCExample {
    func m1() {
        // вызов собственного метода
        m2()
    }

    func m2() {
        let helper = Helper()
        helper.h1()
    }

    func m3() {
        let helper = Helper()
        helper.h2()
    }
}
