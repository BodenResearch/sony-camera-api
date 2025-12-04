#include "ConnectionInfo.h"
#include <cstddef>
#include <cstring>
#include "CRSDK/CameraRemote_SDK.h"
#include "CameraDevice.h"

namespace cli
{
ConnectionType parse_connection_type(text conn_type)
{
    ConnectionType type(ConnectionType::UNKNOWN);

    if (TEXT("IP") == conn_type) {
        type = ConnectionType::NETWORK;
    }
    else if (TEXT("USB") == conn_type) {
        type = ConnectionType::USB;
    }
    return type;
}

} // namespace cli
