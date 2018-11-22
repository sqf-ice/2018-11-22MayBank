/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  water.c
** File indentifier: 
** Synopsis:  Processing translog
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-5
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static int gnCurrentRecNo=0;	

/**
* @brief Create or clean translog file
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitAllBatch(void)
{
	STRECFILE stTransRecordFile;
	STTAANSRECORD stTransRecordRec;

	strcpy( stTransRecordFile.szFileName, FILE_WATER);
	stTransRecordFile.cIsIndex = FILE_CREATEINDEX;				
	stTransRecordFile.unMaxOneRecLen = sizeof(STTAANSRECORD);
	stTransRecordFile.unIndex1Start =(uint)((char *)(stTransRecordRec.szInvoice) - (char *)&stTransRecordRec)+1;
	stTransRecordFile.unIndex1Len =  sizeof(stTransRecordRec.szInvoice);
	stTransRecordFile.unIndex2Start =  (uint)((char *)(stTransRecordRec.szOldInvoice) - (char *)&stTransRecordRec)+1;
	stTransRecordFile.unIndex2Len = sizeof(stTransRecordRec.szOldInvoice);
	gnCurrentRecNo = 0;
	ASSERT_FAIL(PubCreatRecFile(&stTransRecordFile));
	return APP_SUCC;
}

/**
* @brief Append translog
* @param in const STTAANSRECORD *pstTransRecord 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int AppendTransRecord(const STTAANSRECORD *pstTransRecord)
{	
	PubGetRecSum(FILE_WATER, &gnCurrentRecNo );
	ASSERT_FAIL(PubAddRec(FILE_WATER, (const char *)pstTransRecord ));
	gnCurrentRecNo++;
	return APP_SUCC;
}

/**
* @brief Update translog
* @param in const STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int UpdateTransRecord(const STTAANSRECORD *pstTransRecord)
{
	return PubUpdateRec (FILE_WATER, gnCurrentRecNo,  (const char *)pstTransRecord);
}

/**
* @brief Find translog with trace number
* @param in const char *szInvno Trace no
* @param in pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FindRecordWithInvno( const char *pszInvno, STTAANSRECORD *pstTransRecord)
{
	return PubFindRec (FILE_WATER, pszInvno, NULL,1, (char *)pstTransRecord, &gnCurrentRecNo);
}

 /**
* @brief Find translog with trace number and display in the screen
* @param in int PrintFlag  if equal to 1, print the translog
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FindByInvoice(int PrintFlag)
{
	int nRet, nLen;
	STTAANSRECORD stTransRecord;
	char szInvno[6+1];
	char szDispTitle[32] = {0};

	if (PrintFlag)
	{
		strcpy(szDispTitle, "PRINT ANY");
	}
	else
	{
		strcpy(szDispTitle, "VIEW APPOINTED");
	}

	nLen = 6;
	memset(szInvno, 0, sizeof(szInvno));
	ASSERT_QUIT(PubInputDlg(szDispTitle, "TRXN NO:", szInvno, &nLen, 1, 6, 0, INPUT_MODE_NUMBER));	
	PubAddSymbolToStr(szInvno, 6, '0', 0);
	nRet = FindRecordWithInvno( szInvno, &stTransRecord);
	if (nRet==APP_FAIL)
	{
		char szDispBuf[100];
		
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CINVALID TRANS NO.");
		PubMsgDlg(szDispTitle, szDispBuf, 0, 5);
		return APP_FAIL;		
	}
	else
	{
		if (stTransRecord.cStatus == 0x01)
		{
			if (APP_SUCC != FindRecordWithOldInvno(szInvno, &stTransRecord))
			{
				char szDispBuf[100];
				
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS.");
				PubMsgDlg(szDispTitle, szDispBuf, 0, 5);
				return APP_FAIL;		
			}
		}
		PrintWater(&stTransRecord, REPRINT);
#if 0 // require to print directly
		nRet = DispTransRecord(&stTransRecord);
		if(nRet != KEY_ENTER)
		{
			return APP_QUIT;
		}
		
		if (PrintFlag)
		{	
			nRet = PubConfirmDlg(szDispTitle, "PRINT RECIEPT?", 0, 50);
			if (nRet == APP_SUCC)
			{
				PubClearAll();
				DISP_PRINTING_NOW;
			}
		}
#endif
	}
	return APP_SUCC;
}

/**
* @brief Get current translog
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchCurrentRecord(STTAANSRECORD *pstTransRecord)
{
	return PubReadOneRec(FILE_WATER,gnCurrentRecNo, (char *)pstTransRecord);	
}

/**
* @brief Get first translog
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchFirstRecord(STTAANSRECORD *pstTransRecord)
{
	gnCurrentRecNo = 1;
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get next translog
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchNextRecord(STTAANSRECORD *pstTransRecord)
{
	int nRecNum;

	PubGetRecSum(FILE_WATER, &nRecNum );
	gnCurrentRecNo++;
	if( gnCurrentRecNo > nRecNum )
	{
		gnCurrentRecNo = nRecNum;
	}
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get previous translog
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchPreviousRecord(STTAANSRECORD *pstTransRecord)
{
	gnCurrentRecNo--;
	if( gnCurrentRecNo ==  0 )
	{
		gnCurrentRecNo = 1;
	}
	return FetchCurrentRecord(pstTransRecord);
}

/**
* @brief Get last translog
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FetchLastRecord(STTAANSRECORD *pstTransRecord)
{
	PubGetRecSum(FILE_WATER, &gnCurrentRecNo );
	return FetchCurrentRecord(pstTransRecord);
}


int DispTransRecord(const STTAANSRECORD *pstTransRecord)
{
	char szDispBuf[256], szContent[512], szTemp[256];
	int nKey, nRet = 0;
	int i;

	/**
	* Show Trans type
	*/
	switch(pstTransRecord->cTransType)
	{
	case TRANS_SALE:			
		sprintf(szContent, "SALE:");
		break;
	case TRANS_AUTHCOMP:			
		sprintf(szContent, "AUTH COMP:");
		break;
	case TRANS_PREAUTH:		
		sprintf(szContent, "AUTH:");
		break;
	case TRANS_REFUND:				
		sprintf(szContent, "REFUND:");
		break;
	case TRANS_VOID_REFUND:				
		sprintf(szContent, "VOID REFUND:");
		break;
	case TRANS_VOID_OFFLINE:				
		sprintf(szContent, "VOID OFFLINE:");
		break;
	case TRANS_VOID:		
		sprintf(szContent, "VOID:");
		break;
	case TRANS_VOID_AUTHSALE:		
		sprintf(szContent, "VOID AUTH COMP:");
		break;
	case TRANS_VOID_PREAUTH:	
		sprintf(szContent, "VOID AUTH:");
		break;
	case TRANS_OFFLINE:			
		sprintf(szContent, "OFFLINE:");
		break;
	default:
		sprintf(szContent, "UNKNOW:%d",pstTransRecord->cTransType);
		break;
	}

	switch(pstTransRecord->cStatus)
	{
	case 3:		
		break;
	case 1:	
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(VOID)", szContent);
		break;
	case 2:
		break;
	default:	
		switch(pstTransRecord->cTransType)
		{
		case TRANS_VOID:
		case TRANS_VOID_REFUND:
		case TRANS_VOID_OFFLINE:
		case TRANS_VOID_AUTHSALE:
			if(pstTransRecord->cSendFlag < 0xFD)
			{	
				PubHexToAsc((uchar *)pstTransRecord->sOldTrace, 6, 0, (uchar *)szDispBuf);
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R%6.6s", szContent,szDispBuf);
	    	}
			break;
	    	default:	
			break;
		}
		break;
	}

	if(pstTransRecord->cSendFlag >= 0xFD && (pstTransRecord->cStatus == 2 || pstTransRecord->cStatus == 4))
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(ADJUSTED SENDED)", szContent);
	}
	else if(pstTransRecord->cSendFlag >= 0xFD)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(SENDED)", szContent);
	}
	else if(pstTransRecord->cStatus == 2 || pstTransRecord->cStatus == 4)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s|R(ADJUSTED)", szContent);
	}

	/*------Card no--------*/
	memset(szDispBuf, 0, sizeof(szDispBuf));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szDispBuf);
	CtrlCardPan((uchar*)szDispBuf, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	sprintf(szContent+strlen(szContent), "\n%s", szDispBuf);	
	/*------date time--------*/
	sprintf(szContent+strlen(szContent), "\n%02x/%02x %02x:%02x", pstTransRecord->sDate[0], pstTransRecord->sDate[1], pstTransRecord->sTime[0], pstTransRecord->sTime[1]);
	/*------trace no-------------*/
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nTRACE:%02x%02x%02x", pstTransRecord->sTrace[0], pstTransRecord->sTrace[1], pstTransRecord->sTrace[2]);	
	/*------auth code-----------------*/	
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAUTH:%6.6s", pstTransRecord->szAuthCode);

	/*Amt*/
	memset(szDispBuf,0,sizeof(szDispBuf));
	memset(szTemp,0,sizeof(szTemp));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)szDispBuf);
	sprintf(szTemp,"%10.10s.%2.2s",szDispBuf,szDispBuf+10);
	for (i=0; i<9; i++)
	{
		if (szTemp[i] == '0')
		{
			szTemp[i] = ' ';
		}
		else
		{
			break;
		}
	}
	PubLeftTrim(szTemp);
	sprintf(szContent+strlen(szContent), "\nAMT:%s",szTemp);

	/*Ref number*/
	sprintf(szContent+strlen(szContent), "\nREF:%12.12s",pstTransRecord->szRefnum);
	
	nRet = PubUpDownMsgDlg(NULL, szContent, NO, 60, &nKey);	
	
	if(nRet == APP_QUIT)
		return KEY_ESC;
	if(nRet == APP_FUNCQUIT && nKey == KEY_ENTER)
	{
		nRet = PubUpDownMsgDlg(NULL, szContent, YES, 60, &nKey);
		nKey = KEY_ENTER;
	}
	return nKey;
}

