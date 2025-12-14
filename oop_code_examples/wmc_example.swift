import UIKit

class WMCExample: UIViewController {
    var value: Int
    var label: String
    var tag: String = "example"
    var data: Data = Data()

    init(value: Int, label: String) {
        self.value = value
        self.label = label
        foo(label)
        doo(self.label)
        let l = Data()
        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder: NSCoder) {
        self.value = 0
        self.label = "default"
        super.init(coder: coder)
    }

    convenience override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: Bundle?) {
        self.init(value: 1, label: "from nib init")
    }

    convenience init() {
        self.init(value: 1, label: "default")
    }

    convenience init(value: Int) {
        self.init(value: value, label: "default")
    }

    convenience init?(fromString string: String) {
        let parts = string.split(separator: ":", maxSplits: 1).map(String.init)
        guard parts.count == 2, let intValue = Int(parts[0]) else {
            return nil
        }
        self.init(value: intValue, label: parts[1])
    }

    convenience init!(maybeValue: Int?) {
        guard let v = maybeValue else {
            return nil
        }
        self.init(value: v, label: "from init!")
    }

    func increment() {
        value += 1
    }

    func reset() {
        value = 0
    }

    func isEven() -> Bool {
        return value % 2 == 0
    }

    deinit {
    }
}


class FirstChild : WMCExample {
    var data: String = "line"
}

class SecondChild : WMCExample {
    var data: String = "line"
    func test_func() {
        
    }
}

class ThirdChild : SecondChild {
    var data: String = "line"

    func foo() {
        value = 0
    }

}