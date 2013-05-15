#pragma once

#include "ServerType.h"
#include "DataTypes.h"

const static Range<U32> ConnectionIdExclusion = { 0xFFFFFF00, 0xFFFFFFFF };

const U32   ServerToServerConnectionId = ConnectionIdExclusion.high;
const static U32 MaxBufferSize = 8192;