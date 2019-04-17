#include <iostream>
#include "tiny_signal.h"

class Slotter
{
public:
	Slotter(int id):
		id_(id)
	{}

	void fun0()
	{
		introduce();
		std::cout << __FUNCTION__ << std::endl;
	}

	void fun1(int a)
	{
		introduce();
		std::cout << __FUNCTION__ << "(" << a << ")" << std::endl;
	}

	void fun2(int a, float b)
	{
		introduce();
		std::cout << __FUNCTION__ << "(" << a << ", " << b << ")" << std::endl;
	}

private:
	void introduce()
	{
		std::cout << "I'm " << id_ << ":" << std::endl;
	}

	int id_;
};

void fun0()
{
	std::cout << __FUNCTION__ << std::endl;
}

void fun1(int a)
{
	std::cout << __FUNCTION__ << "(" << a << ")" << std::endl;
}

void fun2(int a, float b)
{
	std::cout << __FUNCTION__ << "(" << a << ", " << b << ")" << std::endl;
}

void printLine()
{
	std::cout << "-------------------------------" << std::endl;
}

int main(int argc, const char **argv)
{
	{
		Slotter slotter(0);

		tsignal::Signal signal;
		signal();
		printLine();

		signal.connect(&slotter, &Slotter::fun0);
		signal();
		printLine();

		signal.connect(&fun0);
		signal();
		printLine();

		tsignal::Connection lambda_connection =
		signal.connect([]() {
			std::cout << "lambda in signal" << std::endl;
		});
		signal();
		printLine();

		tsignal::Connection obj_lambda_connection =
			signal.connect(&slotter,[]() {
			std::cout << "object lambda in signal" << std::endl;
		});
		signal();
		printLine();

		signal.disconnect(&slotter, &Slotter::fun0);
		signal();
		printLine();

		signal.disconnect(&fun0);
		signal();
		printLine();

		lambda_connection.disconnect();
		signal();
		printLine();

		signal.disconnect(&slotter);
		signal();
		printLine();
	}

	std::cout << std::endl;

	{
		Slotter slotter(1);

		tsignal::Signal<int> signal;
		signal(1);
		printLine();

		signal.connect(&slotter, &Slotter::fun1);
		signal(1);
		printLine();

		signal.connect(&fun1);
		signal(1);
		printLine();

		tsignal::Connection lambda_connection =
			signal.connect([](int value) {
			std::cout << "lambda in signal: " << value << std::endl;
		});
		signal(1);
		printLine();

		tsignal::Connection obj_lambda_connection =
			signal.connect(&slotter, [](int value) {
			std::cout << "object lambda in signal: " << value << std::endl;
		});
		signal(1);
		printLine();

		signal.disconnect(&slotter);
		signal(1);
		printLine();

		signal.disconnect(&slotter, &Slotter::fun1);
		signal(1);
		printLine();

		signal.disconnect(&fun1);
		signal(1);
		printLine();

		lambda_connection.disconnect();
		signal(1);
		printLine();
	}


	std::cout << std::endl;

	{
		Slotter slotter(2);

		tsignal::Signal<int, float> signal;
		signal(2, 3.2f);
		printLine();

		signal.connect(&slotter, &Slotter::fun2);
		signal(2, 3.2f);
		printLine();

		signal.connect(&fun2);
		signal(2, 3.2f);
		printLine();

		tsignal::Connection lambda_connection =
			signal.connect([](int value, float fvalue) {
			std::cout << "lambda in signal: " << value << ", " << fvalue << std::endl;
		});
		signal(2, 3.2f);
		printLine();

		tsignal::Connection obj_lambda_connection =
			signal.connect(&slotter, [](int value, float fvalue) {
			std::cout << "object lambda in signal: " << value << ", " << fvalue << std::endl;
		});
		signal(2, 3.2f);
		printLine();

		signal.disconnect(&slotter);
		signal(2, 3.2f);
		printLine();

		signal.disconnect(&slotter, &Slotter::fun2);
		signal(2, 3.2f);
		printLine();

		signal.disconnect(&fun2);
		signal(2, 3.2f);
		printLine();

		lambda_connection.disconnect();
		signal(2, 3.2f);
		printLine();
	}

	std::cin.ignore();
	return 0;
}
