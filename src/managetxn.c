/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  E:\NEWLAND\lastest work\paysys\1\code\bank-demo\src\managetxn.c
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  E:\NEWLAND\lastest work\paysys\1\code\bank-demo\src\managetxn.c
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  managetxn.c
** File indentifier: 
** Brief:  Manage transaction processing module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
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
* Interface function implementation
*/
/**
* @brief Get Necessary Field
* @param pstSystem 
* @return void
*/
void DealSystem(STSYSTEM *pstSystem)
{
	char szDateTime[14+1];
	
	GetVarTerminalId(pstSystem->szPosID, pstSystem->nAcqIndex);
	GetVarMerchantId(pstSystem->szShopID, pstSystem->nAcqIndex);
	PubAddSymbolToStr(pstSystem->szShopID, 15, ' ', 1);
PubDebug("&&&&&& index: %d, [tid=%s][mid=%s][nii: %s]", pstSystem->nAcqIndex, pstSystem->szPosID, pstSystem->szShopID,pstSystem->szNii);
	GetVarBatchNo(pstSystem->szBatchNum, pstSystem->nAcqIndex);	
	GetVarInvoiceNo(pstSystem->szInvoice);
	memset(szDateTime, 0, sizeof(szDateTime));
	PubGetCurrentDatetime(szDateTime);
	memcpy(pstSystem->szDate, szDateTime + 4, 4);
	memcpy(pstSystem->szTime, szDateTime + 8, 6);

	GetVarTraceNo(pstSystem->szTrace);	
	GetVarCommTransNii(pstSystem->szNii, pstSystem->nAcqIndex);
	ChangeCommTransTpdu(pstSystem->nAcqIndex);

	return ;
}

