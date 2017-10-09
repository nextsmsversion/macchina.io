//
// UrlPa.cpp
//
// $Id$
//
// Copyright (c) 2007-2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "Poco/OSP/Pa/PaService.h"
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/AutoPtr.h"
#include "Poco/StringTokenizer.h"
#include "Poco/ClassLibrary.h"
#include "Poco/MD5Engine.h"
#include <set>


//added by sam 20170522 for trying FTP START
#include <Poco/Exception.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/NetException.h>
//added by sam 20170522 for trying FTP FINISH

//added by sam 20170523 filestream START
#include "Poco/FileStream.h"
#include <sstream>
//added by sam 20170523 for tokenizing FINISH

//added by sam 20170703 for adding socket to connect to PA server START
#include <Poco/Net/FTPClientSession.h>	//inet_addr
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
//added by sam 20170703 for adding socket to connect to PA server FINISH

//added by sam 20171004 for timer receiving PA messages
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTaskAdapter.h"
//added by sam 20171004 for timer receiving PA messages

//added by sam 20170518 for trying START
#include <Poco/Path.h>
#include <iostream>
#include <fstream>

using namespace Poco::Net;
using namespace Poco;
using namespace std;
//added by sam 20170518 for trying FINISH

using Poco::OSP::BundleActivator;
using Poco::OSP::BundleContext;
using Poco::OSP::Bundle;
using Poco::OSP::Properties;
using Poco::OSP::Service;
using Poco::OSP::ServiceRef;
using Poco::OSP::PreferencesService;
using Poco::AutoPtr;
using Poco::StringTokenizer;


