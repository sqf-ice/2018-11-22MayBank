/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  emv_interface.c
** File indentifier: EMV interface
** Synopsis:  
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

static int FcnGetAndIncTsc(void);
static int FcnIISReferral(uchar *, int);
static int EmvGetBcdAmt(uchar, uchar *, uchar *);
static int GetPinEntry(int, char *);
static int DealPin(const char *, char *, const char *);
static int FcnSelectEcOrEmv(void);
static int AccTypeSel(void);
static int CertConfirm(uchar , uchar *, int);
static int LcdMsg(char * , uchar *, int,  int, int);
static int CandidateSel(candidate *, int, int);
static int Asc2Bcd(uchar *, int, uchar *, int);
static uint C42Unit(uchar *);
static void Unit2C4(uint, uchar *);
static int ExportEntryPointModule(void);
static int _send_msg(entry_point_opt );
static int GetPinEntryPaywave(int, char *);

static char gs9F26_RQ[16+1];
static int CandidateSel_EP(ep_candidate * pstCan, int nAmt, int nTimes);


/**
* @brief Export EMV module
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int ExportEMVModule(void)
{
	char sCapk[2][6];
	char sAid[20][17];
	int nRet;
	char cVal = 0;
	emv_file stEMVFile = 
	{
		"",
		KERNEL_NAME,
		EMVFILE_TERMCAPK,
		EMVFILE_TERMBLKCARD,
		EMVFILE_TERMBLKCERT,
		EMVFILE_TERMLOG
	};
	emv_oper stEMVOper = 
	{	
		NULL,
		GetPinEntry,
		FcnIISReferral,
		AccTypeSel,
		FcnGetAndIncTsc,
		CertConfirm,
		LcdMsg,
		CandidateSel,
		Asc2Bcd,
		Bcd2Asc,
		C42Unit,
		Unit2C4,
		Bcd2Unit64,
		IccRfRw,
		IccRfPowerDown,
		FcnSelectEcOrEmv,
		IccRfPowerUp,
		EmvGetBcdAmt
	};

	if ((nRet = EMVL2_Initialize(&stEMVFile, &stEMVOper)) != APP_SUCC)
	{
		TRACE(("EMV_Initialize() error, nRet = %d, EMV_ErrorCode = %d", nRet, EMV_ErrorCode()));
	}

	/**<Get Aid list num*/
	if (EMV_EnumAID(sAid) <=0)
	{
		SetVarEmvIsDownAIDFlag(YES);
	}

	/**<Get CAPK number*/
	if (EMV_EnumCAPK(0, 1, sCapk) <= 0)
	{
		SetVarEmvIsDownCAPKFlag(YES);
	}

	nRet = ExportEntryPointModule();
	if (nRet != APP_SUCC)
	{
		TRACE(("ExportEntryPointModule() error, nRet = %d", nRet));
	}
	
	//20171227 彻底禁用电子现金脱机
	//cVal = 1;
	//EMVL2_WriteNLTagData(0x0013, (uchar *)&cVal, 1);
	return APP_SUCC;
}

/**
* @brief Export entrypoint Modele
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int ExportEntryPointModule(void)
{
	STEPOPER stEPOper ;
	emv_file stEmvFile;
	int nRet = 0;

	/*Load the file storage path*/
	strcpy(stEmvFile._file_path, CONFIG_PATH); 
	strcpy(stEmvFile._capk_name, EMVFILE_TERMCAPK);
	strcpy(stEmvFile._card_blk, EMVFILE_TERMBLKCARD);
	strcpy(stEmvFile._cert_blk, EMVFILE_TERMBLKCERT);
	strcpy(stEmvFile._emv_log, EMVFILE_TERMLOG);
	sprintf(stEmvFile._conf_name, KERNEL_NAME );//

	stEPOper.emv_get_transamt = NULL;
	stEPOper.emv_get_pinentry = GetPinEntryPaywave;
	stEPOper.iss_ref = NULL;//FcnIISReferral;
	stEPOper.acctype_sel = AccTypeSel;
	stEPOper.inc_tsc = FcnGetAndIncTsc;
	stEPOper.cert_confirm = CertConfirm;
	stEPOper.lcd_msg = LcdMsg;
	stEPOper.candidate_sel = CandidateSel_EP;
	stEPOper.emv_asc_2_bcd = Asc2Bcd;
	stEPOper.emv_bcd_2_asc = Bcd2Asc;
	stEPOper.emv_c4_2_uint = C42Unit;
	stEPOper.emv_uint_2_c4 = Unit2C4;
	stEPOper.bcd_2_uint64 = Bcd2Unit64;
	stEPOper.emv_icc_rw = IccRfRw;
	stEPOper.emv_icc_powerdown = IccRfPowerDown;
	stEPOper.emv_icc_powerup = IccRfPowerUp;
	stEPOper.outcome_msg = NULL;//new
	stEPOper.emv_ec_switch = FcnSelectEcOrEmv;
	stEPOper.send_msg= _send_msg;//不加会溢出 
	stEPOper.dek_det = NULL;

	nRet = SDK_Entry_Point_Initialize(&stEmvFile, &stEPOper);
//		+ SDK_PayPass_Initialize(&stEmvFile, &stEPOper)
//		+ SDK_PayWave_Initialize(&stEmvFile, &stEPOper)
//		+ SDK_Pure_Initialize(&stEmvFile, &stEPOper);
	return nRet;
}


/**
* @brief Delete CAPK
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CallbackCAPK(void)
{

	EMV_OperCAPK(NULL, CAPK_CLR);	
	return APP_SUCC;
}

/**
* @brief Delete AID
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int DelAIDParam(void)
{
	SDK_EP_OperAID(NULL, AID_CLR);
	return APP_SUCC;
}

int Spire_CLSS_SetCfgDefaultkKernel(STCLSSPARAM *pstCLSSParam)
{
	
    memcpy(pstCLSSParam->usAppVer, "\x00\x02", 2);
    memcpy(pstCLSSParam->usFloorlimit, "\x00\x00\x61\xA8", 4);
    memcpy(pstCLSSParam->usAcqId, "\x00\x01\x23\x45\x67\x89", 6);
    memcpy(pstCLSSParam->usMerCategoryCode, "\x12\x34", 2);
    memcpy(pstCLSSParam->usMerchantId, "\x42\x43\x54\x45\x53\x54\x20\x4c\x61\x62\x00\x00\x00\x00\x00", 15);
    memcpy(pstCLSSParam->usTransCurrCode, "\x04\x58", 2);
    memcpy(pstCLSSParam->usTransRefCurrCode, "\x04\x58", 2);
    memcpy(pstCLSSParam->usTermCountryCode, "\x04\x58", 2);
    memcpy(pstCLSSParam->usIfdSerialNum, "\x38\x35\x31\x30\x49\x43\x43\x00", 8);
    memcpy(pstCLSSParam->usTerminalId, "\x4e\x4c\x2d\x38\x35\x31\x30\x00", 8);
    memcpy(pstCLSSParam->usDefaultDdol, "\x9f\x37\x04\x9f\x47\x01\x8f\x01\x9f\x32\x01", 11);
    memcpy(pstCLSSParam->usDefaultTdol, "\x9f\x08\x02", 3);
   	memcpy(pstCLSSParam->usDefaultUdol, "\x9f\x6a\x04", 3);
    memcpy(pstCLSSParam->usMagAppVer, "\x00\x01", 2);
    memcpy(pstCLSSParam->usPwConfig, "\x04\x10", 2);
    memcpy(pstCLSSParam->usKernelId, "\x02\x00\x00\x00\x00\x00\x00\x00", 8);

    //pstCLSSParam->ucEcIndicator = 0x01;
    pstCLSSParam->ucPosEntry = 0x05;
    pstCLSSParam->ucTransCurrExp = 0x02;
    pstCLSSParam->ucTransRefCurrExp = 0x02;
    pstCLSSParam->ucDefaultDdolLen = 0x0b;
    pstCLSSParam->ucDefaultTdolLen = 0x03;
    pstCLSSParam->ucDefaultUdolLen = 0x03;
    pstCLSSParam->ucAppSelIndicator = 0x01;
    pstCLSSParam->ucFallBackPosentry = 0x80;
    pstCLSSParam->ucCapNoCvm = 0x00;

	pstCLSSParam->ucVisaTtqPresent = 0x01;

	return APP_SUCC;

}

int InitEMVL2Param(void)
{
	int nRet;
	STCLSSPARAM stClssEPParam;
	char szMsg[65]={0};
		
	memset(&stClssEPParam, 0, sizeof(STCLSSPARAM));
	nRet = SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_R);
TRACE(("InitEMVL2Param Get config=%d", nRet));	
	if (nRet != 0)
	{
		TRACE(("SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_R) error, nRet = %d", nRet));
	}
	
	/*transaction reference currency convert */
	memset(stClssEPParam.usTransRefConv, 0x00, sizeof(stClssEPParam.usTransRefConv));
	
	/*script length limit */
	stClssEPParam.ucScriptDevLimit= 0x00;		

	/**<ICS*/
	/**<AS   : Application Selection*/
	ics_opt_set(AS_Support_PSE, stClssEPParam.usIcs);
	ics_opt_set(AS_Support_CardHolder_Confirm, stClssEPParam.usIcs);
	ics_opt_set(AS_Support_Preferred_Order, stClssEPParam.usIcs);
	ics_opt_set(AS_Support_Partial_AID, stClssEPParam.usIcs);
	ics_opt_unset(AS_Support_Multi_Language, stClssEPParam.usIcs);
	ics_opt_set(AS_Support_Common_Charset, stClssEPParam.usIcs);
	/**<DA   : Data Authentication*/
	ics_opt_set(DA_Support_IPKC_Revoc_Check, stClssEPParam.usIcs);
	ics_opt_set(DA_Support_Default_DDOL, stClssEPParam.usIcs);
	ics_opt_unset(DA_Support_CAPKLoad_Fail_Action, stClssEPParam.usIcs);
	ics_opt_set(DA_Support_CAPK_Checksum, stClssEPParam.usIcs);
	/**<CV   : Cardholder Verification*/
	ics_opt_set(CV_Support_Bypass_PIN, stClssEPParam.usIcs);
	ics_opt_set(CV_Support_PIN_Try_Counter, stClssEPParam.usIcs);
	ics_opt_set(CV_Support_Fail_CVM, stClssEPParam.usIcs);
	ics_opt_set(CV_Support_Amounts_before_CVM, stClssEPParam.usIcs);
	ics_opt_unset(CV_Support_Bypass_ALL_PIN, stClssEPParam.usIcs);
	/**<TRM  : Terminal Risk Management*/
	ics_opt_set(TRM_Support_FloorLimit, stClssEPParam.usIcs);	
	ics_opt_set(TRM_Support_RandomSelect, stClssEPParam.usIcs);	
	ics_opt_unset(TRM_Support_VelocityCheck, stClssEPParam.usIcs);	//tci close
	ics_opt_set(TRM_Support_TransLog, stClssEPParam.usIcs);	
	ics_opt_set(TRM_Support_ExceptionFile, stClssEPParam.usIcs);	
	ics_opt_unset(TRM_Support_AIPBased, stClssEPParam.usIcs);	
	ics_opt_set(TRM_Use_EMV_LogPolicy, stClssEPParam.usIcs);	
	/**<TAA  : Terminal Action Analysis*/
	ics_opt_set(TAA_Support_TAC, stClssEPParam.usIcs);	
	ics_opt_unset(TAA_Support_DAC_before_1GenAC, stClssEPParam.usIcs);	
	ics_opt_set(TAA_Support_DAC_after_1GenAC, stClssEPParam.usIcs);	
	ics_opt_unset(TAA_Support_Skip_DAC_OnlineFail, stClssEPParam.usIcs);	
	ics_opt_set(TAA_Support_DAC_OnlineFail, stClssEPParam.usIcs);	
	ics_opt_set(TAA_Support_CDAFail_Detected, stClssEPParam.usIcs);	
	ics_opt_set(TAA_Support_CDA_Always_in_ARQC, stClssEPParam.usIcs);	
	ics_opt_set(TAA_Support_CDA_Alawys_in_2TC, stClssEPParam.usIcs);	
	/**<CP  : Completion Process*/
	ics_opt_set(CP_Support_Force_Online, stClssEPParam.usIcs);	
	ics_opt_set(CP_Support_Force_Accept, stClssEPParam.usIcs);	
	ics_opt_unset(CP_Support_Advices, stClssEPParam.usIcs);	
	ics_opt_set(CP_Support_Issuer_VoiceRef, stClssEPParam.usIcs);	
	ics_opt_set(CP_Support_Batch_Data_Capture, stClssEPParam.usIcs);	
	ics_opt_set(CP_Support_Online_Data_capture, stClssEPParam.usIcs);	
	ics_opt_set(CP_Support_Default_TDOL, stClssEPParam.usIcs);	
	/**<MISC : Miscellaneous*/
	ics_opt_set(MISC_Support_Account_Select, stClssEPParam.usIcs);
	ics_opt_unset(MISC_Support_ISDL_Greater_than_128, stClssEPParam.usIcs);
	ics_opt_set(MISC_Support_Internal_Date_Mana, stClssEPParam.usIcs);
	ics_opt_set(MISC_PP_Support_Default_UDOL, stClssEPParam.usIcs);
	ics_opt_set(MISC_MISC_PP_Support_MagAppVer, stClssEPParam.usIcs);
