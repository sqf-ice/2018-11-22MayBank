/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  entrypoint dealing
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

static int EmvEntryPointProcess(TRF_transData *, STENTRYPOINTOPT *);

static int gnKernelType =0;

int GetKernelType(void)
{
	return gnKernelType;
}

int SetKernelType(int nKernelType)
{
	gnKernelType = nKernelType;
	return APP_SUCC;
}

int DealBcdPan(char *pszPan,char *pszIn,int nLen)
{
	int i;
	char szPan[20+1]= {0};
	
	PubHexToAsc((uchar *)pszIn, nLen, 0, (uchar *)(szPan));	
	for(i = 0; i < nLen && i < sizeof(szPan); i++)
	{
		if (szPan[i] == 'f' ||szPan[i] == 'F')
		{
			szPan[i] = 0;
		}
	}
	strcpy(pszPan,szPan);
	return APP_SUCC;
}

int DealBcdTrack2(char *pszTrack2,char *pszIn,int nLen)
{
	int i;
	char szTrack2[38+1]= {0};

	if(nLen>38)
		nLen=38;
	
	PubHexToAsc((uchar *)pszIn, nLen, 0, (uchar *)(szTrack2));
	for(i = 0; i < nLen && i < sizeof(szTrack2); i++)
	{
		if (szTrack2[i] == 'f' ||szTrack2[i] == 'F')
		{
			szTrack2[i] = 0;
		}
	}
	strcpy(pszTrack2,szTrack2);
	return APP_SUCC;
}

int GetPanFromTrack2(char *pszPan,char *pszTrack2)
{
	int i=0;
	
	for(i = 0; i < 19; i++)
	{
		if (pszTrack2[i] == 'D')
		{
			pszPan[i] = 0;
			break;
		}
		pszPan[i]=pszTrack2[i];
	}
	return APP_SUCC;
}

int SwitchEntrypointToEmvRes(STENTRYPOINTOPT *pstEntryPointOpt, int nRes)
{

	switch (nRes)
	{		
		case EMV_TRANS_RF_MCHIP_ACCEPT:
			pstEntryPointOpt->_trans_ret = EMV_TRANS_QPBOC_ACCEPT;
			break;
		case EMV_TRANS_RF_MCHIP_DENIAL:
			pstEntryPointOpt->_trans_ret = EMV_TRANS_QPBOC_DENIAL;
			break;
		case EMV_TRANS_RF_MCHIP_GOONLINE:
			pstEntryPointOpt->_trans_ret = EMV_TRANS_QPBOC_GOONLINE;
			break;
		case EMV_TRANS_RF_MAG_GOONLINE:
			pstEntryPointOpt->_trans_ret = EMV_TRANS_MSD_GOONLINE;
			break;
		default:
			pstEntryPointOpt->_trans_ret = nRes;
			break;
	}
	
	return APP_SUCC;
}

int EmvEntryPointGetData(uint32 unTag, uchar *pszData, int nDataLen)
{
	int nKernelType=0;
	int nRealLen=0;

	nKernelType = GetKernelType();
	TRACE(("Emv get nKernelType=%d", nKernelType));
	if (nKernelType > 0)
	{
		nRealLen = SDK_EP_GetData(unTag, (uchar *)pszData, nDataLen);
	}
	else
	{
		nRealLen = EMV_getdata(unTag, (uchar *)pszData, nDataLen);
	}
	
	if (nRealLen<=0)
	{
		return nRealLen;
	}
	
	TRACE_HEX(((char *)pszData, nRealLen, "%x[%d]:", unTag, nRealLen));

	return nRealLen;
}

int EmvEntryPointSetData(uint32 unTag, uchar *pszData, int nDataLen)
{
	int nKernelType=0;

	nKernelType = GetKernelType();
	TRACE(("nKernelType=%d", nKernelType));
	if (nKernelType > 0)
	{
		nDataLen = SDK_EP_SetData(unTag, (uchar *)pszData, nDataLen);
	}
	else
	{
		nDataLen = EMV_setdata(unTag, (uchar *)pszData, nDataLen);
	}
	if (nDataLen<=0)
	{
		TRACE(("set[%x]fail%d", unTag, nDataLen));
	}
	return APP_SUCC;
}

