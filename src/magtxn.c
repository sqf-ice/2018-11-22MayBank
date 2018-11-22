/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  magtxn.c
** File indentifier: 
** Synopsis:  Transaction processing
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-15
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

extern int PackGeneral(STSYSTEM *, STTAANSRECORD *);

/**
* internal function statement
*/
static int IsAllowDuplicateTran(char *pszTitle, STSYSTEM *pstSystem);
static int GetSecurityCode(const char *pszTitle, int nAcqIndex, char *pszSecurityCode);


/**
* Interface function implementation
*/

/**
* @brief Magnetic sale
* @param in :pstSystem - transaction system data
* @param out :pstSystem - transaction system data
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int MagSale(STSYSTEM *pstSystem)
{
	int nInputPinNum = 0;
	int nRet = 0;
	char *pszTitle = "SALE";
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	PubDisplayTitle(pszTitle);

	/**
	* Advance dial
	*/
#ifdef DEMO
#else	
	CommPreDial();
#endif

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));
	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));
	}
	
	ASSERT_HANGUP_QUIT(GetSecurityCode(pszTitle, pstSystem->nAcqIndex, pstSystem->szCVV2));
	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;
	/**
	* Get Pin
	*/
	if (pstSystem->nAcqType == EM_ACQTYPE_UPI)
	{
		if (1 == pstSystem->cFlagCashback)
		{
			char szTotalAmt[13] = {0};
			
			AmtAddAmt((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)szTotalAmt);
			ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, szTotalAmt, pstSystem->szPin));
		}
		else
		{
			ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
		}
	}
	
	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);

	//check DCC
	if (YES == CheckIsGoDcc(pstSystem->szPan, pstSystem->cTransAttr))
	{
		MagDCCRequst(pstSystem);
	}
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif

	GetVarInvoiceNo(pstSystem->szInvoice);
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0200", 4);
	memcpy(pstSystem->szProcCode, "000000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL));
	
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);
	nInputPinNum++;
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	nRet = DealPackAndComm(pszTitle,DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	else if(nRet != APP_SUCC)
	{
		CommHangUp();
		return APP_FAIL;
	}

	PubClearAll();
	PubDisplayTitle(pszTitle);
	
	SysToRecord(pstSystem, &stTransRecord);

	nRet = AppendTransRecord(&stTransRecord);
	if (nRet != APP_SUCC)
	{
		strcpy(stReversal.szResponse, "96");
		SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
		CommHangUp();
		return APP_FAIL;
	}

	SetVarIsReversal(NO);
	memset(&stReversal, 0, sizeof(STREVERSAL));
	SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
	ChangeSettle(pstSystem);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	IncVarInvoiceNo();
	PrintWater(&stTransRecord, FIRSTPRINT);
	PubSetPosDateTime(pstSystem->szDate, "MMDD", pstSystem->szTime);

	/**
	* Send offline
	*/
	EmvSendTC();
	SendOffline(0);
	
	CommHangUp();
	
	return APP_SUCC;
}


/**
* Interface function implementation
*/
/**
* @brief Void Sale
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int VoidSale()
{
	int nRet = 0, nInputPinNum=0, nLen;
	char szInvno[6+1] = {0};
	char szTitle[32] = "VOID";
	char szContent[100] = {0};
	STSYSTEM stSystem, stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	int nTrueInput;
	int nPinInputNum=0;
	char szTmpStr[128+1]={0};
	char szTotalAmt[13] = {0};

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	stSystem.cTransType = (char)TRANS_VOID;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(szTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));

	/**
	* Find Transaction log
	*/
	ASSERT_QUIT(PubInputDlg(szTitle, "TRXN NO:", szInvno, &nLen, 1, 6, 0, INPUT_MODE_NUMBER));
	PubAddSymbolToStr(szInvno, 6, '0', 0);
	nRet = FindRecordWithInvno(szInvno, &stTransRecord);
	if (nRet == APP_FAIL)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CINVAID STAN");
		PubMsgDlg(szTitle, szContent, 0, 5);
		return APP_FAIL;
	}
	else
	{
		if ((stTransRecord.cTransType == TRANS_SALE
			|| stTransRecord.cTransType == TRANS_REFUND
			|| stTransRecord.cTransType == TRANS_OFFLINE
			|| stTransRecord.cTransType == TRANS_INSTALLMENT
			|| stTransRecord.cTransType == TRANS_CASH_ADVANCE
			) 
			&& (stTransRecord.cStatus != 0))
		{
			if (2 == stTransRecord.cStatus || 4 == stTransRecord.cStatus)
			{
				PubMsgDlg(szTitle, "Adjusted, Not Allowed", 0, 5);
			}
			else
			{
				PubMsgDlg(szTitle, "Voided, Not Allowed", 0, 5);
			}

			return APP_FAIL;
		}
		if (stTransRecord.cTransType != TRANS_SALE 
			&& stTransRecord.cTransType != TRANS_REFUND
			&& stTransRecord.cTransType != TRANS_OFFLINE 
			&& stTransRecord.cTransType != TRANS_INSTALLMENT 
			&& stTransRecord.cTransType != TRANS_CASH_ADVANCE)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CNot Allow Void");
			PubMsgDlg(szTitle, szContent, 0, 5);
			return APP_FAIL;
		}
		ASSERT_FAIL(DispVoidRecord(szTitle, &stTransRecord));
	}

	RecordToSys(&stTransRecord, &stSystem);//add in 20170427
	switch (stTransRecord.cTransType)
	{
	case TRANS_REFUND:
		strcpy(szTitle, "VOID REFUND");
		stSystem.cTransType	= TRANS_VOID_REFUND;
		break;
	case TRANS_OFFLINE:
		strcpy(szTitle, "VOID OFFLINE");
		stSystem.cTransType	= TRANS_VOID_OFFLINE;
		break;
	case TRANS_INSTALLMENT:
		strcpy(szTitle, "VOID INSTALLMENT");
		stSystem.cTransType	= TRANS_VOID_INSTALLMENT;
		break;	
	case TRANS_CASH_ADVANCE:
		strcpy(szTitle, "VOID CASH ADVANCE");
		stSystem.cTransType	= TRANS_VOID_CASH_ADVANCE;
		break;	
	case TRANS_SALE:
	default:
		strcpy(szTitle, "VOID");
		stSystem.cTransType	= TRANS_VOID;
		break;
		break;
	}

	PubDisplayTitle(szTitle);
	
	if (YES == GetVarIsSaleVoidStrip())
	{
		
		ASSERT_QUIT(Emv_Simple_InputCard(szTitle, &stSystem, &nTrueInput));
		PubHexToAsc((uchar *)stTransRecord.sPan,20,1,(uchar *)szTmpStr);
		if(memcmp(stSystem.szPan, szTmpStr, strlen(stSystem.szPan)) !=0)
		{
			PubMsgDlg(szTitle,"Card No Mismacthing",3,10);
			return APP_FAIL;
		}
		PubDisplayTitle(szTitle);
		CommPreDial();
		ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
	}
	
	stSystemBak = stSystem;
