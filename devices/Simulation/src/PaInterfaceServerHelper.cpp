//
// PaInterfaceServerHelper.cpp
//
// Library: IoT/MQTT
// Package: Generated
// Module:  PaInterfaceServerHelper
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: Apache-2.0
//


#include "IoT/MQTT/PaInterfaceServerHelper.h"
#include "IoT/MQTT/PaInterfaceSkeleton.h"
#include "Poco/RemotingNG/URIUtility.h"
#include "Poco/SingletonHolder.h"


namespace IoT {
namespace MQTT {


namespace
{
	static Poco::SingletonHolder<PaInterfaceServerHelper> shPaInterfaceServerHelper;
}


PaInterfaceServerHelper::PaInterfaceServerHelper():
	_pORB(0)
{
	_pORB = &Poco::RemotingNG::ORB::instance();
	registerSkeleton();
}


PaInterfaceServerHelper::~PaInterfaceServerHelper()
{
}


void PaInterfaceServerHelper::shutdown()
{
	PaInterfaceServerHelper::instance().unregisterSkeleton();
	shPaInterfaceServerHelper.reset();
}


Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> PaInterfaceServerHelper::createRemoteObjectImpl(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid)
{
	return new PaInterfaceRemoteObject(oid, pServiceObject);
}


PaInterfaceServerHelper& PaInterfaceServerHelper::instance()
{
	return *shPaInterfaceServerHelper.get();
}


std::string PaInterfaceServerHelper::registerObjectImpl(Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> pRemoteObject, const std::string& listenerId)
{
	return _pORB->registerObject(pRemoteObject, listenerId);
}


void PaInterfaceServerHelper::registerSkeleton()
{
	_pORB->registerSkeleton("IoT.Simulation.PaInterface", new PaInterfaceSkeleton);
}


void PaInterfaceServerHelper::unregisterObjectImpl(const std::string& uri)
{
	_pORB->unregisterObject(uri);
}


void PaInterfaceServerHelper::unregisterSkeleton()
{
	_pORB->unregisterSkeleton("IoT.Simulation.PaInterface", true);
}


} // namespace MQTT
} // namespace IoT

