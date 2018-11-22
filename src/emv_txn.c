/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  emv_txn.c
** File indentifier: 
** Brief:  EMV Transaction processing module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-20
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

static void EmvDispTvrTsi(void);
static int EmvInitApp(STSYSTEM *);
int GetResponseFieldDE55(int nAcqIndex, char *pszDe55, int *pnLen);

static emv_opt  gstEMVOption;
static unsigned long long gullAmount;
static unsigned long long gullCashbackAmount;//for clts mccs

/**
* @brief Emv App Init
* @detail Read App Data GPO
* @param [IN] pstSystem 
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author lg
* @date 2013-3-13
*/
int EmvInitApp(STSYSTEM * pstSystem)
{
	int nRet;

	if (YES == GetVarEmvIsDownAIDFlag())
	{
		DownEmvParam();
		return APP_QUIT;
	}
	
	//EMVL2_SetErrorCode(0);
	memset(&gstEMVOption,0,sizeof(emv_opt));
	gullAmount = 0;
	gullCashbackAmount = 0;
	SetIsKeyInPIN(0);
	EmvSetTxnType(pstSystem->cTransType);
	
	gullAmount = AtoLL(pstSystem->szAmount);
	if(gullAmount > 0)
	{
		if (ATTR_qPBOC == pstSystem->cTransAttr 
			||ATTR_EMV_STANDARD_RF == pstSystem->cTransAttr)
		{
			if (1 == pstSystem->cFlagCashback)
			{		
				EmvSetAmount(pstSystem->szAmount, pstSystem->szCashbackAmount);/**<传入指定的金额，EMV库流程里将不再弹出输入金额界面*/
				gullCashbackAmount = AtoLL(pstSystem->szCashbackAmount);
			}
			else
			{
				EmvSetAmount(pstSystem->szAmount, NULL);
			}
			gstEMVOption._request_amt = EMV_TRANS_REQAMT_NO;
		}
		else
		{
			if (1 == pstSystem->cFlagCashback)
				EmvSetAmount(pstSystem->szAmount, pstSystem->szCashbackAmount);/**<传入指定的金额，EMV库流程里将不再弹出输入金额界面*/
			else
				EmvSetAmount(pstSystem->szAmount, NULL);
			gstEMVOption._request_amt = EMV_TRANS_REQAMT_APS;
		}
	}
	else
	{
		gstEMVOption._request_amt = EMV_TRANS_REQAMT_DDA;
		if (ATTR_qPBOC == pstSystem->cTransAttr
			||ATTR_EMV_STANDARD_RF == pstSystem->cTransAttr)
		{
			gstEMVOption._request_amt = EMV_TRANS_REQAMT_RFPRECESS;
		}
	}
	

	gstEMVOption._seq_to = EMV_PROC_TO_READAPPDATA;
TRACE(("====1"));
	nRet = EmvReadICC(&gstEMVOption, pstSystem);
TRACE(("====2.%d", nRet));
	ResetInputAmtInterface();
	SaveEmvTransSerial();
	if (nRet != APP_SUCC)
	{
		TRACE(("nRet=%d", nRet));
		return nRet;
	}
	return APP_SUCC;
}

/**
* @brief Emv Process App
* @detail Offline Data authentication. Precess Limit
* @param [out] pcOnlineResult    Online Status (FALSE NO,TRUE YES)
* @param [out] pszAmount         Transaction Amount
* @return
* @li APP_FAIL          Fail
* @li APP_SUCC          Success
* @li EMV_TRANS_ACCEPT  Offline
* @li EMV_TRANS_DENIAL  Refuse
* @author lg
* @date 2013-3-13
*/
int EmvProcessApp(char *pszAmount, const char cTransAttr)
{
	int nRet;
	char szMsg[256] = {0};
	char szContent[50] = {0};

	PubClear2To4();
	PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1,"PROCESSING...");
	PubUpdateWindow();
	NDK_ScrPush();
	EmvClrOnlinePin();

PubDebug("-------%s", gstEMVOption._auth_resp_code);
PubDebugData("1++++++code: ", gstEMVOption._auth_resp_code, 2);
	gstEMVOption._seq_to = EMV_PROC_CONTINUE;

	if (ATTR_PBOC_EC_RF == cTransAttr
	        || ATTR_EMV_STANDARD_RF == cTransAttr
	   )
	{
		nRet = EMV_rf_start(&gstEMVOption, gullAmount);
	}
	else
	{
		nRet = EMV_Start(&gstEMVOption);
	}
PubDebugData("2++++++code: ", gstEMVOption._auth_resp_code, 2);

	NDK_ScrPop();

	if (pszAmount != NULL)
	{
		EmvGetAmount(pszAmount);
	}
TRACE(("@@@@@@@@ EMV PROCESS = %d ", nRet));

	EmvDispTvrTsi();
	if (nRet == EMV_TRANS_ACCEPT)
	{
		if (ATTR_PBOC_EC_RF == cTransAttr
		        || ATTR_EMV_STANDARD_RF == cTransAttr)
		{
			EMV_rf_suspend(nRet);
		}
		else
		{
			EMV_Suspend(nRet);
		}
		return nRet;

	}
	else if (nRet == EMV_TRANS_DENIAL)
	{
		strcpy(szContent, "DECLINE");
		strcpy(szMsg, EmvErrMsg(EMV_ErrorCode()));
		goto END;
	}
	else if (nRet == EMV_TRANS_GOONLINE)
	{
		return APP_SUCC;
	}
	else if(nRet == EMV_TRANS_FALLBACK)
	{
		if (ATTR_PBOC_EC_RF == cTransAttr)
		{
			strcpy(szMsg,  "PLS INSERTIC");
		}
		else
		{
			strcpy(szMsg,  "PLS SWIPE");
		}
		strcpy(szContent, "READED CARD ERR");
		goto FALLBACK;
	}
	else
	{
		strcpy(szContent, "ABEND");
		strcpy(szMsg, EmvErrMsg(EMV_ErrorCode()));
		goto END;
	}
FALLBACK:

	if (ATTR_PBOC_EC_RF == cTransAttr
	        || ATTR_EMV_STANDARD_RF == cTransAttr
	   )
	{
		EMV_rf_suspend(nRet);
	}
	else
	{
		EMV_Suspend(nRet);
	}
	DispOutICC(NULL, szContent, szMsg);
	return APP_FALLBACK;

END:
	if (ATTR_PBOC_EC_RF == cTransAttr
	        || ATTR_EMV_STANDARD_RF == cTransAttr
	   )
	{
		EMV_rf_suspend(nRet);
	}
	else
	{
		EMV_Suspend(nRet);
	}
	DispOutICC(NULL, szContent, szMsg);
	return nRet;

}

