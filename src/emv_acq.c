/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  emv_acq.c
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

static char gcIsArpcErr = NO;	

typedef struct
{
	int	uiEmvTag;
	char ucLen;		// for process AMEX bit 55, no used for ADVT/TIP
}DE55AmexTag;

// AMEX format bit 55
DE55AmexTag sgAmexTagList[] =
{
	{0x9F26, 8},
	{0x9F10, 32},//v
	{0x9F37, 4},
	{0x9F36, 2},
	{0x95, 5},
	{0x9A, 3},
	{0x9C, 1},
	{0x9F02, 6},
	{0x5F2A, 2},
	{0x9F1A, 2},
	{0x82, 2},
	{0x9F03, 6},
	{0x5F34, 1},
	{0x9F27, 1},
	{0x9F06,  16},//v
	{0x9F09, 2},
	{0x9F34, 3},
	{0x9F0E, 5},
	{0x9F0F, 5},
	{0x9F0D, 5},
	{0},
};

int AddAmexDE55(char *psF55, int *pnLen)
{
	int i = 0;
	int nPos = 0;
	int nDataLen = 0;
	uchar sBuf[512] = {0};
	
	memcpy(psF55, "\xC1\xC7\xD5\xE2\x00\x01", 6);	
	nPos += 6;

	for(i=0; sgAmexTagList[i].uiEmvTag!=0; i++)
	{
		memset(sBuf, 0, sizeof(sBuf));
		nDataLen = EmvEntryPointGetData(sgAmexTagList[i].uiEmvTag, sBuf, sizeof(sBuf));
//PubDebug("===tag=%x[len=%d]", sgAmexTagList[i].uiEmvTag, nDataLen);
//PubDebugData("data: ", sBuf, nDataLen);
		if (0x9F06 == sgAmexTagList[i].uiEmvTag
			||0x9F10 == sgAmexTagList[i].uiEmvTag)
		{
			*(psF55+nPos) = nDataLen;
			nPos ++;
			memcpy(psF55+nPos, sBuf, nDataLen);
			nPos += nDataLen;		
		}
		else
		{
			memcpy(psF55+nPos, sBuf, nDataLen);
			nPos += nDataLen;		
		}
	}
PubDebug("over....");
	*pnLen = nPos;
	return APP_SUCC;
	
}


