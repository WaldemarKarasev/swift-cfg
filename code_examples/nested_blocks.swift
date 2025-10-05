var x = 2
if x > 0 {
        print("positive")
        if x > 10 {
            print("large positive")
        } else {
            print("small positive")
        }
} else if x == 0 {
    print("zero")
} else {
    print("negative")
}