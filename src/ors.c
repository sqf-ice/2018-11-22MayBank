/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  ors.c
** File indentifier: 
** Brief:  ORS module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2017-4-17
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static const char gszOrsCode[][64]={       
	{"00Approved"},
	{"03Invailid TID or MID"},
	{"12Invailid Transaction"},
	{"14Invailid Card Number"},
	{"20Invailid Response"},
	{"21No Transaction or Invalid Receipt"},
	{"54Card Expired"},
	{"81Exceed Max Redemption"},
	{"82Exceed Invalid Gift Code"},
	{"83Card Zero Balance"},
	{"84Insufficient Points"},
	{"94Duplicated Transmission Transaction has void"},
	{"95Reconcile error"},
	{"96System error"},
	{"99Perform Terminal Data Download after SETTLEMENT"},
	{"--NO RESPONSE CODE"},
};


static int ParseLoyaltyInfo(STORS_INFO *pstOrsInfo)
{
	int nLen = 0;
	char sField[512] = {0};
	int nPos = 0;

	nLen = sizeof(STORS_INFO);
	ASSERT_QUIT(GetField(63, sField, &nLen));

	memcpy((char *)&pstOrsInfo, sField, nLen);
	
	return APP_SUCC;
}

/**
* @brief 
* @param in pszTitle 要显示的标题
* @param in pstSystem 系统数剧结构指针
* @return 返回
* @li APP_SUCC成功
* @li APP_QUIT取消
* @author 
* @date 
*/
int GetTransPoint(char *pszTitle, char *pszPoint)
{
	int nLen = 0;
	
	ASSERT_FAIL(PubInputDlg(pszTitle, "Input Points: ", pszPoint, &nLen, 1, 12, 60, INPUT_MODE_NUMBER));
	return APP_SUCC;
}

/**
* @brief 
* @param in pszTitle 要显示的标题
* @param in pstSystem 系统数剧结构指针
* @return 返回
* @li APP_SUCC成功
* @li APP_QUIT取消
* @author 
* @date 
*/
int GetGiftCode(char *pszTitle, char *pszGiftCode, char *pszQuantity)
{
	int nLen = 0;
	char szTmp[2+1] = {0};
	
	ASSERT_FAIL(PubInputDlg(pszTitle, "Input Gift Code: ", pszGiftCode, &nLen, 1, 9, 60, INPUT_MODE_STRING));
	ASSERT_FAIL(PubInputDlg(pszTitle, "Input Quantity: ", szTmp, &nLen, 1, 2, 60, INPUT_MODE_NUMBER));
	sprintf(pszQuantity, "%02d", atoi(szTmp));
		
	return APP_SUCC;
}

/**
* @brief Magnetic refund
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int OrsPointsInquiry()
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nTrueInput=0;
	char *pszTitle = "POINTS INQUIRY";

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = (char)TRANS_ORS_POINTS_INQUIRY;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));
	
	ASSERT_QUIT(Emv_Simple_InputCard(pszTitle, &stSystem, &nTrueInput));
	
	/**
	* Advance dial
	*/
	CommPreDial();

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
			
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	DealSystem(&stSystem);

	memcpy(stSystem.szMsgID, "0100", 4);
	memcpy(stSystem.szProcCode, "314000", 6);
	memcpy(stSystem.szServerCode, "00", 2);

	/**
	* Pack
	*/
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL));

	ASSERT_HANGUP_FAIL(DealPackAndComm(pszTitle,DPC_DEFAULT,&stSystem,NULL,3));

	ASSERT_HANGUP_FAIL(ParseLoyaltyInfo(&stSystem.stOrsInfo));
	
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_UNSETREVERSAL, &stSystem, &stTransRecord, NULL);
}

