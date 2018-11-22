/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  ********************             
** File name:  card.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-3
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
* define global variable
*/
static char gszPan[MAX_PAN+1];
static char gszTk1[MAX_TK1+1];
static char gszTk2[MAX_TK2+1];
static char gszTk3[MAX_TK3+1];


/**
* internal function statement
*/
static void DispDefaultTxnTitle(void);
static int validTrack(const char *);


/**
** brief: Cardtask
** param [in]: nInPutType, The type of reading card that can be supported.See enum CARDINPUTMODE
** param [in]: nTimeOut, If =0 always wait the swipe task
** param [out]: pcType, The real type of reading card.see enum CARDINPUTMODE
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int SwipeCard(const int nInPutType, const uint nTimeOut, int *pnType)
{
	int nRet = 0, i = 0, nRes = 0;
	STREADCARDPARAM stReadCardParam;

	memset(&stReadCardParam, 0, sizeof(STREADCARDPARAM));
	NDK_ScrPush();
	while(1)
	{
		switch(nInPutType)
		{
		case INPUT_KEYIN:
			stReadCardParam.unFlag = CARD_KEYIN;
			break;	
		case INPUT_STRIPE:
			stReadCardParam.unFlag = CARD_STRIPE;
			break;
		case INPUT_INSERTIC_FORCE:
		case INPUT_INSERTIC:	
			stReadCardParam.unFlag = CARD_INSERTIC;	
			break;
		case INPUT_RFCARD:
			stReadCardParam.unFlag = CARD_RFCARD;
			break;
		case INPUT_UPCARD:
			stReadCardParam.unFlag = CARD_RFCARD;
			break;
		case INPUT_STRIPE_KEYIN:
			stReadCardParam.unFlag = CARD_STRIPE_KEYIN;
			break;
		case INPUT_STRIPE_INSERTIC:
			stReadCardParam.unFlag = CARD_STRIPE_INSERTIC;
			break;			
		case INPUT_STRIPE_RFCARD:
			stReadCardParam.unFlag = CARD_STRIPE_RFCARD;
			break;	
		case INPUT_STRIPE_KEYIN_INSERTIC:
			stReadCardParam.unFlag = CARD_STRIPE_KEYIN_INSERTIC;
			break;		
		case INPUT_STRIPE_KEYIN_RFCARD:
			stReadCardParam.unFlag = CARD_STRIPE_KEYIN_RFCARD;
			break;		
		case INPUT_STRIPE_KEYIN_INSERTIC_RFCARD:
			stReadCardParam.unFlag = CARD_STRIPE_KEYIN_INSERTIC_RFCARD;
			break;	
		case INPUT_STRIPE_INSERTIC_RFCARD:
			stReadCardParam.unFlag = CARD_STRIPE_INSERTIC_RFCARD;
			break;			
		case INPUT_KEYIN_INSERIC:
			stReadCardParam.unFlag = CARD_KEYIN_INSERIC;
			break;
		case INPUT_KEYIN_RFCARD:
			stReadCardParam.unFlag = CARD_KEYIN_RFCARD;
			break;	
		case INPUT_KEYIN_INSERIC_RFCARD:
			stReadCardParam.unFlag = CARD_KEYIN_INSERIC_RFCARD;
			break;
		case INPUT_INSERTIC_RFCARD:
			stReadCardParam.unFlag = CARD_INSERTIC_RFCARD;		
			break;		
		case INPUT_STRIPE_FALLBACK:
			stReadCardParam.unFlag = CARD_STRIPE;
			break;
		case INPUT_STRIPE_TRANSFER_IN:
			stReadCardParam.unFlag = CARD_STRIPE;
			break;
		case INPUT_STRIPE_TRANSFER_OUT:
			stReadCardParam.unFlag = CARD_STRIPE;
			break;
		case INPUT_STRIPE_INSERTIC_TRANSFER_OUT:
			stReadCardParam.unFlag = CARD_STRIPE|CARD_INSERTIC;
			break;
		default:
			return APP_QUIT;
			break;
		}					

		if(stReadCardParam.unFlag & CARD_RFCARD)
		{
			if (YES != GetVarIsSupportRF())
			{
				stReadCardParam.unFlag -= CARD_RFCARD;
			}
		}
		if(stReadCardParam.unFlag == 0)
			return APP_QUIT;

		if (nTimeOut == 0)
		{
			stReadCardParam.unFunctionKeyFlag = 1;
		}
		else
		{
			stReadCardParam.unFunctionKeyFlag = 0;
		}
		
		stReadCardParam.unInputMinLen = 13;		
		stReadCardParam.unInputMaxLen = 19;
		stReadCardParam.unTimeout = (nTimeOut == 0 ? 60 : nTimeOut);
		stReadCardParam.unSleepTime= stReadCardParam.unTimeout + 10;
		stReadCardParam.unMaintk = MAINTK2;

		memset(gszTk1, 0, sizeof(gszTk1));
		memset(gszTk2, 0, sizeof(gszTk2));
		memset(gszTk3, 0, sizeof(gszTk3));
		memset(gszPan, 0, sizeof(gszPan));
		nRet = PubPosReadCard(&stReadCardParam, gszTk1, gszTk2, gszTk3, &nRes);
		if(nRet == APP_SUCC)
		{
			gszTk1[MAX_TK1] = gszTk2[MAX_TK2] =  gszTk3[MAX_TK3] = 0;
			if (validTrack(gszTk2) == APP_FAIL || validTrack(gszTk3) == APP_FAIL || (nRes == CARD_STRIPE && strlen(gszTk2)< KJ_MIN_TK2)) 
			{
				nRet = APP_FAIL;
				nRes = APP_FAIL;
			}
		}

		if (nRet != APP_SUCC)
		{
			if (nRes >= 0xffffff08 && nRes <= 0xfffffffa)
			{			
				if (NULL != pnType)
				{
					*pnType = INPUT_INSERTIC;
				}
				return APP_SUCC;
						
			}
			switch(nRes)
			{
			case CARD_FUNCQUIT:
				if (NULL != pnType)
				{
					*pnType = gszTk2[0];
				}
				if(gszTk2[0] == KEY_ENTER )
				{
					return KEY_ENTER;
				}
				if (gszTk2[0] == KEY_ESC)
				{
					return APP_QUIT;
				}
				if(nTimeOut==0)
				{
					return gszTk2[0];
				}
				continue;
				break;
			case APP_TIMEOUT:
				return APP_TIMEOUT;
				break;
			case APP_QUIT:
				return APP_QUIT;
				break;
			default:
				if (nTimeOut == 0)
				{
					DispDefaultTxnTitle();
				}
				PubClear2To4();
				PubDisplayStrInline(0, 2, "SWIPE ERR");//
				PubDisplayStrInline(0, 3, "TRY AGAIN");
				PubDisplayTail(DISPLAY_ALIGN_BIGFONT, DISPLAY_MODE_CENTER, YES, "Quit Pls [Cancel]");
				PubUpdateWindow();
				nRet = PubGetKeyCode(1);
				if (nRet == KEY_ESC)
				{
					return APP_QUIT;
				}
				NDK_ScrPop();
				PubUpdateWindow();	
				continue;
			}
		}
		
		switch(nRes)
		{
		case CARD_INSERTIC:
			if (NULL != pnType)
			{
				*pnType = INPUT_INSERTIC;
			}
			return APP_SUCC;
			break;
		case CARD_STRIPE:
			if (NULL != pnType)
			{
				*pnType = INPUT_STRIPE;					
			}
			break;
		case CARD_RFCARD:
			if (NULL != pnType)
			{
				if(nInPutType & INPUT_RFCARD)
					*pnType = INPUT_RFCARD;
				else if(nInPutType & INPUT_UPCARD)
					*pnType = INPUT_UPCARD;
				else
					return APP_FAIL;
			}
			return APP_SUCC;
			break;
		case CARD_KEYIN:
			if (strlen(gszTk2) < MIN_PAN)
			{
				if (nTimeOut == 0)
				{
					DispDefaultTxnTitle();
				}
				PubClear2To4();
				PubDisplayStrInline(0, 2, "INVALID ENTER");
				PubDisplayStrInline(0, 3, "...");
				PubDisplayTail(DISPLAY_ALIGN_BIGFONT, DISPLAY_MODE_CENTER, YES, "Quit Pls [Cancel]");
				PubUpdateWindow();
				nRet = PubGetKeyCode(1);
				if (nRet == KEY_ESC)
				{
					return APP_QUIT;
				}
				NDK_ScrPop();
				PubUpdateWindow();
				continue;
			}
			if (NULL != pnType)
			{
				*pnType = INPUT_KEYIN;
			}
			memcpy(gszPan, gszTk2, strlen(gszTk2));
			return APP_SUCC;
			break;
		default:
			return APP_FAIL;
			break;
		}
		break;
	}
	if (0 != gszTk2[0])
	{
		for (i = 0; i < 37; i++)
		{
			if (gszTk2[i] == '=')
			{
				break;
			}
		}
		if (i == 37)
		{
			return APP_FAIL;
		}
		
		if (i > 19)
		{
			memcpy(gszPan, gszTk2, 19);
		}
		else
		{
			memcpy(gszPan, gszTk2, i);
		}
		return APP_SUCC;
	}
	else if (0 != gszTk3[0])
	{
		for (i = 0; i < 104; i++)
		{
			if (gszTk3[i] == '=')
			{
				break;
			}
		}
		if (i == 104)
		{
			return APP_FAIL;
		}
		
		if (i > 19)
		{
			memcpy(gszPan, gszTk3, 19);
		}
		else
		{
			memcpy(gszPan, gszTk3, i);
		}
		return APP_SUCC;
	}
	return APP_FAIL;
}

/**
** brief: Card task (show amount)
** param [in]: pszAmount  ,Amount of transaction
** param [in]: cInPutType ,Type of reading card supported，see enum EM_CARDINPUTMODE
** param [in]: nTimeOu，if =0 always wait the task
** param [out]: pnType, The reak type of reading card，see enum EM_CARDINPUTMODE
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int CardTaskAmt(const char *pszAmount, const int nInPutType, const uint nTimeOut, int *pnType)
{
	char szDispAmt[13+1] = {0};
	char cIsDisp = FALSE;

	PubClear2To4();
	
	if(pszAmount != NULL)
	{	
		ProAmtToDispOrPnt(pszAmount, szDispAmt);
		PubAllTrim(szDispAmt);
		if(nInPutType == INPUT_INSERTIC_RFCARD 
			|| nInPutType == INPUT_STRIPE_INSERTIC_RFCARD
			)
		{
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 2, "|C%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1,"INSERT/WAVE");//"请使用您的银行卡"						
			cIsDisp = TRUE;
		}
		else if (nInPutType == INPUT_RFCARD)
		{	
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 2, "|C%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1,"WAVE");//请挥卡
			cIsDisp = TRUE;			
		}
		else if(nInPutType == INPUT_INSERTIC)
		{
			//PubDispStrFormat(DISPLAY_ALIGN_BIGFONT, 2, "|C%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "%s RM", szDispAmt);
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1,"INSERT");//"请使用您的银行卡"
			cIsDisp = TRUE;
		}

	}	

	switch(nInPutType)
	{
	case INPUT_KEYIN:
		PubDisplayStr(DISPLAY_MODE_NORMAL, 3, 1, "INPUT:");//"请输入卡号:"
		break;	
	case INPUT_STRIPE:
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "SWIPE");//请刷卡-->
		break;
	case INPUT_INSERTIC_FORCE:
	case INPUT_INSERTIC:	
		if(!cIsDisp)
		{
			PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INSERT");
		}
		break;
	case INPUT_RFCARD:
		if(!cIsDisp)
		{
			PubDisplayLogo("wave100#61.jpg", 110, 50);
			//PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "WAVE");
		}
		break;
	case INPUT_STRIPE_KEYIN:
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "SWIPE/MANUAL");
		break;
	case INPUT_STRIPE_INSERTIC:
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INSERT/SWIPE");
		break;			
	case INPUT_STRIPE_RFCARD:
		PubDisplayLogo("wave100#61.jpg", 110, 48);
		PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "SWIPE/WAVE");
		break;	
	case INPUT_STRIPE_KEYIN_INSERTIC:
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INSERT/SWIPE/MANUAL");
		break;		
	case INPUT_STRIPE_KEYIN_RFCARD:
		PubDisplayLogo("wave100#61.jpg", 110, 48);
		PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "MANUAL/SWIPE/WAVE");
		break;		
	case INPUT_STRIPE_KEYIN_INSERTIC_RFCARD:
		PubDisplayLogo("wave100#61.jpg", 10, 48);
		PubDisplayStr(DISPLAY_MODE_NORMAL, 2, 1, "MANUAL/WAVE/");
		PubDisplayStr(DISPLAY_MODE_NORMAL, 3, 1, "SWIPE/INSERT");
		break;	
	case INPUT_STRIPE_INSERTIC_RFCARD:
		if(!cIsDisp)
		{
			PubDisplayLogo("wave100#61.jpg", 110, 48);
			PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "INSERT/SWIPE/WAVE");
		}
		break;			
	case INPUT_KEYIN_INSERIC:
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "INSERT/MANUAL");
		break;
	case INPUT_KEYIN_RFCARD:
		PubDisplayLogo("wave100#61.jpg", 110, 48);
		PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "WAVE/MANUAL");
		break;	
	case INPUT_KEYIN_INSERIC_RFCARD:
		PubDisplayLogo("wave100#61.jpg", 110, 48);
		PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "INSERT/WAVE/MANUAL");
		break;
	case INPUT_INSERTIC_RFCARD:
		if(!cIsDisp)
		{
			PubDisplayLogo("wave100#61.jpg", 110, 48);
			PubDisplayStr(DISPLAY_MODE_CENTER, 4, 1, "INSERT/WAVE");					
		}	
		break;		
	case INPUT_STRIPE_FALLBACK:
		PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "READ CHIP ERR");
		PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "SWIPE");
		PubDisplayTail(DISPLAY_ALIGN_BIGFONT, DISPLAY_MODE_CENTER, YES, "[CANCEL] To Quit");
		break;
	default:
		return APP_QUIT;
		break;
	}					
	PubUpdateWindow();
	
	return SwipeCard(nInPutType, nTimeOut, pnType);
}

/**
** brief: Get card number
** param [in]: 
** param [out]: pszPan, card number
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int GetPan(char *pszPan)
{	
	if (NULL == pszPan)
	{
		return APP_FAIL;
	}
	else
	{
		memcpy(pszPan, gszPan, MAX_PAN);
	}
	return APP_SUCC;
}

/**
** brief: Get track message 
** param [out]: pszTk1
** param [out]: pszTk2
** param [out]: pszTk3
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int GetTrack(char *pszTk1, char *pszTk2, char *pszTk3)
{
	if (NULL != pszTk1)
	{
		memcpy(pszTk1, gszTk1, MAX_TK1);
	}
	if (NULL != pszTk2)
	{
		memcpy(pszTk2, gszTk2, MAX_TK2);
	}
	if (NULL != pszTk3)
	{
		memcpy(pszTk3, gszTk3, MAX_TK3);
	}

	return APP_SUCC;
}

/**
** brief: Show card number
** param [in]: pszPan 
** param [in]: nTimeOut 
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int DispPan(const char *pszPan, int nTimeOut)
{
	char szMsg[64+1];
	char szTmp[64] = {0};
	
	PubClear2To4();
	strcpy(szTmp, pszPan);
	MaskCardNo(szTmp, 1);
	sprintf(szMsg, "PAN: \n%s\n", szTmp);//卡号
	ASSERT_FAIL(PubConfirmDlg(NULL, szMsg, 0, nTimeOut));
	
	return APP_SUCC;
}

/**
** brief: Check if it is Chip
** param [in]: pszTk2 
** param [out]: pcSerCode
** return: 
** li: APP_SUCC: it is Chip
** li: APP_FAIL: not chip
** auther: 
** date: 2016-7-3
** modify: 
*/
int ChkIsICC(const char *pszTk2, char *pcSerCode)
{
	int i;
	char szContent[50] = {0};

	for(i = 0; i < 38; i++)
	{
		if ('=' == *(pszTk2+i))
		{
			break;
		}
	}
	if (38 == i)
	{
		return APP_FAIL;
	}
	if (('2' == *(pszTk2+i+5)) || ('6' == *(pszTk2+i+5)))
	{
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CCHIP CARD");
		PubMsgDlg(NULL, szContent, 0, 1);

		if (NULL != pcSerCode)
		{
			*pcSerCode = (*(pszTk2+i+5));
		}
		return APP_SUCC;
	}
	return APP_FAIL;
}

