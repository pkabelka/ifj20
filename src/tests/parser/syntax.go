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

func rets() (int, int) {
	return 10, 15
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
	identifier := "69"
	a := 100
	a = 100 + 5
	b := 20
	a, b = 158 * 8 + (4 - 6), 12 * 3
	a = ((((((b)))))) + (a)

	//func call
	print("aloha", 400, 158, 4.5, "heyhola")
	call(11)
	c := 0
	f := 4.5
	f = int2float(4)
	c = call(10)
	a, b = rets()
	a, identifier = function(a, f, "hey", 7)
	_, _ = rets()
	_, a = rets()
	a, _ = rets()
	a, b = call(1), (2 + 4) / 7
	a, b = call(1), call(2)

	return 4 * 12, 0 - 7
}

/* long comment 




*/

//if statements and scopes
func statements() {
	if 1 <= 2 {
	} else {
	}

	if 12 - 3 > 10 * 8 {
		if 1 + 2 < (2 + 6) / 18 {
			if 2 >= 0 {
				if 1 <= 1 {
					if "ha" == "hermelin" {
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
}

//for cycle
func cycle() {
	for i := 0; i < 10; i = i + 1 {
	}

	s := ""
	for ; s != "esc" ; {
		s, _ = inputs()
	}

	for a, b := 0, 0; a + b < 100; a, b = a + 1, b + 2 {
	}

	r := 100
	for r = 0; r < 100; r = r + 1 {
	}

	for x := 4; x < 20; x = x + 1 {
		for y := x; y / 2 < 20 * 4; y = y + 1 {
		}
	}
}
