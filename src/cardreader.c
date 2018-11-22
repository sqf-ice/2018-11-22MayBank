#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define READER_PORT PORT_NUM_COM1
uchar gcSeq = 1;
/**
* @brief 报文加上起始符，长度，终止符，校验值，用以异步通信
* @param in out 传入需要加的数据指针，返回已加的数据指针
* @return in out 传入需要加的数据长度指针，返回已加的数据长度指针
* @li APP_SUCC
* @li APP_FAIL
*/
static int InsertAsyn(char *pszBuf, unsigned int *punLen)
{
	int nLen = 2;
	char szTemp[MAX_SEND_SIZE]={0};
	int nPos = 0;

	if (NULL == pszBuf || NULL == punLen)
	{
		return APP_FAIL;
	}

	memcpy(szTemp, pszBuf, *punLen);

	
	pszBuf[nPos] = STX;
	nPos ++;
	
	PubIntToC2(pszBuf + nPos, *punLen); 
	nPos += 2;
	
	memcpy(pszBuf + nPos, szTemp, *punLen);
	nPos += *punLen; 
	
	PubCalcLRC(pszBuf, nPos, pszBuf + nPos);
	nPos ++;
	
	pszBuf[nPos] = ETX;
	nPos ++;
	
	*punLen = nPos;
	return APP_SUCC;
}

int FormCommand(STSYSTEM *pstSystem, char *psBuf, int *pnLen, char cCmd)
{
	int nPos = 0;
	
	psBuf[nPos] = gcSeq;
	nPos ++;

	psBuf[nPos] = cCmd;		
	nPos ++;
	
	switch (cCmd)
	{
	case CMD_ONLINE_SALE:
		PubAscToHex(pstSystem->szAmount, 12, 0, psBuf+nPos);
		nPos += 6;
		break;
	case CMD_ONLINE_PROCESS_SALE:
		psBuf[nPos] = 0;		
		nPos ++;
		break;
	}

	*pnLen = nPos;

	return APP_SUCC;

}

int GetCommPortReadLen()
{
    int nReadLen = 0;
	
	NDK_PortReadLen(PORT_NUM_COM1, &nReadLen);

	return nReadLen;
}

