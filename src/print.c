/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  print.c
** File indentifier: 
** Brief:  Print module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-22
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static int _printwater(void *);
static int PntHex(const char *, const uchar *, const int, char);
static int  _printemvparam();
static int PrintBillHead(int nAcqIndex);
static void GetTransNameForTP(char cTransType, char *pszTransName2, char cIsCashBack, char cEmvStatus);
static int PrintAmt(const STTAANSRECORD *pstTransRecord);
static void PntTraditionOnoff(const char *lszTransFlag);
static void CheckAmtMinus(char cTransType, char *pszAmount);
static int GetPrintAmt(const STTAANSRECORD *pstTransRecord, char *pszDispAmt);
static void ProPrePrintStrFormat(char *pszStrA, char *pszStrB, char ch, int nLineFeed);
static int PrintInstallment(const char *pszInstallmentInfo);
static void GetCardTypeStr(int nCardType, char *pszCardType);
static void PrintCardNo(const STTAANSRECORD *pstTransRecord, int nCurrentPage);
static void GetCardHost(int nCardIndex, char *pszCardHost);
static int PrintOrsPoint(char cTransType, STORS_INFO stOrsInfo);
static int PrintAppVersion();

enum PRINT_PARAM_TYPE
{
	PRINT_PARAM_TYPE_MERCHANTINFO=0x01,		/*Merchant Msg*/
	PRINT_PARAM_TYPE_TRANSCCTRL=0x02,		/*Trans control*/
	PRINT_PARAM_TYPE_SYSTEMCTRL=0x04,		/*System control*/
	PRINT_PARAM_TYPE_COMM=0x08,				/*Comm Param*/
	PRINT_PARAM_TYPE_VERSION=0x10,			/*Version Msg*/
	PRINT_PARAM_TYPE_EMV=0x20,				/*EMV param*/
	PRINT_PARAM_TYPE_OTHER=0x40				/*Other*/
};

typedef struct tagPrintPara
{
	const STTAANSRECORD *pstTransRecord;
	int nReprintFlag;
	int nCurrentPage;
} STPRINTPATA;


/**
* @brief Print receipt
* @param in const STTAANSRECORD *pstTransRecord 
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintWater(const STTAANSRECORD *pstTransRecord, int nReprintFlag)
{
	int nCurrentPage,nPage=1;
	STPRINTPATA stPara;
	
#ifdef MPOS
PubBeep(1);
PubGetKeyCode(1);
return APP_SUCC;
#endif

	if (PubGetPrinter() == _PRINTTYPE_TP)
	{
		GetVarPrintPageCount(&nPage);
	}

	for(nCurrentPage = 1; nCurrentPage <= nPage; nCurrentPage++)
	{
		stPara.pstTransRecord = pstTransRecord;
		stPara.nReprintFlag = nReprintFlag;
		stPara.nCurrentPage = nCurrentPage;

		ASSERT_FAIL(PubPrintCommit( _printwater, (void *)&stPara, 1));
		if (nCurrentPage < nPage)
		{
			NDK_ScrPush();
			PubClearAll();
			PubDisplayStrInline(DISPLAY_MODE_CENTER, 4, "TEAR THE RECEIPT...");
			PubUpdateWindow();
			if(PubGetKeyCode(30) == KEY_ESC)
			{
				return APP_SUCC;
			}
			PubClearLine(4, 4);
			NDK_ScrPop();
		}
	}

	return APP_SUCC;
}

/**
* @brief Get input mode
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
void GetInputMode(const char cAttrType, char *pszName)
{
	switch (cAttrType)
	{
	case ATTR_MAG:
	    strcpy(pszName, "Swipe");
		break;
	case ATTR_MANUAL:
		strcpy(pszName, "Manual");
		break;
	case ATTR_PBOC:	
	case ATTR_EMV_STANDARD:
	case ATTR_EMV_PREDIGEST:
		strcpy(pszName, "Chip");
		break;
	case ATTR_qPBOC:			
	case ATTR_PBOC_EC_RF:		
	case ATTR_EMV_STANDARD_RF:	
	case ATTR_EMV_PREDIGEST_RF:
	case ATTR_MSD:			
		strcpy(pszName, "Wave");
		break;
	case ATTR_FALLBACK:
		strcpy(pszName, "Fall");
		break;
	default:
		strcpy(pszName, "Chip");
		break;
	}
	
	return;
}

/*brife Print string in the middle 
emFontType 
pszStr 
nLineFeed 
*/
int PrePrintInMiddle(EM_PRN_ZM_FONT emFontType, char *pszStr, int nLineFeed)
{
	int nMaxLetter = 0;
	char szLineFeed[64] = {0};
	char szTmp[256] = {0}, szStr[256] = {0};

	if (strlen(pszStr) == 0)
		return APP_QUIT;
	
	if (nLineFeed <= 0)
		return APP_FAIL;
	
	memset(szTmp, '\r', sizeof(szTmp));
	memcpy(szLineFeed, szTmp, nLineFeed);

	switch (emFontType)
	{
	case PRN_ZM_FONT_16x32A:
	case PRN_ZM_FONT_16x32:
	case PRN_ZM_FONT_16x32C:
		nMaxLetter = 24;
		break;	
	case PRN_ZM_FONT_8x16:
		nMaxLetter = 48;
		break;
	case PRN_ZM_FONT_12x24A:
	default:
		nMaxLetter = 32;
		break;	
	}
	if (strlen(pszStr) >= nMaxLetter)
	{
		sprintf(szTmp, "%s%s", pszStr, szLineFeed);
		PubPrePrinter(szTmp);
		return APP_SUCC;
	}

	strcpy(szTmp, pszStr);
	PubAddSymbolToStr(szTmp, nMaxLetter, ' ', ADDCH_MODE_BEFORE_AFTER_STRING);	
	sprintf(szStr, "%s%s", szTmp, szLineFeed);
	PubPrePrinter(szStr);
	return APP_SUCC;
	
}

int PrintWater_TP(const STTAANSRECORD *pstTransRecord, int nReprintFlag, const int nCurrentPage)
{
    int	nPage=3;			
	char szTmpStr[80+1] = {0}, szStr[256];
	char szPosYear[4+1];	
	char szVer[12] = {0};

	/**********************head**************************/
	PrintBillHead(pstTransRecord->nAcqIndex);
	/**********************content**************************/
	if (TRANS_CARDREADER == pstTransRecord->cTransType)
	{
		PubPrePrinter("TID: %s\r", pstTransRecord->szCD_TID);
		PubPrePrinter("MID: %s\r", pstTransRecord->szCD_MID);
	}

	//REF number, trace no and so on
	//Date time
	memset(szPosYear, 0, sizeof(szPosYear));
	PubGetCurrentDatetime(szTmpStr);
	memcpy(szPosYear, szTmpStr, 4);
	PubPrePrinter("DATE: %.4s/%02x/%02x TIME: %02x:%02x:%02x\r", szPosYear, pstTransRecord->sDate[0], pstTransRecord->sDate[1], pstTransRecord->sTime[0], pstTransRecord->sTime[1], pstTransRecord->sTime[2]);

	PubHexToAsc((uchar *)pstTransRecord->szBatchNum, 6, 0,  (uchar *)szTmpStr);
	//PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0,  (uchar *)szTmpStr+10);
	if (TRANS_CARDREADER == pstTransRecord->cTransType)
	{
		PubPrePrinter("BATCH: %.6s    TRACE: %02x%02x%02x\r", szTmpStr, pstTransRecord->sTrace[0], pstTransRecord->sTrace[1], pstTransRecord->sTrace[2]);
	}
	else
	{
		if (TRANS_VOID_REFUND == pstTransRecord->cTransType
			|| TRANS_VOID_INSTALLMENT == pstTransRecord->cTransType
			|| TRANS_VOID_CASH_ADVANCE == pstTransRecord->cTransType
			|| TRANS_VOID == pstTransRecord->cTransType)
			PubPrePrinter("BTH NO: %.6s   TRXN NO: %.6s\r\r", szTmpStr, pstTransRecord->szOldInvoice);
		else
			PubPrePrinter("BTH NO: %.6s   TRXN NO: %.6s\r\r", szTmpStr, pstTransRecord->szInvoice);
	}

/*	GetVarInvoiceNo(szInvoice);
	GetParamHost(szHost);
	PubPrePrinter("HOST:%.3s         INVOICE:%.6s\r\r", szHost, szInvoice);
*/
	//************* TRANSACTION TYPE *************
	GetTransNameForTP( pstTransRecord->cTransType, szStr, pstTransRecord->cFlagCashback, pstTransRecord->cEMV_Status);
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrePrintInMiddle(PRN_ZM_FONT_16x32, szStr, 1);

	//************* CARD NO *************
	PrintCardNo(pstTransRecord, nCurrentPage);

	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (pstTransRecord->sExpDate[0] != 0)
	{
		PubPrePrinter("EXP DATE: XX/XX\r");//, pstTransRecord->sExpDate[0], pstTransRecord->sExpDate[1]
	}
	if (strlen(pstTransRecord->szRefnum) > 0)
	{
		PubPrePrinter("RREF NO: %s\r", pstTransRecord->szRefnum);
	}
	//appr code 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (strlen(pstTransRecord->szAuthCode) > 0)
	{
		if ((strlen(pstTransRecord->szOldAuthCode) > 0)
			&& (TRANS_VOID_REFUND == pstTransRecord->cTransType
			|| TRANS_VOID_INSTALLMENT == pstTransRecord->cTransType
			|| TRANS_VOID_CASH_ADVANCE == pstTransRecord->cTransType
			|| TRANS_VOID == pstTransRecord->cTransType))
		{
			PubPrePrinter("APPR CODE: %s\r", pstTransRecord->szOldAuthCode);
		}
		else
		{
			PubPrePrinter("APPR CODE: %s\r", pstTransRecord->szAuthCode);
		}
	}
	else
	{
		if (strlen(pstTransRecord->szOldAuthCode) > 0 && pstTransRecord->cTransType == TRANS_VOID_OFFLINE)
		{
			PubPrePrinter("APPR CODE: %s\r", pstTransRecord->szOldAuthCode);
		}
	}

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	//Aid and TC
	if ((pstTransRecord->cTransAttr != ATTR_MAG 
		&& pstTransRecord->cTransAttr != ATTR_MANUAL
		&& pstTransRecord->cTransAttr != ATTR_FALLBACK)
		&& (pstTransRecord->cTransType == TRANS_SALE 
		|| pstTransRecord->cTransType == TRANS_INSTALLMENT 
		|| pstTransRecord->cTransType == TRANS_CASH_ADVANCE 
		|| pstTransRecord->cTransType == TRANS_PREAUTH
		|| pstTransRecord->cTransType == TRANS_OFFLINE
		|| pstTransRecord->cTransType == TRANS_CARDREADER))
	{
		STEMVADDTIONWATERINFO *pstAddition = (STEMVADDTIONWATERINFO *)pstTransRecord->sAddition;
		if (TRANS_CARDREADER == pstTransRecord->cTransType)
		{
			PubPrePrinter("APP: %s\r", pstTransRecord->szAppLabel);
			PntHex("AID: ", (uchar *)pstTransRecord->sAID, pstTransRecord->nAidLen, NO);
			PntHex("TC:", (uchar *)pstTransRecord->sTC, 8,  NO);			
			PntHex("TVR: ", (uchar *)pstTransRecord->sTVR, 5, NO);		
		}
		else
		{
			PubPrePrinter("APP: %s\r", pstAddition->szAIDLable);
			PntHex("AID: ", (uchar *)pstAddition->sAID, pstAddition->cAIDLen, NO);
			if (pstTransRecord->nAcqIndex == EM_ACQINDEX_MCCS)
			{
				PntHex("ARQC:", (uchar *)pstAddition->sARQC, 8, NO);	
			}
			else
			{
				PntHex("TC:", (uchar *)pstAddition->sTC, 8,  NO);			
			}
			PntHex("TVR: ", (uchar *)pstAddition->sTVR, 5, NO);		
		}
	}
	ASSERT_FAIL(PubPrePrinter("\r"));


	if (strlen(pstTransRecord->szInstallmentInfo) > 0)
	{
		PrintInstallment((char *)pstTransRecord->szInstallmentInfo);
	}
	
	//add ors by sunh 20170424

	if (YES == CheckTransIsOrs(pstTransRecord->cTransType))
	{
		PrintOrsPoint(pstTransRecord->cTransType, pstTransRecord->stOrsInfo);
	}
	else
	{
		//Print Amt
		PrintAmt(pstTransRecord);
	}	
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	/**********************end**************************/
	//Print statement
    switch(nCurrentPage)
    {
	case 1:  //merchant copy
		PubPrePrinter("------------------------------------------------\r");
		if ((1 == pstTransRecord->cWavePinAndSigFlag) && (1 != pstTransRecord->cHasPinInput))
		{
			ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
			PrePrintInMiddle(PRN_ZM_FONT_16x32, "NO PIN REQUIRED", 1);
			PrePrintInMiddle(PRN_ZM_FONT_16x32, "NO SIGNATURE REQUIRED", 1);
		}
		else if (2 == pstTransRecord->cWavePinAndSigFlag)
		{
			ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
			PubPrePrinter("PIN VERIFIED\r");	
			ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
			PubPrePrinter("SIGNATURE VERIFY\r\r");
			PubPrePrinter("\r\r\r\r\r");
		}
		else
		{
			if(1 == pstTransRecord->cHasPinInput)
			{
				ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
				PrePrintInMiddle(PRN_ZM_FONT_16x32, "PIN VERIFIED", 1);
				PrePrintInMiddle(PRN_ZM_FONT_16x32, "NO SIGNATURE REQUIRED", 1);
			}
			else
			{
				PubPrePrinter("SIGNATURE VERIFY\r\r");
				PubPrePrinter("\r\r\r\r\r");
			}
		}	
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("------------------------------------------------\r");

		if (strlen(pstTransRecord->szHolderName) > 0)
		{
			memset(szTmpStr, 0, sizeof(szTmpStr)); 
			ConvertHolderName((uchar *)pstTransRecord->szHolderName, (uchar *)szTmpStr);
			PubPrePrinter("%s\r", szTmpStr);
		}
		PubPrePrinter("I AGREE TO PAY THE ABOVE TOTAL AMOUNT ACCORDING TO CARD ISSUER AGREEMENT\r");
		break;
	case 2: //customer copy
	default:
		//if(1 == pstTransRecord->cHasPinInput)
		{
			ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
			PrePrintInMiddle(PRN_ZM_FONT_16x32, "NO SIGNATURE REQUIRED", 1);
		}
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
		/*PubPrePrinter("I AGREE TO PAY THE ABOVE TOTAL AMOUNT ACCORDING TO CARD ISSUER AGREEMENT\r\r");*/
		break;
	}
	PrintAppVersion();
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (1 == nCurrentPage)
	{
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "**** MERCHANT COPY ****", 1);
	}
	else if (2 == nCurrentPage)
	{
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "**** CUSTOMER COPY ****", 1);
	}
	else 
	{
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "**** BANK COPY ****", 1);
	}
	
