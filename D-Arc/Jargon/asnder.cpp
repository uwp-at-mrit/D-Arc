#include "CppUnitTest.h"

#include "asn/der.hpp"

#include "datum/string.hpp"
#include "datum/flonum.hpp"
#include "datum/natural.hpp"

#include "syslog.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Jargon::ASN1 {
	define_asn_enum(log, Log, Debug, Info, Notice, Warning, Error, Critical, Alarm, Panic, _ );

	static size_t asn_utf_8_span(std::wstring& str) {
		return asn_utf8_span(str);
	}

	static octets asn_utf_8_to_octets(std::wstring& str) {
		return asn_utf8_to_octets(str);
	}

	static std::wstring asn_bytes_to_utf8(octets& bint, size_t* offset = nullptr) {
		return asn_octets_to_utf8(bint, offset);
	}

	static long long asn_bytes_to_fixnum(octets& bint, size_t* offset = nullptr) {
		return asn_octets_to_fixnum(bint, offset);
	}

	static bool asn_bytes_to_boolean(octets& bint, size_t* offset = nullptr) {
		return asn_octets_to_boolean(bint, offset);
	}

	static std::string asn_bytes_to_ia5(octets& bint, size_t* offset = nullptr) {
		return asn_octets_to_ia5(bint, offset);
	}

	static Log asn_bytes_to_log(octets& bint, size_t* offset = nullptr) {
		return asn_octets_to_log(bint, offset);
	}

	private struct LogMessage : public IASNSequence {
	public:
		LogMessage(Log level, Platform::String^ message, long long timestamp, std::string topic)
			: IASNSequence(4), level(level), message(message), timestamp(timestamp), topic(topic) {}

		LogMessage(octets& basn, size_t* offset = nullptr) : IASNSequence(4) {
			this->from_octets(basn, offset);
		}

	public:
		Log level;
		Platform::String^ message;
		long long timestamp;
		std::string topic;

	protected:
		size_t field_payload_span(size_t idx) override {
			size_t span = 0;

			switch (idx) {
			case 0: span = asn_log_span(this->level); break;
			case 1: span = asn_utf8_span(this->message); break;
			case 2: span = asn_fixnum_span(this->timestamp); break;
			case 3: span = asn_ia5_span(this->topic); break;
			}

			return span;
		}

		size_t fill_field(size_t idx, uint8* octets, size_t offset) override {
			switch (idx) {
			case 0: offset = asn_log_into_octets(this->level, octets, offset); break;
			case 1: offset = asn_utf8_into_octets(this->message, octets, offset); break;
			case 2: offset = asn_fixnum_into_octets(this->timestamp, octets, offset); break;
			case 3: offset = asn_ia5_into_octets(this->topic, octets, offset); break;
			}

			return offset;
		}

		void extract_field(size_t idx, const uint8* basn, size_t* offset) override {
			switch (idx) {
			case 0: this->level = asn_octets_to_log(basn, offset); break;
			case 1: this->message = make_wstring(asn_octets_to_utf8(basn, offset)); break;
			case 2: this->timestamp = asn_octets_to_fixnum(basn, offset); break;
			case 3: this->topic = asn_octets_to_ia5(basn, offset); break;
			}
		}
	};

	/*********************************************************************************************/
	static bool bytes_eq(const char* b10, const uint8* b2, size_t size, Platform::String^ message) {
		uint8* b1 = (uint8*)b10;
		bool eq = true;

		for (size_t i = 0; i < size; i++) {
			if (b1[i] != b2[i]) {
				eq = false;
				break;
			}
		}

		if (!eq) {
			syslog(::Log::Error, L"BytesEq: %s:", message->Data());

			for (size_t i = 0; i < size; i++) {
				syslog(::Log::Error, L"  [%02d] exptected: %02X, given: %02X", i, b1[i], b2[i]);
			}
		}

		return eq;
	}

	static void assert(octets& n, const char* control, Platform::String^ message) {
		Assert::IsTrue(bytes_eq(control, n.c_str(), n.size(), message), message->Data());
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

			test_primitive(0ll,    asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\x00",     msgfmt);
			test_primitive(+1ll,   asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\x01",     msgfmt);
			test_primitive(-1ll,   asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\xFF",     msgfmt);
			test_primitive(+127ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\x7F",     msgfmt);
			test_primitive(-127ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\x81",     msgfmt);
			test_primitive(+128ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x02\x00\x80", msgfmt); // NOTE the embedded null
			test_primitive(-128ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x01\x80",     msgfmt);
			test_primitive(+255ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x02\x00\xFF", msgfmt);
			test_primitive(+256ll, asn_fixnum_span, asn_fixnum_to_octets, asn_bytes_to_fixnum, "\x02\x02\x01\x00", msgfmt);
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
			test_real(180.0, "\x09\x04\x80\x00\x00\xB4");
		}

		TEST_METHOD(Enumerated) {
			const wchar_t* msgfmt = L"Enumerated Log [%s]";

			test_enum(Log::Debug,  asn_log_to_octets, asn_bytes_to_log, "\x0A\x01\x00", msgfmt);
			test_enum(Log::Error, asn_log_to_octets, asn_bytes_to_log, "\x0A\x01\x04", msgfmt);
		}

		TEST_METHOD(String) {
			test_string(std::string("6.0.5361.2"), asn_ia5_span, asn_ia5_to_octets, asn_bytes_to_ia5,
				"\x16\x0A\x36\x2E\x30\x2E\x35\x33\x36\x31\x2E\x32", L"String IA5[%S]");

			// WARNING: std::string can contain embedded '\0',  but be careful when making it with char-array literal.
			test_string(std::wstring(L"λsh\x0\nssh", 8), asn_utf_8_span, asn_utf_8_to_octets, asn_bytes_to_utf8,
				"\x0C\x09\xCE\xBB\x73\x68\x00\x0A\x73\x73\x68", L"String UTF8[%s]");
		}

		TEST_METHOD(Miscellaneous) {
			test_primitive(true,  asn_boolean_span, asn_boolean_to_octets, asn_bytes_to_boolean, "\x01\x01\xFF", L"Boolean %d");
			test_primitive(false, asn_boolean_span, asn_boolean_to_octets, asn_bytes_to_boolean, "\x01\x01\x00", L"Boolean %d");
		}

	private:
		template<typename T, typename Span, typename T2O, typename O2T> 
		void test_primitive(T datum, Span span, T2O asn_to_octets, O2T octets_to_asn, const char* representation, const wchar_t* msgfmt) {
			Platform::String^ message = make_wstring(msgfmt, datum);
			octets basn = asn_to_octets(datum);

			Assert::IsTrue(bytes_eq(representation, basn.c_str(), basn.size(), message), message->Data());
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

			Assert::IsTrue(bytes_eq(representation, breal.c_str(), breal.size(), message), message->Data());
			Assert::AreEqual(breal.size(), asn_span(asn_real_span, real), message->Data());

			asn_real_into_octets(real, (uint8*)breal.c_str(), 0);
			Assert::IsTrue(bytes_eq(representation, breal.c_str(), breal.size(), message), message->Data());

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

			Assert::IsTrue(bytes_eq(representation, basn.c_str(), basn.size(), message), message->Data());

			{ // decode
				T restored = octets_to_asn(basn, nullptr);

				Assert::IsTrue(datum == restored, message->Data());
			}
		}

		template<typename T, typename Span, typename T2O, typename O2T> 
		void test_string(T& datum, Span span, T2O asn_to_octets, O2T octets_to_asn, const char* representation, const wchar_t* msgfmt) {
			Platform::String^ message = make_wstring(msgfmt, datum.c_str());
			octets basn = asn_to_octets(datum);

			Assert::IsTrue(bytes_eq(representation, basn.c_str(), basn.size(), message), message->Data());
			Assert::AreEqual(basn.size(), asn_span(span, datum), message->Data());

			{ // decode
				T restored = octets_to_asn(basn, nullptr);

				Assert::AreEqual(datum.c_str(), restored.c_str(), message->Data());
			}
		}
	};

	private class DERSequence : public TestClass<DERSequence> {
	public:
		TEST_METHOD(PlainSequence) {
			LogMessage log_msg(Log::Debug, "测试", 1585280242148LL, "tamer");

			test_sequence(log_msg, "\x30\x1a\x0a\x01\x00\x0c\x06\xe6\xb5\x8b\xe8\xaf\x95\x02\x06\x01\x71\x1a\x10\xd1\xe4\x16\x05\x74\x61\x6d\x65\x72");
		}

	private:
		void test_sequence(LogMessage& m, const char* representation) {
			Platform::String^ name = make_wstring(m.topic.c_str());
			octets basn = m.to_octets();
			
			Assert::IsTrue(bytes_eq(representation, basn.c_str(), basn.size(), name), name->Data());
			Assert::AreEqual(asn_span(&m), basn.size(), make_wstring(L"%s span", name->Data())->Data());

			{ // decode
				size_t offset = 0;
				LogMessage restored(basn, &offset);

				Assert::IsTrue(m.level == restored.level, make_wstring(L"%s topic", name->Data())->Data());
				Assert::IsTrue(m.message->Equals(restored.message), make_wstring(L"%s message", name->Data())->Data());
				Assert::AreEqual(m.timestamp, restored.timestamp, make_wstring(L"%s timestamp", name->Data())->Data());
				Assert::AreEqual(m.topic.c_str(), restored.topic.c_str(), make_wstring(L"%s topic", name->Data())->Data());
			}
		}
	};
}