/*
*	@brief     get emv tag from Appdata, code in tlv string to 55 fiedld
*	@param          const char	iTransType	 , transaction type
*	@param          char 		*oField55		out put field55, (first 2 byte is length[BCD])
*	@return			APP_SUCC/APP_FAIL
*/
int EmvPackField55(const char cTransType, char *pszOutField55, int *pnOutField55Len, int nAcqIndex)
{
	int nLen = 0;
	uint nTagList[MAX_TAG_LIST_NUM];

	//EmvEntryPointSetData(0x9F66, (uchar *)"\x36\xC0\x00\x00", 4);
	
	switch (cTransType)
	{
	case EMV_BALANCE:
	case EMV_SALE:
	case EMV_PREAUTH:
	case EMV_REVERSAL:
		if (nAcqIndex == 1)
		{
			AddAmexDE55(pszOutField55, pnOutField55Len);
			return APP_SUCC;
		}
		else if (nAcqIndex == 2)
		{
			nTagList[nLen++] = _EMV_TAG_5F2A_TM_CURCODE;
			nTagList[nLen++] = _EMV_TAG_82_IC_AIP;
			nTagList[nLen++] = _EMV_TAG_95_TM_TVR;
			nTagList[nLen++] = _EMV_TAG_9A_TM_TRANSDATE;
			nTagList[nLen++] = _EMV_TAG_9C_TM_TRANSTYPE;

			nTagList[nLen++] = _EMV_TAG_9F02_TM_AUTHAMNTN;
			nTagList[nLen++] = _EMV_TAG_9F03_TM_OTHERAMNTN;
			nTagList[nLen++] = _EMV_TAG_9F10_IC_ISSAPPDATA;
			
			nTagList[nLen++] = _EMV_TAG_9F1A_TM_CNTRYCODE;
			nTagList[nLen++] = _EMV_TAG_9F26_IC_AC;
			nTagList[nLen++] = _EMV_TAG_9F27_IC_CID;
			nTagList[nLen++] = _EMV_TAG_9F36_IC_ATC;
			nTagList[nLen++] = _EMV_TAG_9F37_TM_UNPNUM;
			nTagList[nLen++] = _EMV_TAG_84_IC_DFNAME;
			nTagList[nLen++] = _EMV_TAG_9F34_TM_CVMRESULT;

			//opt		
			nTagList[nLen++] = _EMV_TAG_9F09_TM_APPVERNO;
			nTagList[nLen++] = _EMV_TAG_9F1B_TM_FLOORLMT;
			nTagList[nLen++] = _EMV_TAG_9F41_TM_TRSEQCNTR;
		}
		else
		{
			nTagList[nLen++] = _EMV_TAG_5F2A_TM_CURCODE;
			//nTagList[nLen++] = _EMV_TAG_5F34_IC_PANSN;//
			nTagList[nLen++] = _EMV_TAG_82_IC_AIP;
			nTagList[nLen++] = _EMV_TAG_84_IC_DFNAME;
			
			nTagList[nLen++] = _EMV_TAG_95_TM_TVR;
			nTagList[nLen++] = _EMV_TAG_9A_TM_TRANSDATE;
			nTagList[nLen++] = _EMV_TAG_9C_TM_TRANSTYPE;

			nTagList[nLen++] = _EMV_TAG_9F02_TM_AUTHAMNTN;
			nTagList[nLen++] = _EMV_TAG_9F03_TM_OTHERAMNTN;
			nTagList[nLen++] = _EMV_TAG_9F09_TM_APPVERNO;
			nTagList[nLen++] = _EMV_TAG_9F10_IC_ISSAPPDATA;
			nTagList[nLen++] = _EMV_TAG_9F1A_TM_CNTRYCODE;
			nTagList[nLen++] = _EMV_TAG_9F1E_TM_IFDSN;
			
			nTagList[nLen++] = _EMV_TAG_9F26_IC_AC;
			nTagList[nLen++] = _EMV_TAG_9F27_IC_CID;
			nTagList[nLen++] = _EMV_TAG_9F33_TM_CAP;
			nTagList[nLen++] = _EMV_TAG_9F34_TM_CVMRESULT;
			nTagList[nLen++] = _EMV_TAG_9F35_TM_TERMTYPE;
			nTagList[nLen++] = _EMV_TAG_9F36_IC_ATC;
			nTagList[nLen++] = _EMV_TAG_9F37_TM_UNPNUM;
			
			nTagList[nLen++] = 0x9F6E;//master no need
			
			//nTagList[nLen++] = _EMV_TAG_9F41_TM_TRSEQCNTR;//
			//nTagList[nLen++] = _EMV_TAG_9F53_MCC;			//
			

#if 0
			nTagList[nLen++] = _EMV_TAG_9F45_IC_DTAUTHCODE;
			nTagList[nLen++] = _EMV_TAG_9F4C_IC_ICCDYNNUM;
			nTagList[nLen++] = _EMV_TAG_5A_IC_PAN;
			nTagList[nLen++] = _EMV_TAG_5F24_IC_APPEXPIREDATE;
			nTagList[nLen++] = _EMV_TAG_50_IC_APPLABEL;
#endif			
	/*
			nTagList[nLen++] = _EMV_TAG_72_ISSUE_SCRIPT;
			nTagList[nLen++] = _EMV_TAG_91_TM_ISSAUTHDT;
			nTagList[nLen++] = _EMV_TAG_5F30_IC_SERVICECODE;
			nTagList[nLen++] = _EMV_TAG_9F06_TM_AID;
			nTagList[nLen++] = _EMV_TAG_9F28_ARQC;
			nTagList[nLen++] = _EMV_TAG_9F29_TC;*/
		}
		break;
	default:
		break;
	}

	nLen = EmvEntryPointFetchData(nTagList, nLen, (uchar *)pszOutField55, 256);
	if (nLen <= 0)
	{
		return APP_FAIL;
	}
	*pnOutField55Len = nLen;
	return APP_SUCC;
}