#ifdef DEMO
	PubPrePrinter("---------------\r");
	PubPrePrinter("For debugging only ! \r");
	PubPrePrinter("---------------\r");
#endif

	//footer
	if (YES == CheckTransIsOrs(pstTransRecord->cTransType))
	{
		char szFoot[24+1] = {0};
		GetVarFooterText(szFoot, 0);
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szFoot, 1);
		GetVarFooterText(szFoot, 1);
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szFoot, 1);
		GetVarFooterText(szFoot, 2);
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szFoot, 1);
	}
	else
	{
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "THANK YOU", 1);
	}
	
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	PubPrePrinter("\r\r");

	if (nReprintFlag==REPRINT)
	{
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("DUPLICATED\r");
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	}


    GetVarPrintPageCount(&nPage);	
	if (1 == nCurrentPage || nCurrentPage < nPage)
	{
		PubClearAll();
		if(1 == pstTransRecord->cHasPinInput)
		{
			PubDisplayGen(2, "NO SIGNATURE");
			PubDisplayGen(3, "THANK YOU ! ");
		}
		else
		{
			PubDisplayGen(2, "I HEREBY VERIFY");
			PubDisplayGen(3, "SIGNATURE");
		}
		PubUpdateWindow();
	}
	return APP_SUCC;
}

/**
* @brief Print receipt(Call by PrintWater)
* @param in void *ptrPara 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int _printwater( void *ptrPara )
{
	STPRINTPATA *pstPara;
	pstPara = (STPRINTPATA *)ptrPara;

	ASSERT_FAIL(PrintWater_TP(pstPara->pstTransRecord, pstPara->nReprintFlag, pstPara->nCurrentPage));

	return APP_SUCC;
}

/**
* @brief Get Transaction name according to type
* @param in char cTransType
* @param out char *pszTransName 
* @return void
*/
static void GetTransNameForTP(char cTransType, char *pszTransName, char cIsCashBack, char cEmvStatus)
{
	pszTransName[0] = 0;
	
	switch( cTransType )
	{
	case TRANS_SALE:
		if(cIsCashBack == 1)
		{
			strcpy(pszTransName, "SALE & CASHBACK");
		}
		else
		{
			strcpy(pszTransName, "SALE" ) ;
		}
		break;
	case TRANS_VOID:
		if(cIsCashBack == 1)
		{
			strcpy(pszTransName, "VOID & CASHBACK");
		}
		else
		{
			strcpy(pszTransName, "VOID" ) ;
		}
		break;
	case TRANS_PREAUTH:
		strcpy(pszTransName, "PREAUTH" ) ;
		break;
	case TRANS_VOID_PREAUTH:
		strcpy(pszTransName, "VOID" );
		break;
	case TRANS_AUTHCOMP:
		strcpy(pszTransName, "AUTH_COMP"); 
		break;
	case TRANS_AUTHSALEOFF:
		strcpy(pszTransName, "AUTH SETTLEMENT");
		break;
	case TRANS_VOID_AUTHSALE:
		strcpy(pszTransName, "PRE_AUTH_COMP VOID");
		break;
	case TRANS_REFUND:
		strcpy(pszTransName, "REFUND");	
		break;
	case TRANS_VOID_REFUND:
		strcpy(pszTransName, "VOID REFUND");	
		break;
	case TRANS_VOID_OFFLINE:
		strcpy(pszTransName, "VOID OFFLINE");	
		break;
	case TRANS_OFFLINE:
		strcpy(pszTransName, "OFFLINE");
		break;
	case TRANS_INSTALLMENT:
		strcpy(pszTransName, "EPP");
		break;
	case TRANS_CASH_ADVANCE:
		strcpy(pszTransName, "CASH ADVANCE");
		break;
	case TRANS_VOID_CASH_ADVANCE:
		strcpy(pszTransName, "VOID CASHADV");
		break;
	case TRANS_VOID_INSTALLMENT:
		strcpy(pszTransName, "VOID EPP");
		break;
	case TRANS_BALANCE:
		strcpy(pszTransName, "BALANCE INQUIRY");
		break;
	//ORS add in 20170424
	case TRANS_ORS_INSTANT_REWARD:
		strcpy(pszTransName, "INSTANT REWARD");
		break;
	case TRANS_ORS_VOID_INSTANT_REWARD:
		strcpy(pszTransName, "VOID INSTANT REWARD");
		break;
	case TRANS_ORS_POINTS_RDMPT:
		strcpy(pszTransName, "POINTS REDEMPTION");
		break;
	case TRANS_ORS_VOID_POINTS_RDMPT:
		strcpy(pszTransName, "VOID POINTS RDPT");
		break;
	case TRANS_ORS_VALUE_RDMPT:
		strcpy(pszTransName, "VALUE REDEMPTION");
		break;
	case TRANS_ORS_VOID_VALUE_RDMPT:
		strcpy(pszTransName, "VOID VALUE RDPT");
		break;
	case TRANS_ORS_GIFTCODE_RDMPT:
		strcpy(pszTransName, "GIFT CODE REDEMPTION");
		break;
	case TRANS_ORS_VOID_GIFTCODE_RDMPT:
		strcpy(pszTransName, "VOID GIFT CODE RDPT");
		break;
	case TRANS_ORS_HOTDEAL_RDMPT:
		strcpy(pszTransName, "HOT DEAL REDEMPTION");
		break;
	case TRANS_ORS_VOID_HOTDEAL_RDMPT:
		strcpy(pszTransName, "VOID HOT DEAL RDPT");
		break;
	default:
		strcpy(pszTransName, "SALE" ) ;
		break;
	}
}

int TotalSummary(STTAANSRECORD *pstTransRecord, int *pnTotalCnt, char *pszTotalAmt)
{
	char szAmt[12+1] = {0};
	char szTotalAmt[12+1] = {0};
	char szCashBack[12+1] = {0};

	if (TRANS_VOID == pstTransRecord->cTransType
		|| TRANS_VOID_REFUND == pstTransRecord->cTransType
		|| TRANS_VOID_OFFLINE == pstTransRecord->cTransType
		|| TRANS_VOID_INSTALLMENT == pstTransRecord->cTransType
		|| TRANS_VOID_CASH_ADVANCE == pstTransRecord->cTransType

		|| TRANS_PREAUTH == pstTransRecord->cTransType
		|| TRANS_REFUND == pstTransRecord->cTransType
	)
	{
		return APP_QUIT;
	}

	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szAmt);
	if(1 == pstTransRecord->cFlagCashback)
	{
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0,  (uchar *)szCashBack);
		char szTotal[13] = {0};

		//total
		AmtAddAmt((uchar *)szAmt, (uchar *)szCashBack, (uchar *)szTotal);
		memcpy(szAmt, szTotal, 12);
	}
	
	*pnTotalCnt += 1;
PubDebug("=count: %d", *pnTotalCnt)	;
PubDebug("amt1: %s", szAmt);
PubDebug("amt2: %s", pszTotalAmt);
	AmtAddAmt((uchar *)szAmt, (uchar *)pszTotalAmt, (uchar *)szTotalAmt);
	strcpy(pszTotalAmt, szTotalAmt);
PubDebug("amt: %s", szTotalAmt);
	
	return APP_SUCC;
}

static void PrintSummaryDetail(char *pszCardType, char nCount, char *pszAmt)
{
	char szDispAmt[14] = {0};

	if (0 == nCount)
	{
		return;
	}
	PubPrePrinter("%-17s", pszCardType);
	PubPrePrinter("%-5d", nCount);
	ProAmtToDispOrPnt(pszAmt, szDispAmt);
	PubAllTrim(szDispAmt);
	if (0 == nCount)
	{
		strcpy(szDispAmt, "0.00");
	}
	PubPrePrinter("%16s", szDispAmt);

	return;
}