int OrsInstantReward()
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nTrueInput=0;
	char *pszTitle = "INSTANT REWARD";

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = (char)TRANS_ORS_POINTS_INQUIRY;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));
	
	ASSERT_QUIT(Emv_Simple_InputCard(pszTitle, &stSystem, &nTrueInput));
	
	/**
	* Advance dial
	*/
	CommPreDial();

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
			
	/**
	* Input Amount
	*/
	ASSERT_HANGUP_QUIT(EdcGetAmout(pszTitle, &stSystem));
	
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	DealSystem(&stSystem);

	memcpy(stSystem.szMsgID, "0200", 4);
	memcpy(stSystem.szProcCode, "014000", 6);
	//memcpy(stSystem.szServerCode, "00", 2);

	/**
	* Pack
	*/
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL));

	ASSERT_HANGUP_FAIL(DealPackAndComm(pszTitle,DPC_DEFAULT,&stSystem,NULL,3));

	ASSERT_HANGUP_FAIL(ParseLoyaltyInfo(&stSystem.stOrsInfo));
	
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);

	return TradeComplete(pszTitle, TDC_UNSETREVERSAL, &stSystem, &stTransRecord, NULL);
}

int OrsRedeem(char cTransType)
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nTrueInput=0;
	char szTitle[32] = {0};
	char sDE63[512] = {0};

	switch (cTransType)
	{
	case TRANS_ORS_POINTS_RDMPT:
		strcpy(szTitle, "POINTS RDMPT");
		break;
	case TRANS_ORS_VALUE_RDMPT:
		strcpy(szTitle, "VALUE RDMPT");
		break;
	case TRANS_ORS_GIFTCODE_RDMPT:
		strcpy(szTitle, "GIFT CODE_RDMPT");
		break;
	case TRANS_ORS_HOTDEAL_RDMPT:
		strcpy(szTitle, "HOT DEAL_RDMPT");
		break;
	default:
		return APP_QUIT;
	}
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = (char)cTransType;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(szTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));
	
	ASSERT_QUIT(Emv_Simple_InputCard(szTitle, &stSystem, &nTrueInput));
	
	/**
	* Advance dial
	*/
	CommPreDial();

	PubDisplayTitle(szTitle);
	ASSERT_HANGUP_QUIT(DispPan(stSystem.szPan, DISPPAN_TIMEOUT));
			
	/**
	* Input Amount
	*/
	switch (stSystem.cTransType)
	{
	case TRANS_ORS_POINTS_RDMPT:
		stSystem.cRdmptType = '0';
		ASSERT_HANGUP_QUIT(GetTransPoint(szTitle, stSystem.szAmount));
		break;
	case TRANS_ORS_VALUE_RDMPT:
		stSystem.cRdmptType = '1';
		ASSERT_HANGUP_QUIT(EdcGetAmout(szTitle, &stSystem));
		break;
	case TRANS_ORS_GIFTCODE_RDMPT:
		stSystem.cRdmptType = '2';
		memset(stSystem.szAmount, '0', 12);
		ASSERT_HANGUP_QUIT(GetGiftCode(szTitle, stSystem.szGiftCode, stSystem.szQuantity));
		break;
	case TRANS_ORS_HOTDEAL_RDMPT:
		stSystem.cRdmptType = '3';
		memset(stSystem.szAmount, '0', 12);
		ASSERT_HANGUP_QUIT(GetGiftCode(szTitle, stSystem.szGiftCode, stSystem.szQuantity));
		break;
	default:
		return APP_QUIT;
	}
	
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(szTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	DealSystem(&stSystem);

	memcpy(stSystem.szMsgID, "0200", 4);
	memcpy(stSystem.szProcCode, "004000", 6);
	//memcpy(stSystem.szServerCode, "00", 2);

	/**
	* Pack
	*/
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL));
	//add pack 63
	sDE63[0] = stSystem.cRdmptType;
	memcpy(sDE63 + 1, stSystem.szGiftCode, 9);
	memcpy(sDE63 + 10, stSystem.szQuantity, 2);
	ASSERT_HANGUP_FAIL(SetField(63, sDE63, 12));
	
	ASSERT_HANGUP_FAIL(DealPackAndComm(szTitle,DPC_DEFAULT,&stSystem,NULL,3));

	ASSERT_HANGUP_FAIL(ParseLoyaltyInfo(&stSystem.stOrsInfo));
	memcpy(stSystem.stOrsInfo.szGiftCode, stSystem.szGiftCode, 9);
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);

	return TradeComplete(szTitle, TDC_UNSETREVERSAL, &stSystem, &stTransRecord, NULL);
}


