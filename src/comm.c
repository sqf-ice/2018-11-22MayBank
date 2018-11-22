/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  comm.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-7-4
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
static STAPPCOMMPARAM gstAppCommParam; /**<communication parameter of application*/

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

/**
* internal function
*/
static int CommParamAppToPub(const STAPPCOMMPARAM *, STCOMMPARAM *);
static int SetFuncCommMode(void);
static int SetFuncCommIsPreDial(void);
static int SetFuncCommTimeOut(void);
static int SetFuncCommTelNo(void);
static int SetFuncCommWLMDialNum(void);
static int SetFuncCommUser(void);
static int SetFuncCommPassWd(void);
static int SetFuncCommAPN(void);
static int SetFuncCommPort(void);
static int SetFuncCommBackPort(void);
static int SetFuncCommIp(void);
static int SetFuncCommBackIp(void);
static int SetFuncCommIpAddr(void);
static int SetFuncCommMask(void);
static int SetFuncCommGate(void);
static int SetFuncCommReDialNum(void);
static int SetFuncAuxIsAddTpdu(void);
static int SetFuncWifiMode(void);
static int SetFuncWifiPwd(void);
static int SetFuncWifiSsid(void);
static int SetFuncIsDhcp(void);
static YESORNO GetControlCommInit(void);
static int SetFuncDomainName(void);
static int SetFuncCommDnsIp(void);
static int SetFuncQRDomainName(void);


/**
* Interface function implementation
*/
static char gcIsCommInit = YES;/**<to record if need to initialize, YES-need*/

/**
** brief: ConTrol need to initialize commucation
** param: void
** return: void
** auther: 
** date: 2016-7-4
** modify: 
*/
void SetControlCommInit(void)
{
	gcIsCommInit = YES;
}

/**
** brief: Get the flag if need to initialize
** param: void
** return: YESORNO
** auther: 
** date: 2016-7-4
** modify: 
*/
static YESORNO GetControlCommInit(void)
{
	if (gcIsCommInit==YES)
	{
		return YES;
	}
	else
	{
		return NO;
	} 
}

/**
** brief: Set default communication parameters, it should run one time at first run.
** param: void
** return: 
** li APP_SUCC
** li APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int InitCommParam(void)
{
	STRECFILE stCommParamfile;
	char szBuf[50] = {0};
	
	memset(&gstAppCommParam, 0, sizeof(STAPPCOMMPARAM));
	if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS,szBuf))
	{
		gstAppCommParam.cCommType = COMM_DIAL;
	}
	else if (0 == memcmp(szBuf, "GPRS", 4))
	{
		gstAppCommParam.cCommType = COMM_GPRS;
	}
	else if (0 == memcmp(szBuf, "CDMA", 4))
	{
		gstAppCommParam.cCommType = COMM_CDMA;
	}
	else
	{
		gstAppCommParam.cCommType = COMM_DIAL;
	}

	gstAppCommParam.cPreDialFlag = 1;
	strcpy(gstAppCommParam.szPreDial,"");
	strcpy(gstAppCommParam.szTelNum1, "66522028");
	strcpy(gstAppCommParam.szTelNum2, "66522028");
	strcpy(gstAppCommParam.szTelNum3, "66522028");
	gstAppCommParam.cTimeOut = 60;
	memcpy(gstAppCommParam.sTpdu, "\x60\x00\x12\x00\x00", 5);
	
	gstAppCommParam.cReDialNum = 3;
	gstAppCommParam.cReSendNum = 3;
	gstAppCommParam.cOffResendNum=3;
	strcpy(gstAppCommParam.szAPN1, "MyEvo");
	strcpy(gstAppCommParam.szAPN2, "CMNET");
	strcpy(gstAppCommParam.szDNSIp1, "8.8.8.8");
	strcpy(gstAppCommParam.szDNSIp2, "8.8.8.8");
	strcpy(gstAppCommParam.szDNSIp3, "8.8.8.8");
	strcpy(gstAppCommParam.szIp1, "1.9.32.82");
	strcpy(gstAppCommParam.szIp2, "1.9.32.82");
	strcpy(gstAppCommParam.szPort1, "12396");
	strcpy(gstAppCommParam.szPort2, "12396");
	strcpy(gstAppCommParam.szIpAddr, "192.168.5.114");
	strcpy(gstAppCommParam.szMask, "255.255.255.0");
	strcpy(gstAppCommParam.szGate, "192.168.5.254");
	gstAppCommParam.cMode = 1;
	
	if (gstAppCommParam.cCommType == COMM_GPRS)
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D*99***1#");/**<"D*99***1#"*/
	}
	else if (gstAppCommParam.cCommType == COMM_CDMA)
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D#777");/**<"D#777"*/
		gstAppCommParam.cMode = 0;
	}
	else
	{
		strcpy(gstAppCommParam.szWirelessDialNum, "D*99***1#");/**<"D*99***1#"*/
	}	
	strcpy(gstAppCommParam.szUser, "card");
	strcpy(gstAppCommParam.szPassWd, "card");
	strcpy(gstAppCommParam.szSIMPassWd, "1234");
	strcpy(gstAppCommParam.szDomain, "ssl1.tnsi.com.my");
	gstAppCommParam.cAuxIsAddTpdu= 1;

	gstAppCommParam.cIsDHCP = 1;
	strcpy(gstAppCommParam.szWifiSsid,"ssid");
	strcpy(gstAppCommParam.szWifiKey,"password");
	gstAppCommParam.cWifiKeyType = WIFI_KEY_TYPE_ASCII;
	gstAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_OPEN;
	memcpy(gstAppCommParam.szDomain2,gstAppCommParam.szDomain,50);
	memcpy(gstAppCommParam.szDnsPort1,gstAppCommParam.szPort1,6);
	memcpy(gstAppCommParam.szDnsPort2,gstAppCommParam.szPort2,6);

	memcpy(gstAppCommParam.szSecurityNii,"101",3); // 003
	memcpy(gstAppCommParam.sSecurityTpdu, "\x60\x05\x55\x00\x00", 5);
	memcpy(gstAppCommParam.szTransNii[0],"111",3);

	gstAppCommParam.cIsDnsAna = 0;
	strcpy(gstAppCommParam.szOrDomain, "https://www.inwc.ktb.co.th/");//https://172.27.0.251:3840/

	/**<creat communication parameters file */	
	memset(&stCommParamfile, 0, sizeof(STRECFILE));
	strcpy( stCommParamfile.szFileName, FILE_APPCOMMPARAM);
	stCommParamfile.cIsIndex = FILE_NOCREATEINDEX;				
	stCommParamfile.unMaxOneRecLen = sizeof(STAPPCOMMPARAM);
	stCommParamfile.unIndex1Start = 0;
	stCommParamfile.unIndex1Len = 0;
	stCommParamfile.unIndex2Start = 0;
	stCommParamfile.unIndex2Len = 0;
	ASSERT_FAIL(PubCreatRecFile(&stCommParamfile));	
	ASSERT_FAIL(PubAddRec(FILE_APPCOMMPARAM,(char *)&gstAppCommParam));


	return APP_SUCC;
}

int SetTmpDomain(char *pszDomain)
{
	strcpy(gstAppCommParam.szOrDomain, pszDomain);
}


