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
        print("i=\(i), j=\(j)")
    }
}