/**
* @brief Void Sale
* @param void
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int VoidPoint()
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
		if ((stTransRecord.cTransType == TRANS_SALE) && (stTransRecord.cStatus != 0))
		{
			if (2 == stTransRecord.cStatus || 4 == stTransRecord.cStatus)
			{
				PubMsgDlg(szTitle, "Adjusted, Not Allow Void", 0, 5);
			}
			else
			{
				PubMsgDlg(szTitle, "Voided, Not Allow Void", 0, 5);
			}

			return APP_FAIL;
		}
		if (stTransRecord.cTransType != TRANS_ORS_INSTANT_REWARD 
			&& stTransRecord.cTransType != TRANS_ORS_POINTS_RDMPT
			&& stTransRecord.cTransType != TRANS_ORS_VALUE_RDMPT 
			&& stTransRecord.cTransType != TRANS_ORS_GIFTCODE_RDMPT 
			&& stTransRecord.cTransType != TRANS_ORS_HOTDEAL_RDMPT)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CNot Allow Void");
			PubMsgDlg(szTitle, szContent, 0, 5);
			return APP_FAIL;
		}
		ASSERT_FAIL(DispVoidRecord(szTitle, &stTransRecord));
	}


	switch (stTransRecord.cTransType)
	{
	case TRANS_ORS_INSTANT_REWARD:
		strcpy(szTitle, "VOID INSTANT");
		stSystem.cTransType	= TRANS_ORS_INSTANT_REWARD;
		break;
	case TRANS_ORS_POINTS_RDMPT:
		strcpy(szTitle, "VOID POINTS RDMPT");
		stSystem.cTransType	= TRANS_ORS_POINTS_RDMPT;
		break;
	case TRANS_ORS_VALUE_RDMPT:
		strcpy(szTitle, "VOID VALUE RDMPT");
		stSystem.cTransType	= TRANS_ORS_VALUE_RDMPT;
		break;	
	case TRANS_ORS_GIFTCODE_RDMPT:
		strcpy(szTitle, "VOID GIFTCODE RDMPT");
		stSystem.cTransType	= TRANS_ORS_GIFTCODE_RDMPT;
		break;	
	case TRANS_ORS_HOTDEAL_RDMPT:
		strcpy(szTitle, "VOID HOTDEAL RDMPT");
		stSystem.cTransType	= TRANS_ORS_HOTDEAL_RDMPT;
		break;	
	default:
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
	else
	{
		PubHexToAsc((uchar *)stTransRecord.sPan,20,1,(uchar *)szTmpStr);
		memcpy(stSystem.szPan, szTmpStr, stTransRecord.nPanLen);
		PubHexToAsc((uchar *)stTransRecord.sTrack2, stTransRecord.nTrack2Len, 0, (uchar *)stSystem.szTrack2);
PubDebug("stSystem.szTrack2: %s", stSystem.szTrack2);
	}
	/**
	* Set Amt
	*/
	PubHexToAsc((uchar *)stTransRecord.sAmount, 12, 0, (uchar *)stSystem.szAmount);
	stSystem.cFlagCashback = stTransRecord.cFlagCashback;
	stSystem.cTipFlag = stTransRecord.cTipFlag;
	
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
			ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, stSystem.szAmount, stSystem.szPin));
			if (stTransRecord.nAcqIndex != 1)
			{
				if (memcmp(stSystem.szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
				{
					stSystem.szInputMode[2] = '2';
				}
				else
				{
					stSystem.szInputMode[2] = '1';
				}
			}
	}
	else
	{
		stSystem.szInputMode[2] = '2';
	}

	/**
	* Get necessary data
	*/
	stSystem.nAcqIndex = stTransRecord.nAcqIndex;//get nii
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, "0200", 4); 
	memcpy(stSystem.szProcCode, "214000", 6);	
	memcpy(stSystem.szServerCode, "00", 2);
	
	memcpy(stSystem.szOldAuthCode, stTransRecord.szAuthCode, 6);	  
	PubHexToAsc((unsigned char *)stTransRecord.sTrace, 6, 0, (unsigned char *)stSystem.stField62.sOldTrace);
	PubHexToAsc((unsigned char *)stTransRecord.sDate,  4, 0, (unsigned char *)stSystem.stField62.sOldDate);
	PubHexToAsc((unsigned char *)stTransRecord.sTime,  6, 0, (unsigned char *)stSystem.stField62.sOldTime);
	memcpy(stSystem.szRefnum, stTransRecord.szRefnum, 12);
	PubHexToAsc((uchar *)stTransRecord.sExpDate, 4, 0, (uchar *)stSystem.szExpDate);				/**<14 卡有效期(同原请求交易)*/
	memcpy(stSystem.szInputMode, stTransRecord.szInputMode, 3);
	memcpy(stSystem.szCardSerialNo, stTransRecord.szCardSerialNo, 3);
	stSystem.nAddFieldLen = stTransRecord.nField55Len;
	memcpy(stSystem.sField55, stTransRecord.sField55, stTransRecord.nField55Len);
	stSystem.nAcqType = stTransRecord.nAcqType;
	
	memcpy(stSystem.szInvoice, stTransRecord.szInvoice, 6);
	stSystem.cTransAttr = stTransRecord.cTransAttr;
	//for EPP
	stSystem.nInstallmentMonth = stTransRecord.nInstallmentMonth;

	/**************VOID dont need to send DE55*******************/
	stSystem.nAddFieldLen = 0;
	
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(szTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());

	
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
	
	/**
	* Add new transaction
	*/
	memset(&stTransRecord,0,sizeof(STTAANSRECORD));
	SysToRecord(&stSystem, &stTransRecord);

	return TradeComplete(szTitle, TDC_DEFAULT, &stSystem, &stTransRecord, szInvno);
}

