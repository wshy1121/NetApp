#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H
#include <string>
#include "defs.h"
#include "link_tool.h"
#include "user_manager.h"
#include "trace_handel.h"
#include "trace_packet.h"
#include "trace_server.h"

class CNetClient
{
public:
	static CNetClient* instance();
	bool disConnect();	
	bool connect(char *sip);	
	int dealPacket(char *packet, int packetLen, CLogDataInf &dataInf);
public:
	int getSessionId();	
	bool login(char *userName, char *passWord);	
	bool createAccess(char *access, int &accessLen);	
	bool getAccessRep(char *access, int accessLen, char *accessRep);
	bool verifyAccess(char *access, int accessLen, char *accessRep);
	bool getClientInf(IClientInf *clientInf);
	bool cleanFile(const char *fileName);
	bool getTraceFileList(CTraceClient::TraceFileVec &fileList);
	bool getTraceFileInf(const char *fileName, TraceFileInf &traceFileInf);
private:	
	CNetClient();		
	int send(char *szText,int len);
	bool receiveInfData(CLogDataInf *pDataInf);
private:	
	static  CNetClient* _instance;
	CTraceManager *m_dataWorkManager;
	base::CPthreadMutex socketMutex;
	const int m_maxSessionId;
	int m_sessionId;
	int m_socketClient;
	std::string m_sip;
};



#endif