regetpin:
	stSystem = stSystemBak;
	/**
	* Check the switch if need to Get Pin
	*/
	if (nPinInputNum>0 || YES == GetVarIsVoidPin())
	{
		PubDisplayTitle(szTitle);
		if (1 == stSystem.cFlagCashback)
		{
			AmtAddAmt((uchar *)stSystem.szAmount, (uchar *)stSystem.szCashbackAmount, (uchar *)szTotalAmt);
			ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, szTotalAmt, stSystem.szPin));
		}
		else
		{
			ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, stSystem.szAmount, stSystem.szPin));
		}
		/**
		* SET input mode for PIN
		*/
		SetIsPinInput(&stSystem);
	}
	/**
	* Get necessary data
	*/
#if 0
	stSystem.nAcqIndex = stTransRecord.nAcqIndex;//get nii
#endif
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, "0200", 4); 
	if (stSystem.cTransAttr == ATTR_EMV_STANDARD)
	{	
		//20170919 make it not to send TC for void
		stSystem.cTransAttr = ATTR_EMV_PREDIGEST;
	}
	//process code
	memcpy(stSystem.szProcCode, "020000", 6);
	if (stTransRecord.cTransType == TRANS_REFUND)
		memcpy(stSystem.szProcCode, "220000", 6);
	else if (stTransRecord.cTransType == TRANS_INSTALLMENT)
		memcpy(stSystem.szProcCode, "023000", 6);
	
	if (stTransRecord.nAcqIndex == 1)
	{
		stSystem.szProcCode[2] = '4';
	}
	
	memcpy(stSystem.szServerCode, "00", 2);
	memcpy(stSystem.stField62.sOldMsgID, "0200", 4);	
	
	PubHexToAsc((unsigned char *)stTransRecord.sTrace, 6, 0, (unsigned char *)stSystem.stField62.sOldTrace);
	PubHexToAsc((unsigned char *)stTransRecord.sDate,  4, 0, (unsigned char *)stSystem.stField62.sOldDate);
	PubHexToAsc((unsigned char *)stTransRecord.sTime,  6, 0, (unsigned char *)stSystem.stField62.sOldTime);
	memcpy(stSystem.szOldInvoice, stTransRecord.szInvoice, 6);
	memcpy(stSystem.szOldAuthCode, stTransRecord.szAuthCode, 6);//20171020 void use old approve code
	/**************VOID dont need to send DE55*******************/
	stSystem.nAddFieldLen = 0;
	
	if (stSystem.cTransType	!= TRANS_VOID_OFFLINE 
		&& stTransRecord.cEMV_Status != EMV_STATUS_OFFLINE_SUCC)//20170913 脱机交易的撤销也是脱机的
	{
		//check DCC
		if (YES == CheckIsGoDcc(stSystem.szPan, stSystem.cTransAttr))
		{
			MagDCCRequst(stSystem);
		}
		/**
		* Execute Reveral
		*/
		ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
		PubDisplayTitle(szTitle);

		/**
		* Check Connect again
		*/
#ifdef DEMO
#else
		ASSERT_HANGUP_QUIT(CommConnect());
#endif

		
		/**
		* Begin to pack
		*/
		ClrPack();
		ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, &stTransRecord));

		/**
		* Save reversal data
		*/
		SystemToReveral(&stSystem, &stReversal);
		nInputPinNum++;
#ifdef DEMO
		IncVarTraceNo();
		sprintf(stSystem.szRefnum, "%s%s", stSystem.szTime, stSystem.szTrace);
		sprintf(stSystem.szAuthCode, "%.3s000", stSystem.szTrace+3);
#endif
		nRet = DealPackAndComm(szTitle,DPC_SETREVERSAL,&stSystem,&stReversal,nInputPinNum);
		if(APP_REPIN == nRet)
		{
			PubMsgDlg(szTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
			nPinInputNum++;
			goto regetpin;
		}
		if(nRet != APP_SUCC)
		{
			CommHangUp();
			return APP_FAIL;
		}
	}
	else
	{
		IncVarTraceNo();
	}
	/**
	* Add new transaction
	*/
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);

	return TradeComplete(szTitle, TDC_DEFAULT, &stSystem, &stTransRecord, szInvno);
}

/**
* @brief Change System structure to Reveral structure
* @param in  STSYSTEM *pstSystem  System structure
* @param out STREVERSAL *pstReversal Reveral structure
* @return void
*/
void SystemToReveral(const STSYSTEM *pstSystem, STREVERSAL *pstReversal)
{
	pstReversal->cTransType = pstSystem->cTransType;
	pstReversal->cTransAttr = pstSystem->cTransAttr;
	pstReversal->cEMV_Status = pstSystem->cEMV_Status;

	memcpy(pstReversal->szPan, pstSystem->szPan, 19);
	memcpy(pstReversal->szProcCode, pstSystem->szProcCode, 6);
	memcpy(pstReversal->szAmount, pstSystem->szAmount, 12);
	memcpy(pstReversal->szTrace, pstSystem->szTrace, 6);
	memcpy(pstReversal->szExpDate, pstSystem->szExpDate, 4);
	memcpy(pstReversal->szInputMode, pstSystem->szInputMode, 3);
	memcpy(pstReversal->szCardSerialNo, pstSystem->szCardSerialNo, 3);	
	memcpy(pstReversal->szNii, pstSystem->szNii, 3);
	memcpy(pstReversal->szServerCode, pstSystem->szServerCode, 2);
	memcpy(pstReversal->szResponse, "98", 2);		/**<no receive defualt 98*/
	memcpy(pstReversal->stField62.sOldMsgID, pstSystem->szMsgID, 4);
	memcpy(pstReversal->stField62.sOldTrace, pstSystem->szTrace, 6);
	memcpy(pstReversal->stField62.sOldDate, "0000", 4);
	memcpy(pstReversal->stField62.sOldTime, "000000", 6);
	pstReversal->cIsFirstReversal = 0x01;

	if (pstSystem->nAddFieldLen > 0 && pstSystem->nAddFieldLen <= sizeof(pstReversal->szFieldAdd1))
	{
		memcpy(pstReversal->szFieldAdd1, pstSystem->sField55, pstSystem->nAddFieldLen);
		pstReversal->nFieldAdd1Len = pstSystem->nAddFieldLen;
	}
	else
	{
		pstReversal->nFieldAdd1Len = 0;
	}
	memcpy(pstReversal->szInvoice, pstSystem->szInvoice, 6);
	memcpy(pstReversal->szTrack1, pstSystem->szTrack1, sizeof(pstSystem->szTrack1));
	memcpy(pstReversal->szCVV2, pstSystem->szCVV2, sizeof(pstSystem->szCVV2));
	memcpy(pstReversal->szTrack2, pstSystem->szTrack2, sizeof(pstSystem->szTrack2));
	pstReversal->nAcqType = pstSystem->nAcqType;
	pstReversal->nAcqIndex = pstSystem->nAcqIndex;

	pstReversal->cIsUseDcc = pstSystem->cIsUseDcc;
	memcpy(pstReversal->szLocalAmount, pstSystem->szLocalAmount, 12);
	memcpy(pstReversal->szDccCurrencyCode, pstSystem->szDccCurrencyCode, 3);
	memcpy(pstReversal->szField63, pstSystem->szField63, pstSystem->nField63Len);
	pstReversal->cFlagCashback = pstSystem->cFlagCashback;
	if (1 == pstSystem->cFlagCashback)
	{
		memcpy(pstReversal->szCashbackAmount, pstSystem->szCashbackAmount, 12);
	}
	memcpy(pstReversal->szRefnum, pstSystem->szRefnum, 12);
	return ;
}

