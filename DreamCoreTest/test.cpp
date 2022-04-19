#include "pch.h"
import Core.Reflection;

namespace Reflection {
	TEST(Core, Reflection) {
		// app source.cpp -std=c++20
		int argc = 3;
		const char* argv[3] = {
			"App",
			"C:\\Users\\yhtse\\source\\repos\\DreamEngine\\DreamCoreTest\\Samples\\class.cpp",
			"--"
		};
		int res = Dream::Reflection::getClassTypeDumpInfo(argc, argv);
		EXPECT_EQ(res, 7);
	}
} // namepsace Reflection;