/**
* @brief Display translog when VOID
* @param in STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int DispVoidRecord(const char * pszTitle, const STTAANSRECORD *pstTransRecord)
{
	char szDispBuf[30], szAmount[13];
	char szContent[500];
	char szDispAmt[13+1] = {0};
	int nKey =0;
	
	//Card
	memset(szDispBuf, 0, sizeof(szDispBuf));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szDispBuf);
	CtrlCardPan((uchar*)szDispBuf, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%s", szDispBuf);
	//Trace no
	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)szDispBuf);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nVOUCH NO:|R%s", szDispBuf);

	//Amt
	memset(szAmount, 0, sizeof(szAmount));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)szAmount);	
	ProAmtToDispOrPnt(szAmount, szDispAmt);
	PubAllTrim(szDispAmt);
	if(1 == pstTransRecord->cFlagCashback)		//0!=strlen(pstTransRecord->sCashbackAmount)	BCD码金额不能用strlen
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nSALE AMT:|R%s", szDispAmt);

		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0, (uchar *)szAmount);	
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nCASHBACK AMT:\n|R%s", szDispAmt);
	}
	else if (1 == pstTransRecord->cTipFlag)
	{
		char szTip[13] = {0}, szSaleAmt[13] = {0};

		memset(szAmount, 0, sizeof(szAmount));
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0, (uchar *)szTip);	

		PubAscSubAsc((uchar *)szAmount, (uchar *)szTip, (uchar *)szSaleAmt);
		ProAmtToDispOrPnt(szAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nSALE AMT:|R%s", szDispAmt);
		
		ProAmtToDispOrPnt(szTip, szDispAmt);
		PubAllTrim(szDispAmt);
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nTIP AMT:\n|R%s", szDispAmt);
	}	
	else
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\nAMOUNT:|R%s", szDispAmt);
	}

	//Ref number
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent),"\nREF:|R%s", pstTransRecord->szRefnum);
	//PubUpDownMsgDlg(pszTitle, szContent, YES, 30, &nKey);
	nKey = PubUpDownConfirmDlg(pszTitle, szContent, 0, 30);
	if(APP_SUCC == nKey)
	{
		return APP_SUCC;
	}
	else
	{
	  	return APP_FAIL;
	}
 
}



/**
* @brief Look up translog in the screen
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int WaterOneByOne(void)
{
	int nRet, nWaterRecNum;
	STTAANSRECORD stTransRecord;

	GetRecordNum(&nWaterRecNum);
	if (nWaterRecNum==0) 
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO TRANS");
		PubMsgDlg("LOOK UP", szDispBuf, 0, 5);
		return APP_SUCC;
	}
	
	ASSERT(FetchLastRecord(&stTransRecord));
	while(1)
	{
		nRet = DispTransRecord(&stTransRecord);
		switch(nRet)
		{
		case KEY_LEFT:
		case KEY_UP_720:
		case KEY_LEFT_720:
		case KEY_ENTER:
			ASSERT(FetchPreviousRecord(&stTransRecord));
			break;
		case KEY_RIGHT:
		case KEY_DOWN_720:
		case KEY_RIGHT_720:
			ASSERT(FetchNextRecord(&stTransRecord));
			break;
		case 0:
		case KEY_ESC:
			return APP_QUIT;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Water structure to system structure
* @param in STTAANSRECORD *pstTransRecord
* @param out  STSYSTEM *pstSystem 
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int RecordToSys(const STTAANSRECORD *pstTransRecord, STSYSTEM *pstSystem)
{
	pstSystem->cTransType = pstTransRecord->cTransType;						
	pstSystem->cTransAttr = pstTransRecord->cTransAttr;							
	pstSystem->cEMV_Status = pstTransRecord->cEMV_Status;							
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)pstSystem->szPan);		
	memcpy(pstSystem->szProcCode, pstTransRecord->szProcCode, 6);				
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0, (uchar *)pstSystem->szAmount);				
	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)pstSystem->szTrace);				
	PubHexToAsc((uchar *)pstTransRecord->sTime, 6, 0, (uchar *)pstSystem->szTime);					
	PubHexToAsc((uchar *)pstTransRecord->sDate, 4, 0, (uchar *)pstSystem->szDate);				
	if (memcmp(pstTransRecord->sExpDate, "\x00\x00", 2) == 0)
	{
		;
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sExpDate, 4, 0, (uchar *)pstSystem->szExpDate);				
	}
	PubHexToAsc((uchar *)pstTransRecord->sSettleDate, 4, 0, (uchar *)pstSystem->szSettleDate);		
	strcpy(pstSystem->szInputMode, pstTransRecord->szInputMode);					
	strcpy(pstSystem->szCardSerialNo, pstTransRecord->szCardSerialNo);			
	strcpy(pstSystem->szRefnum, pstTransRecord->szRefnum);						
	PubHexToAsc((uchar *)pstTransRecord->sTrack2, pstTransRecord->nTrack2Len, 0, (uchar *)pstSystem->szTrack2);
	PubHexToAsc((uchar *)pstTransRecord->sTrack3, pstTransRecord->nTrack3Len, 0, (uchar *)pstSystem->szTrack3);
	strcpy(pstSystem->szAuthCode, pstTransRecord->szAuthCode);					
	strcpy(pstSystem->szResponse, pstTransRecord->szResponse);						
	PubHexToAsc((uchar *)pstTransRecord->szBatchNum, 6, 0, (uchar *)pstSystem->szBatchNum);	
	memcpy(pstSystem->szOldAuthCode, pstTransRecord->szOldAuthCode, 6);				
	memcpy(pstSystem->szOldRefnum, pstTransRecord->szOldRefnum, 12);					
	memcpy(pstSystem->szOldInvoice, pstTransRecord->szOldInvoice, 6);					
	strcpy(pstSystem->szInterOrg, pstTransRecord->szInterOrg);						
	PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0, (uchar *)pstSystem->szTipAmount); 	
	memcpy(pstSystem->szServerCode, pstTransRecord->szServerCode, 2);

	pstSystem->nAddFieldLen = pstTransRecord->nField55Len;
	memcpy(pstSystem->sField55, pstTransRecord->sField55, pstTransRecord->nField55Len);
	pstSystem->cFlagCashback= pstTransRecord->cFlagCashback;
	pstSystem->cTipFlag= pstTransRecord->cTipFlag;
	memcpy(pstSystem->szMsgID, pstTransRecord->szMsgId, 4);

	strcpy(pstSystem->szPosID, pstTransRecord->szTID);					
	strcpy(pstSystem->szShopID, pstTransRecord->szMID);						
	strcpy(pstSystem->szNii, pstTransRecord->szNii);						
	pstSystem->nAcqIndex = pstTransRecord->nAcqIndex;
	pstSystem->nAcqType = pstTransRecord->nAcqType;
	if (1 == pstTransRecord->cFlagCashback)
	{
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0, (uchar *)pstSystem->szCashbackAmount); 		/**<提现金额	20161009_hzw_add*/
	}
	memcpy(pstSystem->szInvoice, pstTransRecord->szInvoice, 6);
	pstSystem->cHasPinInput = pstTransRecord->cHasPinInput;
	pstSystem->nInstallmentMonth = pstTransRecord->nInstallmentMonth;
	if (strlen(pstTransRecord->szInstallmentInfo) > 0)
	{
		memcpy(pstSystem->szInstallmentInfo, pstTransRecord->szInstallmentInfo, strlen(pstTransRecord->szInstallmentInfo));
	}
	memcpy(&pstSystem->stOrsInfo, &pstTransRecord->stOrsInfo, sizeof(STORS_INFO));
	
	pstSystem->cIsUseDcc = pstTransRecord->cIsUseDcc;
	memcpy(pstSystem->szLocalAmount, pstTransRecord->szLocalAmount, 12);
	memcpy(pstSystem->szDccCurrencyCode, pstTransRecord->szDccCurrencyCode, 3);
	memcpy(pstSystem->szField63, pstTransRecord->szField63, pstTransRecord->nField63Len);
	pstSystem->cWavePinAndSigFlag = pstTransRecord->cWavePinAndSigFlag;
	memcpy(pstSystem->sTVR, pstTransRecord->sTVR, 5);
	return APP_SUCC;
}

