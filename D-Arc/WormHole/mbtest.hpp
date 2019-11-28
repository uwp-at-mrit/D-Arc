#pragma once

#include "modbus.hpp"
#include "syslog.hpp"

namespace WarGrey::Tamer {
	WarGrey::SCADA::IModbusServer* make_modbus_test_server(WarGrey::SCADA::Syslog* logger);
	WarGrey::SCADA::IModbusClient* make_modbus_test_client(WarGrey::SCADA::Syslog* logger, Platform::String^ device, WarGrey::SCADA::IModbusConfirmation* confirmation);

	void modbus_test_client(Platform::String^ device, WarGrey::SCADA::Syslog* logger = nullptr);
	void modbus_test_server(WarGrey::SCADA::Syslog* logger = nullptr);
}
