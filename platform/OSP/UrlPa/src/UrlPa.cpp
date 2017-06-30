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