PubDebugData("ICS: ", stClssEPParam.usIcs, 7);

	/*Test type indicator*/
	stClssEPParam.ucStatus = MASTERCARD_ENB;//MCCS_ENB;//MASTERCARD_ENB;//;//PBOC2_ENB;

	/**支持电子现金*/
	stClssEPParam.ucEcIndicator= 0x00;//0x01;
	
	/**<TT : Terminal Type*/
	stClssEPParam.ucType= 0x22;

	/**< TC : Terminal Capabilities		定义终端性能*/
	terminal_cap_set(TC_Manual_Key_Entry, stClssEPParam.usCap);
	terminal_cap_set(TC_Magnetic_Stripe, stClssEPParam.usCap);
	terminal_cap_set(TC_IC_With_Contacts, stClssEPParam.usCap);
	terminal_cap_set(TC_Plaintext_PIN, stClssEPParam.usCap);
	terminal_cap_set(TC_Enciphered_PIN_Online, stClssEPParam.usCap);
	terminal_cap_set(TC_Signature_Paper, stClssEPParam.usCap);
	terminal_cap_set(TC_Enciphered_PIN_Offline, stClssEPParam.usCap);
	terminal_cap_set(TC_No_CVM_Required, stClssEPParam.usCap);  
	terminal_cap_unset(TC_Cardholder_Cert, stClssEPParam.usCap);

	terminal_cap_set(TC_SDA, stClssEPParam.usCap);
	terminal_cap_set(TC_DDA, stClssEPParam.usCap);
	terminal_cap_unset(TC_Card_Capture, stClssEPParam.usCap);
	terminal_cap_set(TC_CDA, stClssEPParam.usCap);
PubDebugData("usCap: ", stClssEPParam.usCap, 3);
	/**< ATC : Additional Terminal Capabilities	终端附加性能 */
	add_terminal_cap_unset(ATC_Cash, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Goods, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Services, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Cashback, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Inquiry, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Transfer, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Payment, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Administrative, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Cash_Deposit, stClssEPParam.usAddCap);  //lld 2013-4-17
	add_terminal_cap_set(ATC_Numeric_Keys, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Alphabetic_Special_Keys, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Command_Keys, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Function_Keys, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Print_Attendant, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Print_Cardholder, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Display_Attendant, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Display_Cardholder, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_10, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_9, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_8, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_7, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_6, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_5, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_4, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_3, stClssEPParam.usAddCap);
	add_terminal_cap_unset(ATC_Code_Table_2, stClssEPParam.usAddCap);
	add_terminal_cap_set(ATC_Code_Table_1, stClssEPParam.usAddCap);
PubDebugData("usAddCap: ", stClssEPParam.usAddCap, 5);

	 /* 9F39(Terminal), n2, 1 bytes */
	stClssEPParam.ucPosEntry= 0x80;              
	/* 9F01(Terminal), n6-11, 6 bytes */
	memcpy(stClssEPParam.usAcqId, "\x12\x34\x56\x78\x90\x00", 6);
	/* 9F15(Terminal), n4, 2 bytes */
	memcpy(stClssEPParam.usMerCategoryCode, "\x12\x34", 2);    
	/* 9F16(Terminal), ans15, 15 bytes */
	memcpy(stClssEPParam.usMerchantId, "123456789012345", 15);         
	/* 5F2A(Terminal), n3, 2 bytes */
	memcpy(stClssEPParam.usTransCurrCode, "\x04\x58", 2);      
	/* 5F36(Terminal), n1, 1 bytes */
	stClssEPParam.ucTransCurrExp= 0x00;          
	/* 9F3C(Terminal), n3, 2 bytes */
	memcpy(stClssEPParam.usTransRefCurrCode, "\x04\x58", 2);  
	/* 9F3D(Terminal), n1, 1 bytes */
	stClssEPParam.ucTransRefCurrExp= 0x00;      
	/* 9F1A(Terminal), n3, 2 bytes */
	memcpy(stClssEPParam.usTermCountryCode, "\x04\x58", 2);    
	/* 9F1E(Terminal), an8, 8 bytes */
	memcpy(stClssEPParam.usIfdSerialNum, "00000001", 8);       
	/* 9F1C(Terminal), an8, 8 bytes */
	memcpy(stClssEPParam.usTerminalId, "12345678", 8);   
	/* fallback pos entry*/
	stClssEPParam.ucFallBackPosentry = 0x00;
	 /* limist exist?(判断以下限额是否存在的标识)
								bit 1	=1	EC limint exsit
								bit 2	=1	contactless limit exsit
								bit 3	=1	contactless offline limit exsit
								bit 4	=1	cvm limit  exsit*/
	stClssEPParam.ucLimitExist = 1<<0 | 1<< 1 | 1<< 2 | 1<<3 | 0x10;

	/* 9F7B 电子现金终端限额n12  6bytes*/
	memcpy(stClssEPParam.usEcLimit, "\x00\x00\x00\x05\x00\x00", 6); 
	/* 非接触终端交易限额 n12  6bytes */
	memcpy(stClssEPParam.usClLimit, "\x00\x99\x99\x99\x99\x99", 6); 
	/* 非接触终端脱机最低限额n12  6bytes */
	memcpy(stClssEPParam.usClOfflineLimit, "\x00\x00\x00\x00\x00\x00", 6); 
	/* 终端执行CVM限额	n12  6bytes */
	memcpy(stClssEPParam.usCvmLimit, "\x00\x00\x00\x02\x50\x00", 6); 
	
	/**<终端交易属性*/
	memset(stClssEPParam.usTransProp, 0, sizeof(stClssEPParam.usTransProp));
	
	trans_prop_unset(EMV_PROP_MSD, stClssEPParam.usTransProp);
	trans_prop_unset(EMV_PROP_PBOCCLSS, stClssEPParam.usTransProp);
	trans_prop_set(EMV_PROP_QPBOC, stClssEPParam.usTransProp);
	trans_prop_set(EMV_PROP_PBOC, stClssEPParam.usTransProp);
	trans_prop_unset(EMV_PROP_OFFLINE_ONLY, stClssEPParam.usTransProp);
	trans_prop_set(EMV_PROP_ONLINEPIN, stClssEPParam.usTransProp);
	trans_prop_set(EMV_PROP_SIGNATURE, stClssEPParam.usTransProp);
	trans_prop_set(EMV_PROP_01VERSUPPORT, stClssEPParam.usTransProp);
