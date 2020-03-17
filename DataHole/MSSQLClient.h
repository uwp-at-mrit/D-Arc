#pragma once

#include "Class.g.h"

namespace winrt::WarGrey::DataHole::implementation {
    struct MSSQLClient : ClassT<MSSQLClient> {
        MSSQLClient() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::WarGrey::DataHole::factory_implementation {
    struct MSSQLClient : ClassT<MSSQLClient, implementation::MSSQLClient> {
    };
}
