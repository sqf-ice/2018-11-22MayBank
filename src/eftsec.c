#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"


int AddTLEH(char *psBuf, int *pnLen, int nPctTextLen, int nIndex)
{
	char sTemp[999] = {0};
	int nLen = 0;
	char sKsn[20] = {0};
	int nBcdLen = 0;

	memcpy(sTemp, psBuf, *pnLen);
	
	memcpy(psBuf+nLen, "HTLE", 4);
	nLen+=4;
	memcpy(psBuf+nLen, "04", 2);
	nLen+=2;
	GetVarAcqId(&psBuf[nLen]);
	nLen+=3;
	GetVarTerminalId(&psBuf[nLen], nIndex);
	nLen+=8;
	// 2 is DUKPT key per ANSI X9.24-Part1-2009 without response key
	memcpy(psBuf+nLen, "2321", 4);//Encryption Method 2321 
	nLen+=4;
	GetKSN(sKsn);
	memcpy(psBuf+nLen, sKsn, 10); 
	nLen+=10;
	nBcdLen = 2;
	PubIntToBcd(&psBuf[nLen], &nBcdLen, nPctTextLen);
PubDebugData(" bcd len； ", psBuf+nLen, nBcdLen);
	nLen+=2;
	
	psBuf[nLen] = 'P';//Reserved
	nLen++;
	memset(psBuf+nLen, 0x00, 7);
	nLen+=7;
	
	memcpy(psBuf+nLen, sTemp, *pnLen);
	*pnLen += nLen;
	
	return APP_SUCC;
}


int DelTLEH(char *psBuf, int *pnLen, int *pnProtextLen)
{
	char sTemp[999] = {0};

	PubBcdToInt(psBuf+31, pnProtextLen);
PubDebugData("pretect len: ", psBuf+31, 2);
PubDebug("result: %d", *pnProtextLen);
	memcpy(sTemp, psBuf, *pnLen);
	memcpy(psBuf, sTemp+41, *pnLen-41);
	*pnLen -= 41;
			
	return APP_SUCC;
}

int DelRandom(char *psBuf, int *pnLen)
{
	char sTemp[999] = {0};

	memcpy(sTemp, psBuf, *pnLen);

	memcpy(psBuf, sTemp+8, *pnLen-8);
	*pnLen -= 8;
			
	return APP_SUCC;
}

#if 0
int ClacDukptDes(char *psSrc, int nLen, char *psDest)
{
	int nIndex = GetVarDukptID();
	
	PubDebug("group no = %d ", nIndex);
	ASSERT_FAIL(PubDukptEncryption(nIndex, SEC_KEY_TYPE_TDK, psSrc, nLen, psDest, SEC_DES_ENCRYPT|SEC_DES_MODE_CBC));

	return APP_SUCC;
}

int ClacDukptDesDecrypt(char *psSrc, int nLen, char *psDest)
{
	int nIndex = GetVarDukptID();
	
	ASSERT_FAIL(PubDukptEncryption(nIndex, SEC_KEY_TYPE_RESP_TDK, psSrc, nLen, psDest, SEC_DES_DECRYPT|SEC_DES_MODE_CBC));

	return APP_SUCC;
}

int ClacDukptMac(char *psSrc, int nLen, char *psMac)
{
	int nIndex = GetVarDukptID();

	ASSERT_FAIL(PubDukptMac(nIndex, psSrc, nLen, psMac, SEC_MAC_X99));
	memset(psMac+4, 0x00, 4);

	return APP_SUCC;
}

int ClacDukptMacResponse(char *psSrc, int nLen, char *psMac)
{
	int nIndex = GetVarDukptID();

	ASSERT_FAIL(PubDukptMac(nIndex, psSrc, nLen, psMac, SEC_MAC_RESP_X99));
	memset(psMac+4, 0x00, 4);

	return APP_SUCC;
}
#endif