/**
** brief: Check Has login
** param [in]: 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-9-21
** modify: 
*/
int ChkLoginStatus(void)
{
	return 0;// for show, no need to login
	if (YES != GetVarIsLogin())
	{
		PubMsgDlg(NULL, "PLS SIGN IN", 3, 1);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
** brief: Check Message ID is Match
** param [in]: psReqMsgID request MSGID ASCII
** param [in]: psRespMsgID respond MSGID BCD
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-9-21
** modify: 
*/
int ChkRespMsgID(const char *psReqMsgID, const char *psRespMsgID)
{
	uchar szReqMsgId[2+1];
	char *pszTitle = "UNPACK";

	PubAscToHex((uchar *)psReqMsgID, 4, 0, szReqMsgId);
	szReqMsgId[1] |= 0x10;
	if (memcmp(szReqMsgId, psRespMsgID, 2) != 0)
	{
		PubMsgDlg(pszTitle, "MSG ID MISMATCH", 3, 10);
		TRACE_HEX(((char *)szReqMsgId,2,"szReqMsgId:"));
		TRACE_HEX(((char *)psRespMsgID,2,"psRespMsgID:"));
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Check some fields received is match to the fields sent 
* @param in pstSystem 
* @param in psBitmap 
* @return @li APP_SUCC 
*		@li APP_FAIL
*/
int ChkRespon(STSYSTEM *pstSystem, const char *psBitmap)
{
	struct
	{
		char szProcCode[6];			/**<F3 */
		char szAmount[12+1];			/**<F4 */
		char szTrace[6+1];			/**<F11 */
		char szServerCode[2+1];		/**<F25 */
		char szPosID[8+1];			/**<F41  */
		char szShopID[15+1];			/**<F42  */
		char szCurrencyCode[3+1];	/**<F49 */
	}stMustChkResp;
	int nFieldLen = 0, nBitmapLen = 8;
	int i, j, n;
	uchar cBitMask;
	char *pszTitle = "UNPACK";

	if (NULL == psBitmap)
	{
		return APP_FAIL;
	}
	memset(&stMustChkResp, 0, sizeof(stMustChkResp));
	if (psBitmap[0] & 0x80)
	{
		nBitmapLen = 16;
	}
	else
	{
		nBitmapLen = 8;
	}

	n = 0 ;
	for (i = 0; i < nBitmapLen; i++)
	{
		cBitMask = 0x80 ;
		for(j=1; j<9; j++, cBitMask>>=1)
		{
			if ((psBitmap[i] & cBitMask) == 0) 
				continue ;
			n = (i<<3) + j;
			switch(n)
			{
			case 2:
				nFieldLen = 19;
				ASSERT_FAIL(GetField(n, pstSystem->szPan, &nFieldLen));		
				break;
			case 3:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, stMustChkResp.szProcCode, &nFieldLen));		
				if (memcmp(stMustChkResp.szProcCode, pstSystem->szProcCode, 5) != 0)
				{
					PubDebug("====== MISmatch process");
					//PubMsgDlg(pszTitle, "PROCESS CODE MISMATCH", 3, 10);
					//return APP_FAIL;
				}
				break;
			case 4:
				nFieldLen = 12;
				if (TRANS_BALANCE == pstSystem->cTransType)
				{
					ASSERT_FAIL(GetField(n, pstSystem->szAmount, &nFieldLen));
				}
				break;
			case 11:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, stMustChkResp.szTrace, &nFieldLen));
				if (APP_SUCC != PubIsDigitStr(stMustChkResp.szTrace))
				{
					continue;
				}
				if (memcmp(stMustChkResp.szTrace, pstSystem->szTrace, 6) != 0)
				{
					if (AtoLL(stMustChkResp.szTrace) != AtoLL(pstSystem->szTrace))
					{
						PubMsgDlg(pszTitle, "STAN MISMATCH", 3, 10);
						return APP_FAIL;
					}
				}
				break;
			case 12:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, pstSystem->szTime, &nFieldLen));
				break;
			case 13:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szDate, &nFieldLen));
				break;
			case 14:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szExpDate, &nFieldLen));
				break;
			case 15:
				nFieldLen = 4;
				ASSERT_FAIL(GetField(n, pstSystem->szSettleDate, &nFieldLen));
				break;
			case 25:
				nFieldLen = 2;
				ASSERT_FAIL(GetField(n, stMustChkResp.szServerCode, &nFieldLen));
				if (memcmp(stMustChkResp.szServerCode, pstSystem->szServerCode, 2) != 0)
				{
					PubMsgDlg(pszTitle, "CONDITION CODE MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 37:
				nFieldLen = 12;
				ASSERT_FAIL(GetField(n, pstSystem->szRefnum, &nFieldLen));
				break;
			case 38:
				nFieldLen = 6;
				ASSERT_FAIL(GetField(n, pstSystem->szAuthCode, &nFieldLen));
				break;
			case 39:
				nFieldLen = 2;
				ASSERT_FAIL(GetField(n, pstSystem->szResponse, &nFieldLen));
				break;
			case 41:
				nFieldLen = 8;
				ASSERT_FAIL(GetField(n, stMustChkResp.szPosID, &nFieldLen));
				if (memcmp(stMustChkResp.szPosID, pstSystem->szPosID, 8) != 0)
				{
					PubMsgDlg(pszTitle, "TID MISMATCH", 3, 10);
					return APP_FAIL;
				}
				break;
			case 42:	
				nFieldLen = 15;
				ASSERT_FAIL(GetField(n, stMustChkResp.szShopID, &nFieldLen));
				if (memcmp(stMustChkResp.szShopID, pstSystem->szShopID, 15) != 0)
				{
//20171205 sit 测试时发现
//发送 30 30 30 30 32 37 30 30 38 30 38 38 30 31 35 
//返回 30 32 37 30 30 38 30 38 38 30 31 35 20 20 20 
					TRACE(("recv=%s send=%s", stMustChkResp.szShopID,pstSystem->szShopID));
					//PubMsgDlg(pszTitle, "MID MISMATCH", 3, 10);
					//return APP_FAIL;
				}
				break;
			case 48:
				nFieldLen = 3;
				ASSERT_FAIL(GetField(n, (char *)&(pstSystem->szCVV2), &nFieldLen));
				break;
			case 64:
				pstSystem->cMustChkMAC = 0x01;
				break;
			default:
				break;
			}
		}
	}
	return APP_SUCC;
}

