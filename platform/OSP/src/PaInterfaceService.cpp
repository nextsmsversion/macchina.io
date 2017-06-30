//
// PaInterfaceService.cpp
//
// $Id: //poco/1.7/OSP/src/PaInterfaceService.cpp#1 $
//
// Library: OSP
// Package: PaInterfaceService
// Module:  PaInterfaceService
//
// Copyright (c) 2007-2014, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "Poco/OSP/PaInterfaceService.h"
#include "Poco/OSP/BundleRepository.h"
#include "Poco/URIStreamOpener.h"
#include <memory>


using Poco::URIStreamOpener;


namespace Poco {
namespace OSP {


const std::string PaInterfaceService::SERVICE_NAME("osp.core.painterface");



PaInterfaceService::PaInterfaceService()
{

}

PaInterfaceService::~PaInterfaceService()
{
}



const std::type_info& PaInterfaceService::type() const
{
	return typeid(PaInterfaceService);
}


bool PaInterfaceService::isA(const std::type_info& otherType) const
{
	std::string name(typeid(PaInterfaceService).name());
	return name == otherType.name() || Service::isA(otherType);
}
bool PaInterfaceService::sendPassage(){
	//std::cout << "Inside PaInterfaceService::sendPassage" << std::end;
	return true;
}

} } // namespace Poco::OSP
