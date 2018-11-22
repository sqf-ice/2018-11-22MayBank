/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  settle.c
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-24
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define MAX_AMOUNT "20000000000"
#define ONE_TRADE_MAX_AMOUNT "9999999999"/**<Max amount per transaction*/

/**
* define global variable
*/
static STSETTLE glstSettle[ACQUIRER_NUM];

static int DispSettleDataItem(char *, char *, int, char *);
static int ChangeSettleItem(const char *, SETTLE_NUM *, SETTLE_AMT *, int );
static int UpLoad(const int, const int, int *);

/**
* @brief Settlement
* @param in cFlag 0:Normal settle  nonzero:settle again after interupting settle 
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int Settle(char cFlag)
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
	int i = 0, nTotalBrand = 0;//ACQUIRER_NUM;

	memset(&stSystem, 0, sizeof(STSYSTEM));

	if (YES == GetVarIsSettlePwd())
	{
		ASSERT_QUIT(ProCheckPwd(pszTitle, EM_TRANS_PWD));
	}

	ASSERT_QUIT(SelectAcqIndex(pszTitle, &nAcqIndex, YES));
	
	if (nAcqIndex == 0xFF)
	{
		nTotalBrand = ACQUIRER_NUM;
	}
	else
	{
		
		nTotalBrand = 1;
	}
PubDebug("Total brand: %d", nTotalBrand);
	for (i = 0; i < nTotalBrand; i++)
	{
		if (nTotalBrand == 1)
		{
			stSystem.nAcqIndex = nAcqIndex;
		}
		else
		{
			stSystem.nAcqIndex = i;
		}
		
		if (APP_SUCC != CheckIsNeedSettle(stSystem.nAcqIndex))
		{
			continue;
		}
		
//		ASSERT_QUIT(DispTotal(stSystem.nAcqIndex));
		
		/*
		* Deal POS limit
		*/
		ASSERT_QUIT(DealPrintLimit()); 
		
		if (cFlag)
		{
			/*
			* Get halt step
			*/
			if (YES == GetVarBatchHaltFlag())
			{
				cProcStep = 2;
			}
			else if (YES == GetVarPrintSettleHalt())
			{
				cProcStep = 3;
			}
			else if (YES == GetVarPrintDetialHalt())
			{
				cProcStep = 4;
			}
			else if (YES == GetVarClrSettleDataFlag())
			{
				cProcStep = 5;
			}
		}
		else
		{
			cProcStep = 0;
			SetVarBatchMagOfflineHaltFlag(1);
			SetVarFinanceHaltFlag(1);
			SetVarMessageHaltFlag(1);		
			SetVarEmvOfflineUpNum(1);
			SetVarEmvOnlineUpNum(1);
			SetVarEMVOfflineFailHaltFlag(1);
			SetVarEMVOnlineARPCErrHaltFlag(1);
			SetVarBatchSum(0);
		}
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
#ifdef DEMO
#else
			ASSERT_HANGUP_QUIT(CommConnect());
#endif
			
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
#ifdef DEMO
			PubCommClear();
			
			PubClear2To4();
			PubDisplayStrInline(0, 3, "Sending...");
			PubUpdateWindow();
			NDK_SysDelay(5);
			PubClear2To4();
			PubDisplayStrInline(0, 3, "Receiving...");
			PubUpdateWindow();
			NDK_SysDelay(10);
#else		
			ASSERT_HANGUP_FAIL(CommSend(sPackBuf, nPackLen));
			ASSERT_HANGUP_FAIL(CommRecv(sPackBuf, &nPackLen));
			CommHangUpSocket();
			
			ASSERT_HANGUP_FAIL(Unpack(sPackBuf, nPackLen));
			ASSERT_HANGUP_FAIL(ChkRespMsgID("0500", sPackBuf));
			ASSERT_HANGUP_FAIL(ChkRespon(&stSystem, sPackBuf + 2));
#endif

			/*Check response*/
#ifdef DEMO
			SetVarIsTotalMatch(YES);
#else
			ASSERT_HANGUP_FAIL(CheckSettleReponse(stSystem));
#endif
			
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

TRACE(("-----i=%d", i));
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
			if (nRet != APP_SUCC)
			{
				CommHangUp();
				return APP_FAIL;
			}
	 		SetVarPrintSettleHalt(NO);
	 	}
TRACE(("-----i=%d", i));
		