int GetDecFrmOct(uchar ucOct)
{
	int nDE = 0x00;

	switch (ucOct)
	{
	case 0x00:
		nDE = 0;
		break;
	case 0x01:
		nDE = 1;
		break;
	case 0x02:
		nDE = 2;
		break;
	case 0x03:
		nDE = 3;
		break;
	case 0x04:
		nDE = 4;
		break;
	case 0x05:
		nDE = 5;
		break;
	case 0x06:
		nDE = 6;
		break;
	case 0x07:
		nDE = 7;
		break;
	default:
		nDE = 0;
	}
	return nDE;
}


uchar GetOctFrmDec(int nDec)
{
	uchar ucOct = 0x00;

	switch (nDec)
	{
	case 0:
		ucOct = 0x00;
		break;
	case 1:
		ucOct = 0x01;
		break;
	case 2:
		ucOct = 0x02;
		break;
	case 3:
		ucOct = 0x03;
		break;
	case 4:
		ucOct = 0x04;
		break;
	case 5:
		ucOct = 0x05;
		break;
	case 6:
		ucOct = 0x06;
		break;
	case 7:
		ucOct = 0x07;
		break;

	default:
		ucOct = 0x00;
	}
	return ucOct;
}

// tag 1 is LSB, tag2 is MSB .. if only 1 uint8_t of tag - use only bTag1
int GetDEFromTag(uchar ucTag1, uchar ucTag2)
{
	int nDE = 0;
	int nNum = 0;
	uchar bNib[4];

	bNib[0] = ucTag1;
	bNib[1] = ucTag1;
	bNib[2] = ucTag2;
	bNib[3] = ucTag2;

	bNib[0] &= 0x0F;
	bNib[1] >>= 4;
	bNib[2] &= 0x0F;
	bNib[3] >>= 4;

	nDE = GetDecFrmOct(bNib[0]);
	nNum = GetDecFrmOct(bNib[1]);
	nDE += (8 * nNum);
	nNum = GetDecFrmOct(bNib[2]);
	nDE += (64 * nNum);
	nNum = GetDecFrmOct(bNib[3]);
	nDE += (512 * nNum);

	return nDE;
}

// returns the actual int value (dec value) of the tag(s) passed in ... max is 3 tags (2 for length -> max of 65,535
int GetDecLength(uchar* pTag, int* pnNumofTags)
{
	int nValue = 0;
	uchar bValue[4];

	memset((void*)&bValue, 0x00, sizeof(int));
	if (0x81 == *pTag)
	{
		// 2 tags, first one is a tag, second one is the length value
		bValue[0] = *(pTag + 1);
		memcpy((void*)&nValue, (void*)&bValue[0], sizeof(int));
		*pnNumofTags = 2;
	}
	else if (0x82 == *pTag)
	{
		// 3 tags, first one is a tag, second and third represent the length value
		bValue[0] = *(pTag + 2);
		bValue[1] = *(pTag + 1);
		memcpy((void*)&nValue, (void*)&bValue[0], sizeof(int));
		*pnNumofTags = 3;
	}
	else if (0x00 == (0x80 & *pTag))
	{
		// only 1 uint8_t, with msbit set and the next 7 bits represent the length value
		bValue[0] = 0x7F & *pTag;
		memcpy((void*)&nValue, (void*)&bValue[0], sizeof(int));
		*pnNumofTags = 1;
	}
	else
	{
		// something very wrong here - length value not supported .. eg more than 65535
		nValue = 0;
		*pnNumofTags = 0;
	}
	return nValue;
}

