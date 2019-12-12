#include "CppUnitTest.h"

#include "datum/natural.hpp"
#include "datum/string.hpp"

#include "syslog.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary::MPNatural {
	static void assert(Natural& n, const char* representation, size_t size, size_t bits, Platform::String^ message) {
		Assert::AreEqual(representation, (const char*)n.to_hexstring().c_str(), message->Data());
		Assert::AreEqual(size, n.length(), (message + "[size]")->Data());
		Assert::AreEqual(bits, n.integer_length(), (message + "[bits]")->Data());
	}

	static void assert(Natural& n, const char* representation, size_t bits, Platform::String^ message) {
		Assert::AreEqual(representation, (const char*)n.to_hexstring().c_str(), message->Data());
		Assert::AreEqual(bits, n.integer_length(), (message + "[bits]")->Data());
	}

	static void assert(Natural& n, Natural& control, Platform::String^ message) {
		Assert::AreEqual((const char*)control.to_hexstring().c_str(), (const char*)n.to_hexstring().c_str(), message->Data());
		Assert::AreEqual(control.length(), n.length(), (message + "[size]")->Data());
		Assert::AreEqual(control.integer_length(), n.integer_length(), (message + "[bits]")->Data());
	}

	static uint8 ch[] = { 0xFE, 0xCD, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
	static uint16 wch[] = { 0x0000, 0x0000, 0x7654, 0x3210, 0x0123, 0x4567, 0x89AB, 0xCDEF };
	
	private class Construction : public TestClass<Construction> {
	public:
		TEST_METHOD(Fixnum) {
			assert(Natural(),                    "00",               0U, 0U,  "Empty Natural");
			assert(Natural(0xDU),                "0D",               1U, 4U,  "#xDU");
			assert(Natural(0x4021U),             "4021",             2U, 15U, "#x4021U");
			assert(Natural(0xEFCDBA01U),         "EFCDBA01",         4U, 32U, "#xEFCDBA01U");
			assert(Natural(0x23456789U),         "23456789",         4U, 30U, "#x23456789U");
			assert(Natural(0x7FCDBA0123456789U), "7FCDBA0123456789", 8U, 63U, "#x7FCDBA0123456789U");
			assert(Natural(0xFECDBA0123456789U), "FECDBA0123456789", 8U, 64U, "#xFECDBA0123456789U");
		}

		TEST_METHOD(Memory) {
			static std::string M_ID = "10";
			static std::string M_KEY = "10121";
			static std::string HW_ID = "12345";
			uint16* pwch = wch; // test the constructor dispatcher
			size_t chsize = sizeof(ch);
			size_t wchsize = sizeof(wch);

			assert(Natural(ch),                   "FECDBA98765432100123456789ABCDEF", chsize, 128U, "Base256");
			assert(Natural(pwch, 0, wchsize / 2), "765432100123456789ABCDEF",         12,     95U,  "Base512");
			assert(Natural(M_ID),                 "3130",                             2,      14U,  "M_ID");
			assert(Natural(M_KEY),                "3130313231",                       5,      38U,  "M_KEY");
			assert(Natural(HW_ID),                "3132333435",                       5,      38U,  "HW_ID");
			assert(Natural("ABCDEF"),             "4100420043004400450046",           11,     87U,  "#WABCDEF");
		}

		TEST_METHOD(Hexadecimal) {
			assert(Natural(16, "1234567890ABCDEF"), "1234567890ABCDEF", 8, 61U, "#x1234567890ABCDEF");
			assert(Natural(16, L"FEDCBA098765432"), "0FEDCBA098765432", 8, 60U, "#x0FEDCBA098765432");
			assert(Natural(16, L"000000789FEDCBA"), "0789FEDCBA",       5, 35U, "#x0000000789FEDCBA");
		}

		TEST_METHOD(Decimal) {
			assert(Natural(10, "0000000890"), "037A",     2, 10U, "0000000890");
			assert(Natural(10, L"123456789"), "075BCD15", 4, 27U, "1234567890");
			assert(Natural(10, L"098765432"), "05E30A78", 4, 27U, "098765432");
		}

		TEST_METHOD(Octal) {
			assert(Natural(8, "00000567"),          "0177",         2, 9U,  "#o00000567");
			assert(Natural(8, L"0123456776543210"), "053977FAC688", 6, 43U, "#o0123456776543210");
			assert(Natural(8, L"7654321001234567"), "FAC688053977", 6, 48U, "#o7654321001234567");
		}
	};

	private class Subscription : public TestClass<Subscription> {
	public:
		TEST_METHOD(Byte) {
			Natural n(ch);
			int nsize = int(n.length());

			for (int idx = 0; idx < nsize; idx++) {
				Assert::AreEqual(ch[idx], n[idx], make_wstring(L"n[%d] = %x", idx, ch[idx])->Data());
				Assert::AreEqual(ch[idx], n[idx - nsize], make_wstring(L"n[%d] = %x", idx - nsize, ch[idx])->Data());
				Assert::AreEqual((uintptr_t*)(&n[idx]), (uintptr_t*)(&n[idx - nsize]), make_wstring(L"localtion of n[%d]", idx)->Data());
			}
		}

		TEST_METHOD(Modification) {
			Natural zero;
			Natural ff(0xFFFFFFFFFFFFFFFF);

			zero[-1] = 1U;
			ff[0xFF] = 1U;

			Assert::AreEqual("00", (const char*)zero.to_hexstring().c_str(),  L"capacity > payload");
			Assert::AreEqual("01FFFFFFFFFFFFFF", (const char*)ff.to_hexstring().c_str(),  L"capacity == playload");

			Assert::IsTrue(zero[0] == zero[-1],  L"zero[0] == zero[-1]");
			Assert::IsTrue(zero[-1] == zero[-2], L"zero[-1] == zero[-2]");
			Assert::IsFalse(zero[1] == 0,  L"zero[1] != 0");

			Assert::IsTrue(ff[0xFF] == ff[0],  L"ff[0xFF] == ff[0]");
			Assert::IsFalse(ff[0] == 0xFF,  L"ff[0] has been changed");
		}

		TEST_METHOD(Fixnum) {
			Natural xFECDBA0123456789(0xFECDBA0123456789U);
			Natural n(wch);
			Natural zero;
			int size16 = int(n.fixnum_count(Fixnum::Uint16));
			size_t idx16 = sizeof(wch) / sizeof(uint16) - size16;

			Assert::AreEqual((uint64)0U, zero.fixnum64_ref(0), L"0[0] = 0");
			Assert::AreEqual((uint64)0U, zero.fixnum64_ref(1), L"0[1] = 0");
			Assert::AreEqual((uint64)0U, zero.fixnum64_ref(-1), L"0[-1] = 0");

			for (int idx = 0; idx < size16; idx++) {
				const uint64 v = wch[idx + idx16];

				Assert::AreEqual(v, (const uint64)n.fixnum16_ref(idx), make_wstring(L"n16[%d] = %x", idx, v)->Data());
				Assert::AreEqual(v, (const uint64)n.fixnum16_ref(idx - size16), make_wstring(L"n16[%d] = %x", idx - size16, v)->Data());
			}

			Assert::AreEqual(0xFECDBA01ULL, (const uint64)xFECDBA0123456789.fixnum32_ref(0U), L"x64[0]");
			Assert::AreEqual(0xBA012345ULL, (const uint64)xFECDBA0123456789.fixnum32_ref(-2, 2), L"x64[-2o2]");
		}

		TEST_METHOD(Bytes) {
			Natural HW_ID("12345");
			bytes HW_BS = HW_ID.to_bytes();

			Assert::IsTrue(memcmp(HW_BS.c_str(), &HW_ID[0], HW_BS.size()) == 0,  L"Natural as Bytes");
		}
	};

	private class Arithmetic : public TestClass<Arithmetic> {
	public:
		TEST_METHOD(Increment) {
			test_increment(Natural(), "01", 1U);
			test_increment(Natural(0x09), "0A", 4U);
			test_increment(Natural(0xFF), "0100", 9U);
			test_increment(Natural(0xFF01234567890ABCU), "FF01234567890ABD", 64U);
			test_increment(Natural(0xFFFFFFFFFFFFFFFFU), "010000000000000000", 65U);
		}

	public:
		TEST_METHOD(Addition) {
			test_addition(Natural(0x1), 0x0U, "01", 1U);
			test_addition(Natural(0x2), 0x113198824U, "0113198826", 33U);
			test_addition(Natural(0x2718281828459045U), 0x3141592653589793U, "5859813E7B9E27D8", 63U);
			test_addition(Natural(0x6243299885435508U), 0x6601618158468695U, "C8448B19DD89DB9D", 64U);
			test_addition(Natural(0x7642236535892206U), 0x9159655941772190U, "01079B88BE77004396", 65U);
			test_addition(Natural(16, "161803398874989484820"), 0x35323U, "01618033988749894B9B43", 81U);
			test_addition(Natural(16, "3006050FB7A76AC18302FB593358"), 0x20539, "3006050FB7A76AC18302FB5B3891", 110U);

			test_addition(Natural(0x3), Natural(), "03", 2U);
			test_addition(Natural(wch), Natural(wch), "ECA8642002468ACF13579BDE", 96U);
			test_addition(Natural(ch), Natural(wch), "FECDBA98ECA8642002468ACF13579BDE", 128U);
			test_addition(Natural(ch), Natural(ch), "01FD9B7530ECA8642002468ACF13579BDE", 129U);
		}

		TEST_METHOD(Multiplicaton) {
			test_multiplication(Natural(0x1), 0x0U, "00", 0U);
			test_multiplication(Natural(0x2), 0x113198824U, "0226331048", 34U);
			test_multiplication(Natural(0x2718281828459045U), 0x3141592653589793U, "07859A6C0E1840504FE2128E1EC28A9F", 123U);
			test_multiplication(Natural(0x6243299885435508U), 0x6601618158468695U, "27274A43072E41D0558E8232CEE2ADA8", 126U);
			test_multiplication(Natural(0x7642236535892206U), 0x9159655941772190U, "4324C1DBF4B5587D2396DB4D214FE960", 127U);
			test_multiplication(Natural(16, "161803398874989484820"), 0x1U, "0161803398874989484820", 81U);

			test_multiplication(Natural(0x3), Natural(), "00", 0U);
			test_multiplication(Natural(914), Natural(1), "0392", 10);
			test_multiplication(Natural(914), Natural(84), "012BE8", 17);
			test_multiplication(Natural(wch), Natural(wch), "36B1B9D7A5578492EA6324B6A6F7108CDCA5E20890F2A521", 190U);
			test_multiplication(Natural(ch), Natural(wch), "75C6A1579D00FC474137A8FA8668109EA6F7108CDCA5E20890F2A521", 223U);
			test_multiplication(Natural(ch), Natural(ch), "FD9CE39AEAFE7CEF03503EB6DD17CD62226CFC86A6F7108CDCA5E20890F2A521", 256U);
			test_multiplication(Natural(16, "3006050FB7A76AC18302FB593358"), Natural(16, "20539"), "6106D98FDE64FEDF92AB31451B8D2698", 127U);
		}

	private:
		void test_increment(Natural& n, const char* representation, size_t bits) {
			n++;

			assert(n, representation, bits,
				make_wstring(L"#x%S++", n.to_hexstring().c_str()));
		}
		
		void test_addition(Natural& lhs, unsigned long long rhs, const char* representation, size_t bits) {
			assert(rhs + lhs, representation, bits,
				make_wstring(L"(+ #x%S #x%X)", lhs.to_hexstring().c_str(), rhs));
		}

		void test_addition(Natural& lhs, Natural& rhs, const char* representation, size_t bits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lhs_message = make_wstring(L"(LR+ #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rhs_message = make_wstring(L"(RL+ #x%S #x%S)", rhex.c_str(), lhex.c_str());
			
			assert(lhs + rhs, representation, bits, lhs_message);
			assert(rhs + lhs, representation, bits, rhs_message);
		}

		void test_multiplication(Natural& lhs, unsigned long long rhs, const char* representation, size_t bits) {
			assert(rhs * lhs, representation, bits,
				make_wstring(L"(* #x%S #x%X)", lhs.to_hexstring().c_str(), rhs));
		}

		void test_multiplication(Natural& lhs, Natural& rhs, const char* representation, size_t bits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lhs_message = make_wstring(L"(LR* #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rhs_message = make_wstring(L"(RL* #x%S #x%S)", rhex.c_str(), lhex.c_str());

			assert(lhs * rhs, representation, bits, lhs_message);
			assert(rhs * lhs, representation, bits, rhs_message);
		}
	};
}
