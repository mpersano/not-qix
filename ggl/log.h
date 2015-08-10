#pragma once

#if defined(ANDROID)

#include <android/log.h>

#define log_error(fmt, args...) \
	__android_log_print(ANDROID_LOG_ERROR, "ggl-app", "%s(%d): " fmt "\n", __FILE__, __LINE__, ##args);

#define log_info(fmt, args...) \
	__android_log_print(ANDROID_LOG_INFO, "ggl-app", "%s(%d): " fmt "\n", __FILE__, __LINE__, ##args);

#else

#define log_error(fmt, args...) \
	fprintf(stderr, "[ERROR] %s(%d): " fmt "\n", __FILE__, __LINE__, ##args);

#define log_info(fmt, args...) \
	fprintf(stderr, "[INFO] %s(%d): " fmt "\n", __FILE__, __LINE__, ##args);

#endif