class UrlPaService: public Poco::OSP::Auth::PaService
	/// A very simple implementation of AuthService.
	///
	/// Only two users are known - "user" and "admin". The
	/// names of these two users can be changed in the global 
	/// configuration using the "auth.simple.admin.name" and
	/// "auth.simple.user.name" properties and default to
	/// "admin" and "user", respectively.
	///
	/// The password for "admin" and "user" can be set in the global 
	/// application configuration file as salted MD5 hashes with the
	/// "auth.simple.admin.passwordHash" and "auth.simple.user.passwordHash" 
	/// properties. The (optional) salt can be specified with
	/// the "auth.simple.salt" property.
	/// 
	/// The "admin" user has all permissions. The set of permissions
	/// for "user" can be set in the global configuration file,
	/// using the "auth.simple.user.permissions" property. The permissions are 
	/// specified as a comma-separated list.
{
public:
	UrlPaService(const std::string& adminName, const std::string& adminPasswordHash, const std::string& userName, const std::string& userPasswordHash, const std::set<std::string>& userPermissions, const std::string& salt):
		_adminName(adminName),
		_adminPasswordHash(adminPasswordHash),
		_userName(userName),
		_userPasswordHash(userPasswordHash),
		_userPermissions(userPermissions),
		_salt(salt)
	{
		cerr << "UrlPaService constructor begin" <<	endl;
		_pTimer = new Poco::Util::Timer;			//added by sam 20171004 for timer receiving PA messages
		//_pTimer->schedule(new Poco::Util::TimerTaskAdapter<UrlPaService>(*this, &UrlPaService::reconnect), Poco::Timestamp());
		_pTimer->scheduleAtFixedRate(new Poco::Util::TimerTaskAdapter<UrlPaService>(*this, &UrlPaService::reconnect), 250, 5000);
		cerr << "UrlPaService constructor finish" <<	endl;
	}
	

	~UrlPaService()
	{
	}


	void reconnect(Poco::Util::TimerTask&)
	{
		cerr << "UrlPaService reconnect start" <<	endl;

		int err = 0;
		int index = 0;
		int len = 0;
		int timeout = 0;

		//sEtherProtocol etherProtocol;	TODO use struct or not
		BYTE   receiveMsg[4096]; // reception
		/***/
		int sockid = connectSocket();
		err = ReceiveDataAFC(
							index,
							sockid,
							(char*)receiveMsg,
							&len,
							timeout,
							'\n' //AFC_MESSAGE_END,
							);
		//cerr << receiveMsg << endl;
		/***/
		/**TODO <1>	calling the following to receive PA message and output to data
		*  int ReceiveDataAFC (
		*	int index,
		*	int soc,
		*	char data[],
		*	int *len,
		*	int timeOut,
		*	char end_marker,
		*	bool start_marked,
		*	char start_marker)
		*
		*	TODO <2> append the data[] to MQTT queue
		*	TODO <3> check when there is update of PA, WEBSOCKET the web client
		*	TODO <4> or the web client polling
		***/

		cerr << "UrlPaService reconnect finish" <<	endl;
	}
	
	// AuthService
	bool authenticate(const std::string& userName, const std::string& credentials) const
	{
		return (userName == _userName && hashCredentials(credentials) == _userPasswordHash)
		    || (userName == _adminName && hashCredentials(credentials) == _adminPasswordHash);
	}

	bool authorize(const std::string& userName, const std::string& permission) const
	{
		return userName == _adminName || (userName == _userName && _userPermissions.find(permission) != _userPermissions.end());
	}

	// Service
	const std::type_info& type() const
	{
		return typeid(UrlPaService);
	}
	
	bool isA(const std::type_info& otherType) const
	{
		std::string name(typeid(UrlPaService).name());
		return name == otherType.name() || Poco::OSP::Auth::PaService::isA(otherType);
	}

	//TODO by sam keyword: sockfd; reference from devices/Simulation/src/BundleActivator & SimulatedSensor
	/** by sam 20170807 originally
	std::string sendPaCommand(int nightMode) const */
	std::string sendPaCommand(int nightMode, const std::string& schedTime, const std::string& zoneCode, const std::string& msgCode, const std::string& weekday) const
	{
		int _sockfrd = connectSocket();
		sendPaMsg(_sockfrd, schedTime, zoneCode,msgCode,weekday);
		if(nightMode == 1){
			return "Inside UrlPa: sendPaCommand OFF";
		}else{
			return schedTime; //by sam 20170807 "Inside UrlPa: sendPaCommand ON";
		}
	}
	

	//added by sam 20170703 for adding socket to connect to PA server START
	int connectSocket() const{
		struct sockaddr_in dest;
		char buffer[128];

		/**create socket*/
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		/**initinalize value in dest**/
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(1100);
		dest.sin_addr.s_addr = inet_addr("128.13.109.246");//TODO by sam to get from parameter file

		/**connecting to server**/
		connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));

		return sockfd;
	}
	/** by sam 20170807
	void sendPaMsg(int _sockfd) const{ **/
	void sendPaMsg(int _sockfd, const std::string& schedTime, const std::string& zoneCode, const std::string& msgCode, const std::string& weekday) const{
		cerr << "LOG: Inside UrlPa:sendPaMsg BEGIN for time:" << schedTime << endl;
		int flag =1;
		setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

		//char paMsg[16]="0003C22FN100@71";
		//trial of message replay :
		//1) SMS connect to PA
		//2) Press Night On
		//3) start this server and this message sent to PA to trigger Night Off
		/**	Night Mode Model Msg:
		char paMsg[16]="0003C22FN100@71";	*/
		/** add schedule model message
		char paMsg[200]= "0033C01SA216:15:50	  :  :  	once	   	#009#02.06.05.04.05.04.04#                                   00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              @FB"; */

		char paMsg[200] = "0021C01SA312:30:00	  :  :  	once	   	#009#10.01#                                                  00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              @";

		/**
		 *	Changing of the time
		 *	Bit:11-18
		 *	according to the 8 characters "schedTime"
		 **/
		paMsg[9] = weekday.at(0); // 20170809 by sam changing the WEEKDAY from client

		//20170807 by sam changing the TIME from client according to the 8 characters: 00:00:00
		for(int schedIndex = 0; schedIndex< schedTime.length(); schedIndex ++){
			paMsg[10 + schedIndex ] = schedTime.at(schedIndex); //'2';
		}

		//20170809 by sam changing the ZONE from client
		for(int zoneIndex = 0; zoneIndex< zoneCode.length(); zoneIndex ++){
			paMsg[37 + zoneIndex ] = zoneCode.at(zoneIndex); //'2';
		}

		//201708091340 by sam changing the MESSAGE ID from client
		for(int msgIndex = 0; msgIndex< msgCode.length(); msgIndex ++){
			paMsg[41 + msgIndex ] = msgCode.at(msgIndex); //'2';
		}
/**
		paMsg[41+0] = '#'; // 41
		paMsg[41+1] = '1'; // 42
		paMsg[41+2] = '0'; // 43
		paMsg[41+3] = '.'; // 44
		paMsg[41+4] = '0'; // 45
		paMsg[41+5] = '3'; // 46
**/
		size_t len = strlen(paMsg);

		//TODO by sam to make the individual method to calculate the checksum
		 //by sam 20170804 START for calculating the checksum according to PPAddEndOfMessage
		 static char ConvhexToChar[] = {'0','1','2','3','4','5','6','7','8','9',
		 								'A','B','C','D','E','F'};

		 BYTE sum1 = 0;
		 BYTE sum2;
		 int i;

		 // check checksum
		 for (i=0;i<200-4;i++) sum1 += paMsg[i];
		 sum2 = ((BYTE)0xFF - sum1) + 1;

		 int firstDigit  = sum2 / 16;
		 int secondDigit = sum2 % 16;

		 paMsg[200-3 ] = ConvhexToChar[firstDigit];
		 paMsg[200-2 ] = ConvhexToChar[secondDigit];
		 paMsg[200-1 ] = '\n';

		 //by sam 20170804 FINISH for calculating the checksum according to PPAddEndOfMessage


		 //tmp by sam 20170807 cerr << "LOG: Inside UrlPa:sendPaMsg JUST SENT SCHEDULE ADD command of length: "<< len << " checksum:" << ConvhexToChar[firstDigit] << ConvhexToChar[secondDigit] << endl;


		send(_sockfd, paMsg, sizeof(paMsg), 0);
		//20171003	close(_sockfd);			//by sam comment if this is closed, client list of PA will not store duplicate entries

	}
	//added by sam 20170703 for adding socket to connect to PA server FINISH

	//added by sam 20170929 for a general receiving function
	int ReceiveDataAFC (
			int index,
			int soc,	/**SOCKET soc,**/
			char data[],
			int *len,
			int timeOut,
			char end_marker,
			bool start_marked = false,
			char start_marker = 0)
	{
		typedef unsigned char BYTE;

		u_long lBytesInSocket = 0;
		int err		= 0;
		*len = 0;  // nothing is read  for the moment

		 // we assume that the maximun length of a receive frame cannot exceed ETHER_LEN
		//char temporaryBuffer[ETHER_LEN];

		bool fStart = false;
		bool fEnd   = false;
		bool fOut   = false;
		bool fAuthorizeStart = !(start_marked);

		int nbByte = 0;
		BYTE ch;

		char buffer[512];
		do  // this loop add been introduce in case of the applicative frame is sent in many TCP/IP frame
		{
			// First check if a read would block
			//by sam 20170929	the timer will be set outside of this scope
			//by sam 20170929	err = WillReadBlockWithTimeOut (soc,timeOut);
			//by sam 20170929	if (err > 0)
			{

				err = recv(soc, (char*)&ch,1,0);						//TODO by sam why cannot this receive anything
				if (err > 0)
				{
					// search the START character if it exists
					if (((!fStart)&&(ch == (BYTE)start_marker))||(fAuthorizeStart))
					{
							// the start of the frame is found
							fStart = true;
							nbByte		 = 0;
							data[nbByte++] = ch;
	                        fAuthorizeStart = false;
					}
					else if ((fStart)&&(ch == (BYTE)end_marker))
					{
						data[nbByte++] = ch;
						fEnd = true;
					}
					else if (fStart)
					{
						data[nbByte++] = ch;
					}

				}
				else
				{
					fOut = true;
					//	_stprintf(szBuffer, __TEXT("%s : Disconnection by remote host"),
					//	ProjectContractor::getNameByContractor(ConfigEther[index].idContractor));
					//	Trace(szBuffer);
				}
			}
			/**
			else
			{
				fOut = true;
			}
			**/

			if (fEnd)
			{
				*len = nbByte;
				fOut = true;
			}
		}
		while (!fOut);	//tmp by sam 20171006 -> 20171009

		cerr << "ReceiveDataAFC function data:" << endl; 	//tmp by sam 20171006
		//added by sam 20171009 for debugging
		for(int i= 0; i < strlen(data); i++){
			cerr << data[i];
		}
		cerr << endl;
		//added by sam 20171009 for debugging

		return (fEnd)?(nbByte):0;
	}
	//added by sam 20170929


