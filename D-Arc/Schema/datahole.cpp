#include "CppUnitTest.h"

#include "syslog.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DataHole;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Schema::DataHole {
	private class SqlClient : public TestClass<SqlClient> {
	public:
		TEST_METHOD(Connection) {
			try {
				int connection = 4L;
				MSSqlClient sql(connection);

				//Assert::AreEqual(connection, sql.description(), "connection string");
			} catch (Platform::COMException^ e) {
				syslog(Log::Info, L"%x: %s", e->HResult, e->Message->Data());
			}
		}
	};
}
