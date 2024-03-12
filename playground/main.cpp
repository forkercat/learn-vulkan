//
// Created by Junhao Wang (@forkercat) on 3/11/24.
//

#include "core/core.h"

#include <iostream>
#include <vector>

int main()
{
	PRINT("hahah");

	PRINT("%s -> %s", "123", "456");

	std::string sb = "123";

	PRINT("------- %s", sb.c_str());

	PRINT(STR(123123));

	WARN_IF(true, "Hello World! %s", "123");

	PRINT_IF(true, "Hello!!!! %d", 9999);

	ERROR_IF(true, "Yes!!!!!! error!!!!!");

	PRINT("Cool");

	return 0;
}
