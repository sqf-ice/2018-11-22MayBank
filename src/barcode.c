/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  ********************             
** File name:  card.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: Jack
** Complete date: 2018-11-12
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"


//这个函数是关于在HTTP传输数据过程中的  状态，比如datain以及dataout等信息。
static int OnDebug(CURL *pcurl, curl_infotype itype, char * psData, size_t size, void *p)  
{  
	char szTmp[2048] = {0};	//根据官方手册说明，pData不一定是以0结尾的，但pData的长度肯定等于size，所以要借助这个变量处理下
	STSHOWINFOXY stShowInfoxy;
	
	if(size > sizeof(szTmp) - 1)
	{
		//碰到这种情况直接退出，防止跑飞
		TRACE(("pData Overlength"));  
		return 0; 
	}
	memcpy(szTmp, psData, size);
	
	if(itype == CURLINFO_TEXT)  
	{  
	    	TRACE(("[TEXT]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_HEADER_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
	    TRACE(("[HEADER_IN]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_HEADER_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	TRACE(("[HEADER_OUT]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_DATA_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
    	TRACE(("[DATA_IN]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_DATA_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	TRACE(("[DATA_OUT]:[%s]", szTmp)); 
	}  
	else if(itype == CURLINFO_SSL_DATA_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
    	//TRACE_HEX((szTmp, size, "[SSL_DATA_IN]:")); 
	} 
	else if(itype == CURLINFO_SSL_DATA_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	//TRACE_HEX((szTmp, size, "[SSL_DATA_OUT]:")); 
	} 
	return 0;  
}

static int DealParamReq(const char cTransType, STHTTPPACK * pstHttpPack)
{
	char szDateTime[14+1] = {0};
	char szBatchNo[6+1] = {0};
	char szTerminalid[8+1] = {0};
	char szTraceNo[6+1];
	char szTmpDomain[512] = {0};

	PubGetCurrentDatetime(szDateTime);
	GetVarTraceNo(szTraceNo);
	
	switch(cTransType)
	{
	case TRANS_QR_REGISTER:
		SetTmpDomain(DOMAIN_QR_REGISTER);
		break;
	case TRANS_QR_LOGON:
		SetTmpDomain(DOMAIN_QR_LOGON);
		break;
	default:
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
*@brief		Sprintf buff and calculate MD5
*@param		[in]			pstHttpPack
*@param		[out]		    psBuf
*@param     [out]			buffer length
**/
int HttpPack(STHTTPPACK* pstHttpPack, char *psBuf, int *pnLen)
{
	char szJson[4096] = {0};
	char szTmp[4096] = {0};
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pData = cJSON_CreateObject();

	
	switch(pstHttpPack->cTransType)
	{
	case TRANS_QR_REGISTER:
		cJSON_AddItemToObject(pRoot, "mbbregisterrequest", pData);
		cJSON_AddStringToObject(pData, "creation_date", pstHttpPack->szDateTime);
		cJSON_AddStringToObject(pData, "sub_mch_id", pstHttpPack->szMerchant_Id);
		cJSON_AddStringToObject(pData, "terminal_id", pstHttpPack->szTerminal_Id);
		cJSON_AddStringToObject(pData, "mac_id" , "Z4gocxFcafamVNA7gt9cOQ==");
		cJSON_AddStringToObject(pData, "chksum", pstHttpPack->szChkSum);
		break;
	case TRANS_QR_LOGON:
		cJSON_AddItemToObject(pRoot, "mbblogonrequest", pData);
		cJSON_AddStringToObject(pData, "creation_date", pstHttpPack->szDateTime);
		cJSON_AddStringToObject(pData, "sub_mch_id", pstHttpPack->szMerchant_Id);
		cJSON_AddStringToObject(pData, "terminal_id", pstHttpPack->szTerminal_Id);
		cJSON_AddStringToObject(pData, "mac_id" , "Z4gocxFcafamVNA7gt9cOQ==");
		cJSON_AddStringToObject(pData, "chksum", pstHttpPack->szChkSum);
		break;
	default:
		return APP_FAIL;
	}

	strcpy(szTmp, cJSON_PrintUnformatted(pRoot));	
	strcpy(psBuf, szTmp);
	*pnLen = strlen(psBuf);
	return APP_SUCC;
}

int GetJsonObj(cJSON* pRoot, const char* pszPairName, char* pszValue)
{
	cJSON * pSub = NULL;
	
	pSub = cJSON_GetObjectItem(pRoot, pszPairName);
	if (NULL == pSub)
	{
		return APP_FAIL;
	}
	if (pSub->type == cJSON_Number)
	{
		sprintf(pszValue, "%d", pSub->valueint);
	}
	else
	{
		if (NULL == pSub->valuestring)
		{
			return APP_QUIT;
		}
		strcpy(pszValue, pSub->valuestring);
	}
	return APP_SUCC;
}

int ParseRespondJson(STHTTPPACK* pstHttpPack)
{
	int nFile;
	int nLen = 0;
  	cJSON * pRoot = NULL;
	cJSON * pSub = NULL;
	cJSON * pSub2 = NULL;
	cJSON * pSubSub = NULL, *pSubSubSub = NULL;
	int nFd = 0;
	char sBuf[2048] = {0};
	char szValue[1024] = {0};
	char szContent[256] = {0};
	char sStatusCode[20];
	char sStatusDesc[20];
	char sEndData[20];
	char sSubMchId[20];
	char sTerminalId[20];
	char sMacId[30];
	char sChkSum[100];
	char sWorkKey[100];
	
	switch(pstHttpPack->cTransType)
	{
	case TRANS_QR_REGISTER:
		NDK_FsFileSize(REGISTERFILE, &nLen);	
		nFd = NDK_FsOpen(REGISTERFILE, "r");
		if (nFd <= 0)
		{
			return APP_FAIL;
		}
		NDK_FsRead(nFd, sBuf, (uint)nLen);
		NDK_FsClose(nFd);
	
		pRoot = cJSON_Parse(sBuf);
		if (NULL == pRoot)
		{
			goto Fail_Parse;
		}
		pSub = cJSON_GetObjectItem(pRoot, "mbbregisterresponse");
		if(pSub == NULL)
		{
			PubMsgDlg("Warn", "Get pSub error.", 3, 10);
			goto Fail_Parse;
		}
		GetJsonObj(pSub, "status_code", sStatusCode);
		GetJsonObj(pSub, "status_desc", sStatusDesc);
		GetJsonObj(pSub, "end_date", sEndData);
		GetJsonObj(pSub, "sub_mch_id", sSubMchId);
		GetJsonObj(pSub, "terminal_id", sTerminalId);
		GetJsonObj(pSub, "mac_id", sMacId);
		GetJsonObj(pSub, "chksum", sChkSum);

		PubDebug("\n\n%s\n\n", sStatusCode);
		PubDebug("\n\n%s\n\n", sStatusDesc);
		PubDebug("\n\n%s\n\n", sEndData);
		PubDebug("\n\n%s\n\n", sSubMchId);
		PubDebug("\n\n%s\n\n", sTerminalId);		
		PubDebug("\n\n%s\n\n", sMacId);
		PubDebug("\n\n%s\n\n", sChkSum);

		PubMsgDlg("status_code", sStatusCode, 0, 0);
		PubMsgDlg("status_desc", sStatusDesc, 0, 0);
		PubMsgDlg("end_date", sEndData, 0, 0);
		PubMsgDlg("sub_mch_id", sSubMchId, 0, 0);
		PubMsgDlg("terminal_id", sTerminalId, 0, 0);
		PubMsgDlg("mac_id", sMacId, 0, 0);
		PubMsgDlg("chksum", sChkSum, 0, 0);
		break;
	case TRANS_QR_LOGON:
		NDK_FsFileSize(LOGONFILE, &nLen);	
		nFd = NDK_FsOpen(LOGONFILE, "r");
		if (nFd <= 0)
		{
			return APP_FAIL;
		}
		NDK_FsRead(nFd, sBuf, (uint)nLen);
		NDK_FsClose(nFd);
	
		pRoot = cJSON_Parse(sBuf);
		if (NULL == pRoot)
		{
			goto Fail_Parse;
		}
		pSub = cJSON_GetObjectItem(pRoot, "mbblogonresponse");
		if(pSub == NULL)
		{
			PubMsgDlg("Warn", "Get pSub error.", 3, 10);
			goto Fail_Parse;
		}
		GetJsonObj(pSub, "status_code", sStatusCode);
		GetJsonObj(pSub, "status_desc", sStatusDesc);
		GetJsonObj(pSub, "end_date", sEndData);
		GetJsonObj(pSub, "sub_mch_id", sSubMchId);
		GetJsonObj(pSub, "terminal_id", sTerminalId);
		GetJsonObj(pSub, "mac_id", sMacId);
		GetJsonObj(pSub, "wkey", sWorkKey);
		GetJsonObj(pSub, "chksum", sChkSum);

		PubDebug("\n\nsStatusCode = %s\n\n", sStatusCode);
		PubDebug("\n\nsStatusDesc = %s\n\n", sStatusDesc);
		PubDebug("\n\nsEndData = %s\n\n", sEndData);
		PubDebug("\n\nsSubMchId = %s\n\n", sSubMchId);
		PubDebug("\n\nsTerminalId = %s\n\n", sTerminalId);
		PubDebug("\n\nsMacId = %s\n\n", sMacId);
		PubDebug("\n\nsWorkKey = %s\n\n", sWorkKey);
		PubDebug("\n\nsChkSum = %s\n\n", sChkSum);

		PubMsgDlg("status_code", sStatusCode, 0, 0);
		PubMsgDlg("status_desc", sStatusDesc, 0, 0);
		PubMsgDlg("end_date", sEndData, 0, 0);
		PubMsgDlg("sub_mch_id", sSubMchId, 0, 0);
		PubMsgDlg("terminal_id", sTerminalId, 0, 0);
		PubMsgDlg("mac_id", sMacId, 0, 0);
		PubMsgDlg("wkey", sWorkKey, 0, 0);
		PubMsgDlg("chksum", sChkSum, 0, 0);
		ASSERT_FAIL(LoadQrKey(BASE64_MASTER_KEY, sWorkKey));
		break;
	default:
		break;
	}
	
	cJSON_Delete(pRoot);
	return APP_SUCC;
	
Fail_Parse:
	cJSON_Delete(pRoot);
	return APP_FAIL;
}


/*
** @ brief: calculate checksum
** @ param [in]: source data
** @ param [out]: check sum buffer
** @return 
** @li APP_SUCC
** @li APP_FAIL
*/
int CheckSum(const char *psData, char *psChkSum)
{
	char sDataIn[512] = {0};
	char szDataOut[32+1] = {0};
	char szChkSum[64+1] = {0};
	int nLen, nRet;
	strcpy(sDataIn, psData);
	nLen = strlen(sDataIn);				    
	TRACE(("sDataIn = %s\n", sDataIn));
	TRACE(("nLen = %d\n", nLen));
	nRet = NDK_AlgSHA256(sDataIn, nLen, szDataOut);
	if(nRet != APP_SUCC)
	{
		PubMsgDlg(NULL, "check wrong", 3, 5);
	}
	nLen = sizeof(szDataOut);
	PubDebugData("check sum", szDataOut, nLen);
	PubHexToAsc(szDataOut, 64, 0, szChkSum);
	strcpy(psChkSum, szChkSum);
	return APP_SUCC;
}

/*
** @ brief: Save hex and base64 work key to file.
** @ param [in]: source base64 Main Key
** @ param [in]: source encrypted base64 Work Key
** @return 
** @li APP_SUCC
** @li APP_FAIL
*/
int LoadQrKey(char *psMKey, char *psWKey)
{
	int nRet = 0, nIndex = 0, nLen = 0, nLenTmk = 0, nLenWKey;
	int nFd;	
	char sTmk[32] = {0};			//hex main key
	char sBcdWKey[48] = {0};		//encrypted hex work key
	char sHexWKey[64] = {0};		//hex work key
	char sBaseWKey[48] = {0};		//base64 work key
	//char sIV[] = {0x30,0x32,0x30,0x33,0x30,0x34,0x30,0x35,0x30,0x36,0x30,0x37,0x30,0x38,0x30,0x39};
	char sIV[] = {0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,0x00,0x09};
	//char sIV[] = "0203040506070809";

	aes_context aes_ctx;
	PubDebug("\n\n Load: psMKey = %s\n\n", psMKey);
	PubDebug("\n\n Load: psWKey = %s\n\n", psWKey);

	aes_init(&aes_ctx);
	base64_decode(psMKey, sTmk, &nLenTmk);
	base64_decode(psWKey, sBcdWKey, &nLenWKey);

	PubDebugData("main key: ", sTmk, nLenTmk);	
	PubDebug("\n\n nLenTmk = %d\n\n", nLenTmk);

	PubDebugData("work key: ", sBcdWKey, nLenWKey);
	PubDebug("\n\n nLenWKey = %d\n\n", nLenWKey);

	aes_setkey_dec(&aes_ctx, sTmk, 256);
	aes_crypt_cbc(&aes_ctx, AES_DECRYPT, 48, sIV, sBcdWKey, sHexWKey);

	PubDebugData("*hex Work Key: ", sHexWKey, 32);
	PubDebug("\n\nPubDebug Hex Work Key: %s\n\n", sHexWKey);
	
	base64_encode(sHexWKey, 32, sBaseWKey);
	PubDebugData("#base Work Key: ", sBaseWKey, 48);
	PubDebug("\n\nPubDebug Base64 Work Key: %s\n\n", sBaseWKey);
	/**
	* Write hex work key to file.
	*/
	NDK_FsDel(HEX_WORKKEY_FILE);			
	nFd = NDK_FsOpen(HEX_WORKKEY_FILE, "w");
	if(nFd < 0)
	{	
		NDK_FsClose(nFd);
		PubDispErr("Open hex work key file fail.");
		return APP_FAIL;
	}	
	if(NDK_FsWrite(nFd, sHexWKey, 32)<0)
	{
		NDK_FsClose(nFd);
		PubDispErr("Save hex work key fail.");
		return APP_FAIL;
	}
	NDK_FsClose(nFd);
	
	
	/**
	* Write base64 work key to file.
	*/
	NDK_FsDel(BASE64_WORKKEY_FILE);			
	nFd = NDK_FsOpen(BASE64_WORKKEY_FILE, "w");
	if(nFd < 0)
	{	
		NDK_FsClose(nFd);
		PubDispErr("Open base64 work key file fail.");
		return APP_FAIL;
	}	
	if(NDK_FsWrite(nFd, sBaseWKey, 48)<0)
	{
		NDK_FsClose(nFd);
		PubDispErr("Save base64 work key fail.");
		return APP_FAIL;
	}
	NDK_FsClose(nFd);

	/**
	* Read hex work key from file.
	*/			
	memset(sHexWKey, 0, sizeof(sHexWKey));
	nFd = NDK_FsOpen(HEX_WORKKEY_FILE, "r");
	if(nFd < 0)
	{	
		NDK_FsClose(nFd);
		PubDispErr("Open hex work key file fail.");
		return APP_FAIL;
	}	
	if(NDK_FsRead(nFd, sHexWKey, 32)<0)
	{
		NDK_FsClose(nFd);
		PubDispErr("Read hex work key fail.");
		return APP_FAIL;
	}
	NDK_FsClose(nFd);	
	PubDebug("\n\n Read Hex work Key From File.\n\n");
	PubDebugData("*hex Work Key: ", sHexWKey, 32);
	PubDebug("\n\nPubDebug Hex Work Key: %s\n\n", sHexWKey);


	/**
	* Read Base64 work key from file.
	*/			
	memset(sBaseWKey, 0, sizeof(sBaseWKey));
	nFd = NDK_FsOpen(BASE64_WORKKEY_FILE, "r");
	if(nFd < 0)
	{	
		NDK_FsClose(nFd);
		PubDispErr("Open base64 work key file fail.");
		return APP_FAIL;
	}	
	if(NDK_FsRead(nFd, sBaseWKey, 48)<0)
	{
		NDK_FsClose(nFd);
		PubDispErr("Read base64 work key fail.");
		return APP_FAIL;
	}
	NDK_FsClose(nFd);	
	PubDebug("\n\n Read Hex work Key From File.\n\n");
	PubDebugData("*hex Work Key: ", sBaseWKey, 48);
	PubDebug("\n\nPubDebug Hex Work Key: %s\n\n", sBaseWKey);
	return APP_SUCC;
}

/*
** @ brief: encrypte data by work key
** @ param [in]: source data
** @ param [out]: check sum buffer
** @return 
** @li APP_SUCC
** @li APP_FAIL
*/
int EncryptByWKey(const char *pInData, char *pOutData)
{
	int nFd;
	char sHexWKey[32] = {0};
	//char sIV[] = {0x30,0x32,0x30,0x33,0x30,0x34,0x30,0x35,0x30,0x36,0x30,0x37,0x30,0x38,0x30,0x39};
	char sIV[] = {0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,0x00,0x09};
	char sHexRes[64] = {0};
	
	aes_context aes_ctx;
	/**
	* Read hex work key from file.
	*/
	NDK_FsDel(HEX_WORKKEY_FILE);			
	nFd = NDK_FsOpen(HEX_WORKKEY_FILE, "w");
	if(nFd < 0)
	{	
		NDK_FsClose(nFd);
		PubDispErr("Open work key file fail.");
		return APP_FAIL;
	}	
	if(NDK_FsRead(nFd, 32, sHexWKey)<0)
	{
		NDK_FsClose(nFd);
		PubDispErr("Save work key fail.");
		return APP_FAIL;
	}
	NDK_FsClose(nFd);

	aes_init(&aes_ctx);
	aes_setkey_enc(&aes_ctx, sHexWKey, 256);
	aes_crypt_cbc(&aes_ctx, AES_DECRYPT, 48, sIV, pInData, sHexRes);
	base64_encode(sHexRes, 48, pOutData);
	
	return APP_SUCC;
}

int Register()
{
	STTAANSRECORD stWater;
	STHTTPPACK stHttpPack;
	STREVERSAL stReversal;
	char *pszTitle = "Register";
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char sChkSum[64] = {0};
	char szDateTime[14+1] = {0};
	char sData[512] = {0};
	int nLen, nRet;
	
	memset(&stWater, 0, sizeof(STTAANSRECORD));
	memset(&stHttpPack, 0, sizeof(STHTTPPACK));
	stHttpPack.cTransType = TRANS_QR_REGISTER;
	PubGetCurrentDatetime(szDateTime);
	strcpy(stHttpPack.szDateTime, szDateTime);
	strcpy(stHttpPack.szMerchant_Id, "000027001059617");
	strcpy(stHttpPack.szTerminal_Id, "11051316");
	strcpy(stHttpPack.szMac_Id, "010101010101");
	
	strcpy(sData, stHttpPack.szDateTime);  
	strcat(sData, stHttpPack.szMerchant_Id);
	strcat(sData, stHttpPack.szTerminal_Id);
	strcat(sData, stHttpPack.szMac_Id);
	strcat(sData, BASE64_MASTER_KEY);

	memset(&stWater, 0, sizeof(STTAANSRECORD));
	CheckSum(sData, sChkSum);	
	memcpy(stHttpPack.szChkSum, sChkSum, 64);
	ASSERT_QUIT(DealPosLimit());
	ASSERT_QUIT(DealParamReq(stHttpPack.cTransType, &stHttpPack));

	PubDisplayTitle(pszTitle);
	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	
	HttpPack(&stHttpPack, sPackBuf, &nLen);
	ASSERT_HANGU_PPP_FAIL(OpenPPP());

	ASSERT_HANGU_PPP_FAIL(HttpSendRecv(sPackBuf, &nLen, stHttpPack.cTransType));
	ClosePPP(1);

	nRet = ParseRespondJson(&stHttpPack);
	if (APP_FUNCQUIT == nRet)
	{
		return APP_SUCC;
	}
	else if (APP_SUCC != nRet)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

int Logon()
{
	STTAANSRECORD stWater;
	STHTTPPACK stHttpPack;
	STREVERSAL stReversal;
	char *pszTitle = "Logon";
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char sChkSum[64] = {0};
	char szDateTime[14+1] = {0};
	char sData[512] = {0};
	int nLen, nRet;
	
	memset(&stWater, 0, sizeof(STTAANSRECORD));
	memset(&stHttpPack, 0, sizeof(STHTTPPACK));
	stHttpPack.cTransType = TRANS_QR_LOGON;
	PubGetCurrentDatetime(szDateTime);
	strcpy(stHttpPack.szDateTime, szDateTime);
	strcpy(stHttpPack.szMerchant_Id, "000027001059617");
	strcpy(stHttpPack.szTerminal_Id, "11051316");
	strcpy(stHttpPack.szMac_Id, "010101010101");
	
	strcpy(sData, stHttpPack.szDateTime);  
	strcat(sData, stHttpPack.szMerchant_Id);
	strcat(sData, stHttpPack.szTerminal_Id);
	strcat(sData, stHttpPack.szMac_Id);
	strcat(sData, BASE64_MASTER_KEY);

	memset(sChkSum, 0, sizeof(sChkSum));
	CheckSum(sData, sChkSum);
	memcpy(stHttpPack.szChkSum, sChkSum, 64);
	ASSERT_QUIT(DealPosLimit());
	ASSERT_QUIT(DealParamReq(stHttpPack.cTransType, &stHttpPack));

	PubDisplayTitle(pszTitle);

	HttpPack(&stHttpPack, sPackBuf, &nLen);
	ASSERT_HANGU_PPP_FAIL(OpenPPP());

	ASSERT_HANGU_PPP_FAIL(HttpSendRecv(sPackBuf, &nLen, stHttpPack.cTransType));
	ClosePPP(1);

	nRet = ParseRespondJson(&stHttpPack);
	if (APP_FUNCQUIT == nRet)
	{
		return APP_SUCC;
	}
	else if (APP_SUCC != nRet)
	{
		return APP_FAIL;
	}
	
	return APP_SUCC;
}

int WalletId()
{
	STTAANSRECORD stWater;
	STHTTPPACK stHttpPack;
	STREVERSAL stReversal;
	char *pszTitle = "Wallet ID";
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char sChkSum[64] = {0};
	char szDateTime[14+1] = {0};
	char sData[512] = {0};
	int nLen, nRet;
	
	memset(&stWater, 0, sizeof(STTAANSRECORD));
	memset(&stHttpPack, 0, sizeof(STHTTPPACK));
	stHttpPack.cTransType = TRANS_QR_WALLETID;
	strcpy(stHttpPack.szTable_Id, "T000000001");
	strcpy(stHttpPack.szMerchant_Id, "121212");
	strcpy(stHttpPack.szTerminal_Id, "013467007045764");
	strcpy(stHttpPack.szMac_Id, "010101010101");

	
	strcpy(sData, stHttpPack.szTable_Id);  
	strcat(sData, stHttpPack.szMerchant_Id);
	strcat(sData, stHttpPack.szTerminal_Id);
	strcat(sData, stHttpPack.szMac_Id);
	strcat(sData, BASE64_MASTER_KEY);

	memset(sChkSum, 0, sizeof(sChkSum));
	CheckSum(sData, sChkSum);
	memcpy(stHttpPack.szChkSum, sChkSum, 64);
	ASSERT_QUIT(DealPosLimit());
	ASSERT_QUIT(DealParamReq(stHttpPack.cTransType, &stHttpPack));

	PubDisplayTitle(pszTitle);

	HttpPack(&stHttpPack, sPackBuf, &nLen);
	ASSERT_HANGU_PPP_FAIL(OpenPPP());

	ASSERT_HANGU_PPP_FAIL(HttpSendRecv(sPackBuf, &nLen, stHttpPack.cTransType));
	ClosePPP(1);

	nRet = ParseRespondJson(&stHttpPack);
	if (APP_FUNCQUIT == nRet)
	{
		return APP_SUCC;
	}
	else if (APP_SUCC != nRet)
	{
		return APP_FAIL;
	}	
	return APP_SUCC;
}


int QRPay()
{
	STTAANSRECORD stWater;
	STHTTPPACK stHttpPack;
	STREVERSAL stReversal;
	char *pszTitle = "QR Payment";
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char sChkSum[64] = {0};
	char szDateTime[14+1] = {0};
	char sData[512] = {0};
	int nLen, nRet;
	
	memset(&stWater, 0, sizeof(STTAANSRECORD));
	memset(&stHttpPack, 0, sizeof(STHTTPPACK));
	stHttpPack.cTransType = TRANS_QR_PAYMENT;
	PubGetCurrentDatetime(szDateTime);
	strcpy(stHttpPack.szDateTime, szDateTime);
	strcpy(stHttpPack.szMerchant_Id, "000027001059617");
	strcpy(stHttpPack.szTerminal_Id, "11051316");
	strcpy(stHttpPack.szMac_Id, "010101010101");
	strcpy(stHttpPack.szAppId, "MB000001");
	strcpy(stHttpPack.szQrCode, "");
	strcpy(stHttpPack.szOptIn, "Y");
	strcpy(stHttpPack.szOrderNo, "");
	strcpy(stHttpPack.szTotalAmt, "");
	strcpy(stHttpPack.szCurrency, "MYR");
	strcpy(stHttpPack.szTerminal_Ip, "8.8.8.8");
	strcpy(stHttpPack.szSub_Mch_Id, "1900000109");
	//strcpy(stHttpPack.szWorkKey, );


	
	strcpy(sData, stHttpPack.szDateTime);  
	strcat(sData, stHttpPack.szMerchant_Id);
	strcat(sData, stHttpPack.szTerminal_Id);
	strcat(sData, stHttpPack.szMac_Id);
	strcat(sData, BASE64_MASTER_KEY);

	memset(sChkSum, 0, sizeof(sChkSum));
	CheckSum(sData, sChkSum);
	memcpy(stHttpPack.szChkSum, sChkSum, 64);
	ASSERT_QUIT(DealPosLimit());
	ASSERT_QUIT(DealParamReq(stHttpPack.cTransType, &stHttpPack));

	PubDisplayTitle(pszTitle);

	HttpPack(&stHttpPack, sPackBuf, &nLen);
	ASSERT_HANGU_PPP_FAIL(OpenPPP());

	ASSERT_HANGU_PPP_FAIL(HttpSendRecv(sPackBuf, &nLen, stHttpPack.cTransType));
	ClosePPP(1);

	nRet = ParseRespondJson(&stHttpPack);
	if (APP_FUNCQUIT == nRet)
	{
		return APP_SUCC;
	}
	else if (APP_SUCC != nRet)
	{
		return APP_FAIL;
	}
	
	return APP_SUCC;
}


int BarcodePayment(void)
{
    int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	int nKey = 0;
	int nMenuItem = 0;
	char *pszItems[] = {
		tr("1.Register"),
		tr("2.Logon"),
		tr("3.WalletId")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("Barcode Payment"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			Register();
			break;
		case 2:
			Logon();
			break;
		case 3:
			WalletId();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}