#if 0 //170527 no need to print detail
		/**
		* print detail
		*/
		if (cProcStep <= 4)
		{
			SetVarPrintDetialHalt(YES);
			if (YES == GetVarIsPrintWaterRec())
			{
				int nSelect = 0;
				
				nRet = ProSelectList("0.NO||1.YES", "PRINT DETAIL?", 0xFF, &nSelect);
				if (nRet == APP_SUCC && nSelect == 1)
				{
					PubClear2To4();
					PubDisplayGen(2, "PRINT DETAIL...");
					PubUpdateWindow();
					nRet = PrintAllWater(stSystem.nAcqIndex);
					if (nRet == APP_FAIL)
					{
						return APP_FAIL;
					}
				}
			}

		}
#endif
TRACE(("-----i=%d", i));
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
	}
	
	CommHangUp();

	return APP_SUCC;
}

/**
* @brief Get settle data
* @param in pstSettle
* @return 
* @li APP_SUCC
*/
int GetSettleData(STSETTLE *pstSettle, int nAcqIndex)
{
	memcpy((void *)pstSettle , (void *)&glstSettle[nAcqIndex], sizeof(STSETTLE));
	return APP_SUCC;
}

/**
* @brief Show settle data
* @param in pszTransName	
* @param in nTransNum	
* @param in nTransAmt		
* @return 
* @li APP_SUCC
*/
static int DispSettleDataItem(char *pszTitle, char *pszTransName, int nTransNum, char *pszTransAmt)
{
	int nRet;
	char szDispBuf[100] = {0};
	char szDispAmt[13+1] = {0};
	
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, pszTransName);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf+strlen(szDispBuf), "\nNum:|R%12d", nTransNum);	
	memset(szDispAmt, 0, sizeof(szDispAmt));
	ProAmtToDispOrPnt(pszTransAmt, szDispAmt);
	PubAllTrim(szDispAmt);
	PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf+strlen(szDispBuf),"\nAmt:|R%s", szDispAmt);		
	nRet = PubUpDownMsgDlg(pszTitle, szDispBuf, YES, 60, NULL);
	if (nRet==APP_QUIT)
	{
		return APP_QUIT;
	}
	else
	{
		return APP_SUCC;
	}

}

void GetAcqName(char *pszTitle, int nIndex)
{
	switch (nIndex)
	{
	case 0:
		strcpy(pszTitle, "VISA/MASTER/JCB");
		break;
	case 1:
		strcpy(pszTitle, "AMEX");
		break;
	case 2:
		strcpy(pszTitle, "MYDEBIT");
		break;
	case 3:
		strcpy(pszTitle, "UPI");
		break;
	case 4:
		strcpy(pszTitle, "V/M EPP");
		break;
	case 5:
		strcpy(pszTitle, "AMEX EPP");
		break;
	}
}

/**
* @brief Display summary
* @param in void
* @return 
* @li APP_SUCC
*/
int DispTotal(int nAcqIndex)
{
	STAMT_NUM_SETTLE stSettleInfo;
	char szNum[3+1] = {0}, szNum1[3+1] = {0};
	char szAmt[12+1] = {0}, szAmt1[12+1] = {0};
	char szTitle[32]= {0};

	GetAcqName(szTitle, nAcqIndex);
	memset(&stSettleInfo, 0, sizeof(STAMT_NUM_SETTLE));
	DealSettle(&stSettleInfo, nAcqIndex);

	/* SALE*/
	memcpy(szNum, stSettleInfo.sCreditSaleNum, 3);
	memcpy(szAmt, stSettleInfo.sCreditSaleAmt, 12);
PubDebug("szNum: %s, amt; %s", szNum, szAmt);
#if 1
	/* REFUND*/
	memcpy(szNum1, stSettleInfo.sCreditRefundNum, 3);
	memcpy(szAmt1, stSettleInfo.sCreditRefundAmt, 12);
#endif
	if (0 == atoi(szNum) && 0 == atoi(szNum1))
	{
		return APP_QUIT;
	}
	
	ASSERT_QUIT(DispSettleDataItem(szTitle, "TOTAL SALES:", atoi(szNum), szAmt));
PubDebug("======= %d", GetTransSwitchOnoff(TRANS_REFUND));
	if (YES == GetTransSwitchOnoff(TRANS_REFUND))	
	{
		if (atoi(szNum1) > 0)
		{
			ASSERT_QUIT(DispSettleDataItem(szTitle, "TOTAL REFUNDS:", atoi(szNum1), szAmt1));
		}
	}
	return APP_SUCC;
}

