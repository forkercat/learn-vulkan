//
// Created by Junhao Wang (@forkercat) on 3/11/24.
//

#include "core/core.h"

#include <iostream>
#include <vector>
#include <memory>

#include "print.h"

// void Print(int val) { PRINT("Print(int)"); }
void Print(int&& rref) { PRINT("Print(int&&)"); }
void Print(const int&) { PRINT("Print(const int&)"); }

// void Print(int& ref)
// {
// 	PRINT("Print(int&)");
// }

int main()
{
	int x = 0;
	int& ref = x;
	int&& rref = 1;

	Print(ref);	  // output: Print(int&)
	Print(rref);  // output: Print(int&)

	Print(std::move(ref));	 // output: Print(int&&)
	Print(std::move(rref));	 // output: Print(int&&)

	Print((int&&)ref);	 // output: Print(int&&)

	Print(std::move(4));

	Print(4);
}
