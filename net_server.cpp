#include "stdafx.h"
#include "net_server.h"
#include "string_base.h"
#include "socket_base.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "Global.h"



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
	return NULL;
}

void *CNetServer::_listenThread(void *arg)
{
	int backlog = 5;
	int ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return NULL;
	}
	fd_set fd_read, fd_write;
	sockaddr_in clientAddr;
	socklen_t nLen = sizeof(sockaddr);
	struct timeval cctv = {0, 50};
	SocketList &listRead = m_listClientRead;	
	char recvBuf[1024];
	while(true)
	{
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_SET(m_sockLister, &fd_read);
		SocketList::iterator iterTmp;
		for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); ++iterTmp)	
		{
			FD_SET(*iterTmp, &fd_read);
		}
		
		int count = select(listRead.size()+1, &fd_read, &fd_write, NULL, &cctv);
		for (int i=0; i< count; ++i)
		{
			SocketList::iterator iterRead;
			for (iterRead=listRead.begin(); iterRead!=listRead.end(); ++iterRead)
			{
				if(FD_ISSET(*iterRead, &fd_read))
				{
					memset(recvBuf,0,sizeof(recvBuf));
					if(0 < receive(*iterRead, recvBuf, sizeof(recvBuf)))
					{
					}
					//异常处理
					else
					{
						CGuardMutex guardMutex(m_clientReadMutex);
						base::close(*iterRead);
						printf("close  %d\n", *iterRead);
						iterRead = listRead.erase(iterRead);
						if (iterRead==listRead.end())
						{
							break;
						}
					}
				}
			}

			//处理数据发送
			
			//处理连接消息
			if(FD_ISSET(m_sockLister, &fd_read))
			{
				SOCKET socket = accept(m_sockLister,(sockaddr*)&clientAddr,&nLen);
				{
					CGuardMutex guardMutex(m_clientReadMutex);	
					m_listClientRead.push_back(socket);
					printf("accept	%d\n", socket);
				}
			}
		}
	}
	return NULL;
}


void *CNetServer::_clientThread(void *arg)
{
	char recvBuf[1024];
	memset(recvBuf,0,sizeof(recvBuf));
	fd_set fd_read, fd_write;
	while(true)
	{
		SocketList &listRead = m_listClientRead;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		
		SocketList::iterator iterTmp;
		for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); ++iterTmp)	
		{
			FD_SET(*iterTmp, &fd_read);
		}
	
		struct timeval cctv = {0, 50};
		int count = select(listRead.size(), &fd_read, &fd_write, NULL, &cctv);
		
		SocketList::iterator iterRead = listRead.begin();
		while(count > 0)
		{
			if(FD_ISSET(*iterRead, &fd_read))
			{
				memset(recvBuf, 0, sizeof(recvBuf));
				if(0 < receive(*iterRead, recvBuf, sizeof(recvBuf)))
				{
				}
				else
				{
					CGuardMutex guardMutex(m_clientReadMutex);
					for (iterTmp=listRead.begin(); iterTmp!=listRead.end(); )
					{
						if(*iterRead == *iterTmp)
						{
							base::close(*iterRead);
							printf("close  %d\n", *iterRead);
							iterTmp = listRead.erase(iterTmp);
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


