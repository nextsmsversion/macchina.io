//
// PaInterfaceRemoteObject.h
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


#ifndef IoT_MQTT_PaInterfaceRemoteObject_INCLUDED
#define IoT_MQTT_PaInterfaceRemoteObject_INCLUDED


#include "IoT/MQTT/IPaInterface.h"
#include "Poco/RemotingNG/Identifiable.h"
#include "Poco/RemotingNG/RemoteObject.h"
#include "Poco/SharedPtr.h"


namespace IoT {
namespace MQTT {


class PaInterfaceRemoteObject: public IoT::MQTT::IPaInterface, public Poco::RemotingNG::RemoteObject
	/// The interface for MQTT clients.
	///
	/// Implementations are expected to receive their client ID and
	/// server URI via an implementation defined configuration mechanism.
	/// Once configured, a MQTTClient always uses the same client ID and
	/// connects to the same server. A MQTT client should automatically
	/// attempt to reconnect if the connection to the server is lost.
{
public:
	typedef Poco::AutoPtr<PaInterfaceRemoteObject> Ptr;

	PaInterfaceRemoteObject(const Poco::RemotingNG::Identifiable::ObjectId& oid, Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject);
		/// Creates a PaInterfaceRemoteObject.

	virtual ~PaInterfaceRemoteObject();
		/// Destroys the PaInterfaceRemoteObject.

	IoT::Simulation::PaInterface::MQTTClient IoT::Simulation::PaInterface::MQTTClient();

	virtual const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId() const;

private:
	Poco::SharedPtr<IoT::Simulation::PaInterface> _pServiceObject;
};


inline IoT::Simulation::PaInterface::MQTTClient PaInterfaceRemoteObject::IoT::Simulation::PaInterface::MQTTClient()
{
	return _pServiceObject->IoT::Simulation::PaInterface::MQTTClient();
}


inline const Poco::RemotingNG::Identifiable::TypeId& PaInterfaceRemoteObject::remoting__typeId() const
{
	return IPaInterface::remoting__typeId();
}


} // namespace MQTT
} // namespace IoT


#endif // IoT_MQTT_PaInterfaceRemoteObject_INCLUDED