/**
* @brief Check the limits of terminal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int ChkPosLimit(void)
{
	int nNowWaterSum = 0;
	int nPosMaxWaterSum;
	char szMsg[48+1];

	GetRecordNum(&nNowWaterSum);
	GetVarMaxTransCount(&nPosMaxWaterSum);

	if (nNowWaterSum >= nPosMaxWaterSum)
	{
		sprintf(szMsg, "TXN Log is Up to %d,Pls Settle", nNowWaterSum);
		PubMsgDlg("Warning", szMsg, 3, 30);
		return APP_FAIL;
	}
	return APP_SUCC;
}

//Reboot after 2minutes
int CheckAutoReboot()
{
	int nTime = 0;
	char szVer[64]={0};

	NDK_Getlibver(szVer);
	
	if ((strcmp(szVer, "6.1.0") >= 0) && szVer[0] != 'V')
	{
//	TRACE(("NDK_SecGetTimeToReboot..."));	
		if (APP_SUCC != NDK_SecGetTimeToReboot((uint *)&nTime))//取不到不判断
		{
			return APP_SUCC;
		}

		if (nTime <= 2*60)
		{
			PubClearAll();
			PubDisplayGen(3, "Reboot...");
			PubUpdateWindow();
			NDK_SysReboot();
		}
	}
	
	return APP_SUCC;
}

/**
* @brief Precess the limits of terminal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int DealPosLimit(void)
{
	int nNowWaterSum = 0;
	int nPosMaxWaterSum;
	int nRet;
	ulong nSpace;

	GetRecordNum(&nNowWaterSum);
	GetVarMaxTransCount(&nPosMaxWaterSum);
	NDK_FsGetDiskSpace(1, &nSpace);

	nRet = DealSettleTask();
	 if (APP_SUCC != nRet)
	{
		return nRet;
	}

	if (nNowWaterSum >= nPosMaxWaterSum || nSpace <= (10*1024))
	{
		nRet = PubConfirmDlg("Warning", "Translog is out of limit,pls settle", 3, 30);
		if (nRet == APP_SUCC)
		{
			if (Settle(0) != APP_SUCC)
			{
				PubMsgDlg("Settle Fail", "Pls re settle", 3, 10);
				return APP_FAIL;
			}
			//ASSERT(SetVarIsLogin(NO));
			
			return APP_QUIT;
		}
		else
		{
			return APP_FAIL;
		}
	}

	ASSERT_FAIL(DealPrintLimit());
	
	return APP_SUCC;
}

void DealSend()
{
	int nMaxSendNum = 0;

	GetVarMaxOffSendNum(&nMaxSendNum);
	if(GetVarOfflineUnSendNum() >= nMaxSendNum)
	{
		PubClearAll();
		EmvSendTC();
		SendOffline(0);
		CommHangUp();		
	}
}


/**
* @brief Check the switch of Transaction is Enable
* @param void
* @return @li APP_SUCC support
*		@li APP_FAIL nonsupport
*/
int ChkTransOnOffStatus(char cTransType)
{
#if 0
#ifdef DEMO
	return APP_SUCC;
#endif
#endif
	if (YES != GetTransSwitchOnoff(cTransType))
	{
		PubMsgDlg(NULL, "NONSUPPORT", 3, 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

int PinUpdate()
{	
	char *pszTitle = "TEPIN UPDATE";
	char szTeid[12+1] = {0};
	char szTepin[12+1] = {0};
	char szTeNewpin[12+1] = {0};
	STSYSTEM stSystem;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen;
	int nPos = 0;
	char sHash1[16+1] = {0}, sHash2[16+1] = {0}, sHash3[16+1] = {0};
	int nLen = 0;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	
	ASSERT_QUIT(GetTEIdPin(pszTitle, szTeid, szTepin));
	nLen = 0;
	ASSERT_QUIT(PubInputDlg(pszTitle, "Input NEW TE Pin:", szTeNewpin, &nLen, 1, 8, 60, INPUT_MODE_STRING));

PubDebug("szTeid: %s", szTeid);
PubDebug("szTepin: %s", szTepin);
PubDebug("szTeNewpin: %s", szTeNewpin);
	PINHASH(szTeid, szTepin, sHash1);
PubDebug("psHash: %s", sHash1);
	PINHASH(szTeid, szTeNewpin, sHash2);
	PINUpdateHash(sHash1, sHash2,sHash3);
PubDebug("pinupdate hash: %s", sHash3);	
	TXNHASH(sHash1, sHash2, stSystem.nAcqIndex);
PubDebug("TXN hash: %s", sHash2);	
	memset(&stSystem, 0, sizeof(STSYSTEM));

	PubDisplayTitle(pszTitle);
	/**
	* Dialing
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	
	/**
	*
	*/
	DealSystem(&stSystem);
	GetVarCommSecurityNii(stSystem.szNii);
	ChangeCommTransTpdu(ACQUIRER_NUM+1);
	
	memcpy(stSystem.szMsgID, "0800", 4);
	memcpy(stSystem.szProcCode, "970300", 6);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(SetField(0, stSystem.szMsgID, 4));		
	ASSERT_HANGUP_FAIL(SetField(3, stSystem.szProcCode, 6));
	ASSERT_HANGUP_FAIL(SetField(11, stSystem.szTrace, 6));
	ASSERT_HANGUP_FAIL(SetField(24, stSystem.szNii, 3));

	ASSERT_HANGUP_FAIL(SetField(41, stSystem.szPosID, 8));
	ASSERT_HANGUP_FAIL(SetField(42, stSystem.szShopID, 15));

	//DE62
	memcpy(stSystem.szField62+nPos, "HTLE", 4);
	nPos+=4;
	memcpy(stSystem.szField62+nPos, "04", 2);
	nPos+=2;
	memcpy(stSystem.szField62+nPos, szTeid, 8);
	nPos+=8;
	memcpy(stSystem.szField62+nPos, sHash2, 8);
	nPos+=8;
	memcpy(stSystem.szField62+nPos, sHash3, 8);
	nPos+=8;
	ASSERT_HANGUP_FAIL(SetField(62, stSystem.szField62, nPos));
	
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
	
	IncVarTraceNo();

	ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));
	ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));

	CommHangUp();

	ASSERT_FAIL(Unpack(sPackBuf, nPackLen));
	ASSERT_FAIL(ChkRespMsgID("0800", sPackBuf));
	ASSERT_FAIL(ChkRespon(&stSystem, sPackBuf + 2));

	PubClearAll();
	
	if (memcmp(stSystem.szResponse, "00", 2) != 0)
	{
		if (memcmp(stSystem.szResponse, "LE", 2) == 0)
		{
			DispRespEFTInfo();
		}
		else
		{
			DispResp(stSystem.szResponse, stSystem.nAcqIndex);
		}
		return APP_QUIT;
	}
	
	PubMsgDlg(pszTitle, "RENEW PIN OK", 1, 1);
	return APP_SUCC;
}

