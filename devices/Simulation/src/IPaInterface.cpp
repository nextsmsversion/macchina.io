//
// IPaInterface.cpp
//
// Library: IoT/MQTT
// Package: Generated
// Module:  IPaInterface
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: Apache-2.0
//


#include "IoT/MQTT/IPaInterface.h"


namespace IoT {
namespace MQTT {


IPaInterface::IPaInterface():
	Poco::OSP::Service()

{
}


IPaInterface::~IPaInterface()
{
}


bool IPaInterface::isA(const std::type_info& otherType) const
{
	std::string name(type().name());
	return name == otherType.name();
}


const Poco::RemotingNG::Identifiable::TypeId& IPaInterface::remoting__typeId()
{
	remoting__staticInitBegin(REMOTING__TYPE_ID);
	static const std::string REMOTING__TYPE_ID("IoT.Simulation.PaInterface");
	remoting__staticInitEnd(REMOTING__TYPE_ID);
	return REMOTING__TYPE_ID;
}


const std::type_info& IPaInterface::type() const
{
	return typeid(IPaInterface);
}


} // namespace MQTT
} // namespace IoT

