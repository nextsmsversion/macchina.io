//
// SimulatedSensor.cpp
//
// $Id$
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "SimulatedSensor.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Random.h"


#include "Poco/OSP/Pa/PaService.h"	//by sam 20170705

//added by sam 20170523 filestream START
#include "Poco/FileStream.h"
//added by sam 20170523 filestream FINISH

//added by sam 20170522 for trying FTP START
#include <Poco/Exception.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/NetException.h>
//added by sam 20170522 for trying FTP FINISH

//added by sam 20170523 for tokenizing START
#include <sstream>
//added by sam 20170523 for tokenizing FINISH

//added by sam 20170518 for trying START
#include <Poco/Net/FTPClientSession.h>
#include <Poco/Path.h>
#include <iostream>
#include <fstream>

using namespace Poco::Net;
using namespace Poco;
using namespace std;
//added by sam 20170518 for trying END

//added by sam 20170602 for adding socket to connect to PA server START
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
//added by sam 20170602 for adding socket to connect to PA server FINISH

//added by sam 20170612 START
#include "Poco/RemotingNG/Deserializer.h"
#include "Poco/RemotingNG/MethodHandler.h"
#include "Poco/RemotingNG/Serializer.h"
#include "Poco/RemotingNG/ServerTransport.h"
#include "Poco/RemotingNG/TypeDeserializer.h"
#include "Poco/RemotingNG/TypeSerializer.h"

#include "Poco/RemotingNG/Skeleton.h"
//added by sam 20170612 FINISH

namespace IoT {
namespace Simulation {


class LinearUpdateTimerTask: public Poco::Util::TimerTask
{
public:
	LinearUpdateTimerTask(SimulatedSensor& sensor, double initialValue, double delta, int cycles, int tmpSockfrd):
		_sensor(sensor),
		_initialValue(initialValue),
		_delta(delta),
		_cycles(cycles),
		_count(0),
		_sockfd(tmpSockfrd)
	{

	}


	void run()
	{

		string localfilename = "/Users/sms/a.txt", remotefilename = "PAServer/config/instant.txt";
		getFile(localfilename, remotefilename, "anonymous", "sms", "128.12.46.246");
		loadFile("/Users/sms/a.txt");	//added by sam 20170523 filestream START

		if (++_count == _cycles)
		{
			_sensor.update(_initialValue);

			_sensor.setSymbolicName("INSTANT");	//added by sam 20170523 TODO get the PA instant name
			_count = 0;
		}
		else
		{
			double value = _sensor.value();
			value += _delta;
			_sensor.update(value);

			//by sam 20170706
			string sensorStr = Poco::AnyCast<string>(_sensor._deviceIdentifier);
			int thisSensorId = 	stoi(sensorStr);
			_sensor.setPhysicalQuantity(instantMsgMap[thisSensorId]); //TODO how come this cannot update

			_sensor.setSymbolicName("INSTANT"); //added by sam 20170523 TODO
		}
	}
	void loadFile(	string localfilename){
		//cerr << "LOG: Inside LinearUpdateTimerTask loadFile"  << endl;

		//added by sam 20170523 filestream START
		string line;
		std::ifstream myfile(localfilename);
		if(myfile.is_open()){
			instantMsgMap.clear();
			while(getline(myfile,line)){	//for each line of the file
				//added by sam 20170523 SmsFilePaMgr::ReadInstantMsgConfig for tokenizing the instant file START
				//Suppose string line = "Instant 01E	Test Message";

				/**decoding of the following example message line:
				 * msgID:		msgText:
				 * Instant 01E	Test Message
				 */
				stringstream ss(line);
				string tmpStr = "", msgText = "";
				int instantMsgColumn = 1;
				int msgID;
				while (getline(ss,tmpStr,'\t')){
					switch (instantMsgColumn){
						case 1:		//first column
							msgID = std::stoi(tmpStr.substr(8,2));
							break;
						case 2:		//second column
							msgText = tmpStr;
							break;
					}
					instantMsgColumn ++;
				}
				//added by sam 20170525 for saving the instant message
				//for each row add to the instant message map;
				instantMsgMap[msgID] = msgText;
				//added by sam 20170523 SmsFilePaMgr::ReadInstantMsgConfig for tokenizing the instant file FINISH
			}
			myfile.close();
		}


		//added by sam 20170523 filestream FINISH
	}

/*** getFile -> SmsUpdatePa::ThreadUpdateFtp //COPY from remote to local file path
 * reference from
 * https://git.sch.bme.hu/kk1205/raspberrycloud/blob/cachemeres/program/Source/cloud/FTPAdapter.cpp
 * @param	string localfilename	"/Users/sms/a.txt",
 * 			string remotefilename	"a.txt"
 * 			string USERNAME 		"ftpuser";
 * 			string PASSWORD 		"ftp123456";
 * 			string HOST 			"128.12.46.246";
 */
	void getFile(	string localfilename, string remotefilename,
					string USERNAME, string PASSWORD, string HOST){
		//cerr << "LOG: Inside LinearUpdateTimerTask getFile"  << endl;
		//added by sam 20170518 for establishing FTP session START
		FTPClientSession session(HOST, FTPClientSession::FTP_PORT, USERNAME, PASSWORD);
		Path localFilePath(localfilename);
		ofstream file(localFilePath.toString(), ios::out | ios::binary);	//TODO by sam to set to ASCII??
		//added by sam 20170518 for establishing FTP session FINISH

		//added by sam 20170522 for trying FTP downloading file to local file START
		try {
				session.setFileType(FTPClientSession::TYPE_BINARY);
				auto& is = session.beginDownload(remotefilename);
				StreamCopier::copyStream(is, file);
				//session.endDownload();								//TODO throw exception??
		}
		catch (FTPException& e) {
			cerr << "error: " << e.message() << endl;
		}
		//added by sam 20170522 for trying FTP downloading file to local file FINISH
	}


/*** Remarks: FTPClientSession Need to add "PocoNet" in the following inside Makefile
	 * target_libs    = IoTDevices PocoRemotingNG PocoOSP PocoGeo PocoUtil PocoXML PocoFoundation PocoNet
	 */
private:
	SimulatedSensor& _sensor;
	double _initialValue;
	double _delta;
	int _cycles;
	int _count;

