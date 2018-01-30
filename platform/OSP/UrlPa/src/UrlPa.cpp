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
#include "SmsPaDefine.cpp"
#include <set>

//added by sam 20171010 START for adding push notification
#include <algorithm>
#include <stdexcept>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
//added by sam 20171010 FINISH


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


class SmsFilePaMgr
{

public :
	//TODO by sam static bool ReadZoneConfig( const char *fileName ) ;
	static bool ReadMsgConfig( const char *fileName );
	//TODO by sam static bool ReadInstantMsgConfig( const char *fileName ) ;
	//TODO by sam static bool ReadMhqueue( const char *fileName ) ;
	//TODO by sam static bool ReadSchedule( const char *fileName, long schedDay ) ;

private :
	static bool AnalyseMsgConfigLine( string line, long msgConfigSection ) ;
	static bool AnalyseMsgTypeLine( string line ) ;
	//TODO by sam static bool AnalyseMsgVarLine( RWCString line ) ;
	//TODO by sam static bool AnalysePredefMsgLine( RWCString line, long predefMsgLineType ) ;
	//TODO by sam static bool AnalysePredefMsgLineWithoutVar( RWCString line ) ;
	//TODO by sam static bool AnalysePredefMsgLineWithVar( RWCString line ) ;
	//TODO by sam static bool AnalysePredefMsgLineWithVarEnd( RWCString line ) ;

	//TODO by sam static void SendCommand( long cmdId, IlsMvValue param1 = IlsMvValue(), IlsMvValue param2 = IlsMvValue(),
	//TODO by sam 	IlsMvValue param3 = IlsMvValue(), IlsMvValue param4 = IlsMvValue(), IlsMvValue param5 = IlsMvValue(),
	//TODO by sam 	IlsMvValue param6 = IlsMvValue(), IlsMvValue param7 = IlsMvValue(), IlsMvValue param8 = IlsMvValue() ) ;

	//TODO by sam static bool BuildMask( const char *physicalAddressLine, RWCString &mask, int &physicalAddress ) ;
	//TODO by sam static RWCString ReadTitle( RWCString &text ) ;
	//TODO by sam static bool DecodeMsgCode( const char *msgCode, RWCString &zoneAddress, bool &isPhysicalAddress, RWCString &messageField ) ;

	//------------------------------------------------------------------------------------------------------------------
	//#BEGIN
	//#ATTRIBUT static long OpenSection
	//#COMMENT Part of Pa database file for which the 'transaction' has been open.
	//#END
	//------------------------------------------------------------------------------------------------------------------
	//static long OpenSection ;

	//------------------------------------------------------------------------------------------------------------------
	//#BEGIN
	//#ATTRIBUT static long LastPredefMsgLineType
	//#COMMENT Type of last predefined message line successfully analysed.
	//#END
	//------------------------------------------------------------------------------------------------------------------
	//static long LastPredefMsgLineType ;
} ;