PubDebugData("1stClssEPParam.usTransProp: ", stClssEPParam.usTransProp, 4);
	 stClssEPParam.ucZeroAmountAllow=1;

	/*非接触状态检查*/
	stClssEPParam.ucStatusCheck= 0x00;	/**<默认为0*/  	
	stClssEPParam.ucAidLen = 16;
	Spire_CLSS_SetCfgDefaultkKernel(&stClssEPParam);//非接默认参数沿用接触式的?此处是沿用spire的。

	stClssEPParam.ucPPTlvLen = 8;
	memcpy(stClssEPParam.usPPTlv, "\xDF\x62\x05", 3);
	mccs_ac_set(MCCS_AC_MODE, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_CONTACT, stClssEPParam.usPPTlv+3);
	mccs_ac_unset(MCCS_AC_OFFLINE, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_ONLINEPIN, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_SIGNATURE, stClssEPParam.usPPTlv+3);

	mccs_ac_unset(MCCS_AC_ONLINE_TAP, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_CC_TAP, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_ECHO, stClssEPParam.usPPTlv+3);
	mccs_ac_unset(MCCS_AC_LONG_TAP, stClssEPParam.usPPTlv+3);

	mccs_ac_set(MCCS_AC_AUTH_APPLI, stClssEPParam.usPPTlv+3);
	mccs_ac_unset(MCCS_AC_BALANCE, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_DECLINE, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_OFFLINECAM, stClssEPParam.usPPTlv+3);

	mccs_ac_set(MCCS_AC_RELIAB, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_SDA, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_DDA, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_CVM, stClssEPParam.usPPTlv+3);
	mccs_ac_set(MCCS_AC_TRM, stClssEPParam.usPPTlv+3);
	mccs_ac_unset(MCCS_AC_CDA, stClssEPParam.usPPTlv+3);
PubDebugData("pptlv: ", stClssEPParam.usPPTlv, 8);
	
	nRet = SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_W);
	TRACE(("====set config nRet=%d", nRet));
	if (nRet != 0)
	{
		TRACE(("SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_W) error, nRet = %d", nRet));
		sprintf(szMsg, "Set oper aid\nCode=%d", SDK_EP_ErrorCode());
		PubMsgDlg("EMV initialize", szMsg, 3, 1);
		return APP_SUCC;
	}
	
	ASSERT(SDK_EP_OperAID(NULL, AID_CLR));
	SDK_Entry_Point_BuildAIDList();
	EMV_buildAidList();
	return nRet;
}



/**
* @brief Init emv parameters
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int InitEMVParam(void)
{
	int nRet = 0;
	char cVal = 1;
//	emvparam stEMVParam;
	
	ExportEMVModule();

	CallbackCAPK();
	DelAIDParam();
	//EMV_buildAidList();

	SetVarEmvTransSerial(1);
	SetVarEmvIsDownAIDFlag(YES);
	SetVarEmvIsDownCAPKFlag(YES);

	//20171227 彻底禁用电子现金脱机
	//cVal = 1;
	//EMVL2_WriteNLTagData(0x0013, (uchar *)&cVal, 1);
	InitEMVL2Param();
	
	return nRet;
}


/**
* define global variable
*/
static char gszPan[19+1];
static char gszAmount[12+1];
static char gszCashAmount[12+1];
static char gcTransType;
static char gcAcqIndex;
static char gszOnlinePin[8+1];/**<online pin after encrypting*/
static char gcIsKeyInPIN = 0;

/**
* @brief Set transaction pan
* @param [in] char *pszPan 
* @returnvoid
*/
void EmvSetTxnPan(const char *pszPan)
{
	memset(gszPan, 0, sizeof(gszPan));
	memcpy(gszPan, pszPan, strlen(pszPan)>= 19 ? 19 : strlen(pszPan));
	return ;
}

/**
* @brief Set transaction type
* @param [in] char cTransType 
* @return void
*/
void EmvSetTxnType(const char cTransType)
{
	gcTransType = cTransType;
	return ;
}

/**
* @brief Get transaction amount
* @param [out] char *pszAmount 
* @returnvoid
*/
void EmvGetAmount(char *pszAmount)
{
	if (NULL != pszAmount)
	{
		memcpy(pszAmount, gszAmount, 12);
	}
	return ;
}

int EmvShowAmount(int i)
{
	TRACE(("[%d]gszAmount=%s ", i, gszAmount));
	return APP_SUCC;
}

/**
* @brief   Get emv title
* @return 
* @li 
* @li 
*/
const char *EmvDispTitle(void)
{
	switch(gcTransType)
	{
	case TRANS_SALE:
		return "SALE";
		break;
	case TRANS_PREAUTH:
		return "PREAUTH";
		break;
	case TRANS_BALANCE:
		return "BALANCE";
		break;
	default:
		return "";
		break;
	}
}
/**
* @brief  clear emv online pin
* @return void
* @li 
* @li 
*/
void EmvClrOnlinePin(void)
{
	memset(gszOnlinePin, 0, sizeof(gszOnlinePin));
	return ;
}
/**
* @brief  get emv online pin
* @return 
* @li 
* @li 
*/
void EmvGetOnlinePin(char *psOnlinePin)
{
	if (NULL != psOnlinePin)
	{
		memcpy(psOnlinePin, gszOnlinePin, 8);
	}
	return ;
}




/**
* @brief 
* @return 
* @li EMV_TC
* @li EMV_AAC
*/
int FcnIISReferral(uchar * pszPan, int nPanLen)
{
	return 0;
	//return APP_FAIL;
}



/**
* @brief  Select account type (1: default  2:savings 3:Cheque/debit 4.Credit )
* @return 
* @li  0		Success
* @li  -1		Fail
*/
int AccTypeSel(void)
{
	int nSelect;
	int nRet = 0;
	int nStartItem = 1, nSelcItem = 1;
	char *pszItems[] = {
		tr("1.Default"),
		tr("2.Savings"),
		tr("3.Cheque/debit"),
		tr("4.Credit")
	};
	NDK_ScrPush();
	ASSERT_QUIT(PubShowMenuItems(tr("ACCOUNT TYPE"), pszItems, sizeof(pszItems)/sizeof(char *),&nSelcItem, &nStartItem, 60));
	//ASSERT_QUIT(ProSelectList("1.Default||2.Savings||3.Cheque/debit||4.Credit", "ACCOUNT TYPE", 0xFF, &nSelect));
	NDK_ScrPop();
	return nSelcItem;
}

/**
* @briefCheck Identity of cardholder
* @param [in]  Papers Type(Id card,Officer,Passprot,Entry Card,Temporary Id Card,Other)
* @param [in]  uchar * pszCon  Card Number
* @param [in]  int nLen   
* @return 
* @li  1		Confirm OK
* @li  0		Confirm Fail
*/
int CertConfirm(uchar ucType, uchar * pszCon, int nLen)
{
	char szTypestr[][12] = 
	{
		"ID CARD",
		"OFFICER", 
		"PASSPORT",
		"ENTRY CARD",
		"TEMP IDCARD", 
		"OTHER"
	};
	char szBuf[100] = {0};
	int nRet = 0;

	NDK_ScrPush();
	sprintf(szBuf, "TYPE:%s\nCARD NO:%*.*s", szTypestr[ucType], nLen, nLen, pszCon);
	nRet = PubConfirmDlg("SHOW CARD", szBuf, 1, 60);	
	NDK_ScrPop();
	if (APP_SUCC == nRet)
		return 1;
	else
		return 0;
}


/**
* @brief Show message in the LCD
* @param [in]  char * pszTitle
* @param [in]  uchar * pszMsg  
* @param [in]  int nLen  
* @param [in]  yesno	If need show Confrim or cancel  1Need 0 Not need
* @param [in]  waittime	
* @return    if yesno=1, return 1 means confirm，return 0 means cancel
*			 if yesno=0, retrun the key value you Input
*/
int LcdMsg(char * pszTitle, uchar * pszMsg, int nLen,  int nYesNo, int nWaitTime)
{

	int i = 0, nRet = 0;
	char szBuf[500] = {0};
	
	NDK_ScrPush();

	if (pszTitle != NULL)
	{
		sprintf(szBuf, "%s\n", pszTitle);
	}
	
	if (pszMsg != NULL && nLen > 0) 
	{
		for (i = 0; i < nLen; i++)		
			sprintf(szBuf+strlen(szBuf), "%02x", *(pszMsg + i));
		sprintf(szBuf+strlen(szBuf), "\n");
	}
	if (nYesNo)
	{
		sprintf(szBuf+strlen(szBuf), "YES(Confirm)\nNO(Cancel)\n");
	}
	PubClearAll();
	
	PubDispMultLines(DISPLAY_ALIGN_BIGFONT, 1, 1, szBuf);
	
	PubUpdateWindow();
	while(1)
	{
		nRet = PubGetKeyCode(nWaitTime);
		if (!nYesNo || nRet == KEY_ESC || nRet == KEY_ENTER || nRet <= 0)
		{
			break;
		}
	} 

	if (nYesNo)
	{
		if (nRet == KEY_ENTER) 
		{
			nRet = 1;
		}
		else
		{
			nRet = 0;
		}
	}
	NDK_ScrPop();
	return nRet;
}

/**
* @brief Show AID list and let user make choise
* @param [in]  candidate * pstCan  Aid list structure
* @param [in]  int nAmt  Aid anoumt
* @param [in]  int nTimes  choice times
* @return    >=0		Aid number
*  				-1		Fail
*/

