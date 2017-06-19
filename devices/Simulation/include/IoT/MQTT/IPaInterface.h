//
// IPaInterface.h
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


#ifndef IoT_MQTT_IPaInterface_INCLUDED
#define IoT_MQTT_IPaInterface_INCLUDED


#include "IoT/Simulation/PaInterface.h"
#include "Poco/AutoPtr.h"
#include "Poco/OSP/Service.h"
#include "Poco/RemotingNG/Identifiable.h"


namespace IoT {
namespace MQTT {


class IPaInterface: public Poco::OSP::Service
	/// The interface for MQTT clients.
	///
	/// Implementations are expected to receive their client ID and
	/// server URI via an implementation defined configuration mechanism.
	/// Once configured, a MQTTClient always uses the same client ID and
	/// connects to the same server. A MQTT client should automatically
	/// attempt to reconnect if the connection to the server is lost.
{
public:
	typedef Poco::AutoPtr<IPaInterface> Ptr;

	IPaInterface();
		/// Creates a IPaInterface.

	virtual ~IPaInterface();
		/// Destroys the IPaInterface.

	virtual virtual IoT::Simulation::PaInterface::MQTTClient() = 0;

	bool isA(const std::type_info& otherType) const;
		/// Returns true if the class is a subclass of the class given by otherType.

	static const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId();
		/// Returns the TypeId of the class.

	const std::type_info& type() const;
		/// Returns the type information for the object's class.

};


} // namespace MQTT
} // namespace IoT


#endif // IoT_MQTT_IPaInterface_INCLUDED