/**
* @brief system structure to Water structure 
* @param in  STSYSTEM *pstSystem 
* @param out STTAANSRECORD *pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int SysToRecord(const STSYSTEM *pstSystem, STTAANSRECORD *pstTransRecord )
{
	pstTransRecord->cTransType = pstSystem->cTransType;						
	pstTransRecord->cTransAttr = pstSystem->cTransAttr;						
	pstTransRecord->cEMV_Status = pstSystem->cEMV_Status;
	TRACE(("pstTransRecord->cEMV_Status=%d",pstTransRecord->cEMV_Status));
	pstTransRecord->nPanLen = strlen(pstSystem->szPan);								
	memcpy(pstTransRecord->szProcCode, pstSystem->szProcCode, 6);				
	PubAscToHex((uchar *)pstSystem->szPan, pstTransRecord->nPanLen, 0, (uchar *)pstTransRecord->sPan);	
	PubAscToHex((uchar *)pstSystem->szAmount, 12, 0, (uchar *)pstTransRecord->sAmount);				
	PubAscToHex((uchar *)pstSystem->szTrace, 6, 0, (uchar *)pstTransRecord->sTrace);					
	PubAscToHex((uchar *)pstSystem->szTime, 6, 0, (uchar *)pstTransRecord->sTime);					
	PubAscToHex((uchar *)pstSystem->szDate, 4, 0, (uchar *)pstTransRecord->sDate);				
	PubAscToHex((uchar *)pstSystem->szExpDate, 4, 0, (uchar *)pstTransRecord->sExpDate);				
	PubAscToHex((uchar *)pstSystem->szSettleDate, 4, 0, (uchar *)pstTransRecord->sSettleDate);		
	strcpy(pstTransRecord->szInputMode, pstSystem->szInputMode );				
	strcpy(pstTransRecord->szCardSerialNo, pstSystem->szCardSerialNo);				
	pstTransRecord->nTrack2Len = strlen(pstSystem->szTrack2);		
	PubAscToHex((uchar *)pstSystem->szTrack2, pstTransRecord->nTrack2Len, 0, (uchar *)pstTransRecord->sTrack2);
	pstTransRecord->nTrack3Len = strlen(pstSystem->szTrack3);		
	PubAscToHex((uchar *)pstSystem->szTrack3, pstTransRecord->nTrack3Len, 0, (uchar *)pstTransRecord->sTrack3);
	strcpy(pstTransRecord->szRefnum, pstSystem->szRefnum);							
	strcpy(pstTransRecord->szAuthCode, pstSystem->szAuthCode);						
	strcpy(pstTransRecord->szResponse, pstSystem->szResponse);						
	PubAscToHex((uchar *)(pstSystem->szBatchNum), 6, 0, (uchar *)pstTransRecord->szBatchNum);	
	PubAscToHex((uchar *)(pstSystem->stField62).sOldTrace, 6, 0, (uchar *)pstTransRecord->sOldTrace);
	
	TRACE_HEX((pstTransRecord->szBatchNum,3,"pstTransRecord->szBatchNum:"));
	memcpy(pstTransRecord->szOldAuthCode, pstSystem->szOldAuthCode, 6);				
	memcpy(pstTransRecord->szOldRefnum, pstSystem->szOldRefnum, 12);					
	memcpy(pstTransRecord->szOldInvoice, pstSystem->szOldInvoice, 6);					
	pstTransRecord->cStatus = 0;										
	strcpy(pstTransRecord->szInterOrg, pstSystem->szInterOrg);					
	PubAscToHex((uchar *)pstSystem->szTipAmount, 12, 0, (uchar *)pstTransRecord->sTipAmount); 		
	pstTransRecord->cSendFlag = 0;					
	pstTransRecord->cBatchUpFlag= 0;					

	if (strlen(pstSystem->szHolderName) > 0)
	{
		memcpy(pstTransRecord->szHolderName, pstSystem->szHolderName, strlen(pstSystem->szHolderName));
	}
	memcpy(pstTransRecord->szServerCode, pstSystem->szServerCode, 2);
	pstTransRecord->nField55Len = pstSystem->nAddFieldLen;
	memcpy(pstTransRecord->sField55, pstSystem->sField55, pstSystem->nAddFieldLen);
	pstTransRecord->cFlagCashback = pstSystem->cFlagCashback;
	pstTransRecord->cTipFlag = pstSystem->cTipFlag;
	memcpy(pstTransRecord->szMsgId, pstSystem->szMsgID, 4);

	if (strlen(pstSystem->szInstallmentInfo) > 0)
	{
		memcpy(pstTransRecord->szInstallmentInfo, pstSystem->szInstallmentInfo, strlen(pstSystem->szInstallmentInfo));
	}

	pstTransRecord->nInstallmentMonth = pstSystem->nInstallmentMonth;

	strcpy(pstTransRecord->szTID, pstSystem->szPosID);					
	strcpy(pstTransRecord->szMID, pstSystem->szShopID);						
	strcpy(pstTransRecord->szNii, pstSystem->szNii);						
	pstTransRecord->nAcqIndex = pstSystem->nAcqIndex;
	pstTransRecord->nAcqType = pstSystem->nAcqType;
	if (1 == pstSystem->cFlagCashback)
	{
		PubAscToHex((uchar *)pstSystem->szCashbackAmount, 12, 0, (uchar *)pstTransRecord->sCashbackAmount); 		/**<提现金额	20161009_hzw_add*/
	}
	memcpy(pstTransRecord->szInvoice, pstSystem->szInvoice, 6);
	
	pstTransRecord->cHasPinInput = pstSystem->cHasPinInput;

	memcpy(&pstTransRecord->stOrsInfo, &pstSystem->stOrsInfo, sizeof(STORS_INFO));
	
	pstTransRecord->cIsUseDcc = pstSystem->cIsUseDcc;
	memcpy(pstTransRecord->szLocalAmount, pstSystem->szLocalAmount, 12);
	memcpy(pstTransRecord->szDccCurrencyCode, pstSystem->szDccCurrencyCode, 3);
	memcpy(pstTransRecord->szField63, pstSystem->szField63, pstSystem->nField63Len);
	pstTransRecord->cWavePinAndSigFlag = pstSystem->cWavePinAndSigFlag;
	memcpy(pstTransRecord->sTVR, pstSystem->sTVR, 5);
	memcpy(pstTransRecord->szCD_TID, pstSystem->szCD_TID, 8);
	memcpy(pstTransRecord->szCD_MID, pstSystem->szCD_MID, 15);
	memcpy(pstTransRecord->sAID, pstSystem->sAID, pstSystem->nAidLen);
	pstTransRecord->nAidLen = pstSystem->nAidLen;
	memcpy(pstTransRecord->sTC, pstSystem->sTC, 8);
	memcpy(pstTransRecord->szAppLabel, pstSystem->szAppLabel, strlen(pstSystem->szAppLabel));

	return APP_SUCC;
}