/**
* @brief Change Reveral structure to System structure
* @param in STREVERSAL *pstReversal Reveral structure
* @param out STSYSTEM *pstSystem System structure
* @return void
*/

void ReveralToSystem(const STREVERSAL *pstReversal, STSYSTEM *pstSystem)
{
	pstSystem->cTransType = pstReversal->cTransType;
	pstSystem->cTransAttr = pstReversal->cTransAttr;
	pstSystem->cEMV_Status = pstReversal->cEMV_Status ;
 
	memcpy(pstSystem->szPan, pstReversal->szPan, 19);
	memcpy(pstSystem->szProcCode, pstReversal->szProcCode, 6);
	memcpy(pstSystem->szAmount, pstReversal->szAmount, 12);
	memcpy(pstSystem->szTrace, pstReversal->szTrace, 6);
	memcpy(pstSystem->szExpDate, pstReversal->szExpDate, 4);
	memcpy(pstSystem->szInputMode, pstReversal->szInputMode, 3);
	memcpy(pstSystem->szCardSerialNo, pstReversal->szCardSerialNo, 3);
	memcpy(pstSystem->szNii, pstReversal->szNii, 4);
	memcpy(pstSystem->szServerCode, pstReversal->szServerCode, 2);
	memcpy(pstSystem->szResponse, pstReversal->szResponse, 2);
	memcpy(pstSystem->szInvoice, pstReversal->szInvoice, 6);
	memcpy(pstSystem->szTrack1, pstReversal->szTrack1, sizeof(pstSystem->szTrack1));
	memcpy(pstSystem->szCVV2, pstReversal->szCVV2, sizeof(pstSystem->szCVV2));
	memcpy(pstSystem->szTrack2, pstReversal->szTrack2, sizeof(pstSystem->szTrack2));

	if (pstReversal->nFieldAdd1Len > 0)
	{
		memcpy(pstSystem->sField55, pstReversal->szFieldAdd1, pstReversal->nFieldAdd1Len);
		pstSystem->nAddFieldLen = pstReversal->nFieldAdd1Len ;
	}
	else
	{
		pstSystem->nAddFieldLen = 0;
	}
	pstSystem->nAcqType = pstReversal->nAcqType;
	pstSystem->nAcqIndex = pstReversal->nAcqIndex;

	pstSystem->cIsUseDcc = pstReversal->cIsUseDcc;
	memcpy(pstSystem->szLocalAmount, pstReversal->szLocalAmount, 12);
	memcpy(pstSystem->szDccCurrencyCode, pstReversal->szDccCurrencyCode, 3);
	memcpy(pstSystem->szField63, pstReversal->szField63, pstReversal->nField63Len);
	pstSystem->cFlagCashback = pstReversal->cFlagCashback;
	if (1 == pstReversal->cFlagCashback)
	{
		memcpy(pstSystem->szCashbackAmount, pstReversal->szCashbackAmount, 12);
	}
	memcpy(pstSystem->szRefnum, pstReversal->szRefnum, 12);
	return ;
}

/**
* @brief Auto Reversal
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int DoReversal(void)
{
	int nRet = 0;
	int nPackLen = 0;
	int nFieldLen = 0;
	int nReSend = 0;
	int nConnectFailNum = 0 ;
	char sPackBuf[MAX_PACK_SIZE];
	char cMaxReSend;
	char szContent[100] = {0};
	char szDispBuf[30] = {0};
	STSYSTEM stSystem;
	STREVERSAL stReversal;
	char sMac[8] = {0};

	if (YES != GetVarIsReversal())
	{
		SetVarHaveReversalNum(0);
		return APP_SUCC;
	}
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	GetVarCommReSendNum((char *)&cMaxReSend);
	GetVarHaveReversalNum(&nReSend);/**<Get times have reversal*/

	GetVarReversalData((char *)&stReversal, &nFieldLen);
	memset(&stSystem, 0, sizeof(STSYSTEM));
	DealSystem(&stSystem);
	ReveralToSystem(&stReversal, &stSystem);

	nConnectFailNum = 0;
cMaxReSend = 1;

	while(1)//nReSend <= cMaxReSend
	{
		GetVarReversalData((char *)&stReversal, &nFieldLen);
		DealSystem(&stSystem);
		ReveralToSystem(&stReversal, &stSystem);
//		PubDisplayTitle("REVERSAL");//shtest 
		nRet = CommConnect();
		if (nRet != APP_SUCC)
		{
		       nConnectFailNum++;
			if(nConnectFailNum >= cMaxReSend)
			{
				return APP_FAIL;
			}
			else
			{
				continue;
			}
		}
		nConnectFailNum = 0;

		memcpy(stSystem.szMsgID, "0400", 4);
		/**
		* Begin to pack
		*/
		PackGeneral(&stSystem, NULL);
		ASSERT_FAIL(Pack(sPackBuf, &nPackLen));
		nPackLen -= 8;
		ASSERT_FAIL(AddMac(sPackBuf, &nPackLen, KEY_TYPE_MAC));
	#if 0
		GetMac(sPackBuf, &nPackLen, sMac);
		ASSERT_HANGUP_FAIL(ProcessTLED(&stSystem));
		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		memcpy(sPackBuf+nPackLen-8, sMac, 8);
	#endif	
		//nReSend++;    
		//SetVarHaveReversalNum(nReSend);
		IncVarTraceNo();
		nRet = CommSendRecv(sPackBuf, nPackLen, sPackBuf, &nPackLen);
		if (nRet != APP_SUCC)
		{
			strcpy(stReversal.szResponse, "98");
			SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
			//continue;
			return APP_QUIT;
		}

		strcpy(stReversal.szResponse, "06");
		SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));

		nRet = Unpack(sPackBuf, nPackLen);
		if (nRet != APP_SUCC)
		{
			//continue;
			return APP_QUIT;
		}
		nRet = ChkRespMsgID("0400", sPackBuf);
		if (nRet != APP_SUCC)
		{
			//continue;
			return APP_QUIT;
		}
		nRet = ChkRespon(&stSystem, sPackBuf + 2);
		if (nRet != APP_SUCC)
		{
			//continue;
			return APP_FAIL;
		}
