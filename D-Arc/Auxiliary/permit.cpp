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
			Assert::AreEqual(0x0001ULL, enc_hexadecimal_from_ascii("3031", 2U), L"M_ID Hexadecimal");
			Assert::AreEqual(0x0102030A0BU, enc_hexadecimal_from_ascii("3132334142", 5U), L"M_KEY Hexadecimal");
			Assert::AreEqual(0x0102030408U, enc_hexadecimal_from_ascii("3132333438", 5U), L"HW_ID Hexadecimal");
		}

		TEST_METHOD(Hexadecimal) {
			Assert::AreEqual(0x0100ULL, enc_hexadecimal(0x10U), L"Literal ID -> M_ID");
			Assert::AreEqual(0x0102030405U, enc_hexadecimal("12345"), L"String -> HW_ID");
			Assert::AreEqual(0x0102030405U, enc_hexadecimal(0x12345), L"Literal ID -> HW_ID");
		}

		TEST_METHOD(Padding) {
			Assert::AreEqual(0xC1CB518E9C030303ULL, enc_hexadecimal_pad(0xC1CB518E9CULL), L"0xC1CB518E9C");
			Assert::AreEqual(0x421571CC66030303ULL, enc_hexadecimal_pad(0x421571CC66ULL), L"0x421571CC66");
			test_bytes(hexnumber(0xC1CB518E9C666666ULL), enc_pad(hexnumber(0xC1CB518E9CULL)), L"C1CB518E9C");
			test_bytes(hexnumber(0x421571CC66666666ULL), enc_pad(hexnumber(0x421571CC66ULL)), L"421571CC66");
		}
	};
	
	private class ENCellPermit : public TestClass<ENCellPermit> {
	public:
		TEST_METHOD(HW_ID6) {
			uint64 hw_id = enc_hexadecimal_from_ascii("3132333438", 5U);
			uint64 hw_id6 = enc_hardware_uid6(hw_id);
			const char hw_bytes[] = { 0x01U, 0x02U, 0x03U, 0x04U, 0x08U, 0x01U, 0x0U };

			Assert::AreEqual(0x010203040501U, enc_hardware_uid6(0x0102030405U), L"HW_ID -> HW_ID6");
			Assert::AreEqual("313233343831", (char*)enc_ascii(hw_id6, 6U).c_str(), L"HW_ID6 ASCII");
			Assert::AreEqual(hw_bytes, (char*)enc_natural_bytes(hw_id6, 6U).c_str(), L"HW_ID6 Bytes");
		}

		TEST_METHOD(Encryption) {
			uint64 hw_id = enc_hexadecimal_from_ascii("3132333438", 5U);
			uint64 hw_id6 = enc_hardware_uid6(hw_id);
			BlowfishCipher bf(enc_natural_bytes(hw_id6, 6U));
			
			this->test_encryption(&bf, 0xC1CB518E9CULL, 0xBEB9BFE3C7C6CE68ULL, "Encryption Cell Key 1");
			this->test_encryption(&bf, 0x421571CC66ULL, 0xB16411FD09F96982ULL, "Encryption Cell Key 2");
		}

	private:
		void test_encryption(BlowfishCipher* bf, uint64 cell_key, uint64 expected, Platform::String^ message) {
			uint64 eck = bf->encrypt(enc_hexadecimal_pad(cell_key));
			
			Assert::AreEqual((const char*)hexnumber(expected).c_str(), (const char*)hexnumber(eck).c_str(), message->Data());
		}
	};
}