protected:
	std::string hashCredentials(const std::string& credentials) const
	{
		Poco::MD5Engine md5;
		md5.update(_salt);
		md5.update(credentials);
		return Poco::DigestEngine::digestToHex(md5.digest());
	}

private:
	std::string _adminName;
	std::string _adminPasswordHash;
	std::string _userName;
	std::string _userPasswordHash;
	std::set<std::string> _userPermissions;
	std::string _salt;

	Poco::SharedPtr<Poco::Util::Timer> _pTimer;	//added by sam 20171004 for timer receiving PA messages

	int AFC_LEN	= 4096; // Correction of the site bug 12/12/2003
	typedef unsigned char BYTE;
	typedef struct {

		bool   isTerminate;                 // to stop the PP threads
		BYTE   linkStatus;                  // link status : LINK_COM_OK or LINK_COM_HS
		bool   isConnect;

//	  	HANDLE hEventReceive;
//	  	HANDLE hMutex;

		// be carefull : use AFC_LEN instead of ETHER_LEN to define the greatest buffer
		BYTE   receiveMsg[4096]; // reception
		BYTE   sendMsg[4096];    // sending frame

//	  	HANDLE  hThreadSend;
//	  	HANDLE  hThreadReceive;

//	  	socket  soc;
//	  	SOCKET  socServer;
//	  	struct  sockaddr_in remoteAddr;

		// pointer to an object used by the contractor.
		void*   object;

	} sEtherProtocol;
};


