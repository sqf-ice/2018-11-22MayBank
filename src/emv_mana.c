/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  emv_mana.c
** File indentifier: 
** Brief:  EMV Transaction manage
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-20
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/**
* @brief Send Online Detail
* @param in pstSystem 
* @param in pstEMV_Water
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
static int EmvDetailSendOnline(STSYSTEM *pstSystem, STTAANSRECORD *pstTransRecord)
{
	int nRet;
	char sPack[MAX_PACK_SIZE];
	int nPackLen;
	char sMac[8] = {0};
	
	nRet = PackGeneral(pstSystem, pstTransRecord);	
	ASSERT_HANGUP_FAIL(Pack(sPack, &nPackLen));
	nPackLen -= 8;
	ASSERT_HANGUP_FAIL(AddMac(sPack,&nPackLen, KEY_TYPE_MAC));
#if 0	
	GetMac(sPack, &nPackLen, sMac);
	ASSERT_HANGUP_FAIL(ProcessTLED(pstSystem));
	ASSERT_HANGUP_FAIL(Pack(sPack, &nPackLen));
	memcpy(sPack+nPackLen-8, sMac, 8);
#endif

	PubDisplayTitle("SEND TC");
	ASSERT_HANGUP_FAIL(CommConnect());
	IncVarTraceNo();
	nRet = CommSendRecv(sPack, nPackLen, sPack, &nPackLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	nRet = Unpack(sPack, nPackLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	nRet = ChkRespMsgID("0320", sPack);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	nRet = ChkRespon(pstSystem, sPack + 2);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	
	PubClear2To4();
	if (memcmp(pstSystem->szResponse, "00", 2) != 0)
	{
		if (memcmp(pstSystem->szResponse, "LE", 2) == 0)
		{
			DispRespEFTInfo();
		}
		else
		{
			DispResp(pstSystem->szResponse, pstSystem->nAcqIndex);
		}
		return -5;
	}

	return APP_SUCC;
}

/**
* @brief Send all the successful online detail
* @param in nFileHandle
* @param in nWaterSum
* @param in pnBatchNum
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int EmvBatchUpOnlineSucc(const int nFileHandle, const int nWaterSum, int *pnBatchNum)
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nRet;
	int nLoop, nFailLoop = 0, nIsFirstFail = TRUE;
	char nMaxReSend;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	GetVarCommReSendNum(&nMaxReSend);
	nLoop = 1;
	
	GetVarEmvOnlineUpNum(&nLoop);
	if (0 >= nLoop)
	{
		nLoop = 1;
	}
	
	for (; nLoop <= nWaterSum; nLoop++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		nRet = PubReadRec(nFileHandle, nLoop, (char *)&stTransRecord);
		if (nRet != APP_SUCC)
		{
			continue;
		}
		
		if ((ATTR_EMV_STANDARD == stTransRecord.cTransAttr && EMV_STATUS_ONLINE_SUCC == stTransRecord.cEMV_Status))
		{
			if (stTransRecord.nAcqIndex == 1)//20170315 amex not send tc
			{
				continue;
			}
			;
		}
		else
		{
			continue;
		}

		if(stTransRecord.cTcFlag > nMaxReSend)
			continue;
		RecordToSys(&stTransRecord, &stSystem);
		DealSystem(&stSystem);
		

		if (stTransRecord.sTVR[4]&0x40)
		{
			continue;
		}

		memcpy(stSystem.szMsgID, "0320", 4);
		memcpy(stSystem.szProcCode, "940000", 6);
	
		nRet = EmvDetailSendOnline(&stSystem, &stTransRecord);
		if (nRet != APP_SUCC)
		{
			if(nRet == -5)
			{
				stTransRecord.cTcFlag = 0xFE;
			}
			goto BATCHUPFAIL;
		}
		stTransRecord.cTcFlag = 0xFD;
	
		PubDisplayGen(3, "SEND TC OK");
		PubUpdateWindow();
	BATCHUPFAIL:
		ASSERT_FAIL(DealBatchUpFail(&stTransRecord, nFileHandle, nLoop));
		if(stTransRecord.cTcFlag <= nMaxReSend)
		{
			if(nIsFirstFail)
			{
				nFailLoop = nLoop;
				nIsFirstFail = FALSE;
				SetVarEmvOnlineUpNum(nFailLoop);
			}
		}	
	}
	if(nIsFirstFail)
		nFailLoop = nWaterSum+1;
	SetVarEmvOnlineUpNum(nFailLoop);
	return APP_SUCC;
}

/**
* @brief Send TC
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
* @author 
* @date 
*/
int EmvSendTC(void)
{	
	int nRet = 0;
	int nWaterSum;
	int nFileHandle;
	int nBatchNum = 0;
	
#ifdef DEMO
	return APP_SUCC;
#endif
	GetRecordNum(&nWaterSum);

	if (nWaterSum > 0)
	{
		nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	else
	{
		return APP_FAIL;
	}

	nRet = EmvBatchUpOnlineSucc(nFileHandle, nWaterSum, &nBatchNum);
	if (nRet != APP_SUCC)
 	{   
		PubCloseFile(&nFileHandle);
		return APP_FAIL;
 	}
	
	ASSERT_FAIL(PubCloseFile(&nFileHandle));
	
	SetVarEmvOnlineUpNum(1);

	return APP_SUCC;
}

