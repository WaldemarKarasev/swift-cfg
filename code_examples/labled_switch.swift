var n = 3
lol: switch n {
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