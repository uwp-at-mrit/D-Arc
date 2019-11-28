#include "CppUnitTest.h"

#include "datum/natural.hpp"

using namespace WarGrey::SCADA;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary {
	private class InfiniteNatural : public TestClass<InfiniteNatural> {
	public:
		TEST_METHOD(Fixnum) {
			this->assert(Natural(),                    "00",               "Empty Natural");
			this->assert(Natural(0xDU),                "0D",               "0xDU");
			this->assert(Natural(0x4021U),             "4021",             "0x4021U");
			this->assert(Natural(0xEFCDBA01U),         "EFCDBA01",         "0xEFCDBA01U");
			this->assert(Natural(0x23456789U),         "23456789",         "0x23456789U");
			this->assert(Natural(0x7FCDBA0123456789U), "7FCDBA0123456789", "0x7FCDBA0123456789U");
			this->assert(Natural(0xFECDBA0123456789U), "FECDBA0123456789", "0xFECDBA0123456789U");
		}

		TEST_METHOD(CharArray) {
			char ch[] = { 0xFE, 0xCD, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

			this->assert(Natural(ch), "FECDBA98765432100123456789ABCDEF", "Char Array");
		}

		TEST_METHOD(WideCharArray) {
			wchar_t wch[] = { 0x7654, 0x3210, 0x0123, 0x4567, 0x89AB, 0xCDEF };

			this->assert(Natural(wch), "765432100123456789ABCDEF", "Wide Char Array");
		}

	private:
		void assert(Natural& n, const char* representation, Platform::String^ message) {
			Assert::AreEqual(representation, n.to_hexstring().c_str(), message->Data());
		}
	};
}
