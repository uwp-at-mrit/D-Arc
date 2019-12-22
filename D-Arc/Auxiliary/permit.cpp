#include "CppUnitTest.h"

#include "crypto/enckey.hpp"
#include "crypto/checksum.hpp"
#include "crypto/blowfish.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/**************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary::Crypto {
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
		TEST_METHOD(CRC32) {
			Assert::AreEqual("00000000", (char*)hexnumber(checksum_crc32(""), 4).c_str(), L"Empty Message");
			Assert::AreEqual("414FA339", (char*)hexnumber(checksum_crc32("The quick brown fox jumps over the lazy dog"), 4).c_str(), L"rosettacode");
			Assert::AreEqual(0x7E450C04UL, checksum_crc32("73871727080876A0"), L"S63 Data Protection Scheme(P12)");
			Assert::AreEqual(780699093UL, checksum_crc32("NO4D061320000830BEB9BFE3C7C6CE68B16411FD09F96982"), L"S63 Data Protection Scheme(P50)");
		}

		TEST_METHOD(ASCII) {
			Assert::AreEqual("3130", (char*)enc_ascii(0x0100U, 2U).c_str(), L"M_ID ASCII");
			Assert::AreEqual("3132334142", (char*)enc_ascii(0x0102030A0BU).c_str(), L"M_KEY ASCII");
			
			test_natural_eq(0x0001ULL, enc_natural_from_ascii("3031", 2U), L"M_ID Hexadecimal");
			test_natural_eq(0x0102030A0BU, enc_natural_from_ascii("3132334142", 5U), L"M_KEY Hexadecimal");
			test_natural_eq(0x0102030408U, enc_natural_from_ascii("3132333438", 5U), L"HW_ID Hexadecimal");
		}

		TEST_METHOD(Hexadecimal) {
			test_natural_eq(0x0100ULL, enc_natural(0x10U), L"Literal ID -> M_ID");
			test_natural_eq(0x0102030405U, enc_natural("12345"), L"String -> HW_ID");
			test_natural_eq(0x0102030405U, enc_natural(0x12345), L"Literal ID -> HW_ID");
		}

		TEST_METHOD(Padding) {
			this->test_padding(0xC1CB518E9C030303ULL, 0xC1CB518E9CULL);
			this->test_padding(0x421571CC66030303ULL, 0x421571CC66ULL);
		}

	private:
		void test_padding(unsigned long long expected, unsigned long long actual) {
			test_natural_eq(expected, enc_natural_pad(actual), make_wstring(L"#x%llX", actual));
		}
	};
	
	private class ENCellPermit : public TestClass<ENCellPermit> {
	public:
		TEST_METHOD(HW_ID6) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);

			Assert::AreEqual(0x06ULL, hw_id6.length(), L"HW_ID6 length");
			test_natural_eq(0x010203040801U, hw_id6, L"HW_ID -> HW_ID6");
			Assert::AreEqual("313233343831", (char*)enc_ascii(hw_id6).c_str(), L"HW_ID6 ASCII");
		}

		TEST_METHOD(Encryption) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);
			BlowfishCipher bf(hw_id6.to_bytes().c_str(), hw_id6.length());
			
			this->test_encryption(&bf, 0xC1CB518E9CULL, 0xBEB9BFE3C7C6CE68ULL, "Encryption Cell Key 1");
			this->test_encryption(&bf, 0x421571CC66ULL, 0xB16411FD09F96982ULL, "Encryption Cell Key 2");
		}

		TEST_METHOD(Decryption) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);
			BlowfishCipher bf(hw_id6.to_bytes().c_str(), hw_id6.length());

			this->test_decryption(&bf, 0xBEB9BFE3C7C6CE68ULL, 0xC1CB518E9CULL, "Cell Key 1");
			this->test_decryption(&bf, 0xB16411FD09F96982ULL, 0x421571CC66ULL, "Cell Key 2");
		}

	private:
		void test_encryption(BlowfishCipher* bf, uint64 cell_key, uint64 expected, Platform::String^ message) {
			const uint8 keysize = 8U;
			uint8 cipher[keysize];
			uint64 eck = bf->encrypt(enc_natural_pad(cell_key).to_bytes().c_str(), 0, keysize, cipher, 0U, keysize);
			
			test_natural_eq(expected, Natural(cipher), message);
		}

		void test_decryption(BlowfishCipher* bf, uint64 cell_key, uint64 expected, Platform::String^ message) {
			const uint8 keysize = 8U;
			uint8 plain[keysize];
			uint64 eck = bf->decrypt(enc_natural_pad(cell_key).to_bytes().c_str(), 0, keysize, plain, 0U, keysize);

			test_natural_eq(expected, Natural(plain), message);
		}
	};
}