int CandidateSel(candidate * pstCan, int nAmt, int nTimes)
{
	int i, nCountIndex = 0, nCountLine = 0;
	char lszBuffer[18][40];
	uchar sIndex[9];
	char szTitle[20] = {0};
	char szDispList[200] = {0};
	int nSelect = 0xff;
	int nCupAppNum = 0;
	int nCupAppIndex[9+1] = {0};
	int nDefaultMccs = -1;
	
	/*At most 9 applications*/
	for (i = 0; i < nAmt; i++) 
	{
		if (pstCan[i]._enable)
		{
			if (9 == nCountIndex)
			{
				break;
			}
			if (0 == memcmp(pstCan[i]._aid, "\xA0\x00\x00\x03\x33", 5)) 
			{		
				nCupAppIndex[nCupAppNum++] = i;
			}
			sIndex[nCountIndex ++] = i;
			sprintf(lszBuffer[nCountLine], "%d.", nCountIndex);
			if (pstCan[i]._preferred_name_len != 0) 
			{
				memcpy(&(lszBuffer[nCountLine][2]), pstCan[i]._preferred_name, pstCan[i]._preferred_name_len);
				lszBuffer[nCountLine][pstCan[i]._preferred_name_len + 2] = 0;
			}
			else 
			{
				Bcd2Asc(pstCan[i]._aid, pstCan[i]._aid_len * 2, (uchar *)&(lszBuffer[nCountLine][2]), 0);
			}		
			if (YES == GetVarIsMccsPriority())
			{
				if (0 == memcmp(pstCan[i]._aid, "\xA0\x00\x00\x06\x15", 5))
				{
					nDefaultMccs = i;
		PubDebug("#######nDefaultMccs = %d", nDefaultMccs);			
				}
			}
			nCountLine ++;
		}
	}
	if (nCupAppNum == 1) 
	{
		return nCupAppIndex[nCupAppNum - 1];
	}
	else   
	{
		NDK_ScrPush();
		NDK_ScrClrs();

		if (YES == GetVarIsMccsPriority())
		{
			if (nDefaultMccs >= 0)
			{
		PubDebug("####### default");			
				return nDefaultMccs;
			}
		}
		if (nTimes)
		{
			strcpy(szTitle, "CHOOSE AGAIN");
		} 
		else
		{
			strcpy(szTitle, "CHOOSE THE APP");
		}
		memset(szDispList, 0, sizeof(szDispList));
		for (i=0; i< nCountLine; i++)
		{
			sprintf(szDispList+strlen(szDispList), "%s||", lszBuffer[i]);
		}
		while (1)
		{
PubDebug("Sel app [list=%s][default: %d]", szDispList, sIndex[0]);
			return sIndex[0];//first app shtest  20170316 for shtest
			ASSERT_QUIT(ProSelectList(szDispList, szTitle, 0xff, &nSelect));
			if (nSelect >= 0 && nSelect <= 9)
			{
				break;
			}
		}
		NDK_ScrPop();
		return nSelect-1;
	}
	
}
int CandidateSel_EP(ep_candidate * pstCan, int nAmt, int nTimes)
{
	int i, nCountIndex = 0, nCountLine = 0;
	char lszBuffer[18][40];
	uchar sIndex[9];
	char szTitle[20] = {0};
	char szDispList[200] = {0};
	int nSelect = 0xff;
	int nCupAppNum = 0;
	int nCupAppIndex[9+1] = {0};
	int nDefaultMccs = -1;
PubDebug("########### select app DefaultMccs = %d", nAmt);			
	/*最多9个应用，一行最多15个字符*/
	for (i = 0; i < nAmt; i++) 
	{
PubDebugData("#######aid: ", pstCan[i].usAid, 5);			
PubDebug("########### enable = %d", pstCan[i].ucEnable);			
		if (pstCan[i].ucEnable)
		{
			if (9 == nCountIndex)
			{
				break;
			}
			if (0 == memcmp(pstCan[i].usAid, "\xA0\x00\x00\x03\x33", 5)) //银联应用(卡检测试只有此RID,而我们EMV_OrgSheet还有其他,这个?)
			{		
				nCupAppIndex[nCupAppNum++] = i;
			}
			sIndex[nCountIndex ++] = i;
			sprintf(lszBuffer[nCountLine], "%d.", nCountIndex);
			if (pstCan[i].ucPreferredNameLen != 0) 
			{
				memcpy(&(lszBuffer[nCountLine][2]), pstCan[i].usPreferredName, pstCan[i].ucPreferredNameLen);
				lszBuffer[nCountLine][pstCan[i].ucPreferredNameLen + 2] = 0;
			}
			else 
			{
				Bcd2Asc(pstCan[i].usAid, pstCan[i].ucAidLen * 2, (uchar *)&(lszBuffer[nCountLine][2]), 0);
			}		
			if (YES == GetVarIsMccsPriority())
			{
				if (0 == memcmp(pstCan[i].usAid, "\xA0\x00\x00\x06\x15", 5))
				{
					nDefaultMccs = i;
		PubDebug("#######nDefaultMccs = %d", nDefaultMccs);			
				}
			}
			nCountLine ++;
		}
	}
	if (nCupAppNum == 1)  //有银联应用,而且只有一个(无需考虑外卡应用)
	{
		return nCupAppIndex[nCupAppNum - 1];
	}
	else   
	{
		NDK_ScrPush();
		NDK_ScrClrs();

		if (YES == GetVarIsMccsPriority())
		{
			if (nDefaultMccs >= 0)
			{
		PubDebug("####### default");			
				return nDefaultMccs;
			}
		}
		if (nTimes)
		{
			strcpy(szTitle, "CHOOSE AGAIN");
		} 
		else
		{
			strcpy(szTitle, "CHOOSE THE APP");
		}
		memset(szDispList, 0, sizeof(szDispList));
		for (i=0; i< nCountLine; i++)
		{
			sprintf(szDispList+strlen(szDispList), "%s||", lszBuffer[i]);
		}
		while (1)
		{
			return sIndex[0];//first app shtest  20170316
			ASSERT_QUIT(ProSelectList(szDispList, szTitle, 0xff, &nSelect));
			if (nSelect >= 0 && nSelect <= 9)
			{
				break;
			}
		}
		NDK_ScrPop();
		return nSelect-1;
	}
	
}
/**
* @brief Asc to Bcd
* @param [in]  uchar *psAscstr	
* @param [in]  int nAsclen
* @param [in]  int nAlign   1		add 0x00 at left
*  							0		Add 0x00 at right
* @param [out]  uchar * psBcdstr 
*
* @return    	0		Success
*  				-1		Fail
*/

int Asc2Bcd(uchar * psAscstr, int nAsclen, uchar * psBcdstr, int nAlign)
{
	return PubAscToHex(psAscstr, nAsclen, (char)nAlign, psBcdstr);

}

/**
* @brief Bcd to Asc
* @param [in] uchar * psBcd		
* @param [in]  int nAsclen		
* @param [in]  uchar * psAsc 
* @param [in]  int nAlign   1		add 0x00 at left
*  							0		Add 0x00 at right
*
* @return    	0		Success
*  				-1		Fail
*/
int Bcd2Asc(uchar * psBcd, int nAsclen, uchar * psAsc, int nAlign)
{
	return PubHexToAsc(psBcd, nAsclen, (char)nAlign, psAsc);
}

/**
* @brief convert a 4bytes string to int
* @param [in] uchar * psC4	the string need to change
* @return    int value
*/
uint C42Unit(uchar * psC4)
{
	uint nNum = 0;
	if(psC4 == NULL)
		return APP_FAIL;
	
	PubC4ToInt(&nNum, psC4);
	return nNum;
}


/**
* @brief  convert int to a 4bytes string
* @param [in] uint nNum
* @param [in] uchar * psC4  
* @return   
*/
void Unit2C4(uint nNum, uchar * psC4)
{
	PubIntToC4(psC4, nNum);
}



/**
* @brief FcnGetAndIncTsc
* @param void
* @return
* @li  serial number
*/
int FcnGetAndIncTsc(void)
{
	int nSerial = 0;

	GetVarEmvTransSerial(&nSerial);
	IncVarEmvTransSerial();
	return nSerial;
}

static int ShowPinInputFormat(const char *pszAmount)
{
	int i;
	int nRet;
	int nLen;
	int nFlag=0;
	int nStatus=0;
	int nPressKey=0;
    int nKeyCode = 0;
	uint unFontWidth,unFontHeight;
	uint unX,unY,unFormWidth,unFormHeight;
	char* psPswTip="Please enter PIN:";
	char sPin[20];
    char szPinStr[20+1];
	char szDispAmt[64+1];
	char szExpPinLenIn[100+1];
	uchar ucMode;
	char szKsn[20+1] = {0};

	NDK_ScrGetViewPort(&unX,&unY,&unFormWidth,&unFormHeight);
	memset(szDispAmt, 0, sizeof(szDispAmt));
	PubClear2To4();
	if(pszAmount != NULL && 0 != strlen(pszAmount))
	{
		if (memcmp(pszAmount, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12) == 0)
		{
			;
		}
		else
		{
			PubConvAmount(NULL,(uchar *)pszAmount,2,(uchar *)szDispAmt);
		}
		NDK_ScrSetFontType(DISPFONT_EXTRA_LARGE);
		NDK_ScrGetFontSize(&unFontWidth,&unFontHeight);
		NDK_ScrSetFontColor(0xFFFF, FONTCOLOR_NORMAL);
		NDK_ScrSetAttr(TEXT_ATTRIBUTE_NOBACKCOLOR, NULL);
		NDK_ScrDispPic(25, 40, 269, 50, "money_background.png");
		nLen = strlen(szDispAmt);
		nLen = (nLen>13)?13:nLen;
		unX = 25+(269-20-nLen*(unFontWidth/2))/2;
		NDK_ScrDispString(unX+17+3,40+9,szDispAmt,0);

		NDK_ScrSetFontType(DISPFONT_EXTRA);
		NDK_ScrGetFontSize(&unFontWidth,&unFontHeight);
		NDK_ScrSetFontColor(0x0000, FONTCOLOR_NORMAL);
		NDK_ScrSetFontType(DISPFONT_EXTRA);
		NDK_ScrSetFontColor(0x3384, FONTCOLOR_NORMAL);
		NDK_ScrGetFontSize(&unFontWidth,&unFontHeight);
		nLen = strlen(psPswTip);
		unY = 95;
		NDK_ScrDispString(35,unY,psPswTip,0);
		unX = 35+nLen*unFontWidth/2;
		NDK_ScrRefresh();
	}
	else
	{
		NDK_ScrSetFontType(DISPFONT_EXTRA);
		NDK_ScrGetFontSize(&unFontWidth,&unFontHeight);
		NDK_ScrSetFontColor(0x0000, FONTCOLOR_NORMAL);
		PubDisplayStrInline(0, 2, psPswTip);
		NDK_ScrSetFontType(DISPFONT_EXTRA_LARGE);
		NDK_ScrGetFontSize(&unFontWidth,&unFontHeight);
		NDK_ScrRefresh();
	}
	NDK_ScrSetFontColor(0xFACB, FONTCOLOR_NORMAL);
	
	return APP_SUCC;
}