/**
* @brief Look up or print summary
* @param in void
* @return void
*/
int FindSettle()
{
	int nSelect = 0xFF;
	int nAcqIndex = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.Look up"),
		tr("2.Print")
	};
	while(1)
	{
		//ASSERT_QUIT( PubSelectListItem("1.Look up||2.Print", "Look up summary", NULL, 0xFF, &nSelect));
		nRet = PubShowMenuItems(tr("Look up summary"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			ASSERT_QUIT(SelectAcqIndex("DISPLAY TOTAL", &nAcqIndex, NO));
			DispTotal(nAcqIndex);
			break;
		case 2:
			ASSERT_QUIT(SelectAcqIndex("Print", &nAcqIndex, NO));
			if(APP_SUCC==PubConfirmDlg("Summary", "Print?", 0, 0 )) 
			{
				PubClearAll();
				DISP_PRINTING_NOW;
				PrintTotal(nAcqIndex);
			}
			break;				
		default:
			break;
		}
	}
}


/**
* @brief Clean settle data
* @param in void
* @return void
*/
void ClearAllSettle(void)
{	
	PubDelFile(SETTLE_FILE);

	SetVarWaterSum(0);
	SetVarHaveReSendNum(0);
	SetVarPrintSettleHalt(NO);
	SetVarPrintDetialHalt(NO);
	SetVarPrintWaterHaltFlag(NO);
	SetVarIsTotalMatch(1);
	SetVarBatchHaltFlag(NO);
	SetVarClrSettleDataFlag(NO);
	SetVarFinanceHaltFlag(1);
	SetVarMessageHaltFlag(1);
	SetVarEmvOfflineUpNum(1);
	SetVarEmvOnlineUpNum(1);
	SetVarEMVOfflineFailHaltFlag(1);
	SetVarEMVOnlineARPCErrHaltFlag(1);
		
	SetVarBatchSum(0);
	SetVarSettleDateTime("\x00\x00\x00\x00\x00");
}


/**
* @brief Clean settle data
* @param in void
* @return void
*/
void ClearSettle(int nAcqIndex)
{
	memset((void *)(&glstSettle[nAcqIndex]), 0, sizeof(STSETTLE));
	
	ASSERT(SetVarSettleData(glstSettle[nAcqIndex], nAcqIndex));

	SetVarWaterSum(0);
	SetVarHaveReSendNum(0);
	SetVarPrintSettleHalt(NO);
	SetVarPrintDetialHalt(NO);
	SetVarPrintWaterHaltFlag(NO);
	SetVarIsTotalMatch(1);
	SetVarBatchHaltFlag(NO);
	SetVarClrSettleDataFlag(NO);
	SetVarFinanceHaltFlag(1);
	SetVarMessageHaltFlag(1);
	SetVarEmvOfflineUpNum(1);
	SetVarEmvOnlineUpNum(1);
	SetVarEMVOfflineFailHaltFlag(1);
	SetVarEMVOnlineARPCErrHaltFlag(1);
		
	SetVarBatchSum(0);
	SetVarSettleDateTime("\x00\x00\x00\x00\x00");
}



