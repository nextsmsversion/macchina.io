//
// BundleActivator.cpp
//
// $Id$
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/RemotingNG/ORB.h"
#include "Poco/Util/Timer.h"
#include "IoT/Devices/SensorServerHelper.h"
#include "IoT/Devices/GNSSSensorServerHelper.h"
#include "Poco/Delegate.h"
#include "Poco/ClassLibrary.h"
#include "Poco/Format.h"
#include "Poco/NumberFormatter.h"
#include "Poco/SharedPtr.h"
#include "SimulatedSensor.h"
#include "SimulatedGNSSSensor.h"
#include <vector>

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
//added by sam 20170518 for trying FINISH

using Poco::OSP::BundleContext;
using Poco::OSP::ServiceRegistry;
using Poco::OSP::ServiceRef;
using Poco::OSP::ServiceFinder;
using Poco::OSP::Properties;
using Poco::OSP::PreferencesService;


namespace IoT {
namespace Simulation {


class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator()
	{
	}
	
	~BundleActivator()
	{
	}
	
	void createSensor(const SimulatedSensor::Params& params)
	{
		typedef Poco::RemotingNG::ServerHelper<IoT::Devices::Sensor> ServerHelper;
		
		Poco::SharedPtr<SimulatedSensor> pSensor = new SimulatedSensor(params, *_pTimer);
		ServerHelper::RemoteObjectPtr pSensorRemoteObject = ServerHelper::createRemoteObject(pSensor, params.id);
		
		Properties props;
		props.set("io.macchina.device", SimulatedSensor::SYMBOLIC_NAME);
		if (!params.physicalQuantity.empty())
		{
			props.set("io.macchina.physicalQuantity", params.physicalQuantity);
		}
		
		ServiceRef::Ptr pServiceRef = _pContext->registry().registerService(params.id, pSensorRemoteObject, props);
		_serviceRefs.push_back(pServiceRef);
	}

	void createGNSSSensor(const SimulatedGNSSSensor::Params& params)
	{
		typedef Poco::RemotingNG::ServerHelper<IoT::Devices::GNSSSensor> ServerHelper;
		
		Poco::SharedPtr<SimulatedGNSSSensor> pGNSSSensor = new SimulatedGNSSSensor(params);
		ServerHelper::RemoteObjectPtr pGNSSSensorRemoteObject = ServerHelper::createRemoteObject(pGNSSSensor, params.id);
		
		Properties props;
		props.set("io.macchina.device", SimulatedGNSSSensor::SYMBOLIC_NAME);
		
		ServiceRef::Ptr pServiceRef = _pContext->registry().registerService(params.id, pGNSSSensorRemoteObject, props);
		_serviceRefs.push_back(pServiceRef);
	}