/**
* @brief Emv Complete
* @detail Verify the issuing Bank, Precess the script
* @param [in] cOnlineResult    Online status (FALSE NO,TRUE YES)
* @param [out] pszAmount         Transaction Amount
* @return
* @li APP_FAIL          Fail
* @li APP_SUCC          Success
* @li EMV_TRANS_ACCEPT  Offine Transaction
* @li EMV_TRANS_DENIAL  Refuse
* @author 刘罡
* @date 2013-3-13
*/
int EmvCompleteApp(char * pszTitle,
                   char cOnlineResult,
                   STSYSTEM *pstSystem,
                   STREVERSAL *pstReversal,
                   const char *pszRecvField55,
                   int nField55Len,
                   char * pcTSIComleted,
                   int nInputPinNum)
{
	int nRet;
	char szTSI[2+1] = {0};
	char cReSuccFlag = FALSE;

//return 	0;//shtest
PubDebug("2-------%s", gstEMVOption._auth_resp_code);
PubDebugData("2++++++code: ", gstEMVOption._auth_resp_code, 2);

	if(cOnlineResult == TRUE)
	{
		gstEMVOption._field55=(uchar *)pszRecvField55+2;
		gstEMVOption._field55_len= nField55Len;
		gstEMVOption._auth_resp_code = (uchar *)pstSystem->szResponse;
		if (memcmp(pstSystem->szResponse, "00", 2) == 0)
		{
			gstEMVOption._auth_resp_code = (uchar *)"00";
			gstEMVOption._online_result = EMV_TRANS_ONLINESUCC_ACCEPT;
		}
#ifdef SUPPORT_ISSREF
		else if (memcmp(pstSystem->szResponse, "01", 2) == 0)
		{
			gstEMVOption._online_result = EMV_TRANS_ONLINESUCC_ISSREF;
		}
#endif
		else
		{
			gstEMVOption._online_result = EMV_TRANS_ONLINESUCC_DENIAL;
			if (pstSystem->nAcqIndex == EM_ACQINDEX_AMEX)
			{
				gstEMVOption._auth_resp_code = (uchar *)"05";
			}
		}
	}
	else
	{
	//	gstEMVOption._auth_resp_code = (uchar *)"\x5A\x33";
		gstEMVOption._online_result = EMV_TRANS_ONLINEFAIL;
	}

	if (ATTR_PBOC_EC_RF == pstSystem->cTransAttr
	        || ATTR_EMV_STANDARD_RF == pstSystem->cTransAttr
	   )
	{
		nRet = EMV_rf_start(&gstEMVOption, gullAmount);
		EMV_rf_suspend(nRet);
	}
	else
	{
		nRet = EMV_Start(&gstEMVOption);
		EMV_Suspend(nRet);
	}
	EmvEntryPointGetData(_EMV_TAG_9B_TM_TSI, (uchar *)szTSI, sizeof(szTSI));
	if (szTSI[0]&0x04)/**<If TSI indicate that processing script has completed*/
	{
		*pcTSIComleted = TRUE;
	}
	else
	{
		*pcTSIComleted = FALSE;

	}

	PubClearAll();
	EmvDispTvrTsi();

	if (NULL != pstReversal)
	{
		if (TRUE == cOnlineResult)
		{
			memset(pstReversal->szFieldAdd1, 0, sizeof(pstReversal->szFieldAdd1));
			pstReversal->nFieldAdd1Len = 0;
			EmvPackField55(EMV_REVERSAL, pstReversal->szFieldAdd1, &(pstReversal->nFieldAdd1Len), pstSystem->nAcqIndex);
			SetVarReversalData((char *)pstReversal, sizeof(STREVERSAL));
		}
	}

	if (nRet != EMV_TRANS_ACCEPT)
	{
		if (TRUE == cOnlineResult &&  memcmp(pstSystem->szResponse, "00", 2) == 0)
		{
			cReSuccFlag = TRUE;
		}
		else if (TRUE == cOnlineResult &&  memcmp(pstSystem->szResponse, "00", 2) != 0)
		{
			if (NULL != pstReversal)
			{
				SetVarIsReversal(NO);
				if (memcmp(pstSystem->szResponse, "55", 2) == 0 && (nInputPinNum >= 3))
				{
					PubConfirmDlg(pszTitle, "Input Wrong Pwd for 3 times", 0, 30);
				}
			}
			if (memcmp(pstSystem->szResponse, "LE", 2) == 0)
			{
				DispRespEFTInfo();
			}
			else
			{
				DispResp(pstSystem->szResponse, pstSystem->nAcqIndex);
			}

		}

		if (FALSE == cOnlineResult && nRet == EMV_TRANS_DENIAL)
		{
			IncVarTraceNo();
			pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_FAIL;
			EmvSaveWater(FALSE,pstSystem);
		}
		if (nRet == EMV_TRANS_DENIAL)
		{
			if (TRUE == cReSuccFlag)
			{
				DispOutICC(pszTitle, "DECLINE", "PLS Contact issue bank");
			}
			else 
			{
				if (EMV_TRANS_ONLINESUCC_DENIAL == gstEMVOption._online_result) 
				{
					DispOutICC(pszTitle, "DECLINE", "");
				}
				else
				{
					DispOutICC(pszTitle, "DECLINE", EmvErrMsg(EMV_ErrorCode()));
				}
			}
		}
		else
		{
			if (TRUE == cReSuccFlag)
			{
				DispOutICC(pszTitle, "TRANS FAIL", "PLS CONTACT ISSUING BANK");
			}
			else
			{
				if (EMV_TRANS_ONLINESUCC_DENIAL == gstEMVOption._online_result) 
				{
					DispOutICC(pszTitle, "TRANS FAIL", "");
				}
				else
				{
					DispOutICC(pszTitle, "TRANS FAIL", EmvErrMsg(EMV_ErrorCode()));
				}
			}
		}
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief EMV SALE
* @param in pstSystem
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvSale(STSYSTEM *pstSystem)
{
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char cOnlineResult = FALSE;
	char szRecvFiled55[512+1];
	int nFieldLen = 0,nInputPinNum = 0;
	int nRet = 0;
	char *pszTitle = "SALE";
	char szAid[20+1];
	int nEmvRes = 0;
#ifdef DEMO
#else
	char cTSICompleted = FALSE;
#endif

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	PubDisplayTitle(pszTitle);
	
	ASSERT_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	EmvSetTxnPan(pstSystem->szPan);

	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	
	memcpy(pstSystem->szMsgID, "0200", 4);
	if (1 == pstSystem->cFlagCashback)
	{
		memcpy(pstSystem->szProcCode, "090000", 6);
	}
	else
	{
		memcpy(pstSystem->szProcCode, "000000", 6);
	}
	memcpy(pstSystem->szServerCode, "00", 2);

	cOnlineResult = FALSE;

	nEmvRes = EmvProcessApp(pstSystem->szAmount, pstSystem->cTransAttr);
	if (nEmvRes != APP_SUCC)
	{
		switch (nEmvRes)
		{
		case EMV_TRANS_ACCEPT:
			goto WATERANDPRINT;
			break;
		case EMV_TRANS_DENIAL:
			goto OFFLINEFAIL;
			break;
		default:
			CommHangUp();
			return nEmvRes;
			break;
		}
	}

	/**
	* Get Online Pin
	*/
	EmvGetOnlinePin(pstSystem->szPin);

	/**
	* Advance dial
	*/
#ifdef DEMO
#else
	CommPreDial();
#endif	
	
	ASSERT_HANGUP_QUIT(CheckInputPin(pstSystem));

	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	if(nInputPinNum > 0)
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
	pstSystem->cHasPinInput = GetIsKeyInPIN();

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

	cOnlineResult = TRUE;
	/**
	* Dialing
	*/
#ifdef DEMO
#else
	nRet = CommConnect();
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		return APP_QUIT;//20171222 要直接退出
		//cOnlineResult = FALSE;
	}
#endif

	GetVarTraceNo(pstSystem->szTrace);
	EmvPackField55(EMV_SALE, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);
	SaveData9F26RQ();
	/*
	* Dial Success
	*/
	while(TRUE == cOnlineResult)
	{
		/**
		* Get necessary data
		*/
		DealSystem(pstSystem);
		/**
		* Begin to pack
		*/
		nRet = PackGeneral(pstSystem, NULL);
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			EMV_Suspend(-1);
			DispOutICC(pszTitle, "Pack fail", "");
			return APP_FAIL;
		}
		/**
		* Save reversal data
		*/
		SystemToReveral(pstSystem, &stReversal);
		EmvPackField55(EMV_REVERSAL, stReversal.szFieldAdd1, &stReversal.nFieldAdd1Len, pstSystem->nAcqIndex);
		nInputPinNum++;
#ifdef DEMO
		IncVarTraceNo();
		sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
		sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL|DPC_NOCHECKRESP,pstSystem,&stReversal,nInputPinNum);
		if(APP_REPIN == nRet)
		{
			PubMsgDlg(pszTitle, "\nPASSWORD ERROR!PLS TRY AGAIN!", 3, 30);
			goto regetpin;
		}
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			PubICPowerDown(ICTYPE_IC);
			EMV_Suspend(-1);
			return APP_FAIL;
		}

		/**
		* Get Field 55
		*/
		GetResponseFieldDE55(pstSystem->nAcqIndex, szRecvFiled55, &nFieldLen);

		break;
	}

#ifdef DEMO
#else
	ASSERT_HANGUP_FAIL(EmvCompleteApp(pszTitle,cOnlineResult,pstSystem,&stReversal,szRecvFiled55,nFieldLen, &cTSICompleted, nInputPinNum));
#endif

WATERANDPRINT:

	if (TRUE == cOnlineResult)
	{
		pstSystem->cEMV_Status = EMV_STATUS_ONLINE_SUCC;
		pstSystem->cTransType = TRANS_SALE;
		pstSystem->cTransAttr = ATTR_EMV_STANDARD;		
		EmvSaveWater(TRUE, pstSystem);
		SysToRecord(pstSystem, &stTransRecord);
		EmvAddtionWater(&stTransRecord);

		nRet = AppendTransRecord(&stTransRecord);
		if (nRet != APP_SUCC)
		{
			strcpy(stReversal.szResponse, "96");
			SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
			CommHangUp();
			return APP_FAIL;
		}

		SetVarIsReversal(NO);
	}
	else //脱机
	{
		PubBeep(1);
		pstSystem->cTransAttr = ATTR_PBOC_EC;
		pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_SUCC;

		pstSystem->szInputMode[2] = '2';/** 脱机，未输入联机pin*/

		if (nEmvRes == EMV_TRANS_ACCEPT)
		{
			//strcpy(pstSystem->szResponse, "Y1");
			strcpy(pstSystem->szAuthCode, "Y1");
		}
		else//联机失败转脱机
		{
			//strcpy(pstSystem->szResponse, "Y3");
			strcpy(pstSystem->szAuthCode, "Y3");
		}
		
		EmvPackField55(EMV_SALE, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);
		SaveData9F26RQ();

		/*	增加流水号 */
		IncVarTraceNo();
		
		/*保存脱机交易系统参考号*/
		EmvSaveWater(TRUE, pstSystem);
		memset(&stTransRecord,0,sizeof(STTAANSRECORD));
		SysToRecord(pstSystem, &stTransRecord);
		EmvAddtionWater(&stTransRecord);

		nRet = AppendTransRecord(&stTransRecord);
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			return APP_FAIL;
		}
		IncVarOfflineUnSendNum();		/*需上送流水数加1*/
	}

	ChangeSettle(pstSystem);
	PubClearAll();
	PubDisplayTitle(pszTitle);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	IncVarInvoiceNo();
	PrintWater(&stTransRecord, FIRSTPRINT);
	PubSetPosDateTime(pstSystem->szDate, "MMDD", pstSystem->szTime);
	if (YES == GetVarIsPinpad())
	{
		PubClrPinPad();
	}
	DispOutICC(pszTitle, NULL, "");
	/**
	* Send offline
	*/
	if (TRUE == cOnlineResult)
	{
		EmvSendTC();
		SendOffline(0);
		CommHangUp();
	}
	return APP_SUCC;

OFFLINEFAIL:
	IncVarTraceNo();
	memset(szAid, '\0', sizeof(szAid));
	EmvEntryPointGetData(_EMV_TAG_9F06_TM_AID, (uchar *)szAid, sizeof(szAid));
	EmvOrgSheet(szAid, pstSystem->szInterOrg);
	pstSystem->cEMV_Status = EMV_STATUS_OFFLINE_FAIL;

	EmvPackField55(EMV_SALE, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);

	/*Save Fail Transaction log*/
	EmvSaveWater(FALSE, pstSystem);
	DispOutICC(pszTitle, "DECLINE", EmvErrMsg(EMV_ErrorCode()));
	CommHangUp();
	return APP_FAIL;
}

int EmvBalance(STSYSTEM *pstSystem)
{
	//char szRecvFiled55[260+1];
	int nInputPinNum = 0;//nFieldLen, 
	int nRet = 0;
	char *pszTitle = "BALANCE";
	//char cTSICompleted = FALSE;
	char szAmt[14] = {0};
	STTAANSRECORD stTransRecord;


	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	
	pstSystem->cTransType = (char)TRANS_BALANCE;
	pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD;

	if(memcmp(pstSystem->szInputMode,"07",2) ==0)
	{
		pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD_RF;
	}
	
	PubDisplayTitle(pszTitle);

	/**
	* Advance dial
	*/	
	CommPreDial();

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	EmvSetTxnPan(pstSystem->szPan);

	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0100", 4);
	memcpy(pstSystem->szProcCode, "310000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);


	/**
	* Process Flow
	*/
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	if(pstSystem->cTransAttr == ATTR_EMV_STANDARD)
	{
		nRet = EmvProcessApp(pstSystem->szAmount, pstSystem->cTransAttr);
		if (APP_SUCC != nRet)
		{
			CommHangUp();
			return nRet;
		}
		TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));

		/**
		* Get Online Pin
		*/
		EmvGetOnlinePin(pstSystem->szPin);

		ASSERT_HANGUP_QUIT(CheckInputPin(pstSystem));
	}
	else
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}

	if(nInputPinNum > 0)
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);
	pstSystem->cHasPinInput = GetIsKeyInPIN();
	
	PubDisplayTitle(pszTitle);

	/**
	* reversal
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	
	PubDisplayTitle(pszTitle);
	/**
	* Dialing
	*/
#ifdef DEMO
#else	
	ASSERT_HANGUP_QUIT(CommConnect());
