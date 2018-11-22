/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  tool.c
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

/**
* @brief Asicc Amount to unsigned long long
* @param in pszAmt 
* @return 
* @li the result amount
*/
unsigned long long AtoLL(const char *pszAmt)
{
	char szTmpAmt[12+1] = {0};
	int i;
	unsigned long long ullAmount = 0;
	
	memcpy(szTmpAmt, pszAmt, 12);
	PubLeftTrim(szTmpAmt);
	
	for (i = 0; i< strlen(szTmpAmt); i++)
	{
		if (PubIsDigitChar(szTmpAmt[i]) == APP_SUCC)
		{
			ullAmount = ullAmount * 10 + (szTmpAmt[i] - '0');
		}
		else
		{
			break;
		}
	}
	return ullAmount;
}

//Delete all the '0' in left, If all the string letter is '0', Leave one '0'
void LeftTrimZero(char *pszSrc)
{
	PubLeftTrimChar((uchar *)pszSrc, '0');
	if(pszSrc[0] == 0)
	{
		pszSrc[0] = '0';
		pszSrc[1] = 0;
	}
}

int AscBigCmpAsc(const char *pszNum1,const char *pszNum2)
{
	int nLen1=0,nLen2=0;
	char szNum1[100+1], szNum2[100+1];
	strcpy(szNum1, pszNum1);
	strcpy(szNum2, pszNum2);
	LeftTrimZero(szNum1);
	LeftTrimZero(szNum2);
	
	nLen1=strlen(szNum1);
	nLen2=strlen(szNum2);
	if (nLen1 == nLen2)
		return strcmp(szNum1,szNum2);
	else if (nLen1 > nLen2)
		return 1;
	else
		return -1;
}


/**
* @brief Amout format change to display,like 000000000010 to 0,10
* @param in pszInAmt 
* @param out pszOutAmt 
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int ProAmtToDispOrPnt(const char *pszInAmt, char *pszOutAmt)
{
	int i;
	char szAmt[12+1] = {0}, szTempAmt[12+1] = {0};
	
	if (NULL == pszInAmt ||NULL == pszOutAmt)
	{
		return APP_FAIL;
	}
	if (memcmp(pszInAmt, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12) == 0)
	{
		//return APP_FAIL;
	}
	memcpy(szAmt, pszInAmt, 12);
	PubAllTrim(szAmt);
	if (0 == strlen(szAmt))
	{
		return APP_FAIL;
	}
	
	PubAddSymbolToStr(szAmt,12,'0',ADDCH_MODE_BEFORE_STRING);
	strcpy(szTempAmt, szAmt);
	for (i = 0; i < strlen(szAmt); i++)
	{
		if (PubIsDigitChar(szAmt[i]) == APP_FAIL)
		{
			return APP_FAIL;
		}
	}
	memset(szAmt, 0, sizeof(szAmt));
	memcpy(szAmt, szTempAmt, 10);
	PubAllTrim(szAmt);
	if ((strlen(szAmt) > 9) && (szAmt[0] != '0'))  
	{
		sprintf(pszOutAmt, "%s.%02d", szAmt, atoi(szTempAmt + 10));
	}
	else
	{
		sprintf(pszOutAmt, "%ld.%02d", atol(szAmt), atoi(szTempAmt + 10));
		PubAddSymbolToStr(pszOutAmt, 13, ' ', 0);
	}
	return APP_SUCC;
}

#if 0
int ProSelectYesOrNo(char *pszMainMenu, char *pszTitle, char *pszStr, char *pcSelect)
{
	int nSelect;

	if (*pcSelect == 0 || *pcSelect =='0')
	{
		nSelect = 0;
	}
	else if ( *pcSelect ==1 || *pcSelect =='1')
	{
		nSelect = 1;
	}
	else
	{
		nSelect = 0XFF;
	}
	ASSERT_RETURNCODE(PubSelectListItem(pszStr, pszMainMenu, pszTitle, nSelect, &nSelect));
	*pcSelect = nSelect + '0';
	return APP_SUCC;
}

int ProSelectYesOrNoExt(char *pszMainMenu, char *pszTitle, char *pszStr, char *pcSelect)
{
	int nRet, nSelect;
	char szFunKey[2+1]={0};
	szFunKey[0] = KEY_F1; 
	szFunKey[1] = KEY_F2; 
	
	if (*pcSelect == 0 || *pcSelect =='0')
	{
		nSelect = 0;
	}
	else if ( *pcSelect ==1 || *pcSelect =='1')
	{
		nSelect = 1;
	}
	else
	{
		nSelect = 0XFF;
	}
	nRet = PubSelectListItemExtend(pszStr, pszMainMenu, pszTitle, nSelect, &nSelect, 60, szFunKey);
	if (nRet==APP_QUIT || nRet == APP_TIMEOUT)
	{
		return APP_QUIT;
	}
	if (nSelect - 0xFF == KEY_UP)
    {
		return KEY_UP;
    }
	else if(nSelect - 0xFF == KEY_DOWN)
	{
		return KEY_DOWN;
	}
	*pcSelect = nSelect + '0';
	return APP_SUCC;
}
#endif


int ProChkDigitStr(const char *psStr, const int nLen)
{
	int i;
	
	for (i = 0; i < nLen; i++)
	{
		if (PubIsDigitChar(psStr[i]) != APP_SUCC)
		{
			return APP_FAIL;
		}
	}
	return APP_SUCC;
}

int ProConfirm(void)
{
	while(1)
	{
	    PubUpdateWindow();
		switch(PubGetKeyCode(0))
		{
		case KEY_ENTER:
			break;
		case KEY_ESC:
			return APP_QUIT;
		default:
			continue;
		}
		break;
	}
	return APP_SUCC;
}

unsigned long long Bcd2Unit64( uchar *bcd,  int bcd_len )
{
	unsigned long long  result = 0;
	uchar *ptr = bcd;

	if( bcd == NULL || bcd_len <= 0 )
		return 0;

	while( ptr < bcd + bcd_len ){
		result = result * 10  + (*ptr >> 4);
		result = result  * 10 + (*ptr++ & 0x0f) ;
	}

	return result;
}

void AmtAddAmt(uchar *pszAsc1, uchar *pszAsc2, uchar *pszSum)
{
	PubAscAddAsc(pszAsc1, pszAsc2, pszSum);			
	PubAddSymbolToStr((char *)pszSum, 12, '0', 0);
}

void AmtSubAmt(uchar *pszAsc1, uchar *pszAsc2, uchar *pszResult)
{
	PubAscSubAsc(pszAsc1, pszAsc2, pszResult);
	PubAddSymbolToStr((char *)pszResult, 12, '0', 0);
}

int TlvGetValue(const uint unTag, void *pszOutValue, int *pnOutLen, char *szTlvBuf, const int nTlvBufLen)
{
	tlv_t stTlvObj[12]; 
	int nRet = 0;
	int nIndex = 0;
	
	memset(stTlvObj, 0, sizeof(stTlvObj));

	nRet = EMV_parse_tlv((uchar *)szTlvBuf, nTlvBufLen, stTlvObj, 12, STRING_TLVOBJ);
	if (nRet != 0)
	{
		PubDebug("EMV_parse_tlv->Error,nRet=[%d]\n", nRet);
		return APP_FAIL;
	}

	if((nIndex = EMV_fetch_tlv(0, unTag, stTlvObj, SEARCH_ALL_DESC)) == 0)
	{
		PubDebug("EMV_fetch_tlv->Fail.\n");
		return APP_FAIL;
	}
	memcpy(pszOutValue, stTlvObj[nIndex].pvalue, stTlvObj[nIndex].valuelen);
	*pnOutLen = stTlvObj[nIndex].valuelen;
TRACE_HEX((pszOutValue, *pnOutLen, "Value: "));	
	return APP_SUCC;
}

void ShowLightIdle()
{
	if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
	{
		return ;
	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))  
	{
		NDK_LedStatus(LED_RFID_BLUE_ON|LED_RFID_YELLOW_OFF|LED_RFID_GREEN_OFF|LED_RFID_RED_OFF);
		NDK_SysMsDelay(100);
		NDK_LedStatus(LED_RFID_BLUE_OFF|LED_RFID_YELLOW_OFF|LED_RFID_GREEN_OFF|LED_RFID_RED_OFF);
	}
	else if (16 == PubScrGetColorDepth())
	{
		//Cup_ShowScrLed(LED_UNCHANGED, LED_UNCHANGED, LED_UNCHANGED, LED_FLICK);	
	}
}

void ShowLightReady()
{
    if(YES != GetVarIsSupportRF() || GetVarIsIcFlag() != YES)
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL)) 
	{
		NDK_LedStatus(LED_RFID_BLUE_ON|LED_RFID_YELLOW_OFF|LED_RFID_GREEN_OFF|LED_RFID_RED_OFF);
	}
}


void ShowLightDeal()
{
    if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL)) 
	{
		NDK_LedStatus(LED_RFID_BLUE_ON|LED_RFID_YELLOW_ON|LED_RFID_GREEN_OFF|LED_RFID_RED_OFF);
	}
}

void ShowLightSucc()
{
    if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))  
	{
		NDK_LedStatus(LED_RFID_BLUE_ON|LED_RFID_YELLOW_ON|LED_RFID_GREEN_ON|LED_RFID_RED_OFF);
	}
}

void ShowLightFail()
{
    if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL))
	{
		NDK_LedStatus(LED_RFID_BLUE_OFF|LED_RFID_YELLOW_OFF|LED_RFID_GREEN_OFF|LED_RFID_RED_ON);
	}
}



void ShowLightOff()
{
    if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL)) 
	{
		NDK_LedStatus(LED_RFID_BLUE_OFF|LED_RFID_YELLOW_OFF|LED_RFID_GREEN_OFF|LED_RFID_RED_OFF);
	}
}

void ShowLightOnline()
{
    if(YES != GetVarIsSupportRF() || YES != GetVarIsIcFlag())
   	{
   		return ;
   	}
	if (APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_LED_LIGHT, NULL)) 
	{
		NDK_LedStatus(LED_RFID_BLUE_ON|LED_RFID_YELLOW_ON|LED_RFID_GREEN_FLICK|LED_RFID_RED_OFF);
	}
	else if (16 == PubScrGetColorDepth()) 
	{
		//Cup_ShowScrLed(LED_UNCHANGED, LED_FLICK, LED_UNCHANGED, LED_UNCHANGED);	
	}
}

/**ISO 4217*/
int CurrencyAmtToDisp(const char *pszCurrency, const char *pszAmt, char *pszOutstr)
{
	char *pszCurrcyName[] = 
	{
		"156RMB",	
		"344HKD",	
		"446MOP",	
		"458MYR",	
		"702SGD",	
		"764THB",	
		"360IDR",	
		"840USD",	
		"392JPY",
		"978EUR",	
		"826GBP",	
		"124CAD",	
		NULL,		
	};	

	char **p = NULL;	
	char szDispAmt[13+1] = {0};

	ProAmtToDispOrPnt(pszAmt, szDispAmt);
	PubAllTrim(szDispAmt);
	
	for (p = pszCurrcyName; *p != NULL; p++)
	{
		if (0 == memcmp(pszCurrency, *p, 3))
		{
			sprintf(pszOutstr, "%3.3s %s", *p + 3, szDispAmt);
			return APP_SUCC;
		}
	}

	sprintf(pszOutstr, "OTHER %s", szDispAmt);
	return APP_FAIL;
}


