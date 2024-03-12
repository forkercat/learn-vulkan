//
// Created by Junhao Wang (@forkercat) on 3/11/24.
//

#include "core/core.h"

#include <iostream>
#include <vector>

int main()
{
	LOG("hahah");

	LOG("%s -> %s", "123", "456");

	std::string sb = "123";

	LOG("------- %s", sb.c_str());

	LOG(STR(123123));

	WARN_IF(true, "Hello World! %s", "123");

	LOG_IF(true, "Hello!!!! %d", 9999);

	ERROR_IF(true, "Yes!!!!!! error!!!!!");

	LOG("Cool");

	return 0;
}