static int _printallwater(ST_PRINTEXT_ONLY *pstPrintExt)
{
	static int nHasRecord;
	int nFileHandle;
	STTAANSRECORD stTransRecord;
	int nRecordNum = 0;
	int i;//,nPrintNum = 100;
	int nStartRecNo;
	char szTmpStr[128], szStr[128] = {0};
	char szCardType[128] = {0};
	char szBatchNo[6+1] = {0};
	char szHost[32] = {0};
	int lnTotalAcqCnt[ACQUIRER_NUM];
	char lszTotalAcqAmt[ACQUIRER_NUM][12+1];

	memset(lnTotalAcqCnt, 0, ACQUIRER_NUM*4);
	memset(lszTotalAcqAmt, 0, ACQUIRER_NUM*13);
	
	GetRecordNum(&nRecordNum);
	nStartRecNo = pstPrintExt->nCurrentNo;
	if (nStartRecNo==1)
	{
		nHasRecord=0;
	}

	ASSERT_FAIL(PubOpenFile(FILE_WATER, "r", &nFileHandle));

	/**********************head**************************/
	PrintBillHead(pstPrintExt->nAcqIndex);
	/**********************content**************************/

	//REF number, trace no and so on
	//Date time
	PubGetCurrentDatetime(szTmpStr);
	PubPrePrinter("DATE: %.4s/%.02s/%.02s TIME: %.02s:%.02s:%.02s\r", szTmpStr, szTmpStr+4, szTmpStr+6, szTmpStr+8, szTmpStr+10, szTmpStr+12);

	GetVarBatchNo(szBatchNo, pstPrintExt->nAcqIndex);
	PubPrePrinter("BTH NO: %.6s\r", szBatchNo);
	GetCardHost(pstPrintExt->nAcqIndex, szHost);
	PubPrePrinter("HOST: %s\r\r", szHost);
	
	for(i=0; (pstPrintExt->nCurrentNo <=nRecordNum); (pstPrintExt->nCurrentNo)++)//i<nPrintNum && 
	{
		PubReadRec(nFileHandle, pstPrintExt->nCurrentNo, (char *)&stTransRecord);
	
		if (stTransRecord.nAcqIndex != pstPrintExt->nAcqIndex)	
		{
			continue;
		}
		if (stTransRecord.cStatus == '1' || stTransRecord.cStatus == 0x01 ||stTransRecord.cSendFlag == 2)
		{
			continue;
		}

PubDebug("acqtype: %d[cnt: %d]", stTransRecord.nAcqType, lnTotalAcqCnt[stTransRecord.nAcqType]);
		TotalSummary(&stTransRecord, &lnTotalAcqCnt[stTransRecord.nAcqType], lszTotalAcqAmt[stTransRecord.nAcqType]);
PubDebug("cnt: %d", lnTotalAcqCnt[stTransRecord.nAcqType]);
		
		// Print head
		if (nStartRecNo==1 && i==0 )
		{	
			ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
			PrePrintInMiddle(PRN_ZM_FONT_16x32, "DETAIL REPORT", 2);
			
			ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
			PubPrePrinter("PAN              CARD NAME    EXP DATE\r");
			PubPrePrinter("TRANS            APPR CODE   DATE TIME\r");
			PubPrePrinter("AMOUNT           TRANS NO     RREF NUM\r");
			PubPrePrinter("======================================\r");	
							
		}
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
		//CARD NO
		PubHexToAsc((uchar *)stTransRecord.sPan, stTransRecord.nPanLen, 0, (uchar *)szTmpStr);
		MaskCardNo(szTmpStr, 1);
		if (strlen(szTmpStr) > 16)
		{
			PubPrePrinter("%-19s", szTmpStr);
			
			//TYPE
			GetCardTypeStr(stTransRecord.nAcqType, szCardType);
			PubPrePrinter("%-14s", szCardType);
		}
		else
		{
			PubPrePrinter("%-17s", szTmpStr);
			
			//TYPE
			GetCardTypeStr(stTransRecord.nAcqType, szCardType);
			PubPrePrinter("%-16s", szCardType);

		}
		//EXP DATE
		sprintf(szTmpStr, "XX/XX");	
		PubPrePrinter("%5s\r", szTmpStr);
		
		//TRAMS
		GetTransNameForTP(stTransRecord.cTransType, szStr, stTransRecord.cFlagCashback, stTransRecord.cEMV_Status);
		PubPrePrinter("%-17s", szStr);
		
		//APPR CODE
		PubPrePrinter("%-6s", stTransRecord.szAuthCode);

		//DATE TIME
		sprintf(szStr, "%02x/%02x %02x:%02x:%02x", stTransRecord.sDate[0], stTransRecord.sDate[1], stTransRecord.sTime[0], stTransRecord.sTime[1], stTransRecord.sTime[2]);
		PubPrePrinter("%15s\r", szStr);


		//AMOUNT
		GetPrintAmt(&stTransRecord, szTmpStr);
		PubPrePrinter("%-17s", szTmpStr);	
		
		//TRACE NO
		PubPrePrinter("%-6s", stTransRecord.szInvoice);	
		
		//RREF NO
		PubPrePrinter("%15s\r", stTransRecord.szRefnum);			
		PubPrePrinter("--------------------------------------\r");	
		
		i++;
	}
	
	ASSERT_FAIL(PubCloseFile(&nFileHandle));

	if (i>0)
	{
		nHasRecord = 1;
	}
	else
	{
		//PubMsgDlg("PRINT ALL", "NO RECORD", 0, 5);
		//return -5;
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_10x8, PRN_HZ_FONT_16x16, PRN_MODE_HEIGHT_DOUBLE, 0));
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("--------------------------------------\r");	
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_16x16, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("NO RECORDS\r\r");
		return APP_SUCC;
	}
	
	PubPrePrinter("CREDIT:\r");
	PubPrePrinter("CARD TYPE        COUNT          AMOUNT\r");
	switch (pstPrintExt->nAcqIndex)
	{
	case EM_ACQINDEX_V_M_J:
		PrintSummaryDetail("VISA", lnTotalAcqCnt[EM_ACQTYPE_VISA], lszTotalAcqAmt[EM_ACQTYPE_VISA]);
		PrintSummaryDetail("MASTER", lnTotalAcqCnt[EM_ACQTYPE_MASTER], lszTotalAcqAmt[EM_ACQTYPE_MASTER]);
		PrintSummaryDetail("JCB", lnTotalAcqCnt[EM_ACQTYPE_JCB], lszTotalAcqAmt[EM_ACQTYPE_JCB]);
		break;
	case EM_ACQINDEX_EPP_V_M:
	case 5://vm 12month
	case 6://vm 18month
	case 7://vm 24month
		PrintSummaryDetail("VISA", lnTotalAcqCnt[EM_ACQTYPE_VISA], lszTotalAcqAmt[EM_ACQTYPE_VISA]);
		PrintSummaryDetail("MASTER", lnTotalAcqCnt[EM_ACQTYPE_MASTER], lszTotalAcqAmt[EM_ACQTYPE_MASTER]);
		break;
	case EM_ACQINDEX_AMEX:
	case 8:
	case 9:
	case 10:
	case 11:
		PrintSummaryDetail("AMEX", lnTotalAcqCnt[EM_ACQTYPE_AMEX], lszTotalAcqAmt[EM_ACQTYPE_AMEX]);
		break;
	case EM_ACQINDEX_MCCS:
		PrintSummaryDetail("MCCS", lnTotalAcqCnt[EM_ACQTYPE_MCCS], lszTotalAcqAmt[EM_ACQTYPE_MCCS]);
		break;
	case EM_ACQINDEX_UPI:
		PrintSummaryDetail("UPI", lnTotalAcqCnt[EM_ACQTYPE_UPI], lszTotalAcqAmt[EM_ACQTYPE_UPI]);
		break;
	default:
		break;
	}
	
	//Print tail
	if (pstPrintExt->nCurrentNo > nRecordNum && nHasRecord==1)
	{
		if (PubGetPrinter() == _PRINTTYPE_TP)
		{
			PubPrePrinter( "\r\r- - - - - -X - - - - - - X - - - - - -\r");
		}	
	}
	return APP_SUCC;
}