static int ParseDownloadData()
{
	char szTmp[512] = {0};

	int nLen = 0;
	char sField[512] = {0};
	int nPos = 0;
	char szMaxPoint[12+1] = {0};

	nLen = sizeof(STORS_INFO);
	ASSERT_QUIT(GetField(63, sField, &nLen));

	SetVarHeaderText(sField+nPos, 0);
	nPos+=24;
	SetVarHeaderText(sField+nPos, 1);
	nPos+=24;
	SetVarHeaderText(sField+nPos, 2);
	nPos+=24;
	SetVarFooterText(sField+nPos, 0);
	nPos+=24;
	SetVarFooterText(sField+nPos, 1);
	nPos+=24;
	SetVarFooterText(sField+nPos, 2);
	nPos+=24;
	
	nPos++;//terminal type
	nPos+=12;//date and time

	PubHexToAsc(sField+nPos, 12, 0, szMaxPoint);
	nPos+= 6;
	SetMaxRdmPoint(szMaxPoint);

	SetTransSwitchOnoff(TRANS_ORS_INSTANT_REWARD, sField[nPos++]);
	SetTransSwitchOnoff(TRANS_ORS_POINTS_RDMPT, sField[nPos++]);
	SetTransSwitchOnoff(TRANS_ORS_VALUE_RDMPT, sField[nPos++]);
	SetTransSwitchOnoff(TRANS_ORS_GIFTCODE_RDMPT, sField[nPos++]);
	SetTransSwitchOnoff(TRANS_ORS_HOTDEAL_RDMPT, sField[nPos++]);

	//void
	SetTransSwitchOnoff(TRANS_ORS_VOID_INSTANT_REWARD, sField[nPos]);
	SetTransSwitchOnoff(TRANS_ORS_VOID_POINTS_RDMPT, sField[nPos]);
	SetTransSwitchOnoff(TRANS_ORS_VOID_VALUE_RDMPT, sField[nPos]);
	SetTransSwitchOnoff(TRANS_ORS_VOID_GIFTCODE_RDMPT, sField[nPos]);
	SetTransSwitchOnoff(TRANS_ORS_VOID_HOTDEAL_RDMPT, sField[nPos]);

	nPos ++;

	//point inquiry
	SetTransSwitchOnoff(TRANS_ORS_POINTS_INQUIRY, sField[nPos++]);
	
	//point inquiry
	SetVarIsEncrypted(sField[nPos]);

	return APP_SUCC;
}

