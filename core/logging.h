//
// Created by Junhao Wang (@forkercat) on 3/2/24.
//

#pragma once

#include <cstdio>

// #define LOG(...)                             \
// 	printf("[%s:%d] ", __FUNCTION__, __LINE__); \
// 	printf(__VA_ARGS__);                        \
// 	printf("\n")

#define LOG(fmt, ...) printf("[%s:%d] INFO: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define WARN(fmt, ...) printf("[%s:%d] WARN: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERROR(fmt, ...) printf("[%s:%d] ERROR: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOG_IF(exp, fmt, ...)        \
	do                               \
	{                                \
		if (exp)                     \
			LOG(fmt, ##__VA_ARGS__); \
	} while (0)

#define WARN_IF(exp, fmt, ...)        \
	do                                \
	{                                 \
		if (exp)                      \
			WARN(fmt, ##__VA_ARGS__); \
	} while (0)

#define ERROR_IF(exp, fmt, ...)        \
	do                                 \
	{                                  \
		if (exp)                       \
			ERROR(fmt, ##__VA_ARGS__); \
	} while (0)