/**
* @brief 按彩屏界面显示内容
* @param [in] unY          显示内容纵坐标
* @param [in] pszContent   显示内容
* @return 当前光标Y所在位置
*/
int  ProDispContent(uint unX, uint unY, char *pszContent)
{
	uint unWidth, unHeight;

	NDK_ScrGetFontSize(&unWidth, &unHeight);
	NDK_ScrDispString(unX+8, unY, pszContent ,0);
	NDK_ScrGetxy(&unX, &unY);
	if(unX == 0)
	{
		return unY;
	}
	return unY + unHeight + 0;
}

int ProInputxy(int nClumno, int nLineno, char* pszOut, int* pnOutLen, int nMinLen, int nMaxLen,
	int nTimeOut, int nEditMask)
{
	char	cCurLetter;
	int		i,j;
	int		nKey;					/*处理按键信息*/
	int		nNum = 0;				/*字符的个数*/
	int		numoffset = 0;	
	int		nShowChange = 1;		/*判断是否有效按键按下*/
	int		tableoffset = 0;
	long int lnBigNum,lnSmallNum;
	uint unX,unY;
	uint unScrWidth,unScrHeight;
	uint unFontWidth,unFontHeight;
	char szTmp[128] = {0};
	char szGetBuf[128] = {0};
	int UI_GetHspace = 0;
	const char  ChTbl[12][25]=
	{
		{'0', '=', '<', '>', '(', ')', '&', '{', '}', '[', ']', 0},
		{'1', 'Q', 'Z', 'q', 'z', ' ', '#', '*', 0},
		{'2', 'A', 'B', 'C', 'a', 'b', 'c', 0},
		{'3', 'D', 'E', 'F', 'd', 'e', 'f', 0},
		{'4', 'G', 'H', 'I', 'g', 'h', 'i', 0},
		{'5', 'J', 'K', 'L', 'j', 'k', 'l', 0},
		{'6', 'M', 'N', 'O', 'm', 'n', 'o', 0},
		{'7', 'P', 'R', 'S', 'p', 'r', 's', 0},
		{'8', 'T', 'U', 'V', 't', 'u', 'v', 0},
		{'9', 'W', 'X', 'Y', 'w', 'x', 'y', 0},
		{'.', '-', '_', ',', '@', '\\', '/',
		 '+', '|', '~', '%', '^', ';', '$', 
		 '!', ':', '?', '\'', '\"', '`', 0},
		{'*', '^', ';', '$', '!', ':', '?', 0}
	};

	nNum = strlen(pszOut);
	if (nNum > nMaxLen)
	{
		return APP_FAIL;
	}

	NDK_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	NDK_ScrSetAttr(TEXT_ATTRIBUTE_NOBACKCOLOR, NULL);
	if (nEditMask == INPUT_MODE_AMOUNT)
	{
		if (nNum > 0)
		{/*<如果预先有内容*/
			if(nNum > 9)
			{/*<atol 最大值2147483647 (10位)*/
				memcpy(szTmp, pszOut, nNum-2);
				sprintf( szGetBuf, "%ld%s", atol(szTmp), pszOut+nNum-2 );
			}
			else
			{
				sprintf( szGetBuf, "%ld", atol(pszOut) );
			}
			nNum = strlen(szGetBuf);
		}
		NDK_ScrSetFontType(DISPFONT_EXTRA);
		NDK_ScrGetFontSize(&unFontWidth, &unFontHeight);
		NDK_ScrSetFontColor(0x0000, FONTCOLOR_NORMAL);
	}
	else
	{
		strcpy (szGetBuf, pszOut);
		if(nNum > 0 && nEditMask == INPUT_MODE_STRING)
		{/*计算tableoffset偏移*/
			cCurLetter = szGetBuf[nNum-1];
			for(i=0; i<sizeof(ChTbl)/sizeof(ChTbl[0]); i++)
			{
				for(j=0; j<sizeof(ChTbl[0]); j++)
				{
					if(ChTbl[i][j] == 0)
					{
						break;
					}
					if(cCurLetter == ChTbl[i][j])
					{
						tableoffset = i;
						numoffset = j;
						i = sizeof(ChTbl)/sizeof(ChTbl[0]);/**<找到退出循环*/
						break;
					}
				}
			}
		}
	}
	NDK_KbHit( &nKey );
	NDK_ScrSetFontType(DISPFONT_EXTRA_LARGE);
	NDK_ScrGetFontSize(&unFontWidth, &unFontHeight);
	NDK_ScrSetFontColor(0xFACB, FONTCOLOR_NORMAL);
	unX = (unScrWidth-8*2)/(unFontWidth/2);

	while(1)
	{
		if (nShowChange == 1)
		{
			NDK_ScrClrLine((nLineno-1)*(unFontHeight+UI_GetHspace), (nLineno+(nNum+1)/unX)*(unFontHeight+UI_GetHspace));
			switch (nEditMask)
			{
			case INPUT_MODE_PASSWD:
				memset(szTmp, '*', nNum);
				szTmp[nNum]=0;
				ProDispContent((nClumno-1)*(unFontWidth/2), (nLineno-1)*(unFontHeight+UI_GetHspace), szTmp);
				if(nNum == 0)
				{
					NDK_ScrDispString(8,(nLineno-1)*(unFontHeight+UI_GetHspace), "_" ,0);
				}
				else if (nNum < nMaxLen)
				{
					NDK_ScrPrintf("_");
				}
				break;
			default:
				break;
			}
		}

		nShowChange = 1;
		nKey = PubGetKeyCode(nTimeOut);
		switch (nKey)
		{
		case 0:
			return APP_TIMEOUT;
#if 0
		case KEY_LETTER:
			/*如果字符串输入功能建按下输入字母符号*/
			if (nEditMask == INPUT_MODE_STRING)
			{
				if(nNum == 0)
				{
					//ProBeep(1);
					break;
				}
				if (nNum >= nMaxLen)
				{
					//ProBeep(1);
				}

                numoffset++;
                if (ChTbl[tableoffset][numoffset] == 0)
                {
                	numoffset=0;
                }
                nKey = ChTbl[tableoffset][numoffset];
                if (nKey >= 0x20)
                {
                    nNum --;
                    szGetBuf[nNum] = nKey;
                    nNum++;
                }
			}
			break;
#endif
		case KEY_POINT:
			/*非一般字串模式下不允许输入'.'*/
			if (nEditMask != INPUT_MODE_STRING && nEditMask != INPUT_MODE_NUMBERPOINT)
			{
				break;
			}
			else if ((nEditMask == INPUT_MODE_STRING) && (nNum < nMaxLen))
			{
				szGetBuf[nNum] = nKey;
				nNum++;
				tableoffset = 10;
				numoffset = 0;
				break;
			}
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
			if(nKey == KEY_0)
			{/*金额输入不能以0开始*/
				if ((nEditMask == INPUT_MODE_AMOUNT) && (nNum == 0))
				{
					break;
				}
			}
			if (nNum >= nMaxLen)
			{
				//ProBeep(1);
			}
			else if( nEditMask != INPUT_MODE_STRING)
			{
				szGetBuf[nNum] = nKey;
				nNum++;
				if((nEditMask==INPUT_MODE_AMOUNT) && strlen(szGetBuf)>9)
				{//atol 最大值2147483647 (10位)
					memset(szTmp,0,sizeof(szTmp));
					memcpy(szTmp,szGetBuf,strlen(szGetBuf)-2);
					lnBigNum= atol(szTmp);
					if(lnBigNum > 200000000)
					{
						szGetBuf[--nNum] = 0;
						//ProBeep(1);
					}
				}
			}
			else if( nEditMask == INPUT_MODE_STRING )
			{
				szGetBuf[nNum] = nKey;
				nNum++;
				tableoffset = nKey - '0';
				numoffset = 0;
			}
			break;
		case KEY_BACK:
			if (nNum > 0)
			{
				szGetBuf[--nNum] = 0;
				if( nEditMask == INPUT_MODE_STRING )
				{
					cCurLetter = szGetBuf[nNum-1];
					for(i=0; i<12; i++)
					{
						for(j=0; j<25; j++)
						{
							if(ChTbl[i][j] == 0)
								break;
							if(cCurLetter == ChTbl[i][j])
							{
								tableoffset = i;
								numoffset = j;
								i=13;/**<找到退出循环*/
								break;
							}
						}
					}
				}
			}
			break;
		case KEY_ENTER:
			if ((nNum >= nMinLen) && (nNum <= nMaxLen))
			{
				if (nEditMask == INPUT_MODE_AMOUNT && nMinLen == 1)
				{//如果金额输入不允许0,
					if( atol(szGetBuf) == 0 )
					{
						break;
					}
				}
				memcpy (pszOut, szGetBuf, nNum);
				pszOut[nNum] = '\0';
				*pnOutLen = nNum;
				return APP_SUCC;
			}
			break;
		case KEY_ESC:
			return APP_QUIT;
#if 0
		case KEY_STAR:
			if ((nEditMask == INPUT_MODE_STRING) && (nNum < nMaxLen))
			{
				szGetBuf[nNum] = nKey;
				nNum++;
				tableoffset = 11;
				break;
			}
			break;		
#endif
		default:
			nShowChange = 0;
			break;
		}
	}
	return APP_SUCC;
}



