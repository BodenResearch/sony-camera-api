#ifndef DEVICEID_H
#define DEVICEID_H

#include <cstdint>
#include "Text.h"

namespace cli
{

enum class ConnectionType
{
    UNKNOWN,
    NETWORK,
    USB
};

ConnectionType parse_connection_type(text conn_type);

} // namespace cli

#endif // !DEVICEID_H