int ParseSentitiveTLV(char *psTLV, int nLen, STSYSTEM *pstSystem)
{
	int nPos = 0;
	int nDE = 0;
	int nValueLen = 0;
	int nLenLen = 0;
	char sValue[512] = {0};
	int nDeLen = 0;

	while (1)
	{
		if (nPos >= nLen)
		{
			break;
		}
//parse
//PubDebug("================");
		nDE = GetDEFromTag(*(psTLV+nPos), 0);
		nPos++;
//PubDebug("T: %d", nDE);
		
		nValueLen = GetDecLength((uchar *)(psTLV+nPos), &nLenLen);
		nPos+=nLenLen;
//PubDebug("L: %d", nValueLen);
		
		memset(sValue, 0, sizeof(sValue));
		memcpy(sValue, psTLV+nPos, nValueLen);
		nPos+=nValueLen;
//PubDebugData("V: ", sValue, nValueLen);
		switch (nDE)
		{
		case 2:
			PubBcdToByte(sValue[0], &nDeLen);
			PubHexToAsc((uchar *)sValue+1, nDeLen, 0, (uchar *)pstSystem->szPan);			
			SetField(2, pstSystem->szPan, strlen(pstSystem->szPan));
			break;
		case 35:
			PubBcdToByte(sValue[0], &nDeLen);
			PubHexToAsc((uchar *)sValue+1, nDeLen, 0, (uchar *)pstSystem->szTrack2);
			SetField(35, pstSystem->szTrack2, strlen(pstSystem->szTrack2));
			break;
		case 14:
			PubHexToAsc((uchar *)sValue, nValueLen*2, 0, (uchar *)pstSystem->szExpDate);
			SetField(14, pstSystem->szExpDate, strlen(pstSystem->szExpDate));
			break;
		case 48:
			PubBcdToInt(sValue, &nDeLen);
			memcpy(pstSystem->szCVV2, sValue+2, nDeLen);
			SetField(48, pstSystem->szCVV2, nDeLen);
			break;
		case 55:
			PubBcdToInt(sValue, &nDeLen);
			pstSystem->nAddFieldLen = nDeLen;
			memcpy(pstSystem->sField55, sValue+2, nDeLen);
			SetField(55, pstSystem->sField55, nDeLen);
			break;
		default:
			break;
		}	
	}
	
	return APP_SUCC;
}

//type 01 padding random
int GetIkeyFromEB(char *psIkey, char *psEB)
{
	int i = 1;
	
	while (1)
	{
		if (i == 256)
		{
			break;
		}
		if (0 == *(psEB+i))
		{
			i++;
			break;
		}
		i++;
	}

	memcpy(psIkey, psEB+i, 256-i);
	
	return APP_SUCC;
}

int ProcessKmsData(ST_RSA_PRIVATE_KEY *pstPrivate)
{
	char sDest[999] = {0};
	char sLTMK[16+1] = {0};
	char sBlock[8+1] = {0}, sBckKcv[8+1] = {0};
	char szAscKcv[16+1] = {0};
	char sKSN[20+1] = {0}, sKsnBcd[10+1] = {0};
	int nDukptId = 0;
	char sECB_EN[256+1] = {0};
	int nFieldLen = 0;
	char szRpF62[999] = {0};
	int nPos = 0;

	nFieldLen= 512;
	ASSERT_QUIT(GetField(62, szRpF62, &nFieldLen));

	nPos += 11;
	memcpy(sECB_EN, szRpF62+nPos, 256);
	nPos += 256;
	PubDebugData("sECB_EN: ", sECB_EN, 256);
	ASSERT_QUIT(NDK_AlgRSARecover(pstPrivate->modulus, pstPrivate->bits/8, pstPrivate->exponent, (uchar *)sECB_EN, (uchar *)sDest));
	PubDebugData("dest: ", sDest, 256);
	GetIkeyFromEB(sLTMK, sDest);
	PubDebugData("LTMK: ", sLTMK, 16);
	
	//此处根据返回结果确认是否需要去掉pad
	PubSoftDes3(sLTMK, sBlock, sBckKcv);
	PubHexToAsc((uchar *)sBckKcv, 6, 0, (uchar *)szAscKcv);
PubDebugData("src kcv: ", szRpF62+nPos, 6);	
PubDebugData("dst kcv: ", szAscKcv, 6)	;
	if (0 != memcmp(szRpF62+nPos, szAscKcv, 6))
	{
		PubMsgDlg("DECRYPT LTMK", "KCV mismatch", 3, 3);
		return APP_QUIT;
	}
	nPos += 6;
	
	memcpy(sKSN, szRpF62+nPos, 20);
PubDebug("ksn: %s", sKSN);
	PubAscToHex((uchar *)sKSN, 20, 0, (uchar *)sKsnBcd);
	nDukptId = GetVarDukptID();	
	
#if 0
	return PubLoadDukptIPEK(nDukptId, sLTMK, 16, sKsnBcd);
#endif

	return 0;
}



