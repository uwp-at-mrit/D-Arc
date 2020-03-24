#include "CppUnitTest.h"

#include "crypto/enckey.hpp"
#include "crypto/blowfish.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;
using namespace WarGrey::DTPM;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/**************************************************************************************************/
namespace WarGrey::Tamer::ENChart::Crypto {
	static void test_bytes(bytes& expected, bytes& actual, Platform::String^ message) {
		Assert::AreEqual((const char*)expected.c_str(), (const char*)actual.c_str(), message->Data());
	}

	static void test_natural_eq(Natural& expected, Natural& actual, Platform::String^ message) {
		bytes exp_desc = expected.to_hexstring();
		bytes act_desc = actual.to_hexstring();
		Platform::String^ msg = make_wstring(L"%s [expected: %S, actual: %S]",
			message->Data(), exp_desc.c_str(), act_desc.c_str());

		Assert::IsTrue(actual == expected, msg->Data());
	}

	static void test_natural_eq(unsigned long long expected, Natural& actual, Platform::String^ message) {
		test_natural_eq(Natural(expected), actual, message);
	}

	private class ENCPrimitive : public TestClass<ENCPrimitive> {
	public:
		TEST_METHOD(ASCII) {
			Assert::AreEqual("3130", (char*)enc_ascii(0x10U).c_str(), L"M_ID ASCII");
			Assert::AreEqual("3132334142", (char*)enc_ascii(0x123ABU).c_str(), L"M_KEY ASCII");
			Assert::AreEqual("3230303030383330", (char*)enc_ascii(0x20000830U).c_str(), L"CELL PERMIT DATE");

			test_natural_eq(0x3031ULL, enc_natural_from_ascii("3031", 2U), L"M_ID Hexadecimal");
			test_natural_eq(0x3132334142U, enc_natural_from_ascii("3132334142", 5U), L"M_KEY Hexadecimal");
			test_natural_eq(0x3132333438U, enc_natural_from_ascii("3132333438", 5U), L"HW_ID Hexadecimal");
		}

		TEST_METHOD(Hexadecimal) {
			test_natural_eq(0x3130ULL, enc_natural(0x10U), L"Literal ID -> M_ID");
			test_natural_eq(0x3132333435U, enc_natural("12345", 5U), L"String -> HW_ID");
			test_natural_eq(0x3230303030383330U, enc_natural("20000830", 8U), L"String -> DATE");
			test_natural_eq(0x3132333435U, enc_natural(0x12345U), L"Literal ID -> HW_ID");
		}

		TEST_METHOD(Padding) {
			this->test_padding(0xC1CB518E9C030303ULL, 0xC1CB518E9CULL);
			this->test_padding(0x421571CC66030303ULL, 0x421571CC66ULL);
			this->test_padding(0x0780699093030303ULL, 0x0780699093ULL);
		}

	private:
		void test_padding(unsigned long long expected, unsigned long long actual) {
			test_natural_eq(expected, enc_natural_pad(actual), make_wstring(L"#x%llX", actual));
		}
	};
	
	private class ENCellPermit : public TestClass<ENCellPermit> {
	public:
		ENCellPermit() {
			this->HW_ID = enc_natural_from_ascii("3132333438", 5U);
		}

	public:
		TEST_METHOD(HW_ID6) {
			Natural hw_id6 = enc_hardware_uid6(this->HW_ID);

			test_natural_eq(enc_natural("12348", 5U), this->HW_ID, "HW_ID representation");
			test_natural_eq(0x313233343831U, hw_id6, L"HW_ID -> HW_ID6");
			Assert::AreEqual("313233343831", (const char*)enc_ascii(hw_id6).c_str(), L"HW_ID6 ASCII");
		}

		TEST_METHOD(Encryption) {
			test_natural_eq(0xBEB9BFE3C7C6CE68ULL, enc_cell_permit_encrypt(this->HW_ID, 0xC1CB518E9CULL), "Encrypted Cell Key 1");
			test_natural_eq(0xB16411FD09F96982ULL, enc_cell_permit_encrypt(this->HW_ID, 0x421571CC66ULL), "Encrypted Cell Key 2");
		}

		TEST_METHOD(Checksum) {
			Natural eck1 = enc_cell_permit_encrypt(this->HW_ID, 0xC1CB518E9CULL);
			Natural eck2 = enc_cell_permit_encrypt(this->HW_ID, 0x421571CC66ULL);
			Natural checksum = enc_cell_permit_checksum("NO4D0613", 8U, 2000U, 8U, 30U, eck1, eck2);

			test_natural_eq(780699093UL, checksum, L"Raw CRC32");
			test_natural_eq(0x795C77B204F54D48ULL, enc_cell_permit_encrypt(this->HW_ID, checksum), "Encrypted CRC32");
		}

		TEST_METHOD(Decryption) {
			test_natural_eq(0xC1CB518E9CULL, enc_cell_permit_decrypt(this->HW_ID, 0xBEB9BFE3C7C6CE68ULL), "Cell Key 1");
			test_natural_eq(0x421571CC66ULL, enc_cell_permit_decrypt(this->HW_ID, 0xB16411FD09F96982ULL), "Cell Key 2");
		}

	private:
		Natural HW_ID;
	};
}