int EmvEntryPointFetchData(unsigned int* punTagName, int nTagCnt, unsigned char* pusOutBuf, int nMaxOutLen)
{
	int nKernelType=0;
	int nRet=0;

	nKernelType = GetKernelType();
	TRACE(("nKernelType=%d", nKernelType));
	if (nKernelType > 0)
	{
		nRet = SDK_EP_FetchData(punTagName, nTagCnt, pusOutBuf, nMaxOutLen);
	}
	else
	{
		nRet = EMV_FetchData(punTagName, nTagCnt, pusOutBuf, nMaxOutLen);
	}
	TRACE(("nRet=%d", nRet));
	return nRet;
}

int EmvGetData(STSYSTEM *pstSystem)
{
	char szPan[20+1]={0};
	uchar szTemp[999+1]={0};
	int nDataLen=0;
	char szMsg[128+1]={0};
	char szTrack2[38+1]={0};
	
	nDataLen = EmvEntryPointGetData(_EMV_TAG_5A_IC_PAN, szTemp, sizeof(szTemp));
	if(nDataLen <=0)
	{
		memset(szTemp, 0, sizeof(szTemp));
		nDataLen = EmvEntryPointGetData(_EMV_TAG_57_IC_TRACK2EQUDATA, szTemp, sizeof(szTemp));
		if(nDataLen <=0)
		{
			EmvEntryPointSuspend(-1);
			sprintf(szMsg, "Get pan fail %d", nDataLen);
			DispOutICC(NULL, szMsg, EmvErrMsg(EMV_ErrorCode()));
			return APP_FAIL;
		}
		else
		{
			DealBcdTrack2(szTrack2, (char *)szTemp,nDataLen<<1);
			GetPanFromTrack2(szPan, szTrack2);
		}
	}
	else
	{
		DealBcdPan(szPan,(char *)szTemp,nDataLen<<1);
	}
	TRACE(("szPan=%s", szPan));
	memcpy(pstSystem->szPan, szPan, sizeof(szPan));
	
	memset(szTemp, 0, sizeof(szTemp));
	nDataLen = EmvEntryPointGetData(_EMV_TAG_57_IC_TRACK2EQUDATA, szTemp, sizeof(szTemp));
	if(nDataLen <= 0)
	{
		memset(szTemp, 0, sizeof(szTemp));
		nDataLen = EmvEntryPointGetData(_EMV_TAG_9F6B_MCC_IC_TRACK2DATA, szTemp, sizeof(szTemp));
		TRACE(("nDataLen = %d", nDataLen));
	}

	DealBcdTrack2(pstSystem->szTrack2, (char *)szTemp,nDataLen<<1);
//	pstSystem->szTrack2[strlen(pstSystem->szTrack2) - 1] = '5'; // shtest
	TRACE(("szTrack2=%s", pstSystem->szTrack2));
	
	memset(szTemp, 0, sizeof(szTemp));
	nDataLen = EmvEntryPointGetData(_EMV_TAG_5F34_IC_PANSN, szTemp, sizeof(szTemp));
	if (nDataLen > 0)
	{
		sprintf(pstSystem->szCardSerialNo, "0%02x", szTemp[0]);
	}
#if 0//20170518 master donot send
	else
	{
		memcpy(pstSystem->szCardSerialNo, "000", 3);
	}
#endif
	TRACE(("pstSystem->szCardSerialNo=%s", pstSystem->szCardSerialNo));
	
	memset(szTemp, 0, sizeof(szTemp));
	nDataLen = EmvEntryPointGetData(_EMV_TAG_5F24_IC_APPEXPIREDATE, szTemp, sizeof(szTemp));
	if (nDataLen > 0)
	{
		sprintf(pstSystem->szExpDate, "%02x%02x", (szTemp[0]) , (szTemp[1]));
	}
	TRACE(("pstSystem->szExpDate=%s", pstSystem->szExpDate));

	//Get holder name
	memset(szTemp, 0, sizeof(szTemp));
	nDataLen = EmvEntryPointGetData(_EMV_TAG_5F20_IC_HOLDERNAME, szTemp, sizeof(szTemp));
	if (nDataLen > 0)
	{
		strcpy(pstSystem->szHolderName, (char *)szTemp);
		PubAllTrim(pstSystem->szHolderName);
	}
	TRACE(("pstSystem->szExpDate=%s", pstSystem->szHolderName));
	memset(szTemp, 0, sizeof(szTemp));
	
	nDataLen = EmvEntryPointGetData(_EMV_TAG_9F1F_IC_TRACK1DATA, szTemp, sizeof(szTemp));
	if (nDataLen > 0)
	{
		strcpy(pstSystem->szTrack1, (char *)szTemp);
	}
	TRACE(("pstSystem->szTrack1=%s", pstSystem->szTrack1));

	//get acqindex
	GetEmvAcqType(&pstSystem->nAcqType);
	GetAcqIndex(pstSystem);	
PubDebug("pstSystem->nAcqIndex: %d", pstSystem->nAcqIndex);

	return APP_SUCC;
}