class UrlPaBundleActivator: public BundleActivator
	/// The BundleActivator for the UrlPa.
	/// Registes the UrlPaService with the ServiceRegistry.
{
public:
	UrlPaBundleActivator()
	{
	}
	
	~UrlPaBundleActivator()
	{
	}
	
	void start(BundleContext::Ptr pContext)
	{
		ServiceRef::Ptr pPrefsRef = pContext->registry().findByName(PreferencesService::SERVICE_NAME);
		AutoPtr<PreferencesService> pPrefs = pPrefsRef->castedInstance<PreferencesService>();
		
		std::string adminName = pPrefs->configuration()->getString("auth.simple.admin.name", "admin");
		std::string adminPasswordHash = pPrefs->configuration()->getString("auth.simple.admin.passwordHash", ""); 
		std::string userName = pPrefs->configuration()->getString("auth.simple.user.name", "user");
		std::string userPasswordHash = pPrefs->configuration()->getString("auth.simple.user.passwordHash", "");
		std::string salt = pPrefs->configuration()->getString("auth.simple.salt", "");
		std::string perms = pPrefs->configuration()->getString("auth.simple.user.permissions", "");
		StringTokenizer tok(perms, ",;", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
		std::set<std::string> userPermissions;
		for (StringTokenizer::Iterator it = tok.begin(); it != tok.end(); ++it)
		{
			userPermissions.insert(*it);
		}
		
		AutoPtr<UrlPaService> pService = new UrlPaService(adminName, adminPasswordHash, userName, userPasswordHash, userPermissions, salt);
		_pService = pContext->registry().registerService("osp.urlpa", pService, Properties());
	}
		
	void stop(BundleContext::Ptr pContext)
	{
		pContext->registry().unregisterService(_pService);
	}
	

private:
	ServiceRef::Ptr _pService;
};


POCO_BEGIN_MANIFEST(BundleActivator)
	POCO_EXPORT_CLASS(UrlPaBundleActivator)
POCO_END_MANIFEST
