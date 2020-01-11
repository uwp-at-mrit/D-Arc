#include "CppUnitTest.h"

#include "datum/string.hpp"
#include "asn/der.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary::ASN1 {
	static void assert(octets& n, const char* control, Platform::String^ message) {
		Assert::AreEqual((const char*)(n.c_str()), control, message->Data());
	}
	
	private class DERBase : public TestClass<DERBase> {
	public:
		TEST_METHOD(Length) {
			assert(asn_length_to_octets(127), "\x7F", "length 127");
			assert(asn_length_to_octets(128), "\x81\x80", "length 128");
			assert(asn_length_to_octets(201), "\x81\xC9", "length 201");
			assert(asn_length_to_octets(435), "\x82\x01\xB3", "length 435");
		}

	private:
		void test_length(size_t size, const char* control) {
			octets length = asn_length_to_octets(size);
			size_t offset = 0;

			assert(length, control, make_wstring(L"length %d", size));
			Assert::AreEqual(size, asn_octets_to_length(length, &offset), make_wstring(L"length[%d] size", size)->Data());
			Assert::AreEqual(length.size(), offset, make_wstring(L"length[%d] offset", size)->Data());
		}
	};
}
