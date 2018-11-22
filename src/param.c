/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  param.c
** File indentifier: 
** Brief:  Parameter manage module
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

#define APPCHECKVALUE		APP_NAME"007"	/**<If check value changes, need to initialize*/
#define FILE_APPPOSPARAM	APP_NAME"PARA"
#define APPTYPE				"60"			/**<app type*/
#define APPVER				"31"			/**<app ver*/
#define DEFAULT_ADMINPWD 		"88888888"	/**<Admin pwd*/
#define DEFAULT_USRPWD 		"12345678"	/**<Usr pwd*/
#define DEFAULT_FUNCTION 		"1234"	/**<Function pwd*/
#define DEFAULT_SHORTCUT		"0000"	/**<Function pwd*/
#define FILE_PARAM_INI 		APP_NAME"PARA.ini"
#define FILE_EMVCODE_INI	"/appfs/apps/share/EMVERRCODE.INI"
#define FILE_PARAM_BACK 	APP_NAME"PARABACK"
#define PARAM_BACK_BUFF_LEN	4096


/**
* @struct STSETTLEPARAM 结算参数
*/
typedef struct
{
	int	nWaterSum;						/**<Water num*/
	int nHaveReSendNum;					/**<Count has sent*/
	char cPrintSettleHalt;				/**<Print settle flag*/
	char cPrintDetialHalt;				/**<Print detail flag*/
	char cPrintWaterHaltFlag;			/**<Print water flag*/
	char cIsTotalMatch;					/**<Is total summary */
	char cBatchHaltFlag;				/**<Send batch flag*/
	char cClrSettleDataFlag;			/**<Clear settle flag*/
	int	nFinanceHaltFlag;				/**<Send finance flag*/
	int	nMessageHaltFlag;				/**<Send advice flag*/
	int	nBatchMagOfflinHaltFlag;		/**<Send magnetic offline flag*/
	int	nIcLoadHaltFlag;				/**<Send load flag*/
	int	nBatchSum;						/**<Batch num*/
	char sSettleDateTime[5];			/**<Settle time*/
} STSETTLEPARAM;


/**
* @struct STBANKPARAM 
*/
typedef struct
{
	char cPosMerType;					/**<POS type*/
	char cIsLogin;						/**<Is login*/
	char cIsLock;						/**<Is lock*/
	char szTraceNo[fTraceNo_len+1];		/**<Stan*/
	char szInvoiceNo[fTraceNo_len+1];	/**<Invoice No*/
	char sLastLoginDateTime[5];			/**<Last login time*/
} STBANKPARAM;

/**
* @struct STEMVPARAM EMV交易信息
*/
typedef struct
{
	int	nTransSerial;				/**<EMV Trans serial*/
	int	nOfflineUpNum;				/**<EMV send offline num*/
	int	nOnlineUpNum;				/**<EMV send online num*/
	int	nOfflineFailUpNum;			/**<EMV offline fail num*/
	int	nOnlineARPCErrUpNum;		/**<EMV arpc fail num*/
	char cIsDownCAPKFlag;			/**<Is download CAPK*/
	char cIsDownAIDFlag;			/**<Is download AID*/
	char sScriptData[256];			/**<Script data*/
} STEMVPARAM;


/**
* define global variable
*/
static STAPPPOSPARAM gstAppPosParam;	/**<App parameter*/
static STSETTLEPARAM gstSettleParam;	/**<Settle parameter*/
static STREVERSALPARAM gstReversalParam;/**<Reversal parameter*/
static STBANKPARAM gstBankParam;		/**<Bank parameter*/
static STEMVPARAM gstEmvParam;			/**<EMV parameter*/

//0 V/M/J; 1 AMEX; 2 MCCS; 3 UPI; 
// 4 V/M/J Epp 6 ; 
// 5 V/M/J Epp 12;
// 6 V/M/J Epp 18;
// 7 V/M/J Epp 24;
// 8 AMEX Epp 6 ; 
// 9 AMEX Epp 12;
// 10 AMEX Epp 18;
// 11 AMEX Epp 24;
static const char glszAcquirerName[][48]={       
	{"VISA/MASTER/JCB"},
	{"AMEX"},
	{"MCCS"},
	{"UPI"},
	{"EPP 6 MONTH V/M"},
	{"EPP 12 MONTH V/M"},
	{"EPP 18 MONTH V/M"},
	{"EPP 24 MONTH V/M"},
	{"EPP 6 MONTH AMEX"},
	{"EPP 12 MONTH AMEX"},
	{"EPP 18 MONTH AMEX"},
	{"EPP 24 MONTH AMEX"},
};


//0 V/M/J; 1 AMEX; 2 MCCS; 3 UPI; 4 V/M/J Epp; 5 AMEX EPP
static const char glszBatchIndexName[][48]={       
	{"VISA/MASTER/JCB"},
	{"AMEX"},
	{"MCCS"},
	{"UPI"},
	{"EPP V/M 3M"},
	{"EPP V/M 6M"},
	{"EPP V/M 12M"},
	{"EPP V/M 18M"},
	{"EPP V/M 24M"},
	{"EPP AMEX 3M"},
	{"EPP AMEX 6M"},
	{"EPP AMEX 12M"},
	{"EPP AMEX 18M"},
	{"EPP AMEX 24M"},
};


/**
* Interface function implementation
*/

/**
* @brief Check is first run
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IsFirstRun()
{
	int	i;
	char cCh=0;
	char sBuf[fPosCheck_len+1]= {0};
	char szTmp[fPosCheck_len+1]= {0};

	memset(szTmp,0,sizeof(szTmp));
	strcpy(szTmp,APPCHECKVALUE);

	memset(sBuf,0,sizeof(sBuf));
	PubGetVar(sBuf,fPosCheck_off,fPosCheck_len);

	for(i=0; i<7; i++)
	{
		cCh^=sBuf[i];
	}

	if((memcmp(szTmp,sBuf,7)==0) &&(cCh==sBuf[7]))
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
}

/**
* @brief Set App check value
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetAppCheckValue()
{
	int	i;
	char cCh=0,sBuf[fPosCheck_len+1]= {0};

	memset(sBuf,0,sizeof(sBuf));
	strcpy(sBuf,APPCHECKVALUE);

	for(i=0; i<7; i++)
	{
		cCh^=sBuf[i];
	}
	sBuf[i]=cCh;
	PubSaveVar(sBuf,fPosCheck_off,fPosCheck_len);
	return APP_SUCC;
}

/**
* @brief Initialize default app parameter
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitPosDefaultParam()
{
	STRECFILE stPosParamfile;
	char szBuf[fNVR_Addr_End-fNVR_Addr_Start+1];
	char cStatus = NO;

	/**<Cleat fram*/
	memset(szBuf,0,sizeof(szBuf));
	PubSaveVar(szBuf,fNVR_Addr_Start,fNVR_Addr_End-fNVR_Addr_Start);

	/**<APP parameter*/
	memset(&stPosParamfile, 0, sizeof(STRECFILE));
	strcpy( stPosParamfile.szFileName, FILE_APPPOSPARAM);
	stPosParamfile.cIsIndex = FILE_NOCREATEINDEX;						
	stPosParamfile.unMaxOneRecLen = sizeof(STAPPPOSPARAM);
	stPosParamfile.unIndex1Start =0;
	stPosParamfile.unIndex1Len =0;
	stPosParamfile.unIndex2Start =0;
	stPosParamfile.unIndex2Len =0;
	ASSERT(PubCreatRecFile(&stPosParamfile));

	memset(&gstAppPosParam,0,sizeof(gstAppPosParam));
	PubAddRec(FILE_APPPOSPARAM,(char *)&gstAppPosParam);
	strcpy(gstAppPosParam.szAppDispname,APP_NAMEDESC);
	strcpy(gstAppPosParam.szMerchantNameEn,"EDC TEST NAME");

	strcpy(gstAppPosParam.szTerminalId[0],"12345678");
	strcpy(gstAppPosParam.szMerchantId[0],"123456789012345");

	strcpy(gstAppPosParam.szMerchantAddr[0],"ADDR1");
	strcpy(gstAppPosParam.szMerchantAddr[1],"ADDR2");
	strcpy(gstAppPosParam.szMerchantAddr[2],"ADDR3");
	strcpy(gstAppPosParam.szMerchantAddr[3],"");
	strcpy(gstAppPosParam.szAcqName,"VISA");

	strcpy(gstAppPosParam.szIssuerName,"VISA");
	strcpy(gstAppPosParam.szCurrencyName,"RM");
	
	EmvSetTerminalId((uchar *)gstAppPosParam.szTerminalId[0]);
	EmvSetMerchantId((uchar *)gstAppPosParam.szMerchantId[0]);
	gstAppPosParam.cIsPinPad=NO;
	gstAppPosParam.cPinPadAuxNo=PORT_NUM_COM2;
	strcpy(gstAppPosParam.szPinPadTimeOut,"60");	
	strcpy(gstAppPosParam.szAdminPwd,DEFAULT_ADMINPWD);
	strcpy(gstAppPosParam.szUsrPwd, DEFAULT_USRPWD);
	strcpy(gstAppPosParam.szFuncPwd, DEFAULT_FUNCTION);
	strcpy(gstAppPosParam.szShortCutPwd, DEFAULT_SHORTCUT);
	strcpy(gstAppPosParam.szMainKeyNo,"0");
	gstAppPosParam.cEncyptMode=DESMODE_3DES;
	gstAppPosParam.cPinEncyptMode=PINTYPE_WITHPAN;
	gstAppPosParam.cIsPrintWaterRec=NO;
	gstAppPosParam.cIsPreauthShieldPan = YES;
	gstAppPosParam.cIsPrintErrReport=NO;
	gstAppPosParam.cPrintPageCount='2';
	strcpy(gstAppPosParam.szMaxTransCount,"300");
	strcpy(gstAppPosParam.szMaxRefundAmount,"000000100000");	
	memcpy(gstAppPosParam.sTransSwitch,"\xFF\xE0\x00\x00",4);

	gstAppPosParam.cKeySystemType = '0';
	gstAppPosParam.cLanguage = LANG_EN_US;
	gstAppPosParam.cFontSize = 24;
	gstAppPosParam.cIsIcFlag=YES;
	gstAppPosParam.cIsCardInput=NO;
	gstAppPosParam.cIsSaleVoidStrip=NO;
	gstAppPosParam.cIsVoidPin=NO;
	gstAppPosParam.cDefaultTransType='1';
	gstAppPosParam.cIsTipFlag = NO;
	memcpy(gstAppPosParam.szTipRate, "15", 2);
	gstAppPosParam.cIsDispEMV_TVRTSI = NO;
	memcpy(gstAppPosParam.sUID, "1234567812345678", 16);
	gstAppPosParam.cIsShieldPan = YES;
	gstAppPosParam.cIsSupportRF = YES;
	gstAppPosParam.cIsExRF = NO;
	gstAppPosParam.cIsUsePP60RF= YES;
	if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_RADIO_FREQUENCY, NULL))
	{
		gstAppPosParam.cIsExRF = NO;
	}
	gstAppPosParam.cPntTitleMode = '0';
	strcpy(gstAppPosParam.szPntTitleEn,"POS RECEIPT");
	gstAppPosParam.cIsVoidPwd = YES;
	gstAppPosParam.cIsPrintMinus=YES;
	strcpy(gstAppPosParam.szMaxOffSendNum, "10");
	
	//strcpy(gstAppPosParam.szPrnLogoName,"boclogo.bmp");
	gstAppPosParam.unPrnLogoXpos = 60;
	gstAppPosParam.cSendAutoDl = NO;
	gstAppPosParam.cDownloadMode = 0;
	gstAppPosParam.cWeekDl = GetRandValue(7);
	gstAppPosParam.cHourDl = GetRandValue(12);
	gstAppPosParam.cMinuteDl = GetRandValue(24);

	strcpy(gstAppPosParam.szHost, "MBB");
	gstAppPosParam.cSupportSwipe = 1;
	gstAppPosParam.cIsEncrypt = YES;
	gstAppPosParam.nDukptIndex = 1;
	gstAppPosParam.cMccsPriority = YES;
	PubSetReadRfDelay(0);
	memcpy(gstAppPosParam.szVendorId, "00000010", 8);
	memcpy(gstAppPosParam.szAcquirerId, "001", 8);
	memcpy(gstAppPosParam.szMinCashbackAmt, "000000000500", 12);
	memcpy(gstAppPosParam.szWaveNoPinAmt, "000000025000", 12);
	gstAppPosParam.cIsPinByPass = YES;
	
	memcpy(gstAppPosParam.szMinEPPAmt[0], "000000050000", 12);
	memcpy(gstAppPosParam.szMaxEPPAmt[0], "000010000000", 12);
	memcpy(gstAppPosParam.szMinEPPAmt[1], "000000100000", 12);
	memcpy(gstAppPosParam.szMaxEPPAmt[1], "000010000000", 12);
	memcpy(gstAppPosParam.szMinEPPAmt[2], "000000100000", 12);
	memcpy(gstAppPosParam.szMaxEPPAmt[2], "000010000000", 12);
	memcpy(gstAppPosParam.szMinEPPAmt[3], "000000200000", 12);
	memcpy(gstAppPosParam.szMaxEPPAmt[3], "000010000000", 12);
	gstAppPosParam.cIsDccEnable = NO;
	gstAppPosParam.cIsInputCvv2 = NO;
	gstAppPosParam.cIsPrintIso = NO;
	gstAppPosParam.cCashRegister = NO;
	strcpy(gstAppPosParam.szInquiryNo, "03-2074 8787");
	strcpy(gstAppPosParam.szCenterNo, "03-2056 8888");
	ASSERT(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	/**<Settle data*/
	memset(&gstSettleParam,0,sizeof(gstSettleParam));
	ClearAllSettle();

	/**<Reversal data*/
	memset(&gstReversalParam,0,sizeof(gstReversalParam));
	SetVarIsReversal(NO);
	SetVarHaveReversalNum(0);
	SetVarReversalData(gstReversalParam.sReversalData,sizeof(gstReversalParam.sReversalData));

	/**<Bank data*/
	memset(&gstBankParam,0,sizeof(gstBankParam));
	SetVarIsLogin(NO);
	SetVarIsLock(NO);
	SetVarTraceNo("000001");
	InitVarBatchNo();
	SetVarInvoiceNo("000001");
	SetVarLastLoginDateTime("\x00\x00\x00\x00\x00");
	SetVarOfflineUnSendNum(0);
	/**<EMV Data*/
	memset(&gstEmvParam,0,sizeof(gstEmvParam));
	SetVarEmvTransSerial(1);
	SetVarEmvOfflineUpNum(1);
	SetVarEmvOnlineUpNum(1);
	SetVarEmvOfflineUpNum(1);
	SetVarEmvOnlineUpNum(1);
	SetVarEMVOfflineFailHaltFlag(1);
	SetVarEMVOnlineARPCErrHaltFlag(1);
	SetVarEmvIsDownCAPKFlag(YES);
	SetVarEmvIsDownAIDFlag(YES);
	
	cStatus = NO;
	PubSaveVar(&cStatus, fIsPosParamDown_off, fIsPosParamDown_len);
	cStatus = NO;
#if defined(USE_TMS)
	PubSaveVar(&cStatus, fIsTmsDown_off, fIsTmsDown_len);
#endif

	/**<Set check value*/
	SetAppCheckValue();
	return APP_SUCC;
}

//20160708 tms support ini, if fail do nothing
#define ASSERT_PARA_FAIL(e, msg) \
	if ((e) != APP_SUCC)\
	{\
		/*PubMsgDlg("SET PARAM", msg, 3, 3);\
		PubCloseFile(&nIniHandle);\
		return APP_FAIL;\*/;\
	}
	