#endif

	/*
	* Dial successfully
	*/
	EmvPackField55(EMV_PREAUTH, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);
	SaveData9F26RQ();
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	/**
	* Begin to pack
	*/
	nRet = PackGeneral(pstSystem, NULL);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		DispOutICC(pszTitle, "PACKED FAIL", "");
		return APP_FAIL;
	}
	/**
	* Save reversal data
	*/
	
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
	sprintf(pstSystem->szAmount, "000000005400");
#endif
	if(ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{
		nRet = DealPackAndComm(pszTitle, DPC_NOCHECKRESP,pstSystem,NULL,nInputPinNum);
	}
	else
	{
		nRet = DealPackAndComm(pszTitle, DPC_DEFAULT,pstSystem,NULL,nInputPinNum);		
	}
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR!", 3, 30);
		return APP_SUCC;
	}
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		return APP_FAIL;
	}
#if 0
	if (ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{	
		/**
		* Get Field 55
		*/
		memset(szRecvFiled55, 0, sizeof(szRecvFiled55));
		nFieldLen = 255;
		GetField(55, szRecvFiled55+2, &nFieldLen);
		PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);


		ASSERT_HANGUP_FAIL(EmvCompleteApp(pszTitle,TRUE,pstSystem, NULL,szRecvFiled55,nFieldLen, &cTSICompleted, nInputPinNum));
	}
#endif
	if (memcmp(pstSystem->szResponse, "00", 2) != 0)
	{
		if (memcmp(pstSystem->szResponse, "LE", 2) == 0)
		{
			DispRespEFTInfo();
		}
		else
		{
			DispResp(pstSystem->szResponse, pstSystem->nAcqIndex);
		}
		return APP_FAIL;
	}
	
	PubClear2To4();
	ProAmtToDispOrPnt(pstSystem->szAmount, szAmt);
	PubDisplayStrInline(1, 2, "balance: ");
	//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT,3,"|RRM %s", szAmt);
	PubDisplayStr(DISPLAY_MODE_NORMAL, 3, 1, "RM %s", szAmt);
	PubDisplayStrInline(1, 5, "[ENTER] To Print");
	ASSERT_QUIT(ProConfirm());

	PubClearAll();
	SysToRecord(pstSystem, &stTransRecord);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	PrintBalance(&stTransRecord);
	return APP_SUCC;

}

/**
* @brief Emv Preauth
* @param in pstSystem
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvPreAuth(STSYSTEM *pstSystem)
{
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char szRecvFiled55[260+1];
	int nFieldLen, nInputPinNum = 0;
	int nRet = 0;
	char *pszTitle = "PREAUTH";
	char cTSICompleted = FALSE;

	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	pstSystem->cTransType = (char)TRANS_PREAUTH;
	pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD;

	if(memcmp(pstSystem->szInputMode,"07",2) ==0)
	{
		pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD_RF;
	}
	
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

	EmvSetTxnPan(pstSystem->szPan);

	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0100", 4);
	memcpy(pstSystem->szProcCode, "000000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);


	/**
	* Process Flow
	*/
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	if(pstSystem->cTransAttr == ATTR_EMV_STANDARD)
	{
		nRet = EmvProcessApp(pstSystem->szAmount, pstSystem->cTransAttr);
		if (APP_SUCC != nRet)
		{
			CommHangUp();
			return nRet;
		}
		TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));

		/**
		* Get Online Pin
		*/
		EmvGetOnlinePin(pstSystem->szPin);

		ASSERT_HANGUP_QUIT(CheckInputPin(pstSystem));
	}
	else
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}
	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	if(nInputPinNum > 0)
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);
	pstSystem->cHasPinInput = GetIsKeyInPIN();
	
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
	* Dialing
	*/
#ifdef DEMO
#else	
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/*
	* Dial successfully
	*/
	EmvPackField55(EMV_PREAUTH, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);
	SaveData9F26RQ();
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	/**
	* Begin to pack
	*/
	nRet = PackGeneral(pstSystem, NULL);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		DispOutICC(pszTitle, "PACKED FAIL", "");
		return APP_FAIL;
	}
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);
	EmvPackField55(EMV_REVERSAL, stReversal.szFieldAdd1, &stReversal.nFieldAdd1Len, pstSystem->nAcqIndex);
	nInputPinNum++;
	
#ifdef DEMO
		IncVarTraceNo();
		sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
		sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	if(ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL|DPC_NOCHECKRESP,pstSystem,&stReversal,nInputPinNum);
	}
	else
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);		
	}
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		return APP_FAIL;
	}

	if (ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{	
		/**
		* Get Field 55
		*/
		memset(szRecvFiled55, 0, sizeof(szRecvFiled55));
		nFieldLen = 255;
		GetField(55, szRecvFiled55+2, &nFieldLen);
		PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);


#ifdef DEMO
#else	
		ASSERT_HANGUP_FAIL(EmvCompleteApp(pszTitle,TRUE,pstSystem,&stReversal,szRecvFiled55,nFieldLen, &cTSICompleted, nInputPinNum));
#endif
	}
	
	pstSystem->cEMV_Status = EMV_STATUS_ONLINE_SUCC;

	EmvSaveWater(TRUE, pstSystem);
	SysToRecord(pstSystem, &stTransRecord);
	EmvAddtionWater(&stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);

}

/**
* @brief Read App Icc Data
* @param in out pstSystem STSYSTEM
* @param in out emv_opt *pstEMVOption
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvReadICC(void *pstOper, STSYSTEM *pstSystem)
{
	int nRet;
	int nEmvRes;
	emv_opt *pstEMVOption;
	char cIsCheckOnlyEC = TRUE; 
	STENTRYPOINTOPT stEntryPointOpt;

	SetKernelType(0);

	if (NULL == pstOper)
		return APP_FAIL;
	pstEMVOption = (emv_opt *)pstOper;
	
	memset(&stEntryPointOpt, 0, sizeof(STENTRYPOINTOPT));

	switch(pstSystem->cTransType)
	{
	case TRANS_SALE:
	case TRANS_INSTALLMENT:
	case TRANS_CASH_ADVANCE:
		pstEMVOption->_trans_type = EMV_TRANS_SALE;
		if (1 == pstSystem->cFlagCashback)
		{
			pstEMVOption->_trans_type = EMV_TRANS_CASHBACK;
		}

		if (ATTR_qPBOC == pstSystem->cTransAttr)
		{
			pstEMVOption->_trans_type = EMV_TRANS_RF_SALE;
			pstEMVOption->_seq_to = EMV_PROC_CONTINUE;
			cIsCheckOnlyEC = FALSE;
		}
		cIsCheckOnlyEC = FALSE;
		pstEMVOption->_force_online_enable = TRUE;
		break;
	case TRANS_PREAUTH:
		pstEMVOption->_trans_type = EMV_TRANS_PREAUTH;
		pstEMVOption->_force_online_enable = TRUE;
		break;
	case TRANS_REFUND:
		pstEMVOption->_request_amt = EMV_TRANS_REQAMT_NO;
		pstEMVOption->_trans_type = 0; 			
		if (ATTR_EMV_PREDIGEST_RF == pstSystem->cTransAttr)
		{
			pstEMVOption->_request_amt = EMV_TRANS_REQAMT_NO;
			pstEMVOption->_seq_to = EMV_PROC_CONTINUE;
			pstEMVOption->_trans_type = EMV_TRANS_BALANCE;
		}
		break;
	case TRANS_BALANCE:
		pstEMVOption->_request_amt = EMV_TRANS_REQAMT_APS;
		pstEMVOption->_force_online_enable = TRUE;
		pstEMVOption->_trans_type = EMV_TRANS_BALANCE;
		if(pstSystem->cTransAttr == ATTR_EMV_PREDIGEST_RF)
		{
			pstEMVOption->_trans_type = EMV_TRANS_RF_SALE;
			pstEMVOption->_seq_to = EMV_PROC_CONTINUE;
			pstEMVOption->_request_amt = EMV_TRANS_REQAMT_NO;
		}	
		break;
	default:
		break;
	}
TRACE((""));

	if (ATTR_qPBOC == pstSystem->cTransAttr
	        || ATTR_EMV_STANDARD_RF == pstSystem->cTransAttr
	        || ATTR_PBOC_EC_RF == pstSystem->cTransAttr
	        || ATTR_EMV_PREDIGEST_RF == pstSystem->cTransAttr
	   )   /**Contactless*/
	{
		stEntryPointOpt._request_amt = pstEMVOption->_request_amt;
		stEntryPointOpt._seq_to = pstEMVOption->_seq_to;
		stEntryPointOpt._force_online_enable = pstEMVOption->_force_online_enable;
		stEntryPointOpt._trans_type = pstEMVOption->_trans_type;
		// 20170925 MCCS auth
		if (pstSystem->cTransType == TRANS_SALE && pstSystem->cFlagCashback == 1)
		{
			stEntryPointOpt._trans_type = 0x09;		
		}
		stEntryPointOpt._ctrl = EMV_EP_CTRL_CK_CANDIDATE_LIST;
		nRet = EmvEntryPointStart(&stEntryPointOpt, gullAmount, gullCashbackAmount);
TRACE(("nRet=%d", nRet));
		if( nRet == EMV_INITAPP_QUIT)
		{
			return EMV_INITAPP_QUIT;
		}
		nEmvRes = stEntryPointOpt._trans_ret;
TRACE(("======nEmvRes=%d", nEmvRes));
		TRACE(("nEmvRes=%d", nEmvRes));
		if (ATTR_EMV_PREDIGEST_RF== pstSystem->cTransAttr
		        || ATTR_qPBOC== pstSystem->cTransAttr
		   ) 
		{
			if (APP_SUCC != EmvEntryPointSuspend(nEmvRes))
			{
				ShowLightFail();
				DispOutICC(NULL, "qPBOC Fail", "");
				return EMV_INITAPP_FAIL;
			}
			if (TRANS_BALANCE == pstSystem->cTransType
			        ||( EMV_TRANS_QPBOC_ACCEPT != nEmvRes && EMV_TRANS_QPBOC_GOONLINE != nEmvRes &&
			        0 != nEmvRes
			        && EMV_TRANS_MSD_GOONLINE != nEmvRes)
			   )
			{
				return nEmvRes;
			}
			else
			{
				goto GETDATA;
			}

		}

		if (nEmvRes == EMV_TRANS_FALLBACK)
		{
			DispOutICC(NULL, "NOT ACCEPTED", "PLS SWIPE");
			return APP_FALLBACK;
		}
		else if (nEmvRes == EMV_TRANS_TERMINATE)
		{
			EMV_rf_suspend(nEmvRes);
			DispOutICC(NULL, "TRANS SUSPEND", EmvErrMsg(EMV_ErrorCode()));
			return APP_QUIT;
		}
		else if (nEmvRes != APP_SUCC)
		{
			if (nEmvRes == (-1521))
			{
				TRACE(("nRet = %d, This Card Is Pure EC but cannot get9F74", nEmvRes));
			}
			else
			{
				EMV_rf_suspend(nEmvRes);
				DispOutICC(NULL, "READ FAILED", EmvErrMsg(EMV_ErrorCode()));
				return nEmvRes;
			}
		}
	}
	else   /**Contact*/
	{
		PubClear2To4();
		PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1,"PROCESSING...");
		PubUpdateWindow();
		memcpy(pstSystem->szInputMode, "052", 3);

