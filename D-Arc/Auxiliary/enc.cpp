#include "CppUnitTest.h"

#include "crypto/checksum.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/**************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary::Crypto {
	private class ENChart : public TestClass<ENChart> {
	public:
		TEST_METHOD(CRC32) {
			Assert::AreEqual("00000000", hexnumber(checksum_crc32(""), 8).c_str(), L"Empty Message");
			Assert::AreEqual("414FA339", hexnumber(checksum_crc32("The quick brown fox jumps over the lazy dog"), 8).c_str(), L"rosettacode");
			Assert::AreEqual(0x7E450C04UL, checksum_crc32("73871727080876A0"), L"S63 Data Protection Scheme");
		}
	};
}
