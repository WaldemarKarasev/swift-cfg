class Base {
    var x: Int

    // designated
    init(x: Int) {          
        self.x = x
    }

    // required designated
    required init() {       
        self.x = 0
    }
}

class Child: Base {

    required init() {       
        super.init()
    }

    convenience init(defaultX: Bool) {
        self.init(x: defaultX ? 10 : 20)
    }

    override init(x: Int) {  
        super.init(x: x)
    }
}
