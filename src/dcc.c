/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  ********************             
** File name:  dcc.c
** File indentifier: 
** Synopsis:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2017-4-25
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content:   
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

STDCCITEM gstDccItem[MAX_DCC_ITEM];


int AddDccTlvItem(char *pszTag, int nLen, char *psValue, char *psOut, int *pnOutLen)
{
	int nPos = 0;
	char szLen[4+1] = {0};

	nPos = *pnOutLen;
		
	memcpy(psOut + nPos, pszTag, 2);
	nPos += 2;
	sprintf(szLen, "%02d", nLen);
	memcpy(psOut + nPos, szLen, 2);
	nPos += 2;
	memcpy(psOut + nPos, psValue, nLen);
	nPos += nLen;
	
	*pnOutLen += nPos;
	
	return APP_SUCC;
}



int DecodeDccTLv(char *psStr, int nTotalLen)
{
	int nPos = 0;
	char szTag[2+1] = {0};
	int nLen = 0;
	char sValue[512] = {0};
	int i = 0;

	memset(&gstDccItem, 0, 32*sizeof(STDCCITEM));
	
	while (1)
	{
		if (nPos >= nTotalLen)
			break;
		
		memcpy(szTag, psStr+nPos, 2);
		if (APP_SUCC != PubIsDigitStr(szTag))
		{
			return APP_QUIT;
		}
		nPos += 2;
		
		PubBcdToInt(psStr+nPos, &nLen);
		nPos += 2;

		memset(sValue, 0, sizeof(sValue));
		memcpy(sValue, psStr+nPos, nLen);
		nPos += nLen;

		strcpy(gstDccItem[i].szTag, szTag);
		gstDccItem[i].nLen = nLen;
		memcpy(gstDccItem[i].sValue, sValue, nLen);
		i++;
		
	}
	return APP_SUCC;
}

int GetDccTlvItem(char *pszTag, char szValue, int *pnLen)
{
	int i = 0;

	while (1)
	{
		if (i >= MAX_DCC_ITEM)
		{
			break;
		}
		if (memcmp(pszTag, gstDccItem[i].szTag, 2) == 0)
		{
			memcpy(szValue, gstDccItem[i].sValue, gstDccItem[i].nLen);
			*pnLen = gstDccItem[i].nLen;
			return APP_SUCC;
		}
		i++;
	}

	*pnLen = 0;
	return APP_FAIL;
}


int PackDccDE48(char *psDE48, int *pnLen)
{
	int nLen = 0;
	char sBuf[128] = {0};
	
	AddDccTlvItem("01", 2, "07", psDE48, pnLen);	
	nLen = EmvEntryPointGetData(_EMV_TAG_9F42_IC_APPCURCODE, (uchar *)sBuf, sizeof(sBuf));
	if (nLen > 0)
	{
		AddDccTlvItem("10", 3, sBuf, psDE48, pnLen);
	}
	else
	{
		AddDccTlvItem("10", 3, "000", psDE48, pnLen);
	}
	nLen = 0;
	nLen = EmvEntryPointGetData(_EMV_TAG_5F28_IC_ISSCOUNTRYCODE, (uchar *)sBuf, sizeof(sBuf));
	if (nLen > 0)
	{
		AddDccTlvItem("11", 3, sBuf, psDE48, pnLen);
	}
	else
	{
		AddDccTlvItem("11", 3, "000", psDE48, pnLen);
	}

	return APP_FAIL;
}

static int CalGetDccAmount(STSYSTEM *pstSystem)
{
	char szRate[128] = {0};
	char sValue[4]= {0}, szValue[8+1] = {0} ;
	int nLen = 0;
	int nPos = 0;
	float fAmount = 0;

	GetDccTlvItem(DCC_TAG_FXRATE, sValue, &nLen);// 4 bytes
	PubHexToAsc((uchar *)sValue, 8, 0, (uchar *)szValue);
	memcpy(szRate, sValue+1, 8-sValue[0]-1);
	nPos += 8-sValue[0]-1;
	memcpy(szRate + nPos, ".", 1);
	nPos ++;
	memcpy(szRate + nPos, sValue + sValue[0], sValue[0]);
	szRate[7] = 0;

	sprintf(pstSystem->szDccAmount, "%f", AtoLL(pstSystem->szAmount)/atof(szRate));
	PubAddSymbolToStr(pstSystem->szDccAmount, 12, '0',ADDCH_MODE_BEFORE_STRING);	

	strcpy(pstSystem->szLocalAmount, pstSystem->szAmount);
	strcpy(pstSystem->szAmount, pstSystem->szDccAmount);
	
	return APP_SUCC;
}

