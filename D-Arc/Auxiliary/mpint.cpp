#include "CppUnitTest.h"

#include "datum/natural.hpp"
#include "datum/string.hpp"

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
			Natural DULL(0xDU);
			Natural copied_xDULL = DULL;

			assert(Natural(),                    "00",               0U, 0U,  "Empty Natural");
			assert(copied_xDULL,                 "0D",               1U, 4U,  "#xDU");
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
			test_increment(Natural(0x09U), "0A", 4U);
			test_increment(Natural(0xFFFFU), "010000", 17U);
			test_increment(Natural(0xFF01234567890ABCU), "FF01234567890ABD", 64U);
			test_increment(Natural(0xFFFFFFFFFFFFFFFFU), "010000000000000000", 65U);
		}

		TEST_METHOD(Decrement) {
			test_decrement(Natural(), "00", 0U);
			test_decrement(Natural(0x09U), "08", 4U);
			test_decrement(Natural(0x10000U), "FFFF", 16U);
			test_decrement(Natural(0xFF01234567890ABCU), "FF01234567890ABB", 64U);
		}

	public:
		TEST_METHOD(Addition) {
			test_addition(Natural(0x1), 0x0U, "01", 1U);
			test_addition(Natural(0xFFU), 0xFFFF01U, "01000000", 25U);
			test_addition(Natural(0x2718281828459045U), 0x3141592653589793U, "5859813E7B9E27D8", 63U);
			test_addition(Natural(0x6243299885435508U), 0x6601618158468695U, "C8448B19DD89DB9D", 64U);
			test_addition(Natural(0x7642236535892206U), 0x9159655941772190U, "01079B88BE77004396", 65U);
			test_addition(Natural(16, "161803398874989484820"), 0x35323U, "01618033988749894B9B43", 81U);
			test_addition(Natural(16, "3006050FB7A76AC18302FB593358"), 0x20539, "3006050FB7A76AC18302FB5B3891", 110U);

			test_addition(Natural(wch), Natural(wch), "ECA8642002468ACF13579BDE", 96U);
			test_addition(Natural(ch), Natural(wch), "FECDBA98ECA8642002468ACF13579BDE", 128U);
			test_addition(Natural(ch), Natural(ch), "01FD9B7530ECA8642002468ACF13579BDE", 129U);
		}

		TEST_METHOD(Subtraction) {
			test_subtraction(Natural(0x1), 0x0U, "01", 1U);
			test_subtraction(Natural(0x100U), 0xFFU, "01", 1U);
			test_subtraction(Natural(0x10000U), 0xFFU, "FF01", 16U);
			test_subtraction(Natural(0x2718281828459045U), 0x3141592653589793U, "0A29310E2B13074E", 60U);
			test_subtraction(Natural(0x6243299885435508U), 0x6601618158468695U, "03BE37E8D303318D", 58U);
			test_subtraction(Natural(0x7642236535892206U), 0x9159655941772190U, "1B1741F40BEDFF8A", 61U);
			test_subtraction(Natural(16, "161803398874989484820"), 0x35323U, "016180339887498944F4FD", 81U);
			test_subtraction(Natural(16, "3006050FB7A76AC18302FB593358"), 0x20539, "3006050FB7A76AC18302FB572E1F", 110U);

			test_subtraction(Natural(wch), Natural(wch), "00", 0U);
			test_subtraction(Natural(ch), Natural(wch), "FECDBA98000000000000000000000000", 128U);
		}

		TEST_METHOD(Multiplicaton) {
			test_multiplication(Natural(0x1), 0x0U, "00", 0U);
			test_multiplication(Natural(0x392), 0x54U, "012BE8", 17U);
			test_multiplication(Natural(0x128), 0x1131988242053954U, "13E158569C560A4920", 69U);
			test_multiplication(Natural(0x2718281828459045U), 0x3141592653589793U, "07859A6C0E1840504FE2128E1EC28A9F", 123U);
			test_multiplication(Natural(0x6243299885435508U), 0x6601618158468695U, "27274A43072E41D0558E8232CEE2ADA8", 126U);
			test_multiplication(Natural(0x7642236535892206U), 0x9159655941772190U, "4324C1DBF4B5587D2396DB4D214FE960", 127U);
			test_multiplication(Natural(16, "161803398874989484820"), 0x1U, "0161803398874989484820", 81U);

			test_multiplication(Natural(wch), Natural(wch), "36B1B9D7A5578492EA6324B6A6F7108CDCA5E20890F2A521", 190U);
			test_multiplication(Natural(ch), Natural(wch), "75C6A1579D00FC474137A8FA8668109EA6F7108CDCA5E20890F2A521", 223U);
			test_multiplication(Natural(ch), Natural(ch), "FD9CE39AEAFE7CEF03503EB6DD17CD62226CFC86A6F7108CDCA5E20890F2A521", 256U);
			test_multiplication(Natural(16, "3006050FB7A76AC18302FB593358"), Natural(16, "20539"), "6106D98FDE64FEDF92AB31451B8D2698", 127U);
		}

		TEST_METHOD(Division) {
			test_division(Natural(0x392), 0x54U, "0A", 4, "4A", 7);
			test_division(Natural(0x100U), 0xFFU, "01", 1, "01", 1);
			test_division(Natural(0x10000U), 0xFFU, "0101", 9, "01", 1);
			test_division(Natural(0x2718281828459045U), 0x3141592653589793U, "0EDEC916B5A3A7", 52, "3C", 6);
			test_division(Natural(0x6243299885435508U), 0x6601618158468695U, "01", 1, "0A29310E2B13074E", 60);
			test_division(Natural(0x7642236535892206U), 0x9159655941772190U, "01", 1, "03BE37E8D303318D", 58);
			test_division(Natural(16, "161803398874989484820"), 0x1U, "0161803398874989484820", 81, "00", 0);
			test_division(Natural(16, "3006050FB7A76AC18302FB593358"), 0x20539, "17C4F0C12B65E1D489FF22ED", 93, "01CB93", 17);

			test_division(Natural(wch), Natural(wch), "01", 1U, "00", 0U);
			test_division(Natural(ch), Natural(wch), "0227420275", 34, "5EE1FB4377777779C5ECD1B4", 95);
		}

		TEST_METHOD(Exponentiation) {
			test_exponentiation(Natural(0x1), 0x0U, "01", 1U);
			test_exponentiation(Natural(0x392), 0x54U, "04AF82A2B66B66F5A561E0DB8CAA8230CC792CB02D236D5892278142CE6816E694EE400DA34D979C2721FAB2462ED830DE678DD2BBD7908B5B77807101D3D9C22269FB6DF3C6CBE1872A931CF8AA1A35D31A79970E179A678FA0B679D21000000000000000000000", 827U);
		}

	private:
		void test_increment(Natural& n, const char* representation, size_t bits) {
			Platform::String^ message = make_wstring(L"#x%S++", n.to_hexstring().c_str());
			
			n++;
			assert(n, representation, bits, message);
		}

		void test_decrement(Natural& n, const char* representation, size_t bits) {
			Platform::String^ message = make_wstring(L"#x%S--", n.to_hexstring().c_str());

			n--;
			assert(n, representation, bits, message);
		}
		
		void test_addition(Natural& lhs, unsigned long long rhs, const char* representation, size_t bits) {
			assert(rhs + lhs, representation, bits, make_wstring(L"(+ #x%S #x%X)", lhs.to_hexstring().c_str(), rhs));
			this->test_addition(lhs, Natural(rhs), representation, bits);
		}

		void test_addition(Natural& lhs, Natural& rhs, const char* representation, size_t bits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lhs_message = make_wstring(L"(LR+ #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rhs_message = make_wstring(L"(RL+ #x%S #x%S)", rhex.c_str(), lhex.c_str());

			if (lhs == rhs) {
				assert(lhs + lhs, representation, bits, lhs_message);
			} else {
				assert(lhs + rhs, representation, bits, lhs_message);
				assert(rhs + lhs, representation, bits, rhs_message);
			}
		}

		void test_subtraction(Natural& lhs, Natural& rhs, const char* representation, size_t bits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lhs_message = make_wstring(L"(LR- #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rhs_message = make_wstring(L"(RL- #x%S #x%S)", rhex.c_str(), lhex.c_str());

			if (lhs == rhs) {
				assert(lhs - rhs, representation, bits, lhs_message);
			} else if (lhs > rhs) {
				assert(lhs - rhs, representation, bits, lhs_message);
				assert(rhs - lhs, "00", 0L, rhs_message);
			} else {
				assert(rhs - lhs, representation, bits, rhs_message);
				assert(lhs - rhs, "00", 0L, lhs_message);
			}
		}

		void test_subtraction(Natural& lhs, unsigned long long rhs, const char* representation, size_t bits) {
			if (lhs >= rhs) {
				assert(lhs - rhs, representation, bits, make_wstring(L"(LR- #x%S #x%X)", lhs.to_hexstring().c_str(), rhs));
				assert(rhs - lhs, "00", 0L, make_wstring(L"(RL- #x%X #x%S)", rhs, lhs.to_hexstring().c_str()));
				this->test_subtraction(lhs, Natural(rhs), representation, bits);
			} else {
				this->test_subtraction(Natural(rhs), lhs, representation, bits);
			}
		}

		void test_multiplication(Natural& lhs, unsigned long long rhs, const char* representation, size_t bits) {
			assert(rhs * lhs, representation, bits, make_wstring(L"(* #x%S #x%X)", lhs.to_hexstring().c_str(), rhs));
			this->test_multiplication(lhs, Natural(rhs), representation, bits);
		}

		void test_multiplication(Natural& lhs, Natural& rhs, const char* representation, size_t bits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lhs_message = make_wstring(L"(LR* #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rhs_message = make_wstring(L"(RL* #x%S #x%S)", rhex.c_str(), lhex.c_str());

			if (lhs == rhs) {
				assert(lhs * lhs, representation, bits, lhs_message);
			} else {
				assert(lhs * rhs, representation, bits, lhs_message);
				assert(rhs * lhs, representation, bits, rhs_message);
			}
		}

		void test_division(Natural& lhs, unsigned long long rhs, const char* quotient, size_t qbits, const char* remainder, size_t rbits) {
			this->test_division(lhs, Natural(rhs), quotient, qbits, remainder, rbits);
		}

		void test_division(Natural& lhs, Natural& rhs, const char* quotient, size_t qbits, const char* remainder, size_t rbits) {
			bytes lhex = lhs.to_hexstring();
			bytes rhex = rhs.to_hexstring();
			Platform::String^ lq_message = make_wstring(L"(LR/ #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ lr_message = make_wstring(L"(LR% #x%S #x%S)", lhex.c_str(), rhex.c_str());
			Platform::String^ rq_message = make_wstring(L"(RL/ #x%S #x%S)", rhex.c_str(), lhex.c_str());
			Platform::String^ rr_message = make_wstring(L"(RL% #x%S #x%S)", rhex.c_str(), lhex.c_str());

			if (lhs == rhs) {
				assert(lhs / rhs, quotient, qbits, lq_message);
				assert(lhs % rhs, remainder, rbits, lr_message);
			} else if (lhs > rhs) {
				assert(lhs / rhs, quotient, qbits, lq_message);
				assert(lhs % rhs, remainder, rbits, lr_message);
				assert(rhs / lhs, "00", 0L, rq_message);
			} else {
				assert(rhs / lhs, quotient, qbits, rq_message);
				assert(rhs % lhs, remainder, rbits, rr_message);
				assert(lhs / rhs, "00", 0L, lq_message);
			}
		}

		void test_exponentiation(Natural& base, unsigned long long e, const char* representation, size_t bits) {
			bytes lhex = base.to_hexstring();
			Platform::String^ fixnum_message = make_wstring(L"(fxexpt #x%S #x%x)", lhex.c_str(), e);
			Platform::String^ natural_message = make_wstring(L"(expt #x%S #x%x)", lhex.c_str(), e);

			assert(expt(base, e), representation, bits, fixnum_message);
			assert(expt(base, Natural(e)), representation, bits, natural_message);
		}
	};

	private class BitwiseOperation : public TestClass<BitwiseOperation> {
	public:
		TEST_METHOD(LeftShift) {
			uint64 ns[] = { 0x0ULL, 0x5ULL, 0x1010ULL, 0xFE110ULL, 0xEC0000412ULL };

			for (size_t idx = 0; idx < sizeof(ns) / sizeof(uint64); idx++) {
				for (uint64 i = 0; i < 9; i++) {
					Natural N(ns[idx] << i);
					Natural n(ns[idx]);

					n <<= i;
					assert(n, N, make_wstring(L"%x << %d", ns[idx], i));
				}
			}

			test_lshift(Natural(16, "6243299885435508"), 92, Natural(16, "0624329988543550800000000000000000000000"));
			test_lshift(Natural(16, "2718281828459045"), 10, Natural(16, "9C60A060A116411400"));
		}

		TEST_METHOD(RightShift) {
			uint64 ns[] = { 0x0ULL, 0x5ULL, 0x1010ULL, 0xFE110ULL, 0xEC0000412ULL };

			for (size_t idx = 0; idx < sizeof(ns) / sizeof(uint64); idx++) {
				for (uint64 i = 0; i < 9; i++) {
					Natural N(ns[idx] >> i);
					Natural n(ns[idx]);

					n >>= i;
					assert(n, N, make_wstring(L"%x >> %d", ns[idx], i));
				}
			}

			test_rshift(Natural(16, "2718281828459045"), 10, Natural(16, "09C60A060A1164"));
			test_rshift(Natural(16, "6243299885435508"), 45, Natural(16, "031219"));
			test_rshift(Natural(16, "765432100123456789ABCDEF"), 108, Natural(16, "00"));
			test_rshift(Natural(16, "FECDBA98765432100123456789ABCDEF"), 92, Natural(16, "0FECDBA987"));
		}

		TEST_METHOD(And) {
			test_bitwise_and(Natural(16, "ABC"), 0, Natural(16, "00"));
			test_bitwise_and(Natural(16, "ABCDEF"), 0xAB00FFFF00, Natural(16, "ABCD00"));
			test_bitwise_and(Natural(16, "90ABCDEF"), 0xCD00FFFF00, Natural(16, "ABCD00"));
			test_bitwise_and(Natural(16, "567890ABCDEF"), 0xEF00FFFF00, Natural(16, "6800ABCD00"));
		}

		TEST_METHOD(IOr) {
			test_bitwise_ior(Natural(16, "ABC"), 0, Natural(16, "0ABC"));
			test_bitwise_ior(Natural(16, "ABCDEF"), 0xAB00FFFF00, Natural(16, "AB00FFFFEF"));
			test_bitwise_ior(Natural(16, "90ABCDEF"), 0xCD00FFFF00, Natural(16, "CD90FFFFEF"));
			test_bitwise_ior(Natural(16, "567890ABCDEF"), 0xEF00FFFF00, Natural(16, "56FF90FFFFEF"));
		}

		TEST_METHOD(XOr) {
			test_bitwise_xor(Natural(16, "ABC"), 0, Natural(16, "0ABC"));
			test_bitwise_xor(Natural(16, "ABC"), 0xABC, Natural());
			test_bitwise_xor(Natural(16, "ABC"), 0xACD, Natural(16, "71"));
			test_bitwise_xor(Natural(16, "ABCDEF"), 0xAB00FFCDEF, Natural(16, "AB00540000"));
			test_bitwise_xor(Natural(16, "34567890ABCDEF"), 0xEF00FFFF00, Natural(16, "345697905432EF"));
		}

		TEST_METHOD(Bitset) {
			Natural x5F5 = Natural(0x5F5);

			Assert::IsTrue(x5F5.is_bit_set(0), L"(bitwise-bit-set? 0x5F5 0)");
			Assert::IsFalse(x5F5.is_bit_set(9), L"(bitwise-bit-set? 0x5F5 9)");
			Assert::IsTrue(x5F5.is_bit_set(10), L"(bitwise-bit-set? 0x5F5 10)");
			Assert::IsFalse(x5F5.is_bit_set(20), L"(bitwise-bit-set? 0x5F5 20)");
		}

		TEST_METHOD(Bitfield) {
			unsigned long long x = 0xFFABCDU;
			Natural FFABCD(x);

			assert(Natural(13).bit_field(1, 1), Natural(0U), "(bitwise-bit-field 13 1 1)");
			assert(Natural(13).bit_field(1, 3), Natural(2U), "(bitwise-bit-field 13 1 3)");
			assert(Natural(13).bit_field(1, 4), Natural(6U), "(bitwise-bit-field 13 1 4)");

			for (size_t idx = 0; idx < 33; idx++) {
				assert(FFABCD.bit_field(idx, 32), Natural((x >> idx) & ((1ULL << (32ULL - idx)) - 1ULL)),
					make_wstring(L"(bitwise-bit-field #x%S %d 32)", FFABCD.to_hexstring().c_str(), idx));
			}
		}

	private:
		void test_lshift(Natural& lhs, uint64 rhs, Natural& r) {
			bytes lhex = lhs.to_hexstring();
			Platform::String^ message = make_wstring(L"(<< #x%S %u)", lhex.c_str(), rhs);

			assert(lhs << rhs, r, message);
		}

		void test_rshift(Natural& lhs, uint64 rhs, Natural& r) {
			bytes rhex = lhs.to_hexstring();
			Platform::String^ message = make_wstring(L"(>> #x%S %u)", rhex.c_str(), rhs);

			assert(lhs >> rhs, r, message);
		}

		void test_bitwise_and(Natural& lhs, uint64 rhs, Natural& r) {
			Natural rn(rhs);
			bytes rhex = lhs.to_hexstring();
			Platform::String^ u_message = make_wstring(L"(and #x%S #x%08x)", rhex.c_str(), rhs);
			Platform::String^ n_message = make_wstring(L"(and #x%S #x%S)", rhex.c_str(), rn.to_hexstring().c_str());

			assert(rhs & lhs, r, u_message);
			assert(rn & lhs, r, n_message);
		}

		void test_bitwise_ior(Natural& lhs, uint64 rhs, Natural& r) {
			Natural rn(rhs);
			bytes rhex = lhs.to_hexstring();
			Platform::String^ u_message = make_wstring(L"(ior #x%S #x%08x)", rhex.c_str(), rhs);
			Platform::String^ n_message = make_wstring(L"(ior #x%S #x%S)", rhex.c_str(), rn.to_hexstring().c_str());

			assert(rhs | lhs, r, u_message);
			assert(rn | lhs, r, n_message);
		}

		void test_bitwise_xor(Natural& lhs, uint64 rhs, Natural& r) {
			Natural rn(rhs);
			bytes rhex = lhs.to_hexstring();
			Platform::String^ u_message = make_wstring(L"(xor #x%S #x%08x)", rhex.c_str(), rhs);
			Platform::String^ n_message = make_wstring(L"(xor #x%S #x%S)", rhex.c_str(), rn.to_hexstring().c_str());

			assert(rhs ^ lhs, r, u_message);
			assert(rn ^ lhs, r, n_message);
		}
	};

	private class Comparison : public TestClass<Comparison> {
	public:
		TEST_METHOD(Natural_vs_Natural) {
			unsigned long long median = 0x1010ULL;
			uint64 ns[] = { 0x0ULL, 0x5ULL, median, 0xFE110ULL, 0xEC0000412ULL };
			Natural Median(median);

			for (size_t idx = 0; idx < sizeof(ns) / sizeof(uint64); idx++) {
				this->test_comparison(ns[idx], median, Natural(ns[idx]), Median);
			}
		}

		TEST_METHOD(Natural_vs_Fixnum) {
			unsigned long long median = 0x1010ULL;
			uint64 ns[] = { 0x0ULL, 0x5ULL, median, 0xFE110ULL, 0xEC0000412ULL };

			for (size_t idx = 0; idx < sizeof(ns) / sizeof(uint64); idx++) {
				this->test_comparison(ns[idx], median, Natural(ns[idx]), median);
			}
		}

	private:
		template<typename M>
		void test_comparison(unsigned long long n, unsigned long long median, const Natural& N, M Median) {
			if (n < median) {
				Assert::IsTrue(N.compare(Median) < 0, make_wstring(L"[T]%x compare< %x", n, median)->Data());
				Assert::IsTrue(Median >= N, make_wstring(L"[T]%x operator<= %x", n, median)->Data());
				Assert::IsTrue(Median > N, make_wstring(L"[T]%x operator< %x", n, median)->Data());
				Assert::IsTrue(Median != N, make_wstring(L"[T]%x operator!= %x", n, median)->Data());
				Assert::IsFalse(N == Median, make_wstring(L"[F]%x operator== %x", n, median)->Data());
				Assert::IsFalse(N >= Median, make_wstring(L"[F]%x operator>= %x", n, median)->Data());
				Assert::IsFalse(N > Median, make_wstring(L"[F]%x operator> %x", n, median)->Data());
			} else if (n > median) {
				Assert::IsTrue(N.compare(Median) > 0, make_wstring(L"[T]%x compare> %x", n, median)->Data());
				Assert::IsTrue(Median <= N, make_wstring(L"[T]%x operator>= %x", n, median)->Data());
				Assert::IsTrue(Median < N, make_wstring(L"[T]%x operator> %x", n, median)->Data());
				Assert::IsTrue(Median != N, make_wstring(L"[T]%x operator!= %x", n, median)->Data());
				Assert::IsFalse(N == Median, make_wstring(L"[F]%x operator== %x", n, median)->Data());
				Assert::IsFalse(N <= Median, make_wstring(L"[F]%x operator<= %x", n, median)->Data());
				Assert::IsFalse(N < Median, make_wstring(L"[F]%x operator< %x", n, median)->Data());
			} else {
				Assert::IsTrue(N.compare(Median) == 0, make_wstring(L"[T]%x compare= %x", n, median)->Data());
				Assert::IsTrue(Median == N, make_wstring(L"[T]%x operator== %x", n, median)->Data());
				Assert::IsTrue(Median <= N, make_wstring(L"[T]%x operator<= %x", n, median)->Data());
				Assert::IsTrue(Median >= N, make_wstring(L"[T]%x operator>= %x", n, median)->Data());
				Assert::IsFalse(N != Median, make_wstring(L"[F]%x operator!= %x", n, median)->Data());
				Assert::IsFalse(N < Median, make_wstring(L"[F]%x operator< %x", n, median)->Data());
				Assert::IsFalse(N > Median, make_wstring(L"[F]%x operator> %x", n, median)->Data());
			}
		}
	};
}