/**
** brief: Initialize communication, like init modem,WIFI...
** param: void
** return: void
** li APP_SUCC
** li APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int CommInit(void)
{
	int nRet = 0;
	STCOMMPARAM stPubCommParam;
	static STCOMMPARAM stOldPubCommParam = {0};
	static char cInitFlag = 0;
	
	memset(&stPubCommParam, 0, sizeof(STCOMMPARAM));
	CommParamAppToPub(&gstAppCommParam, &stPubCommParam);

	if (memcmp(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM)) != 0 || 0 == cInitFlag || YES == GetControlCommInit())
	{
		SetStatusBar(STATUSBAR_OTHER_OPEN);	
		PubClearAll();
		PubDisplayGen(2, "Init communication");
		PubDisplayGen(3, "Please wait...");
		PubUpdateWindow();		
		nRet = PubCommInit(&stPubCommParam);
		if (nRet != APP_SUCC)
		{
			PubDispErr("Comm Fail");
			cInitFlag = 0;
			return APP_FAIL;
		}
		memcpy(&stOldPubCommParam, &stPubCommParam, sizeof(STCOMMPARAM));/**<save the old parameters*/
		cInitFlag = 1;
		gcIsCommInit = NO;
	}

	return APP_SUCC;
}

/**
* @brief Dial in advance
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommPreDial(void)
{	
	int nRet = 0;
	
PubDebug("0+++++++++++++++++");
return 0;//shtest
	ASSERT_FAIL(SetSslMode());
	
	nRet = PubCommPreConnect();
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;

}

/**
* @brief 处理Ssl模块
* @param 无
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetSslMode(void)
{
	STSSLMODE stSslMode;

	if(IsSslEnable() == YES)
	{				
		memset(&stSslMode, 0, sizeof(STSSLMODE));
		stSslMode.nAuthOpt = SSL_AUTH_NONE;
		stSslMode.nType = HANDSHAKE_SSLv23;
		return PubSslSetMode(&stSslMode);
	}
	return APP_SUCC;
}

/**
* @brief communication connect
* @param  void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommConnect(void)
{
	int nRet = 0;
	STSHOWINFOXY stShowInfoxy;
	
PubDebug("0+++++++++++++++++");
	ASSERT_FAIL(SetSslMode());
	
	PubClear2To4();
	if(COMM_DIAL == gstAppCommParam.cCommType)
	{
		PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Dialing...");
	}
	else
	{
		PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Connecting...");
	}
//	PubDisplayStrInline(0, 4, "[CANCEL] To Quit");
	PubUpdateWindow();

	stShowInfoxy.nType = 0;
	stShowInfoxy.nColumn = 9;
	stShowInfoxy.nRow = 3;
	PubSetShowXY(stShowInfoxy);
	
	nRet = PubCommConnect();
	if (nRet == APP_QUIT)
	{
		return APP_QUIT;
	}
	if (nRet != APP_SUCC)
	{
		if (nRet != APP_SUCC)
		{
			PubDispErr("Comm");
			return APP_FAIL;
		}
	}
	
	return APP_SUCC;
}


/**
* @brief add tpdu
* @param [in] [out] psBuf,input original data ，output data after adding tpdu
* @param [in] [out] punLen,input original data，output datalen after adding tpdu
* @return
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
static int AddHeader(char *psBuf, uint *punLen)
{
	char sTemp[MAX_SEND_SIZE]={0};

	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf, *punLen);
	memcpy(psBuf, gstAppCommParam.sTpdu, 5);
PubDebugData("tpdu: ", gstAppCommParam.sTpdu, 5);
	memcpy(psBuf + 5, sTemp, *punLen);
	*punLen += 5;
	return APP_SUCC;
}

/**
* @brief delete tpdu
* @param in out psBuf,input original data ，output data after deleting tpdu
* @return in out punLen,input original data，output datalen after deleting tpdu
* @li APP_SUCC
* @li APP_FAIL
*/
static int DelTpdu(char *psBuf, uint *punLen)
{
	char sTemp[MAX_SEND_SIZE];

	if (NULL == psBuf ||NULL == punLen)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	*punLen -= 5;
	memcpy(sTemp, psBuf + 5, *punLen);
	memcpy(psBuf, sTemp, *punLen);
	return APP_SUCC;
}

/**
* @brief Send data
* @param in psSendBuf 
* @param in nSendLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommSend(const char *psSendBuf, int nSendLen)
{
	int nRet = 0;
	int nLen = 0;
	char sSendBuf[MAX_SEND_SIZE];	

	memcpy(sSendBuf, psSendBuf, nSendLen);
	nLen = nSendLen;	
	AddHeader(sSendBuf, (uint *)&nLen);
	//AddCRC(sSendBuf, (uint *)&nLen);
	PubCommClear();
	
	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Sending...");
	PubUpdateWindow();
	
	if (YES == GetVarIsPrintIso() && APP_SUCC == PubConfirmDlg(NULL, "Print ISO?", 0, 10))
	{
		PrintRawData(sSendBuf, nLen);
		PrintIsoData(psSendBuf);
	}
		
	if(IsSslEnable() == YES)
	{
PubDebugData("before pack: , ", sSendBuf, nLen);
		SslRepack(sSendBuf, &nLen);
PubDebugData("after pack: , ", sSendBuf, nLen);
	}

	nRet = PubCommSend(sSendBuf, nLen);
	
	if (nRet != APP_SUCC)
	{
		PubDispErr("COMM FAIL");
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Revieve data
* @param out psRecvBuf 
* @param out nRecvLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommRecv(char *psRecvBuf, int *nRecvLen)
{
	int nRet = 0;
	int nLen = 0;
	char sRecvBuf[MAX_RECV_SIZE+1] = {0};
	char *psRecv = sRecvBuf;
	STSHOWINFOXY stShowInfoxy;
	int nDebugFlag = 0;

	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Receiving...");
	PubUpdateWindow();
	stShowInfoxy.nType = 0;
	stShowInfoxy.nColumn = 9;
	stShowInfoxy.nRow = 3;
	PubSetShowXY(stShowInfoxy);
	
	nRet = PubCommRecv(sRecvBuf, &nLen);
	if(nRet != APP_SUCC)
	{
		PubDispErr("COMM FAIL");
		return APP_FAIL;
	}

	if (YES == GetVarIsPrintIso() && APP_SUCC == PubConfirmDlg(NULL, "Print ISO?", 0, 10))
	{
		nDebugFlag = 1;
	}
	if (1 == nDebugFlag)
	{
		PrintRawData(psRecv, nLen);
	}

	nRet = DelTpdu(psRecv, (uint *)&nLen);
	if (APP_SUCC == nRet)
	{
		memcpy(psRecvBuf, psRecv, nLen);
		*nRecvLen = nLen;
		if (1 == nDebugFlag)
		{
			Unpack(psRecvBuf, nLen);
			PrintIsoData(psRecvBuf);
		}
		return APP_SUCC;
	}

	return APP_FAIL;
}

/**
* @brief send and recieve data
* @param in psSendBuf
* @param in nSendLen 
* @param out psRecvBuf 
* @param out nRecvLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommSendRecv(const char *psSendBuf, int nSendLen, char *psRecvBuf, int *nRecvLen)
{
	int nRet = 0;
	nRet = CommSend(psSendBuf, nSendLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	nRet = CommRecv(psRecvBuf, nRecvLen);

	CommHangUpSocket();

	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}

	return APP_SUCC;

}

/**
* @brief Hangup communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommHangUp(void)
{
	if (COMM_GPRS == gstAppCommParam.cCommType || COMM_CDMA == gstAppCommParam.cCommType)
	{
		if (gstAppCommParam.cMode == 1 || gstAppCommParam.cMode == '1' )
		{
			;
		}
		else/**<not alive，hangup PPP*/
		{
			return PubCommClose();
		}			
	}
	return PubCommHangUp();
}

