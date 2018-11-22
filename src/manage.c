/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  manage.c
** File indentifier: Manage precessing module
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-19
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"


static int DoClear(void);
static int PosPrintSet(void);
static int TransParam(void);
static int MerchantManage(void);
static int ResendManage(void);
static int TxnOffline(void);
static int SetAmtLimit(void);

/**
* @brief Clear revesal flag
* @param in void
* @return void
 */
void ClearRevFlag(void)
{
	STREVERSAL stReversal;
	
	SetVarIsReversal(NO);
	SetVarHaveReversalNum(0);
	
	memset(&stReversal, 0, sizeof(STREVERSAL));
	SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
	PubMsgDlg("CLEAR REVERSAL", "SUCC", 0, 1);
	return;
}

/**
* @brief 		Clear flag
* @param in 	void
* @return 	void
*/
void ClearFlag()
{
	int nRet;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.CLEAR REVERSAL"),
		tr("2.CLEAR LOCK"),
		tr("3.SET NOT SIGNED"),
		tr("4.SET SIGNED"),
		tr("5.CLEAR EMV DATA")
	};
	while(1)
	{
		nRet = PubShowMenuItems("DO CLEAR", pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1: 
			nRet = PubConfirmDlg("CLEAR REVERSAL", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				if (APP_SUCC == ProCheckPwd("CLEAR REVERSAL", EM_SYSTEM_PWD))
				{
					ClearRevFlag(); 
				}
			}
			break;
		case 2: 
			nRet = PubConfirmDlg("CLEAR LOCK", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				SetVarIsLock(NO);
			}
			break;
		case 3: 
			nRet = PubConfirmDlg("SET NOT SIGNED", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				SetVarIsLogin(NO);
			}
			break;
		case 4:
			nRet = PubConfirmDlg("SET SIGNED", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				SetVarIsLogin(YES);
			}
			break;
		case 5:
			if (APP_SUCC == PubConfirmDlg("CLEAR EMV DATA","CONFIRM?", 0, 10))
			{
				PubMsgDlg("CLEAR EMV DATA","PROCESSING", 0, 1);
				PubDelFile(KERNEL_NAME);
				PubDelFile(EMVFILE_TERMCAPK);	
				InitEMVParam();
				PubMsgDlg("CLEAR EMV DATA","COMPLETION", 0, 1);
			}
			break;
		default:                 
			break;
		}
	}
}

/**
* @brief  Do Cleat
* @param in void
* @return     void
*/
int DoClear(void)
{
	int nRet;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.CLEAR REVERSAL"),
		tr("2.CLEAR BATCH"),
		tr("3.INIT EMV")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("CLEAR"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			nRet = PubConfirmDlg("CLEAR REVERSAL", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				ClearRevFlag(); 
				PubMsgDlg("CLEAR REVERSAL", "OK", 1, 1);
			}
			break;
		case 2:
			nRet = PubConfirmDlg("CLEAR BATCH", "CONFIRM?", 0, 0);
			if( nRet == APP_SUCC )
			{
				PubMsgDlg("CLEAR BATCH","PROCESSING...", 0, 1);
				InitAllBatch();
				ClearAllSettle();
				EmvClearWater();
				SetVarOfflineUnSendNum(0);
				ExportSettleData();
				PubMsgDlg("CLEAR BATCH", "OK", 1, 1);
			}
			break;
		case 3:
			if (APP_SUCC == PubConfirmDlg("CLEAR EMV DATA","CONFIRM?", 0, 10))
			{
				PubMsgDlg("CLEAR EMV DATA","PROCESSING...", 0, 1);
				PubDelFile(KERNEL_NAME);
				PubDelFile(EMVFILE_TERMCAPK);	
				InitEMVParam();
				DownEmvParam();
				ASSERT_QUIT(ExportEMVModule());
				PubMsgDlg("CLEAR EMV DATA","OK", 1, 1);
			}
			break;
		default :
			break;
		}
	}
	return APP_SUCC;
}

