if (!session || !session.authenticated)
{
	logger.error("No session or session not authenticated.");
	response.setStatus(401);
	response.send();
	return;
}

var devices = [];

/** TODO instant message from 8 to 100 **/
/** var deviceRefs = serviceRegistry.find('io.macchina.device != ""');	**/
var deviceRefs = serviceRegistry.find('io.macchina.instancemsg != ""');
for (var i = 0; i < deviceRefs.length; i++)
{
	var deviceRef = deviceRefs[i];
	var device = deviceRef.instance();
	if (device)
	{ 
		var deviceInfo = {};
		deviceInfo.id = parseInt(deviceRef.name);
		deviceInfo.name = device.getPropertyString("name");
		deviceInfo.symbolicName = device.getPropertyString("symbolicName");
		
		deviceInfo.PaMessage = "1"; //while no device.hdop();	//by sam 200170619 
		/**"PaMessage" **/
		device.sentPaMessage("PA");
		if (device.hasProperty("PaMessage"))
		{
			deviceInfo.PaMessage = device.getPropertyString("PaMessage");
		}
		if (device.hasProperty("physicalQuantity"))
		{
			deviceInfo.physicalQuantity = device.getPropertyString("physicalQuantity");
		}
		else
		{
			deviceInfo.physicalQuantity = "";
		}
		if (device.hasProperty("physicalUnit"))
		{
			deviceInfo.physicalUnit = device.getPropertyString("physicalUnit");
		}
		else
		{
			deviceInfo.physicalUnit = "";
		}
		if (device.hasProperty("displayValue"))
		{
			deviceInfo.displayValue = device.getPropertyString("displayValue") + " " + deviceInfo.physicalUnit;
		}
		else if (device.hasProperty("displayState"))
		{
			deviceInfo.displayValue = device.getPropertyString("displayState");
		}
		else
		{
			deviceInfo.displayValue = "";
		}
		devices.push(deviceInfo);
	}
}

response.contentType = 'application/json';
response.write(JSON.stringify(devices));
response.send();
