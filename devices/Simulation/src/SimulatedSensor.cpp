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
namespace IoT {
namespace Simulation {


class LinearUpdateTimerTask: public Poco::Util::TimerTask
{
public:
	LinearUpdateTimerTask(SimulatedSensor& sensor, double initialValue, double delta, int cycles):
		_sensor(sensor),
		_initialValue(initialValue),
		_delta(delta),
		_cycles(cycles),
		_count(0)
	{
		connectSocket();
	}

	//added by sam 20170601 for adding socket to connect to PA server START
	//reference : InitClient EtherUtils.cpp
	void connectSocket(){
		cerr << "LOG: Inside SimulatedSensor:connectSocket BEGIN "  << endl;

		/**create socket*/
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		/**initinalize value in dest**/
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(1100);
		dest.sin_addr.s_addr = inet_addr("128.12.46.246");

		/**connecting to server**/
		connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));


		//close(sockfd);
	}

	void sendPaMsg(){
		cerr << "LOG: Inside SimulatedSensor:sendPaMsg BEGIN "  << endl;

		char buffer[128];
		char paMsg[13]="0001C22ST@F1";


		send(sockfd, paMsg, sizeof(paMsg), 0);

		/** Receive messge from the server and print to screen */
		//bzero(buffer, 128);
		//recv(sockfd, buffer, sizeof(buffer), 0);
		//printf("receive from server : %s\n", buffer);
		cerr << "LOG: Inside SimulatedSensor sendPaMsg send(sockfd"  << endl;

		//close(sockfd);
	}
	//added by sam 20170601 for adding socket to connect to PA server FINISH


	void run()
	{

		sendPaMsg();	//added by sam 2017062 for sending trial pa msg
		/** tmp by sam
		string localfilename = "/Users/sms/a.txt", remotefilename = "instant.txt";
		getFile(localfilename, remotefilename,
				"ftpuser", "ftp123456", "128.12.46.246");
				**/
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

			_sensor.setSymbolicName("INSTANT"); //added by sam 20170523 TODO
		}
	}
	void loadFile(	string localfilename){
		cerr << "LOG: Inside LinearUpdateTimerTask loadFile"  << endl;

		//added by sam 20170523 filestream START
		string line;
		std::ifstream myfile(localfilename);
		if(myfile.is_open()){
			while(getline(myfile,line)){	//for each line of the file
				//added by sam 20170523 SmsFilePaMgr::ReadInstantMsgConfig for tokenizing the instant file START
				//Suppose string line = "Instant 01E	Test Message";

				/**decoding of the following example message line:
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
							//cerr << "msgID:<"<< msgID << ">" << endl;
							break;
						case 2:		//second column
							msgText = tmpStr;
							//cerr << "msgText:["<< msgText << "]" ; cerr << endl;
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

		cerr << "PA Instant Message: " << endl;
		cerr << "instantMsgMap[1]:" << instantMsgMap[1] << endl;
		cerr << "instantMsgMap[3]:" << instantMsgMap[3] << endl;
		cerr << "instantMsgMap[6]:" << instantMsgMap[6] << endl;

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
		cerr << "LOG: Inside LinearUpdateTimerTask getFile"  << endl;
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
	int sockfd;	//20170602
	struct sockaddr_in dest;//added by sam 20170602
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


SimulatedSensor::SimulatedSensor(const Params& params, Poco::Util::Timer& timer):
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
	

	if (params.updateRate > 0)
	{
		long interval = 1000/params.updateRate;
		if (params.mode == SIM_LINEAR)
		{
			_timer.scheduleAtFixedRate(new LinearUpdateTimerTask(*this, params.initialValue, params.delta, params.cycles), interval, interval);
		}
		else
		{
			_timer.scheduleAtFixedRate(new RandomUpdateTimerTask(*this, params.initialValue, params.delta, params.cycles), interval, interval);
		}
	}
}

	
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

Poco::Any SimulatedSensor::getPhysicalQuantity(const std::string&) const
{
	return _physicalQuantity;
}


Poco::Any SimulatedSensor::getPhysicalUnit(const std::string&) const
{
	return _physicalUnit;
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