int ChkPinpad(void)
{
	int nRet = 0;
	int nPinpadAux = PORT_NUM_COM2;
	int nPinpadTimeout = 120;
	int nMode = SECRITY_MODE_INSIDE;
	STPINPADPARAM stPinpadParam = 
	{	
		PORT_NUM_COM2,
		60,
		0x00,
		0x01,
		0x01,
		0x04,
		"                ",
		"",
		PINPAD_HY,
		1,
		0,
		0,
		1,
		0x00,
		""
	};


	if((GetVarIsPinpad() == NO) && (GetControlChkInside() == YES))
	{
		SetControlChkPinpad(YES);
		nMode = SECRITY_MODE_INSIDE;
		stPinpadParam.cTimeout = DEFAULT_PIN_INPUT_TIME;
		
		//stPinpadParam.cIsUseNewKey= 0x01;
		nRet = PubInitSecrity(nMode, &stPinpadParam);
		if (APP_SUCC != nRet)
		{
			PubMsgDlg("DETECT DEV", "LOAD SECRITY FAIL", 3, 10);
			return APP_FAIL;
		}
		SetControlChkInside(NO);
	}
	else if ((GetVarIsPinpad() == YES) && (GetControlChkPinpad() == YES))
	{
		SetControlChkInside(YES);
		PubClearAll();
		PubDisplayStrInline(0, 2, "LOAD PINPAD");
		PubDisplayStrInline(0, 4, "PLS WAIT...");
		PubUpdateWindow();

		nMode = SECRITY_MODE_PINPAD;
		
		nRet = GetVarUID(stPinpadParam.szUid);
		if (nRet != APP_SUCC)
		{
			PubMsgDlg("DETECT DEV", "PLS SET USR ID", 3, 10);
			return APP_FAIL;
		}
		
		GetVarPinpadAuxNo(&nPinpadAux);
		stPinpadParam.cAux = nPinpadAux;
		GetVarPinpadTimeOut(&nPinpadTimeout);
		stPinpadParam.cTimeout = nPinpadTimeout;
		stPinpadParam.cAppDirID = 1;

		nRet = PubInitSecrity(nMode, &stPinpadParam);
		if (nRet != APP_SUCC)
		{
			PubMsgDlg("DETECT DEV", "PINPAD NOT CONNECTTED", 3, 10);
			
			return APP_FAIL;
		}
		SetControlChkPinpad(NO);
	}
	
	return APP_SUCC;
}

int ChkRF(void)
{
	STRFIDCARDPARAM stRFIDCardParam;
	memset(&stRFIDCardParam, 0, sizeof(stRFIDCardParam));
	if (YES != GetVarIsSupportRF() || YES != GetControlChkRF())
		return APP_SUCC;
	stRFIDCardParam.cCardType = EM_CARD_RF_CPU;
	if(GetVarIsExRF() == YES)
	{
		stRFIDCardParam.cOperFlag = EM_RF_OUTSIDE;
		if(GetVarIsUsePP60RF() == YES)
		{
			stRFIDCardParam.cAux = PORT_NUM_COM2;
		}
		else
			stRFIDCardParam.cAux = PORT_NUM_COM1;
		stRFIDCardParam.cDefaultType = RFIDREADER_EXTERNAL;
		stRFIDCardParam.cUsedDoubleRF = 0;
	}
	else
	{
		stRFIDCardParam.cOperFlag = EM_RF_INSIDE;
	}

	PubClearAll();
	PubDisplayStrInline(0, 2, "Load card reader");
	PubDisplayStrInline(0, 4, "Please wait...");
	PubUpdateWindow();

	SetControlCommInit();
	if(APP_SUCC != PubInitRFCardParam(&stRFIDCardParam))
	{
		PubMsgDlg(NULL, "Init rf fail", 3, 10);
		SetControlChkRF(YES);
		return APP_FAIL;
	}
	SetControlChkRF(NO);
	return APP_SUCC;
}