int SetStatusBar(EMSTATUSBARTYPE emType)
{
	EM_STATUSBAR emCommType = STATUSBAR_DISP_TIME|STATUSBAR_DISP_DATE|STATUSBAR_DISP_BATTERY;
	STAPPCOMMPARAM stAppCommParam;
	char szVer[64]={0};

	if (STATUSBAR_STANDBY_OPEN  == emType || STATUSBAR_STANDBY_CLOSE == emType) 
	{	
		if (16 == PubScrGetColorDepth())
		{
			return APP_SUCC;
		}
	}
	else 
	{
		if (1 == PubScrGetColorDepth())  
		{
			return APP_SUCC;
		}
	}

	NDK_Getlibver(szVer);

	if ((strcmp(szVer, "4.4.5") >= 0) && szVer[0] != 'V')
	{
		NDK_ScrStatusbar(STATUSBAR_DISP_CLOSE);
		
		if (STATUSBAR_STANDBY_OPEN == emType
			||STATUSBAR_OTHER_OPEN == emType)
		{
			if (16 == PubScrGetColorDepth()) 
			{
				emCommType |= STATUSBAR_POSITION_TOP;
			}
			else
			{
				emCommType |= STATUSBAR_POSITION_BOTTOM;
			}

			memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));
			GetAppCommParam(&stAppCommParam);
			
			switch(stAppCommParam.cCommType)
			{
			case COMM_RS232:
				break;
			case COMM_DIAL:
				emCommType |= STATUSBAR_DISP_MODEM;		
				break;
			case COMM_GPRS:
			case COMM_CDMA:
				emCommType |= STATUSBAR_DISP_WLSIGNAL;
				break;
			case COMM_ETH:
				emCommType |= STATUSBAR_DISP_ETH;
				break;
			case COMM_WIFI:
				emCommType |= STATUSBAR_DISP_WIFI;
				break;
			default:
				break;
			}

			NDK_ScrStatusbar(emCommType);
		}
	}
	
	return APP_SUCC;
}

#define FILE_LOWESTLIB_INI "LIBLIST.INI"

/**
* @brief Get the lowest library, the libray in this project should be newer than lowest version
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetLowestLibIni(const char *pszLibType, char *pszLibVer)
{
	const char *szIniFile = FILE_LOWESTLIB_INI;
	int nIniHandle,nRet;
	int nLen;
	char szBuf[100];
	char szTmp[100] = {0};
	
	if (NULL == pszLibVer)
	{
		return APP_QUIT;
	}
	if (NDK_OK != NDK_FsExist(szIniFile))
	{
		PubMsgDlg("Warning", "Lack lib file", 3, 3);
		return APP_FAIL;
	}
	ASSERT_QUIT(PubOpenFile (szIniFile, "r", &nIniHandle));
	nRet = PubGetINIItemStr (nIniHandle, "BASE", pszLibType,  sizeof(szBuf) - 1, szBuf, &nLen);
	if (APP_SUCC != nRet)
	{
    	sprintf(szTmp, "Get [%s] Version Fail", pszLibType);
		PubMsgDlg("Warning", szTmp, 3, 3);
	}
	else
	{
		memcpy(pszLibVer, szBuf, 16);	
	}
	PubCloseFile(&nIniHandle);
	return nRet;
} 


#define POWER_DEV "/dev/power"
#define POWER_IOC_MAGIC 'P'
#define POWER_GET_STATUS	        _IO(POWER_IOC_MAGIC, 1)
#define ADAPTER			0
#define BATTERY			1
#define ADAPTERONLY		2
#define BATTERY_CHARGING 3
typedef struct {
	unsigned char pmType;
	unsigned char charging;
	unsigned char couple;
	unsigned int adc_volt;
	unsigned char show_adc;
	unsigned int max_volt;
}PM_STATUS;

static int app_get_power_status(unsigned char *pstatus)
{
	int fd;
	int ret;
	PM_STATUS ppw_status;
	fd = open("/dev/power", O_RDWR);
	if (fd < 0) {
		return -1;
	}
	ret = ioctl(fd, POWER_GET_STATUS, &ppw_status);
	if (ret) {
		close(fd);
		return -4;
	}
	if (ppw_status.pmType==ADAPTER){
		if(ppw_status.charging != 1)//None
		{
			*pstatus = ADAPTER;
		}
		else
		{
			*pstatus = BATTERY_CHARGING;//Charging,
		}
	}
	else if(ppw_status.pmType==ADAPTERONLY)
	{
		*pstatus = ADAPTERONLY;
	}
	else
	{
		*pstatus = BATTERY;
	}
	close(fd);
	return 0;
}

static int CheckIsLinkPower(void)
{
	uchar ucPower;
	char szVer[64]={0};
	ST_POWER_INFO stPowerInfo;

	NDK_Getlibver(szVer);

	if(strcmp(szVer,"4.4.7")>=0 && szVer[0]!='V')
	{
		memset(&stPowerInfo, 0, sizeof(stPowerInfo));
		if(NDK_SysGetPowerInfo(&stPowerInfo) == NDK_OK)
		{
			if(stPowerInfo.unPowerType & 1)
			{
				return APP_SUCC;
			}
		}
	}
	else if(app_get_power_status(&ucPower) == 0)
	{
		if(ucPower != BATTERY)
		{
			return APP_SUCC;
		}
	}

	return APP_FAIL;
}

void GotoSleepApp(void)
{
	char szToolVer[32] = {0};
	STAPPCOMMPARAM stAppCommParam;

	if (APP_SUCC == CheckIsLinkPower()) 
	{
		return;
	}
	memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));
	GetAppCommParam(&stAppCommParam);
	PubGetToolsVer(szToolVer);
	if(memcmp(szToolVer,"ALTOOL011405060",15)>=0)
	{
		if (COMM_WIFI != stAppCommParam.cCommType)
		{
			PubCommClose();
		}
		//PubGoSuspend();
	}
	return;
}

/**
* @brief Check if string is digit
* @param [in] psStr 	字符串指针
* @param [in] nLen 	字串长度
* @return 
* @li APP_SUCC 
* @li APP_FAIL
* @author 
* @date
*/
int ChkDigitStr(const char *psStr, const int nLen)
{
	int i;
	
	for (i = 0; i < nLen; i++)
	{
		ASSERT_FAIL(PubIsDigitChar(psStr[i]));
	}
	return APP_SUCC;
}

