//
// PaInterfaceRemoteObject.cpp
//
// Library: IoT/MQTT
// Package: Generated
// Module:  PaInterfaceRemoteObject
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: Apache-2.0
//


#include "IoT/MQTT/PaInterfaceRemoteObject.h"


namespace IoT {
namespace MQTT {


PaInterfaceRemoteObject::PaInterfaceRemoteObject(const Poco::RemotingNG::Identifiable::ObjectId& oid, Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject):
	IoT::MQTT::IPaInterface(),
	Poco::RemotingNG::RemoteObject(oid),
	_pServiceObject(pServiceObject)
{
}


PaInterfaceRemoteObject::~PaInterfaceRemoteObject()
{
	try
	{
	}
	catch (...)
	{
		poco_unexpected();
	}
}


} // namespace MQTT
} // namespace IoT