int OrsTerminalDownload()
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	int nTrueInput=0;
	char *pszTitle = "DATA DOWNLOAD";

	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	stSystem.cTransType = (char)TRANS_ORS_DATADOWNLOAD;
	stSystem.cTransAttr = (char)ATTR_MAG;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_CHECKOPER, stSystem.cTransType, 0, YES));
	
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	DealSystem(&stSystem);

	memcpy(stSystem.szMsgID, "0600", 4);
	memcpy(stSystem.szProcCode, "930000", 6);
	memcpy(stSystem.szServerCode, "00", 2);

	/**
	* Pack
	*/
	ASSERT_HANGUP_FAIL(PackGeneral(&stSystem, NULL));

	ASSERT_HANGUP_FAIL(DealPackAndComm(pszTitle,DPC_DEFAULT,&stSystem,NULL,3));
	
	ASSERT_HANGUP_FAIL(ParseDownloadData());
	PubMsgDlg(pszTitle, "Download OK", 1, 1);
	return APP_SUCC;
}

YESORNO CheckTransIsOrs(char cTransType)
{
	if (cTransType >= TRANS_ORS && cTransType <= TRANS_ORS_DATADOWNLOAD)
	{
		return YES;
	}

	return NO;
}

/**
* @brief Settlement
* @param in cFlag 0:Normal settle  nonzero:settle again after interupting settle 
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int RedemptionSettle()
{	
	char *pszTitle = "SETTLE";
	STSYSTEM stSystem;
	STAMT_NUM_SETTLE stAmtNumSettle;
	char cTailFlag = 0;
	int nRet = 0;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen = 0;
	char szSettleDateTime[5+1];
	char cProcStep = 0;
	int nAcqIndex = 0;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	
	/*
	* Deal POS limit
	*/
	ASSERT_QUIT(DealPrintLimit()); 

	/**
	* Settle online
	*/
	if (cProcStep <= 1)
	{
		/**
		* Check if it has login
		*/
		ASSERT_QUIT(ChkLoginStatus());

		/**
		* Execute Reveral
		*/
		ASSERT_HANGUP_QUIT(DoAdviseAndReveral());

		/**
		* Send offline
		*/
		ASSERT_HANGUP_QUIT(SendOffline(1));

SETTLE_TAIL:
		PubDisplayTitle(pszTitle);		
		/**
		* Check Connect again
		*/
		ASSERT_HANGUP_QUIT(CommConnect());
		
		/**
		* Get necessary data
		*/
		DealSystem(&stSystem);
		memset(&stAmtNumSettle, '0', sizeof(STAMT_NUM_SETTLE));
		DealSettle(&stAmtNumSettle, stSystem.nAcqIndex);
		memset((&stAmtNumSettle+60), '0', 30);
		
		memcpy(stSystem.szMsgID, "0500", 4);
		if (1 == cTailFlag)
		{
			memcpy(stSystem.szProcCode, "960000", 6);
		}
		else
		{
			memcpy(stSystem.szProcCode, "920000", 6);
		}
		
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
		ASSERT_HANGUP_FAIL(SetField(60, stSystem.szBatchNum, 6));		
		ASSERT_HANGUP_FAIL(SetField(62, stSystem.szInvoice, 6));		
		ASSERT_HANGUP_FAIL(SetField(63, (char *)&stAmtNumSettle, 90));

		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));

		IncVarTraceNo();
		ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));
		ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));
		CommHangUpSocket();
		
		ASSERT_HANGUP_FAIL(Unpack(sPackBuf, nPackLen));
		ASSERT_HANGUP_FAIL(ChkRespMsgID("0500", sPackBuf));
		ASSERT_HANGUP_FAIL(ChkRespon(&stSystem, sPackBuf + 2));

		/*Check response*/
		ASSERT_HANGUP_FAIL(CheckSettleReponse(stSystem));
		
		/**
		* Save settle date time
		*/
		PubAscToHex((uchar *)stSystem.szDate, 4, 0, (uchar *)szSettleDateTime);
		PubAscToHex((uchar *)stSystem.szTime, 6, 0, (uchar *)szSettleDateTime + 2);
		SetVarSettleDateTime(szSettleDateTime);
	}

	if (NO == GetVarIsTotalMatch())//total unmatch need to send all the batch
	{
		cTailFlag = 1;
	}
	else
	{
		cTailFlag = 0;
		SetVarBatchHaltFlag(NO);
	}

	/**
	* Send batch
	*/
	if (cProcStep <= 2 && cTailFlag == 1)
	{
		SetVarBatchHaltFlag(YES);
		ASSERT_HANGUP_FAIL(DoBatchUp());
		SetVarBatchHaltFlag(NO);
		goto SETTLE_TAIL;
	}

	CommHangUp();

	/**
	* Print settle
	*/
	if (cProcStep <= 3)
	{
		SetVarPrintSettleHalt(YES);
		PubClearAll();
		PubDisplayTitle("PRINT SETTLE");
		PubDisplayGen(3, "PLS WAIT");
		PubUpdateWindow();
		nRet = PrintSettle(FIRSTPRINT, stSystem.nAcqIndex);
		if (nRet == APP_FAIL)
		{
			return APP_FAIL;
		}
 		SetVarPrintSettleHalt(NO);
 	}
	

	/*
	* Clear settle data and water
	*/
	if (cProcStep <= 5)
	{
		SetVarClrSettleDataFlag(YES);
		PubClearAll();
		PubDisplayTitle(pszTitle);
		PubDisplayGen(2, "PROCESSING...");
		PubUpdateWindow();
		//nRet = InitAllBatch();
		//nRet += EmvClearWater();
		ClearWaterFileByAcqIndex(stSystem.nAcqIndex);
		ClearSettle(stSystem.nAcqIndex);

		IncVarBatchNo(stSystem.nAcqIndex);
		SetVarOfflineUnSendNum(0);
		SetVarClrSettleDataFlag(NO);
	}
	PubClearAll();
	PubDisplayTitle(pszTitle);
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 2, "SETTLE SUCC");
	PubUpdateWindow();
	return APP_SUCC;
}