/**
* @brief Print detail
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintAllWater(int nAcqIndex)
{
	int nRecordNum = 0;
	int nRet = 0;
	ST_PRINTEXT_ONLY stPrintExt;

	stPrintExt.nAcqIndex = nAcqIndex;
	stPrintExt.nCurrentNo = 1;
	
	GetRecordNum(&nRecordNum);
	if( nRecordNum<=0 )
	{
		PubMsgDlg("PRINT ALL", "NO TRANS", 0, 5);
		return APP_SUCC;
	}
	while(1)
	{
		if(stPrintExt.nCurrentNo > nRecordNum)
		{
			break;
		}
		
		nRet = PubPrintCommit(_printallwater, (void *)&stPrintExt, 1);
		if(nRet != APP_SUCC)
		{
			if (APP_FUNCQUIT == nRet)
			{
				PubMsgDlg("PRINT ALL", "NO TRANS TO PRINT", 1, 3);
			}
			return nRet;
		}
	}
	
	return APP_SUCC;
}

int GetSectionSettleData(STSETTLE *plstSecSettle, int nAcqIndex)
{
	STSETTLE lstSecSettle[3];
	int nRet = 0;
	int nWaterSum = 0;
	int nFileHandle;
	int i = 0;
	STTAANSRECORD stTransRecord;
	char szTmp[6092] = {0};
	int nPos = 0;
	char szInvoice[6+1] = {0};

TRACE(("======"));

	memcpy(lstSecSettle, plstSecSettle, sizeof(STSETTLE)*3);
	GetRecordNum(&nWaterSum);
TRACE(("======"));

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
TRACE(("======"));
	
	for (i = 1; i <= nWaterSum; i++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		nRet = PubReadRec(nFileHandle, i, (char *)&stTransRecord);
		if (nAcqIndex != stTransRecord.nAcqIndex)
		{
			continue;
		}
		switch (nAcqIndex)
		{
		case EM_ACQINDEX_V_M_J:	
			switch (stTransRecord.nAcqType)
			{
				case EM_ACQTYPE_VISA:
					ChangeSimpleSettle(&stTransRecord, &lstSecSettle[0]);
					break;
				case EM_ACQTYPE_MASTER:
					ChangeSimpleSettle(&stTransRecord, &lstSecSettle[1]);
					break;
				case EM_ACQTYPE_JCB:
					ChangeSimpleSettle(&stTransRecord, &lstSecSettle[2]);
					break;
			}
			break;
		case 4:	//V/M EPP 6 
		case 5:	//V/M EPP 12
		case 6:	//V/M EPP 18 
		case 7:	//V/M EPP 24 
			switch (stTransRecord.nAcqType)
			{
				case EM_ACQTYPE_VISA:
					ChangeSimpleSettle(&stTransRecord, &lstSecSettle[0]);
					break;
				case EM_ACQTYPE_MASTER:
					ChangeSimpleSettle(&stTransRecord, &lstSecSettle[1]);
					break;
			}
			break;
		default:
			ChangeSimpleSettle(&stTransRecord, &lstSecSettle[0]);
			break;
		}
		
	}
	PubCloseFile(&nFileHandle);	
TRACE(("======"));
	memcpy(plstSecSettle, lstSecSettle, sizeof(STSETTLE)*3);
TRACE(("======"));
	
}

int PrintSecSettle(ST_SETTLE_REC stSettleRec, int *pnSecRecordNum)
{
	char lszCardType[3][20] = 
	{
		"VISA",
		"MASTER",
		"JCB"
	};
	int i = 0;
	int nNum = 0;
	char szDispAmt[13+1] = {0};
	int nWholeTotalCount = 0;
	int nTotalSale = 0;
	char szTotalSale[12+1] = {0};
	char szCardName[24] = {0};

	for (i = 0; i < 3; i++)
	{
		nWholeTotalCount = stSettleRec.lstSecSettle[i]._SaleNum + stSettleRec.lstSecSettle[i]._RefundNum
			+ stSettleRec.lstSecSettle[i]._OfflineNum + stSettleRec.lstSecSettle[i]._VoidSaleNum 
			+ stSettleRec.lstSecSettle[i]._VoidOfflineNum + stSettleRec.lstSecSettle[i]._VoidRefundNum;
		if (nWholeTotalCount == 0)
		{
			continue;
		}
		else
		{
			(*pnSecRecordNum)++;
		}

		nTotalSale = 0;
		memset(szTotalSale, '0', 12);
		switch (stSettleRec.nAcqIndex)
		{
		case EM_ACQINDEX_V_M_J:
		case 4:
		case 5:
		case 6:
		case 7:
			strcpy(szCardName, lszCardType[i]);
			break;
		case EM_ACQINDEX_AMEX:
		case 8:
		case 9:
		case 10:
		case 11:
			strcpy(szCardName, "AMEX");
			break;
		case EM_ACQINDEX_MCCS:
			strcpy(szCardName, "MYDEBIT");
			break;
		case EM_ACQINDEX_UPI:
			strcpy(szCardName, "UNIONPAY");
			break;
		default:
			return APP_QUIT;
		}
		PubPrePrinter("CARD NAME: %s\r", szCardName);
		
		//PubPrePrinter("------------------------------\r");
		PubPrePrinter("             Count       TOTAL\r");
		nNum = stSettleRec.lstSecSettle[i]._SaleNum - stSettleRec.lstSecSettle[i]._VoidSaleNum;
		MathBcd(stSettleRec.lstSecSettle[i]._SaleAmount.sSettleAmt, stSettleRec.lstSecSettle[i]._VoidSaleAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._SaleAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("SALES        %3ld %13s\r", nNum, szDispAmt);
		nTotalSale += nNum;
		IncreaseAmt(stSettleRec.lstSecSettle[i]._SaleAmount.sSettleAmt, szTotalSale);
		
		//VOID
		nNum = stSettleRec.lstSecSettle[i]._VoidSaleNum;
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._VoidSaleAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID SALES   %3ld %13s\r", nNum, szDispAmt);

		//refund
		nNum = stSettleRec.lstSecSettle[i]._RefundNum - stSettleRec.lstSecSettle[i]._VoidRefundNum;
		MathBcd(stSettleRec.lstSecSettle[i]._RefundAmount.sSettleAmt, stSettleRec.lstSecSettle[i]._VoidRefundAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._RefundAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("REFUNDS      %3ld %13s\r", nNum, szDispAmt);

		//VOID refund
		nNum = stSettleRec.lstSecSettle[i]._VoidRefundNum;
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._VoidRefundAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID REFUNDS %3ld %13s\r", nNum, szDispAmt);

		//offline
		nNum = stSettleRec.lstSecSettle[i]._OfflineNum - stSettleRec.lstSecSettle[i]._VoidOfflineNum;
		MathBcd(stSettleRec.lstSecSettle[i]._OfflineAmount.sSettleAmt, stSettleRec.lstSecSettle[i]._VoidOfflineAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._OfflineAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("OFFLINE      %3ld %13s\r", nNum, szDispAmt);
		nTotalSale += nNum;
		IncreaseAmt(stSettleRec.lstSecSettle[i]._OfflineAmount.sSettleAmt, szTotalSale);

		//VOID offline
		nNum = stSettleRec.lstSecSettle[i]._VoidOfflineNum;
		BcdAmtToDisp(stSettleRec.lstSecSettle[i]._VoidOfflineAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID OFFLINE %3ld %13s\r", nNum, szDispAmt);

		PubPrePrinter("------------------------------\r");
		ProAmtToDispOrPnt((char*)szTotalSale, szDispAmt);
		PubAllTrim(szDispAmt);
		PubPrePrinter("CARD TOTAL   %3ld %13s\r", nTotalSale, szDispAmt);					
	}
}

int _printsettle(void *ptrPara)
{
	ST_SETTLE_REC stSettleRec;
	STRECFILE stSettleFile;
	char szTmpstr[41];
	int nNum = 0;
	char szDispAmt[13+1] = {0};
//	int *pnReprintFlag = (int *)ptrPara;
	char szCardHost[32] = {0};
	ST_PRINTEXT_ONLY *pstSettleOnly = (ST_PRINTEXT_ONLY *)ptrPara;
	int nRecordNo = 0;
	int nRet = 0;
	int i  = 0;
	ST_SETTLE_REC stTmp;
	char szIndex[3+1] = {0};
	int nTotalSale = 0;
	char szTotalSale[12+1] = {0};
	int nSecRecordNum = 0;

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	
	if (pstSettleOnly->nReprintFlag == REPRINT)
	{
		sprintf(szIndex, "%03d", pstSettleOnly->nAcqIndex);
	    nRet = PubFindRec(SETTLEFILENAME, szIndex, NULL,1, (char *)&stSettleRec, &nRecordNo);
PubDebug("index: %s[%d]", szIndex, nRet);
PubDebug("record index: %d[%s][record: %d]", stSettleRec.nAcqIndex, stSettleRec.szAcqIndex, nRecordNo);
	}
	else
	{
		GetVarInvoiceNo(stSettleRec.szInvoice);	
		GetVarBatchNo(stSettleRec.BatchNo, pstSettleOnly->nAcqIndex);				
		PubGetCurrentDatetime(stSettleRec.BatchDatetime);
		GetVarSettleDateTime(szTmpstr);
		if (memcmp(szTmpstr, "\x00\x00\x00\x00\x00", 5) == 0)
		{
			;
		}
		else
		{
			PubHexToAsc((uchar *)szTmpstr, 10, 0, (uchar *)(stSettleRec.BatchDatetime+4));
		}
		stSettleRec.SettleFlag = GetVarIsTotalMatch();
		stSettleRec.nAcqIndex = pstSettleOnly->nAcqIndex;
		sprintf(stSettleRec.szAcqIndex, "%03d", stSettleRec.nAcqIndex);
		GetSettleData(&(stSettleRec.stSettle), pstSettleOnly->nAcqIndex); 	
TRACE(("======"));
		GetSectionSettleData(stSettleRec.lstSecSettle, pstSettleOnly->nAcqIndex); 	
		
		if (pstSettleOnly->nReprintFlag==FIRSTPRINT)
		{
			//GetBatchIndex(stSettleRec.nAcqIndex, 0, &nRecordNo);
			//nRecordNo += 1;
			nRet = PubUpdateRec(SETTLEFILENAME, stSettleRec.nAcqIndex + 1, (char *)(&stSettleRec));
PubDebug("3nRecordNo=%d [%d] ", nRecordNo, nRet);
		
		}
	}

	GetVarSettleDateTime(szTmpstr);
	
	if (PubGetPrinter()==_PRINTTYPE_TP)
	{
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	}
	else
	{
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_10x16, PRN_HZ_FONT_16x16, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	}	

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrintBillHead(stSettleRec.nAcqIndex);	
	
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	// SETTLE DATETIME
	PubPrePrinter("DATE: %4.4s/%2.2s/%2.2s  TIME: %2.2s:%2.2s:%2.2s\r\r", 
		stSettleRec.BatchDatetime, stSettleRec.BatchDatetime+4, stSettleRec.BatchDatetime+6, 
		stSettleRec.BatchDatetime+8, stSettleRec.BatchDatetime+10, stSettleRec.BatchDatetime+12);

	//BATCH NO
	GetCardHost(stSettleRec.nAcqIndex, szCardHost);
	PubPrePrinter("HOST: %s\r", szCardHost);
	PubPrePrinter("BATCH NO: %.6s\r", stSettleRec.BatchNo);

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	if (pstSettleOnly->nReprintFlag==TOTAL_PRINT)
		PrePrintInMiddle(PRN_ZM_FONT_16x32, "SUMMARY REPORT", 2);
	else
		PrePrintInMiddle(PRN_ZM_FONT_16x32, "SETTLEMENT REPORT", 2);
	

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	
	PrintSecSettle(stSettleRec, &nSecRecordNum);
	if (nSecRecordNum > 1)
	{
		PubPrePrinter("==============================\r");
		PubPrePrinter("GRAND TOTALS\r");
		//PubPrePrinter("------------------------------\r");
		PubPrePrinter("             Count       TOTAL\r");
		/*******SUMMARY***********************/
		//sale
		nNum = stSettleRec.stSettle._SaleNum - stSettleRec.stSettle._VoidSaleNum;
		MathBcd(stSettleRec.stSettle._SaleAmount.sSettleAmt, stSettleRec.stSettle._VoidSaleAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.stSettle._SaleAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("SALES        %3ld %13s\r", nNum, szDispAmt);
		nTotalSale += nNum;
		IncreaseAmt(stSettleRec.stSettle._SaleAmount.sSettleAmt, szTotalSale);
		
		//VOID
		nNum = stSettleRec.stSettle._VoidSaleNum;
		BcdAmtToDisp(stSettleRec.stSettle._VoidSaleAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID SALES   %3ld %13s\r", nNum, szDispAmt);

		//refund
		nNum = stSettleRec.stSettle._RefundNum - stSettleRec.stSettle._VoidRefundNum;
		MathBcd(stSettleRec.stSettle._RefundAmount.sSettleAmt, stSettleRec.stSettle._VoidRefundAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.stSettle._RefundAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("REFUNDS      %3ld %13s\r", nNum, szDispAmt);

		//VOID refund
		nNum = stSettleRec.stSettle._VoidRefundNum;
		BcdAmtToDisp(stSettleRec.stSettle._VoidRefundAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID REFUNDS %3ld %13s\r", nNum, szDispAmt);

		//offline
		nNum = stSettleRec.stSettle._OfflineNum - stSettleRec.stSettle._VoidOfflineNum;
		MathBcd(stSettleRec.stSettle._OfflineAmount.sSettleAmt, stSettleRec.stSettle._VoidOfflineAmount.sSettleAmt, 6, EM_SUB);
		BcdAmtToDisp(stSettleRec.stSettle._OfflineAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("OFFLINE      %3ld %13s\r", nNum, szDispAmt);
		nTotalSale += nNum;
		IncreaseAmt(stSettleRec.stSettle._OfflineAmount.sSettleAmt, szTotalSale);

		//VOID offline
		nNum = stSettleRec.stSettle._VoidOfflineNum;
		BcdAmtToDisp(stSettleRec.stSettle._VoidOfflineAmount.sSettleAmt, szDispAmt);
		PubPrePrinter("VOID OFFLINE %3ld %13s\r", nNum, szDispAmt);

		PubPrePrinter("-------------------------------\r");
		ProAmtToDispOrPnt((char*)szTotalSale, szDispAmt);
		PubAllTrim(szDispAmt);
		PubPrePrinter("CARD TOTAL   %3ld %13s\r", nTotalSale, szDispAmt);
	}		
	else
	{
		//PubPrePrinter("NO BATCH\r");
	}
	
	if (pstSettleOnly->nReprintFlag==REPRINT)
	{
		PubPrePrinter(" ******REPRINT*******\r");
	}
	else if (pstSettleOnly->nReprintFlag==FIRSTPRINT)
	{
		PubPrePrinter("\r");
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0)); 
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PrePrintInMiddle(PRN_ZM_FONT_16x32, "Settlement Successful", 2);
	}
	
	//PrintSoftVer();
	if (PubGetPrinter()==_PRINTTYPE_TP)
	{
		PubPrePrinter("\r\r" );
	}
	return APP_SUCC;
}

/**
* @brief Print settle
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintSettle(int nReprintFlag, int nAcqIndex)
{	
	int nRet = -1;
	ST_SETTLE_REC stSettleRec;
	ST_PRINTEXT_ONLY stSettleOnly;

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	memset(&stSettleOnly, 0, sizeof(ST_PRINTEXT_ONLY));
	stSettleOnly.nReprintFlag =  nReprintFlag;
	stSettleOnly.nAcqIndex = nAcqIndex;
	
	if (REPRINT == nReprintFlag)
	{
		char szIndex[3+1] = {0};
		int nRecordNo = 0;
		
		sprintf(szIndex, "%03d", stSettleOnly.nAcqIndex);
	    nRet = PubFindRec(SETTLEFILENAME, szIndex, NULL,1, (char *)&stSettleRec, &nRecordNo);
		if (APP_SUCC != nRet)
		{
			PubMsgDlg("REPRINT SETTLE", "NO RECEIPT", 0, 10);
			return APP_QUIT;
		}
	}	
	
	PubClearAll();
	DISP_PRINTING_NOW;
	ASSERT_FAIL(PubPrintCommit( _printsettle, (void *)&stSettleOnly, 1));
	return APP_SUCC;
}

/**
* @brief Print summary
* @param in int nReprintFlag  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PrintTotal(int nAcqIndex)
{
	ST_SETTLE_REC stSettleRec;
	int nPrintFlag = TOTAL_PRINT;
	ST_PRINTEXT_ONLY stSettleOnly;

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	memset(&stSettleOnly, 0, sizeof(ST_PRINTEXT_ONLY));
	
	stSettleOnly.nAcqIndex = nAcqIndex;
	stSettleOnly.nReprintFlag = nPrintFlag;
	PubClearAll();
	DISP_PRINTING_NOW;
	ASSERT_FAIL(PubPrintCommit( _printsettle, (void *)&stSettleOnly, 1));
	return APP_SUCC;
}

int _printParam(void *ptrPara)
{
	char szTmpStr[21];
	STAPPPOSPARAM stAppPosParam;
	STAPPCOMMPARAM stAppCommParam;
	
	int *pnPrintType=(int *)ptrPara	;
	char szSoftVer[16+1];
	char szDispAmt[13+1] = {0};
	

	PubClearAll();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "PRINTING...");
	PubUpdateWindow();

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_8x16, PRN_HZ_FONT_16x16, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	
	if((*pnPrintType)&PRINT_PARAM_TYPE_EMV) /*<EMV PARAM*/
	{
		return _printemvparam();
	}
		
	// Get parameter
	GetAppCommParam(&stAppCommParam);
	GetAppPosParam(&stAppPosParam);

	if((*pnPrintType)&PRINT_PARAM_TYPE_MERCHANTINFO) /*<Merchant msg*/
	{
		PubPrePrinter("MERCHANT INFO\r\r");
		
		PubPrePrinter("Merchant Name:%s\r", stAppPosParam.szMerchantNameEn);

		PubPrePrinter("Addr line1:%s\r", stAppPosParam.szMerchantAddr[0]);
		PubPrePrinter("Addr line2:%s\r", stAppPosParam.szMerchantAddr[1]);
		PubPrePrinter("Addr line3:%s\r", stAppPosParam.szMerchantAddr[2]);
		PubPrePrinter("Addr line4:%s\r", stAppPosParam.szMerchantAddr[4]);

		PubPrePrinter("V/M/J MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_V_M_J]);
		PubPrePrinter("V/M/J TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_V_M_J]);
		PubPrePrinter("AMEX MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_AMEX]);
		PubPrePrinter("AMEX TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_AMEX]);
		PubPrePrinter("MCCS MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_MCCS]);
		PubPrePrinter("MCCS TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_MCCS]);
		PubPrePrinter("UPI MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_UPI]);
		PubPrePrinter("UPI TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_UPI]);
		PubPrePrinter("V/M EPP MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_EPP_V_M]);
		PubPrePrinter("V/M EPP TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_EPP_V_M]);
		PubPrePrinter("AMEX EPP MID:%s\r", stAppPosParam.szMerchantId[EM_ACQINDEX_EPP_AMEX]);
		PubPrePrinter("AMEX EPP TID:%s\r", stAppPosParam.szTerminalId[EM_ACQINDEX_EPP_AMEX]);

		if(strlen(stAppPosParam.szAppDispname)>0)
		{
			PubPrePrinter("App Name:%s\r", stAppPosParam.szAppDispname);
		}
		else
		{
			PubPrePrinter("App Name:%s\r", APP_NAMEDESC);
		}
		
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_VERSION) /*<Version msg*/
	{
		PubPrePrinter("VERSION INFO\r\r");
		
		memset(szSoftVer, 0, sizeof(szSoftVer));
		GetVarSoftVer(&szSoftVer);	
		PubPrePrinter("App Version: %s\r", szSoftVer);
	}
	if((*pnPrintType)&PRINT_PARAM_TYPE_TRANSCCTRL) /*<Transactiion control*/
	{
		PubPrePrinter("TRANS CONTROL\r\r");
		
		PntTraditionOnoff(stAppPosParam.sTransSwitch);
		PubPrePrinter("Enable Manul:%s\r", stAppPosParam.cIsCardInput == '1' ? "YES" : "NO");
		PubPrePrinter("Default Trans:%s\r", stAppPosParam.cDefaultTransType == '1' ? "SALE" : "AUTH");
		memset(szDispAmt, 0, sizeof(szDispAmt));
		ProAmtToDispOrPnt(stAppPosParam.szMaxRefundAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		PubPrePrinter("Max Refund Amt:%s\r", szDispAmt);
		PubPrePrinter("Print Water Record:%s\r", stAppPosParam.cIsPrintWaterRec == '1' ? "YES" : "NO");
		PubPrePrinter("Offline Send Type:%s\r", stAppPosParam.cIsOfflineSendNow == '1' ? "Online" : "Settle");
		PubPrePrinter("Offline Send Times:%d\r", stAppCommParam.cOffResendNum >= 0x30 ? stAppCommParam.cOffResendNum - 0x30 : stAppCommParam.cOffResendNum);
		PubPrePrinter("Enter Admin Pwd:%s\r", stAppPosParam.cIsVoidPwd == '1' ? "YES" : "NO");
	}
	
	if((*pnPrintType)&PRINT_PARAM_TYPE_SYSTEMCTRL) /*<System param*/
	{
		PubPrePrinter("SYSTEM CONTROL\r\r");
		
		memset(szTmpStr, 0, sizeof(szTmpStr));
		GetVarTraceNo(szTmpStr);
		PubPrePrinter("Trace No.:%s\r", szTmpStr);
		PubPrePrinter("Print Page:%d\r", stAppPosParam.cPrintPageCount - '0');
		PubPrePrinter("Reversal Times:%d\r", stAppCommParam.cReSendNum >= 0x30 ? stAppCommParam.cReSendNum - 0x30 : stAppCommParam.cReSendNum);
		PubPrePrinter("Max Trans Count:%d\r", atoi(stAppPosParam.szMaxTransCount));
		PubPrePrinter("Tip Rate:%d\r", atoi(stAppPosParam.szTipRate));
		PubPrePrinter("Print Minus:%s\r", stAppPosParam.cIsPrintMinus == '1' ? "YES" : "NO");
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_OTHER) /*<Other*/
	{
		PubPrePrinter("OTHER\r\r");
		
		PubPrePrinter("Enable Swipe In Void:%s\r", stAppPosParam.cIsSaleVoidStrip == '1' ? "YES" : "NO");
		PubPrePrinter("Enable Pin In Void:%s\r", stAppPosParam.cIsVoidPin == '1' ? "YES" : "NO");
		PubPrePrinter("Key Index:%s\r", stAppPosParam.szMainKeyNo);
		PubPrePrinter("Encypt Mode:%s\r", stAppPosParam.cEncyptMode == '0' ? "DES" : "3DES");
	}

	if((*pnPrintType)&PRINT_PARAM_TYPE_COMM)
	{
		PubPrePrinter("COMM\r\r");
		
		switch(stAppCommParam.cCommType)
		{
		case COMM_RS232:
			PubPrePrinter("Comm Type: %s\r", "RS232");
			break;
		case COMM_DIAL:
			PubPrePrinter("Comm Type: %s\r", "MODEM");
			PubPrePrinter("Tel.1: %s\r", stAppCommParam.szTelNum1);
			PubPrePrinter("Tel.2: %s\r", stAppCommParam.szTelNum1);
			PubPrePrinter("Tel.3: %s\r", stAppCommParam.szTelNum1);
			break;
		case COMM_GPRS:
		case COMM_CDMA:
		case COMM_ETH:
		case COMM_WIFI:
			if (stAppCommParam.cCommType == COMM_GPRS)
			{
				PubPrePrinter("Comm Type: %s\r", "GPRS");
				PubPrePrinter("APN.1: %s\r", stAppCommParam.szAPN1);
				PubPrePrinter("APN.2: %s\r", stAppCommParam.szAPN2);
			}
			else if (stAppCommParam.cCommType == COMM_ETH)
			{
				PubPrePrinter("Comm Type: %s\r", "ETH");
				PubPrePrinter("Ip Addr.1: %s\r", stAppCommParam.szIpAddr);
				PubPrePrinter("Mask: %s\r", stAppCommParam.szMask);
				PubPrePrinter("Gate: %s\r", stAppCommParam.szGate);
			}
			else if (stAppCommParam.cCommType == COMM_CDMA)
			{
				PubPrePrinter("Comm Type: %s\r", "CDMA");			
			}
			else if(stAppCommParam.cCommType == COMM_WIFI)
			{
				PubPrePrinter("Comm Type: %s\r", "Wifi");			
				PubPrePrinter("Ssid: %s\r", stAppCommParam.szWifiSsid);			
				switch(stAppCommParam.cWifiMode)
				{
				case WIFI_NET_SEC_NONE:
					PubPrePrinter("Wifi Net Mode: NONE\r");			
					break;
				case WIFI_NET_SEC_WEP_OPEN:
					PubPrePrinter("Wifi Net Mode: OPEN\r");			
					break;
				case WIFI_NET_SEC_WEP_SHARED:
					PubPrePrinter("Wifi Net Mode: SHARED\r");			
					break;
				case WIFI_NET_SEC_WPA:
					PubPrePrinter("Wifi Net Mode: WAP\r");			
					break;
				case WIFI_NET_SEC_WPA2:
					PubPrePrinter("Wifi Net Mode: WAP2\r");			
					break;
				}
			}

			if(stAppCommParam.cIsDns)
			{
				PubPrePrinter("Dns Ip: %s\r", stAppCommParam.szDNSIp1);			
				PubPrePrinter("Domain.1: %s\r", stAppCommParam.szDomain);			
				PubPrePrinter("Domain.2: %s\r", stAppCommParam.szDomain2);			
				PubPrePrinter("Dns Port.1: %s\r", stAppCommParam.szPort1);			
				PubPrePrinter("Dns Port.2: %s\r", stAppCommParam.szPort2);			
			}
			else
			{
				PubPrePrinter("Ip.1: %s\r", stAppCommParam.szIp1);			
				PubPrePrinter("Ip.2: %s\r", stAppCommParam.szIp2);			
				PubPrePrinter("Port.1: %s\r", stAppCommParam.szPort1);			
				PubPrePrinter("Port.2: %s\r", stAppCommParam.szPort2);			
			}

			if ((stAppCommParam.cCommType == COMM_GPRS)||(stAppCommParam.cCommType == COMM_CDMA))
			{
				PubPrePrinter("User Name: %3.3s****\r", stAppCommParam.szUser);			
				
				PubPrePrinter("User Pwd: %3.3s****\r", stAppCommParam.szPassWd);			

				PubPrePrinter("Wireless Dialnum: %s\r", stAppCommParam.szWirelessDialNum);			

				PubPrePrinter("Wireless Mode: %d\r", stAppCommParam.cMode);			
			}
			break;
		default:
			break;
		}
		
		PubPrePrinter("V/M/J TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_V_M_J]);			
		PubPrePrinter("V/M/J NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_V_M_J]);			
		PubPrePrinter("AMEX TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_AMEX]);			
		PubPrePrinter("AMEX NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_AMEX]);			
		PubPrePrinter("MCCS TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_MCCS]);			
		PubPrePrinter("MCCS NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_MCCS]);			
		PubPrePrinter("UPI TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_UPI]);			
		PubPrePrinter("UPI NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_UPI]);			
		PubPrePrinter("EPP V/M TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_EPP_V_M]);			
		PubPrePrinter("EPP V/M/J NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_EPP_V_M]);			
		PubPrePrinter("EPP AMEX TPDU: %s\r", stAppCommParam.szTransTpdu[EM_ACQINDEX_EPP_AMEX]);			
		PubPrePrinter("EPP AMEX NII: %s\r", stAppCommParam.szTransNii[EM_ACQINDEX_EPP_AMEX]);			
		
		PubPrePrinter("Redial Times: %d\r", stAppCommParam.cReDialNum >= 0x30 ? stAppCommParam.cReDialNum - 0x30 : stAppCommParam.cReDialNum);			

		PubPrePrinter("Timeout: %d\r", stAppCommParam.cTimeOut);			

		PubPrePrinter("Is Predial: %s\r", stAppCommParam.cPreDialFlag=='1' || stAppCommParam.cPreDialFlag==1 ? "YES" : "NO");			
	}
	
	return APP_SUCC;
}

int PrintParam(void)
{
	int nRet;
	int nSelect=0xff;
	int nType=0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.MERCHANT"),
		tr("2.TRANS CONTROL"),
		tr("3.SYSTEM CONTROL"),
		tr("4.COMM"),
		tr("5.VERSION"),
		tr("6.EMV PARA"),
		tr("7.OTHER")
	};
	while(1)
	{
		//nRet = ProSelectList("1.MERCHANT||2.TRANS CONTROL||3.SYSTEM CONTROL||4.COMM||5.VERSION||6.EMV PARA||7.OTHER", "PRINT PARA", nSelect, &nSelect);
		nRet = PubShowMenuItems(tr("PRINT PARA"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}

		switch(nSelcItem)
		{
		case 1:
			nType=PRINT_PARAM_TYPE_MERCHANTINFO;
			break;
		case 2:
			nType=PRINT_PARAM_TYPE_TRANSCCTRL;	
			break;
		case 3:
			nType=PRINT_PARAM_TYPE_SYSTEMCTRL;
			break;
		case 4:
			nType=PRINT_PARAM_TYPE_COMM;
			break;
		case 5:
			nType=PRINT_PARAM_TYPE_VERSION;
			break;
		case 6:
			nType=PRINT_PARAM_TYPE_EMV;
			break;
		case 7:
			nType=PRINT_PARAM_TYPE_OTHER;
			break;
		default :                         
			continue;
		}

		ASSERT_FAIL(PubPrintCommit(_printParam, (void *)&nType, 1));
	}
	return APP_SUCC;
}


/**
* @brief Deal print limit. (if power is low, the printer can not work)
* @param void
* @return
* @li APP_SUCC 
* @li APP_FAIL
* @li APP_QUIT
* @author 
* @date
*/
int DealPrintLimit(void)
{
	ST_POWER_INFO stPowerInfo;
	char szVer[64]={0};
	int nStatus = -1;

	NDK_Getlibver(szVer);
	
	nStatus = PubGetPrintStatus();
PubDebug("==== Status: %d", nStatus);
	if (PRN_STATUS_VOLERR == nStatus)
	{
		PubMsgDlg("Warning", "     LOW BATTERY\n     PLEASE CHARGE", 3, 10);
		return APP_QUIT;
	}
	
	if ((strcmp(szVer, "4.4.7") >= 0) && szVer[0] != 'V')
	{
		memset(&stPowerInfo, 0, sizeof(ST_POWER_INFO));
		NDK_SysGetPowerInfo(&stPowerInfo);
		if (stPowerInfo.unPowerType & (1<<0))
			return APP_SUCC;
		if ((stPowerInfo.unPowerType & (1<<1)) && stPowerInfo.unBattryPercent <= 10)
		{
			PubMsgDlg("Warning", "LOW BATTERY\nPLS CHARGE", 3, 10);
			return APP_QUIT;
		}
	}
	
	return APP_SUCC;
}



/**
*	Print transaction switch
* @param TransFlag 支持的交易标志串
* @return void
*/
static void PntTraditionOnoff(const char *szTransFlag)
{
	int i;
	int nTransNum=TRANSSWITCHNUM;		
	char lszName[TRANSSWITCHNUM][20]= {"","","PREAUTH","REFUND","INSTALLMENT","CASH ADVANCE", "OFFLINE","CASHBACK",
		"BALANCE","","","","","ORS"};//"VOID REFUND","VOID OFFLINE" ,"VOID INSTALLMENT", "VOID CASH ADVANCE"
				
	for(i=0;i<nTransNum;i++)
	{	
		if (lszName[i][0]==0)
			continue;
		if (YES == GetTransSwitchOnoff(i))
		{
			PubPrePrinter("%s: YES\r", lszName[i]);
		}
		else
		{
			PubPrePrinter("%s: NO\r", lszName[i]);
		}

	}
}

static int PntHex(const char *pszTitle, const uchar *psContent, const int nConternLen, char cIsNeedBlank)
{
	int i;
	
	ASSERT_FAIL(PubPrePrinter(pszTitle));
	for (i = 0; i < nConternLen; i++)
	{
		if (YES == cIsNeedBlank)
		{
			ASSERT_FAIL(PubPrePrinter("%02X ", psContent[i]));
		}
		else
		{
			ASSERT_FAIL(PubPrePrinter("%02X", psContent[i]));
		}
	}
	ASSERT_FAIL(PubPrePrinter("\r"));
	return APP_SUCC;		
}

static void GetKernelId(char *psAID, char *psKernelID)
{
	if (memcmp(psAID,"\xa0\x00\x00\x00\x03",5) == 0)
	{
		memcpy(psKernelID, "\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	}
	if (memcmp(psAID,"\xa0\x00\x00\x00\x04",5) == 0)
	{
		memcpy(psKernelID, "\x02\x00\x00\x00\x00\x00\x00\x00", 8);
	}
	if (memcmp(psAID,"\xa0\x00\x00\x00\x65",5) == 0)
	{
		memcpy(psKernelID, "\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	}
	if (memcmp(psAID,"\xa0\x00\x00\x00\x03",5) == 0)
	{
		memcpy(psKernelID, "\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	}
	if (memcmp(psAID,"\xa0\x00\x00\x00\x03",5) == 0)
	{
		memcpy(psKernelID, "\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	}
}

int  _printemvparam()
{
	int nRet;
	int i;
	char sAid[20][17];
	char sCapk[50][6];
	STCLSSPARAM stClssEPParam;
	capk stCAPK;
	int nRes;
	uint nNum;

	PubClearAll();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "PRINTING");
	PubUpdateWindow();
	
	nRet = SDK_EP_EnumAID(sAid);
	if (nRet > 0)
	{
		ASSERT_FAIL(PubPrePrinter("AID_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{	
			memset(&stClssEPParam, 0, sizeof(STCLSSPARAM));
			stClssEPParam.ucAidLen = sAid[i][0];
			memcpy(stClssEPParam.usAid, &sAid[i][1], sAid[i][0]);
			nRes = SDK_EP_OperAID(&stClssEPParam,AID_GET);
			if (nRes == APP_SUCC)
			{
				ASSERT_FAIL(PubPrePrinter("---------------------\r"));
				PntHex("AID:", stClssEPParam.usAid, stClssEPParam.ucAidLen, YES);
				PntHex("VER:", stClssEPParam.usAppVer, 2, YES);
				PntHex("TAC_Default:", stClssEPParam.usTacDefault, 5, YES);
				PntHex("TAC_Decline:", stClssEPParam.usTacDenial, 5, YES);
				PntHex("TAC_Online :", stClssEPParam.usTacOnline, 5, YES);
				PubC4ToInt(&nNum, (uchar *)stClssEPParam.usFloorlimit);
				ASSERT_FAIL(PubPrePrinter("FloorLimit:%d\r", nNum));
				PubC4ToInt(&nNum, (uchar *)stClssEPParam.usThresholdValue);
				ASSERT_FAIL(PubPrePrinter("ThresholdValue:%d\r", nNum));
				PntHex("MaxTargetPercent:", &stClssEPParam.ucMaxTargetPercent, 1, YES);
				PntHex("TargetPercent:", &stClssEPParam.ucTargetPercent, 1, YES);
				PntHex("Default DDOL:", stClssEPParam.usDefaultDdol, stClssEPParam.ucDefaultDdolLen, YES);
				PntHex("Default TDOL:", stClssEPParam.usDefaultTdol, stClssEPParam.ucDefaultTdolLen, YES);
				PntHex("CAP:", stClssEPParam.usCap, 3, YES);
				PntHex("ADDCAP:", stClssEPParam.usAddCap, 5, YES);
				PntHex("ASI(PartialAID):", &stClssEPParam.ucAppSelIndicator, 1, YES);
				PntHex("ICS:", stClssEPParam.usIcs, 7, YES);
				PntHex("TestType:", &stClssEPParam.ucStatus, 1, YES);
				PntHex("SUPPEC:", &stClssEPParam.ucEcIndicator, 1, YES);
				PntHex("_limit_exist:", &stClssEPParam.ucLimitExist, 1, YES);
				PntHex("_ec_limit:", stClssEPParam.usEcLimit, 6, YES);
				PntHex("_cl_limit:", stClssEPParam.usClLimit, 6, YES);
				PntHex("_cl_offline_limit:", stClssEPParam.usClOfflineLimit, 6, YES);
				PntHex("_cvm_limit:", stClssEPParam.usCvmLimit, 6, YES);
				ASSERT_FAIL(PubPrePrinter("---------------------\r"));
			}

		}
	}
	else
	{
		ASSERT_FAIL(PubPrePrinter("EMV_EnumAID=%d\r", nRet));
	}
	if (APP_SUCC != PubConfirmDlg("EMV PARAM", "ENTER TO CONTINUE?", 0, 60))
	{
		ASSERT_FAIL(PubPrePrinter("\r\r\r" ));
		ASSERT_FAIL(PubPrePrinter("- - - - - - - X - - - - - - - X - - - - - - - "));
		return APP_SUCC;
	}
	nRet = SDK_EP_EnumCAPK(0, 49, sCapk);
	if (nRet > 0)
	{
		ASSERT_FAIL(PubPrePrinter("CAPK_Num:%d\r", nRet));
		for (i = 0; i < nRet; i++)
		{
			memcpy(stCAPK._rid, &sCapk[i][0], 5);
			stCAPK._index = sCapk[i][5];
			nRes = SDK_EP_OperCAPK(&stCAPK, CAPK_GET);
			if (nRes == APP_SUCC)
			{
				ASSERT_FAIL(PubPrePrinter("Index:%02x ", stCAPK._index));
				PntHex("RID:", stCAPK._rid, 5, YES);
				PntHex("   ExpDate:", stCAPK._expired_date, 4, YES);
			}
		}
	}
	else
	{
		ASSERT_FAIL(PubPrePrinter("EMV_EnumCAPK=%d\r", nRet));
	}

	ASSERT_FAIL(PubPrePrinter("\r\r\r" ));
	ASSERT_FAIL(PubPrePrinter("- - - - - - - X - - - - - - - X - - - - - - - "));
	return APP_SUCC;
}

static int PrintBillHead(int nAcqIndex)
{
	char szShopname[40+1] = {0};
	char szShopAddr[80+1] = {0};
	char szShopid[15+1] = {0};
	char szTerminalid[8+1] = {0};
	char szTmpStr[80+1] = {0};
	int i = 0;
	
	if (NO == GetIsPntTitleMode())//'0' logo
	{
		//打印logo
		NDK_PrnPicture(25, "PrintLogo.bmp");
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("\r");
	}
	else
	{
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_HEIGHT_DOUBLE, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		GetFunctionPntTitle(szTmpStr);
		if (0 == strlen(szTmpStr))
		{
			strcpy(szTmpStr, "POS RECEIPT");
		}
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szTmpStr, 3);
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	}

	GetVarMerchantNameEn(szShopname);
	//PubPrePrinter("%s\r", szShopname);
	ProTrimStr(szShopname);
	PrePrintInMiddle(PRN_ZM_FONT_12x24A, szShopname, 1);

	for (i = 0; i < 4; i++)
	{
		memset(szShopAddr, 0, sizeof(szShopAddr));
		GetVarMerchantAddr(szShopAddr, i);
		ProTrimStr(szShopAddr);
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szShopAddr, 1);
	}

	PubPrePrinter("\r");

	if (0xff != nAcqIndex)
	{
		GetVarTerminalId(szTerminalid, nAcqIndex);
		PubPrePrinter("TID: %s\r", szTerminalid);
		GetVarMerchantId(szShopid, nAcqIndex);
		PubPrePrinter("MID: %s\r\r", szShopid);
	}

	return APP_SUCC;

}

#if 0
static void ShieldPanNo(char *pszPan)
{
	int i = 0;
	int nLen = strlen(pszPan);

	if (nLen <= 4)
		return;

	for(i = 0; i<(nLen-4);i++)//SHOW LAST 4bytes
	{
		pszPan[i] = '*';
	}
}
#endif

static int PrintAmt(const STTAANSRECORD *pstTransRecord)
{
	char szCurrencyName[3+1] = {0};
	char szStr[128] = {0}, szTmpStr[128] = {0}, szTmp[256] = {0};
	char szDispAmt[15] = {0}, szDispTotal[15] = {0};
	
	GetVarCurrencyName(szCurrencyName);

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	memset(szStr, 0, sizeof(szStr));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szStr);
	memset(szTmpStr, 0, sizeof(szTmpStr));
	if(1 == pstTransRecord->cFlagCashback)
	{
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0,  (uchar *)szTmpStr);
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0,  (uchar *)szTmpStr);
	}
	if(1 == pstTransRecord->cFlagCashback)
	{		
		char szTotal[13] = {0};

		//金额
		ProAmtToDispOrPnt((char*)szStr, szDispAmt);
		PubAllTrim(szDispAmt);
		CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
		ProPrePrintStrFormat("AMOUNT:", szTmp, ' ', 1);

		//cashback
		ProAmtToDispOrPnt((char*)szTmpStr, szDispAmt);
		PubAllTrim(szDispAmt);
		CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
		ProPrePrintStrFormat("CASHBACK:", szTmp, ' ', 1);

		//total
		PubPrePrinter("------------------------\r");
		AmtAddAmt((uchar *)szStr, (uchar *)szTmpStr, (uchar *)szTotal);
		ProAmtToDispOrPnt((char*)szTotal, szDispTotal);
		PubAllTrim(szDispTotal);
		sprintf(szTmp, "%s %s", szCurrencyName, szDispTotal);
		ProPrePrintStrFormat("TOTAL:", szTmp, ' ', 1);
	}
	else
	{
		if (1 == pstTransRecord->cTipFlag)
		{
			char szAmt[13] = {0};
			
			//金额
			AmtSubAmt((uchar *)szStr, (uchar *)szTmpStr, (uchar *)szAmt);
			ProAmtToDispOrPnt((char*)szAmt, szDispAmt);
			PubAllTrim(szDispAmt);
			CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
			sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
			ProPrePrintStrFormat("AMOUNT:", szTmp, ' ', 1);

			//tip
			ProAmtToDispOrPnt((char*)szTmpStr, szDispAmt);
			PubAllTrim(szDispAmt);
			CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
			sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
			ProPrePrintStrFormat("TIPS:", szTmp, ' ', 1);

			//total
			PubPrePrinter("------------------------\r");
			ProAmtToDispOrPnt((char*)szStr, szDispTotal);
			PubAllTrim(szDispTotal);
			sprintf(szTmp, "%s %s", szCurrencyName, szDispTotal);
			ProPrePrintStrFormat("TOTAL:", szTmp, ' ', 1);
		}
		else
		{
			ProAmtToDispOrPnt((char*)szStr, szDispAmt);
			PubAllTrim(szDispAmt);
			CheckAmtMinus(pstTransRecord->cTransType, szDispAmt);
			sprintf(szTmp, "%s %s", szCurrencyName, szDispAmt);
			ProPrePrintStrFormat("AMOUNT:", szTmp, ' ', 1);
		}
	}
	
	return APP_SUCC;
}


static int GetPrintAmt(const STTAANSRECORD *pstTransRecord, char *pszDispAmt)
{
	char szStr[128] = {0}, szTmpStr[128] = {0};
	
	memset(szStr, 0, sizeof(szStr));
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szStr);
	memset(szTmpStr, 0, sizeof(szTmpStr));
	if(1 == pstTransRecord->cFlagCashback)
	{
		PubHexToAsc((uchar *)pstTransRecord->sCashbackAmount, 12, 0,  (uchar *)szTmpStr);
	}
	else
	{
		PubHexToAsc((uchar *)pstTransRecord->sTipAmount, 12, 0,  (uchar *)szTmpStr);
	}
	
	if(1 == pstTransRecord->cFlagCashback)
	{		
		char szTotal[13] = {0};

		//total
		AmtAddAmt((uchar *)szStr, (uchar *)szTmpStr, (uchar *)szTotal);
		ProAmtToDispOrPnt((char*)szTotal, pszDispAmt);
		PubAllTrim(pszDispAmt);
	}
	else
	{
		ProAmtToDispOrPnt((char*)szStr, pszDispAmt);
		PubAllTrim(pszDispAmt);
	}
	return APP_SUCC;
}

static void CheckAmtMinus(char cTransType, char *pszAmount)
{		
	char szAmt[20] = {0};
	
	if(GetVarIsPrintPrintMinus()==YES)
	{
		switch(cTransType)
		{
		case TRANS_VOID:				
		case TRANS_VOID_AUTHSALE: 			
		case TRANS_VOID_PREAUTH:			
		case TRANS_REFUND:  			
		case TRANS_VOID_REFUND:  			
		case TRANS_VOID_OFFLINE:  			
		case TRANS_VOID_INSTALLMENT:  			
		case TRANS_VOID_CASH_ADVANCE:  			
			sprintf(szAmt, "-%s", pszAmount);
			strcpy(pszAmount, szAmt);
			break;
		default:
			break;
		}
	}
}
static void ProPrePrintStrFormat(char *pszStrA, char *pszStrB, char ch, int nLineFeed)
{
	char szOut[128+1] = {0};
	int nLen = 24;//default big font
	char szTmp[128] = {0};
	char szLineFeed[64] = {0};

	memset(szTmp, '\r', sizeof(szTmp));
	memcpy(szLineFeed, szTmp, nLineFeed);
	ProAddSymbolBtnStrs(pszStrA, pszStrB, szOut, nLen, ch);
	PubPrePrinter("%s%s", szOut, szLineFeed);
}

static int PrintInstallment(const char *pszInstallmentInfo)
{
	char  szTemp[128];
	char szDispAmt[20];
	char szFee[13];
	int nPos = 0;
	
	PubPrePrinter("\r");
//	PubPrePrinter("Installment Plan:%s\r",szTemp);
	nPos+=3;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszInstallmentInfo + nPos,2);
	PubPrePrinter("TERM     : %s Months\r",szTemp);
	nPos+=2;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszInstallmentInfo + nPos,1);
//	PubPrePrinter("Compute Method:%s\n",szTemp);
	nPos+=1;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszInstallmentInfo + nPos,6);
	strcpy(szFee,szTemp);
//	PubPrePrinter("INTEREST : 0.00%\r");
	PubPrePrinter("INTEREST : %1.1s.%5.5s%\n",szTemp, szTemp+1);
	nPos+=6;

//	prnPrintf("Interest Free Months:%s\n",szTemp);
	nPos+=2;

//	prnPrintf("First Payment Amount:%s\n",prnAmt);
	nPos+=9;

	memset(szTemp,0,sizeof(szTemp));
	memset(szDispAmt,0,sizeof(szDispAmt));
	memcpy(szTemp,pszInstallmentInfo + nPos,9);
	ProAmtToDispOrPnt((char*)szTemp, szDispAmt);
	PubAllTrim(szDispAmt);
	PubPrePrinter("LAST AMT : RM %s\n",szDispAmt);
	nPos+=9;

	memset(szTemp,0,sizeof(szTemp));
	memset(szDispAmt,0,sizeof(szDispAmt));
	memcpy(szTemp,pszInstallmentInfo + nPos,9);
	ProAmtToDispOrPnt((char*)szTemp, szDispAmt);
	PubAllTrim(szDispAmt);
	PubPrePrinter("MNTH AMT : RM %s\n",szDispAmt);
	nPos+=9;

//	PubPrePrinter("Total Installment Amount:%s\n",prnAmt);
	nPos+=9;
	
//	PubPrePrinter("Outstanding Principal:%s\n",prnAmt);
	nPos+=9;

//	PubPrePrinter("Outstanding Interest:%s\n",prnAmt);
	nPos+=9;

//	PubPrePrinter("FEES : %s\n",prnAmt);
	nPos+=9;

//	PubPrePrinter("Waive from Months:%s\n",szTemp);
	nPos+=2;

	memset(szTemp,0,sizeof(szTemp));
	memcpy(szTemp,pszInstallmentInfo + nPos,2);
//	PubPrePrinter("Waive to Months:%s\n",szTemp);

	memset(szDispAmt,0,sizeof(szDispAmt));
	ProAmtToDispOrPnt((char*)szFee, szDispAmt);
	PubAllTrim(szDispAmt);
	PubPrePrinter("FEES     : RM %s\n", szDispAmt);
	return APP_SUCC;
}

static int _printBalance(const STTAANSRECORD *pstTransRecord)
{
	char szTmpStr[80+1] = {0}, szStr[256];
	char szPosYear[4+1];	
	int nCurrentPage = 1;

	if (NO == GetIsPntTitleMode())//'0' logo
	{
		//打印logo
		NDK_PrnPicture(25, "PrintLogo.bmp");
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("\r");
	}
	else
	{
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_HEIGHT_DOUBLE, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		GetFunctionPntTitle(szTmpStr);
		if (0 == strlen(szTmpStr))
		{
			strcpy(szTmpStr, "POS RECEIPT");
		}
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, szTmpStr, 3);
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	}
	
	//Date time
	memset(szPosYear, 0, sizeof(szPosYear));
	PubGetCurrentDatetime(szTmpStr);
	memcpy(szPosYear, szTmpStr, 4);
	PubPrePrinter("DATE:%.4s/%02x/%02x TIME:%02x:%02x:%02x\r\r", szPosYear, pstTransRecord->sDate[0], pstTransRecord->sDate[1], pstTransRecord->sTime[0], pstTransRecord->sTime[1], pstTransRecord->sTime[2]);
	PubPrePrinter("TID: %s\r", pstTransRecord->szTID);
	PubPrePrinter("MID: %s\r\r\r", pstTransRecord->szMID);

	//Trans type
	GetTransNameForTP( pstTransRecord->cTransType, szStr, pstTransRecord->cFlagCashback, pstTransRecord->cEMV_Status);
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	PrePrintInMiddle(PRN_ZM_FONT_16x32, szStr, 1);
	
	//************* CARD NO *************
	PrintCardNo(pstTransRecord, nCurrentPage);

	//AMT
	PubHexToAsc((uchar *)pstTransRecord->sAmount, 12, 0,  (uchar *)szStr);
	ProAmtToDispOrPnt((char*)szStr, szTmpStr);
	PubAllTrim(szTmpStr);
	PubPrePrinter("RM :%s\r\r\r", szTmpStr);
	
	return APP_SUCC;
}


int PrintBalance(const STTAANSRECORD *pstTransRecord)
{
	ASSERT_FAIL(PubPrintCommit( _printBalance, (void *)pstTransRecord, 1));

	return APP_SUCC;
}

static void GetCardTypeStr(int nCardType, char *pszCardType)
{
	switch (nCardType)
	{
	case EM_ACQTYPE_AMEX:
		strcpy(pszCardType, "AMEX");
		break;
	case EM_ACQTYPE_VISA:
		strcpy(pszCardType, "VISA");
		break;
	case EM_ACQTYPE_MASTER:
		strcpy(pszCardType, "MASTER");
		break;
	case EM_ACQTYPE_JCB:
		strcpy(pszCardType, "JCB");
		break;
	case EM_ACQTYPE_UPI:
		strcpy(pszCardType, "UnionPay");
		break;
	case EM_ACQTYPE_MCCS:
		strcpy(pszCardType, "MYDEBIT");
		break;
	default:
		break;
	}
}


static void GetCardHost(int nCardIndex, char *pszCardHost)
{
	switch (nCardIndex)
	{
	case EM_ACQINDEX_AMEX:
		strcpy(pszCardHost, "AMEX");
		break;
	case EM_ACQINDEX_V_M_J:
		strcpy(pszCardHost, "MBB");
		break;
	case EM_ACQINDEX_UPI:
		strcpy(pszCardHost, "UPI");
		break;
	case EM_ACQINDEX_MCCS:
		strcpy(pszCardHost, "MYDEBIT");
		break;
	case 4:
		strcpy(pszCardHost, "MBB(6 MONTH)");
		break;
	case 5:
		strcpy(pszCardHost, "MBB(12 MONTH)");
		break;
	case 6:
		strcpy(pszCardHost, "MBB(18 MONTH)");
		break;
	case 7:
		strcpy(pszCardHost, "MBB(24 MONTH)");
		break;
	case 8:
		strcpy(pszCardHost, "AMEX(6 MONTH)");
		break;
	case 9:
		strcpy(pszCardHost, "AMEX(12 MONTH)");
		break;
	case 10:
		strcpy(pszCardHost, "AMEX(18 MONTH)");
		break;
	case 11:
		strcpy(pszCardHost, "AMEX(24 MONTH)");
		break;
	default:
		break;
	}
}



static void PrintCardNo(const STTAANSRECORD *pstTransRecord, int nCurrentPage)
{
	char szInputMode[10] = {0};
	char szTmpStr[256] = {0}, szTmp[256] = {0};
	char szCardType[32] = {0};
	
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	//Card no and input mode
	GetInputMode(pstTransRecord->cTransAttr, szInputMode);
	memset(szTmpStr, 0, sizeof(szTmpStr));
	PubHexToAsc((uchar *)pstTransRecord->sPan, pstTransRecord->nPanLen, 0, (uchar *)szTmpStr);

	//CtrlCardPan((uchar*)szTmpStr, (uint)pstTransRecord->nPanLen, (uint)pstTransRecord->cTransType, pstTransRecord->cEMV_Status);
	if (pstTransRecord->cTransType == TRANS_SALE && 
		(pstTransRecord->cEMV_Status == EMV_STATUS_OFFLINE_SUCC || pstTransRecord->cEMV_Status == EMV_STATUS_OFFLINE_FAIL))
	{
		;
	}
	else
	{
		MaskCardNo(szTmpStr, nCurrentPage);
	}
	
	//show card type
	GetCardTypeStr(pstTransRecord->nAcqType, szCardType);
	if (strlen(szCardType) > 0)
	{
		PubPrePrinter("%s\r", szCardType);
	}
	else
	{
		PubPrePrinter("CARD  NO:\r");
	}
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_HEIGHT_DOUBLE, 0));
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(24, PRN_MODE_HEIGHT_DOUBLE, 0));
	sprintf(szTmp, "%s %s", szTmpStr, szInputMode);
	PrePrintInMiddle(PRN_ZM_FONT_16x32, szTmp, 2);

}


static int _printEmvLog(const char *psLog)
{
	STEMVADDTIONWATERINFO *pstEMVAddtionWaterInfo = (STEMVADDTIONWATERINFO *)psLog;

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_8x16, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	
	PubPrePrinter("AID_LABLE [50] [10]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->szAIDLable, 10, YES);
	PubPrePrinter("EQ_track2 [57] [%02x]\r", pstEMVAddtionWaterInfo->sEqTrack2_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sEqTrack2_wl+1, pstEMVAddtionWaterInfo->sEqTrack2_wl[0], YES);
	PubPrePrinter("TransCurrentCode [5F2A] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTransCurrentCode, 2, YES);
	PubPrePrinter("PanSeqNum [5F34] [01]\r");
	PntHex("", (uchar *)&pstEMVAddtionWaterInfo->cPanSeqNum, 1, YES);	
	PubPrePrinter("ResponseCode [8A] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sResponseCode, 2, YES);	
	PubPrePrinter("AIP [82] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sAIP, 2, YES);	
	PubPrePrinter("PubKeyIndex [8F] [01]\r");
	PntHex("", (uchar *)&pstEMVAddtionWaterInfo->cPubKeyIndex, 1, YES);	
	PubPrePrinter("IssAuthData [91] [%02x]\r", pstEMVAddtionWaterInfo->sIssAuthData_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sIssAuthData_wl+1, pstEMVAddtionWaterInfo->sIssAuthData_wl[0], YES);
	PubPrePrinter("TVR [95] [05]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTVR, 5, YES);	
	PubPrePrinter("TSI [9B] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTSI, 2, YES);	
	PubPrePrinter("TxnData [9A] [03]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTxnData, 3, YES);	
	PubPrePrinter("TxnType [9C] [01]\r");
	PntHex("", (uchar *)&pstEMVAddtionWaterInfo->cTxnType, 1, YES);	
	PubPrePrinter("AuthAmt [9F02] [06]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sAuthAmt, 6, YES);	
	PubPrePrinter("OtherAmt [9F03] [06]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sOtherAmt, 6, YES);	
	PubPrePrinter("IACDefault [9F0D] [05]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sIACDefault, 5, YES);	
	PubPrePrinter("IACDenial [9F0E] [05]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sIACDenial, 5, YES);	
	PubPrePrinter("IACOnline [9F0F] [05]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sIACOnline, 5, YES);	
	PubPrePrinter("IAD [9F10] [%02x]\r", pstEMVAddtionWaterInfo->sIAD_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sIAD_wl+1, pstEMVAddtionWaterInfo->sIAD_wl[0], YES);
	PubPrePrinter("IssCodeTbl [9F11] [01]\r");
	PntHex("", (uchar *)&pstEMVAddtionWaterInfo->cIssCodeTbl, 1, YES);	
	PubPrePrinter("AppPreferedName [9F12] [%02x]\r", pstEMVAddtionWaterInfo->szAppPreferedName_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->szAppPreferedName_wl+1, pstEMVAddtionWaterInfo->szAppPreferedName_wl[0], YES);
	PubPrePrinter("MerchantCatCode [9F15] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sMerchantCatCode, 2, YES);	
	PubPrePrinter("TerminalCountryCode [9F1A] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTerminalCountryCode, 2, YES);	
	PubPrePrinter("Track2DscData [9F20] [%02x]\r", pstEMVAddtionWaterInfo->sTrack2DscData_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTrack2DscData_wl+1, pstEMVAddtionWaterInfo->sTrack2DscData_wl[0], YES);
	PubPrePrinter("AppCryptogram [9F26] [08]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sAppCryptogram, 8, YES);	
	PubPrePrinter("CryptInfoData [9F27] [08]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sCryptInfoData, 8, YES);	
	PubPrePrinter("TermCap [9F33] [03]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sTermCap, 3, YES);	
	PubPrePrinter("CVM [9F34] [03]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sCVM, 3, YES);	
	PubPrePrinter("ATC [9F36] [02]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sATC, 3, YES);	
	PubPrePrinter("ATC [9F37] [04]\r");
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->sUnpNum, 4, YES);	
	PubPrePrinter("IssuerScriptResult1 [9F5B] [%02x]\r", pstEMVAddtionWaterInfo->szIssuerScriptResult1_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->szIssuerScriptResult1_wl+1, pstEMVAddtionWaterInfo->szIssuerScriptResult1_wl[0], YES);
	PubPrePrinter("IssuerScriptResult2 [71] [%02x]\r", pstEMVAddtionWaterInfo->szIssuerScriptResult2_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->szIssuerScriptResult2_wl+1, pstEMVAddtionWaterInfo->szIssuerScriptResult2_wl[0], YES);
	PubPrePrinter("IssuerScriptResult3 [73] [%02x]\r", pstEMVAddtionWaterInfo->szIssuerScriptResult3_wl[0]);
	PntHex("", (uchar *)pstEMVAddtionWaterInfo->szIssuerScriptResult3_wl+1, pstEMVAddtionWaterInfo->szIssuerScriptResult3_wl[0], YES);
	
	return APP_SUCC;
}

int PrintEmvLog()
{
	char sBuf[1024] = {0};
	int nLen = 0;

	nLen = sizeof(STEMVADDTIONWATERINFO);
	GetFromFile(sBuf, 0, &nLen, FILE_ICCLOG);

	if (nLen == 0)
	{
		PubMsgDlg("Warning", "No data", 0, 1);
		return APP_QUIT;
	}
	ASSERT_FAIL(PubPrintCommit( _printEmvLog, (void *)sBuf, 1));
	
	return APP_SUCC;
}

void PrintOrsValueItem(const char *psContent, const char *psBcdValue, char cIsAmt)
{
	char szDisp[14] = {0};
	if (YES == cIsAmt)
	{
		ProHexAmtToDIsp(psBcdValue, szDisp);
	}
	else
	{
		ProHexPntToDIsp(psBcdValue, szDisp);
	}
	
	PubPrePrinter("%s: %s\r", psContent, szDisp);
}

static int PrintOrsPoint(char cTransType, STORS_INFO stOrsInfo)
{	
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
	switch (cTransType)
	{
	case TRANS_ORS_POINTS_INQUIRY:
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);
		PrintOrsValueItem("VALUE(RM)", stOrsInfo.sSuppPointRedeem, YES);
		break;
	case TRANS_ORS_INSTANT_REWARD:
	case TRANS_ORS_VOID_INSTANT_REWARD:
		PrintOrsValueItem("PRI PREV POINT BAL", stOrsInfo.sPriPointBal_Prev, NO);
		PrintOrsValueItem("PRI POINT REDEEMED", stOrsInfo.sPrePointRedeem, NO);
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);

		PrintOrsValueItem("SUPP PREV POINT BAL", stOrsInfo.sSuppPointBal_Prev, NO);
		PrintOrsValueItem("SUPP POINT REDEEMED", stOrsInfo.sSuppPointRedeem, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		
		PrintOrsValueItem("TTL PREV POINT BAL", stOrsInfo.sTtlPointBal_Prev, NO);
		PrintOrsValueItem("TTL POINT REDEEMED", stOrsInfo.sTtlPointRedeem, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);
		
		PrintOrsValueItem("PAID AMT(RM)", stOrsInfo.sPayAmt, YES);
		break;
	case TRANS_ORS_POINTS_RDMPT:
	case TRANS_ORS_VOID_POINTS_RDMPT:
		PrintOrsValueItem("PRI PREV POINT BAL", stOrsInfo.sPriPointBal_Prev, NO);
		PrintOrsValueItem("PRI POINT REDEEMED", stOrsInfo.sPrePointRedeem, NO);
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);

		PrintOrsValueItem("SUPP PREV POINT BAL", stOrsInfo.sSuppPointBal_Prev, NO);
		PrintOrsValueItem("SUPP POINT REDEEMED", stOrsInfo.sSuppPointRedeem, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		
		PrintOrsValueItem("TTL PREV POINT BAL", stOrsInfo.sTtlPointBal_Prev, NO);
		PrintOrsValueItem("TTL POINT REDEEMED", stOrsInfo.sTtlPointRedeem, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);
		break;
	case TRANS_ORS_VALUE_RDMPT:
	case TRANS_ORS_VOID_VALUE_RDMPT:
		PrintOrsValueItem("PRI PREV POINT BAL", stOrsInfo.sPriPointBal_Prev, NO);
		PrintOrsValueItem("PRI POINT REDEEMED", stOrsInfo.sPrePointRedeem, NO);
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);

		PrintOrsValueItem("SUPP PREV POINT BAL", stOrsInfo.sSuppPointBal_Prev, NO);
		PrintOrsValueItem("SUPP POINT REDEEMED", stOrsInfo.sSuppPointRedeem, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		
		PrintOrsValueItem("TTL PREV POINT BAL", stOrsInfo.sTtlPointBal_Prev, NO);
		PrintOrsValueItem("TTL POINT REDEEMED", stOrsInfo.sTtlPointRedeem, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);

		PrintOrsValueItem("REDEEM VALUE (RM)", stOrsInfo.sPayAmt, YES);
		break;
	case TRANS_ORS_GIFTCODE_RDMPT:
	case TRANS_ORS_VOID_GIFTCODE_RDMPT:
		PrintOrsValueItem("PRI PREV POINT BAL", stOrsInfo.sPriPointBal_Prev, NO);
		PrintOrsValueItem("PRI POINT REDEEMED", stOrsInfo.sPrePointRedeem, NO);
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);

		PrintOrsValueItem("SUPP PREV POINT BAL", stOrsInfo.sSuppPointBal_Prev, NO);
		PrintOrsValueItem("SUPP POINT REDEEMED", stOrsInfo.sSuppPointRedeem, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		
		PrintOrsValueItem("TTL PREV POINT BAL", stOrsInfo.sTtlPointBal_Prev, NO);
		PrintOrsValueItem("TTL POINT REDEEMED", stOrsInfo.sTtlPointRedeem, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);
		
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(24, PRN_MODE_HEIGHT_DOUBLE, 0));
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "GIFT CODE INFO", 2);
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("GIFT CODE: %s\r", stOrsInfo.szGiftCode);
		PrintOrsValueItem("POINTS PER UNIT", stOrsInfo.sPointPerGiftCode, NO);
		PubPrePrinter("QTY: %x%x\r", stOrsInfo.sQuantity[0], stOrsInfo.sQuantity[1]);
		break;
	case TRANS_ORS_HOTDEAL_RDMPT:
	case TRANS_ORS_VOID_HOTDEAL_RDMPT:
		PrintOrsValueItem("PRI PREV POINT BAL", stOrsInfo.sPriPointBal_Prev, NO);
		PrintOrsValueItem("PRI POINT REDEEMED", stOrsInfo.sPrePointRedeem, NO);
		PrintOrsValueItem("PRI POINT BAL", stOrsInfo.sPriPointBal, NO);

		PrintOrsValueItem("SUPP PREV POINT BAL", stOrsInfo.sSuppPointBal_Prev, NO);
		PrintOrsValueItem("SUPP POINT REDEEMED", stOrsInfo.sSuppPointRedeem, NO);
		PrintOrsValueItem("SUPP POINT BAL", stOrsInfo.sSuppPointBal, NO);
		
		PrintOrsValueItem("TTL PREV POINT BAL", stOrsInfo.sTtlPointBal_Prev, NO);
		PrintOrsValueItem("TTL POINT REDEEMED", stOrsInfo.sTtlPointRedeem, NO);
		PrintOrsValueItem("TTL POINT BAL", stOrsInfo.sTtlPointBal, NO);
		
		PrintOrsValueItem("PAID AMT(RM)", stOrsInfo.sPayAmt, YES);
		
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_16x32, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(24, PRN_MODE_HEIGHT_DOUBLE, 0));
		PrePrintInMiddle(PRN_ZM_FONT_12x24A, "GIFT CODE INFO", 2);
		//ASSERT(PubSetPrintFont(PRN_ZM_FONT_12x24A, PRN_HZ_FONT_24x24A, PRN_MODE_NORMAL, 0));
		ASSERT(PubSetPrtFont(18, PRN_MODE_HEIGHT_DOUBLE, 0));
		PubPrePrinter("GIFT CODE: %s\r", stOrsInfo.szGiftCode);
		PrintOrsValueItem("POINTS PER UNIT", stOrsInfo.sPointPerGiftCode, NO);
		PrintOrsValueItem("AMT PER UNIT", stOrsInfo.sPayAmtPerGiftCode, NO);
		PubPrePrinter("QTY: %x%x\r", stOrsInfo.sQuantity[0], stOrsInfo.sQuantity[1]);
		break;	
	}
	return APP_SUCC;
}

/**
* @brief 根据当前参数信息设置打印内容
* @param in ptrPara  
* @return APP_SUCC
* @author
* @date 
*/
int _printIsoData(void *pvPara)
{
	int nFieldLen = 0;
	int nBitmapLen = 8;
	int n;
	int k;
	uchar cBitMask;
	char szField[999];
	char *psBitmap = (char *)pvPara+2;
	char szTpdu[10+1] = {0};
	int i = 0, j = 0;
	int t = 0, l = 0;
	char sMsgId[2] = {0};
	
	memcpy(sMsgId, pvPara, 2);
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_10x8, PRN_HZ_FONT_16x8, PRN_MODE_HEIGHT_DOUBLE, 0));	
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
//	PubPrePrinter("--------------------------\r");
	PubPrePrinter("MSGID: %02x%02x\r", sMsgId[0], sMsgId[1]);
	PubPrePrinter("BITMAP: ");
	for (j = 0; j < 8; j++)
	{
		PubPrePrinter("%02X", psBitmap[j]);
	}
	PubPrePrinter("\r");
	
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
			if (0 == (psBitmap[i] & cBitMask)) 
			{
				continue;
			}
			n = (i<<3) + j;
			nFieldLen = 999;
			memset(szField, 0, sizeof(szField));
			ASSERT_FAIL(GetField(n, szField, &nFieldLen));
			
			PubPrePrinter("[%d]", n);
			PubPrePrinter("%s", szField);
			PubPrePrinter("\r");
		}
	}
	return APP_SUCC;
}

/**
* @brief 提交打印ISO数据
* @param in pszData iso数据指针
* @return 
* @li APP_SUCC 
* @li APP_FAIL
* @author 
* @date 
*/
int PrintIsoData(const char *pszData)
{
 	ASSERT_QUIT(PubPrintCommit(_printIsoData, (void *)pszData, 1));		
 	return APP_SUCC; 
}


typedef struct
{
	int nLen;
	char sData[1024];
}STISORAW;

/**
* @brief 根据当前参数信息设置打印内容
* @param in ptrPara  
* @return APP_SUCC
* @author
* @date 
*/
int _printRawData(void *pvPara)
{
	STISORAW *pstData = pvPara;
	int i = 0;

	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_10x8, PRN_HZ_FONT_16x16, PRN_MODE_HEIGHT_DOUBLE, 0));	
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	for (i = 0; i < pstData->nLen; i++)
	{
		PubPrePrinter("%02X", pstData->sData[i]);
		if ((i+1)%13 == 0 && (i+1) < pstData->nLen)
		{
			//PubPrePrinter("\r");
		}
		else
		{
			PubPrePrinter(" ");
		}
	}
	PubPrePrinter("\r");
	return APP_SUCC;
}
/**
* @brief 提交打印ISO数据
* @param in pszData iso数据指针
* @return 
* @li APP_SUCC 
* @li APP_FAIL
* @author 
* @date 
*/
int PrintRawData(const char *pszData, int nLen)
{
	STISORAW stData;

	memset(&stData, 0, sizeof(STISORAW));
	stData.nLen = nLen;
	memcpy(stData.sData, pszData, nLen);
	
 	ASSERT_QUIT(PubPrintCommit(_printRawData, (void *)&stData, 1));		
 	return APP_SUCC; 
}

static int PrintAppVersion()
{
	char szPosType[24] = {0};
	char szSoftVer[24] = {4};
	
	//ASSERT(PubSetPrintFont(PRN_ZM_FONT_8x16, PRN_HZ_FONT_16x16, PRN_MODE_NORMAL, 0));
	ASSERT(PubSetPrtFont(12, PRN_MODE_HEIGHT_DOUBLE, 0));
	NDK_SysGetPosInfo(SYS_HWINFO_GET_POS_TYPE,NULL, szPosType);
	GetVarSoftVer(&szSoftVer);
	PubPrePrinter("%s %s\r\r", szPosType, szSoftVer);

	return APP_SUCC;
}