/**
*	@brief  clear emv transaction log
*	@param	void
*	@return	 EMV_SUCC/EMV_FAIL
*/
int EmvClearWater(void)
{
	int i;
	char szLogFile[100];
	STRECFILE stTransRecordFile;
	STEMVWATER stTransRecordRec;
	
	for (i = 0; i < 2; i++)
	{
		memset(&stTransRecordFile, 0, sizeof(STRECFILE));
		
		stTransRecordFile.cIsIndex = FILE_CREATEINDEX;				
		stTransRecordFile.unMaxOneRecLen = sizeof(STEMVWATER);
		stTransRecordFile.unIndex1Start =(uint)((char *)(stTransRecordRec.szInvoice) - (char *)&stTransRecordRec)+1;
		stTransRecordFile.unIndex1Len =  sizeof(stTransRecordRec.sWaterNo);
		stTransRecordFile.unIndex2Start =  1;
		stTransRecordFile.unIndex2Len = 1;
		if (i == 0)
		{
			strcpy( stTransRecordFile.szFileName, FILE_EMV_WATER_FAIL );
			ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));
		}
		else
		{
			strcpy( stTransRecordFile.szFileName, FILE_EMV_WATER_SUCC);
			ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));
		}	
	}
	sprintf(szLogFile, "%s", EMVFILE_TERMLOG);
	PubDelFile(szLogFile);/**<delete emv kernel log*/
	return APP_SUCC;
}