//----------------------------------------------------------------------------------------------------------------------
//#BEGIN
//#METHOD bool SmsFilePaMgr::ReadMsgConfig( const char *fileName )
//#COMMENT This method is used to read Pa database file. This file is devided into three part : message types
// description part, message variables description part and predefined messages part. A command is sent to server for
// each message types, message variables or predefined messages described in file. These commands are sent in three
// different 'transactions'. For each 'transaction', a first command is sent to start transaction, then commands are
// sent and a last command is sent to close transaction. The aim of these transactions is to know which message types,
// message variables and predefined messages are no more described in Pa database file and must be deleted.
//#END
//----------------------------------------------------------------------------------------------------------------------
bool SmsFilePaMgr::ReadMsgConfig( const char *fileName )
{
	printf("SmsFilePaMgr::ReadZoneConfig");

	// Open zone configuration file
    ifstream input( fileName ) ;
	if ( !input )
	{
		cerr << "*** Cannot open file %s ***", fileName  ;
		return false ;
	}

	else{
		cerr << "*** SmsFilePaMgr::ReadMsgConfig ifstream input file successfully" ;
	}

	long OpenSection = PA_SECTION_MSG_TYPE ;
	long LastPredefMsgLineType = PA_MSG_LINE_WITHOUT_VAR ;

	string line;

	while (std::getline(input, line)){


		if ( !AnalyseMsgConfigLine( line, OpenSection ) )
		{
			cerr << "*** Syntax error ***" ;
			cerr << "Analysed line : " << line << std::endl;
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
//#BEGIN
//#METHOD bool SmsFilePaMgr::AnalyseMsgConfigLine( RWCString line, long msgConfigSection )
//#COMMENT This method is used to analyse Pa database file line. This line will be analysed as if it belongs to opened
// section (current transaction). If it does not fit such line, it will be analysed as if it belongs to following
// sections. If it still does not succeed in any following sections, this line will be thought to have a syntax error
// and next line will be analysed (as opened section line). If it succeeds in a following section, opened section will
// be closed and new section will be opened (new transaction).
// NB : Section order is : 1. message types,
//                         2. message variables,
//                         3. predefined messages.
//#END
//----------------------------------------------------------------------------------------------------------------------
bool SmsFilePaMgr::AnalyseMsgConfigLine( string line, long msgConfigSection )
{
	bool success ;

	// Switch on section this line is supposed to belong to
	switch ( msgConfigSection )
	{
		case PA_SECTION_MSG_TYPE :
			success = AnalyseMsgTypeLine( line ) ;
			break ;

/**
 * 		traditionally, it becomes object "msg = new SmsPaPredefMsg( msgId, msgName, firstPartText, firstPartVar ) ;"
 * 		ILS_RP_ATTR_STRING( SmsRpPaMsgPart, msgPartText, setText )
 *		next step is to make it a rest API w.r.t.
 *		TODO 20180130
 *		1) getPaStatusJson
 *		2) serviceRegistry.find('io.macchina.instancemsg != ""') & devices.jss : response.write(JSON.stringify(devices));
 *
 *
 *
 *
		case PA_SECTION_MSG_VAR :
			success = AnalyseMsgVarLine( line ) ;
			break ;

		case PA_SECTION_PREDEF_MSG :
			success = AnalysePredefMsgLine( line, LastPredefMsgLineType ) ;
			break ;
**/
		default :
			cerr << "Invalid message configuration file section" << std::endl;
			success = false ;
			break ;
	}

	return true ;
}

//----------------------------------------------------------------------------------------------------------------------
//#BEGIN
//#METHOD bool SmsFilePaMgr::AnalyseMsgTypeLine( RWCString line )
//#COMMENT Analyse line as message type description line. If it succeeds, send message type creation command to server
// application. If it fails, try to analyse this same line as message variable description line.
//#END
//----------------------------------------------------------------------------------------------------------------------
bool SmsFilePaMgr::AnalyseMsgTypeLine( string line )
{
	//cerr <<	"SmsFilePaMgr::AnalyseMsgTypeLine" << std::endl;

	// Retrieve message type description line format

	string format = "%2d%*1[	]%200c" ; //TODO SmsUpdatePa::Get()->getParameters()->getMsgTypeLineFormat() ;
	int msgTypeId = 0 ;
	char msgTypeName[256] ;
	memset( msgTypeName, 0, sizeof( msgTypeName ) ) ;



	// Analyse line as message type line
	if ( sscanf( line.data(), format.data(), &msgTypeId, msgTypeName ) < 2 )
	{
		//cerr <<	"Cannot comprehend: " << line << std::endl;
		return false;
		//TODO : AND how to remove the recursive function return AnalyseMsgConfigLine( line, PA_SECTION_MSG_VAR ) ;
	}

	// Checks message type id validity
	if ( ( msgTypeId < PA_MIN_MSG_TYPE_ID )||( msgTypeId > PA_MAX_MSG_TYPE_ID ) )	//PA_MAX_MSG_TYPE_ID: 10 exclude INSTANT Msg
	{
		cerr << "*** Invalid message type id %d ***" << msgTypeId  << std::endl;
		return false ;
	}

	cerr << std::endl;
	cerr << "<" << msgTypeId << ">"<< " MESSAGE TYPE LINE: " << msgTypeName << std::endl;//tmp by sam
	/**	TODO output as RESTful API
	RWCString type = msgTypeName ;
	SmsFileAnalyser::ExtractSpaces( type ) ;

	// Create and send command
	SendCommand( PA_CMD_MSG_TYPE_CREATION, type.data(), msgTypeId ) ;

	trace.printn( "Create message type %s : <id>%d", type.data(), msgTypeId ) ;

	**/
	return true ;
}


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
		_pTimer->scheduleAtFixedRate(new Poco::Util::TimerTaskAdapter<UrlPaService>(*this, &UrlPaService::reconnect), 250, 1000);
		cerr << "UrlPaService constructor finish" <<	endl;
	}
	

	~UrlPaService()
	{
	}

/**
 * Function
 * 	@name: 	reconnect(Poco::Util::TimerTask&)
 * 	@usage:	update at a fixed rate in the thread, i.e. UrlPaService::reconnect
 *	@call:	ReceiveDataAFC		:	get the status update from PA server through string : 0000NS000000000000000000000000000000000000xxxx
 *	@call:	setNightModeValue	:	update the status of PA night mode to the MMI
 *	@call: 	getFile				:	FTP the latest PA text file: instant.txt, database.txt
 *	@TODO :		-instant message: 	"props.set"
 *	@step:	decoding the file and get the message (1. instant.txt	2. database.txt [PA_SECTION_PREDEF_MSG])
 *	@step:	decoding the file	:	SmsFilePaMgr::AnalyseMsgConfigLine
 * */
	void reconnect(Poco::Util::TimerTask&)
	{
		cerr << "UrlPaService reconnect start" <<	endl;

#define PA_ReadSchedule


#ifdef PA_ReadSchedule
			//TODO get the PA schedule file
			//TODO decode the file				SmsFilePaMgr::DecodeMsgCode
			//TODO map to the PA zone
			//TODO update to the MMI (Start Time , message #10.01#, comment )	SmsPa::mhqueueLineCreation -> SmsPa::getMsg

			/**	SW_Requirement_Specification
			 * 	7	description of Scheduler file
			 *				startTime	EndTime		repetitionField
			 *													Message Code
			 *													zoneAddress
			 *														messageField
			 *														        PIDSCode[16]	    Comment[35]                                          Comment[35]?                   RepID	SLineID
			 *																																														Station Code
			 */

			string localfile = "/Users/sms/msgConfig.txt", remotefile = "PAServer/config/database.txt";
			//tmp by sam without network getFile(localfile, remotefile, "anonymous", "sms", "128.13.22.246");
			if(!SmsFilePaMgr::ReadMsgConfig(localfile.c_str())){
				cerr <<  "*** Read message config error ***" << endl;
			}
/**
 * 			TODO
 * 			read the file						:	SmsFilePaMgr::ReadMsgConfig( const char *fileName )

 * 			decode and store the file			:	SmsFilePaMgr::AnalyseMsgConfigLine
 			01	Select
			02	Service
			04	Emer
			05	Last
			06	Auto
			07	Courtesy
			09	Crowd
			10	Instant
			#01     01C	.
			#01     01E	one

 *			mapping of the above msgConfigLine	:	SmsPa::mhqueueLineCreation
  			string mhQueue1		=	"14:56:31	4078	ID	22	160	#X0000000000000007#07.31#                                    	00NNNNNNNNNNNNNN	S000S                              	R99P300";
 			string mhQueue2		= 	"14:56:53	4080	ID	22	160	#X0000000000000007#07.23#                                    	00NNNNNNNNNNNNNN	S000S                              	R99P300";
 			string mhQueue3		=	"14:55:38	4084	ID	22	140	#X0000000000000007#10.87#                                    	00NNNNNNNNNNNNNN	S000S                              	R99P300";
 			string mhQueue4		=	"14:56:05	4088	ID	22	140	#X0000000000000007#10.97#                                    	00NNNNNNNNNNNNNN	S000S                              	R99P300";

**/
			string scheduleline = 	"12:34:56	  :  :  	once	   	#009#10.01#                                                  00NNNNNNNNNNNNNN	                                   	0000	4355	S000S                              ";
			/**
			 * TODO					"Time					Loop		Zone;Msg Name: (05) Last   (01) Last train at platform (01) +ONE is the last train for (07) Mei Foo, board immediately
			string todoScheLine = 	"00:34:00	  :  :  	once	   	#099#05.01.01.07#                                            00NNNNNNNNNNNNNN	                                   	0000	9025	S000S                            ";
			 */

			string scheduleformat = "%8c%*1[	]%*8c%*1[	]%*4c%*1[	]%*3c%*1[	]%61c%*16c%*1[	]%35c%*1[	]%4d%*1[	]%4d%*1[	]%*35c";

			char startTime[256] ;
			char msgCode[256] ;
			bool isPhysicalAddress ;
			char commentStr[256] ;
			int repetitionId ;
			int lineId ;

			//int returnValue = sscanf( line.data(), format.data(), msgType, &msgId, &msgPart, chineseOrEnglish, &varType, msgText );
			int resSscanf = sscanf( scheduleline.data(), scheduleformat.data(), startTime, msgCode, commentStr, &repetitionId, &lineId ) ;

			printf("<%s>, <%s>, <%s>, <%d>, <%d>: return %d\n", startTime, msgCode, commentStr, &repetitionId, &lineId, resSscanf);

#endif




		int err = 0;
		int index = 0;
		int len = 0;
		int timeout = 0;

		//sEtherProtocol etherProtocol;
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


		//TODO <2>	void SmsPa::readStatus( RWCString status ) put the following into function
		//following is just a case, needs to be generalize and write re-usable function like SmsPa::readStatus
		//TODO return "{\"night\":1}"
		cerr << "receiveMsg string: " << receiveMsg[199+5] << endl;
		setNightModeValue(((int)receiveMsg[199+5])-48);	//ASCII code : start from 48

		//TODO <3> check when there is update of PA, web client subscribe/WEBSOCKET the MQTT
		//OR	TODO <4> or the web client polling


		/***/
		/** DONE <1> 201710091600	SMS 2.0 PA log ReceiveDataAFC
		*	NONO <2> append the data[] to mosquitto MQTT queue w.r.t. "update->updateCallback" in SMS source code
		*	0000NS00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000030003100000010110000000000000000000031101111000000001120000000000010000000000000000000000000000000000000000000000@57
		*	0000NS00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000030003100000010110000000000000000000031101111000000001110000000000010000000000000000000000000000000000000000000000@58
		*OR	DONE <2> 201710121200	publish to REST API w.r.t. bool SmsCmdMdlPaMgr::buildCommand			Notification CONFIGURATION CHANGED	0000NC000200@8D
		*OR	REFI <2> void SmsPa::readStatus( RWCString status )	Notification STATUS CHANGED ABOVE
		*	TODO <2> display of the courtesy message by decode message of the database.txt w.r.t. bool SmsFilePaMgr::ReadMsgConfig( const char *fileName ) @updatePA
		*	TODO <3> check when there is update of PA, web client subscribe/WEBSOCKET the MQTT
		*OR	TODO <4> or the web client polling
		*	TODO <5> in case of status for updating configuration file database.txt, instant.txt
		*	TODO <6> FTP and read the above configuration file w.r.t. SmsFilePaMgr::ReadMsgConfig( const char *fileName )
		*	TODO <6> Store the message w.r.t. SmsPa::endMsgTypeConfig()		:	list.insert( msgType ) ;
		*	TODO <7> how to display in the MMI	:	w.r.t. SmsPanelPa::editSelectorCB	//instant messages editor
		*
		***/

//#define PA_SECTION_MSG_TYPE_CODE
//#define PA_SECTION_MSG_VAR_CODE
#define PA_SECTION_PREDEF_MSG_CODE	//tmp by sam 20180124


#if defined(PA_SECTION_MSG_TYPE_CODE) || defined(PA_SECTION_MSG_VAR_CODE) || defined(PA_SECTION_PREDEF_MSG_CODE)

#endif

		if(true){ //TODO <5> in case of status receiveMsg[??] for updating configuration file
			string localfilename = "/Users/sms/aDatabase.txt", remotefilename = "PAServer/config/database.txt";
			getFile(localfilename, remotefilename, "anonymous", "sms", "128.13.22.246");

//TODO by sam to generalize the following as functions


#ifdef PA_SECTION_MSG_TYPE_CODE
			/**
			string line	= "07	Courtesy";	**/							//PA_SECTION_MSG_TYPE	SmsPa::msgTypeCreation
			string line = "09	Crowd";
#endif
#ifdef PA_SECTION_MSG_VAR_CODE
			string line 	= "#03     01E	this";						//PA_SECTION_MSG_VAR
#endif
#ifdef PA_SECTION_PREDEF_MSG_CODE
			/**
			string line 	= "Last    01.2E #03	is the last train for";	//PA_SECTION_PREDEF_MSG	: 	SmsPa::predefMsgPartCreation
			string line 	= "Service 05.1E #03	Reduced service due to incident at a station: Your attention please, Train service to";	**/
			string line		= "Crowd   02.1E #14	No blocking in concourse or on platform: Your attention please, Please do not wait";
#endif


			//TODO	SmsUpdatePa::Get()->getParameters()->getMsgTypeLineFormat() ;
#ifdef PA_SECTION_MSG_TYPE_CODE
			string format = "%2d%*1[	]%200c";				//PA_SECTION_MSG_TYPE
#endif
#ifdef PA_SECTION_MSG_VAR_CODE
			/** by sam originally
			string format	= "%8c%2d%[CE]%*1[	]%200c";		//PA_SECTION_MSG_VAR	: msgWithoutVarLineFormat **/
			string format	= "#%2d%*1[     ]%2d%[CE]%*1[	]%200c";		//PA_SECTION_MSG_VAR	: msgWithoutVarLineFormat
#endif
#ifdef PA_SECTION_PREDEF_MSG_CODE
			string format 	= "%8c%2d.%1d%[CE]%*1[ ]#%2d%*1[	]%200c";	//PA_SECTION_PREDEF_MSG	: msgWithVarLineFormat
			/**
			string format 	= "%8c%2d.%1d%[CE]%*1[	]%200c"; 				//						: msgWithVarEndLineFormat**/
#endif


#ifdef PA_SECTION_MSG_TYPE_CODE
			//PA_SECTION_MSG_TYPE
			int msgTypeId = 0;
			char msgTypeName[256] ;
			memset( msgTypeName, 0, sizeof( msgTypeName ) ) ;
#endif
#ifdef PA_SECTION_MSG_VAR_CODE
			//PA_SECTION_MSG_VAR
			int varTypeId = 0 ;
			int varId = 0 ;
			char chineseOrEnglish[256] ;
			memset( chineseOrEnglish, 0, sizeof( chineseOrEnglish ) ) ;
			char varText[256] ;
			memset( varText, 0, sizeof( varText ) ) ;
#endif
#ifdef PA_SECTION_PREDEF_MSG_CODE
			//PA_SECTION_PREDEF_MSG	: msgWithVarEndLineFormat
			char msgType[256] ;
			memset( msgType, 0, sizeof( msgType ) ) ;
			int msgId = 0 ;
			int msgPart = 0 ;
			char chineseOrEnglish[256] ;
			memset( chineseOrEnglish, 0, sizeof( chineseOrEnglish ) ) ;
			int varType = 0 ;
			char msgText[256] ;
			memset( msgText, 0, sizeof( msgText ) ) ;
#endif
			// Analyse line as message type line	by int sscanf ( const char * s, const char * format, ...);
#ifdef PA_SECTION_MSG_TYPE_CODE
			int returnValue = sscanf(line.c_str(), format.c_str(), &msgTypeId, msgTypeName);						//PA_SECTION_MSG_TYPE
#endif
#ifdef PA_SECTION_MSG_VAR_CODE
			int returnValue = sscanf( line.data(), format.data(), &varTypeId, &varId, chineseOrEnglish, varText );	//PA_SECTION_MSG_VAR
#endif
#ifdef PA_SECTION_PREDEF_MSG_CODE
			int returnValue = sscanf( line.data(), format.data(), msgType, &msgId, &msgPart, chineseOrEnglish, &varType, msgText );
#endif

			cerr << line << endl;
#ifdef PA_SECTION_MSG_TYPE_CODE
			printf("%s -> %d return %d\n", msgTypeName, msgTypeId, returnValue);	//PA_SECTION_MSG_TYPE
#endif
#ifdef PA_SECTION_MSG_VAR_CODE
			printf("%d, %d, %s, %s return %d\n", varTypeId, varId, chineseOrEnglish, varText, returnValue);	//PA_SECTION_MSG_VAR
#endif
#ifdef PA_SECTION_PREDEF_MSG_CODE
			printf("%s, %d, %d, %s, %d: <%s> return %d\n", msgType, msgId, msgPart, chineseOrEnglish, varType, msgText, returnValue);	//PA_SECTION_MSG_VAR
#endif
//TODO by sam to generalize the following as functions

		}

		//201710071703
		close(sockid);
		cerr << "UrlPaService reconnect finish" <<	endl;
	}
	
	void setNightModeValue(int tmpNightModeStatus){
		std::stringstream ss;
		ss << tmpNightModeStatus;
		nightModeValue = ss.str();
	}
	//TODO change to this function
	std::string getPaStatusJson() const{
		char jsonStr[80];
		std::strcpy(jsonStr, "{\"night\":");	//"{\"night\":1}"
		std::strcat(jsonStr, nightModeValue.c_str());
		std::strcat(jsonStr, "}");
		return jsonStr;
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
		dest.sin_addr.s_addr = inet_addr("128.13.22.246");//TODO by sam to get from parameter file

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
		//201710071702
		close(_sockfd);			//by sam comment if this is closed, client list of PA will not store duplicate entries

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

//by sam 20171010 START
		if (_pContext->logger().information())	//by sam 20170623 request from extensions.xml -> BundleActionsRequestHandlerFactory
		{
			_pContext->logger().information("ReceiveDataAFC function data:"); //by sam
			//string paMsg(data);
			_pContext->logger().information(data);
		}
//by sam 20171010 FINISH

		//added by sam 20171009 for debugging

		return (fEnd)?(nbByte):0;
	}
	//added by sam 20170929

	//added by sam 20171019 START

	/*** getFile -> SmsUpdatePa::ThreadUpdateFtp //COPY from remote to local file path
	 * reference from
	 * https://git.sch.bme.hu/kk1205/raspberrycloud/blob/cachemeres/program/Source/cloud/FTPAdapter.cpp
	 * @param	string localfilename	"/Users/sms/a.txt",
	 * 			string remotefilename	"a.txt"
	 * 			string USERNAME 		"ftpuser";
	 * 			string PASSWORD 		"ftp123456";
	 * 			string HOST 			"128.13.22.246";
	 */
		bool getFile(	string localfilename, string remotefilename,
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


	//added by sam 20171019 FINISH


	BundleContext::Ptr _pContext;//by sam 20171010

protected:
	std::string hashCredentials(const std::string& credentials) const
	{
		Poco::MD5Engine md5;
		md5.update(_salt);
		md5.update(credentials);
		return Poco::DigestEngine::digestToHex(md5.digest());
	}

private:
	std::string nightModeValue;	//by sam 20171013

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
		pService->_pContext = pContext;	//by sam 20171010
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