/**
** brief: Close default screen
** param: void
** return: void
** auther: 
** date: 2016-7-3
** modify: 
*/
void DisableDispDefault(void)
{
	PubClearAll();
	ShowLightOff();
	SetStatusBar(STATUSBAR_STANDBY_CLOSE);
	return ;
}

/**
** brief: show default screen title
** param: void
** return: void
** auther: 
** date: 2016-7-3
** modify: 
*/
static void DispDefaultTxnTitle(void)
{
	char cDefaultType;
	char *pszTitle;
	
	GetVarDefaultTransType(&cDefaultType);
	if ('1' == cDefaultType)
	{
		pszTitle = "SALE";
	}
	else
	{
		pszTitle = "PREAUTH";
	}
	PubDisplayTitle(pszTitle);
	return ;
}

/**
** brief: Check if track is valid
** param [in]: pszTk
** param [out]: 
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
static int validTrack(const char *pszTk)
{
	int i;

	for(i=0; i<strlen(pszTk); i++)
	{
		if ( (pszTk[i] < '0' || pszTk[i] > '9') && pszTk[i] != '=' )
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

/**
** brief:  Shield section of card number,replace with *
** param [in]: uchar *pCardPan, 	  original card number
** param [in]: const uint nPanLen,    len of card number
** param [in]: const uchar cTransType  
** param [in]: const uint  nMode,   EMV status
** param [out]: uchar *pCardPan,    The card number after dealing
** return: 
** li: APP_SUCC
** li: APP_FAIL
** auther: 
** date: 2016-7-3
** modify: 
*/
int CtrlCardPan(uchar *pCardPan, const uint nPanLen, const uchar cTransType, const uint nMode)
{
	int i = 0;
	
	if (NULL == pCardPan)
	{
		return APP_FAIL;
	}

	if (19 < nPanLen || 12 > nPanLen) 
	{
		return APP_FAIL;
	}

	switch (cTransType) 
	{
	case TRANS_SALE:
		if (nMode == EMV_STATUS_OFFLINE_SUCC || nMode == EMV_STATUS_OFFLINE_FAIL)
		{
			/**<off line trans not shield*/
			return APP_QUIT;
		}
		break;
	case TRANS_VOID:
	case TRANS_REFUND:
	case TRANS_VOID_REFUND:
	case TRANS_VOID_OFFLINE:
	case TRANS_VOID_PREAUTH:
	case TRANS_AUTHCOMP:
	case TRANS_AUTHSALEOFF:
	case TRANS_VOID_AUTHSALE:
		break;
	case TRANS_PREAUTH:
		if(YES != GetVarIsPreauthShieldPan())
		{
			return APP_QUIT;
		}
		break;
	default:
		return APP_QUIT;
	}

	if (YES != GetVarIsShieldPan())
	{
		return APP_QUIT;
	}

	//memset(pCardPan + 6, '*', nPanLen - 6 - 4);
	for(i = 0; i<(nPanLen-4);i++)//shou last 4 letters
	{
		pCardPan[i] = '*';
	}
	

	return APP_SUCC;
}