/**
* @brief Get parameter fron ini file
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetParamFromIni()
{
	const char *szIniFile = FILE_PARAM_INI;
	int nIniHandle;
	int nValueLen, nValue,nCommValue;
	char szBuf[100];
	int i=0;
	char *pszTitle = "SET PARAM FROM INI";
	char lszName[TRANSSWITCHNUM][20]= {"","","PREAUTH","REFUND","INSTALLMENT","CASHADVANCE", "OFFLINE","CASHBACK",
		"BALANCE","","","",""};//"VOID REFUND","VOID OFFLINE" ,"VOID INSTALLMENT", "VOID CASH ADVANCE"
	char lszEppName[EPPMONTHSNUM][20]= {"SIX","TWELVE","EITHTEEN","TWENTYFOUR"};
	char szTmp[10] = "";

	STAPPCOMMPARAM stAppCommParam;
	memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));

	GetAppCommParam(&stAppCommParam);
	ASSERT_QUIT(PubOpenFile (szIniFile, "r", &nIniHandle));

	PubClearAll();
	PubDisplayStrInline(0, 3, "Processing...");
	PubUpdateWindow();

	for (i = 0; i < ACQUIRER_NUM; i++)
	{
		sprintf(szTmp, "MID%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(gstAppPosParam.szMerchantId[i]) - 1, gstAppPosParam.szMerchantId[i], &nValueLen), "Get mid failed");
		sprintf(szTmp, "TID%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(gstAppPosParam.szTerminalId[i]) - 1, gstAppPosParam.szTerminalId[i], &nValueLen), "Get tid failed");
		sprintf(szTmp, "NII%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(stAppCommParam.szTransNii[i]) - 1, stAppCommParam.szTransNii[i], &nValueLen), "Get nii failed");
		sprintf(szTmp, "TPDU%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(stAppCommParam.szTransTpdu[i]) - 1, stAppCommParam.szTransTpdu[i], &nValueLen), "Get tpdu failed");
	}

	for (i=0; i<EPPMONTHSNUM; i++)
	{
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", lszEppName[i], &nValue), "Get epp switch failed");
PubDebug("====== value: %d", nValue);	
		if (nValue)
		{
			gstAppPosParam.sEppMonthSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sEppMonthSwitch[i/8] &= ~(0x01<<(7-i%8));
		}	
	}
	
	for (i = 0; i < EPPMONTHSNUM; i++)
	{
		sprintf(szTmp, "MINAMT%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(gstAppPosParam.szMinEPPAmt[i]) - 1, gstAppPosParam.szMinEPPAmt[i], &nValueLen), "Get amt failed");
		sprintf(szTmp, "MAXAMT%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(gstAppPosParam.szMaxEPPAmt[i]) - 1, gstAppPosParam.szMaxEPPAmt[i], &nValueLen), "Get amt failed");
	}

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "MERCHANTNAME",  sizeof(gstAppPosParam.szMerchantNameEn) - 1, gstAppPosParam.szMerchantNameEn, &nValueLen), "Get merchant name failed");
	for (i= 0; i < 4; i++)
	{
		sprintf(szTmp, "ADDR%d", i+1);
		ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", szTmp,  sizeof(gstAppPosParam.szMerchantAddr[i]) - 1, gstAppPosParam.szMerchantAddr[i], &nValueLen), "Get merchant addr failed");
	}
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "TRACENO",  sizeof(gstBankParam.szTraceNo) - 1, gstBankParam.szTraceNo, &nValueLen), "Get trace failed");
	
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "PRTCOUNT",  &nValue), "Get page num failed");
	gstAppPosParam.cPrintPageCount = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "DEBITOPT",  &nValue), "Get DEBITOPT failed");
	gstAppPosParam.cMccsPriority = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "TIP",  &nValue), "Get TIP failed");
	gstAppPosParam.cIsTipFlag = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "MANUAL",  &nValue), "Get MANUAL failed");
	gstAppPosParam.cIsCardInput = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "CASHREGISTER",  &nValue), "Get CASHREGISTER failed");
	gstAppPosParam.cCashRegister = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "VPWD",  &nValue), "Get VPWD failed");
	gstAppPosParam.cIsVoidPwd = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "SPWD",  &nValue), "Get SPWD failed");
	gstAppPosParam.cIsSettlePwd = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "PINBYPASS",  &nValue), "Get PINBYPASS failed");
	gstAppPosParam.cIsPinByPass = nValue + '0';
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "BASE", "CVV2",  &nValue), "Get PINBYPASS failed");
	gstAppPosParam.cIsInputCvv2 = nValue + '0';

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "VENDERID",  sizeof(gstAppPosParam.szVendorId) - 1, gstAppPosParam.szVendorId, &nValueLen), "Get VENDERID failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "VSPWD",  sizeof(gstAppPosParam.szUsrPwd) - 1, gstAppPosParam.szUsrPwd, &nValueLen), "Get VSPSD failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "SYSPWD",  sizeof(gstAppPosParam.szAdminPwd) - 1, gstAppPosParam.szAdminPwd, &nValueLen), "Get szAdminPwd failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "FUNCPWD",  sizeof(gstAppPosParam.szFuncPwd) - 1, gstAppPosParam.szFuncPwd, &nValueLen), "Get szFuncPwd failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "SHORTCUTPWD",  sizeof(gstAppPosParam.szShortCutPwd) - 1, gstAppPosParam.szShortCutPwd, &nValueLen), "Get szFuncPwd failed");

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "PPCVMLIMIT",  sizeof(gstAppPosParam.szPaypassCvmLimit) - 1, gstAppPosParam.szPaypassCvmLimit, &nValueLen), "Get VSPSD failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "PPFLOORLIMIT",  sizeof(gstAppPosParam.szPaypassFloorLimit) - 1, gstAppPosParam.szPaypassFloorLimit, &nValueLen), "Get VSPSD failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "PWCVMLIMIT",  sizeof(gstAppPosParam.szPaywaveCvmLimit) - 1, gstAppPosParam.szPaywaveCvmLimit, &nValueLen), "Get VSPSD failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "PWFLOORLIMIT",  sizeof(gstAppPosParam.szPaywaveFloorLimit) - 1, gstAppPosParam.szPaywaveFloorLimit, &nValueLen), "Get VSPSD failed");

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "INQUIRYNO",  sizeof(gstAppPosParam.szInquiryNo) - 1, gstAppPosParam.szInquiryNo, &nValueLen), "Get szFuncPwd failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "BASE", "CENTRENO",  sizeof(gstAppPosParam.szCenterNo) - 1, gstAppPosParam.szCenterNo, &nValueLen), "Get szFuncPwd failed");
	
	memset(szBuf,0,sizeof(szBuf));
	if(APP_SUCC == PrivateGetINIItemStr(nIniHandle, "BASE", "PRINTTITLECN", sizeof(gstAppPosParam.szPntTitleEn)-1, szBuf, &nValueLen))
	{
		memset(gstAppPosParam.szPntTitleEn,0,sizeof(gstAppPosParam.szPntTitleEn));
		memcpy(gstAppPosParam.szPntTitleEn,szBuf,nValueLen);
	}


	for (i=0; i<TRANSSWITCHNUM; i++)
	{

		if (lszName[i][0] == 0)
			continue;
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "TRANS", lszName[i], &nValue), "Get trans switch failed");
PubDebug("++++++  value: %d", nValue);	
		if (nValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "PIN", "PINPAD", &nValue), "Get pinpad flag failed");
		gstAppPosParam.cIsPinPad = (nValue ==1 ? YES : NO);
	}
	else
	{
		gstAppPosParam.cIsPinPad=NO;
	}
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "TIMEOUT",  &nValue), "Get timeout failed");
	stAppCommParam.cTimeOut = nValue;

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "NII",  sizeof(stAppCommParam.szSecurityNii)-1, stAppCommParam.szSecurityNii, &nValueLen), "Get Nii failed");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "TPDU",  sizeof(szBuf) - 1, szBuf, &nValueLen), "Get tpdu failed");
	PubAscToHex((uchar *)szBuf, 10, 0, (uchar *)stAppCommParam.sSecurityTpdu);

	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "COMMTYPE",  &nCommValue), "Get comm type failed");
	if (PubGetINIItemInt (nIniHandle, "COMM", "ISDHCP",  &nValue) == APP_SUCC)
	{
		stAppCommParam.cIsDHCP = nValue;
	}
	
	switch(nCommValue)
	{
	case 0:
		stAppCommParam.cCommType = COMM_DIAL;
		break;
	case 1:
		stAppCommParam.cCommType =COMM_ETH;
		break;
	case 2://CDMA
		stAppCommParam.cCommType =COMM_CDMA;
		break;
	case 3://GPRS
		stAppCommParam.cCommType =COMM_GPRS;
		break;
	case 4:
		stAppCommParam.cCommType =COMM_RS232;
		break;
	case 5:
		stAppCommParam.cCommType= COMM_WIFI;
		break;
	default:
		stAppCommParam.cCommType =COMM_DIAL;
		break;
	}
	
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "DIALNUM",  &nValue), "redial err");
	stAppCommParam.cReDialNum = nValue;

	nValueLen = 0;
	memset(stAppCommParam.szPreDial, 0, sizeof(stAppCommParam.szPreDial));
	PrivateGetINIItemStr (nIniHandle, "COMM", "PREPHONE",  sizeof(stAppCommParam.szPreDial) - 1, stAppCommParam.szPreDial, &nValueLen);

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "PHONE1",  sizeof(stAppCommParam.szTelNum1) - 1, stAppCommParam.szTelNum1, &nValueLen), "phone1 err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "PHONE2",  sizeof(stAppCommParam.szTelNum2) - 1, stAppCommParam.szTelNum2, &nValueLen), "phone2 err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "PHONE3",  sizeof(stAppCommParam.szTelNum3) - 1, stAppCommParam.szTelNum3, &nValueLen), "phone3 err");

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "SERVERIP1", sizeof(stAppCommParam.szIp1)-1, stAppCommParam.szIp1, &nValue), "ip1 err");
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "PORT1",  &nValue), "port1 err");
	sprintf(stAppCommParam.szPort1, "%d", nValue);

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "SERVERIP2", sizeof(stAppCommParam.szIp2)-1, stAppCommParam.szIp2, &nValue), "ip2 err");
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "PORT2",  &nValue), "port2 err");
	sprintf(stAppCommParam.szPort2, "%d", nValue);

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "LOCALIP", sizeof(stAppCommParam.szIpAddr)-1, stAppCommParam.szIpAddr, &nValue), "local ip err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "MASK", sizeof(stAppCommParam.szMask)-1, stAppCommParam.szMask, &nValue), "mask err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "GATEWAY", sizeof(stAppCommParam.szGate)-1, stAppCommParam.szGate, &nValue), "gate err");

	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "APN", sizeof(stAppCommParam.szAPN1)-1, stAppCommParam.szAPN1, &nValue), "apn err");
	strcpy(stAppCommParam.szAPN2, stAppCommParam.szAPN1);
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "USERNAME", sizeof(stAppCommParam.szUser)-1, stAppCommParam.szUser, &nValue), "user err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "PWD", sizeof(stAppCommParam.szPassWd)-1, stAppCommParam.szPassWd, &nValue), "pwd err");
	ASSERT_PARA_FAIL(PubGetINIItemInt (nIniHandle, "COMM", "MODE",  &nValue), "wifi mode err");
	switch(nValue)
	{
	case 1:
		stAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_OPEN;
		break;
	case 2:
		stAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_SHARED;
		break;
	case 3:
		stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
		break;
	case 4:
		stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA2;
		break;
	case 5:
		stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
		break;
	default:
		PubMsgDlg(pszTitle, "Wifi mode error", 1, 3);
	}
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "SSID", sizeof(stAppCommParam.szWifiSsid)-1, stAppCommParam.szWifiSsid, &nValue), "Ssid err");
	ASSERT_PARA_FAIL(PrivateGetINIItemStr (nIniHandle, "COMM", "PWD", sizeof(stAppCommParam.szWifiKey)-1, stAppCommParam.szWifiKey, &nValue), "Ssid pwd err");

	PubCloseFile(&nIniHandle);
		
/*
	sprintf(szBuf, "TID:%s\nMID:%s", gstAppPosParam.szTerminalId, gstAppPosParam.szMerchantId);
	PubMsgDlg(pszTitle, szBuf, 1, 3);

	sprintf(szBuf, "Merchant name:%s", gstAppPosParam.szMerchantNameEn);
	PubMsgDlg(pszTitle, szBuf, 1, 3);
*/
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	ASSERT_FAIL(SetAppCommParam(&stAppCommParam));
	SetVarTraceNo(gstBankParam.szTraceNo);

	ASSERT_FAIL(UpdateAmtLimit(VISA_ENB, gstAppPosParam.szPaywaveCvmLimit, gstAppPosParam.szPaywaveFloorLimit, NULL));
	ASSERT_FAIL(UpdateAmtLimit(MASTERCARD_ENB, gstAppPosParam.szPaypassCvmLimit, gstAppPosParam.szPaypassFloorLimit, NULL));

	PubDelFile(szIniFile);
	PubMsgDlg(pszTitle, "Update ok", 1, 1);
	
//	EmvSetMerchantId((uchar *)gstAppPosParam.szMerchantId);
//	EmvSetMerchantName((uchar *)gstAppPosParam.szMerchantNameEn);
//	EmvSetTerminalId((uchar *)gstAppPosParam.szTerminalId);
	return APP_SUCC;
}

/**
* @brief Export pos param 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ExportPosParam()
{
	char szFramBuf[fNVR_Addr_End-fNVR_Addr_Start+1];


	/**<Get Fram*/
	memset(szFramBuf,0,sizeof(szFramBuf));
	PubGetVar(szFramBuf,fNVR_Addr_Start,fNVR_Addr_End-fNVR_Addr_Start);
	
	/**<Get Pos param*/
	memset(&gstAppPosParam,0,sizeof(gstAppPosParam));
	ASSERT_FAIL(PubReadOneRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));


	/**<Settle data*/
	memset(&gstSettleParam,0,sizeof(gstSettleParam));
	memcpy((char *)&(gstSettleParam.nWaterSum), szFramBuf + fWaterSum_off, fWaterSum_len);
	memcpy((char *)&(gstSettleParam.nHaveReSendNum), szFramBuf + fHaveReSendNum_off, fHaveReSendNum_len);
	memcpy((char *)&(gstSettleParam.cPrintSettleHalt), szFramBuf + fSettlePrintHaltFlag_off, fSettlePrintHaltFlag_len);
	memcpy((char *)&(gstSettleParam.cPrintDetialHalt), szFramBuf + fPrintDetialHaltFlag_off, fPrintDetialHaltFlag_len);
	memcpy((char *)&(gstSettleParam.cPrintWaterHaltFlag), szFramBuf + fPrintWaterHaltFlag_off, fPrintWaterHaltFlag_len);
	memcpy((char *)&(gstSettleParam.cIsTotalMatch), szFramBuf + fCardFlag_off, fCardFlag_len);
	memcpy((char *)&(gstSettleParam.cBatchHaltFlag), szFramBuf + fBatchHaltFlag_off, fBatchHaltFlag_len);
	memcpy((char *)&(gstSettleParam.cClrSettleDataFlag), szFramBuf + fClrSettleDataFlag_off, fClrSettleDataFlag_len);
	memcpy((char *)&(gstSettleParam.nFinanceHaltFlag), szFramBuf + fFinanceHaltNum_off, fFinanceHaltNum_len);
	memcpy((char *)&(gstSettleParam.nMessageHaltFlag), szFramBuf + fMessageHaltNum_off, fMessageHaltNum_len);
	memcpy((char *)&(gstSettleParam.nBatchMagOfflinHaltFlag), szFramBuf + fBatchMagOfflineHaltNum_off, fBatchMagOfflineHaltNum_len);
	memcpy((char *)&(gstSettleParam.nIcLoadHaltFlag), szFramBuf + fIcLoadHaltNum_off, fIcLoadHaltNum_len);
	memcpy((char *)&(gstSettleParam.nBatchSum), szFramBuf + fBatchUpSum_off, fBatchUpSum_len);
	memcpy((char *)&(gstSettleParam.sSettleDateTime), szFramBuf + fSettleDateTime_off, fSettleDateTime_len);
	ExportSettleData();

	/**<Reversal data*/
	memset(&gstReversalParam,0,sizeof(gstReversalParam));
	memcpy((char *)&gstReversalParam.cIsReversal,szFramBuf + fIsReversal_off,fIsReversal_len);
	memcpy((char *)&gstReversalParam.nHaveReversalNum,szFramBuf + fHaveReversalNum_off,fHaveReversalNum_len);
	memcpy((char *)&gstReversalParam.nReversalLen,szFramBuf + fReversalLen_off,fReversalLen_len);
	memcpy((char *)&gstReversalParam.sReversalData,szFramBuf + fReversalData_off,fReversalData_len);

	/**<Bank data*/
	memset(&gstBankParam,0,sizeof(gstBankParam));
	memcpy((char *)&gstBankParam.cIsLogin,szFramBuf + fIsLogin_off,fIsLogin_len);
	memcpy((char *)&gstBankParam.cIsLock,szFramBuf + fIsLock_off,fIsLock_len);
	memcpy(gstBankParam.szTraceNo,szFramBuf + fTraceNo_off,fTraceNo_len);
	memcpy(gstBankParam.szInvoiceNo,szFramBuf + fInvoiceNo_off,fInvoiceNo_len);
	memcpy(gstBankParam.sLastLoginDateTime,szFramBuf + fLastLoginDateTime_off,fLastLoginDateTime_len);

	/**<Emv param*/
	memset(&gstEmvParam,0,sizeof(gstEmvParam));
	memcpy((char *)&gstEmvParam.nTransSerial,szFramBuf + fEmvTransSerial_off,fEmvTransSerial_len);
	memcpy((char *)&gstEmvParam.nOfflineUpNum,szFramBuf + fEmvOfflineUpNum_off,fEmvOfflineUpNum_len);
	memcpy((char *)&gstEmvParam.nOnlineUpNum,szFramBuf + fEmvOnlineUpNum_off,fEmvOnlineUpNum_len);
	memcpy((char *)&gstEmvParam.nOfflineFailUpNum,szFramBuf + fEmvOfflineFailUpNum_off,fEmvOfflineFailUpNum_len);
	memcpy((char *)&gstEmvParam.nOnlineARPCErrUpNum,szFramBuf + fEmvOnlineARPCErrUpNum_off,fEmvOnlineARPCErrUpNum_len);
	memcpy((char *)&gstEmvParam.cIsDownCAPKFlag,szFramBuf + fEmvIsDownCAPKFlag_off,fEmvIsDownCAPKFlag_len);
	memcpy((char *)&gstEmvParam.cIsDownAIDFlag,szFramBuf + fEmvIsDownAIDFlag_off,fEmvIsDownAIDFlag_len);


	return APP_SUCC;
}


