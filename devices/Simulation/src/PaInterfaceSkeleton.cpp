//
// PaInterfaceSkeleton.cpp
//
// Library: IoT/MQTT
// Package: Generated
// Module:  PaInterfaceSkeleton
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: Apache-2.0
//


#include "IoT/MQTT/PaInterfaceSkeleton.h"
#include "Poco/RemotingNG/Deserializer.h"
#include "Poco/RemotingNG/MethodHandler.h"
#include "Poco/RemotingNG/Serializer.h"
#include "Poco/RemotingNG/ServerTransport.h"
#include "Poco/RemotingNG/TypeDeserializer.h"
#include "Poco/RemotingNG/TypeSerializer.h"
#include "Poco/SharedPtr.h"


namespace IoT {
namespace MQTT {


PaInterfaceSkeleton::PaInterfaceSkeleton():
	Poco::RemotingNG::Skeleton()

{
}


PaInterfaceSkeleton::~PaInterfaceSkeleton()
{
}


const std::string PaInterfaceSkeleton::DEFAULT_NS("");
} // namespace MQTT
} // namespace IoT