#if 0
		if (stSystem.cMustChkMAC == 0x01)
		{
			//decrypt sentitive data
			if (APP_SUCC == ParseTLED(&stSystem))
			{
				ASSERT_FAIL(DelField(57));
				
				ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		PubDebugData("mac repack data: ", sPackBuf,nPackLen );
				
			}
			nRet = CheckMac(sPackBuf, nPackLen);
			if (nRet != APP_SUCC)
			{
				strcpy(stReversal.szResponse, "A0");
				SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CMAC FROM HOST IS ERR");
				PubMsgDlg("REVERSAL", szContent, 3, 1);
				return APP_FAIL;
			}
		}
#endif
		if (stSystem.cMustChkMAC == 0x01)
		{
			nRet = CheckMac(sPackBuf, nPackLen);
			if (nRet != APP_SUCC)
			{
				strcpy(stReversal.szResponse, "A0");
				SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CMAC FROM HOST IS ERR");
				PubMsgDlg(tr("REVERSAL"), szContent, 3, 1);
				return APP_FAIL;
			}
		}
		
		PubClear2To4();
		if ((memcmp(stSystem.szResponse, "00", 2) == 0))
		{
			;
		}
		else if(stSystem.nAcqIndex == 1 
			&& (memcmp(stSystem.szResponse, "08", 2)==0 || memcmp(stSystem.szResponse, "88", 2)==0))
		{
			;
		}
		else
		{
			PubDisplayTitle("REVERSAL");
			if (memcmp(stSystem.szResponse, "LE", 2) == 0)
			{
				DispRespEFTInfo();
			}
			else
			{
				DispResp(stSystem.szResponse, stSystem.nAcqIndex);
			}
			//PubGetStrFormat(0, szDispBuf, "|CREVERSAL FAIL");
			//PubMsgDlg(NULL, szDispBuf, 0, 3);
			//continue;
			return APP_QUIT;
		}
		SetVarIsReversal(NO);
		SetVarHaveReversalNum(0);
		break;
	}

#if 0
	if (nReSend > cMaxReSend)
	{
		PubGetStrFormat(0, szDispBuf, "|CREVERSAL FAIL");
		PubMsgDlg(NULL, szDispBuf, 0, 3);
		if (YES == GetVarIsPrintErrReport())
		{
			STTAANSRECORD stTransRecord;

			memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
			stSystem.cTransAttr = 0;
			SysToRecord(&stSystem, &stTransRecord);
			PrintWater(&stTransRecord, REVERSAL_PRINT);
		}
		SetVarIsReversal(NO); //shtest
		SetVarHaveReversalNum(0);
	}
	else
#endif
	{
		/*PubClearAll();
		PubDisplayGen(3, "REVERSAL SUCC");
		PubUpdateWindow();
		PubGetKeyCode(1);*/
	}
	return APP_SUCC;
}

/**
* @brief Magnetic Preauth
* @param in STSYSTEM *pstSystem
*		out STSYSTEM *pstSystem
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int MagPreAuth(STSYSTEM *pstSystem)
{
	int nRet = 0, nInputPinNum=0;
	char *pszTitle = "PREAUTH";
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	
	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, pstSystem->cTransType, 0, YES));

	/**
	* Advance dial
	*/
#ifdef DEMO
#else
	CommPreDial();
#endif

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));
	}
	ASSERT_HANGUP_QUIT(GetSecurityCode(pszTitle, pstSystem->nAcqIndex, pstSystem->szCVV2));

	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	/**
	* Get Pin
	*/
	if (pstSystem->nAcqType == EM_ACQTYPE_UPI)
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}
	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);
	
	//check DCC
	if (YES == CheckIsGoDcc(pstSystem->szPan, pstSystem->cTransAttr))
	{
		MagDCCRequst(pstSystem);
	}
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0100", 4);
	memcpy(pstSystem->szProcCode, "000000", 6);
	memcpy(pstSystem->szServerCode, "06", 2);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL));
	
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);

	nInputPinNum++;
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	nRet = DealPackAndComm(pszTitle,DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if(nRet != APP_SUCC)
	{	
		CommHangUp();
		return APP_FAIL;
	}

	SysToRecord(pstSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);
}

/**
* @brief Magnetic refund
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int MagRefund()
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nTrueInput=0;
	char *pszTitle = "REFUND";
	STREVERSAL stReversal;

	memset(&stReversal, 0, sizeof(STREVERSAL));
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = (char)TRANS_REFUND;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));
	
	ASSERT_QUIT(Emv_Simple_InputCard(pszTitle, &stSystem, &nTrueInput));
TRACE(("====refund===="));
	/**
	* Advance dial
	*/
#ifdef DEMO
#else
	CommPreDial();
#endif
	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
TRACE(("====refund===="));
	/**
	* Input Amount
	*/
	ASSERT_HANGUP_QUIT(EdcGetAmout(pszTitle, &stSystem));
TRACE(("====refund===="));

	/**
	*Get Pin always no need in refund
	*/
TRACE(("====refund===="));

	//check DCC
	if (YES == CheckIsGoDcc(stSystem.szPan, stSystem.cTransAttr))
	{
		MagDCCRequst(stSystem);
	}
TRACE(("====refund===="));
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);
TRACE(("====refund===="));

	/**
	* Check Connect again
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
TRACE(("====refund===="));

	DealSystem(&stSystem);

	memcpy(stSystem.szMsgID, "0200", 4);
	memcpy(stSystem.szProcCode, "200000", 6);
	memcpy(stSystem.szServerCode, "00", 2);

	/**
	* Pack
	*/
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL));
TRACE(("====refund===="));

	/**
	* Save reversal data
	*/
	SystemToReveral(&stSystem, &stReversal);
TRACE(("====refund===="));
#ifdef DEMO
	IncVarTraceNo();
	sprintf(stSystem.szRefnum, "%s%s", stSystem.szTime, stSystem.szTrace);
	sprintf(stSystem.szAuthCode, "%.3s000", stSystem.szTrace+3);
#endif
TRACE(("====refund===="));
	ASSERT_HANGUP_FAIL(DealPackAndComm(pszTitle,DPC_SETREVERSAL,&stSystem,&stReversal,3));
TRACE(("====refund===="));
	
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);
TRACE(("====refund===="));

	return TradeComplete(pszTitle, TDC_DEFAULT, &stSystem, &stTransRecord, NULL);
}

