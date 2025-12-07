import UIKit

class WMCExample: UIViewController {
    var value: Int
    var label: String
    var tag: String = "example"

    init(value: Int, label: String) {
        self.value = value
        self.label = label
        super.init(nibName: nil, bundle: nil)
    }
}