	std::map<int,string> instantMsgMap;	//added by sam 20170525 for the instant message
	int _sockfd;	//20170602
};


class RandomUpdateTimerTask: public Poco::Util::TimerTask
{
public: /**TODO by sam to get the data from instant.txt **/
	RandomUpdateTimerTask(SimulatedSensor& sensor, double initialValue, double delta, int cycles):
		_sensor(sensor),
		_initialValue(initialValue),
		_delta(delta),
		_cycles(cycles),
		_count(0)
	{
	}
	
	void run()
	{
		if (++_count == _cycles)
		{
			_sensor.update(_initialValue);
			_count = 0;
		}
		else
		{
			double value = _sensor.value();
			value += _delta*_random.nextDouble()*2 - _delta;
			_sensor.update(value);
		}
	}
	
private:
	SimulatedSensor& _sensor;
	double _initialValue;
	double _delta;
	int _cycles;
	int _count;
	Poco::Random _random;
};


const std::string SimulatedSensor::NAME("Simulated Sensor");
const std::string SimulatedSensor::SYMBOLIC_NAME("io.macchina.simulation.sensor");


//by sam 20170612 TODO make the function specific to SimulatedSensor
class SimulatedSensorSimulatedTopicsMethodHandler: public Poco::RemotingNG::MethodHandler
{
public:
	void invoke(Poco::RemotingNG::ServerTransport& remoting__trans, Poco::RemotingNG::Deserializer& remoting__deser, Poco::RemotingNG::RemoteObject::Ptr remoting__pRemoteObject)
	{
		remoting__staticInitBegin(REMOTING__NAMES);
		static const std::string REMOTING__NAMES[] = {"simulatedTopics"};
		remoting__staticInitEnd(REMOTING__NAMES);
		bool remoting__requestSucceeded = false;
		try
		{
			remoting__deser.deserializeMessageBegin(REMOTING__NAMES[0], Poco::RemotingNG::SerializerBase::MESSAGE_REQUEST);
			remoting__deser.deserializeMessageEnd(REMOTING__NAMES[0], Poco::RemotingNG::SerializerBase::MESSAGE_REQUEST);
			//TODO by sam to call the pSensorRemoteObject as in BundleActivator
			//IoT::Simulation::SimulatedSensor* remoting__pCastedRO = static_cast<IoT::Simulation::SimulatedSensor*>(remoting__pRemoteObject.get());
			//std::vector < IoT::MQTT::TopicQoS > remoting__return = remoting__pCastedRO->subscribedTopics();
			remoting__requestSucceeded = true;
			Poco::RemotingNG::Serializer& remoting__ser = remoting__trans.sendReply(Poco::RemotingNG::SerializerBase::MESSAGE_REPLY);
			remoting__staticInitBegin(REMOTING__REPLY_NAME);
			static const std::string REMOTING__REPLY_NAME("simulatedTopicsReply");
			remoting__staticInitEnd(REMOTING__REPLY_NAME);
			remoting__ser.serializeMessageBegin(REMOTING__REPLY_NAME, Poco::RemotingNG::SerializerBase::MESSAGE_REPLY);
			//Poco::RemotingNG::TypeSerializer<std::vector < IoT::MQTT::TopicQoS > >::serialize(Poco::RemotingNG::SerializerBase::RETURN_PARAM, remoting__return, remoting__ser);
			remoting__ser.serializeMessageEnd(REMOTING__REPLY_NAME, Poco::RemotingNG::SerializerBase::MESSAGE_REPLY);
		}
		catch (Poco::Exception& e)
		{
			if (!remoting__requestSucceeded)
			{
				Poco::RemotingNG::Serializer& remoting__ser = remoting__trans.sendReply(Poco::RemotingNG::SerializerBase::MESSAGE_FAULT);
				remoting__ser.serializeFaultMessage(REMOTING__NAMES[0], e);
			}
		}
		catch (std::exception& e)
		{
			if (!remoting__requestSucceeded)
			{
				Poco::RemotingNG::Serializer& remoting__ser = remoting__trans.sendReply(Poco::RemotingNG::SerializerBase::MESSAGE_FAULT);
				Poco::Exception exc(e.what());
				remoting__ser.serializeFaultMessage(REMOTING__NAMES[0], exc);
			}
		}
		catch (...)
		{
			if (!remoting__requestSucceeded)
			{
				Poco::RemotingNG::Serializer& remoting__ser = remoting__trans.sendReply(Poco::RemotingNG::SerializerBase::MESSAGE_FAULT);
				Poco::Exception exc("Unknown Exception");
				remoting__ser.serializeFaultMessage(REMOTING__NAMES[0], exc);
			}
		}
	}

};

/**by sam 20170612
**/
class SimulatedSensorSkeleton: public Poco::RemotingNG::Skeleton
	/// The interface for MQTT clients.
	///
	/// Implementations are expected to receive their client ID and
	/// server URI via an implementation defined configuration mechanism.
	/// Once configured, a MQTTClient always uses the same client ID and
	/// connects to the same server. A MQTT client should automatically
	/// attempt to reconnect if the connection to the server is lost.
{
	public:
		SimulatedSensorSkeleton(){
			addMethodHandler("simulatedTopics", new IoT::Simulation::SimulatedSensorSimulatedTopicsMethodHandler);
		}
			/// Creates a MQTTClientSkeleton.

		virtual ~SimulatedSensorSkeleton();
			/// Destroys a MQTTClientSkeleton.

		virtual const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId() const;

		static const std::string DEFAULT_NS;
};
SimulatedSensor::SimulatedSensor(const Params& params, Poco::Util::Timer& timer, int tmpSockfrd):
	_value(params.initialValue),
	_valueChangedPeriod(0.0),
	_valueChangedDelta(0.0),
	_pEventPolicy(new IoT::Devices::NoModerationPolicy<double>(valueChanged)),
	_deviceIdentifier(params.id),
	_symbolicName(SYMBOLIC_NAME),
	_name(NAME),
	_physicalQuantity(params.physicalQuantity),
	_physicalUnit(params.physicalUnit),
	_timer(timer)
{
	addProperty("displayValue", &SimulatedSensor::getDisplayValue);
	addProperty("valueChangedPeriod", &SimulatedSensor::getValueChangedPeriod, &SimulatedSensor::setValueChangedPeriod);
	addProperty("valueChangedDelta", &SimulatedSensor::getValueChangedDelta, &SimulatedSensor::setValueChangedDelta);
	addProperty("deviceIdentifier", &SimulatedSensor::getDeviceIdentifier);
	addProperty("symbolicName", &SimulatedSensor::getSymbolicName);
	addProperty("name", &SimulatedSensor::getName);
	addProperty("physicalQuantity", &SimulatedSensor::getPhysicalQuantity);
	addProperty("physicalUnit", &SimulatedSensor::getPhysicalUnit);
	addProperty("PaMessage", &SimulatedSensor::PaMessage);

	if (params.updateRate > 0)
	{
		long interval = 1000/params.updateRate;
		if (params.mode == SIM_LINEAR)
		{
			_timer.scheduleAtFixedRate(new LinearUpdateTimerTask(*this, params.initialValue, params.delta, params.cycles, tmpSockfrd), interval, interval);
		}
		else
		{
			_timer.scheduleAtFixedRate(new RandomUpdateTimerTask(*this, params.initialValue, params.delta, params.cycles), interval, interval);
		}
	}

}


//added by sam 20170601 for adding socket to connect to PA server START
//reference : InitClient EtherUtils.cpp

//added by sam 20170601 for adding socket to connect to PA server FINISH

SimulatedSensor::~SimulatedSensor()
{
}


double SimulatedSensor::value() const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _value;
}


bool SimulatedSensor::ready() const
{
	return true;
}


Poco::Any SimulatedSensor::getValueChangedPeriod(const std::string&) const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _valueChangedPeriod;
}