/**
* @brief Magnetic Offline
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int OfflineSale(void)
{
	int nTrueInput;
	char *pszTitle = "OFFLINE SALE";
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nLen=0;
	STREVERSAL stReversal;
	int nRet=0;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	stSystem.cTransType = (char)TRANS_OFFLINE;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, 0, YES));

	/**
	* Input Amount
	*/
	ASSERT_QUIT(EdcGetAmout(pszTitle, &stSystem));
	
	ASSERT_QUIT(Emv_Simple_InputCard(pszTitle, &stSystem, &nTrueInput));

	/**
	* Advance dial
	*/
//	CommPreDial();
	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
	
	/**
	* Input old auth code
	*/
	nLen = 6;
	ASSERT_HANGUP_QUIT(PubInputDlg(pszTitle, "AUTH CODE:", stSystem.szAuthCode, &nLen, 1, 6, INPUT_STRING_TIMEOUT, INPUT_MODE_STRING));
	PubAddSymbolToStr(stSystem.szAuthCode, 6, ' ', 1);


	//check DCC
	if (YES == CheckIsGoDcc(stSystem.szPan, stSystem.cTransAttr))
	{
		stSystem.cIsUseDcc = 1;
	}
		
	/**
	* Get necessary data
	*/
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, "0220", 4);
	memcpy(stSystem.szProcCode, "000000", 6);
	memcpy(stSystem.szServerCode, "00", 2);		

	IncVarTraceNo();
	PubDisplayTitle(pszTitle);
	SysToRecord(&stSystem, &stTransRecord);
	nRet = AppendTransRecord(&stTransRecord);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	ChangeSettle(&stSystem);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	IncVarInvoiceNo();
	IncVarOfflineUnSendNum();
	PrintWater(&stTransRecord, FIRSTPRINT);
	
	return APP_SUCC;
}