TRACE(("EMV_Start ============"));
		nEmvRes = EMV_Start(pstEMVOption);
TRACE(("nEmvRes: %d", nEmvRes));
		if (ATTR_EMV_PREDIGEST == pstSystem->cTransAttr)
		{
			/**q流程交易直接返回外面处理*/
			if (APP_SUCC != EMV_Suspend(nEmvRes))
			{
				DispOutICC(NULL, "TRANS FAIL", EmvErrMsg(EMV_ErrorCode()));
				return EMV_INITAPP_FAIL;
			}
			if (TRANS_BALANCE == pstSystem->cTransType)
			{
				return nEmvRes;
			}
		}

		/**<标准流程交易*/
		if (nEmvRes == EMV_TRANS_FALLBACK)
		{
			EMV_Suspend(nEmvRes);
			DispOutICC(NULL, "NOT ACCEPTED", "PLS SWIPE");
			return APP_FALLBACK;
		}
		else if (nEmvRes == EMV_TRANS_TERMINATE)
		{
			EMV_Suspend(nEmvRes);
			DispOutICC(NULL, "SUSPEND", EmvErrMsg(EMV_ErrorCode()));
			return APP_QUIT;
		}
		else if (nEmvRes != APP_SUCC)
		{
			if (nEmvRes == (-1521))	/**<该卡是纯电子现金但是读不到9F74*/
			{
				EMV_Suspend(nEmvRes);
				DispOutICC(NULL, "DECLINE", "No auth code in EC-ONLY card");
				return APP_QUIT;
			}
			else
			{
				EMV_Suspend(nEmvRes);
				DispOutICC(NULL, "READ ERR", EmvErrMsg(EMV_ErrorCode()));
				return APP_FAIL;
			}
		}

	}
	/**纯电子现金卡判断*/
	if (TRUE == cIsCheckOnlyEC)
	{
		if(APP_SUCC != CheckIsOnlyEC())
		{
			if (ATTR_qPBOC == pstSystem->cTransAttr
			        || ATTR_EMV_STANDARD_RF == pstSystem->cTransAttr
			        || ATTR_PBOC_EC_RF == pstSystem->cTransAttr
			        || ATTR_EMV_PREDIGEST_RF == pstSystem->cTransAttr)
			{
				EMV_rf_suspend(-1);
			}
			else
			{
				EMV_Suspend(-1);
			}
			return APP_FAIL;
		}
	}
GETDATA:
	ASSERT_FAIL(EmvGetData(pstSystem));

	if (ATTR_qPBOC == pstSystem->cTransAttr) //qPboc返回外面处理
	{
		memcpy(pstSystem->szInputMode, "071", 3);
		return nEmvRes;
	}
	return APP_SUCC;

}
/**
* @brief 基于EMV的借贷记卡流水附加信息
* @param in out pstTransRecord STTAANSRECORD流水结构指针
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvAddtionWater(STTAANSRECORD *pstTransRecord)
{
	int nLen;
	uchar sBuff[256]={0};
	uchar szBuff[256+1]={0};
	
	STEMVADDTIONWATERINFO *pstEMVAddtionWaterInfo = (STEMVADDTIONWATERINFO *)pstTransRecord->sAddition;

#if 0 //only for debug
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_50_IC_APPLABEL, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->szAIDLable, sBuff, nLen);
	pstEMVAddtionWaterInfo->szAIDLable[nLen] = 0;

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_57_IC_TRACK2EQUDATA, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->sEqTrack2_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->sEqTrack2_wl+1, sBuff, nLen);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_5F2A_TM_CURCODE, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTransCurrentCode, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_5F34_IC_PANSN, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cPanSeqNum = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_8A_TM_ARC, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sResponseCode, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_82_IC_AIP, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sAIP, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_8F_IC_CAPKINDEX, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cPubKeyIndex = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_91_TM_ISSAUTHDT, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->sIssAuthData_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->sIssAuthData_wl+1, sBuff, nLen);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_95_TM_TVR, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTVR, sBuff, 5);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9B_TM_TSI, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTSI, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9A_TM_TRANSDATE, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTxnData, sBuff, 3);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9C_TM_TRANSTYPE, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cTxnType = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F02_TM_AUTHAMNTN, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sAuthAmt, sBuff, 6);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F03_TM_OTHERAMNTN, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sOtherAmt, sBuff, 6);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F0D_IC_IAC_DEFAULT, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sIACDefault, sBuff, 5);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F0E_IC_IAC_DENIAL, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sIACDenial, sBuff, 5);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F0F_IC_IAC_ONLINE, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sIACOnline, sBuff, 5);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F10_IC_ISSAPPDATA, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->sIAD_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->sIAD_wl+1, sBuff, nLen);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F11_IC_ISSCTINDEX, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cIssCodeTbl = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F15_TM_MCHCATCODE, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sMerchantCatCode, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F1A_TM_CNTRYCODE, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTerminalCountryCode, sBuff, 2);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F20_IC_TRACK2DATA, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->sTrack2DscData_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->sTrack2DscData_wl+1, sBuff, nLen);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F27_IC_CID, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sCryptInfoData, sBuff, 8);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F33_TM_CAP, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTermCap, sBuff, 3);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F34_TM_CVMRESULT, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sCVM, sBuff, 3);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(0x9F5B, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->szIssuerScriptResult1_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->szIssuerScriptResult1_wl+1, sBuff, nLen);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(0x71, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->szIssuerScriptResult2_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->szIssuerScriptResult2_wl+1, sBuff, nLen);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(0x72, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->szIssuerScriptResult3_wl[0] = nLen;
	memcpy(pstEMVAddtionWaterInfo->szIssuerScriptResult3_wl+1, sBuff, nLen);
#endif
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_84_IC_DFNAME, sBuff, sizeof(sBuff));
	if (nLen > 0)
	{
		memcpy(pstEMVAddtionWaterInfo->sAID, sBuff, nLen);
		pstEMVAddtionWaterInfo->cAIDLen =(char)nLen;
	}
	else
	{
		nLen = EmvEntryPointGetData(_EMV_TAG_9F06_TM_AID, sBuff, sizeof(sBuff));
		memcpy(pstEMVAddtionWaterInfo->sAID, sBuff, nLen);
		pstEMVAddtionWaterInfo->cAIDLen =(char)nLen;
	}
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_95_TM_TVR, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTVR, sBuff, 5);
	
	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_50_IC_APPLABEL, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->szAIDLable, sBuff, nLen);
	pstEMVAddtionWaterInfo->szAIDLable[nLen] = 0;

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F26_IC_AC, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sTC, sBuff, 8);


	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F36_IC_ATC, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sATC, sBuff, 2);


	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F12_IC_APNAME, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->szAppPreferedName_wl[nLen] = 0;
	memcpy(pstEMVAddtionWaterInfo->szAppPreferedName_wl+1, sBuff, nLen);

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(0xdf42, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cSignatureFlag = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(0xdf41, sBuff, sizeof(sBuff));
	pstEMVAddtionWaterInfo->cForceAcceptFlag = sBuff[0];

	memset(sBuff, 0, sizeof(sBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F37_TM_UNPNUM, sBuff, sizeof(sBuff));
	memcpy(pstEMVAddtionWaterInfo->sUnpNum, sBuff, 4);

	memset(sBuff, 0, sizeof(sBuff));
	GetData9F26RQ((char *)sBuff);
	memcpy(pstEMVAddtionWaterInfo->sARQC, sBuff, 8);

	//发卡行脚本结果
	memset(szBuff,0,sizeof(szBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_DF31_IC_IISSCRIRES, (uchar *)szBuff, sizeof(szBuff));
	pstEMVAddtionWaterInfo->nIssuerScriptResultLen = nLen ;
	memcpy(pstEMVAddtionWaterInfo->szIssuerScriptResult, szBuff, nLen) ;
	TRACE_HEX(((char *)szBuff, nLen,"_EMV_TAG_DF31_IC_IISSCRIRES[%d]:", nLen));
	
	memset(szBuff,0,sizeof(szBuff));
	nLen = EmvEntryPointGetData(_EMV_TAG_9F27_IC_CID, (uchar *)szBuff, sizeof(szBuff));
	pstEMVAddtionWaterInfo->cCrmInfoData = szBuff[0] ;
	TRACE_HEX(((char *)szBuff, nLen,"_EMV_TAG_9F27_IC_CID[%d]:", nLen));

	
	pstTransRecord->nAdditionLen = sizeof(STEMVADDTIONWATERINFO);
	
	PubDelFile(FILE_ICCLOG);
	SaveToFile(pstTransRecord->sAddition, 0, pstTransRecord->nAdditionLen, FILE_ICCLOG);
	return APP_SUCC;
}

void EmvDispTvrTsi(void)
{
	char sTVR[5+1];
	char sTSI[2+1];
	char sCVR[12+1];
	int nRet = 0;
	char sBuf[128] = {0};
	char sCVM[3+1] = {0};

	if (YES != GetVarIsDispEMV_TVRTSI())
	{
		return ;
	}

	nRet = EmvEntryPointGetData(_EMV_TAG_95_TM_TVR, (uchar *)sTVR ,sizeof(sTVR));
	if (nRet != 5)
	{
		return ;
	}
	nRet = EmvEntryPointGetData(_EMV_TAG_9B_TM_TSI, (uchar *)sTSI, sizeof(sTSI));
	if (nRet != 2)
	{
		return ;
	}
#if 0
	nRet = EmvEntryPointGetData(_EMV_TAG_9F10_IC_ISSAPPDATA, (uchar *)sBuf, sizeof(sBuf));
	if (nRet <= 0)
	{
		return ;
	}
	memcpy(sCVR, sBuf, 8);
	
	nRet = EmvEntryPointGetData(_EMV_TAG_9F34_TM_CVMRESULT, (uchar *)sCVM, sizeof(sCVM));
	if (nRet <= 0)
	{
		return ;
	}
#endif	
	NDK_ScrPush();
	PubClearAll();
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 2, 1, "TVR:%02X%02X%02X%02X%02X",  sTVR[0], sTVR[1],
	               sTVR[2], sTVR[3], sTVR[4]);
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 3, 1, "TSI:%02X%02X",  sTSI[0], sTSI[1]);

//	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 4, 1, "CVR:%02X%02X%02X%02X%02X%02X%02X%02X",  sCVR[0], sCVR[1],
//	               sCVR[2], sCVR[3], sCVR[4], sCVR[5], sCVR[6], sCVR[7]);
//	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 5, 1, "CVM:%02X%02X%02X",  sCVM[0], sCVM[1], sCVM[2]);
#if 0
	memcpy(sCVR, sBuf+3, 5);
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 4, 1, "CVR:%02X%02X%02X%02X%02X",  sCVR[0], sCVR[1],
	               sCVR[2], sCVR[3], sCVR[4]);
#endif	
	PubUpdateWindow();
	PubGetKeyCode(0);
	NDK_ScrPop();
TRACE(("1,%d", nRet));
	return ;
}

/**
* Interface function implementation
*/

