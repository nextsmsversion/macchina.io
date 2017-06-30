//
// PaInterfaceService.h
//
// $Id: //poco/1.7/OSP/include/Poco/OSP/PaInterfaceService.h#1 $
//
// Library: OSP
// Package: PaInterfaceService
// Module:  PaInterfaceService
//
// Definition of the PaInterfaceService class.
//
// Copyright (c) 2007-2014, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#ifndef OSP_PaInterfaceServiceService_INCLUDED
#define OSP_PaInterfaceServiceService_INCLUDED


#include "Poco/OSP/OSP.h"
#include "Poco/OSP/Service.h"
#include "Poco/OSP/Bundle.h"
#include <istream>


namespace Poco {
namespace OSP {


class BundleRepository;


class OSP_API PaInterfaceService: public Service
	/// The PaInterfaceService allows a bundle to install
	/// other bundles, either from a stream or from a
	/// URI.
	///
	/// The service name of the PaInterfaceService
	/// is "osp.core.installer".
{
public:
	typedef Poco::AutoPtr<PaInterfaceService> Ptr;
	typedef const Ptr ConstPtr;


	PaInterfaceService();

	PaInterfaceService(BundleRepository& repository);
		/// Creates the PaInterfaceService, using
		/// the given BundleRepository.

	Bundle::Ptr installBundle(const std::string& uri);
		/// Installs a bundle from the given URI.
		/// The Poco::URIStreamOpener class is used to
		/// open a stream for the given URI.
		///
		/// Returns a pointer to the installed bundle.

	Bundle::Ptr installBundle(std::istream& istr);
		/// Installs a bundle from the given stream.
		///
		/// Returns a pointer to the installed bundle.

	Bundle::Ptr replaceBundle(const std::string& symbolicName, std::istream& istr);
		/// Upgrades or replaces a bundle from the given stream.
		///
		/// Returns a pointer to the installed bundle.

	Bundle::Ptr replaceBundle(const std::string& symbolicName, const std::string& uri);
		/// Upgrades or replaces a bundle from the given URI.
		/// The Poco::URIStreamOpener class is used to
		/// open a stream for the given URI.
		///
		/// Returns a pointer to the installed bundle.
		
	static const std::string SERVICE_NAME;

	// Service
	const std::type_info& type() const;
	bool isA(const std::type_info& otherType) const;
	bool sendPassage();
protected:
	~PaInterfaceService();
		/// Destroys the PaInterfaceService.
};


} } // namespace Poco::OSP


#endif // OSP_PaInterfaceService_INCLUDED
