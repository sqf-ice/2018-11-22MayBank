/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  ********************             
** File name:  main.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-1
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
/**
* internal function statement
*/
static int AppTrans(void);
static int MenuFuncSel(void);
static int MenuDefault(void);
static int FirstRunChk(void);
static int AppInit(void);
static int ChkPdAndRF(void);

#ifdef DEMO
static int LoadKey()
{
	int nRet = 0, nIndex = 0;
	char sTmk[16] = {0};
	char sPinKey[16] = {0};
	char sMacKey[16] = {0};
	char sDataKey[16] = {0};
	char sIPEK[16] = {0xBD,0x35,0x71,0xDB,0x88,0x36,0xCD,0x71,0xD4,0xE4,0x37,0x47,0x35,0x4E,0xC2,0x36};
	char sKSN[10] = {0x00,0x01,0x03,0x00,0x64,0x56,0x00,0xE0,0x00,0x00}; 
	char sInput[8] = {0}, sOutput[8] = {0}; //Used to veryfiy DUKPT key. It's like to calculate key check value.
	
	memset(sTmk, 0x00, sizeof(sTmk));
	memset(sPinKey, 0x11, 16);	//kcv "\xCA\x25\x1B\x79"
	memset(sMacKey, 0x22, 16);	//kcv "\x10\x82\x38\x76"
	memset(sDataKey, 0x33, 16);	//kcv "\xD9\x7E\xB4\x64"

	GetVarMainKeyNo(&nIndex);

	if(GetVarKeySystemType() == '0')	//MK/SK
	{
		nRet = PubLoadMainKey(nIndex, sTmk, NULL, 16);//sTmk(main key) should be plain text
		if (nRet != APP_SUCC)
		{
			PubDispErr("LOAD MAINKEY FAIL");
			return APP_FAIL;
		}
		
		PubSetCurrentMainKeyIndex(nIndex);
		//sPinKey(work key) should be ciphertext. It you want to inject 32byte 0x11, 
		//you should calculate the ciphertext of 32byte 0x11 (encrypt 32byte 0x11 by sTmk (32 bytes 0x00)), then inject ciphertext to terminal.
		nRet = PubLoadWorkKey(KEY_TYPE_PIN, sPinKey, 16, NULL);
		if (nRet != APP_SUCC)
		{
			PubDispErr(tr("Load PIN Key Fail"));
			return APP_FAIL;
		}
		//sMacKey(work key) should be ciphertext. Same steps as pin key.
		nRet = PubLoadWorkKey(KEY_TYPE_MAC, sMacKey, 16, NULL);
		if (nRet != APP_SUCC)
		{
			PubDispErr(tr("Load MAC Key Fail"));
			return APP_FAIL;
		}
		//sDataKey(work key) should be ciphertext. Same steps as pin key.
		nRet = PubLoadWorkKey(KEY_TYPE_DATA, sDataKey, 16, NULL);
		if (nRet != APP_SUCC)
		{
			PubDispErr(tr("Load Data Key Fail"));
			return APP_FAIL;
		}
	}
	else	//DUKPT
	{
		nRet = PubLoadMainKey(nIndex, sIPEK, sKSN, 16);//sIPEK should be plain text
		if (nRet != APP_SUCC)
		{
			PubDispErr("LOAD MAINKEY FAIL");
			return APP_FAIL;
		}
		PubGetDukptKSN(sKSN);
		TRACE_HEX((sKSN, 10, "sKSN: "));
		PubSetCurrentMainKeyIndex(nIndex);
		PubDes3(sInput, 8, sOutput);
		TRACE_HEX((sOutput, 8, "sOutput: "));
	}
	return APP_SUCC;
}
#endif