/**
* @brief 支持PBOC标准借贷记和QPBOC消费的流程
* @param pnInputMode 获取卡号模式
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvTxnSale(int* pnInputMode, int cFlagCashback)
{
	char cTransType;				/**<交易类型*/
	char *pszTitle = "SALE";
	int nTrueInput = 0;
	STSYSTEM stSystem;

	nTrueInput = *pnInputMode;
	memset(&stSystem,0,sizeof(STSYSTEM));
	cTransType = TRANS_SALE;
	
	stSystem.cTransType = (char)cTransType;
	stSystem.cFlagCashback = (char)cFlagCashback;		
	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, cTransType, 0, YES));

	/**
	* Input Amount
	*/
	ASSERT_FAIL(EdcGetAmout(pszTitle, &stSystem));

	ASSERT_QUIT(Emv_Common_InputCard(pszTitle, &stSystem, &nTrueInput));
	
	TRACE(("stSystem.cTransAttr=%d nTrueInput=%d",stSystem.cTransAttr, nTrueInput));

	if(INPUT_INSERTIC == nTrueInput)
	{
		ASSERT(EmvSale(&stSystem));
	}
	else if(INPUT_STRIPE == nTrueInput || INPUT_KEYIN == nTrueInput)
	{
		ASSERT(MagSale(&stSystem));
	}
	else if (INPUT_RFCARD == nTrueInput)
	{
		ASSERT(qPbocSale(&stSystem));
	}
	
	DISP_OUT_ICC;
	DoAdviseAndReveral();
	return APP_SUCC;
}

/**
* Interface function implementation
*/
/**
* @brief 支持PBOC标准借贷记插卡和挥卡的流程
* @param in pnInputMode 获取卡号模式
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvTxnPreAuth(int* pnInputMode)
{
	char *pszTitle = "PREAUTH";
	int nTrueInput;
	STSYSTEM stSystem;

	memset(&stSystem,0,sizeof(STSYSTEM));
	stSystem.cTransType = TRANS_PREAUTH;
	nTrueInput = *pnInputMode;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, 0, YES));

	/**
	* Input Amount
	*/
	ASSERT_FAIL(EdcGetAmout(pszTitle, &stSystem));

	ASSERT_QUIT(Emv_Common_InputCard(pszTitle, &stSystem, &nTrueInput));

	if (stSystem.nAcqType == EM_ACQTYPE_MCCS)
	{
		PubMsgDlg(pszTitle, "DEBIT NONSUPPORT", 3, 3);
		return APP_QUIT;
	}
	
	if(INPUT_INSERTIC == nTrueInput || INPUT_RFCARD == nTrueInput)
	{
		TRACE(("stSystem.szAmount=%s",stSystem.szAmount));
		ASSERT(EmvPreAuth(&stSystem));
	}
	else if(INPUT_STRIPE == nTrueInput || INPUT_KEYIN == nTrueInput)
	{
		ASSERT(MagPreAuth(&stSystem));
	}
	DISP_OUT_ICC;
	return APP_SUCC;
	
}

int EmvTxnBalance()
{
	char *pszTitle = "BALANCE";
	int nTrueInput = INPUT_NO;
	STSYSTEM stSystem;

	memset(&stSystem,0,sizeof(STSYSTEM));
	stSystem.cTransType = TRANS_BALANCE;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, 0, YES));

	ASSERT_QUIT(Emv_Common_InputCard(pszTitle, &stSystem, &nTrueInput));

	if(INPUT_INSERTIC == nTrueInput || INPUT_RFCARD == nTrueInput)
	{
		ASSERT(EmvBalance(&stSystem));
	}
	else if(INPUT_STRIPE == nTrueInput || INPUT_KEYIN == nTrueInput)
	{
		ASSERT(MagBalance(&stSystem));
	}
	
	DISP_OUT_ICC;
	return APP_SUCC;
	
}

int CheckInputPin(STSYSTEM *pstSystem)
{
	int nRet;
	uchar sTVR[5+1];

	memset(sTVR, 0, sizeof(sTVR));
	nRet = EmvEntryPointGetData(_EMV_TAG_95_TM_TVR, sTVR, sizeof(sTVR));
	TRACE(("nRet=%d", nRet));
	TRACE_HEX(((char *)sTVR,5,"_EMV_TAG_95_TM_TVR:"));
	if (nRet != 5)
	{
		return APP_FAIL;
	}
	
#if 0//打开表示，emv库调过，应用强制输入
	if(!(sTVR[2] & 0x1C))/**<EMV库跳过输入pin处理 */
	{
		//if (pstSystem->nAcqType == EM_ACQTYPE_AMEX)//amex 不用特殊处理
		{
			ASSERT_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
		}
	}
#endif	
	return APP_SUCC;
	
}

/**
* @brief 基于EMV的插卡、刷卡函数
* @param [in] pstSystem STSYSTEM结构指针
* @return
* @li APP_SUCC		
* @li APP_FAIL
* @author 
* @date 
*/
int Emv_Common_InputCard(const char *pszTitle, STSYSTEM *pstSystem, int *pnTrueInput)
{
	int nRet = 0;
	char cInput = INPUT_INSERTIC;

	if(YES==GetVarIsCardInput())
	{
		cInput |= INPUT_KEYIN;
	}
	if(YES == GetVarIsSwipe())
	{
		cInput |= INPUT_STRIPE;
	}	
	if(YES == GetVarIsSupportRF() && TRANS_SALE == pstSystem->cTransType)
	{
		cInput |= INPUT_RFCARD;
		ShowLightReady();
	}
	
	while(1)
	{
		if (*pnTrueInput == 0)
		{
			ASSERT_RETURNCODE(CardTaskAmt(NULL,cInput, STRIPE_TIMEOUT,pnTrueInput));
		}
		if(INPUT_INSERTIC == *pnTrueInput)
		{		
			pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD;
			SetnIsDefaultEC(TRUE);			/*不使用电子现金,不再进行选择*/
TRACE(("====nRet: %d[*pnTrueInput: %d]", nRet, *pnTrueInput));
			nRet = EmvInitApp(pstSystem);
TRACE(("====nRet: %d[*pnTrueInput: %d]", nRet, *pnTrueInput));
			if(APP_SUCC == nRet)
			{
				break;
			}
			else if ((APP_FALLBACK ==nRet) && (YES == GetVarIsIcFlag())
				|| -2 == nRet)
			{
				cInput = INPUT_STRIPE_FALLBACK;
				pstSystem->cTransAttr = ATTR_FALLBACK;
				*pnTrueInput = 0;
				continue;
			}
			else
			{
				TRACE(("nRet=%d", nRet));
				return APP_FAIL;
			}
		}
		else if(INPUT_STRIPE == *pnTrueInput)
		{
			GetTrack(pstSystem->szTrack1, pstSystem->szTrack2, pstSystem->szTrack3);
			GetPan(pstSystem->szPan);
			GetNameFromTk1(pstSystem->szTrack1, pstSystem->szHolderName);
			GetExpDateFromTk2(pstSystem->szTrack2, pstSystem->szExpDate);
			/*IC卡检测*/
			if (YES == GetVarIsIcFlag() && pstSystem->cTransAttr != ATTR_FALLBACK)
			{
				nRet = ChkIsICC(pstSystem->szTrack2, NULL) ;
				if (nRet == APP_SUCC)
				{
					cInput = INPUT_INSERTIC_FORCE;
					*pnTrueInput = 0;
					continue;
				}
			}
			if(pstSystem->cTransAttr != ATTR_FALLBACK)
			{
				pstSystem->cTransAttr = (char)ATTR_MAG;
				memcpy(pstSystem->szInputMode, "021", 3);
			}
			else
			{
				memcpy(pstSystem->szInputMode, "801", 3);		
			}
			break;
		}
		else if(INPUT_KEYIN == *pnTrueInput)
		{
			pstSystem->cTransAttr = (char)ATTR_MANUAL;
			GetTrack(NULL, pstSystem->szTrack2, pstSystem->szTrack3);
			GetPan(pstSystem->szPan);
			memcpy(pstSystem->szInputMode, "011", 3);
			break;
		}
		else if (INPUT_RFCARD == *pnTrueInput)
		{
			break;
		}
	}
	if (*pnTrueInput == INPUT_KEYIN || *pnTrueInput == INPUT_STRIPE)
	{
		GetAcqTypeFormPan(pszTitle, pstSystem->szPan, &pstSystem->nAcqType, pstSystem->cTransAttr);
		GetAcqIndex(pstSystem);
	}

	SetDE22EntryMode(pstSystem->szInputMode, pstSystem->nAcqIndex, pstSystem->cTransAttr);

	//for master special
	if (pstSystem->nAcqType == EM_ACQTYPE_MASTER && pstSystem->cTransAttr == ATTR_FALLBACK)
	{
		pstSystem->szInputMode[0] = '9';
	}
	TRACE(("=================common input=%s",pstSystem->szInputMode));
	return APP_SUCC;
}

