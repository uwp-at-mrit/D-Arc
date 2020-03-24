#include "CppUnitTest.h"

#include "checksum/ipv4.hpp"
#include "checksum/crc32.hpp"

#include "datum/string.hpp"

#include "syslog.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Jargon::Checksum {
	private class IPv4 : public TestClass<IPv4> {
	public:
		TEST_METHOD(Break) {
			uint8 vector[] = { 0x00, 0x01, 0xf2, 0x03, 0xf4, 0xf5, 0xf6, 0xf7 };
			size_t count = sizeof(vector) / sizeof(uint8);
			unsigned short checksum = checksum_ipv4(vector, 0, count);

			for (size_t bidx = 0; bidx < count; bidx++) {
				Platform::String^ message = make_wstring(L"break@%d", bidx);
				unsigned short acc_sum = 0;

				if ((bidx & 0x01) == 1) {
					checksum_ipv4(&acc_sum, vector, bidx, count);
					acc_sum = ((acc_sum & 0xFF) << 8) ^ (acc_sum >> 8); // swap 2nd part checksum
					checksum_ipv4(&acc_sum, vector, 0, bidx);
				} else {
					checksum_ipv4(&acc_sum, vector, 0, bidx);
					checksum_ipv4(&acc_sum, vector, bidx, count);
				}

				Assert::AreEqual(int(checksum), int(acc_sum), message->Data());
			}
		}

		TEST_METHOD(Head) {
			uint8 header[] = {
				0x45, 0x00, 0x00, 0x73, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11,
				0x00, 0x00, 0xc0, 0xa8, 0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7,
				0x00 /* termination */
			};
			size_t checksum_idx = sizeof(header) / sizeof(uint8) / 2;
			unsigned short sum = 0U;

			checksum_ipv4(&sum, header, 0, checksum_idx);
			checksum_ipv4(&sum, header, checksum_idx + 2, checksum_idx * 2);
			
			header[checksum_idx + 0] = sum >> 8U;
			header[checksum_idx + 1] = sum & 0xFFU;

			Assert::AreEqual(0x0, int(checksum_ipv4(header)), L"verify the header's checksum");
		}
	};

	private class CRC32 : public TestClass<CRC32> {
	public:
		TEST_METHOD(Vector) {
			Assert::AreEqual("00000000", (char*)hexnumber(checksum_crc32(""), 4).c_str(), L"Empty Message");
			Assert::AreEqual("414FA339", (char*)hexnumber(checksum_crc32("The quick brown fox jumps over the lazy dog"), 4).c_str(), L"rosettacode");
			Assert::AreEqual(0x7E450C04UL, checksum_crc32("73871727080876A0"), L"S63 Data Protection Scheme(P12)");
			Assert::AreEqual(780699093UL, checksum_crc32("NO4D061320000830BEB9BFE3C7C6CE68B16411FD09F96982"), L"S63 Data Protection Scheme(P50)");
		}

		TEST_METHOD(Accumulated) {
			unsigned long acc_crc = 0;

			checksum_crc32(&acc_crc, "NO4D0613");
			checksum_crc32(&acc_crc, "20000830");

			Assert::AreEqual(780699093UL, checksum_crc32(acc_crc, "BEB9BFE3C7C6CE68B16411FD09F96982"), L"Accumulated CRC32");
		}
	};
}