int SaveOrsSettle(char cTransType, STORS_INFO *pstOrsInfo)
{
	STORS_AMT_NUM_SETTLE stOrsSettle;
	char szNum[6+1] = {0};
	char szTmp[12+1] = {0};

	memset(&stOrsSettle, 0, sizeof(STORS_AMT_NUM_SETTLE));
	GetFromFile((char *)&stOrsSettle, 0, sizeof(STORS_AMT_NUM_SETTLE), ORS_SETTLE_FILE);
	
	switch (cTransType)
	{
	case TRANS_ORS_INSTANT_REWARD:
		IncreaseBcdNum(stOrsSettle.sInstantRewardNum);
		MathBcd(stOrsSettle.sInstantRewardPoints, pstOrsInfo->sTtlPointRedeem, 6, EM_PLUS);
		MathBcd(stOrsSettle.sInstantRewardAmt, pstOrsInfo->sPayAmt, 6, EM_PLUS);
		break;
	case TRANS_ORS_POINTS_RDMPT:
		IncreaseBcdNum(stOrsSettle.sPointRdmptNum);
		MathBcd(stOrsSettle.sPointRdmptPoints, pstOrsInfo->sTtlPointRedeem, 6, EM_PLUS);
		break;
	case TRANS_ORS_VALUE_RDMPT:
		IncreaseBcdNum(stOrsSettle.sValueRdmptNum);
		MathBcd(stOrsSettle.sValueRdmptPoints, pstOrsInfo->sTtlPointRedeem, 6, EM_PLUS);
		break;
	case TRANS_ORS_GIFTCODE_RDMPT:
		IncreaseBcdNum(stOrsSettle.sGiftCodeRdmptNum);
		MathBcd(stOrsSettle.sGiftCodeRdmptPoints, pstOrsInfo->sTtlPointRedeem, 6, EM_PLUS);
		break;
	case TRANS_ORS_HOTDEAL_RDMPT:
		IncreaseBcdNum(stOrsSettle.sHotDealRdmptNum);
		MathBcd(stOrsSettle.sHotDealRdmptPoints, pstOrsInfo->sTtlPointRedeem, 6, EM_PLUS);
		MathBcd(stOrsSettle.sHotDealRdmptAmt, pstOrsInfo->sPayAmt, 6, EM_PLUS);
		break;
	}
}