/**
* @brief Update settle data
* @param in pszAmount	
* @param out pnTransNum	
* @param out pszAmount	
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
static int ChangeSettleItem(const char *pszAmount, SETTLE_NUM *pnTransNum, SETTLE_AMT *pstTransAmt, int nAcqIndex)
{
	int nOffset;
	int nSize = 0;
	uchar szAmt1[12+1] = {0};
	uchar szAmt[12+1] = {0};
	uchar szMaxAmt[12+1] = {0};

	nOffset = (char *)pstTransAmt - (char *)pnTransNum;

	if (nOffset != sizeof(SETTLE_NUM))
	{
		TRACE(("nOffset=%d %d", nOffset, sizeof(SETTLE_NUM)));
		return APP_FAIL;
	}

	nSize = sizeof(STSETTLE);
	nOffset = (char *)pnTransNum - (char *)(&glstSettle[nAcqIndex]);
	if (nOffset < 0)
	{
		return APP_FAIL;
	}
	(*pnTransNum)++;

	memset(szAmt1, 0, sizeof(szAmt1));
	memset(szAmt, 0, sizeof(szAmt));
	PubHexToAsc(pstTransAmt->sSettleAmt, 12, 0, szAmt1);
	AmtAddAmt(szAmt1, (uchar *)pszAmount, szAmt);
	PubAscToHex(szAmt, 12, 0, pstTransAmt->sSettleAmt);

	SaveSettleDataItem( (char *)pnTransNum, nOffset, sizeof(SETTLE_NUM), nAcqIndex);
	SaveSettleDataItem( (char *)pstTransAmt, nOffset +sizeof(SETTLE_NUM) , sizeof(SETTLE_AMT), nAcqIndex);

	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	strcpy((char *)szMaxAmt, MAX_AMOUNT);
	PubAddSymbolToStr((char *)szMaxAmt, 12, '0', 0);
	if(memcmp(szAmt, szMaxAmt, 12) > 0)
	{
		PubMsgDlg("Amount Out Of Limit!", "Pls Settle\nPree Any To Go on", 3, 10);
	}
	
	return APP_SUCC;
}

/**
* @brief Record and Update settle data after doing a transaction
* @param in pszAmount	 
* @param in cTransType	 
* @param in pszInterOrg	 
* @return 
* @li APP_SUCC
*/
int ChangeSettle(const STSYSTEM *pstSystem)
{
	STSETTLE *pstSettle;
	char szAmount [12+1] = {0};
		
	pstSettle = &glstSettle[pstSystem->nAcqIndex]; 

	strcpy(szAmount, pstSystem->szAmount);
	//dcc use ringgit
	if (1 == pstSystem->cIsUseDcc)
	{
		strcpy(szAmount, pstSystem->szLocalAmount);
	}
	
	switch(pstSystem->cTransType)
	{
	case TRANS_SALE:
	case TRANS_INSTALLMENT:
	case TRANS_CASH_ADVANCE:
	case TRANS_BARCODE:
		if (pstSystem->cFlagCashback==1 && strlen(pstSystem->szCashbackAmount)!=0)		
		{
			char szTotal[14] = {0};
			//need total amount
			AmtAddAmt((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)szTotal);
			ChangeSettleItem(szTotal, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount), pstSystem->nAcqIndex);
		}
		else
		{
			ChangeSettleItem(szAmount, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount), pstSystem->nAcqIndex);
		}
		break;
	case TRANS_VOID:	
	case TRANS_VOID_INSTALLMENT:
	case TRANS_VOID_CASH_ADVANCE:
		if (pstSystem->cFlagCashback==1 && strlen(pstSystem->szCashbackAmount)!=0)		
		{
			char szTotal[14] = {0};
			AmtAddAmt((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)szTotal);
			ChangeSettleItem(szTotal, &(pstSettle->_VoidSaleNum), &(pstSettle->_VoidSaleAmount), pstSystem->nAcqIndex);
		}
		else
		{
			ChangeSettleItem(szAmount, &(pstSettle->_VoidSaleNum), &(pstSettle->_VoidSaleAmount), pstSystem->nAcqIndex);
		}
		break;		
	case TRANS_REFUND:
		ChangeSettleItem(szAmount, &(pstSettle->_RefundNum), &(pstSettle->_RefundAmount), pstSystem->nAcqIndex);
        break;        	
	case TRANS_VOID_REFUND:
		ChangeSettleItem(szAmount, &(pstSettle->_VoidRefundNum), &(pstSettle->_VoidRefundAmount), pstSystem->nAcqIndex);
        break;        	
	case TRANS_OFFLINE:
		ChangeSettleItem(szAmount, &(pstSettle->_OfflineNum), &(pstSettle->_OfflineAmount), pstSystem->nAcqIndex);
        break;
	case TRANS_VOID_OFFLINE:
		ChangeSettleItem(szAmount, &(pstSettle->_VoidOfflineNum), &(pstSettle->_VoidOfflineAmount), pstSystem->nAcqIndex);
        break;        	

	case TRANS_ORS_INSTANT_REWARD:
	case TRANS_ORS_POINTS_RDMPT:
	case TRANS_ORS_VALUE_RDMPT:
	case TRANS_ORS_GIFTCODE_RDMPT:
	case TRANS_ORS_HOTDEAL_RDMPT:
		SaveOrsSettle(pstSystem->cTransType, pstSystem->stOrsInfo);
		break;
	default:
		break;
	}
	return APP_SUCC;
}

