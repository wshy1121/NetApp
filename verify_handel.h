#ifndef _VERIFY_HANDLE_H_
#define _VERIFY_HANDLE_H_

#include "data_handle.h"

class CVerifyHandle : public IDealDataHandle
{
public:
	CVerifyHandle();
public:
	void verify(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};


#endif