static int  ProCommAsyn(char * pszBuffer, int nBufLen)
{
	char cLrc = 0;
	
	if( *pszBuffer !=0x02)	
		return APP_FAIL;

	PubCalcLRC(pszBuffer, nBufLen-2, &cLrc);
	
	if(*(pszBuffer+nBufLen-2) != cLrc)
	{
//DispTrace("ProCommCalcLRC=0x%x",cLrc);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int RecvFromRS232(char *psData, int *pnDataLen, int nPort, int nTimewait)
{
	int nLen=0;
	char szBuffer[1024];
	char szRecvBuf[1024];
	int nDateLen=0;
	int nRet;
	int nTimes=0;


	while(1)
	{
		memset(szBuffer,0, sizeof(szBuffer));
		memset(szRecvBuf, 0, sizeof(szRecvBuf));
		nLen = 0;

		if (GetCommPortReadLen() <= 0)
		{		
			nRet = PubKbHit();
			switch(nRet)
			{
			case KEY_ESC://KEY_ENTER:
				//return APP_QUIT;
				break;
			case KEY_F3:
				return APP_FUNCQUIT;
				break;
			default:
				break;
			}
			NDK_SysMsDelay(1000);
TRACE(("1nTimes: %d[%d]", nTimes, nTimewait));	
			if(++nTimes>=nTimewait)
			{
				return APP_FAIL;
			}	
			continue;
		}
		nRet =NDK_PortRead(nPort, 1 ,szBuffer,1000, &nLen);

		if(szBuffer[0]==0x02)
		{	
			break;
		}
	}
	if(szBuffer[0]==0x02)//每包以0x02开头
	{	
		szRecvBuf[0] =szBuffer[0];
	
		/*DATA LEN*/
		memset(szBuffer, 0, sizeof(szBuffer));
		NDK_PortRead( nPort,2 ,(char *)&szBuffer,1000, &nRet);
		if (nRet != 2) {
			return APP_FAIL;
		}
		PubC2ToInt(&nDateLen, szBuffer);
		if (nDateLen <=0 ||nDateLen >1000) {
			return APP_FAIL;
		}
		memcpy(szRecvBuf+nLen, szBuffer, nRet);
		nLen += nRet;
		/*DATA*/
		memset(szBuffer, 0, sizeof(szBuffer));
		NDK_PortRead( nPort,nDateLen ,(char *)&szBuffer,3000, &nRet);
		if (nRet != nDateLen) {
			return APP_FAIL;
		}
		memcpy(szRecvBuf+nLen, szBuffer, nRet);
		nLen += nRet;
		/*ETX+CRC */
		memset(szBuffer, 0, sizeof(szBuffer));
		NDK_PortRead( nPort,2 ,(char *)&szBuffer,1000, &nRet);
		if (nRet != 2  ) {
			return APP_FAIL;
		}
		memcpy(szRecvBuf+nLen, szBuffer, nRet);
		nLen += nRet;
		
PubDebugData("recv from ECR: ", szRecvBuf, nLen);
		if(ProCommAsyn(szRecvBuf, nLen) != APP_SUCC) 
		{
			//return APP_FAIL;
		}
		
		/*DATA*/
		memcpy(psData, szRecvBuf+1+2, nDateLen);

		*pnDataLen = nDateLen;
PubDebug("nLen = %d", nDateLen);
PubDebugData("data: ", psData, *pnDataLen);
		return APP_SUCC;
	}
	return nRet;
}

void IncreaseSeq()
{	
	if (gcSeq == 0xff)
	{
		gcSeq = 0x00;
	}
	gcSeq ++;
}

int ParseRespInfo(STSYSTEM *pstSystem, char *psBuf, int nLen, char cCmd)
{
	int nPos = 0;
	char szContent[256] = {0};
	int nErrCode = 0;
	int nAddtionLen = 0;
	int nValueLen = 0;
	
	
	if (gcSeq != *(psBuf + nPos))
	{ 
		PubMsgDlg("Warn", "Seq Not Match", 3, 3);
		return APP_QUIT;
	}
	nPos ++;

	nErrCode = *(psBuf + nPos);
	if (nErrCode != 0)
	{
		sprintf(szContent, "Error Code: %x", nErrCode);
		PubMsgDlg("Warn", szContent, 3, 3);
		return APP_QUIT;
	}
	nPos++;

	IncreaseSeq();
TRACE(("1"));
	
	if (cCmd != CMD_ONLINE_PROCESS_SALE)
	{
		return APP_SUCC;
	}
TRACE(("1"));
	//TID
	memcpy(pstSystem->szCD_TID, psBuf+nPos, 8);
TRACE(("1"));
	nPos += 8;
	//date time
	PubHexToAsc(psBuf+nPos+1, 4, 0, pstSystem->szDate);
	nPos += 3;
	PubHexToAsc(psBuf+nPos, 6, 0, pstSystem->szTime);
	nPos += 3;
	//pan
	PubHexToAsc(psBuf+nPos, 16, 0, pstSystem->szPan);
	memset(pstSystem->szPan+4, '*', strlen(pstSystem->szPan - 4));
	nPos += 10;
	//hash pan
	nPos += 40;
	//rrn
	memcpy(pstSystem->szRefnum, psBuf+nPos, 12);
	nPos += 12;
	//stan
	PubHexToAsc(psBuf+nPos, 6, 0, pstSystem->szTrace);
	nPos += 3;
	//appr code
	memcpy(pstSystem->szAuthCode, psBuf+nPos, 6);
	nPos += 6;
	//addtion al
	nPos += 2;// 1F06
	PubC2ToInt(&nAddtionLen, psBuf+nPos);
	nPos += 2;// len
TRACE(("nAddtionLen: %d", nAddtionLen));
	TlvGetValue(0x84, pstSystem->sAID, &pstSystem->nAidLen, psBuf+nPos, nAddtionLen);
	TlvGetValue(0x95, pstSystem->sTVR, &nValueLen, psBuf+nPos, nAddtionLen);
	TlvGetValue(0x9F26, pstSystem->sTC, &nValueLen, psBuf+nPos, nAddtionLen);
	TlvGetValue(0xDF60, pstSystem->szBatchNum, &nValueLen, psBuf+nPos, nAddtionLen);
	TlvGetValue(0xDF42, pstSystem->szCD_MID, &nValueLen, psBuf+nPos, nAddtionLen);
	TlvGetValue(0x50, pstSystem->szAppLabel, &nValueLen, psBuf+nPos, nAddtionLen);
	
	return APP_SUCC;
}

int PayByReader()
{
	char szTitle[32] = "PayWave";
	STSYSTEM stSystem;
	char sBuf[1024] = {0};
	int nLen = 0;
	int nRet = 0;
	char cCmd = CMD_ONLINE_SALE;
	STTAANSRECORD stTransRecord;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = TRANS_CARDREADER;
	stSystem.cTransAttr = ATTR_qPBOC;
	stSystem.nAcqType = 0xff;
	stSystem.nAcqIndex = 0xff;
	stSystem.cWavePinAndSigFlag = 1;
	if (NDK_OK != NDK_PortOpen(READER_PORT, "115200,8,N,1"))
	{
		PubMsgDlg(szTitle, "port fail", 3, 3);
		return APP_FAIL;
	}
	
	ASSERT_QUIT(EdcGetAmout(szTitle, &stSystem));

	//init sale
	ASSERT_QUIT(FormCommand(&stSystem, sBuf, &nLen, CMD_ONLINE_SALE));
	InsertAsyn(sBuf, &nLen);
	if (NDK_OK != NDK_PortWrite(READER_PORT, nLen, sBuf))
	{
		PubMsgDlg(szTitle, "send fail", 3, 3);
		goto FAIL;
	}

	PubClearAll();
	PubDisplayGen(2, "Init Sale...");
	PubUpdateWindow();
	memset(sBuf, 0, sizeof(sBuf));
	nLen = 0;

	if (APP_SUCC != RecvFromRS232(sBuf, &nLen, READER_PORT, 22))
	{
		PubMsgDlg(szTitle, "Timeout", 3, 3);
		goto FAIL;
	}
	
	if (APP_SUCC != ParseRespInfo(&stSystem, sBuf, &nLen, CMD_ONLINE_SALE))
	{
		goto FAIL;
	}

	/***************** process sale ******************
	**************************************************/
	ASSERT_QUIT(FormCommand(&stSystem, sBuf, &nLen, CMD_ONLINE_PROCESS_SALE));
	InsertAsyn(sBuf, &nLen);
	if (NDK_OK != NDK_PortWrite(READER_PORT, nLen, sBuf))
	{
		PubMsgDlg(szTitle, "send fail", 3, 3);
		goto FAIL;
	}
	PubClearAll();
	PubDisplayGen(2, "Process Sale...");
	PubUpdateWindow();
	memset(sBuf, 0, sizeof(sBuf));
	nLen = 0;
	if (APP_SUCC != RecvFromRS232(sBuf, &nLen, READER_PORT, 60))
	{
		PubMsgDlg(szTitle, "Timeout", 3, 3);
		goto FAIL;
	}
	if (APP_SUCC != ParseRespInfo(&stSystem, sBuf, &nLen, CMD_ONLINE_PROCESS_SALE))
	{
		goto FAIL;
	}

	
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	
	SysToRecord(&stSystem, &stTransRecord);
	if (APP_SUCC != PrintWater(&stTransRecord, FIRSTPRINT))
	{
		goto FAIL;
	}
	
	//PubMsgDlg(szTitle, "Paywave Succeed", 1, 1);
	NDK_PortClose(READER_PORT);
	APP_SUCC;
FAIL:
	NDK_PortClose(READER_PORT);
	
	return APP_FAIL;;
}