/**
* @brief Processing settle task, Check if settle is completed
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int DealSettleTask(void)
{
	if (YES == GetVarBatchHaltFlag() 
		|| YES == GetVarPrintSettleHalt()
		|| YES == GetVarPrintDetialHalt()
		|| YES == GetVarClrSettleDataFlag()
		)
	{
		if(PubConfirmDlg("SETTLE", "Settlement had not commpleted.Continue?", 0, 30)!=APP_SUCC)
			return APP_FAIL;
		if (APP_SUCC == Settle(1))
		{
			//ASSERT(SetVarIsLogin(NO));

			return APP_QUIT;
		}
		else
		{
			if (PubGetKeyCode(1) == KEY_ESC)
			{
				return APP_QUIT;
			}
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

/**
* @brief Do batch up
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @li APP_QUIT
*/
int DoBatchUp(void)
{	
	int nRet = 0;
	int nWaterSum;
	int nFileHandle;
	int nBatchNum = 0;
	char nMaxReSend, nSendNum = 0;
	int nIsFailBatchNum = TRUE;/**是否存在Fail上送笔数*/

	if (YES == GetVarIsTotalMatch())//对帐平不需要批上送
	{
		return APP_SUCC;
	}

	PubDisplayTitle("UPLOAD");
	GetRecordNum(&nWaterSum);

	if (nWaterSum > 0)
	{
		nRet = PubOpenFile(FILE_WATER, "w", &nFileHandle);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	else
	{
		return APP_SUCC;
	}

	GetVarCommReSendNum(&nMaxReSend);
	nMaxReSend = 1;//shtest
	for(nSendNum = 0; nSendNum <= nMaxReSend && nIsFailBatchNum; nSendNum++)
	{
		nRet = UpLoad(nFileHandle, nWaterSum, &nBatchNum);
		if (nRet != APP_SUCC)
		{
		//	SetVarBatchSum(nBatchNum);
			PubCloseFile(&nFileHandle);			
			return APP_FAIL;
		}
	}
	ASSERT_FAIL(PubCloseFile(&nFileHandle));	
	GetVarBatchSum(&nBatchNum);

	SetVarBatchMagOfflineHaltFlag(1);
	SetVarFinanceHaltFlag(1);
	SetVarMessageHaltFlag(1);	
	SetVarEmvOfflineUpNum(1);
	SetVarEmvOnlineUpNum(1);
	SetVarEMVOfflineFailHaltFlag(1);
	SetVarEMVOnlineARPCErrHaltFlag(1);

	SetVarBatchSum(0);
	return APP_SUCC;
}

/**
* @brief Send batch
* @param in nFileHandle	
* @param in nWaterSum	
* @param in pnBatchNum	
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
static int UpLoad(const int nFileHandle, const int nWaterSum, int *pnBatchNum)
{
	STSYSTEM stSystem;
	STTAANSRECORD stTransRecord;
	char sPackBuf[MAX_PACK_SIZE]; 
	int nPackLen = 0;
	int nLoop;
	int nRet, nFailLoop = 0, nIsFirstFail = TRUE;
	char nMaxReSend;
	char szTitle[32] = "UPLOAD";
	char sMac[8] = {0};
	
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	GetVarCommReSendNum(&nMaxReSend);	
	nLoop = 1;

	GetVarBatchMagOfflineHaltFlag(&nLoop);
	if (0 >= nLoop)
	{
		nLoop = 1;
	}

	for (; nLoop <= nWaterSum; nLoop++)
	{
		memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
		PubReadRec(nFileHandle, nLoop, (char *)&stTransRecord);
		//skip
		if (stTransRecord.nAcqType != EM_ACQTYPE_VISA && stTransRecord.nAcqType != EM_ACQTYPE_MASTER && stTransRecord.nAcqType != EM_ACQTYPE_JCB)
		{
			continue;
		}
		if (stTransRecord.cTransType != TRANS_SALE && stTransRecord.cTransType != TRANS_REFUND 
			&& stTransRecord.cTransType != TRANS_INSTALLMENT && stTransRecord.cTransType != TRANS_CASH_ADVANCE)
		{
			continue;
		}
		if(stTransRecord.cBatchUpFlag > nMaxReSend || stTransRecord.cStatus == 0x01)
		{
			continue;
		}
		
		sprintf(szTitle, "UPLOAD [%d]", nLoop);
		PubDisplayTitle(szTitle);
		PubClear2To4();
		PubDisplayGen(2, "UPLOADING...");
		PubDisplayGen(3, "PLEASE WAIT...");
		PubUpdateWindow();

		
		RecordToSys(&stTransRecord, &stSystem);
		DealSystem(&stSystem);
		
		memcpy(stSystem.szMsgID, "0320", 4);

		PackGeneral(&stSystem, &stTransRecord);
		
		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		nPackLen -= 8;
	PubDebugData("before mac: ", sPackBuf, nPackLen);
		ASSERT_FAIL(AddMac(sPackBuf, &nPackLen, KEY_TYPE_MAC));
		
#if 0
		GetMac(sPackBuf, &nPackLen, sMac);
		ASSERT_HANGUP_FAIL(ProcessTLED(&stSystem));
		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
		memcpy(sPackBuf+nPackLen-8, sMac, 8);
#endif			
		ASSERT_FAIL(CommConnect());
		
		IncVarTraceNo();
		nRet = CommSendRecv(sPackBuf, nPackLen, sPackBuf, &nPackLen);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			goto BATCHUPFAIL;
		}

		nRet = Unpack(sPackBuf, nPackLen);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			goto BATCHUPFAIL;
		}
		nRet = ChkRespMsgID("0320", sPackBuf);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			goto BATCHUPFAIL;
		}
		nRet = ChkRespon(&stSystem, sPackBuf+2);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			goto BATCHUPFAIL;
		}
		PubClear2To4();
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
			stTransRecord.cBatchUpFlag = 0xFE;
			goto BATCHUPFAIL;
		}
		stTransRecord.cBatchUpFlag = 0xFD;
		PubDisplayGen(3, "UPLOAD SUCC");
		PubUpdateWindow();
	BATCHUPFAIL:
		ASSERT_FAIL(DealBatchUpFail(&stTransRecord, nFileHandle, nLoop));
		if(stTransRecord.cBatchUpFlag <= nMaxReSend)
		{
			if(nIsFirstFail)
			{
				nFailLoop = nLoop;
				nIsFirstFail = FALSE;
				SetVarBatchMagOfflineHaltFlag(nFailLoop);
			}
		}
	}
	if(nIsFirstFail)
		nFailLoop = nWaterSum+1;
	SetVarBatchMagOfflineHaltFlag(nFailLoop);
	return APP_SUCC;

}

int CheckTransAmount(const char *pszAmount, const char cTransType, int nAcqIndex)
{	
	SETTLE_AMT stTotalAmount_NK;
	uchar szAmt1_NK[12+1] = {0};
	uchar szAmt1[12+1] = {0};
	uchar szAmt[12+1] = {0};
	uchar szMaxAmt[12+1] = {0};
	
	switch(cTransType)
	{
	case TRANS_SALE:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, "Amout Out Of Range Per Trans", 3, 10);
			return APP_FAIL;
		}
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._SaleAmount, sizeof(SETTLE_AMT));
		break;
	case TRANS_VOID:
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._VoidSaleAmount, sizeof(SETTLE_AMT));
		break;		
	case TRANS_VOID_REFUND:
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._VoidRefundAmount, sizeof(SETTLE_AMT));
		break;		
	case TRANS_VOID_OFFLINE:
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._VoidOfflineAmount, sizeof(SETTLE_AMT));
		break;		
	case TRANS_REFUND:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, "Amout Out Of Range Per Trans", 3, 10);
			return APP_FAIL;
		}
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._RefundAmount, sizeof(SETTLE_AMT));
        break;        	
	case TRANS_OFFLINE:
		memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._OfflineAmount, sizeof(SETTLE_AMT));
		break;	
	case TRANS_PREAUTH: 
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, "Amout Out Of Range Per Trans", 3, 10);
			return APP_FAIL;
		}
		memset(&stTotalAmount_NK, 0, sizeof(SETTLE_AMT));
		break;
	default:
		if(AscBigCmpAsc(pszAmount, ONE_TRADE_MAX_AMOUNT) > 0)
		{
			PubMsgDlg(NULL, "Amout Out Of Range Per Trans", 3, 10);
			return APP_FAIL;
		}
		break;
	}

	memcpy(&stTotalAmount_NK, &glstSettle[nAcqIndex]._SaleAmount, sizeof(SETTLE_AMT));

	memset(szAmt1_NK, 0, sizeof(szAmt1_NK));
	memset(szAmt1, 0, sizeof(szAmt1));
	PubHexToAsc(stTotalAmount_NK.sSettleAmt, 12, 0, szAmt1_NK);
	memset(szAmt, 0, sizeof(szAmt));
	AmtAddAmt(szAmt1, (uchar*)pszAmount, szAmt);
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	strcpy((char *)szMaxAmt, MAX_AMOUNT);
	PubAddSymbolToStr((char *)szMaxAmt, 12, '0', 0);
	
	if (memcmp(szAmt, szMaxAmt, 12) > 0) 
	{
		PubMsgDlg(NULL, "Total Amout Out Of Range", 3, 10);
		return APP_FAIL;
	}
	else
	{
		return APP_SUCC;
	}
}


int DealBatchUpFail(STTAANSRECORD *pstTransRecord, int nHandle, int nRecNo)
{
	int nRet, nBatchNum; 
	char nMaxReSend;

	GetVarCommReSendNum(&nMaxReSend);
	GetVarBatchSum(&nBatchNum);

	if(pstTransRecord->cBatchUpFlag < nMaxReSend)
	{
		pstTransRecord->cBatchUpFlag++;
	}
	else if(pstTransRecord->cBatchUpFlag != 0xFE && pstTransRecord->cBatchUpFlag != 0xFD)
	{
		pstTransRecord->cBatchUpFlag = 0xFF;
		nBatchNum++;
		SetVarBatchSum(nBatchNum);
	}
	else
	{
		nBatchNum++;
		SetVarBatchSum(nBatchNum);
	}
	
	if(pstTransRecord->cBatchUpFlag != 0)
	{	
		nRet = PubUpdateRecByHandle(nHandle, nRecNo, (char *)pstTransRecord);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{	
			return APP_FAIL; 
		}
	}
	return APP_SUCC;
}

int CheckSettleReponse(STSYSTEM stSystem)
{
	if (memcmp(stSystem.szResponse, "95", 2) == 0)
	{
		//DispResp(stSystem.szResponse, stSystem.nAcqIndex);//not require
		SetVarIsTotalMatch(NO);
	}			
	else if(memcmp(stSystem.szResponse, "77", 2) == 0)
	{
		SetVarIsTotalMatch(YES);
		//DispResp(stSystem.szResponse, stSystem.nAcqIndex);	
	}
	else if(memcmp(stSystem.szResponse, "00", 2) == 0)
	{
		ASSERT(1);
		SetVarIsTotalMatch(YES);
	}
	else 
	{
		DispResp(stSystem.szResponse, stSystem.nAcqIndex);
		CommHangUp();
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Precess summary
* @param in 
* @return 
* @li APP_SUCC
*/
int DealSettle(STAMT_NUM_SETTLE *pstSettle, int nAcqIndex)
{
	uchar szAmt1[13] = {0};
	uchar szAmt2[13] = {0};
	uchar szAmt[13] = {0};
	char szTmp[256] = {0};

	sprintf(szTmp, "%03ld", glstSettle[nAcqIndex]._SaleNum + glstSettle[nAcqIndex]._OfflineNum 
		- glstSettle[nAcqIndex]._VoidSaleNum - glstSettle[nAcqIndex]._VoidOfflineNum);
	memcpy(pstSettle->sCreditSaleNum, szTmp, 3);
	
	//sale+offline=A
	PubHexToAsc(glstSettle[nAcqIndex]._SaleAmount.sSettleAmt, 12, 0, szAmt1);
	PubHexToAsc(glstSettle[nAcqIndex]._OfflineAmount.sSettleAmt, 12, 0, szAmt2);
	AmtAddAmt(szAmt1, szAmt2, szAmt);

	//A-void=B
	strcpy((char *)szAmt1, (char *)szAmt);
	PubHexToAsc(glstSettle[nAcqIndex]._VoidSaleAmount.sSettleAmt, 12, 0, szAmt2);
	AmtSubAmt(szAmt1, szAmt2, szAmt);

	//B-void offline=C
	strcpy((char *)szAmt1, (char *)szAmt);
	PubHexToAsc(glstSettle[nAcqIndex]._VoidOfflineAmount.sSettleAmt, 12, 0, szAmt2);
	AmtSubAmt(szAmt1, szAmt2, szAmt);

	memcpy(pstSettle->sCreditSaleAmt, szAmt, 12);
	
	sprintf(szTmp, "%03ld", glstSettle[nAcqIndex]._RefundNum);
	memcpy(pstSettle->sCreditRefundNum, szTmp, 3);
	PubHexToAsc(glstSettle[nAcqIndex]._RefundAmount.sSettleAmt, 12, 0, (uchar *)szTmp);
	memcpy(pstSettle->sCreditRefundAmt, szTmp, 12);

	return APP_SUCC;
}

int ExportSettleData()
{
	int nLen = sizeof(STSETTLE) * ACQUIRER_NUM;
	
	if (NDK_OK != NDK_FsExist(SETTLE_FILE))
	{
		memset(&glstSettle, 0, sizeof(STSETTLE)*ACQUIRER_NUM);
		return APP_SUCC;
	}
	GetFromFile((char *)&glstSettle, 0, &nLen, SETTLE_FILE);

	return APP_SUCC;
}


/**
* @brief Display summary
* @param in void
* @return 
* @li APP_SUCC
*/
int CheckTotal(int nAcqIndex)
{
	STAMT_NUM_SETTLE stSettleInfo;
	char szNum[3+1] = {0}, szNum1[3+1] = {0};
	char szAmt[12+1] = {0}, szAmt1[12+1] = {0};
	
	memset(&stSettleInfo, 0, sizeof(STAMT_NUM_SETTLE));
	DealSettle(&stSettleInfo, nAcqIndex);

	/* SALE*/
	memcpy(szNum, stSettleInfo.sCreditSaleNum, 3);
	memcpy(szAmt, stSettleInfo.sCreditSaleAmt, 12);

	/* REFUND*/
	memcpy(szNum1, stSettleInfo.sCreditRefundNum, 3);
	memcpy(szAmt1, stSettleInfo.sCreditRefundAmt, 12);

	if (0 == atoi(szNum) && 0 == atoi(szNum1))
	{
		return APP_QUIT;
	}
	return APP_SUCC;
	
}

int InitLastSettleFile()
{
	STRECFILE stSettleFile;
	ST_SETTLE_REC stSettleRec;
	int i = 0;
	int nRet = 0;

	strcpy( stSettleFile.szFileName, SETTLEFILENAME );
	stSettleFile.cIsIndex = FILE_CREATEINDEX;						
	stSettleFile.unMaxOneRecLen = sizeof(ST_SETTLE_REC);
	stSettleFile.unIndex1Start =(uint)((char *)(stSettleRec.szAcqIndex) - (char *)&stSettleRec)+1;
	stSettleFile.unIndex1Len =  sizeof(stSettleRec.szAcqIndex);
	stSettleFile.unIndex2Start =  1;
	stSettleFile.unIndex2Len = 1;
	ASSERT(PubCreatRecFile(&stSettleFile));

	memset(&stSettleRec, 0, sizeof(ST_SETTLE_REC));
	for (i = 0; i < MAXBATCH_NUM; i++)
	{
		nRet = PubAddRec(SETTLEFILENAME, (char *)(&stSettleRec));
		PubDebug("1nRet = %d[i=%d]", nRet, i);
	}
	
	return APP_SUCC;
}




/**
* @brief Update settle data
* @param in pszAmount	
* @param out pnTransNum	
* @param out pszAmount	
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
static int IncreaseSettleItem(const char *pszAmount, SETTLE_NUM *pnTransNum, SETTLE_AMT *pstTransAmt)
{
	uchar szAmt1[12+1] = {0};
	uchar szAmt[12+1] = {0};

	(*pnTransNum)++;

	memset(szAmt1, 0, sizeof(szAmt1));
	memset(szAmt, 0, sizeof(szAmt));
	PubHexToAsc(pstTransAmt->sSettleAmt, 12, 0, szAmt1);
	AmtAddAmt(szAmt1, (uchar *)pszAmount, szAmt);
	PubAscToHex(szAmt, 12, 0, pstTransAmt->sSettleAmt);

	return APP_SUCC;
}
/**
* @brief Record and Update settle data after doing a transaction
* @param in pszAmount	 
* @param in cTransType	 
* @param in pszInterOrg	 
* @return 
* @li APP_SUCC
*/
int ChangeSimpleSettle(const STTAANSRECORD *pstTransLog, STSETTLE *pstSettle)
{
	char szAmount [12+1] = {0};
	char szCashBackAmount [12+1] = {0};	

	PubHexToAsc((uchar *)pstTransLog->sAmount, 12, 0,  (uchar *)szAmount);
	PubHexToAsc((uchar *)pstTransLog->sCashbackAmount, 12, 0,  (uchar *)szCashBackAmount);
	
	//dcc use ringgit
	if (1 == pstTransLog->cIsUseDcc)
	{
		strcpy(szAmount, pstTransLog->szLocalAmount);
	}
	
	switch(pstTransLog->cTransType)
	{
	case TRANS_SALE:
	case TRANS_INSTALLMENT:
	case TRANS_CASH_ADVANCE:
		if (pstTransLog->cFlagCashback==1 && AtoLL(szCashBackAmount)!=0)		
		{
			char szTotal[14] = {0};
			//need total amount
			AmtAddAmt((uchar *)szAmount, (uchar *)szCashBackAmount, (uchar *)szTotal);
			IncreaseSettleItem(szTotal, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount));
		}
		else
		{
			IncreaseSettleItem(szAmount, &(pstSettle->_SaleNum), &(pstSettle->_SaleAmount));
		}
		break;
	case TRANS_VOID:	
	case TRANS_VOID_INSTALLMENT:
	case TRANS_VOID_CASH_ADVANCE:
		if (pstTransLog->cFlagCashback==1 && AtoLL(szCashBackAmount)!=0)		
		{
			char szTotal[14] = {0};
			AmtAddAmt((uchar *)szAmount, (uchar *)szCashBackAmount, (uchar *)szTotal);
			IncreaseSettleItem(szTotal, &(pstSettle->_VoidSaleNum), &(pstSettle->_VoidSaleAmount));
		}
		else
		{
			IncreaseSettleItem(szAmount, &(pstSettle->_VoidSaleNum), &(pstSettle->_VoidSaleAmount));
		}
		break;		
	case TRANS_REFUND:
		IncreaseSettleItem(szAmount, &(pstSettle->_RefundNum), &(pstSettle->_RefundAmount));
        break;        	
	case TRANS_VOID_REFUND:
		IncreaseSettleItem(szAmount, &(pstSettle->_VoidRefundNum), &(pstSettle->_VoidRefundAmount));
        break;        	
	case TRANS_OFFLINE:
		IncreaseSettleItem(szAmount, &(pstSettle->_OfflineNum), &(pstSettle->_OfflineAmount));
        break;
	case TRANS_VOID_OFFLINE:
		IncreaseSettleItem(szAmount, &(pstSettle->_VoidOfflineNum), &(pstSettle->_VoidOfflineAmount));
        break;        	
	default:
		break;
	}
	return APP_SUCC;
}

