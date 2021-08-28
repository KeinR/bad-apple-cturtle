 /* L1: Loopy C-Turtles
 * C-Turtle Author: Jesse Walker-Schalder 
 * Original Code Author: Jesse Walker-Schalder and Jan Pearce
 * Modified by: FIXME
 */

#include "CTurtle.hpp";   //This brings in the CTurtle library for use
#include <iostream>
namespace ct = cturtle;  //This makes it possible to use the CTurtle commands using ct::

int main() {

	char answer;
	std::cout << "Do you like purple?\n";
	std::cin >> answer;

	ct::TurtleScreen scr;

	scr.bgcolor({ "white" });
	ct::Turtle turtle(scr);

	turtle.speed(ct::TS_SLOWEST);

	if (answer == 'Y' || answer == 'y') {
		turtle.fillcolor({ "purple" });
	}
	else {
		turtle.fillcolor({ "red" });
	}

	turtle.begin_fill();
	for (int i = 0; i < 4; i++) {
		turtle.forward(50);
		turtle.right(90);
	}
	turtle.end_fill();
	turtle.penup();
	turtle.hideturtle();
	turtle.right(180);
	turtle.forward(120);
	turtle.left(90);
	turtle.forward(100);
	turtle.right(-90);
	turtle.pendown();
	turtle.fillcolor({ "black" });

	turtle.write("We're glad you're in Data Structures!");

	scr.exitonclick();  //exists graphics screen
	return 0;
}
