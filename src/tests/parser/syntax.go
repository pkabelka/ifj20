//correct syntax
package main

//function header
func noargs() {
}

func arg(a int) {
}

func args(a int, b int, c string) {
}

func ret() (int) {
}

func rets() (int, string) {
}

func function(a int, b float64, c string, d int) (int, string) {
}

func call(a int) (int) {

}

//expression, assignment, calling functions
func expression() {
	//basic expressions
	identifier = "69"
	a := 100
	a = 100 + 5
	a, b = 158 * 8 + (4 - 6), 12 * 3
	a = ((((((b)))))) + (a)

	//func call
	c := call(10)
	c = call(call(call(10)))
	c = call((1 + 5) + (b - a) * call(call(12) - (a - 6))) + call(4)
	x, y := call(c), call(a)
	_, _ = rets()
	_, a = rets()
	a, _ = rets()

	//unary operators
	x, y = -call(a), +call(b)
	a = +(-(+(-a)))
	a = +-+-+-+-a

	//TODO: use inter func in expression
}

/* long comment 




*/

//if statements
func statements() {
	if 0 {
	} else {
	}

	if call(10) > 10 {
		if 1 < 2 {
			if 2 >= 0 {
				if 1 <= 1 {
					if call(1) * 3 == call(2) / 4 + 5 {
					} else {
					}
				} else {
				}
			} else {
			}
		} else {
		}
	} else {
		call(10)
		call(12)
	}

	if -10 {
	} else {
		if inputs() == "string" {
		} else {
		}
	}
}
