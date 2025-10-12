func fib(_ n : Int) -> Int
{
    var n = 3
    switch n {
    case 0:
        print("zero")
    case 1...3:
        print("small")
        fallthrough               
    case 4...6 where n % 2 == 0:
        print("even range 4...6")
    case 7, 8, 9:
        print("medium")
    default:
        print("other")
    }

    outerLoop: for i in 1...3 {
        innerLoop: for j in 1...3 {
            if j == 2 {
                print("continue outer at i=\(i), j=\(j)")
                continue outerLoop
            }
            if i == 3 && j == 3 {
                print("breaking outer loop")
                break outerLoop
            }
            if i == 100 {
                print("breaking")
                break 
            }
            if i < 0 {
                return -1
            }
            print("i=\(i), j=\(j)")
        }
    }

    return 0
}