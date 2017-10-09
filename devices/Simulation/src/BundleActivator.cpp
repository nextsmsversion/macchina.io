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

//added by sam 20170601 for adding socket to connect to PA server START
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
//added by sam 20170601 for adding socket to connect to PA server FINISH

using Poco::OSP::BundleContext;
using Poco::OSP::ServiceRegistry;
using Poco::OSP::ServiceRef;
using Poco::OSP::ServiceFinder;
using Poco::OSP::Properties;
using Poco::OSP::PreferencesService;


namespace IoT {
namespace Simulation {


class PaUpdateTimerTask: public Poco::Util::TimerTask
{
private:
	int _cycles;
	int _count;

	//std::map<int,string> _instantMsgMap;

public:
	PaUpdateTimerTask()
/**	tmp by sam
	PaUpdateTimerTask(std::map<int,string> instantMsgMap):
		_instantMsgMap(instantMsgMap)
**/
	{

	}

	void run()
	{
		++_count;
		//tmp by sam 20170831 if (_count == _cycles)
		{//for every cycle

				cerr << "Inside PaUpdateTimerTask::instantMsgMap" <<endl;
				cerr << "PaUpdateTimerTask::up running .." <<endl;
				string localfilename = "/Users/sms/a.txt", remotefilename = "PAServer/config/instant.txt";
				try{
					if(getFile(localfilename, remotefilename, "anonymous", "sms", "128.13.109.246")){ //tmp by sam 201707130845
						//connection is alright -> do nothing
					}else{
						//tmp by sam	_instantMsgMap.clear();	//by sam 20170822 trial
						cerr << "20170831 PaUpdateTimerTask::instantMsgMap Clear : getFile else condition .." <<endl;
					}
				}
				catch (Exception& e1)
				{
					//tmp by sam	_instantMsgMap.clear();	//by sam 20170822 trial
					cerr << "20170831 PaUpdateTimerTask::instantMsgMap Clear : getFile else Exception .." <<endl;
				}
		}
	}

/*** getFile -> SmsUpdatePa::ThreadUpdateFtp //COPY from remote to local file path
 * reference from
 * https://git.sch.bme.hu/kk1205/raspberrycloud/blob/cachemeres/program/Source/cloud/FTPAdapter.cpp
 * @param	string localfilename	"/Users/sms/a.txt",
 * 			string remotefilename	"a.txt"
 * 			string USERNAME 		"ftpuser";
 * 			string PASSWORD 		"ftp123456";
 * 			string HOST 			"128.13.109.246";
 */
	bool getFile(	string localfilename, string remotefilename,
					string USERNAME, string PASSWORD, string HOST){
		//cerr << "LOG: Inside PaUpdateTimerTask getFile"  << endl;
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
				//session.endDownload();	//TODO throw exception??
		}
		catch (FTPException& e) {
			cerr << "20170822 debug error: " << e.message() << endl;
			//under the case the PA server is not existing/up
			return false;
		}
		return true;
		//added by sam 20170522 for trying FTP downloading file to local file FINISH
	}


};


class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator()
	{
	}
	
	~BundleActivator()
	{
	}
	
	void createSensor(const SimulatedSensor::Params& params, int tmpSockfrd, std::map<int,string> instantMsgMap)
	{
		typedef Poco::RemotingNG::ServerHelper<IoT::Devices::Sensor> ServerHelper;

		Poco::SharedPtr<SimulatedSensor> pSensor = new SimulatedSensor(params, *_pTimer, tmpSockfrd);


		ServerHelper::RemoteObjectPtr pSensorRemoteObject = ServerHelper::createRemoteObject(pSensor, params.id);
		
		Properties props;
		props.set("io.macchina.device", SimulatedSensor::SYMBOLIC_NAME);
		if (!params.physicalQuantity.empty())
		{
			props.set("io.macchina.physicalQuantity", params.physicalQuantity);
		}
		
		ServiceRef::Ptr pServiceRef = _pContext->registry().registerService(params.id, pSensorRemoteObject, props);
		if(_pContext){
			//by log params.id -> PA.instantMessage.1
			_pContext->logger().error(Poco::format("Simulation::BundleActivator void createSensor registry().registerService: %s", params.id));//tmp by sam 20170626
		}
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

	//added by sam 20170601 for adding socket to connect to PA server START
	//reference : InitClient EtherUtils.cpp
	bool connectSocket(){
		struct sockaddr_in dest;
		char buffer[128];

		/**create socket*/
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		/**initinalize value in dest**/
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(1100);
		dest.sin_addr.s_addr = inet_addr("128.13.109.246");

		/**connecting to server**/
		int result = connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));
		/** Receive messge from the server and print to screen */
		//bzero(buffer, 128);
		//recv(sockfd, buffer, sizeof(buffer), 0);
		//printf("receive from server : %s\n", buffer);