/**
** brief: Project entry
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
int main(void)
{
	STUIFUN_STYLE stUiFunStyle;
	
	PubInitUiParam();
	memset(&stUiFunStyle, 0, sizeof(STUIFUN_STYLE));
	stUiFunStyle.emMenu = 0;
	strcpy(stUiFunStyle.szPageUp, "->");
	strcpy(stUiFunStyle.szPageDown, "<-");
	PubSetUiStyle(stUiFunStyle);
	PubClearAll();	
	PubKbHit();					/*clear key cache*/
	/**
	* check version(First run check, only run one time)
	*/
	FirstRunChk();

	/**
	*module initialize , important !!!
	*/
	AppInit();
	TRACE(("[%d]size system=%d water=%d reversal=%d %d max=%d", sizeof(STEMVADDTIONWATERINFO), sizeof(STSYSTEM), sizeof(STTAANSRECORD), 
		sizeof(STREVERSAL),fReversalData_len,fNVR_Addr_End));
	TRACE(("size STSETTLE=%d,[%d],[%d]", sizeof(STSETTLE), sizeof(STAPPCOMMPARAM), sizeof(STAPPPOSPARAM)));
		
	SetControlChkPinpad(YES);

	if (APP_SUCC != IsFirstRun())
	{
		SetAppCheckValue();
	}

	PubSetDebugPort(PORT_NUM_COM1);
	PubSetDebugMode(DEBUG_PORT);
	PubSetDebugLevel(DEBUG_LEVEL_NORMAL);
	/*
	* Run task
	*/
	while(1)
	{
		/**
		* device checking
		*/
		ChkPdAndRF();
				
		/**
		* Communication Initialize
		*/
		
		CommInit();
		/**
		* Enter default screen
		*/
		if (APP_QUIT == AppTrans())
		{
			CommDump();
			SetStatusBar(STATUSBAR_OTHER_CLOSE);		
			return APP_QUIT;
		}
	}
}

/**
** brief: Run application firstly and initialize
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int FirstRunChk(void)
{
	if (APP_SUCC != IsFirstRun())
	{
		/**
		* Import default parameter
		*/
		PubDisplayGen(2, "INITIALIZE");
		PubDisplayGen(3, "PLS WAIT....");
		PubUpdateWindow();
		PubDelFile(KERNEL_NAME);
		PubDelFile(EMVFILE_TERMCAPK);	
		InitEMVParam();
		if (NDK_FsExist(FILE_APPCOMMPARAM) != NDK_OK)
		{
			InitCommParam();
			InitAllBatch();
			ClearAllSettle();						/**<clear settle date*/
			PubDeReclFile(SETTLEFILENAME);
			EmvClearWater();
			InitLastSettleFile();
			ASSERT_FAIL(InitPosDefaultParam());	
		}
		
		DownEmvParam();
	}
	return APP_SUCC;
}

/**
** brief: Application initialize
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int AppInit(void)
{
	int nRet = 0;

	PubClearAll();
	PubDisplayStrInline(0, 2, "Loading...");
	PubDisplayStrInline(0, 4, "Please wait...");
	PubUpdateWindow();
	nRet = InitISO8583();
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Warning", "Exporting Iso definition fail", 3, 10);
	}
	nRet = ExportPosParam();
	nRet += ExportCommParam();
	if (APP_SUCC != nRet)
	{
		PubMsgDlg("Warning", "POS param loses", 3, 10);
	}

	if (ExportEMVModule() != APP_SUCC)
	{
		return APP_FAIL;
	}
	SetCustomizedFont(GetVarSystemLanguage());
	SetParamFromIni();
	ParseParamFromTMS();
	return nRet;
}

/**
** brief: Enter into interface of transaction 
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int AppTrans(void)
{ 
	int nRet = 0;
	char cMenuSel = 0xff;			/*Enter default screen*/

	DealSettleTask();//deal settle
	
	while(1)
	{	
		if (cMenuSel == 0)
		{
			/**
			* Enter main menu
			*/
			nRet = MenuFuncSel();
			if (nRet == KEY_FUNC_APP_SELECT)
			{
				return APP_QUIT;
			}
			if (nRet == APP_SUCC)
			{
				continue;
			}
		}
		else
		{
			/**
			* Enter default menu
			*/
			if (MenuDefault() == KEY_FUNC_APP_SELECT)
			{
				return APP_QUIT;
			}
		}
		cMenuSel = ~cMenuSel;
	}
	return APP_SUCC;
}

