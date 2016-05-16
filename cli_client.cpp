#include <boost/thread/mutex.hpp>
#include "cli_client.h"

static boost::mutex g_insMutex;

CCliUdpClient* CCliUdpClient::_instance = NULL;

CCliUdpClient* CCliUdpClient::instance() 
{
	if (NULL == _instance)
	{
		boost::unique_lock<boost::mutex> lock(g_insMutex);
		if (NULL == _instance)
		{
			_instance = new CCliUdpClient();
		}
	}
	return _instance;
}

CCliUdpClient::CCliUdpClient()
{
}



bool CCliUdpClient::getSerIp(Json::Value serverIps)
{
    return true;
}


