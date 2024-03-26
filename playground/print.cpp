//
// Created by Junhao Wang (@forkercat) on 3/22/24.
//

#include "print.h"

template class Object<int>;

template <typename T>
void Print(T value) {
	std::cout << value << std::endl;
}

void Test() {
	Print(10);
}