typedef enum
{
	EM_MENU_SALE = 1,
	EM_MENU_VOID,
	EM_MENU_INSTALLMENT,
	EM_MENU_SETTLE,
	EM_MENU_REPRINT,
	EM_MENU_REPORT,
	EM_MENU_BALANCE,
	EM_MENU_CASHBACK,
	EM_MENU_PREAUTH,
	EM_MENU_OFFLINE,
	EM_MENU_CASHADVANCE,

	EM_MENU_ORS,
	EM_MENU_BARCODE,
	EM_MENU_PAYBYREADER,
	
	
}EM_MAINMENU;


static int MenuORS()
{
	int nRet;
	int nSelect=0xff;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.Points Inquiry"),
		tr("2.Instant Reward"),
		tr("3.Points RDMPT"),
		tr("4.Value RDMPT"),
		tr("5.Gift Code RDMPT"),
		tr("6.Hot Deal RDMPT")
	};
	while (1)
	{
		nRet = PubShowMenuItems("ORS", pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			OrsPointsInquiry();
			break;
		case 2:
			OrsInstantReward();
			break;
		case 3:
			OrsRedeem(TRANS_ORS_POINTS_RDMPT);
			break;
		case 4:
			OrsRedeem(TRANS_ORS_VALUE_RDMPT);
			break;
		case 5:
			OrsRedeem(TRANS_ORS_GIFTCODE_RDMPT);
			break;
		case 6:
			OrsRedeem(TRANS_ORS_HOTDEAL_RDMPT);
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
** brief: Main menu of transaction
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int MenuFuncSel(void)
{
 	int nTrueInput = INPUT_NO;
    int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.PAYWAVE"),
		tr("2.SALE"),
		tr("3.VOID"),
		tr("4.EPP"),
		tr("5.SETTLE"),
		tr("6.REPRINT"),
		tr("7.REPORT"),
		tr("8.BALANCE"),
		tr("9.CASHBACK"),
		tr("10.PREAUTH"),
		tr("11.REFUND"),
		tr("12.OFFLINE"),
		tr("13.CASH ADVANCE"),
		tr("14.ORS"),
		tr("15.BARCODE"),
		tr("16.QUIT")
	}; 

	TRACE(("MenuFuncSel"));
	while (1)
	{	
	    nRet = PubShowMenuItems(tr("MAYBANK"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);	
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelcItem)
		{
		case 1:
			PayByReader();
			break;
		case 2:
			EmvTxnSale(&nTrueInput, 0);
			DISP_OUT_ICC;
			break;
		case 3:
			VoidSale(); 
			DISP_OUT_ICC;
			break;
		case 4:
			EmvTxnInstallment();
			DISP_OUT_ICC;
			break;
		case 5:
			Settle(0);
			DISP_OUT_ICC;
			break;	
		case 6:
			DoPrint();
			break;			
		case 7:
			DoReport();
			break;
		case 8:
			EmvTxnBalance();
			break;
		case 9:
			{	
				char sTmp[6] = {0};
				
				UpdateAmtLimit(MCCS_ENB, "\x00\x00\x00\x00\x00\x00", NULL, sTmp);
PubDebugData("Tmp; ", sTmp, 6);
				EmvTxnSale(&nTrueInput, 1);//cashback
				UpdateAmtLimit(MCCS_ENB, sTmp, NULL, NULL);
			}
			DISP_OUT_ICC;
			break;
		case 10:
			EmvTxnPreAuth(&nTrueInput);
			DISP_OUT_ICC;
			break;
		case 11:
			break;
		case 12:
			OfflineSale();
			DISP_OUT_ICC;
			break;
		case 13:
			EmvTxnCashAdvance();
			DISP_OUT_ICC;
			break;
		case 14:
			MenuORS();
			break;
		case 15:
			BarcodePayment();
			break;
		case 16:
			return KEY_FUNC_APP_SELECT;
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
** brief: Default interface
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int MenuDefault(void)
{
	int nRet = 0;
	int nTrueInputMode = INPUT_NO;
	int nFirstsInput = INPUT_INSERTIC;
	int nSleepCount = 0; 
	int nSleepTime = 60;/**If idle for 60 second, Sleep*/
	int nRecordNum = 0;
 	int nTrueInput = INPUT_NO;
	char cDefaultTransType;
	STTAANSRECORD stTransRecord;
	int nKey = 0;

	GetVarDefaultTransType(&cDefaultTransType);
	if (YES == GetVarIsSwipe())
	{
		nFirstsInput |= INPUT_STRIPE;
	}
	while(1)
	{
		nTrueInput = INPUT_NO;
		nRecordNum = 0;
		nSleepCount = 0;
		
		CheckAutoReboot();
		
		EnableDispDefault();
		nRet = SwipeCard(nFirstsInput, 0, &nTrueInputMode);
		if((nRet >= KEY_1 && nRet <= KEY_8) 
			|| (nRet >= KEY_F1 && nRet <= KEY_F2)
			|| nRet == KEY_ENTER 
		    || nRet == APP_SUCC 
		    || nRet == APP_QUIT)
		{
			DisableDispDefault();
		}
		
		switch(nRet)
		{
		case KEY_BACK:
			nKey =  PubGetKeyCode(1);
			switch (nKey)
			{
			case KEY_1:
				Version();
				break;
			case KEY_3:
				if (APP_SUCC == ProCheckPwd("CLEAR REVERSAL", EM_TRANS_PWD))
				{
					if( PubConfirmDlg("CLEAR REVERSAL", "CONFIRM?", 0, 0) == APP_SUCC)
					{
						ClearRevFlag(); 
					}
				}
				break;
			case KEY_4:
				if (APP_SUCC == PubConfirmDlg("RESET EMV ","CONFIRM?", 0, 10))
				{
					PubMsgDlg("RESET EMV ","PROCESSING...", 0, 1);
					PubDelFile(KERNEL_NAME);
					PubDelFile(EMVFILE_TERMCAPK);	
					InitEMVParam();
					DownEmvParam();
					ASSERT_QUIT(ExportEMVModule());
					PubMsgDlg("RESET EMV ","OK", 1, 1);

				}
				break;
			case KEY_6:
				if (APP_SUCC == ProCheckPwd("OTHER CONTROL", EM_SHOCT_PWD))
					SetFunctionIsCashRegister();
				break;
			case KEY_7:
				if (APP_SUCC == ProCheckPwd("SET COMM", EM_SHOCT_PWD))
					SetEthParam();
				break;
			case KEY_8:
				if (APP_SUCC == ProCheckPwd("SET COMM", EM_SHOCT_PWD))
					SetFuncCommType(YES);
				break;
			case KEY_9:
				if (APP_SUCC == ProCheckPwd("SET COMM", EM_SHOCT_PWD))
					SetWifiParam();
				break;
			case KEY_0:
				PubDisplayTitle("REVERSAL");
				DoReversal();
				break;
			default:
				break;
			}
			break;
		case KEY_9:
			break;
		case KEY_0:
			break;
		case KEY_F1:
		case KEY_F2:
			SystemManage();
			break;
		case APP_QUIT:
			return APP_SUCC;//Come back to main screen
			break;
		case KEY_ENTER:
			nTrueInputMode = INPUT_NO;
			//EmvTxnSale(&nTrueInputMode, 0);
			PayByReader();
			break;
		case KEY_MENU:
			break;
		case APP_TIMEOUT:
			/**rf blue led show(if the led exists)*/
			ShowLightIdle();
			/**go to sleep*/
			nSleepCount++;
			if (nSleepCount > (nSleepTime/3)) /**cardtask timeout is 5 second*/
			{
				nSleepCount = 0;
				GotoSleepApp();
			}
			break;
		default:
			break;
		}
		if (nRet == APP_SUCC)
		{
			if ('1' == cDefaultTransType)/**<'1'-SALE£¬'0'-PREAUTH*/
			{
				EmvTxnSale(&nTrueInputMode, 0);
			}
			else
			{
				EmvTxnPreAuth(&nTrueInputMode);
			}
			DISP_OUT_ICC;
		}
	}
}

/**
** brief: Detect pinpad
** param [in]: 
** param [out]: 
** return: 
** auther: sunh
** date: 2016-7-1
** modify: 
*/
static int ChkPdAndRF(void)
{
	/**
	* Check pinpad£»initialize the secure module
	*/
	ChkPinpad();
	/**
	* check rf reader
	*/
	ChkRF();

	return APP_SUCC;
}

