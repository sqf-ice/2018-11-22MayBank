/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  debug.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-4
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
* define global variable
*/
static char gszDebugData[100];

void LocalDebug(char* , ...);
void LocalDebugHex(const char * ,const void * ,int );
int DebugBufToFile(const char *pszBuf, const int nBufLen);
int DebugBufToLocalFile(const char *, const int , char *, char *);

/**
* @brief 
* @param [in]  char *pszFile 
* @param [in]  char *pszFunc 
* @param [in]  int nLine     
* @return ·µ»Ø
*/
void SetDebugData(const char *pszFile, const char *pszFunc, const int nLine)
{
	sprintf(gszDebugData, "¡¾%s¡¿[%s][%s][%d]>>>", APP_NAMEDESC, pszFile, pszFunc,nLine);
}

/**
** brief: for debug
** param [in]: 
** param [out]: 
** return: 
** auther: 
** date: 2016-7-4
** modify: 
*/
int Debug(const char *pszFile, const char *pszFunc, const int nLine, const int nRes)
{
	STAPPCOMMPARAM stCommPara;

	GetAppCommParam(&stCommPara);
	if(stCommPara.cCommType == COMM_RS232)
	{
		return nRes;
	}
	if (nRes != APP_SUCC)
	{
		PubDebug("¡¾%s¡¿[%s][%s][%d]>>>%d", APP_NAMEDESC, pszFile, pszFunc,nLine,nRes);
#ifdef _DEBUG_
		LocalDebug("¡¾%s¡¿[%s][%s][%d]>>>%d", APP_NAMEDESC, pszFile, pszFunc,nLine,nRes);
#endif
	}
	return nRes;
}

/**
* @brief 
* @param in lpszFormat 
* @return	void
*/
void Trace(char* lpszFormat, ...)
{
	va_list args;
	int nBuf;
	char buf[2048];
	STAPPCOMMPARAM stCommPara;

	GetAppCommParam(&stCommPara);
	if(stCommPara.cCommType == COMM_RS232)
	{
		return;
	}
	
	va_start(args, lpszFormat);
	nBuf=vsprintf(buf, lpszFormat, args);
	PubDebug("%s%s",gszDebugData, buf);	
#ifdef _DEBUG_
	LocalDebug("%s%s",gszDebugData, buf);	
#endif	
	va_end(args);
}


/**
* @brief Trace hex
* @param in lpszFormat 
* @return	void
*/
void TraceHex(char* pszHexBuf, int nLen, char* lpszFormat, ...)
{
	va_list args;
	int nBuf;
	char buf[2048];
	STAPPCOMMPARAM stCommPara;

	GetAppCommParam(&stCommPara);
	if(stCommPara.cCommType == COMM_RS232)
	{
		return;
	}
	
	va_start(args, lpszFormat);
	sprintf(buf, "%s", gszDebugData);
	nBuf=vsprintf(buf+strlen(buf), lpszFormat, args);
	PubDebugData(buf, pszHexBuf, nLen);
#ifdef _DEBUG_
	LocalDebugHex(buf, pszHexBuf, nLen);	
#endif
	va_end(args);
}

/**
* @brief Display msg int the screen
* @param in lpszFormat
* @return	void
*/
void DispTrace(char* lpszFormat, ...)
{
	va_list args;
	char buf[2048] = {0};
	va_start(args, lpszFormat);
	vsprintf(buf, lpszFormat, args);
	PubClearAll();
	NDK_ScrPrintf(buf);
	PubUpdateWindow();
	PubGetKeyCode(0);
	va_end(args);
}
/**
* @brief 
* @param in lpszFormat 
* @return	void
*/
void LocalDebug(char* lpszFormat, ...)
{
	va_list args;
	int nBuf;
	char buf[2048];
	
	va_start(args, lpszFormat);
	nBuf=vsprintf(buf, lpszFormat, args);
	strcat(buf, "\r\n");
	nBuf+=2;
	
	if(nBuf<2000)
		DebugBufToFile(buf, nBuf);
	
	va_end(args);
}