int GetTEIdPin(char *pszTitle, char *pszTEid, char *pszTEpin)
{
	int nLen = 0;
	
	ASSERT_QUIT(PubInputDlg(pszTitle, "Input TE ID:", pszTEid, &nLen, 1, 8, 60, INPUT_MODE_STRING));
	ASSERT_QUIT(PubInputDlg(pszTitle, "Input TE Pin:", pszTEpin, &nLen, 1, 8, 60, INPUT_MODE_STRING));

	return APP_SUCC;
}

int IncreaseKSN()
{
	int nIndex = GetVarDukptID();
	
	return NDK_SecIncreaseDukptKsn(nIndex);
}

int PackTLEDAndMAC(char *psBuf, int *pnLen, STSYSTEM *pstSystem)
{
	int nPackLen = *pnLen;
	char sMac[8] = {0};
		
	nPackLen -= 8;
	ASSERT_FAIL(AddMac(psBuf, &nPackLen, KEY_TYPE_MAC));
#if 0
	GetMac(psBuf, &nPackLen, sMac);
	ASSERT_HANGUP_FAIL(ProcessTLED(pstSystem));
	ASSERT_HANGUP_FAIL(Pack(psBuf, &nPackLen));
	memcpy(psBuf+nPackLen-8, sMac, 8);
#endif
	return APP_SUCC;
}

#if 0
int ProcessTLED(STSYSTEM *pstSystem)
{
	char sRandom[8+1] = {0};
	char sTLED[999] = {0}, sDE57[999] = {0};
	int nLen = 0;
	int nPretectedTextLen = 0;
	char sPretectedText[512] = {0};

	//protected text
	if (YES == IsNeedToSendDE2(pstSystem))
	{
		if (strlen(pstSystem->szPan) > 0)
		{
			ASSERT_QUIT(PackToTLV(2, pstSystem->szPan, strlen(pstSystem->szPan), sPretectedText, &nPretectedTextLen));
		}
		if (ChkDigitStr(pstSystem->szExpDate, 4) == APP_SUCC)
		{
			ASSERT_QUIT(PackToTLV(14, pstSystem->szExpDate, strlen(pstSystem->szExpDate), sPretectedText, &nPretectedTextLen));
		}
	}
	if (YES == IsNeedToSendDE35(pstSystem))
	{
		if (strlen(pstSystem->szTrack2) > MIN_TK2 && strlen(pstSystem->szTrack2) <= MAX_TK2)
		{
			ASSERT_QUIT(PackToTLV(35, pstSystem->szTrack2, strlen(pstSystem->szTrack2), sPretectedText, &nPretectedTextLen));
		}
	}
	if (pstSystem->cTransAttr == ATTR_MANUAL || pstSystem->cTransAttr == ATTR_MAG)
	{	
		if (strlen(pstSystem->szCVV2) > 0 && pstSystem->nAcqIndex != 1)
		{
			ASSERT_QUIT(PackToTLV(48, pstSystem->szCVV2, strlen(pstSystem->szCVV2), sPretectedText, &nPretectedTextLen));
		}
	}
	
	if (pstSystem->nAddFieldLen > 0)
	{
		if (pstSystem->cTransAttr != ATTR_MSD)//add 20170511 
			ASSERT_QUIT(PackToTLV(55, pstSystem->sField55, pstSystem->nAddFieldLen, sPretectedText, &nPretectedTextLen));
	}

	nLen = 0;
	if (nPretectedTextLen > 0)
	{
		//salt
		GetRandom(sRandom, 8);
		memcpy(sTLED + nLen, sRandom, 8);
		nLen += 8;
		memcpy(sTLED + nLen, sPretectedText, nPretectedTextLen);
		nLen += nPretectedTextLen;
		//tail
		PadZeroAtRight(sTLED, &nLen);
		ASSERT_FAIL(ClacDukptDes(sTLED, nLen, sDE57));
	}

	AddTLEH(sDE57, &nLen, nPretectedTextLen, pstSystem->nAcqIndex);

	ASSERT_FAIL(SetField(57, sDE57, nLen));
TRACE(("DE57 len=%d", nLen));
	
	return APP_SUCC;
}