/**
* @brief 
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommHangUpSocket(void)
{
	if (COMM_DIAL != gstAppCommParam.cCommType)
	{
		PubCommHangUp();
	}
	
	return APP_SUCC;
}

/**
* @brief hangup communication
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CommDump(void)
{
	if (COMM_DIAL == gstAppCommParam.cCommType)
	{
		return PubCommHangUp();
	}	
	else
	{
		return PubCommClose();
	}
}

/**
* @brief set predial number and initialize
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
int DoSetPreDialNum(void)
{	
	SetFuncCommPreDialNum();
	return CommInit();
}

/**
* @brief set predial number
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommPreDialNum(void)
{	
	char szTemp[14+1] = {0};
	int nLen = 10;

	strcpy(szTemp, gstAppCommParam.szPreDial);
	ASSERT_RETURNCODE(PubInputDlg("SET COMM", "Exterior line:", szTemp, \
		&nLen, 0, 10, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szPreDial, szTemp, sizeof(gstAppCommParam.szPreDial)-1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM, 1, (char *)&gstAppCommParam));

	return APP_SUCC;
} 

/**
* @brief  communicaiton setting menu
* @param  void
* @return    void
*/
void CommMenu(void)
{
    int nSelect;
    int nRet;
	char szFunKey[2+1]={0};
	szFunKey[0] = '0';
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.COMMTYPE"),
		tr("2.NII"),
		tr("3.MODEM"),
		tr("4.GPRS"),
		tr("5.CDMA"),
		tr("6.ETH"),
		tr("7.RS232"),
		tr("8.WIFI"),
		tr("9.OTHER")
	};
	while(1)
	{
		//nRet = PubSelectListItemExtend("1.COMMTYPE|2.NII|3.MODEM|4.GPRS|5.CDMA|6.ETH|7.RS232|8.WIFI|9.OTHER", "SET COMM",NULL,0xFF,&nSelect,60,szFunKey);
		nRet = PubShowMenuItems(tr("SET COMM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return;
		}
		switch(nSelcItem)
		{
		case 1:
			SetFuncCommType(NO);
			break;
		case 2:
			SetFuncCommSecurityNii();
			SetFuncCommTransNii();
			break;
		case 3:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommTelNo, 
				DoSetPreDialNum, 
				SetFuncCommIsPreDial,
				NULL};		
				PubUpDownMenus(lSetFuns);
			}
			break;
		case 4:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommWLMDialNum,
				SetFuncCommAPN,
				SetFuncCommIp,
				SetFuncDomainName,
				SetFuncCommDnsIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
			    SetFuncCommUser,
				SetFuncCommPassWd,
				SetFuncCommMode, 
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		case 5:
			{
			int (*lSetFuns[])(void) = {
				SetFuncCommWLMDialNum,
				SetFuncCommIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
				SetFuncCommUser,
				SetFuncCommPassWd,
				SetFuncCommMode, 
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		case 6:
			{
			int (*lSetFuns[])(void) = {
				SetFuncIsDhcp,
				SetFuncCommIpAddr, 
			    SetFuncCommGate,
			    SetFuncCommMask,
				SetFuncCommIp,
				SetFuncCommPort,
			    SetFuncCommBackIp,
				SetFuncCommBackPort,
				NULL};
			   	PubUpDownMenus(lSetFuns);
			}
			break;
		case 7:
			SetFuncAuxIsAddTpdu();
			break;
		case 8:
			{
			int (*lSetFuns[])(void) = {
			    SetFuncWifiSsid, 
			    SetFuncWifiPwd,
			    SetFuncIsDhcp,
				SetFuncCommIpAddr,
				SetFuncCommMask,
				SetFuncCommGate,
			    SetFuncCommIp,
				SetFuncCommPort,
				SetFuncCommBackIp,
				SetFuncCommBackPort,
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;	
		case 9:
			{
			int (*lSetFuns[])(void) = {
				SetIsSSLEnable,
				SetFuncCommSecurityTpdu,
				SetFuncCommTransTpdu,
				SetFuncCommTimeOut,
				SetFuncCommReDialNum,
				SetFuncQRDomainName,
				NULL};
			    PubUpDownMenus(lSetFuns);
			}
			break;
		default:
			break;
		}
	}
}

/**
* @brief Set type of communication 
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommType(char cIsSetMore)
{	
	int nSelect = 0;
	char szInfo[64]={0};
	char cOldCommType = gstAppCommParam.cCommType;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.MODEM"),
		tr("2.GPRS"),
		tr("3.CDMA"),
		tr("4.RS232"),
		tr("5.ETH"),
		tr("6.WIFI")
	};
	while(1)
	{
		PubShowMenuItems(tr("SET COMM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		//ASSERT_FAIL(ProSelectList("1.MODEM|2.GPRS|3.CDMA|4.RS232|5.ETH|6.WIFI", "SET COMM", \
				//gstAppCommParam.cCommType, &nSelect));
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		switch(nSelect)
		{
		case 1:
			if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM, NULL))
			{
				PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
				return APP_FAIL;
			}
			gstAppCommParam.cCommType = COMM_DIAL;
			if (YES == cIsSetMore)
			{
				SetModemParam();	
			}
			break;
		case 2:
			if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
			{
				if(memcmp(szInfo, "GPRS", 4) == 0)
				{
					gstAppCommParam.cCommType = COMM_GPRS;
					if (YES == cIsSetMore)
					{
						SetGPRSParam();	
					}
					break;
				}
			}
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		case 3:
			if(APP_SUCC == PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo))
			{
				if(memcmp(szInfo, "CDMA", 4) == 0)
				{
					gstAppCommParam.cCommType = COMM_CDMA;
					break;
				}
			}
			PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
			return APP_FAIL;
		case 4:
			gstAppCommParam.cCommType = COMM_RS232;
			break;
		case 5:
			if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH, NULL))
			{
				PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
				return APP_FAIL;
			}
			gstAppCommParam.cCommType = COMM_ETH;
			break;
		case 6:
			if(APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL))
			{
				PubMsgDlg(NULL, "NONSUPPORT", 0, 3);
				return APP_FAIL;
			}
			gstAppCommParam.cCommType = COMM_WIFI;
			break;
		default:
			return APP_QUIT;
		}
	}
	
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	if (cOldCommType != gstAppCommParam.cCommType)
	{
		CommHangUp();
	}
	/**
	* do initialize
	*/
	if (CommInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	return APP_SUCC;
} 

/**
* @brief set resend time
* @param void
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncCommReSendNum(void)
{	
	int nLen = 0,nRet;
	char szTemp[3+1];
		
	for (;;)
	{
		memset(szTemp, 0, sizeof(szTemp));
		sprintf(szTemp, "%d", gstAppCommParam.cReSendNum >= 0x30 ? (gstAppCommParam.cReSendNum - 0x30)%9 : gstAppCommParam.cReSendNum%9);
		ASSERT_RETURNCODE( PubInputDlg("REVERSAL RESEND", "NUM(<=3):", szTemp, \
			&nLen, 1, 1, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if ((nRet >= 1 )&& (nRet <= 3))
		{
			break; 
		}
		PubMsgDlg("REVERSAL RESEND","INVALID INPUT.TRY AGAIN!",0,1);
	}
	gstAppCommParam.cReSendNum = nRet;
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	return APP_SUCC;
} 

/**
* @brief change app parameter structure to  pub_lib parameter strcuture
* @param in pstApp ,app parameter structure
* @param in pstCommParam  ,pub_lib parameter strcuture
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
static int CommParamAppToPub(const STAPPCOMMPARAM *pstApp, STCOMMPARAM *pstCommParam)
{
	pstCommParam->cPreDialFlag = pstApp->cPreDialFlag == '1' ||pstApp->cPreDialFlag == 1 ? 1 : 0;
	pstCommParam->nTimeOut = pstApp->cTimeOut;
	pstCommParam->cReDialNum = pstApp->cReDialNum > 0x30 ? pstApp->cReDialNum - 0x30 : pstApp->cReDialNum;
	pstCommParam->cMode = pstApp->cMode == '1' || pstApp->cMode == 1 ? 0x01 : 0x02;
	pstCommParam->cIsSupportQuitRecv = SERVERMODE_CLOSENOWAIT|SERVERMODE_INITNOWAIT; //SERVERMODE_RECVQUIT
	pstCommParam->cSslFlag = pstApp->cIsSSL;
	switch(pstApp->cCommType)
	{
	case COMM_RS232:
		pstCommParam->cCommType= COMMTYPE_PORT_ASYN1;
		pstCommParam->ConnInfo.stPortParam.nAux = PORT_NUM_COM1;
		strcpy(pstCommParam->ConnInfo.stPortParam.szAttr, "9600,8,N,1");
		memcpy(pstCommParam->ConnInfo.stPortParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_DIAL:
		pstCommParam->cCommType = COMMTYPE_SYNDIAL;
		memcpy(pstCommParam->ConnInfo.stDialParam.szPredialNo, pstApp->szPreDial, 10);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[0], pstApp->szTelNum1, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[1], pstApp->szTelNum2, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lszTelNo[2], pstApp->szTelNum3, 14);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[0], pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[1], pstApp->sTpdu, 5);
		memcpy(pstCommParam->ConnInfo.stDialParam.lsTPDU[2], pstApp->sTpdu, 5);
		pstCommParam->ConnInfo.stDialParam.nCycTimes = 3;
		pstCommParam->ConnInfo.stDialParam.nCountryid = 5;
		break;
	case COMM_GPRS:
		pstCommParam->cCommType = COMMTYPE_GPRS_HEADLEN;
		memcpy(pstCommParam->ConnInfo.stGprsParam.szNetUsername, pstApp->szUser, 40);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szNetPassword, pstApp->szPassWd, 20);
		memcpy(pstCommParam->ConnInfo.stGprsParam.szPinPassWord, pstApp->szSIMPassWd, 10);
		if(pstApp->szWirelessDialNum[0] == 'D' || pstApp->szWirelessDialNum[0] == 'd')
		{
			memcpy(pstCommParam->ConnInfo.stGprsParam.szModemDialNo, pstApp->szWirelessDialNum+1, 20);
		}
		else
		{
			memcpy(pstCommParam->ConnInfo.stGprsParam.szModemDialNo, pstApp->szWirelessDialNum, 20);
		}
		sprintf(pstCommParam->ConnInfo.stGprsParam.szGprsApn, "%s", pstApp->szAPN1);
		memcpy(pstCommParam->ConnInfo.stGprsParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_CDMA:
		pstCommParam->cCommType = COMMTYPE_CDMA_HEADLEN;
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szNetUsername, pstApp->szUser, 40);
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szNetPassword, pstApp->szPassWd, 20);
		memcpy(pstCommParam->ConnInfo.stCdmaParam.szPinPassWord, pstApp->szSIMPassWd, 10);
		if(pstApp->szWirelessDialNum[0] == 'D' || pstApp->szWirelessDialNum[0] == 'd')
		{
			memcpy(pstCommParam->ConnInfo.stCdmaParam.szModemDialNo, pstApp->szWirelessDialNum+1, 20);
		}
		else
		{
			memcpy(pstCommParam->ConnInfo.stCdmaParam.szModemDialNo, pstApp->szWirelessDialNum, 20);
		}
		memcpy(pstCommParam->ConnInfo.stCdmaParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_ETH:
		pstCommParam->cCommType = COMMTYPE_ETH_HEADLEN;//COMMTYPE_ETH_TPDU_HEADLEN;
		if(pstApp->cIsDHCP != 0)
		{
			pstCommParam->ConnInfo.stEthParam.nDHCP = 1;
		}
		else
		{
			pstCommParam->ConnInfo.stEthParam.nDHCP = 0;
			memcpy(pstCommParam->ConnInfo.stEthParam.szIP, pstApp->szIpAddr, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szGateway, pstApp->szGate, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szMask, pstApp->szMask, 16);
			memcpy(pstCommParam->ConnInfo.stEthParam.szDNS, pstApp->szDNSIp1, 16);
		}
		memcpy(pstCommParam->ConnInfo.stEthParam.sTPDU, pstApp->sTpdu, 5);
		break;
	case COMM_WIFI:
		pstCommParam->cCommType = COMMTYPE_WIFI_HEADLEN;
		if(pstApp->cIsDHCP == '1' || pstApp->cIsDHCP == 1)
		{
			pstCommParam->ConnInfo.stWifiParam.ucIfDHCP = 1;
		}
		else
		{
			pstCommParam->ConnInfo.stWifiParam.ucIfDHCP = 0;
			memcpy(pstCommParam->ConnInfo.stWifiParam.szIP,pstApp->szIpAddr, 16);
			memcpy(pstCommParam->ConnInfo.stWifiParam.szGateway,pstApp->szGate, 16);
			memcpy(pstCommParam->ConnInfo.stWifiParam.szMask,pstApp->szMask, 16);
			strcpy(pstCommParam->ConnInfo.stWifiParam.szDnsPrimary,pstApp->szDNSIp1);
		}		
		pstCommParam->ConnInfo.stWifiParam.emKeyType = pstApp->cWifiKeyType;
		pstCommParam->ConnInfo.stWifiParam.emSecMode = pstApp->cWifiMode;
		memcpy(pstCommParam->ConnInfo.stWifiParam.szSsid,pstApp->szWifiSsid,32);
		memcpy(pstCommParam->ConnInfo.stWifiParam.szKey,pstApp->szWifiKey,32);
		memcpy(pstCommParam->ConnInfo.stWifiParam.sTPDU,pstApp->sTpdu, 5);
		break;
	default:
		return APP_FAIL;
	}
	
	if(pstApp->cIsDns == 0)
	{
		memcpy(pstCommParam->stServerAddress.lszIp[0], pstApp->szIp1, 16);
		memcpy(pstCommParam->stServerAddress.lszIp[1], pstApp->szIp2, 16);
	}
	else
	{
		memcpy(pstCommParam->stServerAddress.szDN, pstApp->szDomain, 50);
		memcpy(pstCommParam->stServerAddress.szDNSIp, pstApp->szDNSIp1, 16);
	}
	pstCommParam->stServerAddress.lnPort[0] = atoi(pstApp->szPort1);
	pstCommParam->stServerAddress.lnPort[1] = atoi(pstApp->szPort2);
	
	pstCommParam->ShowFunc = NULL;
	return APP_SUCC;
}

/**
* @brief set ppp mode
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommMode(void)
{	
	char cSelect = 0;
	char *SelMenu[] = {
		"0.BREAK",
		"1.ALIVE"
	};
	cSelect = gstAppCommParam.cMode == '1' || gstAppCommParam.cMode == 1 ? 1 : 0;
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET COMM", "PPP MODE", "0.BREAK|1.ALIVE", &cSelect));//NO是长连接
	ASSERT_RETURNCODE(PubSelectYesOrNo((char *)tr("SET COMM"), (char *)tr("PPP MODE"), SelMenu, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cMode = 0;
		break;
	case '1':
		gstAppCommParam.cMode = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	return APP_SUCC;
} 


/**
* @brief Set is need to predial
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIsPreDial(void)
{	
	char cSelect = 0;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	cSelect = gstAppCommParam.cPreDialFlag == '1' || gstAppCommParam.cPreDialFlag == 1 ? 1 : 0;
	//ASSERT_RETURNCODE( ProSelectYesOrNo("SET COMM", "Predictive Dialing", "0.NO|1.YES", &cSelect));
	ASSERT_RETURNCODE(PubSelectYesOrNo((char *)tr("SET COMM"), (char *)tr("Predictive Dialing"), SelMenu, &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cPreDialFlag = 0;
		break;
	case '1':
		gstAppCommParam.cPreDialFlag = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
} 

/**
* @brief set timeout
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommTimeOut(void)
{	
	int nRet = 0;
	char szTemp[3+1];
	int nLen = 0;

	for (;;)
	{
		memset(szTemp, 0, sizeof(szTemp));
		sprintf(szTemp, "%d", (uint)gstAppCommParam.cTimeOut);
		ASSERT_RETURNCODE(PubInputDlg("SET COMM", "Communication timeout:", szTemp, \
			&nLen, 2, 2, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 10 || nRet >= 100)
		{
			continue; 
		}
		gstAppCommParam.cTimeOut = nRet;
		break;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
} 

/**
* @brief set redial time
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommReDialNum(void)
{
	int nLen;
	char szNumber[2+1] = {0};

	sprintf(szNumber, "%d", gstAppCommParam.cReDialNum > 0X30 ? gstAppCommParam.cReDialNum - 0X30 : gstAppCommParam.cReDialNum );
	ASSERT_RETURNCODE(PubInputDlg("SET COMM", "Redial times:", szNumber, &nLen, 1, 1, 60, INPUT_MODE_NUMBER));
	gstAppCommParam.cReDialNum = atoi(szNumber);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}
/**
* @brief set offline resend time
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFunctionOffResendNum(void)
{
	char szNum[2] = {0};
	int nNum, nLen;
		
	while(1)
	{	
		szNum[0] = gstAppCommParam.cOffResendNum+'0';
		ASSERT_RETURNCODE( PubInputDlg("OFFLINE RESEND", "NUM(<=9):", szNum, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
		nNum=atoi(szNum);
		if( (nNum >= 1) && (nNum <= 9) )
		{
			break;
		}
		PubMsgDlg("OFFLINE RESEND","INVALID INPUT,TRY AGAIN!",0,1);
	}
	gstAppCommParam.cOffResendNum = nNum;	
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief Set telephone number
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommTelNo(void)
{	
	char szTemp[19+1] = {0};
	int nLen=0, i, nRet;
	char *p = NULL;
	char szContent[16+1] = {0};

	p = gstAppCommParam.szTelNum1;
	for (i = 1; i <= 3; i++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, p, sizeof(gstAppCommParam.szTelNum1) -1);
		sprintf(szContent, "TEL NUM(%d):", i);				
		nRet = PubInputDlg("SET COMM", szContent, szTemp, &nLen, 0, 14, 60, INPUT_MODE_STRING);
		if(nRet == KEY_UP)
		{
			if(i <= 1)
			{
				return KEY_UP;
			}
			else
			{
				i -= 2;
				p -= sizeof(gstAppCommParam.szTelNum1);
				continue;
			}
		}
		else if(nRet == KEY_DOWN)
		{
			p += sizeof(gstAppCommParam.szTelNum1);
			continue;
		}
		ASSERT_RETURNCODE(nRet);		
		memcpy(p, szTemp, sizeof(gstAppCommParam.szTelNum1) -1);
		p += sizeof(gstAppCommParam.szTelNum1);
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	}
	return APP_SUCC;
} 

/**
* @brief set dialing access number of wireless modem 
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommWLMDialNum(void)
{
	char szTemp[20+1] = {0};
	int nLen;

	memcpy(szTemp, gstAppCommParam.szWirelessDialNum, 20);
	ASSERT_RETURNCODE(PubInputDlg("SET COMM", "Wireless dial No.:", szTemp, \
			&nLen, 0, 20, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szWirelessDialNum, szTemp, sizeof(gstAppCommParam.szWirelessDialNum) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set apn
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommAPN(void)
{
	char szTemp[40+1] = {0};
	int nLen;

	memcpy(szTemp, gstAppCommParam.szAPN1, 40);
	ASSERT_RETURNCODE( PubInputDlg("SET COMM", "APN:", szTemp, \
			&nLen, 0, 40, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szAPN1, szTemp, sizeof(gstAppCommParam.szAPN1) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set user name
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommUser(void)
{
	char szTemp[40+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szUser, 40);
	ASSERT_RETURNCODE( PubInputDlg("SET COMM", "USER NAME", szTemp, \
			&nLen, 0, 40, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szUser, szTemp, sizeof(gstAppCommParam.szUser) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set user password
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommPassWd(void)
{
	char szTemp[40+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szPassWd, 40);
	ASSERT_RETURNCODE( PubInputDlg("SET COMM", "USER PASSWORD", szTemp, \
			&nLen, 0, 40, 60, INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szPassWd, szTemp, sizeof(gstAppCommParam.szPassWd) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIp(void)
{
	char szTemp[16+1]= {0};
	int nLen;

	if(gstAppCommParam.cIsDns != 0)
	{
		return APP_FUNCQUIT;
	}
	memcpy(szTemp, gstAppCommParam.szIp1, sizeof(gstAppCommParam.szIp1)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET COMM", "HOST IP1", szTemp, &nLen));
	memcpy(gstAppCommParam.szIp1, szTemp, sizeof(gstAppCommParam.szIp1) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set second ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommBackIp(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDns != 0)
	{
		return APP_FUNCQUIT;
	}
	memcpy(szTemp, gstAppCommParam.szIp2, sizeof(gstAppCommParam.szIp2)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET COMM", "HOST IP2", szTemp, &nLen));
	memcpy(gstAppCommParam.szIp2, szTemp, sizeof(gstAppCommParam.szIp2) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set port
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommPort(void)
{
	char szTemp[20+1];
	int nRet;
	int nLen;
	
	while(1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szPort1, sizeof(gstAppCommParam.szPort1)-1);
		ASSERT_RETURNCODE(PubInputDlg("SET COMM", "HOST PORT1", szTemp, \
				&nLen, 0, 6, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 0 || nRet > 65535)
		{
			continue; 
		}
		break;
	}
	memcpy(gstAppCommParam.szPort1, szTemp, sizeof(gstAppCommParam.szPort1) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set second port
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommBackPort(void)
{
	char szTemp[20+1];
	int nRet;
	int nLen;

	while(1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szPort2, sizeof(gstAppCommParam.szPort2)-1);
		ASSERT_RETURNCODE( PubInputDlg("SET COMM", "HOST PORT2", szTemp, \
				&nLen, 0, 6, 60, INPUT_MODE_NUMBER));
		nRet = atoi(szTemp);
		if (nRet < 0 || nRet > 65535)
		{
			continue; 
		}
		break;
	}
	memcpy(gstAppCommParam.szPort2, szTemp, sizeof(gstAppCommParam.szPort2) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set local ip
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommIpAddr(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{	
		return APP_FUNCQUIT;
	}

	memcpy(szTemp, gstAppCommParam.szIpAddr, sizeof(gstAppCommParam.szIpAddr)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET COMM", "LOCAL IP", szTemp, &nLen));
	memcpy(gstAppCommParam.szIpAddr, szTemp, sizeof(gstAppCommParam.szIpAddr) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	
	return APP_SUCC;
}

/**
* @brief set mask
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommMask(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{
		return APP_FUNCQUIT;
	}
	
	memcpy(szTemp, gstAppCommParam.szMask, sizeof(gstAppCommParam.szMask)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET COMM", "MASK", szTemp, &nLen));
	memcpy(gstAppCommParam.szMask, szTemp, sizeof(gstAppCommParam.szMask) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	return APP_SUCC;
}

/**
* @brief set gate
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncCommGate(void)
{
	char szTemp[16+1] = {0};
	int nLen;

	if(gstAppCommParam.cIsDHCP != 0)
	{
		return APP_FUNCQUIT;
	}
	
	memcpy(szTemp, gstAppCommParam.szGate, sizeof(gstAppCommParam.szGate)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET COMM", "GATE", szTemp, &nLen));
	memcpy(gstAppCommParam.szGate, szTemp, sizeof(gstAppCommParam.szGate) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set if aux need tpdu
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
static int SetFuncAuxIsAddTpdu(void)
{
	char cSelect = gstAppCommParam.cAuxIsAddTpdu;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};	
	ASSERT_RETURNCODE(PubSelectYesOrNo((char *)tr("SET COMM"), (char *)tr("ADD TPDU"), SelMenu, &cSelect));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET COMM", "ADD TPDU", "0.NO|1.YES", &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cAuxIsAddTpdu = 0;
		break;
	case '1':
		gstAppCommParam.cAuxIsAddTpdu = 1;
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief get timeout from app communication parameter
* @param out pszTimeOut 2 ascii bytes
* @li APP_SUCC
*/
int GetVarCommTimeOut(char *pszTimeOut)
{
	char szTmp[2+1] = {0};
	
	sprintf(szTmp, "%02d", gstAppCommParam.cTimeOut);
	memcpy(pszTimeOut, szTmp, 2);

	return APP_SUCC;
}

/**
* @brief set timeout to app communication parameter
* @param in pszTimeOut 2 ascii bytes 
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommTimeOut(const char *pszTimeOut)
{
	char szTemp[2+1] = {0};

	memcpy(szTemp, pszTimeOut, 2);
	gstAppCommParam.cTimeOut = (char)atoi(szTemp);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief get resend time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommReSendNum(char *pcReSendNum)
{
	*pcReSendNum = gstAppCommParam.cReSendNum;
	return APP_SUCC;
}

/**
* @brief set resend time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommReSendNum(const char *pcReSendNum)
{
	char cReSendUum = *pcReSendNum;

	gstAppCommParam.cReSendNum  = cReSendUum >= 0x30 ? (cReSendUum - 0x30)%9 : cReSendUum%9;
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief get offline resend time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommOffReSendNum(char *pcReSendNum)
{
	*pcReSendNum = gstAppCommParam.cOffResendNum;
	return APP_SUCC;
}

/**
* @brief set offline resend time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommOffReSendNum(const char *pcReSendNum)
{
	char cReSendUum = *pcReSendNum;

	gstAppCommParam.cOffResendNum  = cReSendUum >= 0x30 ? (cReSendUum - 0x30)%9 : cReSendUum%9;
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief get redial time
* @param out pcReSendNum 1byte
* @li APP_SUCC
*/
int GetVarCommReDialNum(char *pcReDialNum)
{
	*pcReDialNum = gstAppCommParam.cReDialNum;
	return APP_SUCC;
}

/**
* @brief set redial time
* @param in pcReSendNum 1byte
* @li APP_SUCC
* @li APP_FAIL
*/
int SetVarCommReDialNum(const char *pcReDialNum)
{
	char cReDialUum = *pcReDialNum;

	gstAppCommParam.cReDialNum  = cReDialUum >= 0x30 ? (cReDialUum - 0x30)%9 : cReDialUum%9;
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief Set wifi ssid
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiSsid(void)
{
	char szTemp[100+1] = {0};
	int nLen, nRet, nWifiMode;

	PubClearAll();
	PubDisplayTitle("WIFI PARAM");
	PubDisplayStrInline(1, 2,"SSID");
	PubDisplayStrInline(1, 3, "%s",gstAppCommParam.szWifiSsid);
	PubDisplayStrInline(1, 4, "0.SCAN    1.ENTER");
	PubUpdateWindow();
	nRet = PubGetKeyCode(60);
	if(nRet == KEY_F1 || nRet == KEY_F2)
		return nRet;
	else if(nRet != KEY_0 && nRet != KEY_1 && nRet != KEY_ENTER)
		return APP_FAIL;
	if(nRet == KEY_0)
	{
		PubClear2To4();
		PubDisplayStrInline(0, 2, "SEARCH SSID");
		PubDisplayStrInline(0, 3, "PLS WAIT...");
		PubUpdateWindow();		
		ASSERT_RETURNCODE(PubCommScanWifi("SET COMM", szTemp, &nWifiMode, 60));
		gstAppCommParam.cWifiMode = nWifiMode;
		memset(gstAppCommParam.szWifiSsid, 0, sizeof(gstAppCommParam.szWifiSsid));
		memcpy(gstAppCommParam.szWifiSsid, szTemp, sizeof(gstAppCommParam.szWifiSsid));
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	}
	else if(nRet == KEY_1)
	{
		memcpy(szTemp, gstAppCommParam.szWifiSsid, 32);
		ASSERT_RETURNCODE( PubInputDlg("WIFI PARAM", "SSID", szTemp, \
				&nLen, 1, 32, 60, INPUT_MODE_STRING));
		memcpy(gstAppCommParam.szWifiSsid, szTemp, sizeof(gstAppCommParam.szWifiSsid) -1);
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
		ASSERT_FAIL(SetFuncWifiMode());
	}
	return APP_SUCC;
}

/**
* @brief set wifi password
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiPwd(void)
{
	char szTemp[64+1] = {0};
	int nLen;
	
	if (gstAppCommParam.cWifiMode == WIFI_NET_SEC_NONE)
	{
		gstAppCommParam.cWifiKeyType = WIFI_KEY_TYPE_NOKEY;
		memset(gstAppCommParam.szWifiKey, 0, sizeof(gstAppCommParam.szWifiKey));
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
		return APP_SUCC;
	}
	
	memcpy(szTemp, gstAppCommParam.szWifiKey, 40);
	ASSERT_RETURNCODE( PubInputDlg("WIFI PARAM", "Password", szTemp, \
				&nLen, 0, 32, 60, INPUT_MODE_STRING));
	if (0 == strlen(szTemp))
	{
		gstAppCommParam.cWifiKeyType = WIFI_KEY_TYPE_NOKEY;
	}
	memcpy(gstAppCommParam.szWifiKey, szTemp, sizeof(gstAppCommParam.szWifiKey) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief set wifi modo
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/
int SetFuncWifiMode(void)
{
	int nSelect = 0;
	int nRet = 0;
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.OPEN"),
		tr("2.WEP"),
		tr("3.WPA-PSK"),
		tr("4.WPA2-PSk"),
		tr("5.WPA-CCKM")
	};
	while(1)
	{
		nRet = PubShowMenuItems(tr("WIFI MODE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 60);
		if(nRet == APP_QUIT || nRet == APP_TIMEOUT)
		{
			return nRet;
		}
		
		//ASSERT_RETURNCODE(ProSelectList("1.OPEN|2.WEP|3.WPA-PSK|4.WPA2-PSK|5.WPA-CCKM", "WIFI MODE", \
					//gstAppCommParam.cWifiMode, &nSelect));
		switch(nSelect)
		{
		case 1:
			gstAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_OPEN;
			break;
		case 2:
			gstAppCommParam.cWifiMode = WIFI_NET_SEC_WEP_SHARED;
			break;
		case 3:
			gstAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
			break;
		case 4:
			gstAppCommParam.cWifiMode = WIFI_NET_SEC_WPA2;
			break;
		case 5:
			gstAppCommParam.cWifiMode = WIFI_NET_SEC_WPA;
			break;
		default:
			return APP_QUIT;
		}
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
		return APP_SUCC;
	}
}

/**
* @brief set if need to dhcp
* @param in  void
* @li APP_SUCC
* @li APP_FAIL
*/

static int SetFuncIsDhcp(void)
{
	char cSelect = gstAppCommParam.cIsDHCP;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("SET COMM", "OPEN DHCP", SelMenu, &cSelect));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET COMM", "OPEN DHCP", "0.NO|1.YES", &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsDHCP = 0;
		break;
	case '1':
		gstAppCommParam.cIsDHCP = 1;
		break;
	default:
		return APP_QUIT;
	}
	
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief get app communication parameter
* @param out :pstAppCommParam
* @return void
*/
void GetAppCommParam(STAPPCOMMPARAM *pstAppCommParam)
{
	memcpy((char *)pstAppCommParam, (char *)(&gstAppCommParam), sizeof(STAPPCOMMPARAM));
}

/**
* @brief save app communication parameter
* @param in pstAppCommParam
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
int SetAppCommParam(STAPPCOMMPARAM *pstAppCommParam)
{
	memcpy((char *)(&gstAppCommParam), (char *)pstAppCommParam, sizeof(STAPPCOMMPARAM));
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief Get NII
* @param out pszNii 
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarCommSecurityNii(char * pszNii)
{
	memcpy(pszNii, gstAppCommParam.szSecurityNii, 3);
	return APP_SUCC;
}

/**
* @brief Get NII
* @param out pszNii 
* @li APP_SUCC
* @li APP_FAIL
*/
int GetVarCommTransNii(char * pszNii, int nIndex)
{
	memcpy(pszNii, gstAppCommParam.szTransNii[nIndex], 3);
	return APP_SUCC;
}

/**
** brief: Exprot communication parammeter
** param: void
** return: APP_SUCC or APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int ExportCommParam(void)
{	
	memset(&gstAppCommParam, 0, sizeof(STAPPCOMMPARAM));
	ASSERT_FAIL(PubReadOneRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	return APP_SUCC;
}

/**
** brief: Set Nii
** param: void
** return: APP_SUCC or APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SetFuncCommSecurityNii(void)
{
	char szTemp[3+1];
	int nLen;
	
	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szSecurityNii, sizeof(gstAppCommParam.szSecurityNii)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg("SET COMM", "Security NII:", szTemp, &nLen,3,3,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szSecurityNii, szTemp, sizeof(gstAppCommParam.szSecurityNii) -1);

	//save TPDU
//	sprintf(szTpdu, "600%3.3s0000", gstAppCommParam.szSecurityNii);
//	PubAscToHex((uchar *)szTpdu, 10, 0, (uchar *)gstAppCommParam.sTpdu);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
** brief: Set Nii
** param: void
** return: APP_SUCC or APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SetFuncCommTransNii(void)
{
	char szTemp[3+1];
	int nLen = 0;
	char szTitle[32] = {0};
	int nAcqIndex = 0;

	while (1)
	{
		if (nAcqIndex >= ACQUIRER_NUM)
		{
			break;
		}
		strcpy(szTitle, glszAcquirerName[nAcqIndex]);
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szTransNii[nAcqIndex], sizeof(gstAppCommParam.szTransNii[nAcqIndex])-1);
		ASSERT_RETURNCODE(PubInputDlg(szTitle, "TRANSACTIONS NII", szTemp, &nLen,3,3,60,INPUT_MODE_STRING));
		memcpy(gstAppCommParam.szTransNii[nAcqIndex], szTemp, sizeof(gstAppCommParam.szTransNii[nAcqIndex]) -1);
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
		nAcqIndex++;
	}

	return APP_SUCC;
}


/**
** brief: Set Nii
** param: void
** return: APP_SUCC or APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SetFuncCommTransTpdu(void)
{
	char szTemp[10+1];
	int nLen = 0;
	char szTitle[32] = {0};
	int nAcqIndex = 0;

	while (1)
	{
		if (nAcqIndex >= ACQUIRER_NUM)
		{
			break;
		}
		strcpy(szTitle, glszAcquirerName[nAcqIndex]);
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, gstAppCommParam.szTransTpdu[nAcqIndex], sizeof(gstAppCommParam.szTransTpdu[nAcqIndex])-1);
		ASSERT_RETURNCODE(PubInputDlg(szTitle, "TRANSACTIONS TPDU", szTemp, &nLen,10,10,60,INPUT_MODE_STRING));
		memcpy(gstAppCommParam.szTransTpdu[nAcqIndex], szTemp, sizeof(gstAppCommParam.szTransTpdu[nAcqIndex]) -1);
		ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
		nAcqIndex++;
	}

	return APP_SUCC;
}

/**
* @brief Get NII
* @param out pszNii 
* @li APP_SUCC
* @li APP_FAIL
*/
int ChangeCommTransTpdu(int nIndex)
{	
	if (nIndex < ACQUIRER_NUM)
	{
		PubAscToHex((uchar *)gstAppCommParam.szTransTpdu[nIndex], 10, 0, (uchar *)gstAppCommParam.sTpdu);
	}
	else
	{
		memcpy(gstAppCommParam.sTpdu, gstAppCommParam.sSecurityTpdu, 5);
		//PubAscToHex((uchar *)gstAppCommParam.sSecurityTpdu, 10, 0, (uchar *)gstAppCommParam.sTpdu);
	}
	return APP_SUCC;
}


/**
** brief: set communication parameter
** param: void
** return: APP_SUCC
** auther: 
** date: 2016-7-4
** modify: 
*/
int SetComm()
{
	CommMenu();	
	CommInit();
	return APP_SUCC;
}

/**
** brief: Set Nii
** param: void
** return: APP_SUCC or APP_FAIL
** auther: 
** date: 2016-7-4
** modify: 
*/
int SetFuncCommSecurityTpdu(void)
{
	char szTemp[10+1];
	int nLen = 0;

	memset(szTemp, 0, sizeof(szTemp));
	PubHexToAsc((uchar *)gstAppCommParam.sSecurityTpdu, 10, 0, (uchar *)szTemp);
	ASSERT_RETURNCODE(PubInputDlg("SET COMM", "SECURITY TPDU", szTemp, &nLen,10,10,60,INPUT_MODE_STRING));
	PubAscToHex((uchar *)szTemp, 10, 0, (uchar *)gstAppCommParam.sSecurityTpdu);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));

	return APP_SUCC;
}