/**
* @brief 基于EMV的插卡、刷卡函数
* @param [in] pstSystem STSYSTEM结构指针
* @return
* @li APP_SUCC		
* @li APP_FAIL
* @author 
* @date 
*/
int Emv_Simple_InputCard(const char *pszTitle, STSYSTEM *pstSystem, int *pnTrueInput)
{
	int nRet = 0;
	char cInput = INPUT_INSERTIC;
	
	if(YES == GetVarIsSwipe())
	{
		cInput |= INPUT_STRIPE;
	}	
	
#if 0 //20170320 can not support wave
	if(YES == GetVarIsSupportRF() && YES == GetVarIsIcFlag())
	{
		cInput |= INPUT_RFCARD;
		ShowLightReady();
	}
#endif

	if (YES == GetVarIsCardInput())
	{
		cInput |= INPUT_KEYIN;
	}
	
	while(1)
	{
		ASSERT_RETURNCODE(CardTaskAmt(NULL,cInput, STRIPE_TIMEOUT,pnTrueInput));
		if(INPUT_INSERTIC == *pnTrueInput)
		{
			pstSystem->cTransAttr = (char)ATTR_EMV_PREDIGEST;		
			SetnIsDefaultEC(TRUE);			/*不使用电子现金,不再进行选择*/
			emv_opt stEMVOption;
			memset(&stEMVOption, 0, sizeof(stEMVOption));
			stEMVOption._request_amt = EMV_TRANS_REQAMT_NO;
			stEMVOption._seq_to = EMV_PROC_TO_READAPPDATA;
			nRet = EmvReadICC(&stEMVOption, pstSystem);
			SaveEmvTransSerial();
			EMV_Suspend(nRet);
			if (nRet == APP_FALLBACK && (YES == GetVarIsIcFlag()))
			{
				cInput = INPUT_STRIPE_FALLBACK;
				pstSystem->cTransAttr = (char)ATTR_FALLBACK;
				continue;
			}
			else if (nRet == APP_SUCC)
			{
				/*纯电子现金不能做联机交易,除了充值类,应用判断linjz-20121219*/
				ASSERT_FAIL(CheckIsOnlyEC());
				pstSystem->cTransAttr = (char)ATTR_EMV_PREDIGEST;
				break;
			}
			else
			{
				TRACE(("nRet=%d", nRet));
				return APP_FAIL;
			}
		}
		else if(INPUT_STRIPE == *pnTrueInput)
		{
			pstSystem->cTransAttr = (char)ATTR_MAG;
			GetTrack(pstSystem->szTrack1, pstSystem->szTrack2, pstSystem->szTrack3);
			GetExpDateFromTk2(pstSystem->szTrack2, pstSystem->szExpDate);
			GetPan(pstSystem->szPan);
			/*IC卡检测*/
			if (YES == GetVarIsIcFlag() && pstSystem->cTransAttr != ATTR_FALLBACK)
			{
				nRet = ChkIsICC(pstSystem->szTrack2, NULL) ;
				if (nRet == APP_SUCC)
				{
					cInput = INPUT_INSERTIC_FORCE;
					continue;
				}
			}
			memcpy(pstSystem->szInputMode, "801", 3);
			if(pstSystem->cTransAttr != ATTR_FALLBACK)
			{
				memcpy(pstSystem->szInputMode, "021", 3);
			}
			break;
		}
		else if(INPUT_KEYIN == *pnTrueInput)
		{
			pstSystem->cTransAttr = (char)ATTR_MANUAL;
			memcpy(pstSystem->szInputMode, "011", 3);
			GetTrack(NULL, pstSystem->szTrack2, pstSystem->szTrack3);
			GetPan(pstSystem->szPan);
			ASSERT_HANGUP_QUIT(GetExpiryDate(pszTitle, pstSystem->szExpDate));	
			break;
		}
		else if (INPUT_RFCARD == *pnTrueInput)
		{
			pstSystem->cTransAttr = (char)ATTR_EMV_PREDIGEST_RF;
			memcpy(pstSystem->szInputMode, "071", 3);
			
			SetnIsDefaultEC(TRUE);			/*不使用电子现金,不再进行选择*/
			emv_opt stEMVOption;
			memset(&stEMVOption, 0, sizeof(stEMVOption));
			stEMVOption._request_amt = EMV_TRANS_REQAMT_NO;
			stEMVOption._seq_to = EMV_PROC_TO_READAPPDATA;
			nRet = EmvReadICC(&stEMVOption, pstSystem);
			SaveEmvTransSerial();
			EMV_Suspend(nRet);
			if (nRet == APP_FALLBACK && (YES == GetVarIsIcFlag()))
			{
				cInput = INPUT_STRIPE_FALLBACK;
				pstSystem->cTransAttr = (char)ATTR_FALLBACK;
				continue;
			}
			else if (nRet == APP_SUCC)
			{
				/*纯电子现金不能做联机交易,除了充值类,应用判断linjz-20121219*/
				ASSERT_FAIL(CheckIsOnlyEC());
				//pstSystem->cTransAttr = (char)ATTR_EMV_PREDIGEST;
				break;
			}
			else
			{
				TRACE(("nRet=%d", nRet));
				return APP_FAIL;
			}
			break;
		}
	}
	if (*pnTrueInput == INPUT_KEYIN || *pnTrueInput == INPUT_STRIPE)
	{
		GetAcqTypeFormPan(pszTitle, pstSystem->szPan, &pstSystem->nAcqType, pstSystem->cTransAttr);
		GetAcqIndex(pstSystem);
	}
	SetDE22EntryMode(pstSystem->szInputMode, pstSystem->nAcqIndex, pstSystem->cTransAttr);
	TRACE(("============== simple =%s",pstSystem->szInputMode));
	return APP_SUCC;
}

/**
* @brief 支持PBOC标准借贷记插卡和挥卡的流程
* @param in pnInputMode 获取卡号模式
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvTxnInstallment()
{
	char *pszTitle = "INSTALLMENT";
	int nTrueInput;
	STSYSTEM stSystem;

	memset(&stSystem,0,sizeof(STSYSTEM));
	stSystem.cTransType = TRANS_INSTALLMENT;
	nTrueInput = INPUT_NO;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, 0, YES));

	/**
	* Input Amount
	*/
	ASSERT_FAIL(EdcGetAmout(pszTitle, &stSystem));
	ASSERT_QUIT(Emv_Common_InputCard(pszTitle, &stSystem, &nTrueInput));

	if (EM_ACQINDEX_UPI == stSystem.nAcqIndex
		|| EM_ACQINDEX_MCCS == stSystem.nAcqIndex)
	{
		PubMsgDlg(pszTitle, "Nonsupport For The Card", 3, 3);
		return APP_QUIT;
	}
	if(INPUT_INSERTIC == nTrueInput)
	{
		TRACE(("stSystem.szAmount=%s",stSystem.szAmount));
		ASSERT(EmvInstallment(&stSystem));
	}
	else if(INPUT_STRIPE == nTrueInput || INPUT_KEYIN == nTrueInput)
	{
		ASSERT(MagInstallment(&stSystem));
	}
	DISP_OUT_ICC;
	DoAdviseAndReveral();
	return APP_SUCC;
	
}

/**
* @brief Emv Preauth
* @param in pstSystem
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvInstallment(STSYSTEM *pstSystem)
{
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char szRecvFiled55[260+1];
	int nFieldLen, nInputPinNum = 0;
	int nRet = 0;
	char *pszTitle = "INSTALLMENT";
	char cTSICompleted = FALSE;
	//int nSel = 0;
	int nLen = 0;
	
	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	pstSystem->cTransType = (char)TRANS_INSTALLMENT;
	pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD;

	if(memcmp(pstSystem->szInputMode,"07",2) ==0)
	{
		pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD_RF;
	}
	
	PubDisplayTitle(pszTitle);

	/**
	* Advance dial
	*/

	CommPreDial();

	PubDisplayTitle(pszTitle);
	ASSERT_HANGUP_QUIT(DispPan(pstSystem->szPan, DISPPAN_TIMEOUT));

	/**
	* Select payment term
	*/
	ASSERT_HANGUP_QUIT(SelectEppMonth(pszTitle, &pstSystem->nInstallmentMonth));
	UpdateAcqIndexFromEpp(&pstSystem->nAcqIndex, pstSystem->nInstallmentMonth);
	ASSERT_HANGUP_QUIT(CheckEppAmt(pszTitle, pstSystem->szAmount, pstSystem->nInstallmentMonth));
	PubDisplayTitle(pszTitle);
	
	EmvSetTxnPan(pstSystem->szPan);

	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0200", 4);
	memcpy(pstSystem->szProcCode, "000000", 6);//003000
	memcpy(pstSystem->szServerCode, "00", 2);


	/**
	* Process Flow
	*/
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	nRet = EmvProcessApp(pstSystem->szAmount, pstSystem->cTransAttr);
	if (APP_SUCC != nRet)
	{
		CommHangUp();
		return nRet;
	}
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));

	/**
	* Get Online Pin
	*/
	EmvGetOnlinePin(pstSystem->szPin);

	ASSERT_HANGUP_QUIT(CheckInputPin(pstSystem));
	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	if(nInputPinNum > 0)
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);
	pstSystem->cHasPinInput = GetIsKeyInPIN();

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());

	PubDisplayTitle(pszTitle);

	/**
	* Dialing
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/*
	* Dial successfully
	*/
	EmvPackField55(EMV_PREAUTH, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);//pstSystem->nAcqType
	SaveData9F26RQ();
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	/**
	* Begin to pack
	*/
	nRet = PackGeneral(pstSystem, NULL);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		DispOutICC(pszTitle, "PACKED FAIL", "");
		return APP_FAIL;
	}
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);
	EmvPackField55(EMV_REVERSAL, stReversal.szFieldAdd1, &stReversal.nFieldAdd1Len, stReversal.nAcqIndex);
	nInputPinNum++;
	
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	if(ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL|DPC_NOCHECKRESP,pstSystem,&stReversal,nInputPinNum);
	}
	else
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);		
	}
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		return APP_FAIL;
	}

	if (ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{	
		/**
		* Get Field 55
		*/
		memset(szRecvFiled55, 0, sizeof(szRecvFiled55));
		nFieldLen = 255;
		GetField(55, szRecvFiled55+2, &nFieldLen);
		PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);


#ifdef DEMO
#else	
		ASSERT_HANGUP_FAIL(EmvCompleteApp(pszTitle,TRUE,pstSystem,&stReversal,szRecvFiled55,nFieldLen, &cTSICompleted, nInputPinNum));
#endif
	}
	
#ifdef DEMO
	sprintf(pstSystem->szInstallmentInfo, "001%02d1001010000000086300000086320000086300000517820000500000000017820000000000000", pstSystem->nInstallmentMonth);
#else
	nLen = 99;
	ASSERT_QUIT(GetField(61, pstSystem->szInstallmentInfo, &nLen));
#endif	
	pstSystem->cEMV_Status = EMV_STATUS_ONLINE_SUCC;

	EmvSaveWater(TRUE, pstSystem);
	SysToRecord(pstSystem, &stTransRecord);
	EmvAddtionWater(&stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);

}