void SimulatedSensor::setValueChangedPeriod(const std::string&, const Poco::Any& value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	int period = Poco::AnyCast<int>(value);
	if (period != _valueChangedPeriod)
	{
		if (period == 0)
		{
			_pEventPolicy = new IoT::Devices::NoModerationPolicy<double>(valueChanged);
		}
		else
		{
			_pEventPolicy = new IoT::Devices::MaximumRateModerationPolicy<double>(valueChanged, _value, period, _timer);
		}
		_valueChangedPeriod = period;
	}
}


Poco::Any SimulatedSensor::getValueChangedDelta(const std::string&) const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _valueChangedDelta;
}


void SimulatedSensor::setValueChangedDelta(const std::string&, const Poco::Any& value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	double delta = Poco::AnyCast<double>(value);
	if (delta != _valueChangedDelta)
	{
		if (delta == 0)
		{
			_pEventPolicy = new IoT::Devices::NoModerationPolicy<double>(valueChanged);
		}
		else
		{
			_pEventPolicy = new IoT::Devices::MinimumDeltaModerationPolicy<double>(valueChanged, _value, delta);
		}
		_valueChangedDelta = delta;
	}
}


Poco::Any SimulatedSensor::getDisplayValue(const std::string&) const
{
	return Poco::NumberFormatter::format(value(), 0, 1);
}