void SetWifiParam()
{
	int (*lSetFuns[])(void) = {
    SetFuncWifiSsid, 
    SetFuncWifiPwd,
    SetFuncIsDhcp,
	SetFuncCommIpAddr,
	SetFuncCommMask,
	SetFuncCommGate,
    SetFuncCommIp,
	SetFuncCommPort,
	SetFuncCommBackIp,
	SetFuncCommBackPort,
	NULL};
    PubUpDownMenus(lSetFuns);
}

void SetEthParam()
{
	int (*lSetFuns[])(void) = {
		SetFuncCommWLMDialNum,
		SetFuncCommAPN,
		SetFuncCommIp,
		SetFuncDomainName,
		SetFuncCommDnsIp,
		SetFuncCommPort,
		SetFuncCommBackIp,
		SetFuncCommBackPort,
	    SetFuncCommUser,
		SetFuncCommPassWd,
		SetFuncCommMode, 
		NULL};
	    PubUpDownMenus(lSetFuns);
}

void SetModemParam()
{
	int (*lSetFuns[])(void) = {
		SetFuncCommTelNo, 
		NULL};
	    PubUpDownMenus(lSetFuns);
}

void SetGPRSParam()
{
	int (*lSetFuns[])(void) = {
		SetFuncCommWLMDialNum,
		SetFuncCommAPN,
		SetFuncCommIp,
		SetFuncDomainName,
		SetFuncCommDnsIp,
		SetFuncCommPort,
		SetFuncCommBackIp,
		SetFuncCommBackPort,
	    SetFuncCommUser,
		SetFuncCommPassWd,
		SetFuncCommMode, 
		NULL};
	    PubUpDownMenus(lSetFuns);
}

