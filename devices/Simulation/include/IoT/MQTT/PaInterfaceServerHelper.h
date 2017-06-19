//
// PaInterfaceServerHelper.h
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


#ifndef IoT_MQTT_PaInterfaceServerHelper_INCLUDED
#define IoT_MQTT_PaInterfaceServerHelper_INCLUDED


#include "IoT/MQTT/IPaInterface.h"
#include "IoT/MQTT/PaInterfaceRemoteObject.h"
#include "IoT/Simulation/PaInterface.h"
#include "Poco/RemotingNG/Identifiable.h"
#include "Poco/RemotingNG/ORB.h"
#include "Poco/RemotingNG/ServerHelper.h"


namespace IoT {
namespace MQTT {


class PaInterfaceServerHelper
	/// The interface for MQTT clients.
	///
	/// Implementations are expected to receive their client ID and
	/// server URI via an implementation defined configuration mechanism.
	/// Once configured, a MQTTClient always uses the same client ID and
	/// connects to the same server. A MQTT client should automatically
	/// attempt to reconnect if the connection to the server is lost.
{
public:
	typedef IoT::Simulation::PaInterface Service;

	PaInterfaceServerHelper();
		/// Creates a PaInterfaceServerHelper.

	~PaInterfaceServerHelper();
		/// Destroys the PaInterfaceServerHelper.

	static Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> createRemoteObject(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid);
		/// Creates and returns a RemoteObject wrapper for the given IoT::Simulation::PaInterface instance.

	static std::string registerObject(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid, const std::string& listenerId);
		/// Creates a RemoteObject wrapper for the given IoT::Simulation::PaInterface instance
		/// and registers it with the ORB and the Listener instance
		/// uniquely identified by the Listener's ID.
		/// 
		///	Returns the URI created for the object.

	static std::string registerRemoteObject(Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> pRemoteObject, const std::string& listenerId);
		/// Registers the given RemoteObject with the ORB and the Listener instance
		/// uniquely identified by the Listener's ID.
		/// 
		///	Returns the URI created for the object.

	static void shutdown();
		/// Removes the Skeleton for IoT::Simulation::PaInterface from the ORB.

	static void unregisterObject(const std::string& uri);
		/// Unregisters a service object identified by URI from the ORB.

private:
	static Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> createRemoteObjectImpl(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid);

	static PaInterfaceServerHelper& instance();
		/// Returns a static instance of the helper class.

	std::string registerObjectImpl(Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> pRemoteObject, const std::string& listenerId);

	void registerSkeleton();

	void unregisterObjectImpl(const std::string& uri);

	void unregisterSkeleton();

	Poco::RemotingNG::ORB* _pORB;
};


inline Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> PaInterfaceServerHelper::createRemoteObject(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid)
{
	return PaInterfaceServerHelper::instance().createRemoteObjectImpl(pServiceObject, oid);
}


inline std::string PaInterfaceServerHelper::registerObject(Poco::SharedPtr<IoT::Simulation::PaInterface> pServiceObject, const Poco::RemotingNG::Identifiable::ObjectId& oid, const std::string& listenerId)
{
	return PaInterfaceServerHelper::instance().registerObjectImpl(createRemoteObject(pServiceObject, oid), listenerId);
}


inline std::string PaInterfaceServerHelper::registerRemoteObject(Poco::AutoPtr<IoT::MQTT::PaInterfaceRemoteObject> pRemoteObject, const std::string& listenerId)
{
	return PaInterfaceServerHelper::instance().registerObjectImpl(pRemoteObject, listenerId);
}


inline void PaInterfaceServerHelper::unregisterObject(const std::string& uri)
{
	PaInterfaceServerHelper::instance().unregisterObjectImpl(uri);
}


} // namespace MQTT
} // namespace IoT


REMOTING_SPECIALIZE_SERVER_HELPER(IoT::MQTT, PaInterface)


#endif // IoT_MQTT_PaInterfaceServerHelper_INCLUDED