/**
* @brief Send Offline
* @parm int const char cSendFlag =0 Use when online
* @parm int const char cSendFlag =1 Use when settle
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int SendOffline(const char cSendFlag)
{
	int nRet = 0;
	int nWaterSum = 0;
	int nLoop = 0;
	int nFileHandle;
	int nSendNum = 0;
	int nReSend = 0;
	int nOfflineUnSendNum = 0;
	int nCurrentSendNum = 0;	
	char nMaxReSend;
	char *pszTitle = "SEND OFFLINE";
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;

#ifdef DEMO
	return APP_SUCC;
#endif
	if (0 == cSendFlag)//only send when settlement
	{
		return APP_QUIT;
	}
	
TRACE(("====SendOffline====: %d", cSendFlag));
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	nOfflineUnSendNum = GetVarOfflineUnSendNum();
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
	TRACE(("nOfflineUnSendNum=%d cSendFlag=%d", nOfflineUnSendNum, cSendFlag));
	if(nOfflineUnSendNum <= 0)
	{
		TRACE(("unsendnum=%d",nOfflineUnSendNum));
		return APP_SUCC;
	}
	GetRecordNum(&nWaterSum);
	TRACE(("nWaterSum=%d",nWaterSum));

	if (nWaterSum > 0)
	{
		nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
		if (nRet != APP_SUCC)
		{
			TRACE(("open file %s error nRet=%d", FILE_WATER, nRet));
			return APP_FAIL;
		}
	}
	else
	{
		TRACE(("nWaterSum=%d",nWaterSum));
		return APP_SUCC;
	}
	PubClear2To4();

TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
	GetVarCommOffReSendNum(&nMaxReSend);
	SetVarHaveReSendNum(nReSend);
	for (; nReSend <= nMaxReSend; nReSend++, SetVarHaveReSendNum(nReSend))
	{
		nCurrentSendNum = 0;
		for (nLoop = 1; nLoop <= nWaterSum; nLoop++)
		{
			memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
			PubReadRec(nFileHandle, nLoop, (char *)&stTransRecord);

TRACE(("----: %d", stTransRecord.cTransType));
			switch (stTransRecord.cTransType)
			{
			case TRANS_OFFLINE:
				if (stTransRecord.cStatus == 0x01)// had void .no send
				{
					continue;
				}
				if (stTransRecord.cSendFlag > nMaxReSend)
				{
					continue;
				}
				break;
			case TRANS_SALE:
TRACE(("33==SendOffline====: %d", nOfflineUnSendNum));
				if (stTransRecord.cStatus == 0x01)// had void .no send
				{
					continue;
				}
				if (stTransRecord.cSendFlag <= nMaxReSend && stTransRecord.cEMV_Status == EMV_STATUS_OFFLINE_SUCC &&
				        (stTransRecord.cTransAttr == ATTR_EMV_STANDARD || stTransRecord.cTransAttr == ATTR_PBOC_EC || stTransRecord.cTransAttr == ATTR_qPBOC))
				{
					;
				}
				else
				{
					continue;
				}
				break;
			default:
				continue;
			}
			nCurrentSendNum++;
			nSendNum ++;
			
//			PubDisplayTitle(pszTitle);
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
			nRet = CommConnect();
			if (nRet != APP_SUCC)
			{
				PubCloseFile(&nFileHandle);
				return APP_QUIT;
			}
//			PubClearAll();
//			PubDisplayTitle(pszTitle);

			switch (stTransRecord.cTransType)
			{
			case TRANS_OFFLINE:
			case TRANS_SALE:
				RecordToSys(&stTransRecord, &stSystem);
				if (1 == stSystem.cIsUseDcc)
				{
					if (APP_SUCC != MagDCCRequst(&stSystem))
					{
						PubCloseFile(&nFileHandle);
						return APP_QUIT;
					}
				}
				
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
				DealSystem(&stSystem);
				memcpy(stSystem.szMsgID, "0220", 4);
				memcpy(stSystem.szProcCode, "000000", 6);
				memcpy(stSystem.szServerCode, "00", 2);			
				nRet = PackGeneral(&stSystem, &stTransRecord);
				if (nRet != APP_SUCC)
				{
					PubMsgDlg(pszTitle, "PACKED FAIL", 3, 10);
					PubCloseFile(&nFileHandle);
					return APP_QUIT;
				}
				break;
			default:
				break;
			}
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
			SetVarHaveReSendNum(nReSend+1);
//			PubDisplayTitle(pszTitle);
			nRet = DealPackAndComm(pszTitle,DPC_NOCHECKRESP,&stSystem,NULL,3);
			if(nRet != APP_SUCC)
			{
//				return APP_QUIT;//shtest
				goto SENDFAIL;
			}
			PubClear2To4();

TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
			if (memcmp(stSystem.szResponse, "00", 2) != 0 && memcmp(stSystem.szResponse, "94", 2) != 0)
			{
				if (memcmp(stSystem.szResponse, "LE", 2) == 0)
				{
					DispRespEFTInfo();
				}
				else
				{
					DispResp(stSystem.szResponse, stSystem.nAcqIndex);
				}
				stTransRecord.cSendFlag = 0xFE;
				DelVarOfflineUnSendNum();
				goto SENDFAIL;
			}
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));

//			PubDisplayGen(3, "Send offline OK");
//			PubUpdateWindow();
//			NDK_SysMsDelay(1000);
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));

			stTransRecord.cSendFlag = 0xFD;
			DelVarOfflineUnSendNum();
			memcpy(stTransRecord.szRefnum, stSystem.szRefnum, 12);
SENDFAIL:
			if (stTransRecord.cSendFlag != 0xFE && stTransRecord.cSendFlag != 0xFD)
			{
				if(stTransRecord.cSendFlag >= nMaxReSend)
				{
					stTransRecord.cSendFlag = 0xFF;
					DelVarOfflineUnSendNum();
				}
				else
				{
					stTransRecord.cSendFlag++;
				}
			}
			if(stTransRecord.cSendFlag != 0)
			{
				nRet = PubUpdateRecByHandle(nFileHandle, nLoop, (char *)&stTransRecord);
				if (nRet != APP_SUCC)
				{
					SetVarHaveReSendNum(0);
					PubCloseFile(&nFileHandle);
					return APP_FAIL;
				}
			}
			
			continue;
		}
		if(nSendNum == 0)		
		{
			SetVarHaveReSendNum(0);
			PubCloseFile(&nFileHandle);
			return APP_SUCC;
		}
	}
TRACE(("====SendOffline====: %d", nOfflineUnSendNum));
	SetVarHaveReSendNum(0);
	PubCloseFile(&nFileHandle);
	return APP_SUCC;
}

int DoAdviseAndReveral(void)
{
	int nRet=0;

	nRet = DoReversal();
	return nRet;
}

int InputCard(const int nInputType, STSYSTEM *pstSystem)
{
	int nRet;
	int nInput = nInputType;
	int nTrueInput;

	while(1)
	{
		ASSERT_QUIT(CardTaskAmt(NULL,nInput, STRIPE_TIMEOUT, &nTrueInput));

		if (nTrueInput == INPUT_STRIPE)
		{
			memcpy(pstSystem->szInputMode, "021", 3);
			GetTrack(pstSystem->szTrack1, pstSystem->szTrack2, pstSystem->szTrack3);
			GetExpDateFromTk2(pstSystem->szTrack2, pstSystem->szExpDate);
			GetNameFromTk1(pstSystem->szTrack1, pstSystem->szHolderName);
			if (YES == GetVarIsIcFlag()
			        && pstSystem->cTransAttr != (char)ATTR_FALLBACK			   )
			{
				nRet = ChkIsICC(pstSystem->szTrack2, NULL);
				if (nRet == APP_SUCC)
				{
					nInput = INPUT_INSERTIC;
					continue;
				}
				else if (nRet == APP_QUIT)
				{
					return APP_QUIT;
				}
			}
			break;
		}
		else if (nTrueInput == INPUT_KEYIN)
		{
			memcpy(pstSystem->szInputMode, "011", 3);
			break;
		}
		else if (nTrueInput == INPUT_INSERTIC)
		{
			emv_opt stEMVOption;
			memset(&stEMVOption, 0, sizeof(stEMVOption));
			stEMVOption._request_amt = EMV_TRANS_REQAMT_NO;
			stEMVOption._seq_to = EMV_PROC_TO_READAPPDATA;
			nRet = EmvReadICC(&stEMVOption, pstSystem);
			SaveEmvTransSerial();
			EMV_Suspend(nRet);
			if (nRet == APP_FALLBACK)
			{
				nInput = INPUT_STRIPE_FALLBACK;
				pstSystem->cTransAttr = (char)ATTR_FALLBACK;
				continue;
			}
			else if (nRet == APP_SUCC)
			{
				ASSERT_FAIL(CheckIsOnlyEC());
				pstSystem->cTransAttr = (char)ATTR_EMV_PREDIGEST;
				break;
			}
			else
			{
				return APP_FAIL;
			}
		}
	}
	return APP_SUCC;
}


YESORNO IsChipTrans(const char *pszInputMode)
{
	if (memcmp(pszInputMode, "02", 2) == 0
		|| memcmp(pszInputMode, "80", 2) == 0
		|| memcmp(pszInputMode, "01", 2) == 0)
	{
		return NO;
	}
	return YES;
}

static int IsAllowDuplicateTran(char *pszTitle, STSYSTEM *pstSystem)
{
	STTAANSRECORD stTransRecord;
	char szPan[19+1] = {0};
	char szAmt[12+1] = {0};
	char szTipAmt[12+1] = {0};
	
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	if (APP_SUCC != FetchLastRecord(&stTransRecord))
	{
		return YES;
	}

	PubHexToAsc((uchar *)stTransRecord.sPan, 20, 1, (uchar *)szPan);
	PubHexToAsc((uchar *)stTransRecord.sAmount, 12, 0, (uchar *)szAmt);
	PubHexToAsc((uchar *)stTransRecord.sTipAmount, 12, 0, (uchar *)szTipAmt);
	
	if( stTransRecord.cTransType != pstSystem->cTransType ||
		strncmp(szPan, pstSystem->szPan, 19)!=0 ||
		memcmp(szAmt, pstSystem->szAmount, 12)!=0 ||
		memcmp(szTipAmt, pstSystem->szTipAmount, 12)!=0 
		)
	{
		return YES;
	}
	if (APP_SUCC != PubConfirmDlg(pszTitle, "DUPLICATE? Y/N", 0, 60))
	{
		return NO;
	}
	return YES;
	
}

//Input amount
int EdcGetAmout(char *pszTitle, STSYSTEM *pstSystem)
{
	char szContent[256] = {0};
	int nAmtLen = 12;
	char szTotalAmt[12+1] = {0}, szDispAmt[14] = {0};
	
	while(1)
	{
		if (pstSystem->cFlagCashback == 1)
		{
			strcpy(szContent, "SALE AMOUNT:");
			memset(pstSystem->szAmount, 0, sizeof(pstSystem->szAmount));
			ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
			if (CheckTransAmount(pstSystem->szAmount, pstSystem->cTransType, pstSystem->nAcqIndex) != APP_SUCC )
			{
				continue;
			}

			while (1)
			{
				strcpy(szContent, "CASHBACK AMOUNT:");
				memset(pstSystem->szCashbackAmount, 0, sizeof(pstSystem->szCashbackAmount));
				ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szCashbackAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
				if (CheckTransAmount(pstSystem->szCashbackAmount, TRANS_CASHBACK, pstSystem->nAcqIndex) != APP_SUCC )
				{
					continue;
				}
				if (APP_SUCC != CheckCashbackAmt(pszTitle, pstSystem->szCashbackAmount))
				{
					continue;
				}
TRACE(("+++++++"));
				if (AtoLL(pstSystem->szCashbackAmount) == 0)
				{
					pstSystem->cFlagCashback = 0;
				}
TRACE(("+++++++"));
				break;
			}
		}
		else
		{

			if (YES == GetVarIsTipFlag() && TRANS_SALE == pstSystem->cTransType)
			{
				strcpy(szContent, "SALE AMOUNT:");
				memset(pstSystem->szAmount, 0, sizeof(pstSystem->szAmount));
				ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
				if (CheckTransAmount(pstSystem->szAmount, pstSystem->cTransType, pstSystem->nAcqIndex) != APP_SUCC )
				{
					continue;
				}

				while (1)
				{
					strcpy(szContent, "TIPS:");
					memset(pstSystem->szTipAmount, 0, sizeof(pstSystem->szTipAmount));
					ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szTipAmount, &nAmtLen, INPUT_AMOUNT_MODE_NONE, INPUT_AMOUNT_TIMEOUT));
					if (CheckTransAmount(pstSystem->szTipAmount, TRANS_SALE, pstSystem->nAcqIndex) != APP_SUCC )
					{
						continue;
					}
PubDebug("+++++++++++%d[%s]", strlen(pstSystem->szTipAmount), pstSystem->szTipAmount);
					if (AtoLL(pstSystem->szTipAmount) > 0)
					{
						PubAscAddAsc((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szTipAmount, (uchar *)szTotalAmt);
						strcpy(pstSystem->szAmount, szTotalAmt);
						PubAddSymbolToStr(pstSystem->szAmount,12,'0',ADDCH_MODE_BEFORE_STRING);					
						pstSystem->cTipFlag = 1;
					}
					break;
				}
			}
			else
			{
				strcpy(szContent, "ENTER AMOUNT:");
				memset(pstSystem->szAmount, 0, sizeof(pstSystem->szAmount));
				ASSERT_HANGUP_QUIT(PubInputAmount(pszTitle, szContent, pstSystem->szAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
				if (CheckTransAmount(pstSystem->szAmount, pstSystem->cTransType, pstSystem->nAcqIndex) != APP_SUCC )
				{
					continue;
				}
			}
			
		}

#if 0
		/**<确定总额*/
		memset(szTotalAmt, 0, sizeof(szTotalAmt));
		if (1 == pstSystem->cFlagCashback)
		{
			PubAscAddAsc((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)szTotalAmt);
			PubDisplayGen(2, "TOTAL:");
		}
		else if (1 == pstSystem->cTipFlag)
		{
			PubAscAddAsc((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szTipAmount, (uchar *)szTotalAmt);
			strcpy(pstSystem->szAmount, szTotalAmt);
			PubAddSymbolToStr(pstSystem->szAmount,12,'0',ADDCH_MODE_BEFORE_STRING);
			PubDisplayGen(2, "TOTAL:");
		}
		else
		{
			strcpy(szTotalAmt, pstSystem->szAmount);
			PubDisplayGen(2, "AMOUNT:");
		}
#endif		
		/**<检查金额的有效性*/
		if (CheckTransAmount(szTotalAmt, pstSystem->cTransType, pstSystem->nAcqIndex) != APP_SUCC )
		{
			continue;
		}
#if 0 // no need to confirm amount
		ProAmtToDispOrPnt(szTotalAmt, szDispAmt);
		PubAllTrim(szDispAmt);
		PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 3, "|R%s",szDispAmt);
		PubDisplayGen(4, "CORRECT ? Y/N");
		if (APP_SUCC == ProConfirm())
		{
			break;
		}
