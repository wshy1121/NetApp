#include "net_server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "defs.h"



extern CPthreadMutex g_insMutexCalc;

CNetServer* CNetServer::_instance = NULL;

CNetServer::CNetServer():SERVER_PORT(8889), m_sockLister(INVALID_SOCKET)
{

	return ;
}

CNetServer* CNetServer::instance() 
{	
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
#ifdef WIN32	
			WSADATA wsa={0};
			WSAStartup(MAKEWORD(2,2),&wsa);
#endif
			_instance = new CNetServer;
		}
	}
	return _instance;
}

bool CNetServer::startServer()
{
	if(INVALID_SOCKET != m_sockLister)
	{
		printf("server is runing!\n");
		return false;
	}
	
	m_sockLister = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockLister == INVALID_SOCKET)
	{
		printf("create sock error!\n");
		return false;
	}

	int opt = 0;
	if (setsockopt(m_sockLister, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt))< 0)
	{
		printf("set sock error!\n");
		return false;
	}
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = INADDR_ANY;
	svraddr.sin_port = htons(SERVER_PORT);

	int ret = bind(m_sockLister, (struct sockaddr*) &svraddr, sizeof(svraddr));
	if (ret == SOCKET_ERROR)
	{
		printf("bind sock error!\n");
		return false;
	}

	base::pthread_create(&m_hListenThread, NULL,listenThread,NULL);
	base::pthread_create(&m_hClientThread, NULL,clientThread,NULL);

	printf("server is start!\n");
	return true;
}


void *CNetServer::listenThread(void *arg)
{
	return CNetServer::instance()->_listenThread(arg);
}

void *CNetServer::clientThread(void *arg)
{
	return CNetServer::instance()->_clientThread(arg);
}

void *CNetServer::_listenThread(void *arg)
{
	int backlog = 5;
	int ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return NULL;
	}

	while(1)
	{
		sockaddr_in clientAddr;
		socklen_t nLen = sizeof(sockaddr);
		SOCKET socket = accept(m_sockLister,(sockaddr*)&clientAddr,&nLen);
		{
			CGuardMutex guardMutex(m_clientReadMutex);	
			m_listClientRead.push_back(socket);
		}
	}
	return NULL;
}


void *CNetServer::_clientThread(void *arg)
{
	char recvBuf[MAX_BUFFER_SIZE];
	memset(recvBuf,0,MAX_BUFFER_SIZE);

	char sendBuf[MAX_BUFFER_SIZE];
	memset(sendBuf,0,MAX_BUFFER_SIZE);

	fd_set fd_read, fd_write;
	while(true)
	{
		SocketList &listRead = m_listClientRead;
		SocketList &listWirte = m_listClientWrite;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		
		SocketList::iterator iterTmp;
		for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); ++iterTmp)	
		{
			FD_SET(*iterTmp, &fd_read);
		}
		for (iterTmp=listWirte.begin(); iterTmp!=listWirte.end(); ++iterTmp)	
		{
			FD_SET(*iterTmp, &fd_write);
		}
		
		struct timeval cctv = {0, 50};
		int count = select(100, &fd_read, &fd_write, NULL, &cctv);
		
		SocketList::iterator iterRead = listRead.begin();
		while(count > 0)
		{
			SocketList::iterator iterWrite;
			for (iterWrite=listWirte.begin(); iterWrite!=listWirte.end(); ++iterWrite)	
			{
				if(FD_ISSET(*iterWrite, &fd_write))
				{
					--count;
					send(*iterWrite, sendBuf, strlen(sendBuf));
				}
			}
			listWirte.clear();
			if(FD_ISSET(*iterRead, &fd_read))
			{
				memset(recvBuf, 0, sizeof(recvBuf));
				if(0 < receive(*iterRead, recvBuf, sizeof(recvBuf)))
				{
					strcpy(sendBuf, recvBuf);
					
					for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); ++iterTmp)	
					{
						listWirte.push_back(*iterTmp);
					}
				}
				else
				{
					CGuardMutex guardMutex(m_clientReadMutex);
					for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); )
					{
						if(*iterRead == *iterTmp)
						{
							close(*iterRead);
							iterTmp = listRead.erase(iterTmp);
							break;
						}
					}
					for (iterTmp=listWirte.begin(); iterTmp!=listWirte.end(); )
					{
						if(*iterRead == *iterTmp)
						{
							iterTmp = listWirte.erase(iterTmp);
							break;
						}
					}
				}
				break;
			}
			++iterRead;
		}
	}
	return NULL;
}


int CNetServer::receive(SOCKET fd,char *szText,int len)
{
	int rc;
	rc=recv(fd,szText,len,0);
	if(rc <= 0)
	{
		return -1;
	}
	return rc;
}

int CNetServer::send(SOCKET fd,char *szText,int len)
{
	int cnt;
	int rc;
	cnt=len;
	while(cnt>0)
	{
		rc=::send(fd,szText,cnt,0);
		if(rc==SOCKET_ERROR)
		{
			return -1;
		}
		if(rc==0)
		{
			return len-cnt;
		}
		szText+=rc;
		cnt-=rc;
	}
	return len;
}


