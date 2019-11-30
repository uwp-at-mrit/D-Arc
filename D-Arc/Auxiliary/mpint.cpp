#include "CppUnitTest.h"

#include "datum/natural.hpp"

using namespace WarGrey::SCADA;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary {
	private class MPNatural : public TestClass<MPNatural> {
	public:
		TEST_METHOD(Fixnum) {
			this->assert(Natural(),                    "00",               0U, 0U,  "Empty Natural");
			this->assert(Natural(0xDU),                "0D",               1U, 4U,  "#xDU");
			this->assert(Natural(0x4021U),             "4021",             2U, 15U, "#x4021U");
			this->assert(Natural(0xEFCDBA01U),         "EFCDBA01",         4U, 32U, "#xEFCDBA01U");
			this->assert(Natural(0x23456789U),         "23456789",         4U, 30U, "#x23456789U");
			this->assert(Natural(0x7FCDBA0123456789U), "7FCDBA0123456789", 8U, 63U, "#x7FCDBA0123456789U");
			this->assert(Natural(0xFECDBA0123456789U), "FECDBA0123456789", 8U, 64U, "#xFECDBA0123456789U");
		}

		TEST_METHOD(Memory) {
			uint8 ch[] = { 0xFE, 0xCD, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
			uint16 wch[] = { 0x0000, 0x0000, 0x7654, 0x3210, 0x0123, 0x4567, 0x89AB, 0xCDEF };
			uint16* pwch = wch; // test the constructor dispatcher
			std::string base256 = "1234567890ABCDEF";
			size_t chsize = sizeof(ch);
			size_t wchsize = sizeof(wch);

			this->assert(Natural(ch),                   "FECDBA98765432100123456789ABCDEF", chsize, 128U, "Base256");
			this->assert(Natural(pwch, 0, wchsize / 2), "765432100123456789ABCDEF",         12,     95U,  "Base512");
			this->assert(Natural(base256),              "31323334353637383930414243444546", 16,     126U, "#F1234567890ABCDEF");
			this->assert(Natural("ABCDEF"),             "4100420043004400450046",           11,     87U,  "#WABCDEF");
		}

		TEST_METHOD(Hexadecimal) {
			this->assert(Natural(16, "1234567890ABCDEF"), "1234567890ABCDEF", 8, 61U, "#x1234567890ABCDEF");
			this->assert(Natural(16, L"FEDCBA098765432"), "0FEDCBA098765432", 8, 60U, "#x0FEDCBA098765432");
			this->assert(Natural(16, L"000000789FEDCBA"), "0789FEDCBA",       5, 35U, "#x0000000789FEDCBA");
		}

		TEST_METHOD(Decimal) {
			this->assert(Natural(10, "0000000890"), "037A",     2, 10U, "0000000890");
			this->assert(Natural(10, L"123456789"), "075BCD15", 4, 27U, "1234567890");
			this->assert(Natural(10, L"098765432"), "05E30A78", 4, 27U, "098765432");
		}

		TEST_METHOD(Octal) {
			this->assert(Natural(8, "00000567"),          "0177",         2, 9U,  "#o00000567");
			this->assert(Natural(8, L"0123456776543210"), "053977FAC688", 6, 43U, "#o0123456776543210");
			this->assert(Natural(8, L"7654321001234567"), "FAC688053977", 6, 48U, "#o7654321001234567");
		}

	private:
		void assert(Natural& n, const char* representation, size_t size, size_t bits, Platform::String^ message) {
			//Assert::AreEqual(size, n.length(), (message + "[size]")->Data());
			//Assert::AreEqual(bits, n.integer_length(), (message + "[bits]")->Data());
			Assert::AreEqual(representation, n.to_hexstring().c_str(), message->Data());
		}
	};
}