int EmvEntryPointStart(STENTRYPOINTOPT *pstEntryPointOpt, unsigned long long lAmount, unsigned long long lCashbackAmount)
{
	TRF_transData	stTransData;
	char szDateTime[14+1]={0};
	uchar usFCI[1024];
	uchar usPreProcess[200];
	char szContent[100];
	int nRet = 0;

	memset(&stTransData,0,sizeof(TRF_transData));

	TRACE(("type=%d amount=%ld", pstEntryPointOpt->_trans_type, lAmount));
	stTransData._amount = lAmount;
	stTransData._amountOther = lCashbackAmount;
	stTransData._final_aid_fci = usFCI;//must be set
	stTransData._pre_process_indicator = usPreProcess;//maybe need prePreocess struction
	PubGetCurrentDatetime(szDateTime);
	PubAscToHex((uchar *)szDateTime,14,0,stTransData._date);
	
	
	pstEntryPointOpt->_request_amt = lAmount;

	
	switch(pstEntryPointOpt->_trans_type)
	{
		case EMV_TRANS_RF_SALE:
			pstEntryPointOpt->_trans_type= EMV_TRANS_EP_PURCHASE;
			break;
		case EMV_TRANS_BALANCE:
			pstEntryPointOpt->_trans_type = EMV_TRANS_EP_REFUND;
			break;
		default:
			break;
	}
	TRACE(("stEntryPointOpt._trans_type=%d",pstEntryPointOpt->_trans_type));
	
	pstEntryPointOpt->_card_no = 0xA1;	
	pstEntryPointOpt->_seq_start = START_A; //start of EntryPoint
	pstEntryPointOpt->_UI_message_id = UI_MSGID_PROCESSING;
	pstEntryPointOpt->_UI_status = UI_STATUS_IDLE;
	pstEntryPointOpt->_OP_status = OP_STATUS_NA;
	pstEntryPointOpt->_OP_start = OP_START_NA;
	pstEntryPointOpt->_OP_cvm = OP_CVM_NA;
	pstEntryPointOpt->_OP_discretionary_data_present = 1;
	pstEntryPointOpt->_OP_receipt = 0;//N/A
	pstEntryPointOpt->_OP_alternate_interface_preference = OP_ALTERNATE_INTERFACE_PREFERENCE;
	pstEntryPointOpt->_OP_field_off_request = OP_FIELD_OFF_REQUEST;
	pstEntryPointOpt->_OP_online_response_data = OP_ONLINE_RESPONSE_DATA_NA;

	TRACE(("start."));
	nRet = SDK_Entry_Point_Process(pstEntryPointOpt , &stTransData);
	TRACE(("nRet = %d", nRet));
	if (nRet != EMV_TRANS_RF_ACTIVE_CARD)
	{
		TRACE(("SDK_Entry_Point_Process cannot actived card!"));
		SDK_Entry_Point_Suspend(nRet);
		return EMV_INITAPP_QUIT;
	}
	if (PubCardPowerUp(NULL) != APP_SUCC)
	{
		SDK_Entry_Point_Suspend(nRet);
		ShowLightFail();
		PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CPower up fail,try again");
		PubMsgDlg(NULL, szContent, 3, 10);
		return EMV_INITAPP_QUIT;
	}

	ShowLightDeal();

	do
	{
		ASSERT_FAIL(EmvEntryPointProcess(&stTransData, pstEntryPointOpt));

	}while(EMV_TRANS_RF_SELECT_NEXT_AID == pstEntryPointOpt->_trans_ret);

	TRACE(("@@@@@@@@@ nRet=%d pstEntryPointOpt->_UI_message_id=%d", nRet,pstEntryPointOpt->_UI_message_id));
	if(UI_MSGID_SEE_PHONE == pstEntryPointOpt->_UI_message_id)
	{
		PubMsgDlg("","Pls See Phone", 1, 5);
	}
	return nRet;
}