int MagDCCRequst(STSYSTEM *pstSystem)
{
	int nTrueInput=0;
	char *pszTitle = "DCC REQUST";
	char szTmp[128] = {0};
	char szField[999] = {0};
	int nLen = 0;
	char sValue[128] = {0};

	pstSystem->cTransType = (char)TRANS_DCC_REQUEST;

	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(pszTitle);

	/**
	* Check Connect again
	*/
	ASSERT_HANGUP_QUIT(CommConnect());
	DealSystem(pstSystem);

	memcpy(pstSystem->szMsgID, "0600", 4);
	memcpy(pstSystem->szProcCode, "310000", 6);

	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(SetField(0, pstSystem->szMsgID, 4));	
	ASSERT_HANGUP_FAIL(SetField(2, pstSystem->szPan, strlen(pstSystem->szPan)));
	ASSERT_HANGUP_FAIL(SetField(3, pstSystem->szProcCode, 6));
	ASSERT_HANGUP_FAIL(SetField(4, pstSystem->szAmount, 12));//local
	memcpy(szTmp, pstSystem->szDate, 4);
	memcpy(szTmp + 4, pstSystem->szTime, 6);
	ASSERT_HANGUP_FAIL(SetField(7, szTmp, strlen(szTmp)));
	ASSERT_HANGUP_FAIL(SetField(11, pstSystem->szTrace, 6));
	ASSERT_HANGUP_FAIL(SetField(19, COUNTRYCODE, 3));
	ASSERT_HANGUP_FAIL(SetField(32, ACQUIRING_ID, strlen(ACQUIRING_ID)));
	
	ASSERT_HANGUP_FAIL(SetField(41, pstSystem->szPosID, 8));
	ASSERT_HANGUP_FAIL(SetField(42, pstSystem->szShopID, 15));

	
	ASSERT_HANGUP_FAIL(SetField(42, pstSystem->szShopID, 15));

	//DE48
	ASSERT_HANGUP_FAIL(PackDccDE48(szField, &nLen));
	ASSERT_HANGUP_FAIL(SetField(48, szField, nLen));
	ASSERT_HANGUP_FAIL(SetField(49, COUNTRYCODE, 3));

	ASSERT_HANGUP_FAIL(DealPackAndComm(pszTitle,DPC_DEFAULT,pstSystem,NULL,3));

	nLen = 12;
	GetField(5, pstSystem->szDccAmount, &nLen);

	nLen = 10;
	GetField(10, pstSystem->szDccRate, &nLen);

	nLen = 3;
	GetField(50, pstSystem->szDccCurrencyCode, &nLen);
	
	pstSystem->nField63Len = 256;
	GetField(63, pstSystem->szField63, &pstSystem->nField63Len);
	
	//alpha currency code
	ASSERT_FAIL(DecodeDccTLv(szField, nLen));
	GetDccTlvItem(DCC_TAG_ALPHACC, sValue, &nLen);

	//dcc current code 
	GetDccTlvItem(DCC_TAG_NUMERICCC, sValue, &nLen);//bcd
	if (nLen > 0)
	{
		PubHexToAsc((uchar *)sValue, nLen, 0, (uchar *)pstSystem->szDccCurrencyCode);
	}
	
	GetDccTlvItem(DCC_TAG_FXMARGIN, sValue, &nLen);
	CalGetDccAmount(pstSystem);
	GetDccTlvItem(DCC_TAG_CURRECYCOMPONENT, sValue, &nLen);
	GetDccTlvItem(DCC_TAG_DBTVERSIONNUMBER, sValue, &nLen);
	if (nLen > 0)
		SetVarDBTVersion(sValue, nLen);

	pstSystem->cIsUseDcc = 1;
	
	return APP_SUCC;;
}

static int ParseDBTData(int nCurrentPage, int *pnTotalPage, int *pnPos)
{
	char szField[999] = {0};
	int nLen = 0;
	char szValue[512] = {0};
	
	nLen = 999;
	memset(szField, 0, sizeof(szField));
	ASSERT_QUIT(GetField(63, szField, &nLen));

	ASSERT_QUIT(DecodeDccTLv(szField, nLen));
	
	GetDccTlvItem(DCC_TAG_DBTVERSION, szValue, &nLen);
	if (6 == nLen)
	{
		SetVarDBTVersion(szValue);
	}

	GetDccTlvItem(DCC_TAG_TOTALPAGE, szValue, &nLen);
	PubBcdToByte(szValue[0], pnTotalPage);
	if (nCurrentPage == 0 && (*pnTotalPage > 0))
	{
		NDK_FsDel(FILE_DCCCARDBIN);
	}

	GetDccTlvItem(DCC_TAG_CARDBINVALUE, szValue, &nLen);
	if (nLen > 0)
	{
		SaveToFile(szValue, *pnPos, nLen, FILE_DCCCARDBIN);
		*pnPos += nLen;
	}
	
	
}

