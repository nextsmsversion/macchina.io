//
// PaInterface.h
//
// $Id$
//
// Library: IoT/MQTT
// Package: PaInterface
// Module:  PaInterface
//
// Definition of the MQTTClient interface.
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#ifndef IoT_MQTT_PaInterface_INCLUDED
#define IoT_MQTT_PaInterface_INCLUDED


#include "Poco/BasicEvent.h"
#include "Poco/SharedPtr.h"
#include <vector>

//
// The following block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the IoTMQTT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// IoTMQTT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && defined(POCO_DLL)
	#if defined(IoTMQTT_EXPORTS)
		#define IoTMQTT_API __declspec(dllexport)
	#else
		#define IoTMQTT_API __declspec(dllimport)
	#endif
#endif


#if !defined(IoTMQTT_API)
	#define IoTMQTT_API
#endif

namespace IoT {
	namespace Simulation {
	//@ remote
	class IoTMQTT_API PaInterface
		/// The interface for MQTT clients.
		///
		/// Implementations are expected to receive their client ID and
		/// server URI via an implementation defined configuration mechanism.
		/// Once configured, a MQTTClient always uses the same client ID and
		/// connects to the same server. A MQTT client should automatically
		/// attempt to reconnect if the connection to the server is lost.
		{
			public:
				MQTTClient();

		};


	}
} // namespace IoT::MQTT


#endif // IoT_MQTT_PaInterface_INCLUDED
