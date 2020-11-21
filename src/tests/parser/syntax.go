//correct syntax
package main

//function header and return
func noargs() {
}

func empty() {
	return
}

func arg(a int) {
}

func args(a int, b int, c string) {
}

func ret() (int) {
	return 0
}

func without() int {
	return 0
}

func rets() (int, string) {
	return 10, "aloha"
}

func function(a int, b float64, c string, d int) (int, string) {
	return 0, "hey"
}

func call(a int) (int) {
	return 1
}

//expression, assignment, calling functions
func expression() (int, int) {
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

	return call(10) + call(((((1) + 2) -+-+ 4)) * 8), call(call(call((a))))
}

/* long comment 




*/

//if statements and scopes
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

//for cycle
func cycle() {
	for i := 0; i < 10; i = i + 1 {
	}

	for ; inputs() != "esc" ; {
	}

	for a, b := 0, 0; a + b < 100; a, b = a + 1, b + 2 {
	}

	r := 100
	for r = 0; r < 100; r = r + 1 {
	}

	for x := call(call(r)) - 1; x < 20; x = x + 1 {
		for y := x; y < 20 * call(x); y = y + call(1) {
		}
	}
}
