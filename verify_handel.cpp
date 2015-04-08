#include "stdafx.h"
#include "verify_handel.h"
#include "safe_server.h"

CVerifyHandle::CVerifyHandle()
{
	addMethod("verify", (IDealDataHandle::Method)&CVerifyHandle::verify);
}



void CVerifyHandle::verify(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	base::CLogDataInf &dataInf = pCalcInf->m_dataInf;

	char *oper = dataInf.m_infs[0];
	char *sessionId = dataInf.m_infs[1];
	
	char *keyInf = dataInf.m_infs[2];
	int keyInfLen = dataInf.m_infLens[2];
	
	char *userName = dataInf.m_infs[3];	
	int userNameLen = dataInf.m_infLens[3];
	
	char *passWord = dataInf.m_infs[4];
	int passWordLen = dataInf.m_infLens[4];


	CSafeServer::instance()->decode(keyInf, keyInfLen, userName, userNameLen,userName);
	CSafeServer::instance()->decode(keyInf, keyInfLen, passWord, passWordLen,passWord);
	printf("CVerify::dealDataHandle %s  %s  %s\n", oper, userName, passWord);

	{
		base::CLogDataInf &dataInf = repCalcInf->m_dataInf;
		
		char keyInf[KEY_INF_LEN];
		CSafeServer::instance()->createKeyInf(keyInf, sizeof(keyInf));

		char _userName[32];
		int _userNameLen = sizeof(_userName);
		CSafeServer::instance()->encode(keyInf, sizeof(keyInf), userName, strlen(userName)+1, _userName, _userNameLen);

		char _passWord[32];
		int _passWordLen = sizeof(_passWord);
		CSafeServer::instance()->encode(keyInf, sizeof(keyInf), passWord, strlen(passWord)+1, _passWord, _passWordLen);

		dataInf.putInf(oper);
		dataInf.putInf(sessionId);//session id(大于0)
		dataInf.putInf(keyInf, sizeof(keyInf));//密钥
		dataInf.putInf(_userName, _userNameLen);//用户名
		dataInf.putInf(_passWord, _passWordLen); //密码
		dataInf.packet();

	}
}