/**
* @brief get name from Track1
* @param [out] pszTk1
* @param [out] pszName 
* @return 
* @li APP_FAIL
* @li APP_SUCC 
* @author 
* @date 
*/
int GetNameFromTk1(const char *pszTk1, char *pszName)
{
	int i,j;
	int nMaxNameLen = 26;
	int nLen;

	for(i=0; i<strlen(pszTk1); i++)
	{
		if (pszTk1[i] == '^' )
		{
			i++;
			break;
		}
		
	}
	for(j=i; j<strlen(pszTk1);j++)
	{
		if (pszTk1[j] == '^' )
		{
			break;
		}
	}
	nLen = j-i;
	if((nLen<=nMaxNameLen)&&(nLen > 1))
	{
		memcpy(pszName, &pszTk1[i], nLen);
		return APP_SUCC;
	}	
	return APP_FAIL;
}

/**
** brief: Show default screen
** param: void
** return: void
** auther: 
** date: 2016-7-4
** modify: 
*/
void EnableDispDefault(void)
{
	char szShowInfo[16+1]={0};
	char szInquiryNo[32+1] = {0};
	char szCentreNo[32+1] = {0};
	char szTmp1[132+1] = {0};
	char szTmp2[132+1] = {0};
	//open status bar
	SetStatusBar(STATUSBAR_STANDBY_OPEN);

	if(GetVarIsSwipe() == YES)	
	{	
		strcpy(szShowInfo, "SWIPE/INSERT");//"请刷/插卡"
	}
	else
	{
		strcpy(szShowInfo, "INSERT");
	}
	PubClearAll();
	PubDisplayLogo("320#148.jpg", 0, 0);

	NDK_ScrSetFontColor(0x00, FONTCOLOR_NORMAL);//0x3384
	NDK_ScrSetFontType(DISPFONT_CUSTOM);
//	NDK_ScrDispString(10, 160, "Payment Inquiry:      03-2074 8787", 0);
//	NDK_ScrDispString(10, 185, "Authorization Centre: 03-2056 8888", 0);
	GetVarCenterNo(szCentreNo);
	GetVarInquiryNo(szInquiryNo);
	sprintf(szTmp1, "Payment Inquiry: %s", szInquiryNo);
	sprintf(szTmp2, "Authorization Centre: %s", szCentreNo);
	NDK_ScrDispString(10, 160, szTmp1, 0);
	NDK_ScrDispString(10, 185, szTmp2, 0);
	PubUpdateWindow();
	
	ShowLightIdle();
	return;
}