//psStr1
void EdcProXor(char *psStr1, char *psStr2, int nLen)
{
	int i = 0;
	
	while (1)
	{
		if (nLen == i)
			break;
		*(psStr1+i) ^= *(psStr2+i);
		i++;
	}
}


/**
* @brief  bcd amount calculate, the result is display format like X.XX
* @param [in] cType	0Plus，1Minus
* @param [in] psBcdAmt1
* @param [in] psBcdAmt2	
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int MathBcdAmtAndToDisp(const uchar *psBcdAmt1, const uchar *psBcdAmt2, EM_MATH_TYPE emType, char *pszDispAmt)
{
	uchar szAmt[12+1] = {0}, szAmt1[12+1] = {0}, szAmt2[12+1] = {0};
	
	if (NULL == pszDispAmt)
		return APP_FAIL;

	PubHexToAsc(psBcdAmt1, 12, 0, szAmt1);
	PubHexToAsc(psBcdAmt2, 12, 0, szAmt2);
	memset(szAmt, 0, sizeof(szAmt));
	switch (emType)
	{
	case EM_SUB:
		PubAscSubAsc(szAmt1, szAmt2, szAmt);
		break;
	case EM_PLUS:
	default:
		PubAscAddAsc(szAmt1, szAmt2, szAmt);
		break;
	}
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	ProAmtToDispOrPnt((char*)szAmt, pszDispAmt);
	PubAllTrim(pszDispAmt);
	return APP_SUCC;
}

/**
* @brief Bcd amount to Display format
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int BcdAmtToDisp(const uchar *psBcdAmt, char *pszDispAmt)
{
	char szAmt[12+1] = {0};
	
	if (NULL == pszDispAmt)
		return APP_FAIL;
	PubHexToAsc(psBcdAmt, 12, 0,  (uchar*)szAmt);
	ProAmtToDispOrPnt((char*)szAmt, pszDispAmt);
	PubAllTrim(pszDispAmt);
	return APP_SUCC;
}


/**
* @brief Bcd amount to Display format
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int IncreaseAmt(const uchar *psBcdAmt, char *pszAmount)
{
	uchar szAmt[12+1] = {0}, szAmt1[12+1] = {0}, szAmt2[12+1] = {0};
	int emType = EM_PLUS;
	
	PubHexToAsc(psBcdAmt, 12, 0, szAmt1);
	strcpy(szAmt2, pszAmount);
	
	switch (emType)
	{
	case EM_SUB:
		PubAscSubAsc(szAmt1, szAmt2, szAmt);
		break;
	case EM_PLUS:
	default:
		PubAscAddAsc(szAmt1, szAmt2, szAmt);
		break;
	}
	
	PubAddSymbolToStr((char *)szAmt, 12, '0', 0);
	strcpy(pszAmount, szAmt);
	
	return APP_SUCC;
}


typedef enum
{
	EM_FIRST,
	EM_LAST,
	EM_MEDDLE,
	EM_NONE
}EM_TITLEPOS;
typedef struct
{
	char nCount;			/*Total index*/
	char nItemNum;			/*Amount in one page */
	char szContent[256];  /*Content*/
}STDyMenuPage;

void ProShowTitle(const char *pszTitle, int nStyle)
{
	char szTmp[MAX_LINELETTER_MIDDLE+1] = {0};

	memset(szTmp, ' ', sizeof(szTmp));
	szTmp[MAX_LINELETTER_MIDDLE] = 0;
	memcpy(szTmp+(MAX_LINELETTER_MIDDLE-strlen(pszTitle))/2, pszTitle, strlen(pszTitle));
	switch (nStyle)
	{
	case EM_FIRST:
		memcpy(szTmp+MAX_LINELETTER_MIDDLE-2, "→", 2);//↓
		PubDisplayTitle(szTmp);
		break; 
	case EM_LAST:
		memcpy(szTmp, "←", 2);//↑
		PubDisplayTitle(szTmp); 
		break;
	case EM_MEDDLE:
		memcpy(szTmp, "←", 2);//↑
		memcpy(szTmp+MAX_LINELETTER_MIDDLE-2, "→", 2);//↓
		PubDisplayTitle(szTmp);
		break;
	case EM_NONE:
		PubDisplayTitle((char *)pszTitle);
		break;
	default:
		break;
	}
}

int ProDyMenuSel(const char *pszTitle, const char *plszItem, int nItemNum, int nTimeout)
{
	int nMaxLine = 5;//600 6
	int nMaxLineLetter = 20;//600 26 
	int nItemIndex = 0, nPageIndex = 0, nKey = 0, nCurrentNum = 0, nTotalPage = 0, nSel = 0;
	STDyMenuPage stPage[MAX_PAGE];
	char szTmp[200] = {0};
	char lszList[MAX_ITEM][128];	

	memset(lszList, 0, MAX_ITEM*128);
	memcpy(lszList, plszItem, nItemNum*128);
	memset(&stPage, 0, sizeof(stPage));
	
	if (nItemNum > MAX_ITEM)
	{
		PubMsgDlg(pszTitle, "ITEM TO MUCH", 3, 1);
		return APP_FAIL;
	}
	if (NULL != pszTitle)
	{
		nMaxLine = 4;//5
	}

	while (1)
	{
		if (0 == strlen(lszList[nItemIndex]))
		{
			nItemIndex++;
			continue;
		}
		//Add 1.\n
		if ((strlen(stPage[nPageIndex].szContent)+strlen(lszList[nItemIndex])+2) > (nMaxLine*nMaxLineLetter))// 3
		{			
			stPage[nPageIndex+1].nCount = stPage[nPageIndex].nCount + stPage[nPageIndex].nItemNum;		
			nPageIndex++;
			nCurrentNum = 0;
			continue;
		}
		sprintf(szTmp, "%d.%s", nCurrentNum +1, lszList[nItemIndex]);
		if (strlen(szTmp)%nMaxLineLetter != 0)
		{
			PubAddSymbolToStr(szTmp, strlen(szTmp)+(nMaxLineLetter - strlen(szTmp)%nMaxLineLetter) , ' ', 1);//- 1
		}
		sprintf(stPage[nPageIndex].szContent+strlen(stPage[nPageIndex].szContent), "%s", szTmp);//\n
		stPage[nPageIndex].nItemNum++;
		nCurrentNum++;
		nItemIndex++;
		if (nItemIndex >= nItemNum)
			break;
	}
	nTotalPage = nPageIndex+1;
	
	nPageIndex = 0;
	while (1)
	{
		PubClearAll();
		if (NULL != pszTitle)
		{
			if (0 == nPageIndex && 1 == nTotalPage)
			{
				ProShowTitle(pszTitle, EM_NONE);
			}
			else if (0 == nPageIndex && nTotalPage > 1)
			{
				ProShowTitle(pszTitle, EM_FIRST);
			}
			else if (nPageIndex == nTotalPage - 1)
			{
				ProShowTitle(pszTitle, EM_LAST);
			}
			else
			{
				ProShowTitle(pszTitle, EM_MEDDLE);
			}
			PubDispMultLines(0, 2, 1, stPage[nPageIndex].szContent);
		}
		else
		{
			PubDispMultLines(0, 1, 1, stPage[nPageIndex].szContent);
		}
		PubUpdateWindow();
		NDK_KbGetCode((uint)nTimeout, &nKey);
		switch (nKey)
		{
		case 0:
			return APP_TIMEOUT;
		case KEY_ESC:
			return APP_QUIT;
		case KEY_LEFT:
			if (nPageIndex > 0)
				nPageIndex--;
			break;
		case KEY_RIGHT:
			if (nPageIndex < (nTotalPage-1))
				nPageIndex++;
			break;
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
			if ((nKey-0x30)>stPage[nPageIndex].nItemNum)
				continue;
			nSel = stPage[nPageIndex].nCount+(nKey-0x30-1);
			return nSel;
			
		}
	}
	return APP_SUCC;
}


