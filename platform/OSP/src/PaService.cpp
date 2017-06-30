//
// PaService.cpp
//
// $Id: //poco/1.7/OSP/src/PaService.cpp#1 $
//
// Library: OSP
// Package: Service
// Module:  PaService
//
// To be used by BundleActionsRequestHandler.cpp -> later rename to PaRequestHandler
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "Poco/OSP/Pa/PaService.h"
#include "Poco/OSP/ServiceFactory.h"


namespace Poco {
namespace OSP {
namespace Auth {


PaService::PaService()
{
}


PaService::~PaService()
{
}


const std::type_info& PaService::type() const
{
	return typeid(PaService);
}


	
bool PaService::isA(const std::type_info& otherType) const
{
	std::string name(typeid(PaService).name());
	return name == otherType.name() || Service::isA(otherType);
}

/**
std::string PaService::sendPaCommand(int nightMode) const{
	return "Inside PaService::sendPaCommand OFF";
}
**/
} } } // namespace Poco::OSP::Auth