#else
		break;
#endif
	}

	//EDC 判断交易是否重复
	IsAllowDuplicateTran(pszTitle, pstSystem);
	return APP_SUCC;

}

int MagBalance(STSYSTEM *pstSystem)
{
	int nRet = 0, nInputPinNum=0;
	char *pszTitle = "BALANCE";
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char szAmt[14] = {0};
		
	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	
	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, pstSystem->cTransType, 0, YES));

	/**
	* Advance dial
	*/	
	CommPreDial();


	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));
	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));
	}

	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	/**
	* Get Pin
	*/
	if (pstSystem->nAcqType == EM_ACQTYPE_UPI)
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0100", 4);
	memcpy(pstSystem->szProcCode, "310000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);
	memcpy(pstSystem->szInputMode, "021", 3);//always 021

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL));
	
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);

	nInputPinNum++;
	nRet = DealPackAndComm(pszTitle,DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if(nRet != APP_SUCC)
	{	
		CommHangUp();
		return APP_FAIL;
	}

	PubClear2To4();
	ProAmtToDispOrPnt(pstSystem->szAmount, szAmt);
	PubDisplayStrInline(1, 2, "balance: ");
	PubDisplayStr(DISPLAY_MODE_NORMAL, 3, 1, "RM %s", szAmt);
	PubDisplayStrInline(1, 5, "[ENTER] To Print");
	ASSERT_QUIT(ProConfirm());
	SysToRecord(pstSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);
}


int MagInstallment(STSYSTEM *pstSystem)
{
	int nRet = 0, nInputPinNum=0;
	char *pszTitle = "INSTALLMENT";
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	int nLen = 0;

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	
	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, pstSystem->cTransType, 0, YES));

	/**
	* Advance dial
	*/
#ifdef DEMO
#else	
	CommPreDial();
#endif
	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));
	}
	ASSERT_HANGUP_QUIT(GetSecurityCode(pszTitle, pstSystem->nAcqIndex, pstSystem->szCVV2));

	ASSERT_HANGUP_QUIT(SelectEppMonth(pszTitle, &pstSystem->nInstallmentMonth));
	UpdateAcqIndexFromEpp(&pstSystem->nAcqIndex, pstSystem->nInstallmentMonth);

	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	/**
	* Get Pin
	*/
	if (pstSystem->nAcqType == EM_ACQTYPE_UPI)
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
#ifdef DEMO
#else	
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0100", 4);
	memcpy(pstSystem->szProcCode, "000000", 6);//003000
	memcpy(pstSystem->szServerCode, "06", 2);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL));
	
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);

	nInputPinNum++;

#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif	
	nRet = DealPackAndComm(pszTitle,DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if(nRet != APP_SUCC)
	{	
		CommHangUp();
		return APP_FAIL;
	}

#ifdef DEMO
	sprintf(pstSystem->szInstallmentInfo, "001%02d1001010000000086300000086320000086300000517820000500000000017820000000000000", pstSystem->nInstallmentMonth);
#else
	nLen = 99;
	ASSERT_QUIT(GetField(61, pstSystem->szInstallmentInfo, &nLen));
#endif	
	
	SysToRecord(pstSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);
}