static int EmvEntryPointProcess(TRF_transData *pstTransData, STENTRYPOINTOPT *pstEntryPointOpt)
{
	int nRet=0;
	
	TRACE(("start."));
	nRet = SDK_Entry_Point_Process( pstEntryPointOpt , pstTransData);
	TRACE(("return nRet = %d", nRet));

	if (nRet != EMV_TRANS_RF_ACTIVE_KERNEL)  //Pre_process Ok,then decide whith Kernel ID
	{
		TRACE(("SDK_Entry_Point_Process cannot actived card!"));
		SDK_Entry_Point_Suspend(nRet);
		pstEntryPointOpt->_trans_ret= -1;

		TRACE(("nRet=%d KernelID=%d", pstEntryPointOpt->_trans_ret,pstTransData->_kernel_id[0]));
		TRACE(("status=%d _OP_start=%d",pstEntryPointOpt->_OP_status, pstEntryPointOpt->_OP_start));
		TRACE(("_ER_MSG_ON_ERROR=%d",pstEntryPointOpt->_ER_MSG_ON_ERROR));
		TRACE(("_OP_ui_request_on_restart_present=%d",pstEntryPointOpt->_OP_ui_request_on_restart_present));

		return APP_FAIL;
	}

	TRACE(("Kernel_id[0] = %d\r\n",pstTransData->_kernel_id[0]));
	TRACE_HEX(((char *)pstTransData->_aid, pstTransData->_aid_len, "aid data: "));
	TRACE_HEX(((char *)pstTransData->_pre_process_indicator, pstTransData->_pre_process_indicator_len,"[PreProcess result]:"));

	TRACE_HEX(((char *)pstTransData->_aid, 16, "pstRFData->_aid:"));
	TRACE_HEX(((char *)pstTransData->_kernel_id, 8, "pstRFData->_kernel_id:"));

	if(0 != pstTransData->_kernel_id[0])
	{
		gnKernelType = pstTransData->_kernel_id[0];
	}
	else
	{
		gnKernelType = KERNEL_ID_PAYOHTER;
	}
	//SetKernelType(gnKernelType);

	if((KERNEL_ID_PAYPASS == pstTransData->_kernel_id[0] ||
	   KERNEL_ID_PAYWAVE == pstTransData->_kernel_id[0]) && 
	   EMV_TRANS_RF_INQUIRE_AMOUNT == pstEntryPointOpt->_trans_type)
	{
		TRACE(("paypass paywave not support EC"));
		pstEntryPointOpt->_trans_ret= -1;	
		SDK_Entry_Point_Suspend(-1);
		return APP_FAIL;
	}
	
	//if(memcmp(pstTransData->_aid, PBOC_RID, 5) == 0)
	if(pstTransData->_kernel_id[0] == KERNEL_ID_UNIONPAY)
	{
		TRACE(("Start."));
		nRet = SDK_Qpboc_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_UNIONPAY;
	}
	else if(pstTransData->_kernel_id[0] == KERNEL_ID_PAYPASS)
	{
		TRACE(("Start."));
		pstEntryPointOpt->_request_amt = EMV_TRANS_REQAMT_APS;
		nRet = SDK_PayPass_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_PAYPASS;
	}
	else if(pstTransData->_kernel_id[0] == KERNEL_ID_PAYWAVE)
	{
		TRACE(("Start."));
		pstEntryPointOpt->_request_amt = EMV_TRANS_REQAMT_APS;
		nRet = SDK_PayWave_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_PAYWAVE;
	
	}
	else if(pstTransData->_kernel_id[0] == KERNEL_ID_MCCS)
	{
		TRACE(("Start."));
		pstEntryPointOpt->_request_amt = EMV_TRANS_REQAMT_APS;
		nRet =  SDK_Pure_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_MCCS;
	}
	else if(pstTransData->_kernel_id[0] == KERNEL_ID_AME)
	{
		TRACE(("Start."));
		pstEntryPointOpt->_request_amt = EMV_TRANS_REQAMT_APS;
		nRet =  SDK_ExpressPay_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_AME;
	}
	else if(pstTransData->_kernel_id[0] == KERNEL_ID_JCB)
	{
		TRACE(("Start."));
		pstEntryPointOpt->_request_amt = EMV_TRANS_REQAMT_APS;
		nRet =  SDK_JCB_Process(pstEntryPointOpt, pstTransData);
		TRACE(("%d.",nRet));
		gnKernelType = KERNEL_ID_JCB;
	}
	SwitchEntrypointToEmvRes(pstEntryPointOpt, nRet);		
	
	return APP_SUCC;
}