Poco::Any SimulatedSensor::getDeviceIdentifier(const std::string&) const
{
	return _deviceIdentifier;
}


Poco::Any SimulatedSensor::getName(const std::string&) const
{
	return _name;
}


Poco::Any SimulatedSensor::getSymbolicName(const std::string&) const
{
	return _symbolicName;
}

void SimulatedSensor::setSymbolicName(std::string value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	//if (_symbolicName != value)
	{
		_symbolicName = value;
		//_pEventPolicy->valueChanged(value);
	}
}
void SimulatedSensor::setPhysicalQuantity(std::string value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	//if (_symbolicName != value)
	{
		_physicalQuantity = value;
		//_pEventPolicy->valueChanged(value);
	}
}

Poco::Any SimulatedSensor::getPhysicalQuantity(const std::string&) const
{
	return _physicalQuantity;
}


Poco::Any SimulatedSensor::getPhysicalUnit(const std::string&) const
{
	return _physicalUnit;
}

Poco::Any SimulatedSensor::PaMessage(const std::string&) const
{
	return Poco::NumberFormatter::format(value(), 2, 3);
}


void SimulatedSensor::update(double value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	if (_value != value)
	{
		_value = value;
		_pEventPolicy->valueChanged(value);
	}
}


} } // namespace IoT::Simulation