//0 V/M/J; 1 AMEX; 2 MCCS; 3 UPI; 4 V/M Epp; 5 AMEX EPP
int SetDE22EntryMode(char *pszInputMode, int nAcqIndex, char cAttr)
{
	memcpy(pszInputMode, "001", 3);
	
	switch (nAcqIndex)
	{
	case EM_ACQINDEX_AMEX://AMEX EM_ACQINDEX_V_M_J
		pszInputMode[0] = '5';
		pszInputMode[2] = '3';
		switch (cAttr)
		{
		case ATTR_MAG:
			pszInputMode[1] = '6';//not input 4dbc use '2'
			break;
		case ATTR_EMV_STANDARD:
		case ATTR_EMV_PREDIGEST:
			pszInputMode[1] = '5';
			break;
		case ATTR_qPBOC:
			break;
		case ATTR_FALLBACK:
			pszInputMode[0] = '6';
			pszInputMode[1] = '6';//not input 4dbc use '2'
			break;
		case ATTR_MANUAL:
			pszInputMode[1] = '7';//not input 4dbc use '1'
			break;
		default:
			break;
		}
		break;
	case EM_ACQINDEX_MCCS:
		switch (cAttr)
		{
		case ATTR_EMV_STANDARD:
		case ATTR_EMV_PREDIGEST:
			pszInputMode[0] = '2';
			pszInputMode[1] = '6';
			break;
		case ATTR_FALLBACK:
			pszInputMode[0] = '2';
			pszInputMode[1] = '2';
			break;
		case ATTR_qPBOC:
			pszInputMode[0] = '2';
			pszInputMode[1] = '7';
			pszInputMode[2] = '0';
			break;
		case ATTR_MAG:
			pszInputMode[0] = '0';
			pszInputMode[1] = '2';
			break;
		case ATTR_MANUAL:
			pszInputMode[0] = '3';
			pszInputMode[1] = '1';
			break;
		default:
			break;
		}
		break;
	default:
		switch (cAttr)
		{
		case ATTR_EMV_STANDARD:
		case ATTR_EMV_PREDIGEST:
			pszInputMode[1] = '5';
			break;
		case ATTR_FALLBACK:
			pszInputMode[0] = '8';
			break;
		case ATTR_qPBOC:
			pszInputMode[1] = '7';
			break;
		case ATTR_MAG:
			pszInputMode[1] = '2';
			break;
		case ATTR_MANUAL:
			pszInputMode[1] = '1';
			break;
		default:
			break;
		}
		break;
	}


	return APP_SUCC;
}

static int GetSecurityCode(const char *pszTitle, int nAcqIndex, char *pszSecurityCode)
{
	int nLen = 0; 

	if (YES != GetVarIsInputCvv2())
	{
		return APP_SUCC;
	}
	
	if (3 == nAcqIndex)//UPI no need the cvv2 or 4DBC
	{
		return APP_SUCC;
	}
	
	if (1 == nAcqIndex)
	{
		ASSERT_HANGUP_QUIT(PubInputDlg(pszTitle, "Input 4DBC:", pszSecurityCode, &nLen, 0, 4, 0, INPUT_MODE_STRING));
	}
	else
	{
		ASSERT_HANGUP_QUIT(PubInputDlg(pszTitle, "Input CVV2:", pszSecurityCode, &nLen, 0, 3, 0, INPUT_MODE_STRING));
	}
	return APP_SUCC;
}

int GetExpiryDate(const char *pszTitle, char *pszDate)
{
	int  nLen = 0;
	char szDateTime[14+1]={0};
	char szDate[8+1]={0};
	char szInputDate[4+1] = {0};


	while (1)
	{
		PubGetCurrentDatetime(szDateTime);
		memcpy(szDate, szDateTime, 8);

		ASSERT_QUIT(PubInputDlg(pszTitle, "Expiry Date(MMYY):", szInputDate, &nLen, 4, 4, 60, INPUT_MODE_NUMBER));
		sprintf(pszDate, "%.2s%.2s", szInputDate+2, szInputDate);
			
		memcpy(szDate+2, pszDate, 4);
#if 0//不需要检查
		if(PubIsValidDate(szDate) != APP_SUCC)
		{
			PubMsgDlg(pszTitle, "INVALID INPUT", 3, 5);
			return APP_FAIL;
		}
#endif		
		break;
	}

	return APP_SUCC;
}

int DealEppMonthMenu(char *pszContent)
{
//	if (YES == GetEppMonthSwitchOnoff(EM_MENU_EPP_MONTH3))
//		strcat(pszContent, "3 months, 3/");
	if (YES == GetEppMonthSwitchOnoff(EM_MENU_EPP_MONTH6))
		strcat(pszContent, "6 months, 6/");
	if (YES == GetEppMonthSwitchOnoff(EM_MENU_EPP_MONTH12))
		strcat(pszContent, "12 months, 12/");
	if (YES == GetEppMonthSwitchOnoff(EM_MENU_EPP_MONTH18))
		strcat(pszContent, "18 months, 18/");
	if (YES == GetEppMonthSwitchOnoff(EM_MENU_EPP_MONTH24))
		strcat(pszContent, "24 months, 24/");

	return APP_SUCC;
}

int SelectEppMonth(const char *pszTitle, int *pnMonth)
{
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.6 months"),
		tr("2.12 months"),
		tr("3.18 months"),
		tr("4.24 months")
	};

	while(1)
	{
		nRet = PubShowMenuItems(pszTitle, pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			*pnMonth = 6;
			break;
		case 2:
			*pnMonth = 12;
			break;
		case 3:
			*pnMonth = 18;
			break;
		case 4:
			*pnMonth = 24;
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

void SetIsPinInput(STSYSTEM *pstSystem)
{
	if (pstSystem->nAcqIndex != 1)
	{
		if (memcmp(pstSystem->szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
		{
			pstSystem->szInputMode[2] = '1';//'2' make it same to verfione
			pstSystem->cHasPinInput = 0;
		}
		else
		{
			pstSystem->szInputMode[2] = '1';
			pstSystem->cHasPinInput = 1;
			SetIsKeyInPIN(1);
		}
	}
	else
	{
		if (memcmp(pstSystem->szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
		{
			pstSystem->cHasPinInput = 0;
		}
		else
		{
			pstSystem->cHasPinInput = 1;
			SetIsKeyInPIN(1);
		}
	}
}

int MagCashAdvance(STSYSTEM *pstSystem)
{
	int nRet = 0, nInputPinNum=0;
	char *pszTitle = "CASH ADVANCE";
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	
	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, pstSystem->cTransType, 0, YES));

	/**
	* Advance dial
	*/
#ifdef DEMO
#else
	CommPreDial();
#endif
	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));
	}
	ASSERT_HANGUP_QUIT(GetSecurityCode(pszTitle, pstSystem->nAcqIndex, pstSystem->szCVV2));

	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	/**
	* Get Pin
	*/
	ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0200", 4);
	memcpy(pstSystem->szProcCode, "010000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(PackGeneral(pstSystem, NULL));
	
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);

	nInputPinNum++;
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	nRet = DealPackAndComm(pszTitle,DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if(nRet != APP_SUCC)
	{	
		CommHangUp();
		return APP_FAIL;
	}

	SysToRecord(pstSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);
}