int EmvEntryPointSuspend(int nEmvRes)
{
	int nRet =0;
	int nKernelType=0;

	nKernelType = GetKernelType();
	if(KERNEL_ID_PAYOHTER == nKernelType || 0== nKernelType)
	{		
		nRet = EMV_rf_suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if(KERNEL_ID_PAYPASS == nKernelType )
	{
		nRet = SDK_PayPass_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if(KERNEL_ID_PAYWAVE == nKernelType )
	{
		nRet = SDK_PayWave_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if(KERNEL_ID_MCCS == nKernelType )
	{
		nRet = SDK_Pure_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if (KERNEL_ID_UNIONPAY == nKernelType)
	{
		nRet = SDK_Qpboc_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if (KERNEL_ID_AME == nKernelType)
	{
		nRet = SDK_ExpressPay_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else if (KERNEL_ID_JCB == nKernelType)
	{
		nRet = SDK_JCB_Suspend(nEmvRes);
		nRet += SDK_Entry_Point_Suspend(0);
	}
	else
	{
		nRet = EMV_Suspend(nEmvRes);		
	}
	TRACE(("kernel=%d nRet=%d", nKernelType, nRet));
	
	return nRet;
}

int GetPanForPaypass(char *pszPan)
{
	int nDataLen=0;
	char szTemp[256+1]={0};
	char szMsg[128+1]={0};
	char szTrack2[37+1]={0};
	char szPan[19+1]={0};
	
	nDataLen = EmvEntryPointGetData(_EMV_TAG_5A_IC_PAN, (uchar *)szTemp, sizeof(szTemp));
	if(nDataLen <=0)
	{
		memset(szTemp, 0, sizeof(szTemp));
		nDataLen = EmvEntryPointGetData(_EMV_TAG_57_IC_TRACK2EQUDATA, (uchar *)szTemp, sizeof(szTemp));
		if(nDataLen <=0)
		{
			EmvEntryPointSuspend(-1);
			sprintf(szMsg, "Get pan fail %d", nDataLen);
			DispOutICC(NULL, szMsg, EmvErrMsg(EMV_ErrorCode()));
			//return APP_FAIL;
		}
		else
		{
			DealBcdTrack2(szTrack2, (char *)szTemp,nDataLen<<1);
			GetPanFromTrack2(szPan, szTrack2);
		}
	}
	else
	{
		DealBcdPan(szPan,(char *)szTemp,nDataLen<<1);
	}
	TRACE(("szPan=%s", szPan));
	memcpy(pszPan, szPan,19);
	return APP_SUCC;
}

/**
 获取卡片的应用类型
 成功 返回 CardAttribute类型
 失败 返回 APP_FAIL
*/
int GetCardAttribute(char *pcCardAttr)
{
	int nLen = 0;
	char szBuf[20+1];
	char sAID[16];

	nLen = EmvEntryPointGetData(_EMV_TAG_84_IC_DFNAME, (uchar *)szBuf, sizeof(szBuf));
	if (nLen > 0)
	{
		memcpy(sAID, szBuf, nLen);
	}
	else
	{
		nLen = EmvEntryPointGetData(_EMV_TAG_9F06_TM_AID, (uchar *)szBuf, sizeof(szBuf));
		memcpy(sAID, szBuf, nLen);
	}
	
	TRACE_HEX(((char *)sAID, nLen, "sAID:"));
	
	if(memcmp(sAID, "\xA0\x00\x00\x03\x33\x01\x01\x01", 8)  == 0)		//A000000333010101 借记卡
	{
		*pcCardAttr = DEBITCARD;
		TRACE(("借记卡"));
	}
	else if(memcmp(sAID, "\xA0\x00\x00\x03\x33\x01\x01\x02", 8)  == 0)	//A000000333010102 贷记卡
	{
		*pcCardAttr = CREDITCARD;
		TRACE(("贷记卡"));
	}
	else if(memcmp(sAID, "\xA0\x00\x00\x03\x33\x01\x01\x03", 8)  == 0)	//A000000333010103 准贷记卡
	{
		*pcCardAttr = QUACREDEITCARD;
		TRACE(("准贷记卡"));
	}
	else
	{
		TRACE(("return err"));
		return APP_FAIL;	
	}

	return APP_SUCC;
}

/**
* @brief 检测交易是否符合免密要求
* @param pszAmtTrans 交易金额
* @param pszPan 交易主账号
* @return
* @li APP_SUCC 允许免签
* @li APP_FAIL/APP_QUIT 不允许免密
*/
int EmvCheckIsQPS(const char *pszAmtTrans, char *pcPinAndSigFlag, char *pcIsGetPin)
{
	int nRet;
	char cCardAttr = 0;	//区分借贷记
	char cCardType; 	/*内外卡标志 OUTSIDECARD-外卡 INSIDECARD-内卡  UNKNOWCARD-未知卡*/
	char szCardTransQualifiers[3] = {0};
	char szQPSAMTLimit[13] = {0};
	char sBuf[128] = {0};
	int nLen = 0;
			
	ASSERT_QUIT(GetCvmLimit(PBOC2_ENB, szQPSAMTLimit, NULL));
	ASSERT_QUIT(GetCardAttribute(&cCardAttr));
	//借记卡和高于免签限额的要判断9F6C
	nRet = EmvEntryPointGetData(0x9F6C, (uchar *)szCardTransQualifiers, 2);

	switch (cCardAttr)
	{
	case CREDITCARD:
	case QUACREDEITCARD:
		if(AscBigCmpAsc(pszAmtTrans, szQPSAMTLimit) < 0)
		{
			*pcPinAndSigFlag = 1; // no pin no sig
			*pcIsGetPin = NO;
		}
		else
		{
			if(szCardTransQualifiers[0]&0x80)
			{
				*pcPinAndSigFlag = 2; // no pin no sig
				*pcIsGetPin = YES;
			}
			else if(szCardTransQualifiers[0]&0x40)
			{
				*pcPinAndSigFlag = 0; // no pin no sig
				*pcIsGetPin = NO;
			}
		}
		break;
	case DEBITCARD:
		if(AscBigCmpAsc(pszAmtTrans, szQPSAMTLimit) < 0)
		{
			if(szCardTransQualifiers[0]&0x80)
			{
				*pcPinAndSigFlag = 0; // no pin no sig
				*pcIsGetPin = YES;
			}
			else if(szCardTransQualifiers[0]&0x40)
			{
				*pcPinAndSigFlag = 0; 
				*pcIsGetPin = NO;
			}
			else
			{
				*pcPinAndSigFlag = 1; 
				*pcIsGetPin = NO;
			}		
		}
		else
		{
			if(szCardTransQualifiers[0]&0x80)
			{
				*pcPinAndSigFlag = 0; // no pin no sig
				*pcIsGetPin = YES;
			}
			else if(szCardTransQualifiers[0]&0x40)
			{
				*pcPinAndSigFlag = 0; // no pin no sig
				*pcIsGetPin = NO;
			}
		}
		break;
		
	}
	return APP_SUCC;
}



int CheckInputPinPayPass(STSYSTEM *pstSystem)
{
	int nKernelType=0;
	int nLen = 0;
	uchar sBuf[128] = {0};
	uchar szAmt[12+1]= {0};
	char cIsGetPin = YES;

	nKernelType = GetKernelType();
	TRACE(("nKernelType=%d", nKernelType));
	switch (nKernelType)
	{
		case KERNEL_ID_PAYPASS:
			GetCvmLimit(MASTERCARD_ENB, szAmt, NULL);
			if (memcmp(pstSystem->szAmount, szAmt, 12) < 0)
			{
				pstSystem->cWavePinAndSigFlag = 1;
				return APP_SUCC;
			}
			break;
		case KERNEL_ID_PAYWAVE:
			GetCvmLimit(VISA_ENB, szAmt, NULL);
PubDebugData("cvm limit: ", sBuf, nLen);
PubDebug("szAmt: %s", szAmt);
			if (memcmp(pstSystem->szAmount, szAmt, 12) < 0)
			{
				pstSystem->cWavePinAndSigFlag = 1;
				return APP_SUCC;
			}
			break;
		case KERNEL_ID_MCCS:
			GetCvmLimit(MCCS_ENB, szAmt, NULL);
PubDebugData("cvm limit: ", sBuf, nLen);
PubDebug("szAmt: %s", szAmt);
			if (memcmp(pstSystem->szAmount, szAmt, 12) < 0)
			{
				pstSystem->cWavePinAndSigFlag = 1;
				return APP_SUCC;
			}
			break;
		case KERNEL_ID_UNIONPAY:
			EmvCheckIsQPS(pstSystem->szAmount, &pstSystem->cWavePinAndSigFlag, &cIsGetPin);
			if (YES == cIsGetPin)
			{
				ASSERT_HANGUP_QUIT(GetPin(pstSystem->szPan, pstSystem->szAmount, pstSystem->szPin));
			}
			break;
		default:	
			break;
	}

	return APP_SUCC;
	
}