/**
* @brief 设置是否启动SSL
* @param in  无
* @li APP_SUCC
* @li APP_FAIL
*/
int SetIsSSLEnable(void)
{
	char cSelect = NO;
	char *SelMenu[] = {
		"0.NO",
		"1.YES"
	};
	if (gstAppCommParam.cIsSSL == 1)
	{
		cSelect = YES;
	}
	ASSERT_RETURNCODE(PubSelectYesOrNo("SET SSL", "ENABLE SSL", SelMenu, &cSelect))
	//ASSERT_RETURNCODE(ProSelectYesOrNo("SET SSL", "ENABLE SSL", "0.NO|1.YES", &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsSSL = 0;
		gstAppCommParam.cIsDns = 0;//同时域名解析
		break;
	case '1':
		if(gstAppCommParam.cCommType == COMM_DIAL/*江苏需要通过串口转发，所以这边屏蔽 || gstAppCommParam.cCommType == COMM_RS232*/)
		{
			PubMsgDlg("Warn", "Nonsupport", 3, 3);
			gstAppCommParam.cIsSSL = 0;
			gstAppCommParam.cIsDns = 0;//同时域名解析
			ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
			return APP_QUIT;
		}
		gstAppCommParam.cIsSSL = 1;
		gstAppCommParam.cIsDns = 1;//同时域名解析
		break;
	default:
		return APP_QUIT;
	}
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	CommInit();

	return APP_SUCC;
}

