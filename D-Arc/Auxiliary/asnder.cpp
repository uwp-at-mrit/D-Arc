#include "CppUnitTest.h"

#include "asn/der.hpp"

#include "datum/string.hpp"
#include "datum/flonum.hpp"
#include "datum/natural.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Auxiliary::ASN1 {
	static octets asn_utf_8_to_octets(std::wstring& str) {
		return asn_utf8_to_octets(str);
	}

	static size_t asn_utf_8_span(std::wstring& str) {
		return asn_utf8_span(str);
	}

	define_asn_enum(ASNOrder, order, zero, first, second, third, forth);

	/*********************************************************************************************/
	static void assert(octets& n, const char* control, Platform::String^ message) {
		Assert::AreEqual((const char*)(n.c_str()), control, message->Data());
	}

	/*********************************************************************************************/
	private class DERBase : public TestClass<DERBase> {
	public:
		TEST_METHOD(Length) {
			test_length(127, "\x7F");
			test_length(128, "\x81\x80");
			test_length(201, "\x81\xC9");
			test_length(435, "\x82\x01\xB3");
		}

	private:
		void test_length(size_t size, const char* control) {
			octets length = asn_length_to_octets(size);
			size_t offset = 0;

			assert(length, control, make_wstring(L"length %d", size));
			Assert::AreEqual(size, asn_octets_to_length(length, &offset), make_wstring(L"length[%d] size", size)->Data());
			Assert::AreEqual(length.size(), offset, make_wstring(L"length[%d] offset", size)->Data());
			Assert::AreEqual(asn_length_span(size), offset, make_wstring(L"length[%d] span", size)->Data());
		}
	};

	private class DERPrimitive : public TestClass<DERPrimitive> {
	public:
		TEST_METHOD(Fixnum) {
			const wchar_t* msgfmt = L"Integer[%ld]";

			test_primitive(0ll,    asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\x00",     msgfmt);
			test_primitive(+1ll,   asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\x01",     msgfmt);
			test_primitive(-1ll,   asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\xFF",     msgfmt);
			test_primitive(+127ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\x7F",     msgfmt);
			test_primitive(-127ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\x81",     msgfmt);
			test_primitive(+128ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x02\x00\x80", msgfmt); // NOTE the embedded null
			test_primitive(-128ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x01\x80",     msgfmt);
			test_primitive(+255ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x02\x00\xFF", msgfmt);
			test_primitive(+256ll, asn_fixnum_span, asn_fixnum_to_octets, asn_octets_to_fixnum, "\x02\x02\x01\x00", msgfmt);
		}

		TEST_METHOD(Natural) {
			test_natural("807fbc", "paded zero is an embedded null");
			test_natural("7fbc8ce9af7a9eb54c817fc7c1c796d1b1c80bddbcbacb15942480f5aa4ee120d27f93ebcf43275d01", "17^80");
		}

		TEST_METHOD(Real) {
			test_real(0.0,       "\x09\x00");
			test_real(+infinity, "\x09\x01\x40");
			test_real(-infinity, "\x09\x01\x41");
			test_real(flnan,     "\x09\x01\x42");
			test_real(-0.0,      "\x09\x01\x43");

			test_real(0.1, "\x09\x09\x80\xC9\x0C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(0.2, "\x09\x09\x80\xCA\x0C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(0.3, "\x09\x09\x80\xCA\x13\x33\x33\x33\x33\x33\x33");
			test_real(0.4, "\x09\x09\x80\xCB\x0C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(0.5, "\x09\x03\x80\xFF\x01");
			test_real(0.6, "\x09\x09\x80\xCB\x13\x33\x33\x33\x33\x33\x33");
			test_real(0.7, "\x09\x09\x80\xCC\x0B\x33\x33\x33\x33\x33\x33");
			test_real(0.8, "\x09\x09\x80\xCC\x0C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(0.9, "\x09\x09\x80\xCB\x1C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(1.0, "\x09\x03\x80\x00\x01");
			test_real(1.1, "\x09\x09\x80\xCD\x08\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(1.2, "\x09\x09\x80\xCC\x13\x33\x33\x33\x33\x33\x33");
			test_real(1.3, "\x09\x09\x80\xCC\x14\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(1.4, "\x09\x09\x80\xCD\x0B\x33\x33\x33\x33\x33\x33");
			test_real(1.5, "\x09\x03\x80\xFF\x03");
			test_real(1.6, "\x09\x09\x80\xCD\x0C\xCC\xCC\xCC\xCC\xCC\xCD");

			test_real(2.718281828459045,  "\x09\x09\x80\xCD\x15\xBF\x0A\x8B\x14\x57\x69");
			test_real(3.141592653589793,  "\x09\x09\x80\xD0\x03\x24\x3F\x6A\x88\x85\xA3");
			test_real(0.5772156649015329, "\x09\x09\x80\xCB\x12\x78\x8C\xFC\x6F\xB6\x19");
			test_real(1.618033988749895,  "\x09\x09\x80\xCF\x03\x3C\x6E\xF3\x72\xFE\x95");
			test_real(0.915965594177219,  "\x09\x09\x80\xCB\x1D\x4F\x97\x13\xE8\x13\x5D");

			test_real(-0.0015625, "\x09\x09\xC0\xC3\x0C\xCC\xCC\xCC\xCC\xCC\xCD");
			test_real(-15.625, "\x09\x03\xC0\xFD\x7D");
		}

		TEST_METHOD(Enumerated) {
			const wchar_t* msgfmt = L"Enumerated Order [%s]";

			test_enum(ASNOrder::zero,  asn_order_to_octets, asn_octets_to_order, "\x0A\x01\x00", msgfmt);
			test_enum(ASNOrder::forth, asn_order_to_octets, asn_octets_to_order, "\x0A\x01\x04", msgfmt);
		}

		TEST_METHOD(String) {
			test_string(std::string("6.0.5361.2"), asn_ia5_span, asn_ia5_to_octets, asn_octets_to_ia5,
				"\x16\x0A\x36\x2E\x30\x2E\x35\x33\x36\x31\x2E\x32", L"String IA5[%S]");

			// WARNING: std::string can contain embedded '\0',  but be careful when making it with char-array literal.
			test_string(std::wstring(L"λsh\x0\nssh", 8), asn_utf_8_span, asn_utf_8_to_octets, asn_octets_to_utf8,
				"\x0C\x09\xCE\xBB\x73\x68\x00\x0A\x73\x73\x68", L"String UTF8[%s]");
		}

		TEST_METHOD(Miscellaneous) {
			test_primitive(true,  asn_boolean_span, asn_boolean_to_octets, asn_octets_to_boolean, "\x01\x01\xFF", L"Boolean %d");
			test_primitive(false, asn_boolean_span, asn_boolean_to_octets, asn_octets_to_boolean, "\x01\x01\x00", L"Boolean %d");
		}

	private:
		template<typename T, typename Span, typename T2O, typename O2T> 
		void test_primitive(T datum, Span span, T2O asn_to_octets, O2T octets_to_asn, const char* representation, const wchar_t* msgfmt) {
			Platform::String^ message = make_wstring(msgfmt, datum);
			octets basn = asn_to_octets(datum);

			Assert::AreEqual(representation, (const char*)(basn.c_str()), message->Data());
			Assert::AreEqual(basn.size(), asn_span(span, datum), message->Data());

			{ // decode
				size_t offset = 0;
				T restored = octets_to_asn(basn, &offset);
				
				Assert::AreEqual(datum, restored, message->Data());
				Assert::AreEqual(basn.size(), offset, message->Data());
			}
		}

		void test_natural(Platform::String^ representation, const char* readable_name) {
			Platform::String^ message = make_wstring(L"Natural[%S]", readable_name);
			::Natural nat((uint8)16U, (const uint16*)representation->Data(), 0, representation->Length());
			octets bnat = asn_natural_to_octets(nat);
			::Natural restored = asn_octets_to_natural(bnat);
			
			Assert::IsTrue(nat == restored, message->Data());
			Assert::AreEqual(bnat.size(), asn_span(asn_natural_span, nat), message->Data());

			asn_natural_into_octets(nat, (uint8*)bnat.c_str(), 0);
			Assert::IsTrue(nat == asn_octets_to_natural(bnat), message->Data());
		}

		void test_real(double real, const char* representation) {
			Platform::String^ message = make_wstring(L"Real[%lf]", real);
			octets breal = asn_real_to_octets(real);

			Assert::AreEqual(representation, (const char*)(breal.c_str()), message->Data());
			Assert::AreEqual(breal.size(), asn_span(asn_real_span, real), message->Data());

			asn_real_into_octets(real, (uint8*)breal.c_str(), 0);
			Assert::AreEqual(representation, (const char*)(breal.c_str()), message->Data());

			{ // decode
				size_t offset = 0;
				double restored = asn_octets_to_real(breal, &offset);

				if (flisnan(real)) {
					Assert::IsTrue(flisnan(restored), message->Data());
				} else if (real == 0.0) {
					Assert::AreEqual(flsign(real), flsign(restored), message->Data());
				} else {
					Assert::AreEqual(real, restored, message->Data());
				}
			}
		}

		template<typename T, typename T2O, typename O2T>
		void test_enum(T datum, T2O asn_to_octets, O2T octets_to_asn, const char* representation, const wchar_t* msgfmt) {
			Platform::String^ message = make_wstring(msgfmt, datum.ToString()->Data());
			octets basn = asn_to_octets(datum);

			Assert::AreEqual(representation, (const char*)(basn.c_str()), message->Data());

			{ // decode
				T restored = octets_to_asn(basn, nullptr);

				Assert::IsTrue(datum == restored, message->Data());
			}
		}

		template<typename T, typename Span, typename T2O, typename O2T> 
		void test_string(T& datum, Span span, T2O asn_to_octets, O2T octets_to_asn, const char* representation, const wchar_t* msgfmt) {
			Platform::String^ message = make_wstring(msgfmt, datum.c_str());
			octets basn = asn_to_octets(datum);

			Assert::AreEqual(representation, (const char*)basn.c_str(), message->Data());
			Assert::AreEqual(basn.size(), asn_span(span, datum), message->Data());

			{ // decode
				T restored = octets_to_asn(basn, nullptr);

				Assert::AreEqual(datum.c_str(), restored.c_str(), message->Data());
			}
		}
	};
}