int ParseTLED(STSYSTEM *pstSystem)
{
	char sTLED[999] = {0}, sDE57[999] = {0};
	int nLen = 0;
	int nProtext = 0;

	nLen = 999;
	ASSERT_FAIL(GetField(57, sDE57, &nLen));
	if (0 == nLen)
	{
		return APP_SUCC;
	}
PubDebugData("de57: ", sDE57, nLen);

	//delete TLEH
	ASSERT_FAIL(DelTLEH(sDE57, &nLen, &nProtext));
PubDebugData("after delhead: ", sDE57, nLen);

	if (nProtext == 0)
	{
		return APP_SUCC;
	}

	//decrypt
	ASSERT_FAIL(ClacDukptDesDecrypt(sDE57, nLen, sTLED));
PubDebugData("Decrypt result: ", sTLED, nLen);

	//delete random
	ASSERT_FAIL(DelRandom(sTLED, &nLen));
PubDebugData("protect: ", sTLED, nProtext);

	//parse TLV
	ASSERT_FAIL(ParseSentitiveTLV(sTLED, nProtext, pstSystem));
		
	return APP_SUCC;
}
#endif

int PackToTLV(int nDE, char *psProtectDE, int nDeLen, char *psOut, int *pnLen)
{
	int nTagLen = 0, nLenlen = 0, nValueLen = 0;
	char sTag[2+1] = {0};
	char sLen[3+1] = {0};
	char sValue[512] = {0};
	int nPos = *pnLen;
	int nBcdLen = 2;
	
PubDebug("======pack tlv [DE: %d]======", nDE);
	if (!strlen(psProtectDE))
	{
		return APP_QUIT;
	}

	switch (nDE)
	{
	case 2:
	case 35:
		PubByteToBcd(nDeLen, &sValue[0]);
		PubAscToHex((uchar *)psProtectDE, nDeLen, 0, (uchar *)(sValue+1));
		nValueLen = (strlen(psProtectDE)+1)/2+1;
		break;
	case 14:
		PubAscToHex((uchar *)psProtectDE, nDeLen, 0, (uchar *)(sValue));
		nValueLen = (strlen(psProtectDE)+1)/2;
		break;
	case 48:
	case 55:
		nBcdLen = 2;
		PubIntToBcd(sValue, &nBcdLen, nDeLen);
		memcpy(sValue+2, psProtectDE, nDeLen);
		nValueLen = nDeLen+2;
		break;
	default:
		break;
	}

	//T
	GetTagForDE(nDE, (uchar *)sTag, &nTagLen);
	memcpy(psOut+nPos, sTag, nTagLen);
	nPos += nTagLen;
PubDebugData("T: ", sTag, nTagLen);
	//L
	GetLengthTag(nValueLen, (uchar *)sLen, &nLenlen);
	memcpy(psOut+nPos, sLen, nLenlen);
	nPos += nLenlen;
PubDebugData("L: ", sLen, nLenlen);
	//V
	memcpy(psOut+nPos, sValue, nValueLen);
	nPos += nValueLen;
PubDebugData("V: ", sValue, nValueLen);

	*pnLen = nPos;

	DelField(nDE);
	return APP_SUCC;
}

/**
* @brief Display Error message
* @param in const char *szRespCode 
* @return 返回
*/
int DispRespEFTInfo()
{
	int nLen = 0;
	char szField[512] = {0}, szContent[512] = {0};
	
	nLen = 512;
	ASSERT_QUIT(GetField(63, szField, &nLen));
	sprintf(szContent, "%.2s-%s", szField+2, szField+4);	
	PubMsgDlg("TRANS FAIL", szContent, 3, 30);
	return APP_SUCC;
}