//Del space enter tab
void ProTrimStr(char *pszString)
{
#define ISSPACE(ch) ( ((ch) == ' ')  || ((ch) == '\t') || \
                      ((ch) == '\n') || ((ch) == '\r') )

    char *p, *q;

    if( !pszString || !*pszString )
    {
        return;
    }

    p = &pszString[strlen((char*)pszString) - 1];
    while(( p > pszString) && ISSPACE(*p) )
    {
        *p-- = 0;
    }
    if( (p == pszString) && ISSPACE(*p) )  *p = 0;

    for(p = pszString; *p && ISSPACE(*p); p++);
    if( p != pszString )
    {
        q = pszString;
        while( *p )   *q++ = *p++;
        *q = 0;
    }
#undef ISSPACE
}

// "A/B.Mr" --> "Mr A B"
void ConvertHolderName(uchar *pszOrgName, uchar *pszNormalName)
{
	char	*pszTitle, *pszMidName, *pszTemp, szBuff[50];

	sprintf((char *)pszNormalName, "%s", pszOrgName);
	if( *pszOrgName==0 )
	{
		return;
	}
	pszTemp = (char *)pszNormalName;

	pszMidName = strchr((char *)pszOrgName, '/');
	if( pszMidName==NULL )
	{
		return;
	}

	pszTitle = strrchr((char *)pszOrgName, '.');
	if( pszTitle!=NULL )
	{
		sprintf(szBuff, "%s ", pszTitle+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);

		sprintf(szBuff, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
		sprintf(szBuff, "%.*s ", (int)(pszTitle-pszMidName-1), pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
		return ;
	}
	else
	{
		sprintf(szBuff, "%s", pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	sprintf(pszTemp, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
}

// "Amex/F D.Mr" --> "Mr F D Amex"
void ConvertHolderName1(uchar *pszOrgName, uchar *pszNormalName)
{
	char	*pszTitle, *pszMidName, *pszTemp, szBuff[50];

	sprintf((char *)pszNormalName, "%s", pszOrgName);
	if( *pszOrgName==0 )
	{
		return;
	}
	pszTemp = (char *)pszNormalName;

	pszMidName = strchr((char *)pszOrgName, '/');
	if( pszMidName==NULL )
	{
		return;
	}

	pszTitle = strrchr((char *)pszOrgName, '.');
	if( pszTitle!=NULL )
	{
		sprintf(szBuff, "%s ", pszTitle+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);

		sprintf(szBuff, "%.*s ", (int)(pszTitle-pszMidName-1), pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	else
	{
		sprintf(szBuff, "%s", pszMidName+1);
		ProTrimStr(szBuff);
		pszTemp += sprintf(pszTemp, "%s ", szBuff);
	}
	sprintf(pszTemp, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
}

/**
* @brief Add some same letter between String A and String B
* @param [in] pszStrA 
* @param [in] pszStrB       
* @param [in] ch      
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2012-06-19
*/
int ProAddSymbolBtnStrs(char *pszStrA, char *pszStrB, char *pszOut, int nLen, char ch)
{
	int nSpaceLen = 0;
	int nLenA = 0, nLenB = 0;
	
	if (NULL == pszOut)
		return APP_FAIL;

	if (NULL != pszStrA)
	{
		nLenA = strlen(pszStrA);
	}
	if (NULL != pszStrB)
	{
		nLenB = strlen(pszStrB);
	}
	
	if (NULL != pszStrA)
		strcpy(pszOut, pszStrA);

	if (NULL != pszStrB)
	{
		if (0 == ch)
		{
			memcpy(pszOut+nLenA, pszStrB, nLenB);
			return APP_SUCC;
		}
		nSpaceLen = nLen-nLenB;
		if (nSpaceLen < 0)
		{
			nSpaceLen = 0;
		}
		if ((nSpaceLen - nLenA) > 0)
		{	
			PubAddSymbolToStr(pszOut, nSpaceLen, ch, 1);
		}
		memcpy(pszOut+nSpaceLen, pszStrB, nLenB);
	}
	else
	{
		PubAddSymbolToStr(pszOut, nLen, ch, 0);	
	}
	
	return APP_SUCC;
}

/*
 YYYYMMDD
*/
int CaculateWeekDay(char *pszData)
{
    char szYear[4+1];
    char szMonth[2+1];
    char szDay[2+1];
    int y;
    int m;
    int d;
    int iWeek;

    if (pszData == NULL)
    {
        return APP_FAIL;
    }

    //YEAR
    memset(szYear, 0, sizeof(szYear));
    memcpy(szYear, pszData, sizeof(szYear)-1);
    //MONTH
    memset(szMonth, 0, sizeof(szMonth));
    memcpy(szMonth, pszData+4, sizeof(szMonth)-1);
    //DAY
    memset(szDay, 0, sizeof(szDay));
    memcpy(szDay, pszData+4+2, sizeof(szDay)-1);

    y = atoi(szYear);
    m = atoi(szMonth);
    d = atoi(szDay);
    
    if(m==1||m==2) {
        m+=12;
        y--;
    }
    iWeek=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
    iWeek++;
    
    return iWeek;
} 

#define _U 0x01 /* upper */
#define _L 0x02 /* lower */
#define _D 0x04 /* digit */
#define _C 0x08 /* cntrl */
#define _P 0x10 /* punct */
#define _S 0x20 /* white space (space/lf/tab) */
#define _X 0x40 /* hex digit */
#define _SP 0x80 /* hard space (0x20) */
static unsigned char gucType[] =
{
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 0-7 */
	_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,  /* 8-15 */
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 16-23 */
	_C,_C,_C,_C,_C,_C,_C,_C,   /* 24-31 */
	_S|_SP,_P,_P,_P,_P,_P,_P,_P,   /* 32-39 */
	_P,_P,_P,_P,_P,_P,_P,_P,   /* 40-47 */
	_D,_D,_D,_D,_D,_D,_D,_D,   /* 48-55 */
	_D,_D,_P,_P,_P,_P,_P,_P,   /* 56-63 */
	_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U, /* 64-71 */
	_U,_U,_U,_U,_U,_U,_U,_U,   /* 72-79 */
	_U,_U,_U,_U,_U,_U,_U,_U,   /* 80-87 */
	_U,_U,_U,_P,_P,_P,_P,_P,   /* 88-95 */
	_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L, /* 96-103 */
	_L,_L,_L,_L,_L,_L,_L,_L,   /* 104-111 */
	_L,_L,_L,_L,_L,_L,_L,_L,   /* 112-119 */
	_L,_L,_L,_P,_P,_P,_P,_C,   /* 120-127 */
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 128-143 */
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 144-159 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 128-143 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 144-159 */	
	_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
	_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
	_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
	_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
	_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
	_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L		/* 240-255 */
};

#define LINE_LEN		128			// Set   line   length   in   configuration   files

#define __ismask(x)	(gucType[(int)(unsigned char)(x)])
#define Proisgraph(c) ((__ismask(c)&(_P|_U|_L|_D|(_S|_SP))) != 0)
#define Proisalnum(c) ((__ismask(c)&(_U|_L|_D|(_S|_SP))) != 0)

static int ProIsremark(const char* pszLine)
{
	int i;

	if(pszLine ==  NULL)
	{
		return APP_FAIL;
	}
	for(i = 0; i < strlen(pszLine); i++)
	{
		if(Proisgraph(pszLine[i]))
		{
			if(pszLine[i] == '#')
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}

static int ProGetSection(const char* pszLine, char* pszSection, unsigned int nSeclen)
{
	unsigned int unStart;
	unsigned int unMid;
	unsigned int unEnd;

	if(pszLine ==  NULL || pszSection == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; unStart < strlen(pszLine); unStart++)
	{
		if(Proisgraph(pszLine[unStart]))
		{
			if(pszLine[unStart] != '[')
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unStart >= strlen(pszLine)) 
	{
		return 0;
	}

	for(unEnd = strlen(pszLine); unEnd > 1; unEnd --)
	{
		if(Proisgraph(pszLine[unEnd]))
		{
			if(pszLine[unEnd] != ']')
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unEnd <= 1) 
	{
		return 0;
	}

	if(unEnd - unStart < 2) 
	{
		return 0;
	}

	for(unMid = unStart + 1; unMid < unEnd; unMid++)
	{
		if(!Proisalnum(pszLine[unMid])) 
		{
			return 0;
		}
	}

	if(nSeclen > unEnd - unStart - 1)
	{
		memcpy(pszSection, &pszLine[unStart + 1], unEnd - unStart - 1);
		pszSection[unEnd - unStart - 1] = '\0';
		return(unEnd - unStart - 1);
	}
	else
	{
		memcpy(pszSection, &pszLine[unStart + 1], nSeclen - 1);
		pszSection[nSeclen - 1] = '\0';
		return(nSeclen -1);
	}
}

static int ProGetKeyName(const char* pszLine, char* pszKeyName, unsigned int unKeyLen)
{
	unsigned int unStart;
	unsigned int unMid;
	unsigned int unEnd;

	if(pszLine ==  NULL || pszKeyName == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; unStart < strlen(pszLine); unStart++)
	{
		if (Proisgraph(pszLine[unStart]))
		{
			if(!Proisalnum(pszLine[unStart]))
			{
				return 0;
			}
			else
			{
				break;
			}
		}
	}

	if(unStart >= strlen(pszLine)) 
	{
		return 0;
	}

	for(unEnd = unStart; pszLine[unEnd] != '=' && unEnd < strlen(pszLine); unEnd++) 
	{
		if (!Proisalnum(pszLine[unEnd]) && pszLine[unEnd] != ' ' && pszLine[unEnd] != '\t')
		{
			return 0;
		}
	}

	if(unEnd >= strlen(pszLine)) 
	{
		return 0;
	}

	for(unEnd = unEnd- 1; !Proisalnum(pszLine[unEnd]); unEnd--)
	{
		;
	}
	for(unMid = unStart; unMid <= unEnd; unMid++)
	{
		if (pszLine[unMid] == '\t' ) 
		{
			return 0;
		}
	}

	if(unKeyLen > unEnd - unStart + 1)
	{
		memcpy(pszKeyName, &pszLine[unStart], unEnd - unStart + 1);
		pszKeyName[unEnd - unStart + 1] = '\0';
		return (unEnd - unStart + 1);
	}
	else
	{
		memcpy(pszKeyName, &pszLine[unStart], unKeyLen - 1);
		pszKeyName[unKeyLen - 1] = '\0';
		return (unKeyLen -1);
	}
}

static int ProIsjoint(const char* pszLine)
{
	unsigned int nI;
	unsigned int nSign = 0;

	if(pszLine ==  NULL)
	{
		return APP_FAIL;
	}
	for(nI = 0; nI < strlen(pszLine); nI++)
	{
		if(pszLine[nI] == ' ' || pszLine[nI] == '\t' || pszLine[nI] == '\\')
		{
			if(pszLine[nI] == ' ' || pszLine[nI] == '\t' )
			{
				if(nSign < 1 )
				{
					nSign++;
				}
			}
			else
			{
				if(nSign > 0 )
				{
					nSign++;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}

	if(nSign == 2)
	{
		return 1;
	}

	return 0;
}

static int ProFGetC(int nFp)
{
	char ch;

	if(NDK_FsRead(nFp, (char *)&ch, 1) != 1)
	{
		return (-1);
	}
	else
	{
		return ch;
	}
}

static char * ProFGets(char *pszBuf, int nMaxLen, int nFp)
{
	char *p;
	int nC;

	if(pszBuf ==  NULL || nMaxLen <= 0)
	{
		return NULL;
	}

	p = pszBuf;
	while(--nMaxLen)
	{
		if((nC = ProFGetC(nFp)) == (-1))
		{
			break;
		}
		if ((*p++ = nC) == '\n')
		{
			break;
		}
	}

	if (p > pszBuf)
	{
		*p = 0;
		return pszBuf;
	}
	return NULL;
}

static long ProFilelength (int nFd)
{
	ulong ulFileLen = 0;
	ulong ulOldOffset = 0;
	if(NDK_FsTell(nFd, &ulOldOffset) != NDK_OK)
		return APP_FAIL;
	if(NDK_FsSeek(nFd, 0L, SEEK_END) != NDK_OK)
		return APP_FAIL;
	if(NDK_FsTell(nFd, &ulFileLen) != NDK_OK)
		return APP_FAIL;
	if(NDK_FsSeek(nFd, ulOldOffset, SEEK_SET) != NDK_OK)
		return APP_FAIL;
	return (long)ulFileLen;
}

static int ProFoef(int nFd)
{
	long lFileLen = 0;
	ulong lCurOff = 0;

	if((lFileLen = ProFilelength(nFd)) < 0)
	{
		return (-1);
	}
	NDK_FsTell(nFd, &lCurOff);
	if(lCurOff  < 0)
	{
		return (-1);
	}

	if(lCurOff >= lFileLen)	
	{
		return (-1);
	}

	return 0;
}

 static char * ProReadline(char* pszLine, int nLen, int nFp)
 {
	static char* psRs;

	if(pszLine ==  NULL)
	{
		return NULL;
	}
  	psRs = ProFGets(pszLine, nLen, nFp);
	if(strlen(pszLine) >= 1 )
	{
		pszLine[strlen(pszLine) - 1] = pszLine[strlen(pszLine)]; 
		if(pszLine[strlen(pszLine)-1] == 0x0d)				
		{
			pszLine[strlen(pszLine) - 1] = pszLine[strlen(pszLine)];
		}
	}
  	return psRs;
 }

static int ProGetKeyAdd(const char* pszLine, char* pszKeyValue, unsigned int unValen)
{
	unsigned int unStart;
	unsigned int unEnd;
	int nLen;

	if(pszLine ==  NULL || pszKeyValue == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; !Proisgraph(pszLine[unStart]) && unStart < strlen(pszLine); unStart++)
	{
		;
	}
	if(unStart >= strlen(pszLine))
	{
		return 0;
	}
	for(unEnd = unStart;
		pszLine[unEnd] != ' ' && pszLine[unEnd] != '\t' && unEnd < strlen(pszLine);
		unEnd++)
	{
		;
	}

	if(unValen > unEnd - unStart)
	{
		memcpy(pszKeyValue, &pszLine[unStart], unEnd - unStart);
		pszKeyValue[unEnd - unStart] = '\0';
		nLen = unEnd - unStart;
	}
	else
	{
		memcpy(pszKeyValue, &pszLine[unStart], unValen - 1);
		pszKeyValue[unValen - 1] = '\0';
		nLen = unValen -1;
	}

	if(unEnd >= strlen(pszLine))
	{
		return nLen;
	}
	else
	{
		if(ProIsjoint(&pszLine[unEnd]))
		{
			return -nLen;
		}
		return nLen;
	}
}

static int ProGetKeyValue(const char* pszLine, char* pszKeyValue, unsigned int unValen)
{
	unsigned int unStart;
	unsigned int unEnd;
	int nLen;

	if(pszLine ==  NULL || pszKeyValue == NULL)
	{
		return APP_FAIL;
	}
	for(unStart = 0; pszLine[unStart] != '=' && unStart < strlen(pszLine); unStart++)
	{
		;
	}

	for(unStart = unStart + 1;
		!Proisgraph(pszLine[unStart]) && unStart < strlen(pszLine);
		unStart++)
	{
		;
	}

	if( unStart >= strlen(pszLine)) 
	{
		return 0;
	}
	for(unEnd = unStart;
		/*pszLine[unEnd] != ' ' &&*/ pszLine[unEnd] != '\t' && unEnd < strlen(pszLine);
		unEnd++)
	{
		;
	}

	if(unValen > unEnd - unStart)
	{
		memcpy(pszKeyValue, &pszLine[unStart], unEnd - unStart);
		pszKeyValue[unEnd - unStart] = '\0';
		nLen = unEnd - unStart;
	}
	else
	{
		memcpy(pszKeyValue, &pszLine[unStart], unValen - 1);
		pszKeyValue[unValen - 1] = '\0';
		nLen = unValen -1;
	}

	if(unEnd >= strlen(pszLine))
	{
		return nLen;
	}
	else
	{
		if(ProIsjoint(&pszLine[unEnd]))
		{
			return -nLen;
		}
		return nLen;
	}
}

int PrivateGetINIItemStr (const int nHandle, const char *psSeg, const char *psItem,
					  const int nMaxStrLen, char *psValueStr, int *pnValueLen)
{
	int nStep = 0;
	int nRs = 0;
	int nRa = 0;
	char szLine[LINE_LEN];
	char szSec[LINE_LEN];
	char szKen[LINE_LEN];
	char szKev[LINE_LEN];

	if(psSeg ==  NULL || psItem == NULL || psValueStr == NULL || pnValueLen == NULL || nMaxStrLen <= 0 )
	{
		return APP_FAIL;
	}
	memset(szLine, 0, sizeof(szLine));
	memset(szSec, 0, sizeof(szSec));
	memset(szKen, 0, sizeof(szKen));
	memset(szKev, 0, sizeof(szKev));

	NDK_FsSeek(nHandle, 0L, SEEK_SET);		

	while(!ProFoef(nHandle))
	{
		if(ProReadline(szLine, 100, nHandle) == NULL)
		{
			return APP_FAIL;	/*   error   occur   in   reading   configuration   file   */
		}

		if(!ProIsremark(szLine))
		{
			if(nStep == 0)
			{
				if(ProGetSection(szLine, szSec, LINE_LEN))
				{
					if(strcmp(szSec, psSeg) == 0)
					{
						nStep = 1;
					}
				}
			}
			else if(nStep == 1)
			{
				if(ProGetKeyName(szLine, szKen, LINE_LEN))
				{
					if(strcmp(szKen, psItem) == 0)
					{
						nRs = ProGetKeyValue(szLine, szKev, LINE_LEN);
						if(nRs > 0)
						{
							strncpy(psValueStr, szKev, nMaxStrLen);
							*pnValueLen = strlen(psValueStr);
							return APP_SUCC;  /*  read configuration file successfully   */
						}
						else if(nRs < 0)
						{
							nStep = 2;
							nRs = abs(nRs);
						}
						else
						{
							strcpy(psValueStr, "");
							*pnValueLen = 0;
							return APP_FAIL;/* invalid  format  in  configuration  file  */
						}

					} // end if(strcmp(szKen, psItem) == 0)
				} // end if(ProGetKeyName(szLine, szKen, LINE_LEN))
				else
				{
					//return APP_FAIL;
				}
			} // else if(nStep == 1)
			else
			{
				nRa = ProGetKeyAdd(szLine, szKev + nRs, LINE_LEN - nRs);
				if(nRa > 0)
				{
					strncpy(psValueStr, szKev, nMaxStrLen);
					*pnValueLen = strlen(psValueStr);
					return APP_SUCC;
				}
				else if( nRa < 0 )
				{
					nRs += abs(nRa);
				}
				else
				{
					strcpy(psValueStr, "");
					*pnValueLen = 0;
					return APP_FAIL;
				}
			} // end else
		}  // end if(!ProIsremark(szLine))
	} // end while
	return APP_FAIL;
}

int GetRandValue(int nRange)
{
	srand( (unsigned)time( NULL ) );  
    return rand()%nRange+1;
}

int PINHASH(char* pszTEID , char * pszTEPIN, char* psHash)
{
	char szTmp[32+1];
	char szResult[20+1];
	int nlen = 0;;
	
	nlen = strlen(pszTEID);
	if(nlen!=8)
	{
		memset(szTmp, 0x30, sizeof(szTmp));	
		memcpy(szTmp+8-nlen,pszTEID,nlen);
	}
	else
	{
		memset(szTmp,0,sizeof(szTmp));
		memcpy(szTmp,pszTEID,nlen);
	}
	memcpy(szTmp+8,pszTEPIN,nlen);
	memcpy(szTmp+16,"1234",4);
	szTmp[20]=0;
	
	NDK_AlgSHA1((uchar *)szTmp, 20, (uchar *)szResult);
	PubHexToAsc((uchar *)szResult, 8, 0, (uchar *)psHash);
	return APP_SUCC;
}


int TXNHASH(char * pszHash1, char *pszHash, int nIndex)
{
	char szData[999] = {0};
	char szResult[20+1];
	char szSTAN[6+1] = {0};
	char szTID[8+1] = {0};

	memcpy(szData, pszHash1, 8);
	GetVarTerminalId(szTID, nIndex);
PubDebug("TID: %s", szTID);
	memcpy(szData+8, szTID, 8);
	GetVarTraceNo(szSTAN);
PubDebug("szSTAN: %s", szSTAN);
	memcpy(szData+16, szSTAN+2, 4);
	
	NDK_AlgSHA1((uchar *)szData, 20, (uchar *)szResult);
	PubHexToAsc((uchar *)szResult, 8, 0, (uchar *)pszHash);
	
	return APP_SUCC;
	
}


int TXNHASH3(char * pszHash1, char *psPubKey,int nKeyLen, char *pszHash3, int nIndex)
{
	char szData[999] = {0};
	char szResult[20+1];
	char szSTAN[6+1] = {0};
	char szTID[8+1] = {0};

	memcpy(szData, pszHash1, 8);
	GetVarTerminalId(szTID, nIndex);
	memcpy(szData+8, szTID, 8);
	GetVarTraceNo(szSTAN);
	memcpy(szData+16, szSTAN+2, 4);
	memcpy(szData+20, psPubKey, nKeyLen);
	
	NDK_AlgSHA1((uchar *)szData, 20+nKeyLen, (uchar *)szResult);

	pszHash3[0] = 0xA2;
	memcpy(pszHash3+1, szResult, 4);
	memset(pszHash3+5, 0x00, 3);
	
	return APP_SUCC;
	
}
void PINUpdateHash(char * hash1,char* hash2,char* hash)
{
	char bcdHash1[4+1];
	char bcdHash2[4+1];
	PubAscToHex((uchar *)hash1,8,0, (uchar *)bcdHash1);
	PubAscToHex((uchar *)hash2,8,0, (uchar *)bcdHash2);
	EdcProXor(bcdHash1,bcdHash2,4);
	PubHexToAsc((uchar *)bcdHash1,8,0, (uchar *)hash);
	hash[8]=0;
	return ;
}


int GetTerminalSn(char *psSn)
{
	uint nLen = 100;
	
	NDK_SysGetPosInfo(SYS_HWINFO_GET_POS_USN, &nLen, psSn);
PubDebug("sn: %s", psSn);
	return APP_SUCC;
}


int GetRandom(char *psRandom,int nLen)
{
	uchar *psBuf;
	int i;
	
	psBuf = (unsigned char *)psRandom;
	srand((unsigned)time(NULL));
	
	for(i = 0; i < nLen; i++)
	{
		*psBuf++ = (unsigned char)(rand()%256);
	}
	
	return APP_SUCC;
}

//only support 128 DE
int GetTagForDE(int nDE, uchar *sTag, int *pnTagLen)
{
	uchar ucNib[4] = {0};

	int nNum = nDE;
	int nCnt = 0;

	while (nNum != 0)
	{
		ucNib[nCnt] = GetOctFrmDec(nNum % 8);
		nNum = nNum / 8;
		nCnt++;
		
	}
	
	ucNib[1] <<= 4;
	ucNib[3] <<= 4;

	ucNib[0] |= ucNib[1];
	ucNib[1] = ucNib[2] | ucNib[3];

	*sTag = ucNib[0];
	sTag[1] = ucNib[1];
	
	if (0x00 != ucNib[1])
	{
		*pnTagLen = 2;
	}

	*pnTagLen = 1;
	
	return APP_SUCC;
}

int GetLengthTag(int nLength, uchar* psLen, int *pnLenLen)
{
	uchar ucInt[4] = {0};

	memset((void*)&ucInt, 0x00, sizeof(int));
	memcpy((void*)&ucInt, (void*)&nLength, sizeof(int));

	if ((0 <= nLength) && (127 >= nLength))
	{
		ucInt[0] |= 0x00;
		*psLen = ucInt[0];
		*pnLenLen = 1;
	}
	else if (( 128 <= nLength ) && (255 >= nLength ))
	{
		*psLen = 0x81; // set msb of first uint8_t, and the lower nibble indicates only 1 uint8_t of length to follow
		*(psLen+1) = ucInt[0];
		*pnLenLen = 2;
	}
	else if ((256 <= nLength) && (65535 >= nLength))
	{
		*psLen = 0x82; // set msb of first uint8_t, and the lower nibble indicates 2 uint8_ts of length to follow
		*(psLen + 1) = ucInt[1];
		*(psLen + 2) = ucInt[0];
		*pnLenLen = 3;
	}
	else
	{
		*pnLenLen = 0;
	}

	return APP_SUCC;
}

void PadZeroAtRight(char *psData, int *pnLen)
{
	int nLen = *pnLen;
		
	if (nLen % 8)
	{
		memcpy(&psData[nLen],"\x00\x00\x00\x00\x00\x00\x00\x00",(8-(nLen%8)));
		nLen += (8 - (nLen%8));
	}

	*pnLen = nLen;
}

int SaveToFile(const char *psInBuf,int nOff, int nLen, const char *pszFileName)
{
	int fp;

	if((fp=NDK_FsOpen(pszFileName,"w"))<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}
	
	if(NDK_FsSeek (fp,(long)nOff,SEEK_SET)<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}
	
	if(NDK_FsWrite(fp, psInBuf,nLen)<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}
	NDK_FsClose(fp);	
	return APP_SUCC;
}

int GetFromFile(char *psOutbuf,int nOff, int *pnLen, const char *pszFileName)
{
	int fp;
	int nReadLen = -1, nFileLen = 0;
	
	nFileLen = *pnLen;

	if((fp=NDK_FsOpen(pszFileName,"r"))<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}
	
	if(NDK_FsSeek (fp,(long)nOff,SEEK_SET)<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}
	nReadLen = NDK_FsRead(fp, psOutbuf,nFileLen);
	if(nReadLen<0)
	{
		NDK_FsClose(fp);
		return APP_FAIL;
	}

	*pnLen = nReadLen;
	NDK_FsClose(fp);	
	return APP_SUCC;
}

int SelectAcqIndex(char *pszTitle, int *pnIndex, YESORNO cIsNeedAll)
{
	int nEppMonth = 0;
	int nSel = 1;
	int nStartItem = 1;
	char *pszItems1[] = {
		tr("1.VISA/MASTER/JCB"),
		tr("2.AMEX"),
		tr("3.MYDEBIT"),
		tr("4.UPI"),
		tr("5.V/M EPP"),
		tr("6.AMEX EPP"),
		tr("7.ALL")
	};
	char *pszItems2[] = {
		tr("1.VISA/MASTER/JCB"),
		tr("2.AMEX"),
		tr("3.MYDEBIT"),
		tr("4.UPI"),
		tr("5.V/M EPP"),
		tr("6.AMEX EPP")
	};
	if (YES == cIsNeedAll)
	{
		ASSERT_QUIT(PubShowMenuItems(pszTitle, pszItems1, sizeof(pszItems1)/sizeof(char *), &nSel, &nStartItem, 60));
		//ASSERT_QUIT(ProSelectList("1.VISA/MASTER/JCB||2.AMEX||3.MYDEBIT||4.UPI||5.V/M EPP||6.AMEX EPP||7.ALL", pszTitle, 0xFF, &nSel));
	}
	else
	{
		ASSERT_QUIT(PubShowMenuItems(pszTitle, pszItems2, sizeof(pszItems2)/sizeof(char *), &nSel, &nStartItem, 60));
		//ASSERT_QUIT(ProSelectList("1.VISA/MASTER/JCB||2.AMEX||3.MYDEBIT||4.UPI||5.V/M EPP||6.AMEX EPP", pszTitle, 0xFF, &nSel));
	}

	switch (nSel)
	{
	case 5:
	case 6:
		*pnIndex = nSel - 1; 
		ASSERT_QUIT(SelectEppMonth(pszTitle, &nEppMonth));
		UpdateAcqIndexFromEpp(pnIndex, nEppMonth);
		break;
	case 7:
		*pnIndex = 0xFF; 
		break;
	default:
		*pnIndex = nSel - 1; 
		break;
	}
	
	return APP_SUCC;
}

void MaskCardNo(char *pszCardNo, int nCopyType)
{
	int i = 0;
	int nLen = strlen(pszCardNo);

	switch (nCopyType)
	{
	case 2:
		for(i = 0; i<(nLen-4);i++)//show last 4 letters
		{
			pszCardNo[i] = '*';
		}
		break;
	case 1:
	default:
		memset(pszCardNo + 6, '*', nLen - 6 - 4);
		break;
	}
}

/*
*	@brief          Put a Tag-Len-Value in oTlvBuf
*	@param          const uint		iTag
*	@param          const int		iLen
*	@param          const char		*iValue
*	@return			TLVERR_xxx
*/
int TlvAdd(const uint unTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen)
{
	int i, nTagLen,nLen, nLenLen;
	char sTag[5];
	char sLenBuf[8];
	sprintf(sTag, "%x", unTag);
	nTagLen = strlen(sTag);
	PubAscToHex((uchar *)sTag, nTagLen, 0, (uchar *)(pszOutTlvBuf+*pnOutTlvBufLen));
	nTagLen >>= 1;
	*pnOutTlvBufLen += nTagLen;

	if (nInLen > 127)
	{
		nLen = nInLen;
		nLenLen = nInLen/256+1;	
		sLenBuf[0] = 128 | nLenLen;
		for (i=0; i<nLenLen; i++)
		{
			sLenBuf[nLenLen-i] = nLen%256;
			nLen /= 256;
		}
	} else
	{
		sLenBuf[0] = nInLen;
		nLenLen = 0;
	}
	memcpy(pszOutTlvBuf+*pnOutTlvBufLen, sLenBuf, nLenLen+1);
	*pnOutTlvBufLen += nLenLen+1;

	memcpy(pszOutTlvBuf+*pnOutTlvBufLen, pszInValue, nInLen);
	*pnOutTlvBufLen += nInLen;
	
	return 0;
}

/**
* @brief Amout format change to display,like 000000000010 to 0,10
* @param in pszInAmt 
* @param out pszOutAmt 
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int ProHexPntToDIsp(const char *psPnt, char *pszDispPnt)
{
	int i;
	char pszPnt[12+1] = {0};

	PubHexToAsc((uchar *)psPnt, 12, 0, (uchar *)pszPnt);
	PubAllTrim(pszPnt);
	sprintf(pszDispPnt, "%lld", AtoLL(pszPnt));
	
	return APP_SUCC;
}

/**
* @brief Amout format change to display,like 000000000010 to 0,10
* @param in pszInAmt 
* @param out pszOutAmt 
* @return 
* @li APP_SUCC Success
* @li APP_FAIL Fail
*/
int ProHexAmtToDIsp(const char *psAmt, char *pszDispPnt)
{
	int i;
	char pszAmt[12+1] = {0};

	PubHexToAsc((uchar *)psAmt, 12, 0, (uchar *)pszAmt);

	ProAmtToDispOrPnt(pszAmt, pszDispPnt);
	
	return APP_SUCC;
}

/**
* @brief  bcd 2 string, result is  bcd 1
* @param [in] cType	0Plus，1Minus
* @param [in] psBcdAmt1
* @param [in] psBcdAmt2	
* @param [in] psBcdAmt2	
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
int MathBcd(uchar *psBcd1, const uchar *psBcd2, int nBcdLen, EM_MATH_TYPE emType)
{
	uchar szAmt[20+1] = {0}, szAmt1[20+1] = {0}, szAmt2[20+1] = {0};
	
	PubHexToAsc(psBcd1, nBcdLen*2, 0, szAmt1);
	PubHexToAsc(psBcd2, nBcdLen*2, 0, szAmt2);
	memset(szAmt, 0, sizeof(szAmt));
	switch (emType)
	{
	case EM_SUB:
		PubAscSubAsc(szAmt1, szAmt2, szAmt);
		break;
	case EM_PLUS:
	default:
		PubAscAddAsc(szAmt1, szAmt2, szAmt);
		break;
	}
	PubAddSymbolToStr(szAmt,12,'0',ADDCH_MODE_BEFORE_STRING);;
	PubAscToHex(szAmt, nBcdLen*2, 0, psBcd1);
	return APP_SUCC;
}

//increase 3byte bcd  
int IncreaseBcdNum(uchar *psBcd)
{
	char szNum[6+1] = {0};
	PubHexToAsc(psBcd, 6, 0, szNum);
	PubIncNum(psBcd);
	PubAscToHex(szNum, 6, 0, psBcd);
	return APP_SUCC;
}

void LongToIp(long lIp, char *psIp)
{
	char szTmp[4+1] = {0};
	
	szTmp[0] = (lIp & 0xFF000000) >> 24;
	szTmp[1] = (lIp & 0x00FF0000) >> 16;
	szTmp[2] = (lIp & 0x0000FF00) >> 8;
	szTmp[3] = (lIp & 0x000000FF);

	sprintf(psIp, "%d.%d.%d.%d", szTmp[0], szTmp[1], szTmp[2], szTmp[3]);
	
}

//设置休眠最多三次
int CommSetSuspend(uint unFlag)
{
	int nRet = -1;
	int i = 0;
	
	while (1)
	{
		if (i == 3)
		{
			break;
		}
		nRet = NDK_SysSetSuspend(unFlag);
		if (NDK_OK != nRet)
		{
			NDK_SysDelay(30);//延时3秒
			i++;
			continue;
		}
		return APP_SUCC;
	}
	return APP_FAIL;
}

int OpenPPP(void)
{
	int nRet;
	STAPPCOMMPARAM stAppCommParam;
	STSHOWINFOXY stShowInfoxy;
		
	GetAppCommParam(&stAppCommParam);
	if(stAppCommParam.cCommType == COMM_DIAL || stAppCommParam.cCommType == COMM_RS232)
	{
		PubMsgDlg("Info", "Currnet comm typr does not support HTTP", 3, 3);
		return APP_FAIL;
	}
	
	PubClear2To4();
	PubDisplayStrInline(0, 2, "Connect to network...");
	PubDisplayStrInline(0, 4, "<Cancel> to quit");
	PubUpdateWindow();
	memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
	stShowInfoxy.nColumn = 9;
	stShowInfoxy.nRow = 2;
	stShowInfoxy.nType = 0;
	PubSetShowXY(stShowInfoxy);
	CommSetSuspend(0);
	nRet = PubCommDialNet();
	if (nRet != APP_SUCC )
	{
		PubDispErr("fail to open ppp");
		return APP_FAIL;
	}

	return APP_SUCC;
}
	
int ClosePPP(int nHangFlag)
{
	STAPPCOMMPARAM stAppCommParam;	
	memset(&stAppCommParam, 0, sizeof(stAppCommParam));
	
	GetAppCommParam(&stAppCommParam);
	CommSetSuspend(1);
	if (COMM_GPRS == stAppCommParam.cCommType || COMM_CDMA == stAppCommParam.cCommType)
	{
		if (stAppCommParam.cMode == 1 || stAppCommParam.cMode == '1' )
		{
			return APP_SUCC;
		}
		else/**<短链接，需要挂断PPP*/
		{
			return NDK_PppHangup(nHangFlag);
		}			
	}
	return APP_SUCC;
}

/*
*通过OPENSSL命令行实现获取签名
*/
int GetRsaValue(char *pszInData, int nInLen, char *pszOutData, int *pnOutLen)
{
	int nFd=-1;
	char szCmd[256+1]={0};

	nFd = NDK_FsOpen("data.txt", "w");
	NDK_FsWrite(nFd, pszInData, nInLen);
	NDK_FsClose(nFd);

	strcpy(szCmd, "openssl dgst -sign rsa_private_key.pem -sha256 -out data.bin data.txt");
	system(szCmd);
	
//	strcpy(szCmd, "openssl base64 -in data.bin -out base64.txt");
//	system(szCmd);

	nFd = NDK_FsOpen("data.bin", "r");
//	nFd = NDK_FsOpen("base64.txt", "r");
	*pnOutLen = NDK_FsRead(nFd, pszOutData,  1024);
TRACE(("EN= %d", *pnOutLen));
PubDebugData("data: ", pszOutData, *pnOutLen);
	NDK_FsClose(nFd);

	NDK_FsDel("data.txt");
	NDK_FsDel("data.bin");
//	NDK_FsDel("base64.txt");

	return APP_SUCC;
}

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*@brief		
*@param		[in]			bindata
*@param		[in]			binlength
*@param		[out]			base64
*/
int base64_encode( const unsigned char * bindata, int binlength, char * base64)
{
	int i, j;
	unsigned char current;

	for ( i = 0, j = 0 ; i < binlength ; i += 3 )
	{
		current = (bindata[i] >> 2) ;
		current &= (unsigned char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
		if ( i + 1 >= binlength )
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
		if ( i + 2 >= binlength )
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
		base64[j++] = base64char[(int)current];

		current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
		base64[j++] = base64char[(int)current];
	}
	base64[j] = '\0';
	return APP_SUCC;
}


/*@brief		
*@param		[in]		base64
*@param		[out]		bindata
*@param 	[out]		nLen_bindata
*/
int base64_decode( const char * base64, unsigned char * bindata, int* nLen_bindata)
{
	int i, j;
	unsigned char k;
	unsigned char temp[4];
	for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
	{
		memset( temp, 0xFF, sizeof(temp) );
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i] )
				temp[0]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+1] )
				temp[1]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+2] )
				temp[2]= k;
		}
		for ( k = 0 ; k < 64 ; k ++ )
		{
			if ( base64char[k] == base64[i+3] )
				temp[3]= k;
		}

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
			((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
		if ( base64[i+2] == '=' )
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
			((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
		if ( base64[i+3] == '=' )
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
		((unsigned char)(temp[3]&0x3F));
	}

	*nLen_bindata = j;
	
	return 0;
}