		//by sam 20170822 to test if PA server is up
		if(result == 0){
			return true;
		}else{
			return false;
		}
		//close(sockfd);
	}
	//added by sam 20170601 for adding socket to connect to PA server FINISH

	void start(BundleContext::Ptr pContext)
	{
		_pTimer = new Poco::Util::Timer;
		_pContext = pContext;
		_pPrefs = ServiceFinder::find<PreferencesService>(pContext);

		//20170831 by sam to setup a timer
		if(!instantMsgMap.empty())
		//if(*instantMsgMap != NULL)
		{
			long interval = 100;
			_pTimer->scheduleAtFixedRate(new PaUpdateTimerTask(), interval, interval);

			cerr << "new PaUpdateTimerTask at 201708311029 for checking connection of PA" << endl;
		}

		//TODO by sam reference: BOOL PPOpenObject to connect to PA server as client
		//TODO by sam 20170822 to handle the case PA has not been switched on
		//by sam 20170822 	if(sockfd == 0){//added by sam 20170606
		//by sam 20170822 		sockfd = connectSocket();//added by sam 20170601 for adding socket to connect to PA server
		//by sam 20170822 	}

		//TODO get the file from FTP
		//added by sam 20170525 to get the PA mesages through FTP START
		string localfilename = "/Users/sms/a.txt", remotefilename = "PAServer/config/instant.txt";
		//if(connectSocket() == true)
		{ //by sam 20170822 when PA server is valid
		//	getFile(localfilename, remotefilename, "anonymous", "sms", "128.13.109.246"); //tmp by sam 201707130845 //TODO *** solve the problem if the FTP is not ON
		//	loadFile("/Users/sms/a.txt");	//added by sam 20170523 filestream START
		}
		//added by sam 20170525 to get the PA mesages through FTP FINISH

		//if(connectSocket() == true){	//by sam 20170822
		//bool testingPAserverIsUp = connectSocket();	//testing if PA server is up

			///added by sam 20170524 for setting timer to update the PA from FTP START
			for(int i=1; i< 9 ;i++ )	//added by sam 20170529
			{
				std::string baseKey = "PA.instantMessage.";
				SimulatedSensor::Params params;
				params.id = std::to_string(i);	//this must be only number otherwise, cannot perform: Poco::AnyCast<string>(_sensor._deviceIdentifier);

				params.physicalQuantity = instantMsgMap[i]; //"db";// _pPrefs->configuration()->getString(baseKey + ".physicalQuantity", "");
				params.physicalUnit     = "db";// _pPrefs->configuration()->getString(baseKey + ".physicalUnit", "");
				params.initialValue     = -6.0;// _pPrefs->configuration()->getDouble(baseKey + ".initialValue", 0.0);
				params.delta            = -6.0;// _pPrefs->configuration()->getDouble(baseKey + ".delta", 0.0);
				params.cycles           = 100; //_pPrefs->configuration()->getInt(baseKey + ".cycles", 0);
				params.updateRate       = 11.0;//_pPrefs->configuration()->getDouble(baseKey + ".updateRate", 0.0);
				params.mode = SimulatedSensor::SIM_LINEAR;

				try
				{
					//if(testingPAserverIsUp == true)
					{
						createSensor(params, sockfd, instantMsgMap);
					}
				}
				catch (Poco::Exception& exc)
				{
					pContext->logger().error(Poco::format("Cannot create simulated sensor: %s", exc.displayText()));
				}
			}

			//added by sam 20170524 for setting timer to update the PA from FTP FINISH

	}
	void loadFile(	string localfilename){
		cerr << "LOG: Inside PaUpdateTimerTask loadFile"  << endl;

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
	 * 			string HOST 			"128.13.109.246";
	 */
	void getFile(	string localfilename, string remotefilename,
						string USERNAME, string PASSWORD, string HOST){
			cerr << "LOG: Inside PaUpdateTimerTask getFile"  << endl;
			try {
			//added by sam 20170518 for establishing FTP session START
			FTPClientSession session(HOST, FTPClientSession::FTP_PORT, USERNAME, PASSWORD); cerr << "LOG: 246"  << endl;
			Path localFilePath(localfilename);												cerr << "LOG: 247"  << endl;
			ofstream file(localFilePath.toString(), ios::out | ios::binary);				cerr << "LOG: 248"  << endl;
			//added by sam 20170518 for establishing FTP session FINISH

			//added by sam 20170522 for trying FTP downloading file to local file START

					session.setFileType(FTPClientSession::TYPE_BINARY);						cerr << "LOG: 253"  << endl;
					auto& is = session.beginDownload(remotefilename);						cerr << "LOG: 254"  << endl;
					StreamCopier::copyStream(is, file);										cerr << "LOG: 255"  << endl;
					//tmp by sam
					loadFile("/Users/sms/a.txt");	cerr << "LOG: 334 loadfile"  << endl;
					//session.endDownload();								//TODO throw exception??
			}
			catch (FTPException& e) {
				cerr << "error: " << e.message() << endl;
			}
			catch (Exception& e1){
				cerr << "error: " << e1.message() << endl;
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

	std::map<int,string> instantMsgMap;	//added by sam 20170529 for the PA instant message

	int sockfd = 0 ;	//added by sam 20170606
};


} } // namespace IoT::Simulation


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(IoT::Simulation::BundleActivator)
POCO_END_MANIFEST
