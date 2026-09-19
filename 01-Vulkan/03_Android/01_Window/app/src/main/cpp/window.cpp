/***************************************************************************
 * S.G.M.P 
 * @file: window.cpp
 * @brief:
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: Saturday 2026-09-19 10:11:25 AM
 ***************************************************************************/

#include <stdio.h>
#include <android/log.h>
#include <android_native_app_glue.h>


// int main(void)
void android_main(struct android_app* state)
{
	// Code
	__android_log_print(ANDROID_LOG_INFO, "MST: ", "Hello, World!!!\n");

	// return(0);
}
