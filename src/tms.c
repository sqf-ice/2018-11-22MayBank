#ifdef USE_TMS
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

int SetDownloadParam(STDOWNLOADPARAM *pstDownloadParam)
{
	char szPosType[20+1] = {0};
	
	pstDownloadParam->nCommBufLen = 4000;
	pstDownloadParam->nMaxPackNum = 4;
	pstDownloadParam->nPackDelayTime = 1;
	
	memset(pstDownloadParam->szHardwareSerialNo , 0x00, sizeof(pstDownloadParam->szHardwareSerialNo));
	memset(pstDownloadParam->szPosType, 0x00, sizeof(pstDownloadParam->szPosType));

	NDK_SysGetPosInfo(SYS_HWINFO_GET_POS_USN,NULL,pstDownloadParam->szHardwareSerialNo);

	strcpy(pstDownloadParam->szPosType, PubGetPosTypeStr());

	return APP_SUCC;

}

#if 0
int TmsCheckUpdate()
{
	STCOMMPARAM stCommParam;
	STDOWNLOADPARAM stDownloadParam;
	STAPPCOMMPARAM stAppCommParam;
	
	PubGetCommParam(&stCommParam);
	GetAppCommParam(&stAppCommParam);
	SetDownloadParam(&stDownloadParam);
	
	stCommParam.ShowFunc = NULL;
	switch(stCommParam.cCommType & 0xF0)
	{
	case COMMTYPE_SYNDIAL:
	case COMMTYPE_ASYNDIAL:
		return APP_FAIL;
		break;
	case COMMTYPE_GPRS:
		stCommParam.cCommType = COMMTYPE_GPRS_TPDU_HEADLEN;
		memcpy(stCommParam.ConnInfo.stGprsParam.sTPDU, stAppCommParam.sTMSTpdu, 5);
		break;
	case COMMTYPE_CDMA:
		stCommParam.cCommType = COMMTYPE_CDMA_TPDU_HEADLEN;
		memcpy(stCommParam.ConnInfo.stCdmaParam.sTPDU, stAppCommParam.sTMSTpdu, 5);
		break;
	case COMMTYPE_ETH:
		stCommParam.cCommType = COMMTYPE_ETH_TPDU_HEADLEN;
		memcpy(stCommParam.ConnInfo.stEthParam.sTPDU, stAppCommParam.sTMSTpdu, 5);
		break;
	case COMMTYPE_WIFI:
		stCommParam.cCommType = COMMTYPE_WIFI_TPDU_HEADLEN;
		memcpy(stCommParam.ConnInfo.stWifiParam.sTPDU, stAppCommParam.sTMSTpdu, 5);
		break;
	default:
		return APP_FAIL;
	}
	if(stCommParam.cCommType == COMMTYPE_GPRS_TPDU_HEADLEN
		|| stCommParam.cCommType == COMMTYPE_CDMA_TPDU_HEADLEN
		|| stCommParam.cCommType == COMMTYPE_ETH_TPDU_HEADLEN
		|| stCommParam.cCommType == COMMTYPE_WIFI_TPDU_HEADLEN)
	{
		memcpy(stCommParam.stServerAddress.lszIp[0], stAppCommParam.szTMSIp1, 16);
		memcpy(stCommParam.stServerAddress.lszIp[1], stAppCommParam.szTMSIp2, 16);
		stCommParam.stServerAddress.lnPort[0] = atoi(stAppCommParam.szTMSPort1);
		stCommParam.stServerAddress.lnPort[1] = atoi(stAppCommParam.szTMSPort2);
		stCommParam.cSslFlag = 0;
	}
	TMS_InitTMSModule(&stCommParam, &stDownloadParam);
	TMS_CheckUpdate();
	SetControlCommInit();
	ParseParamFromTMS();
	CommInit();
}
#endif

#endif
/* End of this file */
