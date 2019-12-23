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

			{ // Accumulated CRC32
				unsigned long acc_crc = 0;
				
				checksum_crc32(&acc_crc, "NO4D0613");
				checksum_crc32(&acc_crc, "20000830");
				
				Assert::AreEqual(780699093UL, checksum_crc32(acc_crc, "BEB9BFE3C7C6CE68B16411FD09F96982"), L"Accumulated CRC32");
			}
		}

		TEST_METHOD(ASCII) {
			Assert::AreEqual("3130", (char*)enc_ascii(0x10U).c_str(), L"M_ID ASCII");
			Assert::AreEqual("3132334142", (char*)enc_ascii(0x123ABU).c_str(), L"M_KEY ASCII");

			test_natural_eq(0x3031ULL, enc_natural_from_ascii("3031", 2U), L"M_ID Hexadecimal");
			test_natural_eq(0x3132334142U, enc_natural_from_ascii("3132334142", 5U), L"M_KEY Hexadecimal");
			test_natural_eq(0x3132333438U, enc_natural_from_ascii("3132333438", 5U), L"HW_ID Hexadecimal");
		}

		TEST_METHOD(Hexadecimal) {
			test_natural_eq(0x3130ULL, enc_natural(0x10U), L"Literal ID -> M_ID");
			test_natural_eq(0x3132333435U, enc_natural("12345", 5U), L"String -> HW_ID");
			test_natural_eq(0x3132333435U, enc_natural(0x12345), L"Literal ID -> HW_ID");
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
		TEST_METHOD(HW_ID6) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);

			test_natural_eq(enc_natural("12348", 5U), hw_id, "HW_ID representation");
			test_natural_eq(0x313233343831U, hw_id6, L"HW_ID -> HW_ID6");
			Assert::AreEqual("313233343831", (const char*)enc_ascii(hw_id6).c_str(), L"HW_ID6 ASCII");
		}

		TEST_METHOD(Encryption) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			
			test_natural_eq(0xBEB9BFE3C7C6CE68ULL, enc_cell_permit_encrypted_key(hw_id, 0xC1CB518E9CULL), "Encrypted Cell Key 1");
			test_natural_eq(0xB16411FD09F96982ULL, enc_cell_permit_encrypted_key(hw_id, 0x421571CC66ULL), "Encrypted Cell Key 2");
		}

		TEST_METHOD(Checksum) {
			const uint8 keysize = 8U;
			uint8 cipher[keysize];
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);
			BlowfishCipher bf(hw_id6.to_bytes().c_str(), hw_id6.length());
			Natural CRC(780699093ULL);
			uint64 ecrc = bf.encrypt(enc_natural_pad(CRC).to_bytes().c_str(), 0, keysize, cipher, 0U, keysize);

			test_natural_eq(0x795C77B204F54D48ULL, Natural(cipher), "Encrypted CRC32");
		}

		TEST_METHOD(Decryption) {
			Natural hw_id = enc_natural_from_ascii("3132333438", 5U);
			Natural hw_id6 = enc_hardware_uid6(hw_id);
			BlowfishCipher bf(hw_id6.to_bytes().c_str(), hw_id6.length());

			this->test_key_decryption(&bf, 0xBEB9BFE3C7C6CE68ULL, 0xC1CB518E9CULL, "Cell Key 1");
			this->test_key_decryption(&bf, 0xB16411FD09F96982ULL, 0x421571CC66ULL, "Cell Key 2");
		}

	private:
		void test_key_decryption(BlowfishCipher* bf, uint64 cell_key, uint64 expected, Platform::String^ message) {
			const uint8 keysize = 8U;
			uint8 plain[keysize];

			bf->decrypt(enc_natural_pad(cell_key).to_bytes().c_str(), 0, keysize, plain, 0U, keysize);
			test_natural_eq(expected, enc_natural_unpad(Natural(plain)), message);
		}
	};
}