int GetAcqIndex(STSYSTEM *pstSystem)
{
	switch (pstSystem->nAcqType)
	{
	case EM_ACQTYPE_VISA:
	case EM_ACQTYPE_MASTER:
	case EM_ACQTYPE_JCB:
		if (pstSystem->cTransType == TRANS_INSTALLMENT || pstSystem->cTransType == TRANS_VOID_INSTALLMENT)
		{
			pstSystem->nAcqIndex = 4;
		}
		else
		{
			pstSystem->nAcqIndex = 0;
		}
		break;
	case EM_ACQTYPE_AMEX:
		if (pstSystem->cTransType == TRANS_INSTALLMENT || pstSystem->cTransType == TRANS_VOID_INSTALLMENT)
		{
			pstSystem->nAcqIndex = 5;
		}
		else
		{
			pstSystem->nAcqIndex = 1;
		}
		break;
	case EM_ACQTYPE_UPI:
		pstSystem->nAcqIndex = 3;
		break;
	case EM_ACQTYPE_MCCS:
		pstSystem->nAcqIndex = 2;
		break;
	default:
		break;
	}

	return APP_SUCC;
	
}

int GetResponseFieldDE55(int nAcqIndex, char *pszDe55, int *pnLen)
{	
	char szRecvFiled55[512] = {0};
	int nFieldLen = 0;

//return 0;//shtest	
	/**
	* Get Field 55
	*/
	memset(szRecvFiled55, 0, sizeof(szRecvFiled55));
	nFieldLen = 512;

	if (nAcqIndex == 1)
	{
		int nTagLen = 0;
		int nPos = 0;
		int nPosOriginal = 0;
		
		GetField(55, szRecvFiled55, &nFieldLen);
		if (nFieldLen <= 0)
		{
			return APP_QUIT;
		}
#if 0
PubDebug("nlen= %d", nFieldLen);
		pszDe55[2] = 0x91;
		memcpy(pszDe55+3, szRecvFiled55 + 6, nFieldLen - 6);
		*pnLen = nFieldLen-6+1;
		PubIntToC2((uchar *)pszDe55, (uint)*pnLen);
PubDebugData("Amex 55: ", pszDe55, *pnLen+2);		
#endif	
		nPosOriginal += 6;
		nPos += 2; // 方长度
		nTagLen = szRecvFiled55[nPosOriginal];
		//91
		if (nTagLen > 0)
		{
			pszDe55[nPos] = 0x91;
			nPos += 1;

			pszDe55[nPos] = nTagLen;
			nPos += 1;
			nPosOriginal += 1;
			
			memcpy(pszDe55 + nPos, szRecvFiled55+nPosOriginal, nTagLen);
			nPosOriginal += nTagLen;
			nPos += nTagLen;

			//脚本
			nTagLen = szRecvFiled55[nPosOriginal];
			if (nTagLen > 0)
			{
				nPosOriginal++;
				
				pszDe55[nPos] = szRecvFiled55[nPosOriginal];
				nPos++;
				nPosOriginal++;

				nTagLen = szRecvFiled55[nPosOriginal];	
				pszDe55[nPos] = nTagLen;
				nPosOriginal++;
				nPos++;
					
				memcpy(pszDe55 + nPos, szRecvFiled55+nPosOriginal, nTagLen);
				nPosOriginal += nTagLen;
				nPos += nTagLen;
			}
			
			*pnLen = nPos - 2;
		}
		
		
		PubIntToC2((uchar *)pszDe55, (uint)*pnLen);
PubDebugData("=Amex 55: ", pszDe55, *pnLen+2);		
	}
	else
	{
		GetField(55, szRecvFiled55+2, &nFieldLen);
		if (nFieldLen <= 0)
		{
			return APP_QUIT;
		}
		PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);
		memcpy(pszDe55, szRecvFiled55, nFieldLen+2);
		*pnLen = nFieldLen;
	}
	return APP_SUCC;
}


/**
* @brief 基于借贷记非接触快速消费
* @param in pcInputMode 触发的刷卡介质方式
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int qPbocSale(STSYSTEM *pstSystem)
{
	STSYSTEM stSystem, stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char cOnlineResult = FALSE;/* 联机结果,0链接失败,非0成功*/
	int nRet = 0;
	char *pszTitle = "SALE";
	char szMsg[256];
	int nEmvRes = 0;
	uchar sEcBalance[13];
	int nEcBalanceLen;
	int nInputPinNum = 0;
	char szAid[20+1];
	char szContent[100];
	char szMsgBuf[256] = {0};

	memset(szContent, 0, sizeof(szContent));
	memset(&stSystem, 0, sizeof(STSYSTEM));
	memset(&stReversal, 0, sizeof(STREVERSAL));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));

	memcpy(&stSystem, pstSystem, sizeof(STSYSTEM));
	
	stSystem.cTransType = (char)TRANS_SALE;
	stSystem.cTransAttr = (char)ATTR_qPBOC;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, ATTR_PBOC_EC_RF, YES));

	EmvClrOnlinePin();

	/**
	* 执行流程
	*/
	nEmvRes = EmvInitApp(&stSystem);
	
	SetDE22EntryMode(stSystem.szInputMode, stSystem.nAcqIndex, stSystem.cTransAttr);
	EmvDispTvrTsi();

	stSystem.cTransType = (char)TRANS_SALE;
	if (EMV_INITAPP_FAIL == nEmvRes || EMV_INITAPP_QUIT == nEmvRes) //退出即可
	{
		return APP_QUIT;
	}
	else if (-1321 == nEmvRes) //select ppse 失败返回-1321,走Upcard,其他情况走qPboc
	{
		ShowLightOff();
		return EMV_INITAPP_UPCARD; 
	}
	else if (nEmvRes == EMV_TRANS_QPBOC_ACCEPT)
	{

	}
	else if (nEmvRes == EMV_TRANS_QPBOC_GOONLINE)
	{
		ShowLightOnline();
	}
	else if (nEmvRes == EMV_TRANS_MSD_GOONLINE) /**联机*/
	{
		stSystem.cTransAttr = ATTR_MSD;//add by 20170511
		GetExpDateFromTk2(stSystem.szTrack2, stSystem.szExpDate);
		memcpy(stSystem.szInputMode, "91", 2);
		ShowLightOnline();
	}
	else
	{
		ShowLightFail();

		if (nEmvRes == EMV_TRANS_QPBOC_DENIAL)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szMsg, "|CDECLINE\n|C%s", EmvErrMsg(EMV_ErrorCode()));
		}
		else if (nEmvRes == -15) //for cdet test 20170503
		{
			EMV_rf_suspend(nEmvRes);
			PubMsgDlg(pszTitle, "Pls Refer to your payment device", 3, 10);
			return APP_QUIT;
		}
		else
		{
			memset(szMsgBuf, 0, sizeof(szMsgBuf));
			if (-2105 == EMV_ErrorCode()) /**超过限额,提示尝试其他交易方式,lld,2014-2-17,szbctc*/
			{
				strcpy(szMsgBuf, "Pls Try Other Way");
			}
			else
			{
				strcpy(szMsgBuf, EmvErrMsg(EMV_ErrorCode()));
			}

			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szMsg, "|CTERMINATE\n|C%s", szMsgBuf);
		}
		PubMsgDlg(pszTitle, szMsg, 3, 10);
		return APP_FAIL;
	}

	memset(sEcBalance, 0, sizeof(sEcBalance));
	nRet = EmvEntryPointGetData(0x9F5D, sEcBalance, sizeof(sEcBalance));
	if (nRet < 0)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "\n|CGet Q banlance fail");
		PubMsgDlg(pszTitle, szContent, 3, 10);
		nRet = 0;
	}
	nEcBalanceLen = nRet;
	PubHexToAsc(sEcBalance, nEcBalanceLen * 2, 0, (uchar *)stSystem.szTipAmount);	/**<借用小费金额保存余额*/

	/**
	* 采集数据
	*/
	DealSystem(&stSystem);
	memcpy(stSystem.szMsgID, "0200", 4);
	memcpy(stSystem.szProcCode, "000000", 6);
	memcpy(stSystem.szServerCode, "00", 2);

	
	/**<脱机*/
	if (nEmvRes == EMV_TRANS_QPBOC_ACCEPT)
	{
		EMV_setdata(_EMV_TAG_8A_TM_ARC, (uchar *)"\x59\x31", 2);/**<8A（tag）	授权响应码	an2		ASCII	M		取值如下：Y1：脱机接受
															Y3：不能联机（脱机接受）
															*/
		stSystem.cWavePinAndSigFlag = 1;
		goto WATERANDPRINT;
	}



	stSystemBak = stSystem;
regetpin:
	stSystem = stSystemBak;
	PubDisplayTitle(pszTitle);

	
	/**
	* 输入密码，这个 部分是输入联机密码
	*/
	memset(stSystem.szPin, 0, sizeof(stSystem.szPin));
	if(0 == nInputPinNum)
	{
		/**
		* 获取联机PIN，密文
		*/
		EmvGetOnlinePin(stSystem.szPin);
		ASSERT_HANGUP_QUIT(CheckInputPinPayPass(&stSystem));
	}
	else
	{
		ASSERT_HANGUP_QUIT(GetPin(stSystem.szPan, stSystem.szAmount, stSystem.szPin));
	}
	

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(&stSystem);
	stSystem.cHasPinInput = GetIsKeyInPIN();
	//2017112901
	if (stSystem.nAcqIndex == EM_ACQINDEX_MCCS && stSystem.cHasPinInput == 0)
	{
		stSystem.szInputMode[2] = '0';
	}
	
		
	PubDisplayTitle(pszTitle);

	//check DCC
	if (YES == CheckIsGoDcc(stSystem.szPan, stSystem.cTransAttr))
	{
		MagDCCRequst(stSystem);
	}
	/**
	* 执行冲正
	*/
	nRet = DoAdviseAndReveral();
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		ShowLightOff();
		return APP_QUIT;
	}

	PubDisplayTitle(pszTitle);
	/**
	* 拨号
	*/
#ifdef DEMO
#else
	nRet = CommConnect();
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		ShowLightFail();
		NDK_SysMsDelay(100);
		DispOutICC(pszTitle,"CANCEL", "");
		return nRet;
	}
#endif
	cOnlineResult = TRUE;

	GetVarTraceNo(stSystem.szTrace);/**<联机时需要重新确定流水号，之前可能存在脚本上送等联机交易*/

	EmvPackField55(EMV_SALE, stSystem.sField55, &stSystem.nAddFieldLen, stSystem.nAcqIndex);
	SaveData9F26RQ();

TRACE(("-------------invoicc no: %s", stSystem.szInvoice));
	/**
	* 开始打包
	*/
	nRet = PackGeneral(&stSystem, NULL);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		ShowLightFail();
		DispOutICC(pszTitle, "PACK FAIL", "");
		return APP_FAIL;
	}
	/**
	* 保存冲正数据
	*/
	SystemToReveral(&stSystem, &stReversal);
	EmvPackField55(EMV_REVERSAL, stReversal.szFieldAdd1, &stReversal.nFieldAdd1Len, stReversal.nAcqIndex);
	nInputPinNum++;