/**
* @brief Key manage
* @param in void
* @return     void
*/
int KeyManage(void)
{
	int	nRet;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.PIN UPDATE"),
		tr("2.DUKPY")
	};
	
	ASSERT_QUIT(ProCheckPwd("KEY MANAGE", EM_SYSTEM_PWD));
	
	while(1)
	{
	    nRet = PubShowMenuItems(tr("KEY MANAGE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch (nSelcItem)
		{
		case 1:
			PinUpdate();
			break;
		case 2:
			DukptDownload();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Set Tmk by hand
*/
int SetTmkByHand(void)
{
	int nKeyIndex;
	int nRet;
	int nLen = 0;
	char szKeyIndex[2+1];
	char szTmpStr[21] = {0};
	char szAscKey[32+1], sBcdKey[16];
	char *pszTitle = "INPUT BY HAND";
	YESORNO cIsPinPad = NO;

	cIsPinPad = GetVarIsPinpad();
	if (YES == cIsPinPad && YES == GetControlChkPinpad())
	{
		if (APP_SUCC != ChkPinpad())
		{
			return APP_FAIL;
		}
		SetControlChkPinpad(NO);
	}
	memset(szKeyIndex, 0, sizeof(szKeyIndex));
	ASSERT_QUIT(PubInputDlg("INPUT KEY NO.:", "(0 ~ 9)", szKeyIndex, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
	nKeyIndex = atoi(szKeyIndex);

	memset(szAscKey, 0, sizeof(szAscKey));	
	//strcpy(szAscKey,"B7E9F9BF64B929783AA562FFD705EA9C");
	ASSERT_QUIT(PubInputDlg(pszTitle, "INPUT TMK(32):", szAscKey, &nLen, 32, 32, 0, INPUT_MODE_STRING));	
	PubAscToHex((uchar *)szAscKey, nLen, 0, (uchar *)sBcdKey);
	nLen >>= 1;
	
	//ZMK decrypt TMK
	nRet = PubLoadMainKey(nKeyIndex, sBcdKey, NULL, nLen);
	if (nRet != APP_SUCC)
	{
		PubDispErr("LOAD MAINKEY FAIL");
		return APP_FAIL;
	}

	sprintf(szTmpStr, "LOAD(%d) SUC)", nKeyIndex);	
	PubMsgDlg(pszTitle, "SET TMK OK", 1, 1);
	
	return APP_SUCC;
}


/**
* @brief 	set pinkey for offline pin in Pinpad which used to encrypt pinpad and decrypt pos
* @param in psKeyBuf if = NULL £¬key by hand
* @param in nKeyIndex 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetPinpadKey(char *psKeyBuf, int nKeyIndex)
{
	int nRet = APP_SUCC;
	int nPort = PORT_NUM_COM2;
	int nLen;
	char szAscMainKey[32+1], szBcdMainKey[16+1];
	char szMainKey[16+1];
	char *pszTitle = "KEY MANAGE";

	if (YES == GetVarIsPinpad())
	{
		GetVarPinpadAuxNo(&nPort);
	}
	else
	{
		return APP_SUCC;
	}
	nKeyIndex = nKeyIndex >= 0x30 ? nKeyIndex-0x30: nKeyIndex;
	if (NULL == psKeyBuf)
	{
		nLen = 16;
		memset(szAscMainKey, 0, sizeof(szAscMainKey));	
		ASSERT_QUIT(PubInputDlg(pszTitle, "Input PINKEY of offline pin", szAscMainKey, &nLen, nLen, nLen, 0, INPUT_MODE_STRING));
		PubMsgDlg(NULL, "Writing Key", 0, 1);
		PubAscToHex((uchar *)szAscMainKey, 16, 0, (uchar *)szBcdMainKey);
	}
	else
	{
		memcpy(szBcdMainKey, psKeyBuf, 8);
	}
	PubSoftDes(szBcdMainKey, szBcdMainKey, szMainKey);
	nRet = PubLoadMainKey(nKeyIndex, szBcdMainKey, NULL, 8);
	PubSetCurrentMainKeyIndex(nKeyIndex);
	nRet += PubLoadWorkKey(KEY_TYPE_PIN, szMainKey, 8, NULL);
	if (nRet != APP_SUCC)
	{
		PubMsgDlg(pszTitle, "install offline pinkey failed", 3, 10);
	}
	return nRet;
}

static int PosPrintSet()
{
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems = {
		tr("1.RECEIPT HEAD"),
		tr("2.PAGE"),
		tr("3.OTHER")
	};
		
	while(1)
	{
		nRet = PubShowMenuItems(tr("SET PRINT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionPrintTitleMode();
			break;
		case 2:
			SetFunctionPrintPageCount();
			break;
		case 3:
			{
				int (*lSetFuns[])(void) = {					
					//SetFunctionIsPrintWaterRec,				   	
				   	SetFunctionReprintSettle,   	
				   	SetFunctionIsPrintPrintMinus,
				   NULL};
				PubUpDownMenus(lSetFuns);	
			}
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}


static int TransParam(void)
{
	int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.TRACE NO"),
		tr("2.BATCH NO"),
		tr("3.INVOICE NO"),
		tr("4.AMOUNT LIMIT"),
		tr("5.TRANS COUNT"),
		tr("6.SET PRINT")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("TRANS PARAM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		switch(nSelcItem)
		{
		case 1:
			SetFunctionTraceNo();
			break;
		case 2:
			SetFunctionBatchNo();
			break;
		case 3:
			SetFunctionInvoiceNo();
			break;
		case 4:
			SetAmtLimit();
			break;
		case 5:
			SetFunctionMaxTransCount();
			break;
		case 6:
			PosPrintSet();
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}

static int MerchantManage(void)
{
	int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.MERCHANT ID"),
		tr("2.TERMINAL ID"),
		tr("3.MERCHANT NAME"),
		tr("4.MERCHANT ADDR"),
		tr("5.VERDOR ID"),
		tr("6.ACQ ID"),
		tr("7.CENTRE NO"),
		tr("8.INQUIRY NO")
	};
	ASSERT_QUIT(ProCheckPwd("SET TERMINAL", EM_SYSTEM_PWD));
	
	while(1)
	{
		nRet = PubShowMenuItems(tr("SET TERMINAL"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionMerchantId();
			break;
		case 2:
			SetFunctionTerminalId();
			break;
		case 3:
			SetFunctionMerchantName();
			break;
		case 4:
			SetFunctionMerchantAddr();
			break;
		case 5:
			SetFunctionVendorId();
			break;
		case 6:
			SetFunctionAcqId();
			break;
		case 7:
			SetFunctionCenterNo();
			break;
		case 8:
			SetFunctionInquiryNo();
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}

static int ResendManage(void)
{
	int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.OFFLINE RESEND"),
		tr("2.REVERSAL RESEND")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("SET RESEND"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionOffResendNum(); 
			break;
		case 2:
			SetFuncCommReSendNum();
			break;
		default:                         
			break;
		}
	}

	return APP_SUCC;
}

static int TxnOffline(void)
{
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.OFF SEND NUM"),
		tr("2.OFF SEND TYPE")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("OFFLINE CONTROL"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		switch(nSelcItem)
		{
		case 1:
			SetFunctionMaxOffSendNum();
			break;
		case 2:
			SetFunctionIsOfflineSendNow();
			break;		
		default:                         
			break;
		}
	}
	return APP_SUCC;

}

int DoPrint()
{
	int nAcqIndex = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.LAST RECEIPT"),
		tr("2.ANY RECEIPT"),
		tr("3.LAST SETTLE")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("REPRINT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		
		switch (nSelcItem)
		{
		case 1:
			PrnLastTrans();
			break;
		case 2:
			RePrnSpecTrans();
			break;
		case 3:
			ASSERT_QUIT(SelectAcqIndex("LAST SETTLE", &nAcqIndex, NO));
			PrintSettle(REPRINT, nAcqIndex);
			break;
		default:
			break;
		}		
	}
	return APP_SUCC;
}

int DoReport()
{
	int nSelect=0xff;
	int nAcqIndex = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.SUMMARRY REPORT"),
		tr("2.DETAIL REPORT"),
		tr("3.SHOW TOTAL")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("REPORT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		
		switch (nSelcItem)
		{
		case 1:	
			ASSERT_QUIT(SelectAcqIndex("SUMMARRY REPORT", &nAcqIndex, NO));
			PrintTotal(nAcqIndex);
			break;
		case 2:
			ASSERT_QUIT(SelectAcqIndex("DETAIL REPORT", &nAcqIndex, NO));
			PrintAllWater(nAcqIndex);
			break;
		case 3:
			ASSERT_QUIT(SelectAcqIndex("SHOW TOTAL", &nAcqIndex, NO));
			DispTotal(nAcqIndex);
		default:
			break;
		}
	}
	return APP_SUCC;
}


int DoLookUp()
{
	int nSelect=0xff;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.LAST TRANS"),
		tr("2.APPOINTED TRANS"),
		tr("3.VIEW SUMMURY")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("LOOK UP"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch (nSelcItem)
		{
		case 1:
			ViewTranList();
			break;
		case 2:
			ViewSpecList();
			break;
		case 3:
			ViewTotal();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

int SetPaymentType()
{
	int nRet = 0;
	int nSelect = 0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.SWIPE ENABLE"),
		tr("2.MANUAL ENABLE"),
		tr("3.WAVE ENABLE")
	};
	while (1)
	{
		nRet = PubShowMenuItems(tr("PAYMENT TYPE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionIsSwipe();
			break;
		case 2:
			SetFunctionIsManul();
			break;
		case 3:
			SetFunctionIsSupportRF();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int OtherControl(void)
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.PRINT ISO"),
		tr("2.CASH REGISTER"),
		tr("3.PWD ENABLE")
	};
	while (1)
	{
		nRet = PubShowMenuItems(tr("OTHER CONTROL"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionIsPrintIso();
			break;
		case 2:
			SetFunctionIsCashRegister();
			break;
		case 3:
			SetFunctionIsVoidPwd();
			SetFunctionIsSettlePwd();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Set parameters of transaction
* @param in void
* @return     void
*/
int ControlOnOf(void)
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.TRANS ENABLE"),
		tr("2.PAYMENT TYPE"),
		tr("3.TIP ENABLE"),
		tr("4.MYDEBIT OPT"),
		tr("5.NO PIN AMT"),
		tr("6.EPP MONTH"),
		tr("7.PIN BY PASS"),
		tr("8.CVV2"),
		tr("9.OTHER CONTROL")
	};
	ASSERT_QUIT(ProCheckPwd("ON OFF", EM_SYSTEM_PWD));
	
	while (1)
	{
		nRet = PubShowMenuItems(tr("ON OFF"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionTransSwitch();
			break;
		case 2:
			SetPaymentType();
			break;
		case 3:
			SetFunctionIsTipFlag();
			break;
		case 4:
			SetFunctionIsMccsPriority();
			break;
		case 5:
			SetFunctionWaveNoPinAmount();
			break;
		case 6:
			SetFunctionEppSwitch();
			break;
		case 7:
			SetFunctionIsPinByPass();
			break;
		case 8:
			SetFunctionIsInputCvv2();
			break;
		case 9:
			OtherControl();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Set parameters of transaction
* @param in void
* @return     void
*/
static int SetAmtLimit(void)
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.CASHBACK AMOUNT")
	};
	while (1)
	{
		nRet = PubShowMenuItems(tr("AMOUNT LIMIT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFunctionMinCashbackAmount();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

int AdminMenu()
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.CHANGE PASSWORD"),
		tr("2.SET TIME"),
		tr("3.INITIALIZE")
	};
		
	while (1)
	{
		nRet = PubShowMenuItems(tr("SYSTEM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			ChangePassword();
			break;
		case 2:
			SetFunctionPosDateTime();
			break;
		case 3:
			InitializeParam();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

static int DebugFlag()
{
	int nSelect=0xff;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.Close"),
		tr("2.Port"),
		tr("3.File")
	};
	while(1)
	{	
		ASSERT_QUIT(PubShowMenuItems(tr("Debug"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60));
		switch(nSelcItem)
		{
		case 1:
			PubSetDebugMode(0);
			break;
		case 2:
			PubSetDebugMode(0x01);
			break;
		case 3:
			PubSetDebugMode(0x02);
			break;
		default:
			break;
		}
	}
}
	
int OtherMenu()
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.LOOK UP"),
		tr("2.VERSION"),
		tr("3.SHOW TVR"),
		tr("4.PRINT PARAM"),
		tr("5.DEBUG")
	};
	while (1)
	{
		nRet = PubShowMenuItems(tr("OTHER"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			ViewTranList();
			break;
		case 2:
			Version();
			break;
		case 3:
			ViewTVR_TSI();
			break;
		case 4:
			PrintParam();
			break;
		case 5:
			DebugFlag();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

int SystemManage()
{
	int nSelect = 0xFF;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.MERCHANT"),
		tr("2.COMMUNICATION"),
		tr("3.ON-OF"),
		tr("4.TRANS PARAM"),
		tr("5.KEY MANAGE"),
		tr("6.CLEAR"),
		tr("7.SYSTEM"),
		tr("8.OTHER")
	};
	ASSERT_QUIT(ProCheckPwd("MANAGE", EM_FUNC_PWD));
	
 	while(1)
	{	
		nRet = PubShowMenuItems(tr("MANAGE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT)
		{
			if (APP_SUCC == PubConfirmDlg("MANAGE", "QUIT?", 0, 30))
				return APP_QUIT;
			continue;
		}
		if (nRet == APP_TIMEOUT)
		{
			continue;
		}
		switch(nSelcItem)
		{		
		case 1:
			MerchantManage();
			break;
		case 2:
			SetComm();
			break;
		case 3:
			ControlOnOf();
			break;
		case 4:
			TransParam();
			break;
		case 5:
			KeyManage();
			break;
		case 6:
			DoClear();
			break;
		case 7:
			AdminMenu();
			break;
		case 8:
			OtherMenu();
			break;
		case 9:
			break;
		default:
			break;
		}
			
 	}
	return APP_SUCC;
}

