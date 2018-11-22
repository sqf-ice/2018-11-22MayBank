/**
* @file inout.c
* @brief �������ģ��
* @version  1.0
* @author �Ž�
* @date 2007-01-22
*/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/**
* @brief Get Pin
* @param in pszPan ���ڼ����õĿ���
* @param in pszAmount ������ʾ�õĽ��
* @param in pszPin �������Ĺ̶�8���ֽ�
* @return ����
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