YESORNO IsSslEnable(void)
{	
	if(gstAppCommParam.cCommType != COMM_DIAL)
	{
		if(gstAppCommParam.cIsSSL == 1)
		{
			return YES;
		}
	}
	return NO;
}

static int SetFuncDomainName(void)
{
	char szTemp[50+1];
	int nLen;

	if(gstAppCommParam.cIsSSL == 0)
	{
		return APP_FUNCQUIT;//返回此值可以保证向上翻页
	}

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szDomain, sizeof(gstAppCommParam.szDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg("SET SSL", "Domain Name", szTemp, &nLen,0,50,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szDomain, szTemp, sizeof(gstAppCommParam.szDomain) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

/**
* @brief 设置pos网关
* @param 无
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date
*/
static int SetFuncCommDnsIp(void)
{
	char szTemp[16+1];
	int nLen;

	if(gstAppCommParam.cIsSSL == 0)
	{
		return APP_FUNCQUIT;//返回此值可以保证向上翻页
	}
	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szDNSIp1, sizeof(gstAppCommParam.szDNSIp1)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputIp("SET SSL", "DNS IP", szTemp, &nLen));
	memcpy(gstAppCommParam.szDNSIp1, szTemp, sizeof(gstAppCommParam.szDNSIp1) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}
extern int GetFuncIsUseDnsAna(void)
{
	return gstAppCommParam.cIsDnsAna;
}

/**
* @brief 设置是否使用DNS
* @param in  无
* @li APP_SUCC
* @li APP_FAIL
*/

static int SetFuncIsUseDnsAna(void)
{
	char cSelect = gstAppCommParam.cIsDnsAna;
	char *SelMenu[] = {
		"0.No",
		"1.Yes"
	};
	ASSERT_RETURNCODE(PubSelectYesOrNo("Comm Setting", "Use DNSAna", SelMenu, &cSelect));
	//ASSERT_RETURNCODE(ProSelectYesOrNo("Comm Setting", "Use DNSAna?", "0.No|1.Yes", &cSelect));
	switch(cSelect)
	{
	case '0':
		gstAppCommParam.cIsDnsAna = 0;
		break;
	case '1':
		gstAppCommParam.cIsDnsAna = 1;
		break;
	default:
		return APP_QUIT;
	}
	
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

static int SetFuncQRDomainName(void)
{
	char szTemp[50+1];
	int nLen;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, gstAppCommParam.szOrDomain, sizeof(gstAppCommParam.szOrDomain)-1);
	nLen = strlen(szTemp);
	ASSERT_RETURNCODE(PubInputDlg("SET QR", "Domain Name", szTemp, &nLen,0,50,60,INPUT_MODE_STRING));
	memcpy(gstAppCommParam.szOrDomain, szTemp, sizeof(gstAppCommParam.szOrDomain) -1);
	ASSERT_FAIL(PubUpdateRec(FILE_APPCOMMPARAM,1,(char *)&gstAppCommParam));
	return APP_SUCC;
}

