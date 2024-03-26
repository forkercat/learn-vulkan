//
// Created by Junhao Wang (@forkercat) on 3/11/24.
//

#include "core/core.h"

#include <iostream>
#include <vector>
#include <memory>

#include "print.h"

int main()
{
	int num = 999999;
	int* p = &num;

	// void* vp = (void*) p;
	void* vp = static_cast<void*>(p);
	int* ip = static_cast<int*>(vp);
	void* vpp = reinterpret_cast<void*>(p);
	int* ipp = reinterpret_cast<int*>(vp);

	// double* doublePtr = static_cast<double*>(p);
	// char* charPtr = static_cast<char*>(p);

	double* doublePtr = (double*)p;

	// double* doublePtr = reinterpret_cast<double*>(p);

	PRINT("%lf", *doublePtr);

	return 0;
}