/**
* @brief Get Pin
* @param in cType Pin type   0x01－Offline Pin  0x02-Final Offline pin   0x03 online pin
* @param in pszPin Cleat Pin
* @return 
* @li -1 Fail;  -2 Not Get Pin; -3 Suspend transaction; > 0 the length of Pin
*/

int GetPinEntry(int cType, char * pszPin)
{
	char szPin[12+1] = {0};
	int nRet = 0, i;
	int nPinLen = 12;
	char szDispAmt[64+1] = {0};
	char szMsg[64] = {0};
	char *pszAmount = NULL;
	int nTimeOut = 120;
	char szRandKey[8+1] = {0};
	char szContent[100] = {0};
	char szTotal[14] = {0};
	
	AmtAddAmt((uchar *)gszAmount, (uchar *)gszCashAmount, (uchar *)szTotal);

	GetVarPinpadTimeOut(&nTimeOut);

	TRACE(("cType=%d pszPin=%s amount=%s gcTransType=%d", cType,pszPin,gszAmount,gcTransType));
	switch(cType)
	{
	case 0x01:
//		strcpy(szMsg, "Offline Pin:");
		strcpy(szMsg, "Please enter PIN:");
		break;
	case 0x02:
		strcpy(szMsg, "Final Offline Pin:");
		break;
	case 0x03:
		strcpy(szMsg, "Online Pin:");
		break;
	default:
		strcpy(szMsg, "Pls Input Pin:");
		break;
	}
	if ((gcTransType != TRANS_BALANCE))
	{
		pszAmount = szTotal;
	}
	/**
	* Show Amount
	*/
	memset(szDispAmt, 0, sizeof(szDispAmt));
	if (NULL != pszAmount)
	{
		ProAmtToDispOrPnt(pszAmount, szDispAmt);
	}

	memset(gszOnlinePin, 0, sizeof(gszOnlinePin));
	
	if (cType == 0x03)
	{
		nRet = GetPin(gszPan, pszAmount, gszOnlinePin);
		if (nRet != APP_SUCC)
		{			
			if (nRet == APP_QUIT)
			{
				return -3;
			}
			return APP_FAIL;
		}
		if (memcmp(gszOnlinePin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8)  == 0)
		{	
			TRACE_HEX((gszOnlinePin, 8, "gszOnlinePin is "));
			return -2;
		}
		if (NULL != pszPin)
		{
			memcpy(pszPin, "123456", 6);/**<Retrun to kernel，kernel only use to check*/
		}
		gcIsKeyInPIN = 1;
		return 6;
	}

	if (YES == GetVarIsPinpad())
	{
		PubClear2To4();

		/**
		* Show Amount
		*/
		if (NULL != pszAmount)
		{
			PubDispPinPad(NULL, szDispAmt, NULL, NULL);
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT,2,"|R%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_NORMAL, 2, 1,"%s RM", szDispAmt);
		}

		PubDisplayStrInline(0, 3, szMsg);
		PubUpdateWindow();
		srand((unsigned) time (NULL));
		i = rand ();
		memcpy(szRandKey, &i, 4);
		memcpy(szRandKey+4, &i, 4);
		nRet =  SetPinpadKey(szRandKey, EMV_PINPAD_POS_KEYINDEX);
		if (nRet != APP_SUCC)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CSetPinpadKey FAIL");
			PubMsgDlg("Input Pin", szContent, 3, 1);
			PubClrPinPad();
			return APP_FAIL;
		}

		PubSetCurrentMainKeyIndex(EMV_PINPAD_POS_KEYINDEX);
#if 0
		nRet = PubActiveKey(DESMODE_3DES == GetVarEncyptMode() ? DESMODE_3DES : DESMODE_DES,\
			EMV_PINPAD_POS_KEYINDEX, KEY_TYPE_PIN);
		if (nRet != APP_SUCC)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CActiveKey FAIL");
			PubMsgDlg("Input Pin", szContent, 3, 10);
			PubClrPinPad();
			return APP_FAIL;
		}
#endif
		memset(szPin, 0, sizeof(szPin));
		nRet = PubGetPin(szPin, &nPinLen, PINTYPE_WITHPAN, "0000000000000", szDispAmt, PIN_MAX_LEN, PIN_MIN_LEN);
		if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			PubClrPinPad();
			return -3;
		}
		if (nRet != APP_SUCC)
		{
			PubClrPinPad();
			return APP_FAIL;
		}
		if (memcmp(szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
		{
			PubClrPinPad();
			return -2;
		}
		nRet = DealPin(szPin, pszPin, szRandKey);
		if (nPinLen > 0)
		{
			gcIsKeyInPIN = 1;
		}
		return nRet;
	}
	else
	{
		while(1)
		{
			PubClear2To4();
			memset(szPin, 0, sizeof(szPin));
			ShowPinInputFormat(pszAmount);
			//PubDisplayStrInline(0, 2, szMsg);
//			PubDisplayStrInline(0, 5, "Pls [Enter]");
			//PubUpdateWindow();
			nPinLen = 0;
			nRet = ProInputxy(3, 5, szPin, &nPinLen, 0, PIN_MAX_LEN, nTimeOut, INPUT_MODE_PASSWD);
			if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
			{
				return -3;
			}
			if (nRet != APP_SUCC)
			{
				return APP_FAIL;
			}
			if (nPinLen == 0)
			{
				return -2;
			}
			if (nPinLen < PIN_MIN_LEN)
			{
				continue;
			}
			break;
		}
		memcpy(pszPin, szPin, nPinLen);
		if (nPinLen > 0)
		{
			gcIsKeyInPIN = 1;
		}
		return nPinLen;
	}
	return APP_FAIL;
}

static char gcEnInputAmtInterface = 1;
void EmvSetAmount(char *pszAmount, char *psCashbackAmt)
{
	TRACE(("pszAmount=%s", pszAmount));
	if (pszAmount != NULL)
	{
		gcEnInputAmtInterface = 0;
		memset(gszAmount, 0, sizeof(gszAmount));	
		memset(gszCashAmount, '0', sizeof(gszCashAmount)-1);	
		memcpy(gszAmount, pszAmount, 12);

	}	
	if (psCashbackAmt != NULL)
	{
		memcpy(gszCashAmount, psCashbackAmt, 12);
	}
	
	return ;
}


/**
* @brief Get Bcd Amount
* @param [in] uchar ucTransType     
* @param [in] uchar *pusBCDCash 	
* @param [in] uchar *pusBCDCashBack 
* @return 
* @li 0	Success	-1 Fail
*/
int EmvGetBcdAmt(uchar ucTransType, uchar *pusBCDCash, uchar *pusBCDCashBack)
{
	int nRet = 0;
	int nAmtLen = 12;

	if(ucTransType == EMV_TRANS_BALANCE)
	{
		memset(gszAmount, '0', 12);
		gcEnInputAmtInterface = 0;
	}
	if (gcEnInputAmtInterface == 1)
	{
		/**
		* Input Amount
		*/
		while(1)
		{
			nAmtLen = 12;
			memset(gszAmount, 0, sizeof(gszAmount));
	
			nRet = PubInputAmount(EmvDispTitle(), "Pls Input Amount:", gszAmount, &nAmtLen, INPUT_AMOUNT_MODE_NOT_NONE, INPUT_AMOUNT_TIMEOUT);
		
			if (nRet == APP_QUIT)
			{
				return -2;
			}
			if (nRet == APP_TIMEOUT)
			{
				return -3;
			}
			if (nRet != APP_SUCC)
			{
				return APP_FAIL;
			}
			/**
			* Check if Amount Is Valid
			*/
			if (CheckTransAmount(gszAmount, gcTransType,gcAcqIndex) == APP_SUCC)
			{
				break;
			}
		}
	}
	if (pusBCDCash != NULL)
	{
		 PubAscToHex((uchar *)gszAmount,12,0,pusBCDCash);
	}
	if (pusBCDCashBack != NULL)
	{
		PubAscToHex((uchar *)gszCashAmount, 12, 0, pusBCDCashBack);//"000000000000"
	}

	gcEnInputAmtInterface = 1;
	return APP_SUCC;	
}

/**
* @brief 	Get the Clear Key
* @param in psKey
* @param in inPass Pin under Key
* @param out outPass Clear Pin
* @return
* @li >0 Real Length
* @li APP_FAIL Fail
*/
static int  DealPin(const char *pszInPass, char *pszOutPass, const char *psKey)
{
	int i;
	char szTmpBuf[16+1],szPinBcd[8+1];
	char szBbuf[2+1];
	int nRet;
	char szContent[100];
	
	memset(szContent, 0, sizeof(szContent));

	memset(szBbuf, '\0', sizeof(szBbuf));
	memset(szPinBcd, '\0', sizeof(szPinBcd));
	memset(szTmpBuf, '\0', sizeof(szTmpBuf));
	
	nRet = PubSoftUnDes(psKey, (char *)pszInPass, szPinBcd);
	if (nRet != APP_SUCC)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CNot Set Encrypt Key");
		PubMsgDlg("Pinpad", szContent, 3, 10);
		return APP_FAIL;
	}
	PubHexToAsc((uchar *)szPinBcd, 16, 0, (uchar *)szTmpBuf);
	memcpy(szBbuf, szTmpBuf, 2);

	if ((szTmpBuf[1] >= 0x30 && szTmpBuf[1] <= 0x39))
	{
		i = atoi(szBbuf);
	}
	else if ((szTmpBuf[1] >= 'A' && szTmpBuf[1] <= 'C'))//
	{
		i = szTmpBuf[1]-'A'+0x0A;
	}
	else
	{
		i = 12;// At most maximum is 12
	}
	if (i > 12 || i < 4)
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CSet Encrypt Key Fail");
		PubMsgDlg("Pinpad", szContent, 3, 10);
		return APP_FAIL;
	}
	memcpy(pszOutPass, szTmpBuf+2, i);

	return i;
}