/**
* @brief Get translog number
* @param out int *pnRecordNum
* @return void
*/
void GetRecordNum( int *pnRecordNum)
{
	PubGetRecSum(FILE_WATER, pnRecordNum);
}

int GetUnSendNum(int *pnUnSendNum)
{
	STTAANSRECORD stTransRecord;
	int nRet = 0;
	int nWaterSum;
	int nLoop;
	int nFileHandle;
	char nMaxReSend;
	int nUnSendSum = 0;


	GetRecordNum(&nWaterSum);
	if (nWaterSum > 0)
	{
		nRet = PubOpenFile(FILE_WATER, "r", &nFileHandle);
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
	GetVarCommReSendNum(&nMaxReSend);
	for (nLoop = 1; nLoop <= nWaterSum; nLoop++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		if((nRet = PubReadRec(nFileHandle, nLoop, (char *)&stTransRecord))!= APP_SUCC)
		{
			PubCloseFile(&nFileHandle);
			return nRet;
		}
		switch (stTransRecord.cTransType)
		{
		case TRANS_OFFLINE:
			if (stTransRecord.cSendFlag > nMaxReSend)
			{
				continue;
			}
			else
				nUnSendSum++;
			break;
		case TRANS_SALE:
			if (stTransRecord.cSendFlag <= nMaxReSend && stTransRecord.cEMV_Status == EMV_STATUS_OFFLINE_SUCC && 
				(stTransRecord.cTransAttr == ATTR_EMV_STANDARD || stTransRecord.cTransAttr == ATTR_PBOC_EC || stTransRecord.cTransAttr == ATTR_qPBOC))
			{
				nUnSendSum++;
			}
			else
			{
				continue;
			}
			break;
		default:
			continue;
		}
	}
	*pnUnSendNum = nUnSendSum;
	PubCloseFile(&nFileHandle);
	return APP_SUCC;
}

int GetFailSendNum(char cFlag)
{
	STTAANSRECORD stTransRecord;
	int nWaterSum;
	int i;
	int nFileHandle;
	int nFailSendSum = 0;
	GetRecordNum(&nWaterSum);
	ASSERT_FAIL(PubOpenFile(FILE_WATER, "r", &nFileHandle));
	for(i=1; i<= nWaterSum;i++)
	{
		PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		switch(stTransRecord.cTransType)
		{
		case TRANS_OFFLINE:
			if(stTransRecord.cSendFlag== cFlag)
				break;
			else
				continue;
		case TRANS_SALE:
			if((stTransRecord.cEMV_Status == EMV_STATUS_OFFLINE_SUCC) && (stTransRecord.cSendFlag==cFlag))
				break;
			else
				continue;
		default:
			continue;
		}
		nFailSendSum++;
	}
	PubCloseFile(&nFileHandle);
	return nFailSendSum;
}

int GetTcNum(int *pnTcNum)
{
	STTAANSRECORD stTransRecord;
	int nWaterSum;
	int i;
	int nFileHandle;
	int nTcNum = 0;
	
	GetRecordNum(&nWaterSum);
	ASSERT_FAIL(PubOpenFile(FILE_WATER, "r", &nFileHandle));
	
	for(i=1; i<= nWaterSum;i++)
	{
		PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		if (memcmp(stTransRecord.szInputMode, INPUTMODE_IC_RFID, 2) == 0
			||memcmp(stTransRecord.szInputMode, INPUTMODE_IC_INSERT, 2) == 0)//amex not sent
		{
			nTcNum++;
		}
	}
	PubCloseFile(&nFileHandle);
	*pnTcNum = nTcNum;
	return APP_SUCC;
}

void testGet()
{
STTAANSRECORD stTransRecord;
memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	FetchLastRecord(&stTransRecord);
TRACE(("===========type: %d", stTransRecord.cTransType));	
}

/**
* @brief Create or clean translog file
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ClearWaterFileByAcqIndex(int nAcqIndex)
{
	int nRet = 0;
	int nWaterSum = 0;
	int nFileHandle;
	int i = 0;
	STTAANSRECORD stTransRecord;
	char szTmp[6092] = {0};
	int nPos = 0;
	char szInvoice[6+1] = {0};
	
	GetRecordNum(&nWaterSum);

	if (0 == nWaterSum)
	{
		return APP_SUCC;
	}
	nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
	ASSERT(nRet);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	
	for (i = 1; i <= nWaterSum; i++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		nRet = PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		if (nAcqIndex == stTransRecord.nAcqIndex)
		{
			memcpy(szTmp+nPos, stTransRecord.szInvoice, 6);
			nPos+=6;
		}
	}
	PubCloseFile(&nFileHandle);

	if (nPos == 0)
	{
		return APP_QUIT;
	}
	
	i = 0;
	while (1)
	{
		if (i >= (nPos/6))
			break;

PubDebugData("DEL invoice: ", szTmp+i*6, 6);
		memset(szInvoice, 0, sizeof(szInvoice));
		memcpy(szInvoice, szTmp+i*6, 6);
		PubDelRec(FILE_WATER, szInvoice, NULL);
		PubDelRec(FILE_EMV_WATER_SUCC, szInvoice, NULL);
		PubDelRec(FILE_EMV_WATER_FAIL, szInvoice, NULL);

		i++;
	}

	return APP_SUCC;
}



/**
* @brief Get translog number
* @param out int *pnRecordNum
* @return void
*/
int CheckRecordNumWithBatch(char *pszBatchNo)
{
	int nRet = 0;
	int nWaterSum = 0;
	int nFileHandle;
	int i = 0;
	STTAANSRECORD stTransRecord;
	char sBatchNo[3] = {0};

	GetRecordNum(&nWaterSum);
	if (0 == nWaterSum)
	{
		return APP_FAIL;
	}
	nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
	ASSERT(nRet);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	
	for (i = 1; i <= nWaterSum; i++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		nRet = PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		PubAscToHex((uchar *)(pszBatchNo), 6, 0, (uchar *)sBatchNo);	
		if (0 == memcmp(stTransRecord.szBatchNum, sBatchNo, 3))
		{
			PubCloseFile(&nFileHandle);
			return APP_SUCC;
		}
	}
	
	PubCloseFile(&nFileHandle);
	return APP_QUIT;
}



/**
* @brief Get translog number
* @param out int *pnRecordNum
* @return void
*/
int CheckIsNeedSettle(int nIndex)
{
	int nRet = 0;
	int nWaterSum = 0;
	int nFileHandle;
	int i = 0;
	STTAANSRECORD stTransRecord;
	char sBatchNo[3] = {0};
	
	GetRecordNum(&nWaterSum);
	if (0 == nWaterSum)
	{
		return APP_FAIL;
	}
	nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
	ASSERT(nRet);
	if (nRet != APP_SUCC)
	{ 
		return APP_FAIL;
	}
	
	for (i = 1; i <= nWaterSum; i++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		nRet = PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		if (stTransRecord.nAcqIndex  == nIndex)
		{
			PubCloseFile(&nFileHandle);
			return APP_SUCC;
		}
	}
	PubCloseFile(&nFileHandle);
	return APP_QUIT;
}

/**
* @brief Find translog with trace number
* @param in const char *szInvno Trace no
* @param in pstTransRecord
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int FindRecordWithOldInvno( const char *pszInvno, STTAANSRECORD *pstTransRecord)
{
	return PubFindRec (FILE_WATER, NULL,pszInvno, 1, (char *)pstTransRecord, &gnCurrentRecNo);
}