#ifdef DEMO
	IncVarTraceNo();
	sprintf(stSystem.szRefnum, "%s%s", stSystem.szTime, stSystem.szTrace);
	sprintf(stSystem.szAuthCode, "%.3s000", stSystem.szTrace+3);
#endif
	nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL|DPC_NOCHECKRESP,&stSystem,&stReversal,nInputPinNum);
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "\nPIN ERROR(50), TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if (nRet != APP_SUCC)
	{
		ShowLightFail();
		NDK_SysMsDelay(300);/**DispOutICC里面,会把灯全灭了，此处加延时为了看到红灯*/
		DispOutICC(pszTitle, "FAIL", "");
		CommHangUp();
		return APP_FAIL;
	}

WATERANDPRINT:

	PubClear2To4();

//	EmvDispTvrTsi();
	if (TRUE == cOnlineResult)
	{
#ifdef DEMO
		strcpy(stSystem.szResponse, "00");
#endif
		if (memcmp(stSystem.szResponse, "00", 2) != 0)
		{
			ShowLightFail();
			if (memcmp(stSystem.szResponse, "55", 2) == 0 && (nInputPinNum >= 3))
			{
				PubConfirmDlg(pszTitle, "PIN ERROR 3 TIMES", 0, 30);
			}
			else
			{
				if (memcmp(stSystem.szResponse, "LE", 2) == 0)
				{
					DispRespEFTInfo();
				}
				else
				{
					DispResp(stSystem.szResponse, stSystem.nAcqIndex);
				}
			}
			SetVarIsReversal(NO);
			/*联机返回失败也要上送*/
			CommHangUp();
			return APP_FAIL;
		}
		ShowLightSucc();
		stSystem.cEMV_Status = EMV_STATUS_ONLINE_SUCC;
		EmvEntryPointSetData(_EMV_TAG_8A_TM_ARC, (uchar *)stSystem.szResponse, 2);
		EmvSaveWater(TRUE, &stSystem);
		SysToRecord(&stSystem, &stTransRecord);
		EmvAddtionWater(&stTransRecord);

		nRet = AppendTransRecord(&stTransRecord);
		if (nRet != APP_SUCC)
		{
			strcpy(stReversal.szResponse, "96");
			SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
			CommHangUp();
			return APP_FAIL;
		}

		SetVarIsReversal(NO);
		PubSetPosDateTime(pstSystem->szDate, "MMDD", pstSystem->szTime);
	}
	else  //脱机
	{
		PubBeep(1);

		/**
		* 这里是为了得到AID去查表得到国际组织代码
		*/
		EmvPackField55(EMV_SALE, stSystem.sField55, &stSystem.nAddFieldLen, stSystem.nAcqIndex);

		stSystem.cEMV_Status = EMV_STATUS_OFFLINE_SUCC;
		strcpy(stSystem.szAuthCode,"VLP111");
		strcpy(stSystem.szResponse, "Y1");
		

		/*	增加流水号 */
		IncVarTraceNo();
		/*保存脱机交易系统参考号*/
		EmvSaveWater(TRUE, &stSystem);
		memset(&stTransRecord,0,sizeof(STTAANSRECORD));
		SysToRecord(&stSystem, &stTransRecord);
		EmvAddtionWater(&stTransRecord);

		ASSERT_HANGUP_FAIL(AppendTransRecord(&stTransRecord));
		IncVarOfflineUnSendNum();		/*需上送流水数加1*/
	}

TRACE(("=============================="));
	ChangeSettle(&stSystem);
	IncVarInvoiceNo();

	PubClearAll();
TRACE(("=============================="));
	PubDisplayTitle(pszTitle);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;

TRACE(("-------------invoicc no: %s", stTransRecord.szInvoice));
	PrintWater(&stTransRecord, FIRSTPRINT);
TRACE(("=============================="));
	if (YES == GetVarIsPinpad())
	{
		PubClrPinPad();
	}
	DispOutICC(pszTitle, NULL, "");

	/**
	* 离线上送
	*/
	if (TRUE == cOnlineResult)
	{
		SendOffline(0);
		CommHangUp();
	}	
	
	return APP_SUCC;
}


/**
* @brief Emv Preauth
* @param in pstSystem
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvCashAdvance(STSYSTEM *pstSystem)
{
	STSYSTEM stSystemBak;
	STTAANSRECORD stTransRecord;
	STREVERSAL stReversal;
	char szRecvFiled55[260+1];
	int nFieldLen, nInputPinNum = 0;
	int nRet = 0;
	char *pszTitle = "CASH ADVANCE";
	char cTSICompleted = FALSE;
	//int nSel = 0;
//	int nLen = 0;
	
	memset(&stSystemBak, 0, sizeof(STSYSTEM));
	memset(&stTransRecord, 0, sizeof(STTAANSRECORD));
	memset(&stReversal, 0, sizeof(STREVERSAL));

	pstSystem->cTransType = (char)TRANS_CASH_ADVANCE;
	pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD;

	if(memcmp(pstSystem->szInputMode,"07",2) ==0)
	{
		pstSystem->cTransAttr = (char)ATTR_EMV_STANDARD_RF;
	}
	
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
TRACE(("++++++++"));
	/**
	* Select payment term
	*/
	PubDisplayTitle(pszTitle);
	
	EmvSetTxnPan(pstSystem->szPan);

	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	memcpy(pstSystem->szMsgID, "0200", 4);
	memcpy(pstSystem->szProcCode, "010000", 6);
	memcpy(pstSystem->szServerCode, "00", 2);
TRACE(("++++++++"));


	/**
	* Process Flow
	*/
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	if(pstSystem->cTransAttr == ATTR_EMV_STANDARD)
	{
		nRet = EmvProcessApp(pstSystem->szAmount, pstSystem->cTransAttr);
		if (APP_SUCC != nRet)
		{
			CommHangUp();
			return nRet;
		}
		TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));

		/**
		* Get Online Pin
		*/
		EmvGetOnlinePin(pstSystem->szPin);

		ASSERT_HANGUP_QUIT(CheckInputPin(pstSystem));
	}
	else
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}
	stSystemBak = *pstSystem;
regetpin:
	*pstSystem = stSystemBak;

	if(nInputPinNum > 0)
	{
		ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
	}

	/**
	* SET input mode for PIN
	*/
	SetIsPinInput(pstSystem);
	pstSystem->cHasPinInput = GetIsKeyInPIN();

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());

	PubDisplayTitle(pszTitle);

	/**
	* Dialing
	*/
#ifdef DEMO
#else
	ASSERT_HANGUP_QUIT(CommConnect());
#endif
	/*
	* Dial successfully
	*/
	EmvPackField55(EMV_SALE, pstSystem->sField55, &pstSystem->nAddFieldLen, pstSystem->nAcqIndex);
	SaveData9F26RQ();
	/**
	* Get necessary data
	*/
	DealSystem(pstSystem);
	TRACE(("stSystem.szAmount=%s",pstSystem->szAmount));
	/**
	* Begin to pack
	*/
	nRet = PackGeneral(pstSystem, NULL);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		DispOutICC(pszTitle, "PACKED FAIL", "");
		return APP_FAIL;
	}
	/**
	* Save reversal data
	*/
	SystemToReveral(pstSystem, &stReversal);
	EmvPackField55(EMV_REVERSAL, stReversal.szFieldAdd1, &stReversal.nFieldAdd1Len, stReversal.nAcqIndex);
	nInputPinNum++;
	
#ifdef DEMO
	IncVarTraceNo();
	sprintf(pstSystem->szRefnum, "%s%s", pstSystem->szTime, pstSystem->szTrace);
	sprintf(pstSystem->szAuthCode, "%.3s000", pstSystem->szTrace+3);
#endif
	if(ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL|DPC_NOCHECKRESP,pstSystem,&stReversal,nInputPinNum);
	}
	else
	{
		nRet = DealPackAndComm(pszTitle, DPC_SETREVERSAL,pstSystem,&stReversal,nInputPinNum);		
	}
	if(APP_REPIN == nRet)
	{
		PubMsgDlg(pszTitle, "PASSWORD ERROR.\nPLS TRY AGAIN!", 3, 30);
		goto regetpin;
	}
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		EMV_Suspend(-1);
		return APP_FAIL;
	}

	if (ATTR_EMV_STANDARD == pstSystem->cTransAttr)
	{	
		/**
		* Get Field 55
		*/
		memset(szRecvFiled55, 0, sizeof(szRecvFiled55));
		nFieldLen = 255;
		GetField(55, szRecvFiled55+2, &nFieldLen);
		PubIntToC2((uchar *)szRecvFiled55, (uint)nFieldLen);


#ifdef DEMO
#else	
		ASSERT_HANGUP_FAIL(EmvCompleteApp(pszTitle,TRUE,pstSystem,&stReversal,szRecvFiled55,nFieldLen, &cTSICompleted, nInputPinNum));
#endif
	}
		
	pstSystem->cEMV_Status = EMV_STATUS_ONLINE_SUCC;

	EmvSaveWater(TRUE, pstSystem);
	SysToRecord(pstSystem, &stTransRecord);
	EmvAddtionWater(&stTransRecord);

	return TradeComplete(pszTitle, TDC_DEFAULT, pstSystem, &stTransRecord, NULL);

}


/**
* @brief 支持PBOC标准借贷记插卡和挥卡的流程
* @param in pnInputMode 获取卡号模式
* @return @li APP_SUCC
*		@li APP_FAIL
*		@li APP_QUIT
*/
int EmvTxnCashAdvance()
{
	char *pszTitle = "CASH ADVANCE";
	int nTrueInput;
	STSYSTEM stSystem;

	memset(&stSystem,0,sizeof(STSYSTEM));
	stSystem.cTransType = TRANS_CASH_ADVANCE;
	nTrueInput = INPUT_NO;

	ASSERT_QUIT(TradeInit(pszTitle, TDI_DEFAULT, stSystem.cTransType, 0, YES));

	/**
	* Input Amount
	*/
	ASSERT_FAIL(EdcGetAmout(pszTitle, &stSystem));
	
	ASSERT_QUIT(Emv_Common_InputCard(pszTitle, &stSystem, &nTrueInput));

	if(INPUT_INSERTIC == nTrueInput)
	{
		TRACE(("stSystem.szAmount=%s",stSystem.szAmount));
		ASSERT(EmvCashAdvance(&stSystem));
	}
	else if(INPUT_STRIPE == nTrueInput || INPUT_KEYIN == nTrueInput)
	{
		ASSERT(MagCashAdvance(&stSystem));
	}
	DISP_OUT_ICC;
	DoAdviseAndReveral();
	return APP_SUCC;
	
}