/**
* @brief Wait for Cardholder Get their Card
* @param in pszTitle
* @param in pszContent2
* @param in pszContent3
* @return 
* @li APP_SUCC
* @li APP_QUIT
*/
int DispOutICC(char *pszTitle, char *pszContent2, char *pszContent3)
{
	PubICPowerDown(ICTYPE_IC);
	PubRFCPUPowerDown();
	EMV_Suspend(-1);
	EMV_rf_suspend(-1);	
	ShowLightOff();
	if(PubGetCardStatus() == APP_SUCC)
	{
		PubClear2To4();
		if (NULL != pszTitle)
		{	
			PubDisplayTitle(pszTitle);
		}
		if (NULL != pszContent2)
		{
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 2, "|C%s", pszContent2);
			PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "%s", pszContent2);
		}
		if (NULL != pszContent3)
		{
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 3, "|C%s", pszContent3);
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "%s", pszContent3);
		}
		PubDisplayGen(4, "PLS REMOVE CARD");
	    PubUpdateWindow();
		PubKbHit();
		while (1)
		{
			if (PubGetCardStatus() == APP_FAIL)
			{
				break;
			}

			PubBeep(1);
			NDK_SysDelay(1);
		}	
	}
	else if ((NULL != pszContent2 && strcmp(pszContent2, "")) || (NULL != pszContent3 && strcmp(pszContent3, "")))
	{
		char szContent[64+1];
		
		memset(szContent, 0, sizeof(szContent));
		if(NULL != pszContent2)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "|C%s", pszContent2);
		}
		
		if(NULL != pszContent3)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent+strlen(szContent), "\n|C%s", pszContent3);
		}
		PubMsgDlg(pszTitle, szContent, 0, 1);
	}
	return APP_SUCC;
}

/**
* @brief Get Error Message form Config File with Error Code
* @return Message Content
*/
char *EmvErrMsg(int nErrCode)
{
	static char szMsg[100] = {0};

	GetEmvErrorCodeIni(nErrCode, szMsg);

	return szMsg;
}

/**
* @brief Search International organization Code
* @param in pRid RID
* @param out pOrg 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int EmvOrgSheet(const char *pRid, char *pOrg)
{
	if (memcmp(pRid,"\xa0\x00\x00\x00\x03",5) == 0)
	{
		memcpy(pOrg,"VIS",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x04",5) == 0)
	{
		memcpy(pOrg,"MAE",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x65",5) == 0)
	{
		memcpy(pOrg,"JCB",3);
		return APP_SUCC;
	}
	else if (memcmp(pRid,"\xa0\x00\x00\x00\x25",5) == 0)
	{
		memcpy(pOrg,"AMX",3);
		return APP_SUCC;
	}
	else
	{
		if (memcmp(pRid,"\xD1\x56\x00\x00\x01",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else if (memcmp(pRid,"\xa0\x00\x00\x03\x33",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else if (memcmp(pRid,"\xB0\x00\x00\x03\x33",5) == 0)
		{
			memcpy(pOrg,"CUP",3);
			return APP_SUCC;
		}
		else
		{
			memcpy(pOrg, "   ", 3);/**<unknow*/
		}
	}
	return APP_FAIL;
}

/**
* @brief Set Emv Merchant Name
* @param [in]?char *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/


int EmvSetMerchantName(uchar *psName)
{
	STCLSSPARAM stClssEPParam;
	int nRet = -1;
		
	memset(&stClssEPParam, 0, sizeof(STCLSSPARAM));
	nRet = SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_R);		
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Emv Config", "Get Config Fail", 3, 3);
		return APP_FAIL;
	}
	memset(stClssEPParam.usMerchantName, 0, sizeof(stClssEPParam.usMerchantName));
	memcpy(stClssEPParam.usMerchantName, psName, 20);
	if (APP_SUCC != SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_W))
	{
		PubMsgDlg("Emv Config", "Set Config Fail", 3, 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
* @brief Set Emv Ternimal ID
* @param [in]?char *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int EmvSetTerminalId(uchar *psId)
{
	STCLSSPARAM stClssEPParam;
	int nRet = -1;

	memset(&stClssEPParam, 0, sizeof(STCLSSPARAM));
	nRet = SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_R);		
TRACE(("+++ get aid: %d", nRet));	
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Emv Config", "Get Emv Config Fail", 3, 3);
		return APP_FAIL;
	}
	memset(stClssEPParam.usTerminalId, ' ', sizeof(stClssEPParam.usTerminalId));
	memcpy(stClssEPParam.usTerminalId, psId, 8);
	if (APP_SUCC != SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_W))
	{
		PubMsgDlg("Emv Config", "Set Emv Config Fail", 3, 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Set EMV Mid
* @param [in]?char *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/

int EmvSetMerchantId(uchar *psId)
{
	//emvparam stTermConfig;
	STCLSSPARAM stClssEPParam;
	int nRet = -1;
		
	memset(&stClssEPParam, 0, sizeof(STCLSSPARAM));
	nRet = SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_R);
TRACE(("+++ get 2aid: %d", nRet));	
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Emv Config", "Get Emv Config Fail", 3, 3);
		return APP_FAIL;
	}
	memset(stClssEPParam.usMerchantId, ' ', sizeof(stClssEPParam.usMerchantId));
	memcpy(stClssEPParam.usMerchantId, psId, 15);
	if (APP_SUCC != SDK_EP_OperAID(&stClssEPParam, AID_CONFIG_W))
	{
		PubMsgDlg("Emv Config", "Set Emv Config Fail", 3, 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}


/**
* @brief Contactless Power up
* @param [in]?char *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/

int IccRfPowerUp(int *pnCardNo)
{
	int nRet;
	
	nRet = PubCardPowerUp(NULL);
	if (pnCardNo != NULL)
	{
		*pnCardNo = 0;
	}

	return nRet;
}

/**
* @brief Contactless Power Down
* @param [in]?uchar *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int IccRfPowerDown(int cardno)
{
	char sUid[256+1] = {0};
	char cType = 0;
	char cStatus;

	if (PubGetICStatus(&cStatus) == APP_SUCC)
	{
		;
	}
	else if (PubGetCardStatus() == APP_SUCC)
	{
		PubBeep(1);
	}
		
	return PubCardPowerDown();
}


/**
* @brief Contactless Read and Write
* @param [in]?char *psName  
* @return
* @li APP_SUCC
* @li APP_FAIL
*/

int IccRfRw(int nCardNo, uchar *puszInBuf, int nInLen, uchar *pszOutBuf, int nOutLen)
{
	int nLen = 0;
	int nRet = APP_FAIL;

	nRet = PubCardrw(nInLen, (char *)puszInBuf, &nLen, (char *)pszOutBuf);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	return nLen; 

}

int IccRfBeep(ushort nDelayTime, uchar nTimes)
{
	if (YES == GetVarIsExRF() && YES == GetVarIsSupportRF())
	{
		return PubRfReaderBeep(nDelayTime, nTimes, 0);
	}
	else 
	{
		PubBeep(nTimes);
	//	NDK_SysTimeBeep(300, 500);
		return APP_SUCC;
	}
	
	return APP_FAIL;
}



static int gnIsSelectEcOrEmv = TRUE;/**<Check if need to select*/
static int gnIsDefaultEC	= FALSE;	/**<TRUE: Not Use EC*/
void SetnIsSelectEcOrEmv(int nFlag)
{
	gnIsSelectEcOrEmv = nFlag;
}

void SetnIsDefaultEC(int nFlag)
{
	gnIsDefaultEC = nFlag;
}
/**
* @brief Select EC Flow or Debit-credit Flow
* @return Return:			1		EC
*	   					    0		Normal
*	   				    	-1		Suspend
*	   				    	-3		Timeout
*/
int FcnSelectEcOrEmv(void)
{
	int nSelect = 0;
	int nRet;
	char *selMenu[] = {
		tr("0.NO"),
		tr("1.YES")
	};
	/**<Not Use EC*/
	if(gnIsDefaultEC)
	{
		gnIsDefaultEC = FALSE;
		return 0;
	}

	if (!gnIsSelectEcOrEmv)
	{
		gnIsSelectEcOrEmv = TRUE;
		return 1;
	}
	nRet = PubSelectYesOrNo(tr("SALE"), tr("IS USE EC"), selMenu, &nSelect);
	//nRet = PubSelectListItem("0.NO|1.YES", "SALE", "IS USE EC", 0xFF, &nSelect);
	if (nRet == APP_QUIT)
	{
		return -1;/**<Suspend*/
	}
	if (nRet != APP_SUCC)
	{
		return -3;
	}

	switch(nSelect)
	{
	case 0:
		return 0;
	case 1:
		return 1;
	default:
		break;
	}
	return -3;
}


/**
* @brief Save 9F26(Cryptograph First genarate In Transaction, use to send)
* @return Return:		
*/
int SaveData9F26RQ()
{
	memset(gs9F26_RQ, 0, sizeof(gs9F26_RQ));
	EmvEntryPointGetData(_EMV_TAG_9F26_IC_AC, (uchar *)gs9F26_RQ, sizeof(gs9F26_RQ));
	return APP_SUCC;
}

int GetData9F26RQ(char *pszInOutStr)
{
	if (NULL != pszInOutStr )
	{
		memcpy(pszInOutStr, gs9F26_RQ, sizeof(gs9F26_RQ));	
	}
	return APP_SUCC;
}