/**
* @brief Set system language
* @param int
* @return
* @li cLanguage EM_LANG
* @author 
* @date 
*/
int SetVarSystemLanguage(const EM_LANG cLanguage)
{
	ASSERT_FAIL(SetCustomizedFont(cLanguage));
	gstAppPosParam.cLanguage = cLanguage;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief Get Is login
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsLogin()
{
	if (gstBankParam.cIsLogin==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set login flag
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsLogin(const YESORNO cFlag)
{
	gstBankParam.cIsLogin = cFlag;
	PubSaveVar((char*)&gstBankParam.cIsLogin,fIsLogin_off,fIsLogin_len);
	return APP_SUCC;
}

/**
* @brief Get is lock
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsLock()
{
	if (gstBankParam.cIsLock == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set lock flag
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
*/
int SetVarIsLock(const YESORNO cFlag)
{
	gstBankParam.cIsLock=cFlag;
	PubSaveVar((char*)&gstBankParam.cIsLock,fIsLock_off,fIsLock_len);
	return APP_SUCC;
}


/**
* @brief Get Trace no
* @param out pszTraceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarTraceNo(char *pszTraceNo)
{
	memcpy(pszTraceNo,gstBankParam.szTraceNo,fTraceNo_len);
	return APP_SUCC;
}

/**
* @brief Save trace no
* @param in pszTraceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarTraceNo(const char *pszTraceNo)
{
	memcpy(gstBankParam.szTraceNo,pszTraceNo,fTraceNo_len);
	PubSaveVar(gstBankParam.szTraceNo,fTraceNo_off,fTraceNo_len);
	return APP_SUCC;
}

/**
* @brief Increace Trace No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarTraceNo()
{
	PubIncNum(gstBankParam.szTraceNo);
	SetVarTraceNo(gstBankParam.szTraceNo);
	return APP_SUCC;
}

/**
* @brief Get Batch No
* @param out pszBatchNo Batch No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchNo(char *pszBatchNo, int nAcqIndex)
{
	memcpy(pszBatchNo, gstAppPosParam.lszBatchNo[nAcqIndex], 6);
	
	return APP_SUCC;
}

/**
* @brief Set batch No
* @param in pszBatchNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitVarBatchNo()
{
	int i = 0;

	for (i = 0; i < MAXBATCH_NUM; i++)
	{
		memcpy(gstAppPosParam.lszBatchNo[i],"000001",6);
	}	
	PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam);	
	return APP_SUCC;
}

/**
* @brief Increase Batch no
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarBatchNo(int nAcqIndex)
{
	PubIncNum(gstAppPosParam.lszBatchNo[nAcqIndex]);
	PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam);
	return APP_SUCC;
}

/**
* @brief Get Invoice No
* @param out pszInvoiceNo Fix 6 bytes
* @return
* @li APP_SUCC
* @li APP_FAIL
*/	
int GetVarInvoiceNo(char *pszInvoiceNo)
{
	memcpy(pszInvoiceNo,gstBankParam.szInvoiceNo,fInvoiceNo_len);
	return APP_SUCC;
}

/**
* @brief Save invoice No
* @param in pszInvoiceNo 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarInvoiceNo(const char *pszInvoiceNo)
{
	memcpy(gstBankParam.szInvoiceNo,pszInvoiceNo,fInvoiceNo_len);
	PubSaveVar(gstBankParam.szInvoiceNo,fInvoiceNo_off,fInvoiceNo_len);
	return APP_SUCC;
}

/**
* @brief Increase invoice No
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarInvoiceNo()
{	
	PubIncNum(gstBankParam.szInvoiceNo);
	SetVarInvoiceNo(gstBankParam.szInvoiceNo);
	return APP_SUCC;
}

/**
* @brief Get Is reversal
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsReversal()
{
	if (gstReversalParam.cIsReversal==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save Is reversal
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsReversal(const YESORNO cFlag)
{
	gstReversalParam.cIsReversal=cFlag;
	PubSaveVar((char*)&gstReversalParam.cIsReversal,fIsReversal_off,fIsReversal_len);
	return APP_SUCC;
}


/**
* @brief Get the num the time had sent
* @param out pnNum: the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarHaveReversalNum(int *pnNum)
{
	*pnNum=gstReversalParam.nHaveReversalNum;
	return APP_SUCC;
}

/**
* @brief Save the time had sent
* @param in nNum: the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarHaveReversalNum(const int nNum)
{
	gstReversalParam.nHaveReversalNum=nNum;
	PubSaveVar((char *)&gstReversalParam.nHaveReversalNum,fHaveReversalNum_off,fHaveReversalNum_len);
	return APP_SUCC;
}

/**
* @brief Increase the time had sent
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarHaveReversalNum()
{
	gstReversalParam.nHaveReversalNum++;
	SetVarHaveReversalNum(gstReversalParam.nHaveReversalNum);
	return APP_SUCC;
}

/**
* @brief Get reversal data and len
* @param out psData 
* @param out pnLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarReversalData(char *psData,int *pnLen)
{
	*pnLen=gstReversalParam.nReversalLen;
	memcpy(psData,gstReversalParam.sReversalData,gstReversalParam.nReversalLen);

	return APP_SUCC;
}

/**
* @brief Save reversal data and len
* @param in sData 
* @param in nLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarReversalData(const char *sData,const int nLen)
{
	if ((nLen>fReversalData_len)||(nLen<0))
	{
		TRACE(("nLen=%d max=%d", nLen, fReversalData_len));
		return APP_FAIL;
	}
	memcpy(gstReversalParam.sReversalData,sData,nLen);
	gstReversalParam.nReversalLen=nLen;
	PubSaveVar((char *)&gstReversalParam.nReversalLen,fReversalLen_off,fReversalLen_len);
	PubSaveVar(gstReversalParam.sReversalData,fReversalData_off,fReversalData_len);

	return APP_SUCC;
}

/**
* @brief Get TID
* @param out pszValue 
* @return void
*/
void GetVarTerminalId(char *pszValue, int nIndex)
{
	memcpy(pszValue,gstAppPosParam.szTerminalId[nIndex],sizeof(gstAppPosParam.szTerminalId[nIndex])-1);
	return;
}

/**
* @brief Save TID
* @param out pszValue Max 8 bytes
* @return void
*/
void SetVarTerminalId(char *pszValue, int nIndex)
{
	memcpy(gstAppPosParam.szTerminalId[nIndex],pszValue,sizeof(gstAppPosParam.szTerminalId[nIndex])-1);
	PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam);
	return;
}

/**
* @brief Get MID
* @param out pszValue Max 15 bytes
* @return void
*/
void GetVarMerchantId(char *pszValue, int nAcqIndex)
{
	memcpy(pszValue,gstAppPosParam.szMerchantId[nAcqIndex],sizeof(gstAppPosParam.szMerchantId[nAcqIndex])-1);
	return;
}

/**
* @brief Get MID
* @param out pszValue 
* @return void
*/
void SetVarMerchantId(char *pszValue, int nAcqIndex)
{
	memcpy(gstAppPosParam.szMerchantId[nAcqIndex],pszValue,sizeof(gstAppPosParam.szMerchantId[nAcqIndex])-1);
	PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam);
	return;
}


/**
* @brief Get merchant english name
* @param out pszValue 最长40个字节
* @return void
*/
void GetVarMerchantNameEn(char *pszValue)
{
	memcpy(pszValue,gstAppPosParam.szMerchantNameEn,sizeof(gstAppPosParam.szMerchantNameEn)-1);
	return;
}

/**
* @brief Get is use pinpad
* @return
* @li YES 
* @li NO 
*/
YESORNO GetVarIsPinpad()
{
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		if (gstAppPosParam.cIsPinPad==YES)
		{
			return YES;
		}
		else
		{
			return NO;
		}
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get the pinpad port at terminal
* @param out pnValue Port no，1:AUX1,2:AUX2
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarPinpadAuxNo(int *pnValue)
{
	//Always PORT_NUM_COM2 
	*pnValue = PORT_NUM_COM2;
	return APP_SUCC;
}

/**
* @brief Get pinpad timeout
* @param out pnValue timeout，0-999
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarPinpadTimeOut(int *pnValue)
{
	char szTmp[10]= {0};

	memcpy(szTmp,gstAppPosParam.szPinPadTimeOut,sizeof(gstAppPosParam.szPinPadTimeOut)-1);
	if (atoi(szTmp)>0 && atoi(szTmp)<999)
	{
		*pnValue=atoi(szTmp);
	}
	else
	{
		*pnValue=60;
	}

	return APP_SUCC;
}

/**
* @brief Get password according to type
* @param out pnValue 
* @param in cType See[EM_PWD_TYPE]
* @return void
*/
void GetVarPwd(char *pszValue, uchar cType)
{
	if (EM_TRANS_PWD == cType)
	{
		memcpy(pszValue,gstAppPosParam.szUsrPwd,sizeof(gstAppPosParam.szUsrPwd)-1);
	}
	else if (EM_SYSTEM_PWD)
	{
		memcpy(pszValue,gstAppPosParam.szAdminPwd,sizeof(gstAppPosParam.szAdminPwd)-1);	
	}
	else if (EM_FUNC_PWD)
	{
		memcpy(pszValue,gstAppPosParam.szFuncPwd,sizeof(gstAppPosParam.szFuncPwd)-1);	
	}
	else if (EM_SHOCT_PWD)
	{
		memcpy(pszValue,gstAppPosParam.szShortCutPwd,sizeof(gstAppPosParam.szShortCutPwd)-1);	
	}
	return;
}

/**
* @brief Get mainkey index
* @param out pnValue index no
* @return void
*/
void GetVarMainKeyNo(int *pnValue)
{
	char szTmp[10] = {0};

	memcpy(szTmp,gstAppPosParam.szMainKeyNo,sizeof(gstAppPosParam.szMainKeyNo)-1);
	*pnValue=atoi(szTmp);
	return;
}

/**
* @brief Get key system type
* @param void
* @return
* @li '0' -- MK/SK
* @li '1' -- DUKPT
* @author 
* @date 
*/	
int GetVarKeySystemType(void)
{
	return gstAppPosParam.cKeySystemType;
}

/**
* @brief Save mainkey index
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetVarMainKeyNo(const int nIndex)
{
	sprintf(gstAppPosParam.szMainKeyNo, "%02d", nIndex%100);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set key system type
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionKeySystemType(void)
{
	char* SelMenu[] = {
		"0.MK/SK",
		"1.DUKPT"
		};
	
	ASSERT_RETURNCODE(PubSelectYesOrNo(tr("KEY SYSTEM TYPE"),tr("KEY SYSTEM TYPE"), SelMenu, &gstAppPosParam.cKeySystemType));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	PubSetKeySystemType(gstAppPosParam.cKeySystemType == '0' ? SECRITY_KEYSYSTEM_MSKEY : SECRITY_KEYSYSTEM_DUKPT);
	return APP_SUCC;
}


/**
* @brief Get encrypt mode
* @return
* @li DESMODE_3DES
* @li DESMODE_DES
*/
int  GetVarEncyptMode()
{
	if (gstAppPosParam.cEncyptMode==DESMODE_3DES)
	{
		return DESMODE_3DES;
	}
	return DESMODE_DES;
}
/**
* @brief Save encrypt mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetVarEncyptMode(const char cEncyptMode)
{
	if(DESMODE_3DES== cEncyptMode || DESMODE_DES== cEncyptMode)
	{
		gstAppPosParam.cEncyptMode = cEncyptMode;
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		return APP_SUCC;
	}
	TRACE(("cEncyptMode=%d error",cEncyptMode));
	return APP_FAIL;
}

/**
* @fn SetFunctionDesMode
* @brief 设置密钥算法
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/	
int SetFunctionDesMode(void)
{
	int nSel = 0xff;
	int nStartItem = 1;
	char *pszItems[] = {
		tr("1.Single DES"),
		tr("2.Single 3DES"),
		tr("DUKPT")
	};
	switch (gstAppPosParam.cEncyptMode)
	{
	case DESMODE_DES:
		nSel = 1;
		break;
	case DESMODE_3DES:
		nSel = 2;
		break;
	case ALGORITHEN_DUKPT:
		nSel = 3;
		break;
	default:
		break;
	}
	ASSERT_QUIT(PubShowMenuItems(tr("Set Algorithm"), pszItems, sizeof(pszItems)/sizeof(char *), &nSel, &nStartItem, 60));
	//ASSERT_QUIT(PubSelectListItem("KEY MANAGE", "Set Algorithm", "1.Single DES||2.Single 3DES||3.DUKPT", nSel, &nSel));
	switch (nSel)
	{
	case 1:
		gstAppPosParam.cEncyptMode = DESMODE_DES;
		break;
	case 2:
		gstAppPosParam.cEncyptMode = DESMODE_3DES;
		break;
	case 3:
		gstAppPosParam.cEncyptMode = ALGORITHEN_DUKPT;
		break;
	default:
		break;
	}

	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}


/**
* @brief Get Pin encrypt mode
* @return
* @li PINTYPE_WITHOUTPAN
* @li PINTYPE_WITHPAN
*/
int GetVarPinEncyptMode()
{
	if (gstAppPosParam.cPinEncyptMode==PINTYPE_WITHOUTPAN)
	{
		return PINTYPE_WITHOUTPAN;
	}
	return PINTYPE_WITHPAN;
}

/**
* @brief Get is display TVR OR TSI during transaction
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsDispEMV_TVRTSI(void)
{
	if (gstAppPosParam.cIsDispEMV_TVRTSI==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Get is shield pan
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsShieldPan(void)
{
	if (gstAppPosParam.cIsShieldPan != YES  && gstAppPosParam.cIsShieldPan != NO)
	{
		gstAppPosParam.cIsShieldPan = YES;
	}
	if (gstAppPosParam.cIsShieldPan == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Get is print water
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPrintWaterRec()
{
	if (gstAppPosParam.cIsPrintWaterRec==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Is preauth shield pan
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPreauthShieldPan()
{
	if (gstAppPosParam.cIsPreauthShieldPan==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is print err report
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsPrintErrReport()
{
	if (gstAppPosParam.cIsPrintErrReport==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get the count of print page
* @param out pnValue page count
* @return void
*/
void GetVarPrintPageCount(int *pnValue)
{
	if (gstAppPosParam.cPrintPageCount>='0' && gstAppPosParam.cPrintPageCount<='9')
	{
		*pnValue = gstAppPosParam.cPrintPageCount - 0x30;
	}
	else
	{
		*pnValue=1;
	}
	return;
}

/**
* @brief Get MAX trans count, more than this count , you need to settle
* @param out pnValue 1-99999
* @return void
*/
void GetVarMaxTransCount(int *pnValue)
{
	char szTmp[10] = {0};

	memcpy(szTmp,gstAppPosParam.szMaxTransCount,sizeof(gstAppPosParam.szMaxTransCount)-1);
	if (atoi(szTmp)>0&&atoi(szTmp)<99999)
	{
		*pnValue=atoi(szTmp);
	}
	else
	{
		*pnValue=1000;
	}
	return;
}

/**
* @brief Get Max refund amount
* @param out szMaxRefundAmt
* @return void
*/
void GetVarMaxRefundAmount(char *szMaxRefundAmt)
{
	memcpy(szMaxRefundAmt, gstAppPosParam.szMaxRefundAmount,sizeof(gstAppPosParam.szMaxRefundAmount)-1);
	return;
}

/**
* @brief Get tip rate
* @param out psValue 2bytes
* @return void
*/
void GetVarTipRate(char *psValue)
{
	memcpy(psValue,gstAppPosParam.szBaksheeshPrim,4);
	return;
}

/**
* @brief Save tip rate
* @param out psValue 2bytes
* @return
* @li APP_SUCC 
* @li APP_FAIL
*/
int SetVarTipRate(const char *psValue)
{
	memcpy(gstAppPosParam.szBaksheeshPrim, psValue, 4);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Get Is support tip
* @return
* @li YES support
* @li NO nonsupport
*/
YESORNO GetVarIsTipFlag(void)
{
	if (gstAppPosParam.cIsTipFlag==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save is support tip
* @param in pFlag YES/NO
* @return
* @li APP_SUCC 
* @li APP_FAIL 
*/
int SetVarIsTipFlag(const YESORNO pFlag)
{
	gstAppPosParam.cIsTipFlag = pFlag;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief Get is support Chip 
* @return
* @li YES 
* @li NO 
*/
YESORNO GetVarIsIcFlag()
{
	if (gstAppPosParam.cIsIcFlag==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is support manual
* @return
* @li YES 
* @li NO 
*/
YESORNO GetVarIsCardInput()
{
	if (gstAppPosParam.cIsCardInput==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save is support manual
* @param in pFlag YES/NO
* @return
* @li APP_SUCC 
* @li APP_FAIL
*/
int SetVarIsCardInput(const YESORNO cFlag)
{
	gstAppPosParam.cIsCardInput = cFlag;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Get is void need to insert card
* @return
* @li YES need 
* @li NO  no need
*/
YESORNO GetVarIsSaleVoidStrip()
{
	if (gstAppPosParam.cIsSaleVoidStrip==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get void is need to Get Pin
* @return
* @li YES need
* @li NO void no need 
*/
YESORNO GetVarIsVoidPin()
{
	if (gstAppPosParam.cIsVoidPin==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get default transtype
* @param out pcValue '0'-Sale，'1'-Preauth
* @return void
*/
void GetVarDefaultTransType(char *pcValue)
{
	gstAppPosParam.cDefaultTransType = gstAppPosParam.cDefaultTransType == '0' ? '0' : '1';
	*pcValue=gstAppPosParam.cDefaultTransType;
	return;
}

/**
* @brief Get emv trans serial
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarEmvTransSerial(int *pnSerial)
{
	gstEmvParam.nTransSerial = gstEmvParam.nTransSerial%99999999;
	*pnSerial = gstEmvParam.nTransSerial == 0 ? ++gstEmvParam.nTransSerial : gstEmvParam.nTransSerial;
	return APP_SUCC;
}

/**
* @brief Save emv trans serial
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvTransSerial(const int nSerial)
{
	gstEmvParam.nTransSerial=nSerial%99999999;
	PubSaveVar((char *)&gstEmvParam.nTransSerial,fEmvTransSerial_off,fEmvTransSerial_len);
	return APP_SUCC;
}

/**
* @brief emv increase emv trans serial 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarEmvTransSerial()
{
	gstEmvParam.nTransSerial = ++gstEmvParam.nTransSerial%99999999;
#if 0 
	SetVarEmvTransSerial(gstEmvParam.nTransSerial);
#endif
	return APP_SUCC;
}

/**
* @brief Save emv trans serial
* @return
* @li APP_SUCC
*/
int SaveEmvTransSerial(void)
{
	SetVarEmvTransSerial(gstEmvParam.nTransSerial);
	return APP_SUCC;
}



/**
* @brief Get emv offline send num
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarEmvOfflineUpNum(int *pnSerial)
{
	*pnSerial=gstEmvParam.nOfflineUpNum;
	return APP_SUCC;
}


/**
* @brief Save emv offline send num
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvOfflineUpNum(const int nSerial)
{
	gstEmvParam.nOfflineUpNum=nSerial;
	PubSaveVar((char *)&gstEmvParam.nOfflineUpNum,fEmvOfflineUpNum_off,fEmvOfflineUpNum_len);
	return APP_SUCC;
}

/**
* @brief
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarEmvOfflineUpNum()
{
	gstEmvParam.nOfflineUpNum++;
	SetVarEmvTransSerial(gstEmvParam.nOfflineUpNum);
	return APP_SUCC;
}

/**
* @brief 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarEmvOnlineUpNum(int *pnSerial)
{
	*pnSerial=gstEmvParam.nOnlineUpNum;
	return APP_SUCC;
}


/**
* @brief 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvOnlineUpNum(const int nSerial)
{
	gstEmvParam.nOnlineUpNum=nSerial;
	PubSaveVar((char *)&gstEmvParam.nOnlineUpNum,fEmvOnlineUpNum_off,fEmvOnlineUpNum_len);
	return APP_SUCC;
}

/**
* @brief 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarEmvOnlineUpNum()
{
	gstEmvParam.nOnlineUpNum++;
	SetVarEmvTransSerial(gstEmvParam.nOnlineUpNum);
	return APP_SUCC;
}

/**
* @brief
* @param in nOfflineFailUpNum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEMVOfflineFailHaltFlag(const int nOfflineFailUpNum)
{

	gstEmvParam.nOfflineFailUpNum = nOfflineFailUpNum;
	PubSaveVar( (char *)(&gstEmvParam.nOfflineFailUpNum), fEmvOfflineFailUpNum_off, fEmvOfflineFailUpNum_len);
	return APP_SUCC;
}

/**
* @brief
* @param in nOnlineARPCErrUpNum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEMVOnlineARPCErrHaltFlag(const int nOnlineARPCErrUpNum)
{

	gstEmvParam.nOnlineARPCErrUpNum = nOnlineARPCErrUpNum;
	PubSaveVar( (char *)(&gstEmvParam.nOnlineARPCErrUpNum), fEmvOnlineARPCErrUpNum_off, fEmvOnlineARPCErrUpNum_len);
	return APP_SUCC;
}

/**
* @brief 
* @param in pnOnlineARPCErrUpNum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarEMVOnlineARPCErrHaltFlag(int *pnOnlineARPCErrUpNum)
{

	*pnOnlineARPCErrUpNum = gstEmvParam.nOnlineARPCErrUpNum;
	return APP_SUCC;
}

/**
* @brief Get is download CAPK
* @return
* @li YES
* @li NO
*/
YESORNO GetVarEmvIsDownCAPKFlag()
{
	if (gstEmvParam.cIsDownCAPKFlag==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Save Is download CAPK flag
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvIsDownCAPKFlag(const YESORNO cFlag)
{
	gstEmvParam.cIsDownCAPKFlag=cFlag;
	PubSaveVar((char*)&gstEmvParam.cIsDownCAPKFlag,fEmvIsDownCAPKFlag_off,fEmvIsDownCAPKFlag_len);
	return APP_SUCC;
}

/**
* @brief Get is download AID
* @return
* @li YES
* @li NO
*/
YESORNO GetVarEmvIsDownAIDFlag()
{
	if (gstEmvParam.cIsDownAIDFlag==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Get is download AID flag
* @param in pFlag YES/NO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarEmvIsDownAIDFlag(const YESORNO cFlag)
{
	gstEmvParam.cIsDownAIDFlag=cFlag;
	PubSaveVar((char*)&gstEmvParam.cIsDownAIDFlag,fEmvIsDownAIDFlag_off,fEmvIsDownAIDFlag_len);
	return APP_SUCC;
}

/**
* @brief Set Mainkey index
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMainKeyNo(void)
{
	int nLen, nNumber;
	char szNumber[2+1] = {0};

	gstAppPosParam.szMainKeyNo[2] = 0;
	nNumber = atoi(gstAppPosParam.szMainKeyNo);
	sprintf(szNumber, "%1.1u", nNumber%10);
	ASSERT_QUIT(PubInputDlg("SET KEY INDEX", "ENTER KEY INDEX(0-9):", szNumber, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
	sprintf(gstAppPosParam.szMainKeyNo, "0%s", szNumber);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set Max refund amount
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMaxRefundAmount(void)
{
	int nLen = 12;

	gstAppPosParam.szMaxRefundAmount[12] = 0;
	ASSERT_RETURNCODE(PubInputAmount("OTHER CONTROL", "MAX REFUND AMOUT:", gstAppPosParam.szMaxRefundAmount, &nLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set print page
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPrintPageCount(void)
{
	char szNum[2] = {0};
	int nNum, nLen;
	
	if ( PubGetPrinter() != _PRINTTYPE_TP )
	{
		PubMsgDlg("Warning","NONSUPPORT", 3, 1);
		return APP_FUNCQUIT;
	}


	while(1)
	{
		szNum[0] = gstAppPosParam.cPrintPageCount;
		ASSERT_RETURNCODE( PubInputDlg("Receipt pages", "Num of page(1-3):", szNum, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
		nNum = atoi(szNum);
		if( (nNum >= 1) && (nNum <= 3) )
		{
			break;
		}
		PubMsgDlg("Receipt pages","INVALID INPUT",1,1);
	}
	gstAppPosParam.cPrintPageCount = '0' + atoi(szNum);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set max trans count
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMaxTransCount(void)
{
	int nLen, nNumber;
	char szNumber[5+1] = {0};


	nNumber = atoi(gstAppPosParam.szMaxTransCount);

	while(1)
	{
		sprintf(szNumber, "%d", nNumber );
		ASSERT_RETURNCODE(PubInputDlg("TRANS COUNT", "MAX TRANS COUNT\n(   <=500):", szNumber, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
		nNumber = atoi(szNumber);
		if ( (nNumber>0 )&&(nNumber <=500) )
		{
			break;
		}
		PubMsgDlg("TRANS COUNT","INVALID INPUT!",0,1);
	}
	sprintf(gstAppPosParam.szMaxTransCount, "%05d", nNumber);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set MID
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantId(void)
{
	int nLen, nWaterSum;
	char szMerchantId[15+1] = {0};
	int nIndex = 0;
	char szTitle[32] = {0};

	while (1)
	{
		if (nIndex >= ACQUIRER_NUM)
		{
			break;
		}
		memcpy(szMerchantId, gstAppPosParam.szMerchantId[nIndex], sizeof(gstAppPosParam.szMerchantId[nIndex]));
		strcpy(szTitle, glszAcquirerName[nIndex]);
		ASSERT_RETURNCODE(PubInputDlg(szTitle, "MERCHANT ID(15):", szMerchantId, &nLen, 8, 15, 0, INPUT_MODE_STRING));
		if (memcmp(szMerchantId, gstAppPosParam.szMerchantId[nIndex], 15) !=0)
		{
			GetRecordNum(&nWaterSum);
			if (nWaterSum > 0)
			{
				PubMsgDlg("MERCHANT ID", "PLS SETTLE FITSTLY", 3, 10);
				return APP_FAIL;
			}
			//PubAddSymbolToStr(szMerchantId, 15, ' ', 1);
			memcpy(gstAppPosParam.szMerchantId[nIndex], szMerchantId, sizeof(gstAppPosParam.szMerchantId[nIndex]));
			ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM, 1, (char *)&gstAppPosParam));
		}
		nIndex++;
	}

	return APP_SUCC;
}

/**
* @brief Set TID
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTerminalId(void)
{
	int nLen = 0, nWaterSum = 0;
	char szTerminalId[8+1] = {0};
	int nIndex = 0;
	char szTitle[32] = {0};
	
	while (1)
	{
		if (nIndex >= ACQUIRER_NUM)
		{
			break;
		}
		memcpy(szTerminalId, gstAppPosParam.szTerminalId[nIndex], sizeof(gstAppPosParam.szTerminalId[nIndex]));
		strcpy(szTitle, glszAcquirerName[nIndex]);
		ASSERT_RETURNCODE(PubInputDlg(szTitle, "TERMINAL ID(8):", szTerminalId, &nLen, 8, 8, 0, INPUT_MODE_STRING));
		if (memcmp(szTerminalId, gstAppPosParam.szTerminalId[nIndex], 8) !=0)
		{
			GetRecordNum(&nWaterSum);
			if (nWaterSum > 0)
			{
				PubMsgDlg("MERCHANT ID", "PLS SETTLE FITSTLY", 3, 10);
				return APP_FAIL;
			}
			memcpy( gstAppPosParam.szTerminalId[nIndex], szTerminalId, sizeof(gstAppPosParam.szTerminalId[nIndex] ) );
			ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		}
		nIndex++;
	}


	return APP_SUCC;
}

/**
* @brief Set APP name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionAppName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szAppDispname);
	ASSERT_RETURNCODE(PubInputDlg("APP NAME", NULL, szTmp, &nLen, 0, 14, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szAppDispname, szTmp);	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief Set merchant address
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantAddr(void)
{
	int nLen = 0;
	char szTmp[128] = {0};
	int i = 0;
	char szContent[128] = {0};

	for (i = 0; i < 4; i++)
	{
		strcpy(szTmp, gstAppPosParam.szMerchantAddr[i]);
		sprintf(szContent, "ADDRESS lINE %d", i+1);
		ASSERT_RETURNCODE(PubInputDlg(szContent, NULL, szTmp, &nLen, 0, 80, 0, INPUT_MODE_STRING));
		strcpy(gstAppPosParam.szMerchantAddr[i], szTmp);	
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	}
	return APP_SUCC;
}

/**
* @brief Set merchant name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMerchantName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szMerchantNameEn);
	ASSERT_RETURNCODE(PubInputDlg("MERCHANT NAME", NULL, szTmp, &nLen, 0, 40, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szMerchantNameEn, szTmp);	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief  Set stan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTraceNo(void)
{
	int nLen,nWaterSum;
	char szTraceNo[fTraceNo_len+1]= {0};

	memcpy(szTraceNo, gstBankParam.szTraceNo, fTraceNo_len);
	ASSERT_RETURNCODE(PubInputDlg("TRACE NO", "ENTER:", szTraceNo, &nLen, fTraceNo_len, fTraceNo_len, 0, INPUT_MODE_NUMBER));
	if (memcmp(gstBankParam.szTraceNo, szTraceNo, fTraceNo_len) !=0)
	{
		GetRecordNum(&nWaterSum);
		if (nWaterSum > 0)
		{
			PubMsgDlg("TRACE NO", "PLS SETTLE FIRSTLT", 3, 10);
			return APP_FAIL;
		}
		memcpy(gstBankParam.szTraceNo, szTraceNo, fTraceNo_len);
		PubSaveVar(gstBankParam.szTraceNo,fTraceNo_off,fTraceNo_len);
	}
	return APP_SUCC;
}

/**
* @brief  Set batch no
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionBatchNo(void)
{
	int nLen,nWaterSum;
	char szBatchNo[6+1] = {0};
	int nBatchIndex = 0;
	char szTitle[32] = {0};
	
//SSERT_QUIT(GetBatchIndex(nAcqIndex, nEppMonth, &nBatchIndex));	

	while (1)
	{
		if (nBatchIndex >= MAXBATCH_NUM)
		{
			break;
		}
		memcpy(szBatchNo, gstAppPosParam.lszBatchNo[nBatchIndex], sizeof(gstAppPosParam.lszBatchNo[nBatchIndex]));
		strcpy(szTitle, glszBatchIndexName[nBatchIndex]);
		ASSERT_RETURNCODE(PubInputDlg(szTitle, "BATCH NO(6):", szBatchNo, &nLen, 6, 6, 0, INPUT_MODE_STRING));
		if (memcmp(szBatchNo, gstAppPosParam.lszBatchNo[nBatchIndex], 6) !=0)
		{
			if (CheckRecordNumWithBatch(szBatchNo) == APP_SUCC)//该批次有流水
			{
				PubMsgDlg("MERCHANT ID", "PLS SETTLE FITSTLY", 3, 10);
				return APP_FAIL;
			}
			memcpy(gstAppPosParam.lszBatchNo[nBatchIndex], szBatchNo, sizeof(gstAppPosParam.lszBatchNo[nBatchIndex] ) );
			ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		}
		nBatchIndex++;
	}
 
	return APP_SUCC;
}

static char gcIsChkPinpad = YES;/**<Is need to check pinpad*/
static char gcIsChkInside = YES;/**<Is inside pinpad init>*/
static char gcIsChkRF = YES; /**<Is need to check RF(contactless)>*/

/*
* @brief Save Is need to check pinpad
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkPinpad(const YESORNO cIsChk)
{
	gcIsChkPinpad = cIsChk;
}
/**
* @brief Save inside pinpad init
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkInside(const YESORNO cIsChk)
{
	gcIsChkInside = cIsChk;
}
/**
* @brief 
* @param in cIsChk YES/NO
* @return  void
*/
void SetControlChkRF(const YESORNO cIsChk)
{
	gcIsChkRF = cIsChk;
}

/**
* @brief Get is check pinpad
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkPinpad(void)
{
	if (gcIsChkPinpad==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}
/**
* @brief 
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkInside(void)
{
	if (gcIsChkInside==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}
/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetControlChkRF(void)
{
	if (gcIsChkRF ==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Set is check pinpad
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPinPad(void)
{
	char *SelMenu[] = {
		"0.BUILT-IN",
		"1.EXTERNAL"
	};
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		ASSERT_RETURNCODE(PubSelectYesOrNo("PINPAD", "EXTERNAL PINPAD", SelMenu, &gstAppPosParam.cIsPinPad));
		//ASSERT_RETURNCODE(ProSelectYesOrNo("PINPAD", "EXTERNAL PINPAD", "0.BUILT-IN|1.EXTERNAL", &gstAppPosParam.cIsPinPad));
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	}
	else
	{
		gstAppPosParam.cIsPinPad= NO;
		PubMsgDlg("Warning", "BUILDIN SUPPORT ONLY", 1, 3);
	}
	if (gstAppPosParam.cIsPinPad == YES)
	{
		SetFunctionPinpadTimeOut();
	}
	ChkPinpad();
	return APP_SUCC;
}

/**
* @brief Set offline send type
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsOfflineSendNow(void)
{
	char *SelMenu[] = {
		"0.BEFORE SETTLE",
		"1.NEXT ONLINE"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("OFF SEND TYPE", NULL, SelMenu, &gstAppPosParam.cIsOfflineSendNow));
	//ASSERT_RETURNCODE( ProSelectYesOrNo("OFF SEND TYPE", NULL, "0.BEFORE SETTLE||1.NEXT ONLINE", &gstAppPosParam.cIsOfflineSendNow));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set is support chip
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsIcFlag(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER", "SUPPORT INSERTIC", NULL, &gstAppPosParam.cIsIcFlag));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER", "SUPPORT INSERTIC", "0.NO|1.YES", &gstAppPosParam.cIsIcFlag));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set is support manual
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsManul(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("PAYMENT TYPE", "MANUAL ENABLE", NULL, &gstAppPosParam.cIsCardInput));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("PAYMENT TYPE", "MANUAL ENABLE", "0.NO|1.YES", &gstAppPosParam.cIsCardInput));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set is print error report
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPrintErrReport(void)
{
	ASSERT_QUIT(PubSelectYesOrNo("OTHER CONTROL", "PRINT FAULT REPORT", NULL, &gstAppPosParam.cIsPrintErrReport));
	//ASSERT_QUIT(ProSelectYesOrNo("OTHER CONTROL", "PRINT FAULT REPORT", "0.NO|1.YES", &gstAppPosParam.cIsPrintErrReport));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set dafault transaction type
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionDefaultTransType(void)
{
	char *SelMenu[] = {
		"0.AUTH",
		"1.SALE"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER CONTROL", "DEFAULT TRANSACTION", SelMenu, &gstAppPosParam.cDefaultTransType));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER CONTROL", "DEFAULT TRANSACTION", "0.AUTH|1.SALE", &gstAppPosParam.cDefaultTransType));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set void is need to input pin
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsVoidPin(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("SET VOID", "Void Input Password", NULL, &gstAppPosParam.cIsVoidPin));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET VOID", "Void Input Password", "0.NO|1.YES", &gstAppPosParam.cIsVoidPin));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set void is need to Insert card
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFunctionIsSaleVoidStrip(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("SWIPE ENABLE", "Void Swipe", NULL, &gstAppPosParam.cIsSaleVoidStrip));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SWIPE ENABLE", "Void Swipe", "0.NO|1.YES", &gstAppPosParam.cIsSaleVoidStrip));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set pin encrypt mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPinEncyptMode(void)
{
	char *SelMenu[] = {
		"0.With Pan",
		"1.Without Pan"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("Set terminal", "PIN MODE", SelMenu, &gstAppPosParam.cPinEncyptMode));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("Set terminal", "PIN MODE", "0.With Pan||1.Without Pan", &gstAppPosParam.cPinEncyptMode));
	if('0' == gstAppPosParam.cPinEncyptMode)
	{
		gstAppPosParam.cPinEncyptMode = PINTYPE_WITHPAN;
	}
	else
	{
		gstAppPosParam.cPinEncyptMode = PINTYPE_WITHOUTPAN;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set Preauth is shield pan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPreauthShieldPan(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER", "IS PREAUTH SHIELDED PAN", NULL, &gstAppPosParam.cIsPreauthShieldPan));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER", "IS PREAUTH SHIELDED PAN", "0.NO|1.YES", &gstAppPosParam.cIsPreauthShieldPan));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set is print detail
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsPrintWaterRec(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("SETTLE CONTROL", "PRINT DETAIL", NULL, &gstAppPosParam.cIsPrintWaterRec));
	//ASSERT_RETURNCODE( ProSelectYesOrNo("SETTLE CONTROL", "PRINT DETAIL", "0.NO|1.YES", &gstAppPosParam.cIsPrintWaterRec));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set is support tip
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsTipFlag(void)
{
	if (gstAppPosParam.cIsTipFlag != '0' && gstAppPosParam.cIsTipFlag != '1' )
	{
		gstAppPosParam.cIsTipFlag = '1';
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER CONTROL", "TIP ENABLE", NULL, &gstAppPosParam.cIsTipFlag));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER CONTROL", "TIP ENABLE", "0.NO|1.YES", &gstAppPosParam.cIsTipFlag));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
#if 0//
	if( gstAppPosParam.cIsTipFlag == '1' )
	{
		return SetFunctionTipRate();
	}
#endif	
	return APP_SUCC;
}

/**
* @brief Set tip rate
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTipRate()
{
	int nLen;
	
	if( gstAppPosParam.szTipRate[0] == 0 )
	{
		strcpy( gstAppPosParam.szTipRate, "15" );
	}

	ASSERT_RETURNCODE(PubInputDlg("SET TIP RATE", "Tip rate(%):", gstAppPosParam.szTipRate, &nLen, 1, 2, 0, INPUT_MODE_NUMBER));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set is show TVR and TSI
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsDispEMV_TVRTSI(void)
{
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER", "SHOW TVR TSI IN TRANS", NULL, &gstAppPosParam.cIsDispEMV_TVRTSI));
	//ASSERT_QUIT(ProSelectYesOrNo("OTHER", "SHOW TVR TSI IN TRANS", "0.NO|1.YES", &gstAppPosParam.cIsDispEMV_TVRTSI));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Set is shield pan
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsShieldPan(void)
{
	if (gstAppPosParam.cIsShieldPan != YES  && gstAppPosParam.cIsShieldPan != NO)
	{
		gstAppPosParam.cIsShieldPan = YES;
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo("SYSTEM MANAGE", "Shield pan", NULL, &gstAppPosParam.cIsShieldPan));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SYSTEM MANAGE", "Shield pan", "0.NO|1.YES", &gstAppPosParam.cIsShieldPan));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief  Check the password is correct
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ProCheckPwd(char *pszTitle, uchar cType)
{
	int nLen;
	char szAdminPasswd[8+1];
	char szThePwd[8+1] = {0};
	char szTmp[128] = {0};
	
#ifdef DEMO
	return APP_SUCC;
#endif

	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		strcpy(szThePwd, gstAppPosParam.szAdminPwd);
		strcpy(szTmp, "SYSTEM PASSWORD:");
		break;
	case EM_TRANS_PWD:
		strcpy(szThePwd, gstAppPosParam.szUsrPwd);
		strcpy(szTmp, "INPUT PASSWORD:");
		break;
	case EM_FUNC_PWD:
		strcpy(szThePwd, gstAppPosParam.szFuncPwd);
		strcpy(szTmp, "FUNCTION PASSWORD:");
		break;
	case EM_SHOCT_PWD:
		strcpy(szThePwd, gstAppPosParam.szShortCutPwd);
		strcpy(szTmp, "SHORTCUT PASSWORD:");
		break;
	}	
	
	while(1)
	{
		memset(szAdminPasswd, 0, sizeof(szAdminPasswd));
		ASSERT_QUIT(PubInputDlg(pszTitle, szTmp, szAdminPasswd, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));
		if(strlen(szThePwd) != nLen
			|| memcmp(szThePwd, szAdminPasswd, nLen) != 0)
		{
			char szDispBuf[100];
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "ERROR,TRY AGAIN");
			PubMsgDlg(pszTitle, szDispBuf, 3, 1 ) ;
			continue;
		}
		break;
	}
	return APP_SUCC;
}


/**
* @brief Set system font size
* @return
* @li font size
* @author 
*/
int GetVarSystemFontSize(void)
{
	return gstAppPosParam.cFontSize;
}


/**
* @brief Get system language
* @return
* @li EM_LANG
* @author 
*/
int GetVarSystemLanguage(void)
{
	return gstAppPosParam.cLanguage;
}

/**
* @brief Set system font size
* @param int
* @return
* @li cFontSize font size
* @author 
* @date 
*/
int SetVarSystemFontSize(const char cFontSize)
{
	PubSetDispForm(cFontSize, cFontSize / 4);
	gstAppPosParam.cFontSize = cFontSize;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief  Modify password
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ChangePwd(uchar cType)
{
	int nLen;
	char szAdminPasswd1[16+1];
	char szAdminPasswd2[16+1];
	char szTmpCheck[64] = {0}, szTmpOld[64] = {0}, szTmpNew[64] = {0}, szTitle[32] = "CHANGE PASSWORD";
	char szThePwd[16+1] = {0};

	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		strcpy(szThePwd, gstAppPosParam.szAdminPwd);
		strcpy(szTmpCheck, "System password:");
		break;
	case EM_TRANS_PWD:
		strcpy(szThePwd, gstAppPosParam.szUsrPwd);
		strcpy(szTmpCheck, "User password:");
		break;
	case EM_FUNC_PWD:
		strcpy(szThePwd, gstAppPosParam.szFuncPwd);
		strcpy(szTmpCheck, "Func password:");
		break;
	case EM_SHOCT_PWD:
		strcpy(szThePwd, gstAppPosParam.szShortCutPwd);
		strcpy(szTmpCheck, "ShortC password:");
		break;
	}	
	strcpy(szTmpOld, "New password:");
	strcpy(szTmpNew, "Confirm new password:");

	memset(szAdminPasswd1, 0, sizeof(szAdminPasswd1));

	ASSERT_QUIT(PubInputDlg("VERIFY PASSWORD", szTmpCheck, szAdminPasswd1, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));
	if(nLen != strlen(szThePwd) 
		|| memcmp(szThePwd, szAdminPasswd1, nLen) != 0 )
	{
		PubMsgDlg(szTitle, "Verify failed", 0, 5) ;
		return APP_FAIL;
	}
	
	while(1)
	{
		/**
		* input new password
		*/
		memset(szAdminPasswd1, 0, sizeof(szAdminPasswd1));
		ASSERT_QUIT(PubInputDlg(szTitle, szTmpOld, szAdminPasswd1, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));

		memset(szAdminPasswd2, 0, sizeof(szAdminPasswd2));
		ASSERT_QUIT(PubInputDlg(szTitle, szTmpNew, szAdminPasswd2, &nLen, 1, 16, 0, INPUT_MODE_PASSWD));

		if(memcmp(szAdminPasswd1, szAdminPasswd2, 16) != 0)
		{
			PubMsgDlg(szTitle, "Old and new are mismatch", 3, 1) ;
			continue;
		}
		else
		{
			SetVarPwd(szAdminPasswd1, cType);
			PubMsgDlg(szTitle, "CHANGE OK", 1, 1) ;
			return APP_SUCC;
		}
	}
}


/**
* @brief  Save password
* @param in szSecurity Passwd 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPwd(const char *szSecurityPasswd, uchar cType)
{
	switch (cType)
	{
	case EM_NONE_PWD:
		return APP_SUCC;
		break;
	case EM_SYSTEM_PWD:
		memset(gstAppPosParam.szAdminPwd,0,sizeof(gstAppPosParam.szAdminPwd));
		strcpy(gstAppPosParam.szAdminPwd, szSecurityPasswd);
		break;
	case EM_TRANS_PWD:
		memset(gstAppPosParam.szUsrPwd,0,sizeof(gstAppPosParam.szUsrPwd));
		strcpy(gstAppPosParam.szUsrPwd, szSecurityPasswd);
		break;
	case EM_FUNC_PWD:
		memset(gstAppPosParam.szFuncPwd,0,sizeof(gstAppPosParam.szFuncPwd));
		strcpy(gstAppPosParam.szFuncPwd, szSecurityPasswd);
		break;
	case EM_SHOCT_PWD:
		memset(gstAppPosParam.szShortCutPwd,0,sizeof(gstAppPosParam.szShortCutPwd));
		strcpy(gstAppPosParam.szShortCutPwd, szSecurityPasswd);
		break;
	}	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief Save water number
* @param in nWaterSum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarWaterSum(const int  nWaterSum)
{
	gstSettleParam.nWaterSum = nWaterSum;
	PubSaveVar( (char *)&(gstSettleParam.nWaterSum), fWaterSum_off, fWaterSum_len);
	return APP_SUCC;
}

/**
* @brief Get water num
* @param in pnWaterSum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarWaterSum(int *pnWaterSum)
{
	*pnWaterSum = gstSettleParam.nWaterSum;
	return APP_SUCC;
}

/**
* @brief Save count had sent
* @param in nHaveReSendNum
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarHaveReSendNum(const int  nHaveReSendNum)
{
	gstSettleParam.nHaveReSendNum = nHaveReSendNum;
	PubSaveVar( (char *)&(gstSettleParam.nHaveReSendNum), fHaveReSendNum_off, fHaveReSendNum_len);
	return APP_SUCC;
}

/**
* @brief Get count had sent
* @param in pnHaveReSendNum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarHaveReSendNum(int *pnHaveReSendNum)
{
	*pnHaveReSendNum = gstSettleParam.nHaveReSendNum;
	return APP_SUCC;
}

/**
* @brief Increase count had sent
* @param in void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IncVarOfflineUnSendNum()
{
	int nNum = 0;
	PubGetVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	nNum++;
	PubSaveVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	return APP_SUCC;
}

/**
* @brief Decrease the offline count whcich have not sent
* @param in void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int DelVarOfflineUnSendNum()
{
	int nNum = 0;
	PubGetVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	nNum--;
	if(nNum <0)
		nNum = 0;
	PubSaveVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	return APP_SUCC;
}


/**
* @brief Get offline count have not sent
* @param in void
* @return
* @li
*/
int GetVarOfflineUnSendNum()
{
	int nNum = 0;
	PubGetVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	return nNum;
}


/**
* @brief Save offline count have not sent
* @param in void
* @return
* @li APP_SUCC
*/
int SetVarOfflineUnSendNum(int nNum)
{
	PubSaveVar( (char *)&nNum, fOfflineUnSendNum_off, fOfflineUnSendNum_len);
	return APP_SUCC;
}

/**
* @brief
* @param in 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintSettleHalt(const YESORNO cPrintSettleHalt)
{
	gstSettleParam.cPrintSettleHalt = cPrintSettleHalt;
	PubSaveVar( (char *)&(gstSettleParam.cPrintSettleHalt), fSettlePrintHaltFlag_off, fSettlePrintHaltFlag_len);
	return APP_SUCC;
}

/**
* @brief 
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintSettleHalt()
{
	if (gstSettleParam.cPrintSettleHalt==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief 
* @param in 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintDetialHalt(const YESORNO cPrintDetialHalt)
{
	gstSettleParam.cPrintDetialHalt = cPrintDetialHalt;
	PubSaveVar( (char *)&(gstSettleParam.cPrintDetialHalt), fPrintDetialHaltFlag_off, fPrintDetialHaltFlag_len);
	return APP_SUCC;
}

/**
* @brief 
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintDetialHalt(void)
{
	if (gstSettleParam.cPrintDetialHalt == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief
* @param in 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarPrintWaterHaltFlag(const YESORNO cPrintWaterHaltFlag)
{
	gstSettleParam.cPrintWaterHaltFlag= cPrintWaterHaltFlag;
	PubSaveVar( (char *)&(gstSettleParam.cPrintWaterHaltFlag), fPrintWaterHaltFlag_off, fPrintWaterHaltFlag_len);
	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarPrintWaterHaltFlag(void)
{
	if (gstSettleParam.cPrintWaterHaltFlag== YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief 
* @param in
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarClrSettleDataFlag(const YESORNO cClrSettleDataFlag)
{
	gstSettleParam.cClrSettleDataFlag = cClrSettleDataFlag;
	PubSaveVar( (char *)&(gstSettleParam.cClrSettleDataFlag), fClrSettleDataFlag_off, fClrSettleDataFlag_len);
	return APP_SUCC;
}

/**
* @brief
* @return
* @li YES
* @li NO
*/
YESORNO GetVarClrSettleDataFlag(void)
{
	if (gstSettleParam.cClrSettleDataFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Save is summary match flag
* @param in cIsTotalMatch
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarIsTotalMatch(const char cIsTotalMatch)
{
	gstSettleParam.cIsTotalMatch = cIsTotalMatch;
	PubSaveVar( (char *)&(gstSettleParam.cIsTotalMatch), fCardFlag_off, fCardFlag_len);
	return APP_SUCC;
}

/**
* @brief Get is summary match flag
* @return
* @li YES
* @li NO
*/
char GetVarIsTotalMatch(void)
{
	return gstSettleParam.cIsTotalMatch;
}

/**
* @brief 
* @param in BatchHaltFlag 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchHaltFlag(const YESORNO cBatchHaltFlag)
{
	gstSettleParam.cBatchHaltFlag = cBatchHaltFlag;
	PubSaveVar( (char *)&(gstSettleParam.cBatchHaltFlag), fBatchHaltFlag_off, fBatchHaltFlag_len);
	return APP_SUCC;
}

/**
* @brief 
* @return
* @li YES
* @li NO
*/
YESORNO GetVarBatchHaltFlag(void)
{
	if (gstSettleParam.cBatchHaltFlag == YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief 
* @param in nFinanceHaltFlag 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarFinanceHaltFlag(const int  nFinanceHaltFlag)
{
	gstSettleParam.nFinanceHaltFlag = nFinanceHaltFlag;
	PubSaveVar( (char *)&(gstSettleParam.nFinanceHaltFlag), fFinanceHaltNum_off, fFinanceHaltNum_len);
	return APP_SUCC;
}

/**
* @brief 
* @param in pnFinanceHaltFlag 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarFinanceHaltFlag(int *pnFinanceHaltFlag)
{
	*pnFinanceHaltFlag = gstSettleParam.nFinanceHaltFlag;
	return APP_SUCC;
}

/**
* @brief
* @param in nMessageHaltFlag 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarMessageHaltFlag(const int  nMessageHaltFlag)
{
	gstSettleParam.nMessageHaltFlag = nMessageHaltFlag;
	PubSaveVar( (char *)(&gstSettleParam.nMessageHaltFlag), fMessageHaltNum_off, fMessageHaltNum_len);
	return APP_SUCC;
}

/**
* @brief 
* @param in pnMessageHaltFlag
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarMessageHaltFlag(int *pnMessageHaltFlag)
{
	*pnMessageHaltFlag = gstSettleParam.nMessageHaltFlag;
	return APP_SUCC;
}

/**
* @brief
* @param in nMessageHaltFlag
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchMagOfflineHaltFlag(const int  nBatchMagOfflineHaltFlag)
{
	gstSettleParam.nBatchMagOfflinHaltFlag = nBatchMagOfflineHaltFlag;
	PubSaveVar( (char *)(&gstSettleParam.nBatchMagOfflinHaltFlag), fBatchMagOfflineHaltNum_off, fBatchMagOfflineHaltNum_len);
	return APP_SUCC;
}

/**
* @brief 
* @param in pnMessageHaltFlag 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchMagOfflineHaltFlag(int *pnBatchMagOfflineHaltFlag)
{
	*pnBatchMagOfflineHaltFlag = gstSettleParam.nBatchMagOfflinHaltFlag;
	return APP_SUCC;
}

/**
* @brief Save batch sum
* @param in nBatchSum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarBatchSum(const int  nBatchSum)
{
	gstSettleParam.nBatchSum = nBatchSum;
	PubSaveVar( (char *)&(gstSettleParam.nBatchSum), fBatchUpSum_off, fBatchUpSum_len);
	return APP_SUCC;
}

/**
* @brief Get batch sum
* @param in pnBatchSum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarBatchSum(int *pnBatchSum)
{
	*pnBatchSum = gstSettleParam.nBatchSum;
	return APP_SUCC;
}

/**
* @brief Get settle date
* @param in psSettleDateTime 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarSettleDateTime(char *psSettleDateTime)
{
	memcpy(psSettleDateTime, gstSettleParam.sSettleDateTime, fSettleDateTime_len);
	return APP_SUCC;
}

/**
* @brief Save settle time
* @param in nBatchSum 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarSettleDateTime(const char *psSettleDateTime)
{
	memcpy( gstSettleParam.sSettleDateTime, psSettleDateTime, fSettleDateTime_len);
	PubSaveVar( (char *)&(gstSettleParam.sSettleDateTime), fSettleDateTime_off, fSettleDateTime_len);
	return APP_SUCC;
}


/**
* @brief Save settle data
* @param in stSettle
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarSettleData(const STSETTLE stSettle, int nAcqIndex)
{
	SaveToFile((char *)&stSettle, nAcqIndex*sizeof(STSETTLE), sizeof(STSETTLE), SETTLE_FILE);
	return APP_SUCC;
}

/**
* @brief Get settle data
* @param in pstSettle
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarSettleData(STSETTLE *pstSettle, int nAcqIndex)
{		
	int nLen = sizeof(STSETTLE);
	
	GetFromFile((char *)pstSettle, nAcqIndex*sizeof(STSETTLE), &nLen, SETTLE_FILE);
	return APP_SUCC;
}

/**
* @brief Save settle data item
* @param in psSettDataItem		
* @param in nOffset		
* @param in nLen			
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SaveSettleDataItem( const char * psSettDataItem, const int nOffset, const int nLen, int nAcqIndex)
{
	return SaveToFile(psSettDataItem, nAcqIndex*sizeof(STSETTLE) + nOffset, nLen, SETTLE_FILE);
}

/**
* @brief Get APP POS parameter
* @param out pstAppPosParam  
* @return void
*/
void GetAppPosParam(STAPPPOSPARAM *pstAppPosParam )
{
	memcpy( (char *)pstAppPosParam, (char *)(&gstAppPosParam), sizeof(STAPPPOSPARAM) );
}


/**
* @brief Save APP POS parameter
* @param out pstAppPosParam  
* @return void
*/
int SetAppPosParam(STAPPPOSPARAM *pstAppPosParam )
{
	memcpy((char *)(&gstAppPosParam),  (char *)pstAppPosParam, sizeof(STAPPPOSPARAM));

	return PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam);
}

/**
* @brief Get UID
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarUID(char *psValue)
{
	if (NULL == psValue)
	{
		return APP_FAIL;
	}

	memcpy(psValue, gstAppPosParam.sUID, 16);
	return APP_SUCC;
}


/**
* @brief Get is use extern RF
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsExRF(void)
{
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_PINPAD, NULL))
	{
		if (gstAppPosParam.cIsExRF==YES)
		{
			return YES;
		}
		else
		{
			return NO;
		}
	}
	else
	{
		return NO;
	}
}


/**
* @brief Set is use extern RF
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsExRF(void)
{
	char cOldIsExRF = gstAppPosParam.cIsExRF;
	char *SelMenu[] = {
		"0.INSIDE",
		"1.EXTERN"
	};
	if(gstAppPosParam.cIsSupportRF == NO)
	{
		return APP_FUNCQUIT;
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER", "EXTERN RF", SelMenu, &gstAppPosParam.cIsExRF));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER", "EXTERN RF", "0.INSIDE|1.EXTERN", &gstAppPosParam.cIsExRF));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	if (cOldIsExRF != gstAppPosParam.cIsExRF)
	{
		SetControlChkRF(YES);
	}	

	return APP_SUCC;
}

/**
* @brief Get is use PP60RF(pinpad type)
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsUsePP60RF(void)
{
	if (gstAppPosParam.cIsUsePP60RF==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Set is use PP60RF(pinpad type)
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsUsePP60RF(void)
{
	char *SelMenu[] = {
		"0.829",
		"1.PP60/70"
	};
	if(gstAppPosParam.cIsSupportRF == NO ||gstAppPosParam.cIsExRF== NO)
		return APP_FUNCQUIT;

	if(GetVarIsPinpad() == YES)
	{
		char cOldIsUsePP60RF = gstAppPosParam.cIsUsePP60RF;
		ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER", "EXTERN RF READER", SelMenu, &gstAppPosParam.cIsUsePP60RF));
		//ASSERT_RETURNCODE(ProSelectYesOrNo("OTHER", "EXTERN RF READER", "0.829|1.PP60/70", &gstAppPosParam.cIsUsePP60RF));
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		if (cOldIsUsePP60RF != gstAppPosParam.cIsUsePP60RF)
			SetControlChkRF(YES);
		return APP_SUCC;
	}
	else
	{
		gstAppPosParam.cIsUsePP60RF = NO;
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		return APP_FUNCQUIT;
	}
}

/**
* @brief Get is support RF
* @return
* @li YES
* @li NO
*/
YESORNO GetVarIsSupportRF(void)
{
	if (gstAppPosParam.cIsSupportRF==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}


/**
* @brief Set is support RF
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsSupportRF(void)
{
	char cOldIsSupportRF = gstAppPosParam.cIsSupportRF;
	ASSERT_RETURNCODE(PubSelectYesOrNo("PAYMENT TYPE", "WAVE ENABLE", NULL, &gstAppPosParam.cIsSupportRF));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("PAYMENT TYPE", "WAVE ENABLE", "0.NO|1.YES", &gstAppPosParam.cIsSupportRF));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	if ((NO == cOldIsSupportRF) && (YES == gstAppPosParam.cIsSupportRF))
	{
		SetControlChkRF(YES);
		ChkRF();
	}
	else if(NO == gstAppPosParam.cIsSupportRF)
	{
		SetControlChkRF(NO);
	}
	return APP_SUCC;
}

void GetVarSoftVer(char* pszSoftVer)
{
	memcpy(pszSoftVer, APP_VERSION, 9);
}


int SetFunctionMaxOffSendNum(void)
{
	int nLen;
	int nNumber;
	char szNumber[5+1] = {0};

	nNumber = atoi(gstAppPosParam.szMaxOffSendNum);
	while(1)
	{
		sprintf(szNumber,  "%d", nNumber );
		ASSERT_RETURNCODE(PubInputDlg("OFF SEND NUM", "ENTER NUM", szNumber, &nLen, 1, 2, 0, INPUT_MODE_NUMBER));
		nNumber = atoi(szNumber);
		if (nNumber <= 0)
		{
			PubMsgDlg("OFF SEND NUM","INVALID NUM,TRY AGAIN!",0,3);
			continue;
		}
		
		break;
	}
	sprintf(gstAppPosParam.szMaxOffSendNum, "%02d", nNumber);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


void GetVarMaxOffSendNum(int *pnValue)
{
	char szTmp[10] = {0};

	memcpy(szTmp,gstAppPosParam.szMaxOffSendNum,sizeof(gstAppPosParam.szMaxOffSendNum)-1);
	*pnValue=atoi(szTmp);
	return;
}


int SetFunctionPinpadTimeOut(void)
{
	int nLen;
	char szOldTimeOut[3+1] = {0};

	memcpy(szOldTimeOut, gstAppPosParam.szPinPadTimeOut, 3);
	
	if (YES == GetVarIsPinpad())
	{
		ASSERT_RETURNCODE(PubInputDlg("Pinpad param", "TIMEOUT:", gstAppPosParam.szPinPadTimeOut, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
		ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
		if (memcmp(szOldTimeOut, gstAppPosParam.szPinPadTimeOut, 3) != 0)
		{
			SetControlChkPinpad(YES);
		}
		return APP_SUCC;
	}
	return APP_SUCC;
}
/**
* @brief Set head print mode of receipt
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPrintTitleMode(void)
{
	char *SelMenu[] = {
		"0.LOGO",
		"1.STRING"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("SET PRINT", "RECEIPT HEAD", SelMenu, &gstAppPosParam.cPntTitleMode));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET PRINT", "RECEIPT HEAD", "0.LOGO||1.STRING", &gstAppPosParam.cPntTitleMode));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	if ( NO == gstAppPosParam.cPntTitleMode) /*<打印中文*/
	{
		return SetFunctionPntTitle();
	}
	return APP_SUCC;
}
/**
* @brief Get Title print mode
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
YESORNO GetIsPntTitleMode(void)
{
	if (gstAppPosParam.cPntTitleMode== YES || gstAppPosParam.cPntTitleMode== 1)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set receipt title 
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPntTitle(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szPntTitleEn);
	ASSERT_RETURNCODE(PubInputDlg("PRINT TITLE", NULL, szTmp, &nLen, 0, 40, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szPntTitleEn, szTmp);	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}
/**
* @brief  Get receipt title
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int GetFunctionPntTitle(char *pPntTitleCn)
{
	strcpy(pPntTitleCn,gstAppPosParam.szPntTitleEn);
	return APP_SUCC;
}

/**
* @brief Get is need to check pin when do void
* @return
* @li YES 
* @li NO void 
*/
YESORNO GetVarIsVoidPwd()
{
	if (gstAppPosParam.cIsVoidPwd==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set is need to check pin when do void
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsVoidPwd(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER CONTROL", "VERIFY VOID PWD", SelMenu, &gstAppPosParam.cIsVoidPwd));
	//ASSERT_RETURNCODE( ProSelectYesOrNo("OTHER CONTROL", "VERIFY VOID PWD", "0.NO|1.YES", &gstAppPosParam.cIsVoidPwd));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


YESORNO GetVarIsPrintPrintMinus(void)
{
	return gstAppPosParam.cIsPrintMinus;
}

int SetFunctionIsPrintPrintMinus(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("SET PRINT", "PRINT MINUS", SelMenu, &gstAppPosParam.cIsPrintMinus));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET PRINT", "PRINT MINUS", "0.NO|1.YES", &gstAppPosParam.cIsPrintMinus));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int SetFunctionReprintSettle()
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("REPRINT SETTLE", "ENABLE", SelMenu, &gstAppPosParam.cIsReprintSettle));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("REPRINT SETTLE", "ENABLE", "0.NO|1.YES", &gstAppPosParam.cIsReprintSettle));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

YESORNO GetVarReprintSettle(void)
{
	if (YES == gstAppPosParam.cIsReprintSettle)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

int GetPrnLogo(char *pszInOutStr, uint *pInOutXpos)
{
	memcpy(pszInOutStr, gstAppPosParam.szPrnLogoName, sizeof(gstAppPosParam.szPrnLogoName));
	*pInOutXpos = gstAppPosParam.unPrnLogoXpos;
	return APP_SUCC;
}

/**
* @brief Show terminal version
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int Version(void)
{
	int nRet;
	char szSoftVer[16]= {0};

	GetVarSoftVer(&szSoftVer);

	PubClearAll();
	PubDisplayTitle("Software Info");
	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 2, "%s", szSoftVer);
	PubUpdateWindow();
	nRet = PubGetKeyCode(0);
	if (nRet == KEY_BACK)
	{
		PubDisplayTitle("Build Time");
		PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 2, "%s", __DATE__);
		PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 3, "%s", __TIME__);
		PubUpdateWindow();
		nRet = PubGetKeyCode(0);
	}
	return APP_SUCC;
}


/**
* @brief Get emv error code msg from ini 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetEmvErrorCodeIni(int nEmvErrCode, char *pszErr)
{
	const char *szIniFile = FILE_EMVCODE_INI;
	int nIniHandle,nRet;
	int nLen;
	char szBuf[100];
	char szErrCode[12] = {0};

	if (NULL == pszErr || nEmvErrCode >= 0)
	{
		return APP_QUIT;
	}

	sprintf(szErrCode, "%d", nEmvErrCode);


	ASSERT_QUIT(PubOpenFile (szIniFile, "r", &nIniHandle));

	nRet = PrivateGetINIItemStr (nIniHandle, "BASE", szErrCode+1,  sizeof(szBuf) - 1, szBuf, &nLen);
	if (nRet == APP_SUCC)
	{
		sprintf(pszErr, "%s%s", szBuf, szErrCode);
	}
	else
	{
		sprintf(pszErr, "Unknown Error Code%s", szErrCode);
	}
	PubCloseFile(&nIniHandle);
	return nRet;
}


static int CheckIsValidTime(const char *pszTime)
{
	char szTmp[2+1];

	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime, 2);
	if(atoi(szTmp) >= 24)
	{
		return APP_FAIL;
	}
	
	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime+2, 2);
	if(atoi(szTmp) >= 60)
	{
		return APP_FAIL;
	}
	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszTime+4, 2);
	if(atoi(szTmp) >= 60)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Set date time
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionPosDateTime(void)
{
	int  nLen = 0;
	char szDateTime[14+1]={0};
	char szDate[8+1]={0};
	char szTime[6+1]={0};


	PubGetCurrentDatetime(szDateTime);
	memcpy(szDate, szDateTime, 8);
	memcpy(szTime, szDateTime+8, 6);
	ASSERT_RETURNCODE(PubInputDate("SYSTEM TIME", "DATE(YYYYMMDD):", szDate, INPUT_DATE_MODE_YYYYMMDD, 60));
	
	if(PubIsValidDate(szDate) != APP_SUCC)
	{
		PubMsgDlg("SYSTEM TIME", "INVALID INPUT", 3, 5);//"输入的日期格式错误"
		return APP_FAIL;
	}

	PubSetPosDateTime(szDate, "YYYYMMDD", szTime);
	
	ASSERT_RETURNCODE(PubInputDlg("SYSTEM TIME", "TIME(HHMMSS):", szTime, &nLen, 6, 6, 60, INPUT_MODE_NUMBER));

	if(CheckIsValidTime(szTime) != APP_SUCC)
	{
		PubMsgDlg("SYSTEM TIME", "INVALID INPUT", 3, 5);
		return APP_FAIL;
	}
	
	PubSetPosDateTime(szDate, "YYYYMMDD", szTime);
	return APP_SUCC;

}

/**
* @brief Get the switch of transaction
* @param in cTransType 交易类型
* @return 
* @li YES
* @li NO
* @author 
* @date 
*/
int SetTransSwitchOnoff(int nTransType, char cIsEnable)
{
	int nTransNum;

	nTransNum = nTransType;
	if((nTransType>=0) && (nTransType < sizeof(gstAppPosParam.sTransSwitch)*8))
	{
		if (cIsEnable == YES)
		{
			gstAppPosParam.sTransSwitch[nTransNum/8] |= (0x80>>(nTransNum%8));
		}
		else
		{
			gstAppPosParam.sTransSwitch[nTransNum/8] &= ~(0x80>>(nTransNum%8));
		}
	}
	else 
	{
		TRACE(("cTransType=%d cIsEnable=%d",nTransType,cIsEnable));
		return APP_FAIL;
	}
	
	//ASSERT(SwitchMenuByTransType(nTransType, cIsEnable));
	return APP_SUCC;
}

/**
* Save is download parameter
*/
int SetVarIsDownParmFlag(char cFlag)
{
	return PubSaveVar(&cFlag, fIsPosParamDown_off, fIsPosParamDown_len);
}

YESORNO GetVarIsDownParmFlag(void)
{
	char cStatus=NO;
	
	PubGetVar(&cStatus, fIsPosParamDown_off, fIsPosParamDown_len);

	if(YES == cStatus || 1 == cStatus)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief GetVarAdjustRate
* @param out psValue
* @return void
*/	
void GetVarAdjustRate(char *psValue)
{	
	memcpy(psValue,gstAppPosParam.szAdjustPrim,4);
	return;
}

/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
void GetVarMerchantAddr(char *pszValue, int nIndex)
{
	memcpy(pszValue,gstAppPosParam.szMerchantAddr[nIndex],sizeof(gstAppPosParam.szMerchantAddr[nIndex])-1);
	return;
}


/**
* @brief Get Issuer bank Name
* @param out pszValue 
* @return void
*/
void GetVarIssueBank(char *pszValue)
{
	memcpy(pszValue,gstAppPosParam.szIssuerName,sizeof(gstAppPosParam.szIssuerName)-1);
	return;
}


/**
* @brief Get currency type
* @param out pszValue 
* @return void
*/
void GetVarCurrencyName(char *pszValue)
{
	memcpy(pszValue,gstAppPosParam.szCurrencyName,sizeof(gstAppPosParam.szCurrencyName)-1);
	return;
}


/**
* @brief Set acquirer bank name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionAcqName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szAcqName);
	ASSERT_RETURNCODE(PubInputDlg("ACQUIRER NAME", NULL, szTmp, &nLen, 0, 20, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szAcqName, szTmp);	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;

}

/**
* @brief Set issue name
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIssuerName(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	strcpy(szTmp, gstAppPosParam.szIssuerName);
	ASSERT_RETURNCODE(PubInputDlg("ISSUER NAME", NULL, szTmp, &nLen, 0, 11, 0, INPUT_MODE_STRING));
	strcpy(gstAppPosParam.szIssuerName, szTmp);	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;

}
/**
* @brief Get switch of transaction, if support ,return YES
* @param in cTransType 
* @return
* @li YES
* @li NO
*/
YESORNO GetTransSwitchOnoff(char cTransType)
{
	int nTransNum;

	nTransNum = cTransType; 
	if (cTransType > TRANSSWITCHNUM)
	{
		return NO;
	}
	if (gstAppPosParam.sTransSwitch[nTransNum/8] & (0x80>>(nTransNum%8)))
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

/**
* @brief Set switch of transaction
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionTransSwitch(void)
{
	int i, nRet;
	int nTransNum=TRANSSWITCHNUM;		
	char cSelect;
	char lszName[TRANSSWITCHNUM][20]= {"","","PREAUTH","REFUND","INSTALLMENT","CASH ADVANCE", "OFFLINE","CASHBACK",
		"BALANCE","","","","","ORS"};//"VOID REFUND","VOID OFFLINE" ,"VOID INSTALLMENT", "VOID CASH ADVANCE"

	for(i = 0; i < nTransNum; i++)
	{
		if (lszName[i][0] == 0)
		{
			continue;
		}
		cSelect =  (gstAppPosParam.sTransSwitch[i/8]&(0x80>>(i%8))) ? 1 : 0;
		nRet = PubSelectYesOrNo("IS SUPPORT?", lszName[i], NULL, &cSelect);
		//nRet = ProSelectYesOrNoExt("IS SUPPORT?", lszName[i], "0.NO|1.YES", &cSelect);
		if (nRet == KEY_UP)
		{
			i = i - 2;
			if (i  < -1)
			{
				i = -1;
			}
			continue;
		}
		else if(nRet == KEY_DOWN)
		{
			continue;
		}
		else if (nRet == APP_FAIL || nRet == APP_QUIT)
		{
			break;
		}
		if (cSelect == '1')
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}

	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int InitializeParam()
{
	int nRet = 0;
	
	nRet = PubConfirmDlg("INITIALIZE", "All datas will be clear", 0, 0);
	if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
	{
		return nRet;
	}
	PubMsgDlg("INITIALIZE", "PROCESSING...", 0, 1);
	InitCommParam();
	InitAllBatch();			
	ClearAllSettle();						
	PubDeReclFile(SETTLEFILENAME);
	PubDelFile(KERNEL_NAME);
	PubDelFile(EMVFILE_TERMCAPK);	
	InitEMVParam();
	EmvClearWater();
	DownEmvParam();
	InitLastSettleFile();
	InitPosDefaultParam();
	PubMsgDlg("INITIALIZE", "Clear ok", 1, 1);
	
	CommInit();
	if (ExportEMVModule() != APP_SUCC)
	{
		return APP_FAIL;
	}

	return APP_SUCC;
}
int ChangePassword()
{
	int nSelect = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.SYSTEM PWD"),
		tr("2.USER PWD"),
		tr("3.FUNC PWD"),
		tr("4.SHORTC PWD")
	};
	ASSERT_QUIT(PubShowMenuItems(tr("CHANGE PASSWORD"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60));
	//ASSERT_QUIT(ProSelectList("1.SYSTEM PWD||2.USER PWD||3.FUNC PWD||4.SHORTC PWD", "CHANGE PASSWORD", 0xFF, &nSelect));
	switch(nSelcItem)
	{
	case 1:
		ChangePwd(EM_SYSTEM_PWD);
		break;
	case 2:
		ChangePwd(EM_TRANS_PWD);
		break;
	case 3:
		ChangePwd(EM_FUNC_PWD);
		break;
	case 4:
		ChangePwd(EM_SHOCT_PWD);
		break;
	default:
		break;
	}
	return APP_SUCC;
}
int ViewTranList()
{
	WaterOneByOne();
	return APP_SUCC;
}
int ViewSpecList()
{
	FindByInvoice(0);
	return APP_SUCC;
}
int ViewTotal()
{
	FindSettle();
	return APP_SUCC;
}
int ViewTVR_TSI()
{
	SetFunctionIsDispEMV_TVRTSI();
	return APP_SUCC;
}
int RePrnSettle()
{
	
	return APP_SUCC;
}
int PrnLastTrans()
{
	int nRecordNum = -1;
	STTAANSRECORD stTransRecord;

	GetRecordNum(&nRecordNum);
	if (nRecordNum <=0 )
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO RECORD");
		PubMsgDlg("ANY RECEIPT", szDispBuf, 0, 50);
		return APP_FAIL;
	}
	FetchLastRecord(&stTransRecord);
	PubClearAll();
	DISP_PRINTING_NOW;
	PrintWater(&stTransRecord, REPRINT);
	return APP_SUCC;
}

int RePrnSpecTrans()
{
	int nRecordNum = -1;
	
	GetRecordNum(&nRecordNum);
	if (nRecordNum <=0 )
	{
		char szDispBuf[100];
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szDispBuf, "|CNO RECORD");
		PubMsgDlg("PRINT SPECIAL", szDispBuf, 0, 50);
		return APP_FAIL;
	}
	FindByInvoice(1);
	return APP_SUCC;
}

int SendOffSale()
{
	SendOffline(0);
	return APP_SUCC;
}

int SetVarLastLoginDateTime(const char *psLastLoginDateTime)
{
	memcpy( gstBankParam.sLastLoginDateTime, psLastLoginDateTime, fLastLoginDateTime_len);
	PubSaveVar( (char *)&(gstBankParam.sLastLoginDateTime), fLastLoginDateTime_off, fLastLoginDateTime_len);
	return APP_SUCC;
}

int GetVarLastLoginDateTime(char *psLastLoginDateTime)
{
	memcpy(psLastLoginDateTime, gstBankParam.sLastLoginDateTime, fLastLoginDateTime_len);
	return APP_SUCC;
}

YESORNO GetVarIsSendAutoDown()
{
	return gstAppPosParam.cSendAutoDl;
}

int SetVatIsSendAutoDown(char cIsSend)
{
	gstAppPosParam.cSendAutoDl = cIsSend;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int SetFunctionIsSendAutoDown(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("TMS", "Is Send flag", SelMenu, &gstAppPosParam.cSendAutoDl));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("TMS", "Is Send flag", "0.NO|1.YES", &gstAppPosParam.cSendAutoDl));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

char GetVarDlWeek()
{
	return gstAppPosParam.cWeekDl;
}

char GetVarDlHour()
{
	return gstAppPosParam.cHourDl;
}

char GetVarDlMinute()
{
	return gstAppPosParam.cMinuteDl;
}
void GetVarDlDate(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szDownloadDate, 4);
}

int SetVarDlDate(const char *pszValue)
{
	memcpy(gstAppPosParam.szDownloadDate, pszValue, 4);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}
char GetVarDlMode()
{
	return gstAppPosParam.cDownloadMode;
}

int SetFunctionDownMode(void)
{
	int sSel = gstAppPosParam.cDownloadMode;
	int nLen = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("0.day"),
		tr("1.week")
	};
	nRet = PubShowMenuItems("Download Mode", pszItems, sizeof(pszItems)/sizeof(char *), &sSel, &nStartItem, 60);
	if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
	{
		return nRet;
	}
	//ASSERT_RETURNCODE(ProSelectList("0.day|1.week", "download mode", sSel, &sSel));
	gstAppPosParam.cDownloadMode = sSel;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	if (1 == gstAppPosParam.cDownloadMode)
	{
		SetFunctionDlWeek();
	}
	else 
	{
		ASSERT_RETURNCODE(PubInputDlg("TMS", "date", gstAppPosParam.szDownloadDate, &nLen, 0, 4, 0, INPUT_MODE_STRING));
	}
	SetFunctionDlHour();
	return APP_SUCC;
}


int SetFunctionDlWeek(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	sprintf(szTmp, "%d", gstAppPosParam.cWeekDl);
	ASSERT_RETURNCODE(PubInputDlg("TMS", "week", szTmp, &nLen, 0, 80, 0, INPUT_MODE_STRING));
	gstAppPosParam.cWeekDl = atoi(szTmp);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int SetFunctionDlHour(void)
{
	int nLen = 0;
	char szTmp[128] = {0};

	sprintf(szTmp, "%d", gstAppPosParam.cHourDl);
	ASSERT_RETURNCODE(PubInputDlg("TMS", "hour", szTmp, &nLen, 0, 2, 0, INPUT_MODE_STRING));
	gstAppPosParam.cHourDl = atoi(szTmp);
	sprintf(szTmp, "%d", gstAppPosParam.cMinuteDl);
	ASSERT_RETURNCODE(PubInputDlg("TMS", "Minute", szTmp, &nLen, 0, 2, 0, INPUT_MODE_STRING));
	gstAppPosParam.cMinuteDl = atoi(szTmp);
	
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

void GetParamHost(char *pszHost)
{
	memcpy(pszHost, gstAppPosParam.szHost, sizeof(gstAppPosParam.szHost));
}

//return YES or NO
YESORNO GetVarIsSwipe()
{
	return gstAppPosParam.cSupportSwipe + '0';
}

/**
* @brief 设置手输入磁条
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionIsSwipe(void)
{
	char cChoice = 0;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	cChoice = gstAppPosParam.cSupportSwipe + '0';

	ASSERT_RETURNCODE(PubSelectYesOrNo("PAYMENT TYPE", "SWIPE ENABLE", SelMenu, &cChoice));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("PAYMENT TYPE", "SWIPE ENABLE", "0.NO|1.YES", &cChoice));
	gstAppPosParam.cSupportSwipe = cChoice - '0';
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

YESORNO GetVarIsEncrypted()
{
	return gstAppPosParam.cIsEncrypt;
}

int SetFunctionOpenDukpt(void)
{
	char cChoice = 0;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	cChoice = gstAppPosParam.cIsOpenDukpt;

	ASSERT_RETURNCODE(PubSelectYesOrNo("TRANS CONTROL", "Enable DUKPT", SelMenu, &cChoice));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("TRANS CONTROL", "Enable DUKPT", "0.NO|1.YES", &cChoice));
	gstAppPosParam.cIsOpenDukpt = cChoice;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

YESORNO GetVarIsOpenDukpt()
{
	return gstAppPosParam.cIsOpenDukpt;
}


/**
* @brief Set Mainkey index
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionDukptKeyNo(void)
{
	int nLen, nNumber;
	char szNumber[2+1] = {0};

	gstAppPosParam.szDukptKeyNo[2] = 0;
	nNumber = atoi(gstAppPosParam.szDukptKeyNo);
	sprintf(szNumber, "%1.1u", nNumber%10);
	ASSERT_QUIT(PubInputDlg("SET Dukpt Key Index", "ENTER KEY INDEX(0-99):", szNumber, &nLen, 1, 2, 0, INPUT_MODE_NUMBER));
	sprintf(gstAppPosParam.szDukptKeyNo, "0%s", szNumber);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

void GetVarAcqId(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szAcquirerId, strlen(gstAppPosParam.szAcquirerId));
}

int SetFunctionAcqId(void)
{
	int nLen;

	ASSERT_QUIT(PubInputDlg("SET ACQID", "Acq Id:", gstAppPosParam.szAcquirerId, &nLen, 1, 3, 0, INPUT_MODE_NUMBER));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}
void GetVarVendorId(char *pszValue)
{
	memcpy(pszValue, gstAppPosParam.szVendorId, strlen(gstAppPosParam.szVendorId));
}

int SetFunctionVendorId(void)
{
	int nLen;

	ASSERT_QUIT(PubInputDlg("SET VENDOR ID", "Vendor Id:", gstAppPosParam.szVendorId, &nLen, 1, 8, 0, INPUT_MODE_NUMBER));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int GetVarDukptID()
{
	return gstAppPosParam.nDukptIndex;
}

int GetKSN(char* pszKSN)
{
PubDebug("Get ksn: %d", gstAppPosParam.nDukptIndex);
	if (NDK_OK != NDK_SecGetDukptKsn((uchar )gstAppPosParam.nDukptIndex, (uchar *)pszKSN))
	{
		PubMsgDlg("Warn", "Get KSN failed", 3, 3);
		return APP_QUIT;
	}
	return APP_SUCC;
}
/**
* @brief  设置票据号
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionInvoiceNo(void)
{
	int nLen;
	char szInvoiceNo[fInvoiceNo_len+1] = {0};
	int nWaterSum = 0;
	
	memcpy(szInvoiceNo, gstBankParam.szInvoiceNo, sizeof(gstBankParam.szInvoiceNo) - 1);
	ASSERT_RETURNCODE(PubInputDlg("SET MAYBANK", "Invoice NO.", szInvoiceNo, &nLen, fInvoiceNo_len, fInvoiceNo_len, 0, INPUT_MODE_NUMBER));
	if (memcmp(gstBankParam.szInvoiceNo, szInvoiceNo, 6) !=0)
	{
		GetRecordNum(&nWaterSum);
		if (nWaterSum > 0)
		{
			PubMsgDlg("SET MAYBANK", "Pls settle", 3, 10);
			return APP_FAIL;
		}
		memcpy(gstBankParam.szInvoiceNo, szInvoiceNo, fInvoiceNo_len);
		PubSaveVar(gstBankParam.szInvoiceNo,fInvoiceNo_off,fInvoiceNo_len);
	}

	PubSaveVar(gstBankParam.szInvoiceNo,fInvoiceNo_off,fInvoiceNo_len);
	return APP_SUCC;
}


/**
* @brief Set Max refund amount
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMaxEppAmount(const char *pszTitle, int nIndex)
{
	int nLen = 12;
	char szTitle[32] = "EPP";

	if (NULL != pszTitle)
	{
		strcpy(szTitle, pszTitle);
	}
	ASSERT_RETURNCODE(PubInputAmount(szTitle, "Maximun EPP Amount:", gstAppPosParam.szMaxEPPAmt[nIndex], &nLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set Max refund amount
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMinEppAmount(const char *pszTitle, int nIndex)
{
	int nLen = 12;
	char szTitle[32] = "EPP";

	if (NULL != pszTitle)
	{
		strcpy(szTitle, pszTitle);
	}
	ASSERT_RETURNCODE(PubInputAmount(szTitle, "Minimun EPP Amount:", gstAppPosParam.szMinEPPAmt[nIndex], &nLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

int CheckEppAmt(char *pszTitle, char *pszAmt, int nMonth)
{
	char szContent[128] = {0};
	char szDispMinAmt[14] = {0}, szDispMaxAmt[14] = {0};
	int nIndex = 0;

	switch (nMonth)
	{
//	case 3:
//		nIndex = EM_MENU_EPP_MONTH3;
//		break;
	case 6:
		nIndex = EM_MENU_EPP_MONTH6;
		break;
	case 12:
		nIndex = EM_MENU_EPP_MONTH12;
		break;
	case 18:
		nIndex = EM_MENU_EPP_MONTH18;
		break;
	case 24:
		nIndex = EM_MENU_EPP_MONTH24;
		break;
	default:
		nIndex = EM_MENU_EPP_MONTH6;
		break;
	}

	ProAmtToDispOrPnt(gstAppPosParam.szMinEPPAmt[nIndex], szDispMinAmt);
	PubAllTrim(szDispMinAmt);
	ProAmtToDispOrPnt(gstAppPosParam.szMaxEPPAmt[nIndex], szDispMaxAmt);
	PubAllTrim(szDispMaxAmt);

	sprintf(szContent, "Amount Should Be\n %s ~ %s", szDispMinAmt, szDispMaxAmt);
	
	if (memcmp(pszAmt, gstAppPosParam.szMinEPPAmt[nIndex], 12) < 0
		|| memcmp(pszAmt, gstAppPosParam.szMaxEPPAmt[nIndex], 12) > 0)
	{
		PubMsgDlg(pszTitle, szContent, 3, 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}




/**
* @brief Set Max refund amount
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionMinCashbackAmount(void)
{
	int nLen = 12;

	ASSERT_RETURNCODE(PubInputAmount("CASHBACK", "Minimun Cashback Amount:", gstAppPosParam.szMinCashbackAmt, &nLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}


int CheckCashbackAmt(char *pszTitle, char *pszAmt)
{
	char szContent[128] = {0};
	char szDispMinAmt[14] = {0};

	ProAmtToDispOrPnt(gstAppPosParam.szMinCashbackAmt, szDispMinAmt);
	PubAllTrim(szDispMinAmt);

	sprintf(szContent, "Amount Should Be \n >= %s", szDispMinAmt);
	
	if (memcmp(pszAmt, gstAppPosParam.szMinCashbackAmt, 12) < 0)
	{
		PubMsgDlg(pszTitle, szContent, 3, 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}


YESORNO GetVarIsMccsPriority()
{
	return gstAppPosParam.cMccsPriority;
}

int SetFunctionIsMccsPriority(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("MAYBANK", "Mydebit Priority", SelMenu, &gstAppPosParam.cMccsPriority));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("MAYBANK", "Mydebit Priority", "0.NO|1.YES", &gstAppPosParam.cMccsPriority));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

/**
* @brief Set Max refund amount
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionWaveNoPinAmount(void)
{
	int nLen = 12;

	ASSERT_RETURNCODE(PubInputAmount("MAYBANK", "Maximum No PIN Amount:", gstAppPosParam.szWaveNoPinAmt, &nLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

YESORNO IsBelowAndEqualNoPinAmt(char *pszAmt)
{	
	if (memcmp(pszAmt, gstAppPosParam.szWaveNoPinAmt, 12) >= 0)
	{
		return NO;
	}
	return YES;
}

/**
* @brief Set switch of transaction
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
*/
int SetFunctionEppSwitch(void)
{
	int i, nRet;
	int nTransNum=EPPMONTHSNUM;		
	char cSelect;
	char lszName[EPPMONTHSNUM][20]= {"6 months","12 months","18 months","24 months"};//"3 months",
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	for(i = 0; i < nTransNum; i++)
	{
		if (lszName[i][0] == 0)
		{
			continue;
		}
		cSelect =  (gstAppPosParam.sEppMonthSwitch[i/8]&(0x80>>(i%8))) ? 1 : 0;
		nRet = PubSelectYesOrNo("IS SUPPORT?", lszName[i], SelMenu, &cSelect);
		//nRet = ProSelectYesOrNoExt("IS SUPPORT?", lszName[i], "0.NO|1.YES", &cSelect);
		if (nRet == KEY_UP)
		{
			i = i - 2;
			if (i  < -1)
			{
				i = -1;
			}
			continue;
		}
		else if(nRet == KEY_DOWN)
		{
			continue;
		}
		else if (nRet == APP_FAIL || nRet == APP_QUIT)
		{
			break;
		}
		if (cSelect == '1')
		{
			gstAppPosParam.sEppMonthSwitch[i/8] |= 0x01<<(7-i%8);
			SetFunctionMinEppAmount(lszName[i], i);
			SetFunctionMaxEppAmount(lszName[i], i);
		}
		else
		{
			gstAppPosParam.sEppMonthSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}

	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Get switch of transaction, if support ,return YES
* @param in cTransType 
* @return
* @li YES
* @li NO
*/
YESORNO GetEppMonthSwitchOnoff(char cEppMonth)
{
	if (cEppMonth >= EPPMONTHSNUM)
	{
		return NO;
	}
	
	if (gstAppPosParam.sEppMonthSwitch[cEppMonth/8] & (0x80>>(cEppMonth%8)))
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

YESORNO GetVarIsPinByPass()
{
	return gstAppPosParam.cIsPinByPass;
}

int SetFunctionIsPinByPass(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("MAYBANK", "Enable PIN By Pass", SelMenu, &gstAppPosParam.cIsPinByPass));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("MAYBANK", "Enable PIN By Pass", "0.NO|1.YES", &gstAppPosParam.cIsPinByPass));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}


/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
void GetVarHeaderText(char *pszValue, int nIndex)
{
	memcpy(pszValue,gstAppPosParam.szHeaderText[nIndex],sizeof(gstAppPosParam.szHeaderText[nIndex])-1);
	return;
}


/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
int SetVarHeaderText(char *pszValue, int nIndex)
{
	memcpy(gstAppPosParam.szHeaderText[nIndex],pszValue,sizeof(gstAppPosParam.szHeaderText[nIndex])-1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
void GetVarFooterText(char *pszValue, int nIndex)
{
	memcpy(pszValue,gstAppPosParam.szFooterText[nIndex],sizeof(gstAppPosParam.szFooterText[nIndex])-1);
	return;
}


/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
int SetVarFooterText(char *pszValue, int nIndex)
{
	memcpy(gstAppPosParam.szFooterText[nIndex],pszValue,sizeof(gstAppPosParam.szFooterText[nIndex])-1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
int SetMaxRdmPoint(char *pszValue)
{
	memcpy(gstAppPosParam.szMaxRdmPoint,pszValue,sizeof(gstAppPosParam.szMaxRdmPoint)-1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief Get merchant address
* @param out pszValue 
* @return void
*/
void GetMaxRdmPoint(char *pszValue)
{
	memcpy(pszValue,gstAppPosParam.szMaxRdmPoint,sizeof(gstAppPosParam.szMaxRdmPoint)-1);
	return;
}

int SetVarIsEncrypted(YESORNO cIsEncrypt)
{
	gstAppPosParam.cIsEncrypt = cIsEncrypt;
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

int SetFunctionIsDccEnbale(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("MAYBANK", "Enable DCC", SelMenu, &gstAppPosParam.cIsDccEnable));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("MAYBANK", "Enable DCC", "0.NO|1.YES", &gstAppPosParam.cIsDccEnable));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

YESORNO GetVarIsDCC()
{
	return gstAppPosParam.cIsDccEnable;
}


int SetVarDBTVersion(char *pszVersion)
{
	memcpy(gstAppPosParam.szDBTVersion, pszVersion, 6);
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	
	return APP_SUCC;;
}

void GetVerDBTVersion(char *pszVersion)
{
	memcpy(pszVersion, gstAppPosParam.szDBTVersion, 6);
}

int SetFunctionIsInputCvv2(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_QUIT(PubSelectYesOrNo("MAYBANK", "Enable Cvv2", SelMenu, &gstAppPosParam.cIsInputCvv2));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("MAYBANK", "Enable Cvv2", "0.NO|1.YES", &gstAppPosParam.cIsInputCvv2));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));

	return APP_SUCC;
}

YESORNO GetVarIsInputCvv2()
{
	return gstAppPosParam.cIsInputCvv2;
}

/**
* @brief 设置打印报文开关
* @param 无
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author 
* @date 
*/	
int SetFunctionIsPrintIso(void)
{	
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_QUIT(PubSelectYesOrNo("SET PRINTF", "IS PRINT ISO", SelMenu, &gstAppPosParam.cIsPrintIso));
	//ASSERT_QUIT(ProSelectYesOrNo("SET PRINTF", "IS PRINT ISO", "0.NO|1.YES", &gstAppPosParam.cIsPrintIso));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief获取打印报文开关
* @param 无
* @return
* @li YES
* @li NO
* @author 
* @date 
*/	
int GetVarIsPrintIso(void)
{
	return gstAppPosParam.cIsPrintIso;
}

int GetBatchIndex(int nAcqIndex, int nEppMonth, int *pnBatchIndex)
{
	if (nAcqIndex >= MAXBATCH_NUM)
	{
		return APP_FAIL;
	}
	
	if (nAcqIndex >= 0 && nAcqIndex < 4)//非EPP
	{
		*pnBatchIndex = nAcqIndex;
		return APP_SUCC;
	}

	switch (nAcqIndex)
	{
	case EM_ACQINDEX_EPP_V_M:
		switch (nEppMonth)
		{
//		case EM_EPP_MONTH3:
//			*pnBatchIndex = 4;
			break;
		case EM_EPP_MONTH6:
			*pnBatchIndex = 4;
			break;
		case EM_EPP_MONTH12:
			*pnBatchIndex = 5;
			break;
		case EM_EPP_MONTH18:
			*pnBatchIndex = 6;
			break;
		case EM_EPP_MONTH24:
			*pnBatchIndex = 7;
			break;
		default:
			return APP_FAIL;
		}
		break;
	case EM_ACQINDEX_EPP_AMEX:
		switch (nEppMonth)
		{
//		case EM_EPP_MONTH3:
//			*pnBatchIndex = 8;
			break;
		case EM_EPP_MONTH6:
			*pnBatchIndex = 8;
			break;
		case EM_EPP_MONTH12:
			*pnBatchIndex = 9;
			break;
		case EM_EPP_MONTH18:
			*pnBatchIndex = 10;
			break;
		case EM_EPP_MONTH24:
			*pnBatchIndex = 11;
			break;
		default:
			return APP_FAIL;
		}
		break;
	default:
		return APP_FAIL;
	}
	return APP_SUCC;
}

int UpdateAcqIndexFromEpp(int *pnAcqIndex, int nEppMonth)
{	
	int nAcqIndex = *pnAcqIndex;
		
	if (nAcqIndex >= 0 && nAcqIndex < 4)//非EPP
	{
		*pnAcqIndex = nAcqIndex;
		return APP_SUCC;
	}

	switch (nAcqIndex)
	{
	case EM_ACQINDEX_EPP_V_M:
		switch (nEppMonth)
		{
//		case EM_EPP_MONTH3:
//			*pnBatchIndex = 4;
			break;
		case EM_EPP_MONTH6:
			*pnAcqIndex = 4;
			break;
		case EM_EPP_MONTH12:
			*pnAcqIndex = 5;
			break;
		case EM_EPP_MONTH18:
			*pnAcqIndex = 6;
			break;
		case EM_EPP_MONTH24:
			*pnAcqIndex = 7;
			break;
		default:
			return APP_FAIL;
		}
		break;
	case EM_ACQINDEX_EPP_AMEX:
		switch (nEppMonth)
		{
//		case EM_EPP_MONTH3:
//			*pnBatchIndex = 8;
			break;
		case EM_EPP_MONTH6:
			*pnAcqIndex = 8;
			break;
		case EM_EPP_MONTH12:
			*pnAcqIndex = 9;
			break;
		case EM_EPP_MONTH18:
			*pnAcqIndex = 10;
			break;
		case EM_EPP_MONTH24:
			*pnAcqIndex = 11;
			break;
		default:
			return APP_FAIL;
		}
		break;
	default:
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief 设置打印报文开关
* @param 无
* @return
* @li APP_SUCC
* @li APP_QUIT
* @li APP_FAIL
* @author 
* @date 
*/	
int SetFunctionIsCashRegister(void)
{	
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_QUIT(PubSelectYesOrNo("OTHER CONTROL", "Enable Cash Register", SelMenu, &gstAppPosParam.cCashRegister));
	//ASSERT_QUIT(ProSelectYesOrNo("OTHER CONTROL", "Enable Cash Register", "0.NO|1.YES", &gstAppPosParam.cCashRegister));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

/**
* @brief获取打印报文开关
* @param 无
* @return
* @li YES
* @li NO
* @author 
* @date 
*/	
int GetVarIsCashRegister(void)
{
	return gstAppPosParam.cCashRegister;
}

int SetFunctionIsSettlePwd(void)
{
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("OTHER CONTROL", "VERIFY SETTLE PWD", SelMenu, &gstAppPosParam.cIsSettlePwd));
	//ASSERT_RETURNCODE( ProSelectYesOrNo("OTHER CONTROL", "VERIFY SETTLE PWD", "0.NO|1.YES", &gstAppPosParam.cIsSettlePwd));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

YESORNO GetVarIsSettlePwd()
{
	if (gstAppPosParam.cIsSettlePwd==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

void GetVarInquiryNo(char *pszNo)
{
	memcpy(pszNo, gstAppPosParam.szInquiryNo, strlen(gstAppPosParam.szInquiryNo));
}
void SetVarInquiryNo(char *pszNo)
{
	memcpy(gstAppPosParam.szInquiryNo, pszNo, strlen(pszNo));
}
int SetFunctionInquiryNo(void)
{
	int nLen = 0;

	ASSERT_RETURNCODE(PubInputDlg("OTHER", "Inquiry No", gstAppPosParam.szInquiryNo, &nLen, 0, 24, 0, INPUT_MODE_STRING));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}

void GetVarCenterNo(char *pszNo)
{
	memcpy(pszNo, gstAppPosParam.szCenterNo, strlen(gstAppPosParam.szCenterNo));
}
void SetVarCenterNo(char *pszNo)
{
	memcpy(gstAppPosParam.szCenterNo, pszNo, strlen(pszNo));
}

int SetFunctionCenterNo(void)
{
	int nLen = 0;

	ASSERT_RETURNCODE(PubInputDlg("OTHER", "Centre No", gstAppPosParam.szCenterNo, &nLen, 0, 24, 0, INPUT_MODE_STRING));
	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	return APP_SUCC;
}


int GetParamFormTMSTag(int nFd, const char *pszTag, const int nMaxLen, char *pszValue, int *nLen)
{
	char szParamValue[512+1] = {0};
	int nPosition = 32;
	
	if(PubFindParam(nFd, nPosition, pszTag, szParamValue, &nLen) != APP_SUCC)
	{
		return APP_FAIL;
	}
	if (nMaxLen == 1)
	{
		*pszValue = atoi(szParamValue);
	}
	else
	{
		memset(pszValue, 0, nMaxLen);
		memcpy(pszValue, szParamValue, nLen);
	}
	return APP_SUCC;
}

int ParseParamFromTMS(void)
{	
	int nFd = 0;
	char szParamValue[200+1] = {0};
	int nParamLen;
	char szPath[100] = {0};
	char szFileName[100] = {0};
	char *pszFileName;
	int i = 0;
	char szTag[6+1] = {0};
	char cValue = 0;
	//int nCommValue = 0;

	STAPPCOMMPARAM stAppCommParam;
	GetAppCommParam(&stAppCommParam);

	//get current dirent
	memset(szPath, 0, sizeof(szPath));
	getcwd(szPath, sizeof(szPath) - 1);

	//get name of parameter file which recieved from tms.
	pszFileName = strrchr(szPath, '/');
	sprintf(szFileName, "%s.TMS", pszFileName + 1);
	
	ASSERT_FAIL(PubOpenFile(szFileName, "r", &nFd));

	PubClearAll();
	PubDisplayGen(3, "TMS Configuring...");
	PubUpdateWindow();

	for (i = 0; i < ACQUIRER_NUM; i++)
	{
		sprintf(szTag, "00000%02d1", i);
		GetParamFormTMSTag(nFd, szTag, sizeof(gstAppPosParam.szMerchantId[i]), gstAppPosParam.szMerchantId[i], &nParamLen);
		sprintf(szTag, "00000%02d2", i);
		GetParamFormTMSTag(nFd, szTag, sizeof(gstAppPosParam.szTerminalId[i]), gstAppPosParam.szTerminalId[i], &nParamLen);

		sprintf(szTag, "00000%02d3", i);
		GetParamFormTMSTag(nFd, szTag, sizeof(stAppCommParam.szTransTpdu[i]), stAppCommParam.szTransTpdu[i], &nParamLen);
		sprintf(szTag, "00000%02d4", i);
		GetParamFormTMSTag(nFd, szTag, sizeof(stAppCommParam.szTransNii[i]), stAppCommParam.szTransNii[i], &nParamLen);

	}

	for (i = 0; i < 4; i++)
	{
		sprintf(szTag, "000020%d", i);
		if (APP_SUCC == GetParamFormTMSTag(nFd, szTag, 1, &cValue, &nParamLen))
		{
			if (cValue)
			{
				gstAppPosParam.sEppMonthSwitch[i/8] |= 0x01<<(7-i%8);
			}
			else
			{
				gstAppPosParam.sEppMonthSwitch[i/8] &= ~(0x01<<(7-i%8));
			}	
		}

	}
	GetParamFormTMSTag(nFd, "00000204", sizeof(gstAppPosParam.szMinEPPAmt[0]), gstAppPosParam.szMinEPPAmt[0], &nParamLen);
	GetParamFormTMSTag(nFd, "00000205", sizeof(gstAppPosParam.szMaxEPPAmt[0]), gstAppPosParam.szMaxEPPAmt[0], &nParamLen);
	GetParamFormTMSTag(nFd, "00000206", sizeof(gstAppPosParam.szMinEPPAmt[1]), gstAppPosParam.szMinEPPAmt[1], &nParamLen);
	GetParamFormTMSTag(nFd, "00000207", sizeof(gstAppPosParam.szMaxEPPAmt[1]), gstAppPosParam.szMaxEPPAmt[1], &nParamLen);
	GetParamFormTMSTag(nFd, "00000208", sizeof(gstAppPosParam.szMinEPPAmt[2]), gstAppPosParam.szMinEPPAmt[2], &nParamLen);
	GetParamFormTMSTag(nFd, "00000209", sizeof(gstAppPosParam.szMaxEPPAmt[2]), gstAppPosParam.szMaxEPPAmt[2], &nParamLen);
	GetParamFormTMSTag(nFd, "00000210", sizeof(gstAppPosParam.szMinEPPAmt[3]), gstAppPosParam.szMinEPPAmt[3], &nParamLen);
	GetParamFormTMSTag(nFd, "00000211", sizeof(gstAppPosParam.szMaxEPPAmt[3]), gstAppPosParam.szMaxEPPAmt[3], &nParamLen);

	
	GetParamFormTMSTag(nFd, "00000212", sizeof(gstAppPosParam.szMerchantNameEn), gstAppPosParam.szMerchantNameEn, &nParamLen);

	GetParamFormTMSTag(nFd, "00000213", sizeof(gstAppPosParam.szMerchantAddr[0]), gstAppPosParam.szMerchantAddr[0], &nParamLen);
	GetParamFormTMSTag(nFd, "00000214", sizeof(gstAppPosParam.szMerchantAddr[1]), gstAppPosParam.szMerchantAddr[1], &nParamLen);
	GetParamFormTMSTag(nFd, "00000215", sizeof(gstAppPosParam.szMerchantAddr[2]), gstAppPosParam.szMerchantAddr[2], &nParamLen);
	GetParamFormTMSTag(nFd, "00000216", sizeof(gstAppPosParam.szMerchantAddr[3]), gstAppPosParam.szMerchantAddr[3], &nParamLen);

	GetParamFormTMSTag(nFd, "00000217", sizeof(gstBankParam.szTraceNo), gstBankParam.szTraceNo, &nParamLen);
//	GetParamFormTMSTag(nFd, "00000218", sizeof(gstBankParam.szTraceNo), gstBankParam.szTraceNo, &nParamLen);

	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000219", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cPrintPageCount = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000220", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cMccsPriority = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000221", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsTipFlag = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000222", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsCardInput = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000223", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cCashRegister = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000224", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsVoidPwd = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000225", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsSettlePwd = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000226", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsPinByPass = cValue + '0';
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000227", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsInputCvv2 = cValue + '0';
	}
	GetParamFormTMSTag(nFd, "00000228", sizeof(gstAppPosParam.szVendorId), gstAppPosParam.szVendorId, &nParamLen);
	GetParamFormTMSTag(nFd, "00000229", sizeof(gstAppPosParam.szUsrPwd), gstAppPosParam.szUsrPwd, &nParamLen);
	GetParamFormTMSTag(nFd, "00000230", sizeof(gstAppPosParam.szAdminPwd), gstAppPosParam.szAdminPwd, &nParamLen);
	GetParamFormTMSTag(nFd, "00000231", sizeof(gstAppPosParam.szFuncPwd), gstAppPosParam.szFuncPwd, &nParamLen);
	GetParamFormTMSTag(nFd, "00000232", sizeof(gstAppPosParam.szShortCutPwd), gstAppPosParam.szShortCutPwd, &nParamLen);

	GetParamFormTMSTag(nFd, "00000233", sizeof(gstAppPosParam.szPaypassCvmLimit), gstAppPosParam.szPaypassCvmLimit, &nParamLen);
	GetParamFormTMSTag(nFd, "00000234", sizeof(gstAppPosParam.szPaypassFloorLimit), gstAppPosParam.szPaypassFloorLimit, &nParamLen);
	GetParamFormTMSTag(nFd, "00000235", sizeof(gstAppPosParam.szPaywaveCvmLimit), gstAppPosParam.szPaywaveCvmLimit, &nParamLen);
	GetParamFormTMSTag(nFd, "00000236", sizeof(gstAppPosParam.szPaywaveFloorLimit), gstAppPosParam.szPaywaveFloorLimit, &nParamLen);

	GetParamFormTMSTag(nFd, "00000237", sizeof(gstAppPosParam.szInquiryNo), gstAppPosParam.szInquiryNo, &nParamLen);
	GetParamFormTMSTag(nFd, "00000238", sizeof(gstAppPosParam.szCenterNo), gstAppPosParam.szCenterNo, &nParamLen);


	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000239", 1, &cValue, &nParamLen))
	{
		i = TRANS_PREAUTH;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000240", 1, &cValue, &nParamLen))
	{
		i = TRANS_REFUND;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000241", 1, &cValue, &nParamLen))
	{
		i = TRANS_INSTALLMENT;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000242", 1, &cValue, &nParamLen))
	{
		i = TRANS_CASH_ADVANCE;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000243", 1, &cValue, &nParamLen))
	{
		i = TRANS_OFFLINE;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000244", 1, &cValue, &nParamLen))
	{
		i = TRANS_CASHBACK;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000245", 1, &cValue, &nParamLen))
	{
		i = TRANS_BALANCE;
		if (cValue)
		{
			gstAppPosParam.sTransSwitch[i/8] |= 0x01<<(7-i%8);
		}
		else
		{
			gstAppPosParam.sTransSwitch[i/8] &= ~(0x01<<(7-i%8));
		}
	}
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000246", 1, &cValue, &nParamLen))
	{
		gstAppPosParam.cIsPinPad = cValue + '0';
	}
	GetParamFormTMSTag(nFd, "00000247", sizeof(stAppCommParam.szSecurityNii), stAppCommParam.szSecurityNii, &nParamLen);
	GetParamFormTMSTag(nFd, "00000248", sizeof(szParamValue), szParamValue, &nParamLen);
	PubAscToHex((uchar *)szParamValue, 10, 0, (uchar *)stAppCommParam.sSecurityTpdu);
	
	GetParamFormTMSTag(nFd, "00000249", 1, &stAppCommParam.cTimeOut, &nParamLen);

	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000250", 1, &stAppCommParam.cCommType, &nParamLen))
	{
		switch(stAppCommParam.cCommType)
		{
		case 0:
			stAppCommParam.cCommType = COMM_DIAL;
			break;
		case 1:
			stAppCommParam.cCommType =COMM_ETH;
			break;
		case 2://CDMA
			stAppCommParam.cCommType =COMM_CDMA;
			break;
		case 3://GPRS
			stAppCommParam.cCommType =COMM_GPRS;
			break;
		case 4:
			stAppCommParam.cCommType =COMM_RS232;
			break;
		case 5:
			stAppCommParam.cCommType= COMM_WIFI;
			break;
		default:
			stAppCommParam.cCommType =COMM_DIAL;
			break;
		}
	}
	GetParamFormTMSTag(nFd, "00000251", 1, &stAppCommParam.cIsDHCP, &nParamLen);
	GetParamFormTMSTag(nFd, "00000252", sizeof(stAppCommParam.szIp1), stAppCommParam.szIp1, &nParamLen);
	GetParamFormTMSTag(nFd, "00000253", sizeof(stAppCommParam.szPort1), stAppCommParam.szPort1, &nParamLen);
	GetParamFormTMSTag(nFd, "00000254", sizeof(stAppCommParam.szIp2), stAppCommParam.szIp2, &nParamLen);
	GetParamFormTMSTag(nFd, "00000255", sizeof(stAppCommParam.szPort2), stAppCommParam.szPort2, &nParamLen);
	GetParamFormTMSTag(nFd, "00000256", sizeof(stAppCommParam.szIpAddr), stAppCommParam.szIpAddr, &nParamLen);
	GetParamFormTMSTag(nFd, "00000257", sizeof(stAppCommParam.szMask), stAppCommParam.szMask, &nParamLen);
	GetParamFormTMSTag(nFd, "00000258", sizeof(stAppCommParam.szGate), stAppCommParam.szGate, &nParamLen);

	GetParamFormTMSTag(nFd, "00000259", 1, &stAppCommParam.cReDialNum, &nParamLen);
	GetParamFormTMSTag(nFd, "00000260", 1, &stAppCommParam.szPreDial, &nParamLen);
	GetParamFormTMSTag(nFd, "00000261", sizeof(stAppCommParam.szTelNum1), stAppCommParam.szTelNum1, &nParamLen);
	GetParamFormTMSTag(nFd, "00000262", sizeof(stAppCommParam.szTelNum2), stAppCommParam.szTelNum2, &nParamLen);
	GetParamFormTMSTag(nFd, "00000263", sizeof(stAppCommParam.szTelNum3), stAppCommParam.szTelNum3, &nParamLen);
	
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000264", sizeof(stAppCommParam.szAPN1), stAppCommParam.szAPN1, &nParamLen))
	{
		strcpy(stAppCommParam.szAPN2, stAppCommParam.szAPN1);
	}
	GetParamFormTMSTag(nFd, "00000265", sizeof(stAppCommParam.szUser), stAppCommParam.szUser, &nParamLen);
	GetParamFormTMSTag(nFd, "00000266", sizeof(stAppCommParam.szPassWd), stAppCommParam.szPassWd, &nParamLen);

	GetParamFormTMSTag(nFd, "00000267", sizeof(stAppCommParam.szWifiSsid), stAppCommParam.szWifiSsid, &nParamLen);
	GetParamFormTMSTag(nFd, "00000268", sizeof(stAppCommParam.szWifiKey), stAppCommParam.szWifiKey, &nParamLen);
	if (APP_SUCC == GetParamFormTMSTag(nFd, "00000269", 1, stAppCommParam.cWifiMode, &nParamLen))
	{
		switch(stAppCommParam.cWifiMode)
		{
		case 1:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_OPEN;
			break;
		case 2:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_SHARED;
			break;
		case 3:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
			break;
		case 4:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA2;
			break;
		case 5:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
			break;
		default:
			stAppCommParam.cWifiMode = WIFI_NET_SEC_WPA2;
			break;;
		}
	}

	ASSERT_FAIL(PubUpdateRec(FILE_APPPOSPARAM,1,(char *)&gstAppPosParam));
	ASSERT_FAIL(SetAppCommParam(&stAppCommParam));
	SetVarTraceNo(gstBankParam.szTraceNo);
	
	PubDelFile(szFileName);
	return APP_SUCC;
}