#define RSAKEYFILE "GenRsaFile"
int DukptDownload()
{
	char *pszTitle = "LTMK KEY";
	char szTeid[12+1] = {0};//"00000010";
	char szTepin[12+1] = {0};//"22222222";
	ST_RSA_PUBLIC_KEY stPubKey;
	ST_RSA_PRIVATE_KEY stPrivate;
	STSYSTEM stSystem;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen;
	int nPos = 0;
	char sHash1[16+1] = {0}, sHash3[16+1] = {0};
	char szDeviceSn[32] = {0};
	int nLen = 0;
	char szAcqId[3+1] = {0};
	char szTID[8+1] = {0}, szVendorId[8+1] = {0};
	char sBcdModulus[256+1] = {0};


	memset(&stPubKey, 0, sizeof(ST_RSA_PUBLIC_KEY));
	memset(&stPrivate, 0, sizeof(ST_RSA_PRIVATE_KEY));
	memset(&stSystem, 0, sizeof(STSYSTEM));
	
	stSystem.nAcqIndex = 0;
	
	ASSERT_QUIT(GetTEIdPin(pszTitle, szTeid, szTepin));
	
#if 1//shtest
	if (NDK_OK == NDK_FsExist(RSAKEYFILE))
	{	
		nLen = sizeof(ST_RSA_PUBLIC_KEY);
		GetFromFile((char *)&stPubKey, 0, &nLen, RSAKEYFILE);
		nLen = sizeof(ST_RSA_PRIVATE_KEY);
		GetFromFile((char *)&stPrivate, sizeof(ST_RSA_PUBLIC_KEY), &nLen, RSAKEYFILE);
	}
	else
#endif
	{
		PubClear2To4();
		PubDisplayGen(2, "Generating KEYS");
		PubDisplayGen(3, "Please Wait...");
		PubUpdateWindow();
PubDebug("begin");
		ASSERT_QUIT(NDK_AlgRSAKeyPairGen(RSA_KEY_LEN_2048, RSA_EXP_10001, &stPubKey, &stPrivate));
PubDebug("over");
#if 1//shtest
		SaveToFile((char *)&stPubKey, 0, sizeof(ST_RSA_PUBLIC_KEY), RSAKEYFILE);
		SaveToFile((char *)&stPrivate, sizeof(ST_RSA_PUBLIC_KEY), sizeof(ST_RSA_PRIVATE_KEY), RSAKEYFILE);
#endif
	}
PubDebug("exp: %s", stPubKey.exponent);
PubDebug("modulus: %s", stPubKey.modulus);
//PubDebug("pszTEid: %s[len=%d]", szTeid,strlen(szTeid) );

	PINHASH(szTeid, szTepin, sHash1);
	memset(sHash3, 0, sizeof(sHash3));

	PubAscToHex(stPubKey.modulus, 512, 0, (uchar *)sBcdModulus);
	TXNHASH3(sHash1, sBcdModulus, 256, sHash3, stSystem.nAcqIndex);
	memset(&stSystem, 0, sizeof(STSYSTEM));

	PubDisplayTitle(pszTitle);
	/**
	* Dialing
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	
	/**
	*
	*/
	DealSystem(&stSystem);
	GetVarCommSecurityNii(stSystem.szNii);
	ChangeCommTransTpdu(ACQUIRER_NUM+1);
	memcpy(stSystem.szMsgID, "0800", 4);
	memcpy(stSystem.szProcCode, "970000", 6);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(SetField(0, stSystem.szMsgID, 4));		
	ASSERT_HANGUP_FAIL(SetField(3, stSystem.szProcCode, 6));
	ASSERT_HANGUP_FAIL(SetField(11, stSystem.szTrace, 6));
	ASSERT_HANGUP_FAIL(SetField(24, stSystem.szNii, 3));

	ASSERT_HANGUP_FAIL(SetField(41, stSystem.szPosID, 8));
	ASSERT_HANGUP_FAIL(SetField(42, stSystem.szShopID, 15));

	//DE62
	memcpy(stSystem.szField62+nPos, "HTLE", 4);
	nPos+=4;
	memcpy(stSystem.szField62+nPos, "04", 2);
	nPos+=2;
	memcpy(stSystem.szField62+nPos, "5", 1);
	nPos+=1;
	memcpy(stSystem.szField62+nPos, "1", 1);//Request-Type
	nPos+=1;
	GetVarAcqId(szAcqId);
	memcpy(stSystem.szField62+nPos, szAcqId, 3);//Request-Type	
	nPos+=3;
	GetVarTerminalId(szTID, stSystem.nAcqIndex);
	memcpy(stSystem.szField62+nPos, szTID, 8);
	nPos+=8;
	GetVarVendorId(szVendorId);
	//GetVarTerminalId(szVendorId);
	memcpy(stSystem.szField62+nPos, szVendorId, 8);
	nPos+=8;
	memcpy(stSystem.szField62+nPos, szTeid, 8);
	nPos+=8;
	memcpy(stSystem.szField62+nPos, sHash3, 8);
	nPos+=8;
	//memcpy(stSystem.szField62+nPos, stPubKey.exponent, 3);
	PubAscToHex(stPubKey.exponent, 6, 0, (uchar *)(stSystem.szField62+nPos));
	nPos+=3;
	//memcpy(stSystem.szField62+nPos, stPubKey.modulus, 256);
	PubAscToHex(stPubKey.modulus, 512, 0, (uchar *)(stSystem.szField62+nPos));
	nPos+=256;
	ASSERT_HANGUP_FAIL(SetField(62, stSystem.szField62, nPos));

	//DE63
	memcpy(szDeviceSn, "DSN", 3);
	GetTerminalSn(&szDeviceSn[3]);
	//PubAddSymbolToStr(szDeviceSn, 18, ' ', 1);
	memcpy(szDeviceSn+15, "   ", 3);
	ASSERT_HANGUP_FAIL(SetField(63, szDeviceSn, 18));
	
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
	
	IncVarTraceNo();

	ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));
	ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));

	CommHangUp();

	ASSERT_FAIL(Unpack(sPackBuf, nPackLen));
	ASSERT_FAIL(ChkRespMsgID("0800", sPackBuf));
	ASSERT_FAIL(ChkRespon(&stSystem, sPackBuf + 2));

	PubClearAll();
	
	if (memcmp(stSystem.szResponse, "00", 2) != 0)
	{
		if (memcmp(stSystem.szResponse, "LE", 2) == 0)
		{
			DispRespEFTInfo();
		}
		else
		{
			DispResp(stSystem.szResponse, stSystem.nAcqIndex);
		}
		return APP_QUIT;
	}
	PubDisplayTitle(pszTitle);

	ASSERT_QUIT(ProcessKmsData(&stPrivate));
	PubMsgDlg(pszTitle, "DOWNLOAD OK", 1, 1);
	PubSetPosDateTime(stSystem.szDate, "MMDD", stSystem.szTime);
	return APP_SUCC;
}


