/**
* @file inout.c
* @brief 输入输出模块
* @version  1.0
* @author 张捷
* @date 2007-01-22
*/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/**
* @brief Get Pin
* @param in pszPan 用于加密用的卡号
* @param in pszAmount 用于显示用的金额
* @param in pszPin 密码密文固定8个字节
* @return 返回
*/

int GetPin(const char *pszPan, const char *pszAmount, char *pszPin)
{
	int nRet, nMainKeyNo = 0, nPinLen=0, nMode = PINTYPE_WITHOUTPAN;
	char szDispAmt[64+1] = {0};

	GetVarMainKeyNo(&nMainKeyNo);
	PubSetCurrentMainKeyIndex(nMainKeyNo);

	if (NULL != pszAmount)
	{
		/*<Amount for display*/
		ProAmtToDispOrPnt(pszAmount, szDispAmt);
	}

	if(PINTYPE_WITHPAN == GetVarPinEncyptMode())
	{
		nMode = PINTYPE_WITHPAN;
	}

	if (YES == GetVarIsPinpad())
	{
		PubClear2To4();
		if (NULL != pszAmount)
		{
			/*<Amount for display*/
			PubDisplayStrInline(DISPLAY_MODE_TAIL, 2, "%sRM", szDispAmt);
		}		
		PubDisplayStrInline(0, 3, tr("Pls Input Pin"));
		PubUpdateWindow();
	}
	nRet = PubGetPin(pszPin, &nPinLen, nMode, pszPan, pszAmount, PIN_MAX_LEN, PIN_MIN_LEN);
	if (nRet != APP_SUCC)
	{
		if (nRet == APP_QUIT)
		{
			return APP_QUIT;
		}
		//Get PIN error message
		PubDispErr(tr("Get Pin"));
		return APP_FAIL;
	}
	return APP_SUCC;
}