/**
*	@brief          get emv translog

*	@param          char		*iBatchNo		
*	@param          char		*iWaterNo		
*	@param			TEMV_WATER	*oWater			
*	@param			int falg    TRUE-success log FALSE-fail log
*	@return			EMV_SUCC/EMV_FAIL
*/
int EmvLoadWaterByTrace(const char * pszInvoice, STEMVWATER * pstOutWater, int nFlag)
{
	int nRet;
	int nRecNO;


	if(nFlag)  
	{
		nRet = PubFindRec(FILE_EMV_WATER_SUCC, pszInvoice, NULL, 1, (char *)pstOutWater, &nRecNO);
	}
	else
	{
		nRet = PubFindRec(FILE_EMV_WATER_FAIL, pszInvoice, NULL, 1, (char *)pstOutWater, &nRecNO);
	}
	
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
*	@brief          save emv translog
*	@param			in waterflag	TRUE  success	FALSE fail
*	@param			in pstSystem	
*	@return			
*/
int EmvSaveWater(int nWaterFlag, const STSYSTEM *pstSystem)
{
	int nRet;
	int nLen;
	STEMVWATER stEmvWater;
	char szBuff[512];

	gcIsArpcErr = NO;
	
	memset(&stEmvWater, 0, sizeof(STEMVWATER));
	PubAscToHex((uchar *)pstSystem->szPan, strlen(pstSystem->szPan), 0, (uchar *)stEmvWater.szPan);
	stEmvWater.nPanLen = strlen(pstSystem->szPan);
	stEmvWater.nPanSeq = atoi(pstSystem->szCardSerialNo);
	PubAscToHex((uchar *)pstSystem->szDate, 4, 0, (uchar *)stEmvWater.sDate);
	PubAscToHex((uchar *)pstSystem->szTime, 6, 0, (uchar *)stEmvWater.sTime);
	PubAscToHex((uchar *)pstSystem->szAmount, 12, 0, (uchar *)stEmvWater.sAmount);
	PubAscToHex((uchar *)pstSystem->szTrace, 6, 0, (uchar *)stEmvWater.sWaterNo);
	PubAscToHex((uchar *)pstSystem->szBatchNum, 6, 0, (uchar *)stEmvWater.sBatchNo);
	stEmvWater.cTransType = pstSystem->cTransType;
	memcpy(stEmvWater.szInvoice, pstSystem->szInvoice, 6);
	
	nLen = EmvEntryPointGetData(_EMV_TAG_8A_TM_ARC, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvWater.sAuthResp, szBuff, 2);

	nLen = EmvEntryPointGetData(_EMV_TAG_95_TM_TVR, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvWater.sTVR, szBuff, 5);

	nLen = EmvEntryPointGetData(_EMV_TAG_9B_TM_TSI, (uchar *)szBuff, sizeof(szBuff));
	memcpy(stEmvWater.sTSI, szBuff, 2);

	if ((ATTR_EMV_STANDARD == pstSystem->cTransAttr && EMV_STATUS_ONLINE_SUCC == pstSystem->cEMV_Status && (stEmvWater.sTVR[4]&0x40))
		||(ATTR_PBOC_EC == pstSystem->cTransAttr && EMV_STATUS_ONLINE_SUCC == pstSystem->cEMV_Status && (stEmvWater.sTVR[4]&0x40)))
	{
		gcIsArpcErr = YES;
	}
	EmvPackField55(EMV_SALE, stEmvWater.szField55+2, &nLen, pstSystem->nAcqIndex);
	if (gcIsArpcErr == YES)
	{
		char szBuff[128];
		int nBufLen;
		nBufLen = EmvEntryPointGetData(_EMV_TAG_91_TM_ISSAUTHDT, (uchar *)szBuff, sizeof(szBuff));
		TRACE_HEX((szBuff, nBufLen, "EMV_getdata 0x91->"));
		if (nBufLen > 0)
		{
			TlvAdd(_EMV_TAG_9F26_IC_AC, 8, (char *)szBuff,
				 stEmvWater.szField55+2, &nLen);
		}
	}
	gcIsArpcErr = NO;

	if ((pstSystem->cTransType == TRANS_SALE
		&& (pstSystem->cTransAttr == ATTR_PBOC_EC || pstSystem->cTransAttr == ATTR_qPBOC)
		&& pstSystem->cEMV_Status == EMV_STATUS_OFFLINE_SUCC))
	{
		char szBuff[128];
		int nBufLen ;
		nBufLen = EmvEntryPointGetData(0x9F74, (uchar *)szBuff,  sizeof(szBuff));
		if (nBufLen > 0)
		{
			TlvAdd(0x9F74, nBufLen, (char *)szBuff,
				 stEmvWater.szField55+2, &nLen);
		}
	}
	PubIntToC2((uchar *)szBuff, (uint)nLen);
	memcpy(stEmvWater.szField55, szBuff, 2);

	if(memcmp(stEmvWater.sAuthResp, "\x59\x31", 2)==0 || memcmp(stEmvWater.sAuthResp, "\x59\x33", 2)==0 || 
		memcmp(stEmvWater.sAuthResp, "\x5a\x31", 2)==0 || memcmp(stEmvWater.sAuthResp, "\x5a\x33", 2)==0 )
	{
//		len = pstSystem->nAddFieldLen;
		stEmvWater.szField55[nLen+2] = 0x8a;
		stEmvWater.szField55[nLen+3] = 0x02;
		stEmvWater.szField55[nLen+4] = stEmvWater.sAuthResp[0];
		stEmvWater.szField55[nLen+5] = stEmvWater.sAuthResp[1];
		stEmvWater.szField55[0] = (nLen+4)/256;
		stEmvWater.szField55[1] = (nLen+4)%256;
	}
	
	//deal script
	nLen = EmvEntryPointGetData(_EMV_TAG_DF31_IC_IISSCRIRES, (uchar *)szBuff, sizeof(szBuff));
	stEmvWater.nIssuerScriptResultLen = nLen ;
	memcpy(stEmvWater.szIssuerScriptResult, szBuff, nLen) ;

	if(memcmp(pstSystem->szInputMode, "05", 2) == 0)
	{
		stEmvWater.cInputMode = '5';
	}
	else if(memcmp(pstSystem->szInputMode, "07", 2) == 0)
	{
		stEmvWater.cInputMode = '7';
	}
	stEmvWater.cBatchUpFlag = 0;

	if (nWaterFlag)  
	{
		nRet = PubAddRec(FILE_EMV_WATER_SUCC, (char *)&stEmvWater);
	}
	else
	{
		nRet = PubAddRec(FILE_EMV_WATER_FAIL, (char *)&stEmvWater);
	}

	
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief set terminal cap according to aid
* @param in pRid RID
* @param out pcType 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int EmvPeekTypeByAid(const uchar *psRid, uchar *pcType, uchar *psCap, uchar *usKernel)
{
	if (NULL == pcType)
	{
		return APP_FAIL;
	}
PubDebugData("rid: ", psRid, 5);
	if (memcmp(psRid,"\xA0\x00\x00\x00\x03",5) == 0)
	{
		*pcType = VISA_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	PubDebugData("visa cap: ", psCap, 3);	
		return APP_SUCC;
	}
	else if (memcmp(psRid,"\xA0\x00\x00\x00\x04",5) == 0)
	{
		*pcType = MASTERCARD_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_unset(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x02\x00\x00\x00\x00\x00\x00\x00", 8);
	PubDebugData("master cap: ", psCap, 3);	
		return APP_SUCC;
	}
	else if (memcmp(psRid,"\xA0\x00\x00\x00\x65",5) == 0)
	{
		*pcType = JCB_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x05\x00\x00\x00\x00\x00\x00\x00", 8);
		return APP_SUCC;
	}
	else if (memcmp(psRid,"\xA0\x00\x00\x06\x15",5) == 0)
	{
		*pcType = MCCS_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x20\x00\x00\x00\x00\x00\x00\x00", 8);
		return APP_SUCC;
	}
	else if (memcmp(psRid,"\xD9\x99\x99\x99\x99",5) == 0)
	{
		*pcType = MCCS_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x20\x00\x00\x00\x00\x00\x00\x00", 8);
		return APP_SUCC;
	}
	else if (memcmp(psRid,"\xA0\x00\x00\x00\x25",5) == 0)
	{
		*pcType = AMEX_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_unset(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_unset(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x04\x00\x00\x00\x00\x00\x00\x00", 8);
		return APP_SUCC;
	}
	//discover6
	else if (memcmp(psRid,"\xA0\x00\x00\x03\x33",5) == 0)
	{
		*pcType = PBOC2_ENB;
		terminal_cap_set(TC_Plaintext_PIN, psCap);
		terminal_cap_set(TC_Enciphered_PIN_Online, psCap);
		terminal_cap_set(TC_Signature_Paper, psCap);
		terminal_cap_unset(TC_Enciphered_PIN_Offline, psCap);
		terminal_cap_set(TC_No_CVM_Required, psCap);
		terminal_cap_set(TC_Cardholder_Cert, psCap);
		memcpy(usKernel, "\x07\x00\x00\x00\x00\x00\x00\x00", 8);
		return APP_SUCC;
	}
	return APP_FAIL;
}

/**
*	@brief   Get pan from emv tag
*	@param  [in]   char *pszPan
*	@param  [out]   char *pszPan
*	@return			
*	@li APP_SUCC
*	@li APP_FAIL
**/
int Deal5APan(char *pszPan)
{
	char szTemp[10+1];
	char szPan[20+1]= {0};
	int nRet,nPanLen,i;
	
	memset(szTemp,0,sizeof(szTemp));
	nRet = EmvEntryPointGetData(_EMV_TAG_5A_IC_PAN, (uchar *)szTemp, sizeof(szTemp));
	if (nRet <= 0)
	{
		return APP_FAIL;
	}
	nPanLen = nRet<<1;
	PubHexToAsc((uchar *)szTemp, nPanLen, 0, (uchar *)(szPan));	
	for(i = 0; i < nPanLen && i < sizeof(szPan); i++)
	{
		if (szPan[i] == 'f' ||szPan[i] == 'F')
		{
			szPan[i] = 0;
		}
	}
	strcpy(pszPan,szPan);
	
	return APP_SUCC;
}

