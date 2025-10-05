let someInteger = 1

switch someInteger {
case 1:
    print("One")
    fallthrough 
case 2:
    print("Two")
default:
    print("Something else")
}