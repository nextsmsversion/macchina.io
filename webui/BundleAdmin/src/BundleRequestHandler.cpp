//
// BundleRequestHandler.cpp
//
// $Id$
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "BundleRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/OSP/Web/WebSession.h"
#include "Poco/OSP/Web/WebSessionManager.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/Auth/AuthService.h"
#include "Poco/NumberParser.h"
#include "Utility.h"

#include "Poco/OSP/Pa/PaService.h"	//by sam 20171010

namespace IoT {
namespace Web {
namespace BundleAdmin {


BundleRequestHandler::BundleRequestHandler(Poco::OSP::BundleContext::Ptr pContext):
	_pContext(pContext)
{
}

/**
 * TODO by sam to handle the request as follows:
 * @link
 * http://localhost:22080/macchina/bundles/bundle.json?symbolicName=com.appinf.osp.js.data
 * @return
 * {"id":1,"symbolicName":"com.appinf.osp.js.data","name":"OSP JavaScript Data Module","version":"1.0.0","state":"active","vendor":"Applied Informatics","copyright":"(c) 2016, Applied Informatics Software Engineering GmbH","runlevel":"600","path":"/Users/sms/nextsmsversion/macchina.smsv2/platform/OSP/bundles/com.appinf.osp.js.data_1.0.0.bndl","requires": [{"symbolicName":"com.appinf.osp.js","versions":"[1.0.0,2.0.0)"},{"symbolicName":"poco.data","versions":"[1.0.0,2.0.0)"}],"requiredBy": []}
 */

void BundleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	Poco::OSP::Web::WebSession::Ptr pSession;
	{
		Poco::OSP::ServiceRef::Ptr pWebSessionManagerRef = context()->registry().findByName(Poco::OSP::Web::WebSessionManager::SERVICE_NAME);
		if (pWebSessionManagerRef)
		{
			Poco::OSP::Web::WebSessionManager::Ptr pWebSessionManager = pWebSessionManagerRef->castedInstance<Poco::OSP::Web::WebSessionManager>();
			pSession = pWebSessionManager->find(context()->thisBundle()->properties().getString("websession.id"), request);
		}
	}
	if (!Utility::isAuthenticated(pSession, response)) return;

	std::string username = pSession->getValue<std::string>("username");
	Poco::OSP::Auth::AuthService::Ptr pAuthService = Poco::OSP::ServiceFinder::findByName<Poco::OSP::Auth::AuthService>(context(), "osp.auth");

	if (!pAuthService->authorize(username, "bundleAdmin"))
	{
		response.setContentLength(0);
		response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
		response.send();
		return;
	}

	Poco::OSP::Bundle::Ptr pBundle;
	Poco::Net::HTMLForm form(request, request.stream());
	if (form.has("id"))
	{
		int id;
		if (Poco::NumberParser::tryParse(form.get("id", ""), id))
		{
			pBundle = context()->findBundle(id);
		}
		else
		{
			response.setContentLength(0);
			response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.send();
			return;
		}
	}
	else if (form.has("symbolicName"))
	{
		pBundle = context()->findBundle(form.get("symbolicName"));
	}
	//added by sam 20171010
	else if (form.has("paservice")){
		Poco::OSP::Auth::PaService::Ptr pPaService = Poco::OSP::ServiceFinder::findByName<Poco::OSP::Auth::PaService>(context(), "osp.urlpa");
		context()->logger().information("TODO return response.send() from pPaService->ReceiveDataAFC()");
		response.setChunkedTransferEncoding(true);
			response.setContentType("application/json");
			std::ostream& ostr = response.send();

			ostr << "paservice night mode :" << pPaService->getNightModeStatus() ;
			return;
	}
	//added by sam 20171010

	if (!pBundle)
	{
		response.setContentLength(0);
		response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
		response.send();
		return;
	}

	response.setChunkedTransferEncoding(true);
	response.setContentType("application/json");
	std::ostream& ostr = response.send();
	
	ostr
		<< "{"
		<< "\"id\":" << pBundle->id() << ","
		<< "\"symbolicName\":" << Utility::jsonize(pBundle->symbolicName()) << ","
		<< "\"name\":" << Utility::jsonize(pBundle->name()) << ","
		<< "\"version\":" << Utility::jsonize(pBundle->version().toString()) << ","
		<< "\"state\":" << Utility::jsonize(pBundle->stateString()) << ","
		<< "\"vendor\":" << Utility::jsonize(pBundle->vendor()) << ","
		<< "\"copyright\":" << Utility::jsonize(pBundle->copyright()) << ","
		<< "\"runlevel\":" << Utility::jsonize(pBundle->runLevel()) << ","
		<< "\"path\":" << Utility::jsonize(pBundle->path()) << ","
		<< "\"requires\": [";

	const Poco::OSP::BundleManifest::Dependencies& deps = pBundle->requiredBundles();
	for (Poco::OSP::BundleManifest::Dependencies::const_iterator itDep = deps.begin(); itDep != deps.end(); ++itDep)
	{
		if (itDep != deps.begin()) ostr << ",";
		ostr 
			<< "{"
			<< "\"symbolicName\":" << Utility::jsonize(itDep->symbolicName) << ","
			<< "\"versions\":" << Utility::jsonize(itDep->versions.toString())
			<< "}";
	}
	ostr
		<< "],"
		<< "\"requiredBy\": [";

	BundleVec depending;
	dependingBundles(pBundle, depending, false);
	for (BundleVec::const_iterator itDep = depending.begin(); itDep != depending.end(); ++itDep)
	{
		if (itDep != depending.begin()) ostr << ",";
		ostr 
			<< "{"
			<< "\"symbolicName\":" << Utility::jsonize((*itDep)->symbolicName()) << ","
			<< "\"versions\":" << Utility::jsonize((*itDep)->version().toString())
			<< "}";
	}

	ostr
		<< "]"
		<< "}";
}


void BundleRequestHandler::dependingBundles(Poco::OSP::Bundle::Ptr pBundle, BundleVec& dependingBundles, bool runningOnly)
{
	BundleVec bundles;
	context()->listBundles(bundles);
	for (BundleVec::const_iterator it = bundles.begin(); it != bundles.end(); ++it)
	{
		if (!runningOnly || (*it)->isStarted())
		{
			const Poco::OSP::BundleManifest::Dependencies& deps = (*it)->requiredBundles();
			for (Poco::OSP::BundleManifest::Dependencies::const_iterator itDep = deps.begin(); itDep != deps.end(); ++itDep)
			{
				if (itDep->symbolicName == pBundle->symbolicName())
				{
					dependingBundles.push_back(*it);
					break;
				}
			}
		}
	}
}


Poco::Net::HTTPRequestHandler* BundleRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	return new BundleRequestHandler(context());
}


} } } // namespace IoT::Web::BundleAdmin