/**
* @brief 从二磁道获取有效期
* @param [in] pszTk2 二磁道指针
* @param [out] pszExpDate 有效期指针
* @return 
* @li APP_FAIL
* @li APP_SUCC 
* @author 
* @date 
*/
int GetExpDateFromTk2(const char *pszTk2, char *pszExpDate)
{
	int i;

	for(i=0; i<strlen(pszTk2); i++)
	{
		if (pszTk2[i] == '=' || pszTk2[i] == 'D')
		{
			if (NULL == pszExpDate)
			{
				return APP_FAIL;
			}
			memcpy(pszExpDate, &pszTk2[i+1], 4);
			return APP_SUCC;
		}
	}
	return APP_FAIL;
}

int GetAcqTypeFormPan(const char *pszTitle, const char *pszPan, int *pnAcqType, char cAttr)
{
	int nSel = 0xFF;	
	char szStr[128] = {0};
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems1[] = {
		tr("1.VISA"),
		tr("2.MYDEBIT")
	};
	char *pszItems2[] = {
		tr("1.MASTER"),
		tr("2.MYDEBIT")
	};
	
	if(memcmp(pszPan,"4000000000",10)>=0&&memcmp(pszPan,"4999999999",10)<=0)
	{
		*pnAcqType = EM_ACQTYPE_VISA;
	}
	else if((memcmp(pszPan,"5000000000",10)>=0&&memcmp(pszPan,"5999999999",10)<=0)
		||(memcmp(pszPan,"2221000000",10)>=0&&memcmp(pszPan,"2720999999",10)<=0))
	{
		*pnAcqType = EM_ACQTYPE_MASTER;
	}
	else if(memcmp(pszPan,"3500000000",10)>=0&&memcmp(pszPan,"3599999999",10)<=0)
	{
		*pnAcqType = EM_ACQTYPE_JCB;
	}
	else if(memcmp(pszPan,"34",2)==0||memcmp(pszPan,"37",2)==0)
	{
		*pnAcqType = EM_ACQTYPE_AMEX;
	}
	else 
	{
		*pnAcqType = EM_ACQTYPE_UPI;
	}
	
	if(*pnAcqType!=EM_ACQTYPE_VISA && *pnAcqType!=EM_ACQTYPE_MASTER)
	{
		return APP_SUCC;
	}

	if (cAttr == ATTR_MANUAL)//by manaul only go VISA/MASTER
	{
		return APP_SUCC;
	}
	
	if (*pnAcqType == EM_ACQTYPE_VISA)
	{
		strcpy(szStr, "1.VISA||2.MYDEBIT");
		nRet = PubShowMenuItems(pszTitle, pszItems1, sizeof(pszItems1)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		if(2 == nSelcItem)
		{
			*pnAcqType = EM_ACQTYPE_MCCS;
		}
	}
	else
	{
		strcpy(szStr, "1.MASTER||2.MYDEBIT");
	}	
	return APP_SUCC;;	
}