#if 0
/**
* @brief Check the chip card is EC card
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int CheckIsEC(void)
{
	int nRet = 0;
	uchar sPDOLBuf[200] = {0};
	
	nRet = EmvEntryPointGetData(_EMV_TAG_9F38_IC_PDOL, sPDOLBuf, sizeof(sPDOLBuf));
	TRACE(("9f38=%d", nRet));
	TRACE_HEX(((char*)sPDOLBuf, sizeof(sPDOLBuf),"9F38_IC_PDOL[%d]:", strlen((char *)sPDOLBuf)));
	if (nRet > 0)
	{
		nRet = EMVL2_ChkTagIsInDOL(0x9F7A, sPDOLBuf, nRet);
		TRACE(("0x9F7A=%d", nRet));
		if (nRet <= 0)
		{
			return APP_FAIL;
		}
	}
	
	return APP_SUCC;
}
#endif

/**
* @brief Check The Chip Card is Only support EC
* @param in cCheckFlag: TRUE
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int CheckIsOnlyEC()
{
	int nLen = 0;
	uchar sBuff[128] = {0};
	
	nLen = EmvEntryPointGetData(_EMV_TAG_4F_IC_AID, sBuff, sizeof(sBuff));
	
	TRACE(("_EMV_TAG_4F_IC_AID nLen=%d ",nLen));
	TRACE_HEX(((char *)sBuff, sizeof(sBuff), "4F[%d]:", strlen((char *)sBuff)));
	if (nLen > 5 && sBuff[nLen-1] == 0x06)
	{
		PubMsgDlg(NULL, "This Card Only Support EC\nREFUSE", 3, 3);
		return APP_FAIL;
	}		
	return APP_SUCC;
}

int ResetInputAmtInterface(void)
{
	gcEnInputAmtInterface = 1;
	return APP_SUCC;
}


/**
* Check the EC account is changed
*/
YESORNO CheckIsPay(void)
{
	char szApp[32+1] = {0};
	int nRet = -1;
	
	memset(szApp, 0, sizeof(szApp));
	EmvEntryPointGetData(_EMV_TAG_9F10_IC_ISSAPPDATA, (uchar *)szApp, sizeof(szApp));
	
	if(((szApp[4] & 0x10) && !(szApp[4] & 0x20))) 
	{
		nRet = EMV_ErrorCode();

		if (-1513 == nRet || -2117 == nRet) /**read last record Fail||offline data authentication Fail*/
		{
			return YES;
		}
	}

	return NO;
}


int _send_msg(entry_point_opt opt)
{
    return 0;
}

/**
* @brief paywave Get Pin
* @param in cType Pin Type  0x01－Offline Pin  0x02-Final Offline Pin 0x03 Online Pin
* @param in pszPin Clear Pin
* @return 
* @li -1 Fail  -2 Not Get Pin -3 Suspend > 0 Lenth of Pin
*/
int GetPinEntryPaywave(int cType, char * pszPin)
{
	char szPin[12+1] = {0};
	int nRet = 0, i;
	int nPinLen = 12;
	char szDispAmt[64+1] = {0};
	char szMsg[64] = {0};
	char *pszAmount = NULL;
	int nTimeOut = 120;
	char szRandKey[8+1] = {0};
	char szContent[100] = {0};
	char szPan[19+1]={0};
	char szTotal[14] = {0};

	memset(szPan,0,sizeof(szPan));

	ASSERT(GetPanForPaypass(szPan));
	GetVarPinpadTimeOut(&nTimeOut);

	AmtAddAmt((uchar *)gszAmount, (uchar *)gszCashAmount, (uchar *)szTotal);
	TRACE(("cType=%d pszPin=%s amount=%s gcTransType=%d", cType,pszPin,gszAmount,gcTransType));
	TRACE(("gszPan=%s", gszPan));
	TRACE(("szPan=%s", szPan));
	
	switch(cType)
	{
	case 0x01:
//		strcpy(szMsg, "Input Offline Pin:");
		strcpy(szMsg, "Please enter PIN:");
		break;
	case 0x02:
		strcpy(szMsg, "Final Offline Pin:");
		break;
	case 0x03:
		strcpy(szMsg, "Input Online Pin:");
		break;
	default:
		strcpy(szMsg, "Pls Input Pin:");
		break;
	}
	
	if ((gcTransType != TRANS_BALANCE))
	{
		pszAmount = szTotal;
	}
	/**
	* Show Amount
	*/
	memset(szDispAmt, 0, sizeof(szDispAmt));
	if (NULL != pszAmount)
	{
		ProAmtToDispOrPnt(pszAmount, szDispAmt);
	}

	memset(gszOnlinePin, 0, sizeof(gszOnlinePin));
	
	if (cType == 0x03)
	{
		TRACE_HEX((gszOnlinePin, 8, "gszOnlinePin is "));
		nRet = GetPin(szPan, pszAmount, gszOnlinePin);
		TRACE_HEX((gszOnlinePin, 8, "gszOnlinePin is "));
		if (nRet != APP_SUCC)
		{			
			if (nRet == APP_QUIT)
			{
				return -3;
			}
			return APP_FAIL;
		}
		if (memcmp(gszOnlinePin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8)  == 0)
		{	
			TRACE_HEX((gszOnlinePin, 8, "gszOnlinePin is "));
			return -2;
		}
		if (NULL != pszPin)
		{
			memcpy(pszPin, "123456", 6);/**<Return to Kernel , only to check*/
		}
		
		return 6;
	}

	if (YES == GetVarIsPinpad())
	{
		PubClear2To4();

		/**
		* Show Amount
		*/
		if (NULL != pszAmount)
		{
			PubDispPinPad(NULL, szDispAmt, NULL, NULL);
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT,2,"|R%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_NORMAL, 2, 1, szDispAmt);
		}

		PubDisplayStrInline(0, 3, szMsg);
		PubUpdateWindow();
		srand((unsigned) time (NULL));
		i = rand ();
		memcpy(szRandKey, &i, 4);
		memcpy(szRandKey+4, &i, 4);
		nRet =  SetPinpadKey(szRandKey, EMV_PINPAD_POS_KEYINDEX);
		if (nRet != APP_SUCC)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CSet Pinpad Key Fail");
			PubMsgDlg("Get Pin", szContent, 3, 1);
			PubClrPinPad();
			return APP_FAIL;
		}

		PubSetCurrentMainKeyIndex(EMV_PINPAD_POS_KEYINDEX);
#if 0
		nRet = PubActiveKey(DESMODE_3DES == GetVarEncyptMode() ? DESMODE_3DES : DESMODE_DES,\
			EMV_PINPAD_POS_KEYINDEX, KEY_TYPE_PIN);
		if (nRet != APP_SUCC)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CPinpad active Fail");
			PubMsgDlg("Get Pin", szContent, 3, 10);
			PubClrPinPad();
			return APP_FAIL;
		}
#endif
		memset(szPin, 0, sizeof(szPin));
		nRet = PubGetPin(szPin, &nPinLen, PINTYPE_WITHPAN, "0000000000000", szDispAmt, PIN_MAX_LEN, PIN_MIN_LEN);
		if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			PubClrPinPad();
			return -3;
		}
		if (nRet != APP_SUCC)
		{
			PubClrPinPad();
			return APP_FAIL;
		}
		if (memcmp(szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
		{
			PubClrPinPad();
			return -2;
		}
		nRet = DealPin(szPin, pszPin, szRandKey);
		return nRet;
	}
	else
	{
		while(1)
		{
			PubClear2To4();
			memset(szPin, 0, sizeof(szPin));
			ShowPinInputFormat(pszAmount);
			nPinLen = 0;
			nRet = PubInputxy(3, 5, szPin, &nPinLen, 0, PIN_MAX_LEN, nTimeOut, INPUT_MODE_PASSWD);
			if (nRet == APP_QUIT || nRet == APP_TIMEOUT)
			{
				return -3;
			}
			if (nRet != APP_SUCC)
			{
				return APP_FAIL;
			}
			if (nPinLen == 0)
			{
				return -2;
			}
			if (nPinLen < PIN_MIN_LEN)
			{
				continue;
			}
			break;
		}
		memcpy(pszPin, szPin, nPinLen);
		return nPinLen;
	}
	return APP_FAIL;
}

/**
* @brief 纯电子 现金卡不允许进行联机交易
* @param in cCheckFlag: TRUE进行检查
* @return
* @li APP_SUCC		
* @li APP_FAIL
*/
int GetEmvAcqType(int *pemAcqType)
{
	int nLen = 0;
	uchar sBuff[128] = {0};
	
	nLen = EmvEntryPointGetData(_EMV_TAG_4F_IC_AID, sBuff, sizeof(sBuff));
	if (nLen == 0)
	{
		return APP_FAIL;
	}
PubDebugData("AID: ", sBuff, nLen);	
	if (memcmp(sBuff, "\xA0\x00\x00\x06\x15", 5) == 0
		|| memcmp(sBuff, "\xD9\x99\x99\x99\x99", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_MCCS;
	}
	else if (memcmp(sBuff, "\xA0\x00\x00\x00\x03", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_VISA;
	}
	else if (memcmp(sBuff, "\xA0\x00\x00\x00\x65", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_JCB;
	}
	else if (memcmp(sBuff, "\xA0\x00\x00\x00\x04", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_MASTER;
	}
	else if (memcmp(sBuff, "\xA0\x00\x00\x00\x25", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_AMEX;
	}
	else if (memcmp(sBuff, "\xA0\x00\x00\x03\x33", 5) == 0)
	{
		*pemAcqType = EM_ACQTYPE_UPI;
	}
	else
	{
		return APP_FAIL;
	}
	return APP_SUCC;
}

char GetIsKeyInPIN()
{
	return gcIsKeyInPIN;
}

void SetIsKeyInPIN(char cValue)
{
	gcIsKeyInPIN = cValue;
}


