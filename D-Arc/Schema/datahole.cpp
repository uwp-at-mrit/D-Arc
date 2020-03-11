#include "CppUnitTest.h"

using namespace WarGrey::DataHole;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*************************************************************************************************/
namespace WarGrey::Tamer::Schema::DataHole {
	private class SqlClient : public TestClass<SqlClient> {
	public:
		TEST_METHOD(Connection) {
			SqlClient sql;
		}
	};
}