int DccDownloadRequst(char cTransType)
{
	int nTrueInput=0;
	char szTitle[32] = "DBT DOWNLOAD";
	char szTmp[128] = {0};
	char szField[128] = {0};
	int nLen = 0;
	char szCurrentPag[2+1];
	int nPage = 0;
	int nTotalPage = 0;
	int nPos = 0;
	STSYSTEM stSystem;

	memset(&stSystem, 0, sizeof(STSYSTEM));
	stSystem.cTransType = cTransType;
	if (TRANS_DCC_DISCOLSURE == cTransType)
	{
		strcpy(szTitle, "DCC DISCLOSURE DOWNLOAD");
	}
	
	/**
	* Execute Reveral
	*/
	ASSERT_HANGUP_QUIT(DoAdviseAndReveral());
	PubDisplayTitle(szTitle);


	while (1)
	{		
		/**
		* Check Connect again
		*/
		ASSERT_HANGUP_QUIT(CommConnect());
		DealSystem(&stSystem);

		memcpy(stSystem.szMsgID, "0600", 4);
		memcpy(stSystem.szProcCode, "320000", 6);

		/**
		* Begin to pack
		*/
		ClrPack();
		ASSERT_HANGUP_FAIL(SetField(0, stSystem.szMsgID, 4));	
		ASSERT_HANGUP_FAIL(SetField(3, stSystem.szProcCode, 6));
		ASSERT_HANGUP_FAIL(SetField(4, stSystem.szAmount, 12));//local
		memcpy(szTmp, stSystem.szDate, 4);
		memcpy(szTmp + 4, stSystem.szTime, 6);
		ASSERT_HANGUP_FAIL(SetField(7, szTmp, strlen(szTmp)));
		ASSERT_HANGUP_FAIL(SetField(11, stSystem.szTrace, 6));
		ASSERT_HANGUP_FAIL(SetField(19, COUNTRYCODE, 3));
		ASSERT_HANGUP_FAIL(SetField(32, ACQUIRING_ID, strlen(ACQUIRING_ID)));
		
		ASSERT_HANGUP_FAIL(SetField(41, stSystem.szPosID, 8));
		ASSERT_HANGUP_FAIL(SetField(42, stSystem.szShopID, 15));

		//DE63
		if (TRANS_DCC_DBT == cTransType)
		{
			AddDccTlvItem(DCC_TAG_FUNCTIONCODE, 2, "04", szField, &nLen);
			sprintf(szCurrentPag, "%02d", nPage);
			AddDccTlvItem(DCC_TAG_PAGENUM, 2, szCurrentPag, szField, &nLen);
			AddDccTlvItem(DCC_TAG_BASECURRENCY, 3, COUNTRYCODE, szField, &nLen);
		}
		else
		{
			AddDccTlvItem(DCC_TAG_FUNCTIONCODE, 2, "05", szField, &nLen);
		}
		ASSERT_HANGUP_FAIL(DealPackAndComm(szTitle,DPC_DEFAULT, &stSystem,NULL,3));

		ASSERT_FAIL(ParseDBTData(nPage, &nTotalPage, &nPos));
		if (nPage >= nTotalPage)
		{
			break;
		}
		nPage ++;

	}
	
	return APP_SUCC;;
}

YESORNO CheckIsInsideDBT(char *pszPan)
{
	int fp;
	int nReadLen = -1, nFileLen = 0;
	int nPerLen  = 10;
	char szCardBin[128] = {0};
	
	if((fp=NDK_FsOpen(FILE_DCCCARDBIN,"r"))<0)
	{
		NDK_FsClose(fp);
		return NO;
	}

	while (1)
	{
		memset(szCardBin, 0, sizeof(szCardBin));
		nReadLen = NDK_FsRead(fp, szCardBin, nPerLen);
		if(nReadLen != nPerLen)
		{
			NDK_FsClose(fp);
			return NO;
		}
		PubDelSymbolFromStr(szCardBin, 'F');
	PubDebug("cardbin: %s", szCardBin)	;
		if (memcmp(pszPan, szCardBin, strlen(szCardBin)) == 0)
		{
			return YES;
		}
	}

	NDK_FsClose(fp);	
	return NO;
}


YESORNO CheckIsGoDcc(char *pszPan, char cTransAttr)
{
	int nLen = 0;
	char sBuf[128] = {0};

	if (YES != GetVarIsDCC())
	{
		return NO;
	}
	
	if (ATTR_MAG != cTransAttr || ATTR_MANUAL != cTransAttr)
	{
		nLen = EmvEntryPointGetData(_EMV_TAG_9F42_IC_APPCURCODE, (uchar *)sBuf, sizeof(sBuf));
		if (nLen > 0)
		{
			if (memcmp(sBuf, COUNTRYCODE, 3) == 0)
			{
				return YES;
			}
			else
			{
				return NO;
			}
		}
		nLen = EmvEntryPointGetData(_EMV_TAG_5F28_IC_ISSCOUNTRYCODE, (uchar *)sBuf, sizeof(sBuf));
		if (nLen > 0)
		{
			if (memcmp(sBuf, COUNTRYCODE, 3) == 0)
			{
				return YES;
			}
			else
			{
				return NO;
			}
		}
	}
	return CheckIsInsideDBT(pszPan);
}

int PackDCCFinancial(char *psDE63, int *pnLen)
{
	AddDccTlvItem(DCC_TAG_FUNCTIONCODE, 2, "03", psDE63, pnLen);
	
	return APP_SUCC;
}


