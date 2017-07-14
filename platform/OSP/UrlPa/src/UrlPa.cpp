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
	}
	
	~UrlPaService()
	{
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
	std::string sendPaCommand(int nightMode) const
	{
		int _sockfrd = connectSocket();
		sendPaMsg(_sockfrd);
		if(nightMode == 1){
			return "Inside UrlPa: sendPaCommand OFF";
		}else{
			return "Inside UrlPa: sendPaCommand ON";
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
		dest.sin_addr.s_addr = inet_addr("128.12.46.246");//TODO by sam to get from parameter file

		/**connecting to server**/
		connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));

		return sockfd;
	}

	void sendPaMsg(int _sockfd) const{
		cerr << "LOG: Inside UrlPa:sendPaMsg BEGIN "  << endl;
		int flag =1;
		setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

		//char paMsg[16]="0003C22FN100@71";
		//trial of message replay :
		//1) SMS connect to PA
		//2) Press Night On
		//3) start this server and this message sent to PA to trigger Night Off
		//tmp by sam 20170714 char paMsg[16]="0003C22FN100@71";
		//tmp by sam 20170714 paMsg[15] = '\n';
		/**
		 * TODO by sam 20170714 add schedule **/
		 //char paMsg[200]= "0033C01SA216:15:50	  :  :  	once	   	#009#02.06.05.04.05.04.04#                                   00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              @FB";
		 char paMsg[200]= "0021C01SA212:34:56	  :  :  	once	   	#009#10.01#                                                  00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              @FD";
		 paMsg[199] = '\n';
		 //char paMsg[213]= "0005C22SA100:00:00	  :  :  	once	   	#X0000000000380E00#02.01#                                    00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              P00000R000001@3B";
		 //char paMsg[213]= "0003C22SA100:00:00	  :  :  	once	   	#X0000000000380E00#02.01#                                    00NNNNNNNNNNNNNN	                                   	0000	0000	S000S                              P00000R000001@3D";
		 //paMsg[212] = '\n';
		 cerr << "LOG: Inside UrlPa:sendPaMsg JUST SENT SCHEDULE ADD command "  << endl;


		send(_sockfd, paMsg, sizeof(paMsg), 0);
		close(_sockfd);

	}
	//added by sam 20170703 for adding socket to connect to PA server FINISH


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
