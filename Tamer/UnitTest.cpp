#include "CppUnitTest.h"

#include "syslog.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tamer {
    TEST_CLASS(UnitTest1) {
    public:
        TEST_METHOD(TestMethod1) {
			char* expected = "expected";
			char* actual = "actual";

			Assert::AreEqual(expected, actual, false, L"message");
        }
    };
}