	void start(BundleContext::Ptr pContext)
	{
		_pTimer = new Poco::Util::Timer;
		_pContext = pContext;
		_pPrefs = ServiceFinder::find<PreferencesService>(pContext);


		//TODO get the file from FTP
		//added by sam 20170525 to get the PA mesages through FTP START
		string localfilename = "/Users/sms/a.txt", remotefilename = "instant.txt";
		getFile(localfilename, remotefilename,
				"ftpuser", "ftp123456", "192.168.11.84");
		loadFile("/Users/sms/a.txt");	//added by sam 20170523 filestream START
		//added by sam 20170525 to get the PA mesages through FTP FINISH

		///added by sam 20170524 for setting timer to update the PA from FTP START
		{
			std::string baseKey = "simulation.sensors.";
			SimulatedSensor::Params params;
			params.id = SimulatedSensor::SYMBOLIC_NAME;
			params.id += "#PA";

			params.physicalQuantity = "db";// _pPrefs->configuration()->getString(baseKey + ".physicalQuantity", "");
			params.physicalUnit     = "db";// _pPrefs->configuration()->getString(baseKey + ".physicalUnit", "");
			params.initialValue     = -6.0;// _pPrefs->configuration()->getDouble(baseKey + ".initialValue", 0.0);
			params.delta            = -6.0;// _pPrefs->configuration()->getDouble(baseKey + ".delta", 0.0);
			params.cycles           = 100; //_pPrefs->configuration()->getInt(baseKey + ".cycles", 0);
			params.updateRate       = 11.0;//_pPrefs->configuration()->getDouble(baseKey + ".updateRate", 0.0);
			params.mode = SimulatedSensor::SIM_LINEAR;

			try
			{
				createSensor(params);
			}
			catch (Poco::Exception& exc)
			{
				pContext->logger().error(Poco::format("Cannot create simulated sensor: %s", exc.displayText()));
			}
		}
		//added by sam 20170524 for setting timer to update the PA from FTP FINISH

		Poco::Util::AbstractConfiguration::Keys keys;
		_pPrefs->configuration()->keys("simulation.sensors", keys);
		int index = 0;
		for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
		{
			std::string baseKey = "simulation.sensors.";
			baseKey += *it;

			SimulatedSensor::Params params;
			params.id = SimulatedSensor::SYMBOLIC_NAME;
			params.id += "#";
			params.id += Poco::NumberFormatter::format(_serviceRefs.size());

			params.physicalQuantity = _pPrefs->configuration()->getString(baseKey + ".physicalQuantity", "");
			params.physicalUnit     = _pPrefs->configuration()->getString(baseKey + ".physicalUnit", "");
			params.initialValue     = _pPrefs->configuration()->getDouble(baseKey + ".initialValue", 0.0);
			params.delta            = _pPrefs->configuration()->getDouble(baseKey + ".delta", 0.0);
			params.cycles           = _pPrefs->configuration()->getInt(baseKey + ".cycles", 0);
			params.updateRate       = _pPrefs->configuration()->getDouble(baseKey + ".updateRate", 0.0);

			std::string mode = _pPrefs->configuration()->getString(baseKey + ".mode", "linear");
			if (mode == "linear")
				params.mode = SimulatedSensor::SIM_LINEAR;
			else if (mode == "random")
				params.mode = SimulatedSensor::SIM_RANDOM;

			try
			{
				createSensor(params);
			}
			catch (Poco::Exception& exc)
			{
				pContext->logger().error(Poco::format("Cannot create simulated sensor: %s", exc.displayText()));
			}
			index++;
		}

		std::string gpxPath = _pPrefs->configuration()->getString("simulation.gnss.gpxPath", "");
		if (!gpxPath.empty())
		{
			SimulatedGNSSSensor::Params params;
			params.id = SimulatedGNSSSensor::SYMBOLIC_NAME;
			params.gpxPath = gpxPath;
			params.loopReplay = _pPrefs->configuration()->getBool("simulation.gnss.loopReplay", true);
			params.speedUp = _pPrefs->configuration()->getDouble("simulation.gnss.speedUp", 1.0);

			try
			{
				createGNSSSensor(params);
			}
			catch (Poco::Exception& exc)
			{
				pContext->logger().error(Poco::format("Cannot create simulated GNSS sensor: %s", exc.displayText()));
			}
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
				stringstream ss(line);
				string tmpStr;
				cerr << "PA Instant Message: " << endl;
				while (getline(ss,tmpStr,'\t')){
					cerr << tmpStr << endl;
				}
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
	 * 			string HOST 			"192.168.11.84";
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

	void stop(BundleContext::Ptr pContext)
	{
		_pTimer->cancel(true);
		_pTimer = 0;

		for (std::vector<ServiceRef::Ptr>::iterator it = _serviceRefs.begin(); it != _serviceRefs.end(); ++it)
		{
			_pContext->registry().unregisterService(*it);
		}
		_serviceRefs.clear();

		_pPrefs = 0;
		_pContext = 0;
	}
	
private:
	Poco::SharedPtr<Poco::Util::Timer> _pTimer;
	BundleContext::Ptr _pContext;
	PreferencesService::Ptr _pPrefs;
	std::vector<ServiceRef::Ptr> _serviceRefs;
};


} } // namespace IoT::Simulation


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(IoT::Simulation::BundleActivator)
POCO_END_MANIFEST