/**
* @brief 
* @param in lpszFormat
* @return	void
*/
void LocalDebugHex(const char* pszTitle,const void* pData,int nLen)
{
	int nTitleLen=0;
	char szBuf[8192+1]={0};
	int i=0;

	strcat(szBuf, pszTitle);
	nTitleLen +=strlen(pszTitle);
	if(nLen+nTitleLen >8000)
	{
		strcat(szBuf, "too long than 8000 bytes\r\n");
		nLen = (8000 - strlen(szBuf))/2;
	}
	for(i=0; i<nLen; i++)
	{
		sprintf(szBuf+nTitleLen+i*2, "%02X", *(char *)(pData+i));
	}
	strcat(szBuf, "\r\n");
	DebugBufToFile(szBuf, strlen(szBuf));
	return;
}

/**
** brief: Debug buffer to file
** param [in]: pszBuf
** param [in]: nBufLen
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DebugBufToFile(const char *pszBuf, const int nBufLen)
{
#ifdef _DEBUG_
	FILE * fp;
	char szTmp[64+1]={0};
	char szFileName[64+1]={0};
	struct timeb stTimeB;
	struct tm *pstTime;

	ftime(&stTimeB);
	pstTime = localtime(&stTimeB.time);
	fp=fopen("/tmp/debug.log", "a");
	if(fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		
		{
			memset(szTmp,0,sizeof(szTmp));
			sprintf(szTmp,"[%04d-%02d-%02d %02d:%02d:%02d.%03d]",
				pstTime->tm_year+1900,pstTime->tm_mon+1, pstTime->tm_mday,
				pstTime->tm_hour,pstTime->tm_min, pstTime->tm_sec, stTimeB.millitm);
			fwrite(szTmp, strlen(szTmp), 1, fp);
		}
		fwrite(pszBuf, nBufLen, 1, fp);
		
		sprintf(szFileName,"%04d%02d%02d.log",pstTime->tm_year+1900,pstTime->tm_mon+1, pstTime->tm_mday);
		//DebugBufToLocalFile(pszBuf,nBufLen,szTmp,szFileName);
		return fclose(fp);
	}
	else
	{
		return APP_FAIL;
	}
#else
	return APP_SUCC;
#endif
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DebugBufToLocalFile(const char *pszBuf, const int nBufLen, char *pszHead, char *pszFileName)
{	
	FILE * fpLocal;
	
	fpLocal=fopen(pszFileName, "a");
	if(fpLocal != NULL)
	{
		fseek(fpLocal,0,SEEK_END);		
		fwrite(pszHead, strlen(pszHead), 1, fpLocal);
		fwrite(pszBuf, nBufLen, 1, fpLocal);		
		return fclose(fpLocal);
	}
	else
	{
		return APP_FAIL;
	}
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DebugSendHex(char * psRecvBuf, int nLen)
{
#ifdef _DEBUG_
	char szBuf[8092];
	int i=0;

	memset(szBuf, 0, sizeof(szBuf));
	for(i=0; i<nLen; i++)
	{
		sprintf(szBuf+i*2, "%02X ", *(psRecvBuf+i));
	}
	strcat(szBuf, "\r\n");
	DebugBufToFile(szBuf, strlen(szBuf));
#endif
	return APP_SUCC;
}

#define MAX_SEND_BUF_LEN 1024
/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
static int ExportLogFile(char *pszFilename)
{
	int nBytesRead;
	char sBuf[MAX_SEND_BUF_LEN+1];
	char szParamList[1024+1]={0};
	char *pszTitle="DEBUG LOG";
	int i=0;
	int nFileSize=0,nPacketSum=0;
	EM_PORT_NUM emPort=PORT_NUM_COM1;
	FILE *fp = NULL;
	
	if((fp = fopen(pszFilename,"r"))== NULL)
	{
		memset(szParamList,0,sizeof(szParamList));
		sprintf(szParamList,"NON LOG %s", pszFilename);
		PubMsgDlg(pszTitle, szParamList, 3, 3);
		return APP_FAIL;
	}
	if (NDK_OK != NDK_PortOpen(emPort, "115200,8,N,1")	)
	{
		memset(szParamList,0,sizeof(szParamList));
		sprintf(szParamList,"RS232[%d] open failed", emPort);
		PubMsgDlg(pszTitle, szParamList, 3, 3);
		fclose(fp);
		return APP_FAIL;
	}
	memset(szParamList,0,sizeof(szParamList));
	sprintf(szParamList,"Port:RS232[%d]\n115200\nConnect PC then press any\n", emPort);
	PubMsgDlg(pszTitle, szParamList, 1, 60);

	fseek(fp,(long)0,SEEK_END);
	nFileSize = ftell(fp);
	nPacketSum = (nFileSize/MAX_SEND_BUF_LEN) + ((nFileSize%MAX_SEND_BUF_LEN)?1:0);
	
	fseek(fp,(long)0,SEEK_SET);
	nBytesRead = 1;
	i =0;
	while(nBytesRead>0)
	{
		memset(sBuf,0,sizeof(sBuf));
		nBytesRead = fread(sBuf, MAX_SEND_BUF_LEN, 1, fp);
		
		memset(szParamList,0,sizeof(szParamList));
		sprintf(szParamList,"Connected PC\nProcessing...\nSent %dth package\n[%d] [%d] [%d]\n", 
			++i, nPacketSum, nFileSize, nBytesRead);
		
		NDK_PortWrite(emPort, MAX_SEND_BUF_LEN, sBuf);
		if(i%10 == 0)
		{
			PubDisplayStrInline(DISPLAY_MODE_CENTER,5,"[%03d\%]", i*100/nPacketSum);
			PubUpdateWindow();
		}
	}
	PubDisplayStrInline(DISPLAY_MODE_CENTER,5,"[%03d\%]", i*100/nPacketSum);
	PubUpdateWindow();

	NDK_PortClose(emPort);
	fclose(fp);	

	PubMsgDlg(pszTitle, "Export log ok", 1, 1);
	return APP_SUCC;
}

/**
** brief: del log
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
static int DelLogFile(char *pszFilename)
{
	ASSERT_FAIL(NDK_FsDel(pszFilename));
	PubMsgDlg("","Del log ok",1,5);
	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SelectExportLogALL(void)
{
	ASSERT_FAIL(ExportLogFile("/tmp/debug.log"));
	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SelectExportLogEmv(void)
{
	int nSelect;
	int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.EXP EMVLOG"),
		tr("2.DEL EMVLOG"),
		tr("3.EXP EMVPRM"),
		tr("4.EXP DEBUG"),
		tr("5.EMV AID"),
		tr("6.DlFlag"),
		tr("7.tmsMode")
	};
	ASSERT_QUIT(PubShowMenuItems(tr("DEBUG"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60));
	//ASSERT_QUIT(ProSelectList("1.EXP EMVLOG||2.DEL EMVLOG||3.EXP EMVPRM||4.EXP DEBUG||6.DlFlag||7.tmsMode", "DEBUG", 0xFF, &nSelect));
	switch(nSelcItem)
	{
		case 1:
			ASSERT_FAIL(ExportLogFile("emvDebugFile"));
			break;
		case 2:
			ASSERT_FAIL(DelLogFile("emvDebugFile"));
			break;
		case 3:
			ASSERT_FAIL(DumpEmvParam());
			break;
		case 4:
			ASSERT_FAIL(SelectExportLogALL());
			break;
		case 5:
			ASSERT(DumpEmvAIDParam());
			break;
		case 6:
			SetFunctionIsSendAutoDown();
			break;
		case 7:
			SetFunctionDownMode();
			break;
		default :
			break;
	}
	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DumpEmvOneCpak(void)
{
	int nRet;
	int i;
	char sCapk[50][6];
	capk stCAPK;
	int nRes;

	nRet = EMV_EnumCAPK(0, 49, sCapk);
	if (nRet > 0)
	{
		TRACE(("CAPK_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{
			memcpy(stCAPK._rid, &sCapk[i][0], 5);
			stCAPK._index = sCapk[i][5];
			nRes = EMV_OperCAPK(&stCAPK, CAPK_GET);
			if (nRes == APP_SUCC)
			{
				TRACE(("[Index]:%02x ", stCAPK._index));
				TRACE_HEX(((char *)stCAPK._rid, 5,"[RID]:" ));
				TRACE_HEX(((char *)stCAPK._expired_date, 4,"[ExpDate]:"));
				TRACE_HEX(((char *)stCAPK._hashvalue, 20,"[HashValue]:"));
				TRACE(("[_pk_algorithm]:%02x ", stCAPK._pk_algorithm));
				TRACE(("[_hash_algorithm]:%02x ", stCAPK._hash_algorithm));
				TRACE(("[_disable]:%02x ", stCAPK._disable));				
				TRACE_HEX(((char *)stCAPK._resv, 3,"[_resv]:"));
				TRACE_HEX(((char *)stCAPK._key.pk_exponent, 3,"[pk_exponent]:"));
				TRACE(("[pk_mod_len]:%02x ", stCAPK._key.pk_mod_len));				
				TRACE_HEX(((char *)stCAPK._key.pk_modulus, MAX_MODULUS_LEN,"[pk_modulus]:"));
			}
			TRACE(("---------------------\r"));
		}
	}
	else
	{
		TRACE(("EMV_EnumCAPK=%d\r", nRet));
	}

	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DumpEmvOneAID(emvparam stAID)
{
	uint nNum;

	TRACE(("---------------------\r"));
	TRACE_HEX(((char *)stAID._aid, stAID._aid_len,"AID:"));
	TRACE_HEX(((char *)stAID._app_ver, 2, "VER:"));
	TRACE_HEX(((char *)stAID._tac_default, 5, "TAC_Default:"));
	TRACE_HEX(((char *)stAID._tac_denial, 5, "TAC_Decline:"));
	TRACE_HEX(((char *)stAID._tac_online, 5, "TAC_Online :"));
	PubC4ToInt(&nNum, (uchar *)stAID._floorlimit);
	TRACE(("FloorLimit:%d\r", nNum));
	PubC4ToInt(&nNum, (uchar *)stAID._threshold_value);
	TRACE(("ThresholdValue:%d\r", nNum));
	TRACE_HEX(((char *)&stAID._max_target_percent, 1, "MaxTargetPercent:"));
	TRACE_HEX(((char *)&stAID._target_percent, 1, "TargetPercent:"));
	TRACE_HEX(((char *)stAID._default_ddol, stAID._default_ddol_len, "Default DDOL:"));
	TRACE_HEX(((char *)stAID._default_tdol, stAID._default_tdol_len, "Default TDOL:"));
	TRACE_HEX(((char *)stAID._cap, 3, "CAP:"));
	TRACE_HEX(((char *)stAID._add_cap, 5,"ADDCAP:"));
	TRACE_HEX(((char *)&stAID._app_sel_indicator, 1, "ASI(PartialAID):"));
	TRACE_HEX(((char *)stAID._ics, 7, "ICS:"));
	TRACE_HEX(((char *)&stAID._status, 1, "TestType:"));
	TRACE_HEX(((char *)&stAID._ec_indicator, 1, "SUPPEC:"));
	TRACE_HEX(((char *)&stAID._limit_exist, 1, "_limit_exist:"));
	TRACE_HEX(((char *)stAID._ec_limit, 6, "_ec_limit:"));
	TRACE_HEX(((char *)stAID._cl_limit, 6, "_cl_limit:"));
	TRACE_HEX(((char *)stAID._cl_offline_limit, 6,"_cl_offline_limit:"));
	TRACE_HEX(((char *)stAID._cvm_limit, 6, "_cvm_limit:"));
	TRACE(("---------------------\r"));

	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DumpEmvL2OneAID(STCLSSPARAM stClssEPParam)
{
	uint nNum;

	TRACE(("stClssEPParam---------------------\r"));
	TRACE_HEX(((char *)stClssEPParam.usAid, stClssEPParam.ucAidLen,"AID:"));
	TRACE_HEX(((char *)stClssEPParam.usAppVer, 2, "VER:"));
	TRACE_HEX(((char *)stClssEPParam.usKernelId, 8, "kernelID:"));
	TRACE_HEX(((char *)stClssEPParam.usTacDefault, 5, "TAC_Default:"));
	TRACE_HEX(((char *)stClssEPParam.usTacDenial, 5, "TAC_Decline:"));
	TRACE_HEX(((char *)stClssEPParam.usTacOnline, 5, "TAC_Online :"));
	PubC4ToInt(&nNum, (uchar *)stClssEPParam.usFloorlimit);
	TRACE(("FloorLimit:%d\r", nNum));
	PubC4ToInt(&nNum, (uchar *)stClssEPParam.usThresholdValue);
	TRACE(("ThresholdValue:%d\r", nNum));
	TRACE_HEX(((char *)&stClssEPParam.ucMaxTargetPercent, 1, "MaxTargetPercent:"));
	TRACE_HEX(((char *)&stClssEPParam.ucTargetPercent, 1, "TargetPercent:"));
	TRACE_HEX(((char *)stClssEPParam.usDefaultDdol, stClssEPParam.ucDefaultDdolLen, "Default DDOL:"));
	TRACE_HEX(((char *)stClssEPParam.usDefaultTdol, stClssEPParam.ucDefaultTdolLen, "Default TDOL:"));
	TRACE_HEX(((char *)stClssEPParam.usCap, 3, "CAP:"));
	TRACE_HEX(((char *)stClssEPParam.usAddCap, 5,"ADDCAP:"));
	TRACE_HEX(((char *)&stClssEPParam.ucAppSelIndicator, 1, "ASI(PartialAID):"));
	TRACE_HEX(((char *)stClssEPParam.usIcs, 7, "ICS:"));
	TRACE_HEX(((char *)&stClssEPParam.ucStatus, 1, "TestType:"));
	TRACE_HEX(((char *)&stClssEPParam.ucEcIndicator, 1, "SUPPEC:"));
	TRACE_HEX(((char *)&stClssEPParam.ucLimitExist, 1, "_limit_exist:"));
	TRACE_HEX(((char *)stClssEPParam.usEcLimit, 6, "_ec_limit:"));
	TRACE_HEX(((char *)stClssEPParam.usClLimit, 6, "_cl_limit:"));
	TRACE_HEX(((char *)stClssEPParam.usClOfflineLimit, 6,"_cl_offline_limit:"));
	TRACE_HEX(((char *)stClssEPParam.usCvmLimit, 6, "_cvm_limit:"));
	TRACE(("---------------------\r"));

	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DumpEmvParam(void)
{
	int nRet;
	int i;
	int nRes;
	uint nNum;
	char sAid[20][17];
	char sCapk[50][6];
	emvparam stAID;
	capk stCAPK;
	
	nRet = EMV_EnumAID(sAid);
	if (nRet > 0)
	{
		TRACE(("AID_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{	
			stAID._aid_len = sAid[i][0];
			memcpy(stAID._aid, &sAid[i][1], sAid[i][0]);
			nRes = EMV_OperAID(&stAID,AID_GET);
			if (nRes == APP_SUCC)
			{
				TRACE(("---------------------\r"));
				TRACE_HEX(((char *)stAID._aid, stAID._aid_len,"AID:"));
				TRACE_HEX(((char *)stAID._app_ver, 2, "VER:"));
				TRACE_HEX(((char *)stAID._tac_default, 5, "TAC_Default:"));
				TRACE_HEX(((char *)stAID._tac_denial, 5, "TAC_Decline:"));
				TRACE_HEX(((char *)stAID._tac_online, 5, "TAC_Online :"));
				PubC4ToInt(&nNum, (uchar *)stAID._floorlimit);
				TRACE(("FloorLimit:%d\r", nNum));
				PubC4ToInt(&nNum, (uchar *)stAID._threshold_value);
				TRACE(("ThresholdValue:%d\r", nNum));
				TRACE_HEX(((char *)&stAID._max_target_percent, 1, "MaxTargetPercent:"));
				TRACE_HEX(((char *)&stAID._target_percent, 1, "TargetPercent:"));
				TRACE_HEX(((char *)stAID._default_ddol, stAID._default_ddol_len, "Default DDOL:"));
				TRACE_HEX(((char *)stAID._default_tdol, stAID._default_tdol_len, "Default TDOL:"));
				TRACE_HEX(((char *)stAID._cap, 3, "CAP:"));
				TRACE_HEX(((char *)stAID._add_cap, 5,"ADDCAP:"));
				TRACE_HEX(((char *)&stAID._app_sel_indicator, 1, "ASI(PartialAID):"));
				TRACE_HEX(((char *)stAID._ics, 7, "ICS:"));
				TRACE_HEX(((char *)&stAID._status, 1, "TestType:"));
				TRACE_HEX(((char *)&stAID._ec_indicator, 1, "SUPPEC:"));
				TRACE_HEX(((char *)&stAID._limit_exist, 1, "_limit_exist:"));
				TRACE_HEX(((char *)stAID._ec_limit, 6, "_ec_limit:"));
				TRACE_HEX(((char *)stAID._cl_limit, 6, "_cl_limit:"));
				TRACE_HEX(((char *)stAID._cl_offline_limit, 6,"_cl_offline_limit:"));
				TRACE_HEX(((char *)stAID._cvm_limit, 6, "_cvm_limit:"));
				TRACE(("---------------------\r"));
			}

		}
	}
	else
	{
		TRACE(("EMV_EnumAID=%d\r", nRet));
	}
	TRACE(("- - - - - - - X - - - - - - - X - - - - - - - "));
	
	nRet = EMV_EnumCAPK(0, 49, sCapk);
	if (nRet > 0)
	{
		TRACE(("CAPK_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{
			memcpy(stCAPK._rid, &sCapk[i][0], 5);
			stCAPK._index = sCapk[i][5];
			nRes = EMV_OperCAPK(&stCAPK, CAPK_GET);
			if (nRes == APP_SUCC)
			{
				TRACE(("[Index]:%02x ", stCAPK._index));
				TRACE_HEX(((char *)stCAPK._rid, 5,"[RID]:" ));
				TRACE_HEX(((char *)stCAPK._expired_date, 4,"[ExpDate]:"));
				TRACE_HEX(((char *)stCAPK._hashvalue, 20,"[HashValue]:"));
				TRACE(("[_pk_algorithm]:%02x ", stCAPK._pk_algorithm));
				TRACE(("[_hash_algorithm]:%02x ", stCAPK._hash_algorithm));
				TRACE(("[_disable]:%02x ", stCAPK._disable));				
				TRACE_HEX(((char *)stCAPK._resv, 3,"[_resv]:"));
				TRACE_HEX(((char *)stCAPK._key.pk_exponent, 3,"[pk_exponent]:"));
				TRACE(("[pk_mod_len]:%02x ", stCAPK._key.pk_mod_len));				
				TRACE_HEX(((char *)stCAPK._key.pk_modulus, MAX_MODULUS_LEN,"[pk_modulus]:"));
			}
			TRACE(("---------------------\r"));
		}
	}
	else
	{
		TRACE(("EMV_EnumCAPK=%d\r", nRet));
	}
	
	TRACE(("\r\r\r" ));
	TRACE(("- - - - - - - X - - - - - - - X - - - - - - - "));

	
	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int DumpEmvAIDParam(void)
{
	int nRet;
	int i;
	char sAid[20][17];
	STCLSSPARAM stClssEPParam;
	int nRes;
	
	nRet = EMV_EnumAID(sAid);
	if (nRet > 0)
	{
		TRACE(("AID_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{	
			stClssEPParam.ucAidLen= sAid[i][0];
			memcpy(stClssEPParam.usAid, &sAid[i][1], sAid[i][0]);
			nRes = SDK_EP_OperAID(&stClssEPParam,AID_GET);
			TRACE_HEX(((char *)stClssEPParam.usAid, stClssEPParam.ucAidLen,"AID:"));
			if (nRes == APP_SUCC)
			{
				DumpEmvL2OneAID(stClssEPParam);
			}
			else
			{
				TRACE(("EMVL2_OperAID error nRes=%d", nRes));
			}

		}
	}
	else
	{
		TRACE(("EMV_EnumAID=%d\r", nRet));
	}
	TRACE(("- - - - - - - X - - - - - - - X - - - - - - - "));

	
	TRACE(("\r\r\r" ));
	TRACE(("- - - - - - - X - - - - - - - X - - - - - - - "));

	
	return APP_SUCC;
}

/**
** brief: 
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int Kerneltest()
{
	uint unFileSize=0;
	STCLSSPARAM stClssParam;
	int nFd=0;
	int i=0;

	
	ASSERT(NDK_FsFileSize("kernel1.app", &unFileSize));
	TRACE(("file._conf_name: %s size=%d", "kernel1.app", unFileSize));
	TRACE(("STCLSSPARAM size=%d",sizeof(STCLSSPARAM)));

	nFd = NDK_FsOpen("kernel1.app","r");
	TRACE(("NDK_FsOpen kernel1.app nfd=%d", nFd));
	for(i=0;i<unFileSize/sizeof(STCLSSPARAM);i++)
	{
		NDK_FsRead(nFd,(char *)&stClssParam,sizeof(STCLSSPARAM));
		TRACE_HEX(((char *)stClssParam.usAid, 16, "AID:"));
		TRACE_HEX(((char *)stClssParam.usKernelId, 8, "kid:"));
	}
	ASSERT_FAIL(NDK_FsClose(nFd));
	return APP_SUCC;
}

