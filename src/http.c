/**
* @file http.c
* @brief http post请求模块
* @version  1.0
* @author
* @date 2014年9月15日
*/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"


//这个函数是关于在HTTP传输数据过程中的  状态，比如datain以及dataout等信息。
static int OnDebug(CURL *pcurl, curl_infotype itype, char * psData, size_t size, void *p)  
{  
	//根据官方手册说明，pData不一定是以0结尾的，但pData的长度肯定等于size，所以要借助这个变量处理下
	char szTmp[2048] = {0};	
	STSHOWINFOXY stShowInfoxy;
	
	if(size > sizeof(szTmp) - 1)
	{
		//碰到这种情况直接退出，防止跑飞
		TRACE(("pData Overlength"));  
		return 0; 
	}
	memcpy(szTmp, psData, size);
	
	if(itype == CURLINFO_TEXT)  
	{  
	    	TRACE(("[TEXT]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_HEADER_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
	    	TRACE(("[HEADER_IN]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_HEADER_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	TRACE(("[HEADER_OUT]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_DATA_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
    	TRACE(("[DATA_IN]:[%s]", szTmp));  
	}  
	else if(itype == CURLINFO_DATA_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	TRACE(("[DATA_OUT]:[%s]", szTmp)); 
	}  
	else if(itype == CURLINFO_SSL_DATA_IN)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
		memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
		stShowInfoxy.nColumn = 9;
		stShowInfoxy.nRow = 4;
		stShowInfoxy.nType = 0;
		PubSetShowXY(stShowInfoxy);		
    	//TRACE_HEX((szTmp, size, "[SSL_DATA_IN]:")); 
	} 
	else if(itype == CURLINFO_SSL_DATA_OUT)  
	{  
		PubClear2To4();
		PubDisplayStrInline(0, 2, "");
		PubDisplayStrInline(0, 3, "Processing...");
		PubDisplayStrInline(0, 4, "");
		PubUpdateWindow();
    	//TRACE_HEX((szTmp, size, "[SSL_DATA_OUT]:")); 
	} 
	return 0;  
}  

/**
* @brief 写数据回调函数(libcurl原型)
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date 
*/
static size_t WriteDataFunc(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    int nFd;
    size_t nWriteSize;

	nFd = *((int *)user_p);
    nWriteSize = NDK_FsWrite(nFd, buffer, size * nmemb);
    
    return nWriteSize;
}

/**
* @brief 发送接收数据
* @param		[in]		pszTitle Title
* @param		[out]	pstHttpPacket http参数字段
* @return 
* @li APP_SUCC
* @li APP_FAIL
* @author 
* @date 
*/
int HttpSendRecv(char *psBuf, int *pnLen, char cTransType)
{
	CURL *pCurl = NULL;
	CURLcode res;
	struct curl_slist *pList = NULL;
	int nRet = APP_SUCC;
	int i;
	int nFd;
	char szUrl[MAX_PACK_SIZE] = {0}, szHost[100+1]={0};
	char szContent[100 + 1];
	char szDateTime[14+1];

	switch(cTransType)
	{
	case TRANS_QR_REGISTER:
		strcpy(szUrl, DOMAIN_QR_REGISTER);
		NDK_FsDel(REGISTERFILE);
		nFd = NDK_FsOpen(REGISTERFILE, "w");
		if(nFd < 0)
		{	
			return APP_FAIL;
		}
		break;
	case TRANS_QR_LOGON:
		strcpy(szUrl, DOMAIN_QR_LOGON);
		NDK_FsDel(LOGONFILE);
		nFd = NDK_FsOpen(LOGONFILE, "w");
		if(nFd < 0)
		{	
			return APP_FAIL;
		}
		break;
	default:
		break;
	}

	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	
	curl_global_init(CURL_GLOBAL_ALL);
	pCurl = curl_easy_init();
	if (NULL == pCurl)
	{
		NDK_FsClose(nFd);
		curl_global_cleanup();
		PubMsgDlg(NULL, "Init CURL Err", 1, 3);
		return APP_FAIL;
	}

	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	PubDebug("\n\n psBuf = %s \n\n", psBuf);
	pList = curl_slist_append(pList, "Content-Type:application/json");
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(pCurl, CURLOPT_URL, szUrl);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, psBuf);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteDataFunc);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &nFd);
	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, OnDebug); 

	
	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	for(i = 0; i < 3; i++)
	{
		res = curl_easy_perform(pCurl);
		if(CURLE_OK == res)
			break;
	}
	PubDebug("\n\n i = %d\n\n", i);
	SetStatusBar(STATUSBAR_OTHER_OPEN);
	NDK_FsClose(nFd);
	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	if (res == CURLE_OK)
	{
		long retcode = 0;
	    res = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &retcode);
		PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	    if ((CURLE_OK == res) && (retcode != 200) && (retcode != 400))
	    {
	    	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	    	memset(szContent, 0, sizeof(szContent));
			sprintf(szContent, "Network Busy:%ld\n, Pls try again later", retcode);	
			PubMsgDlg(NULL, szContent, 1, 30);
			nRet = APP_FAIL;
	    }
	}
	else
	{  
		PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
  		memset(szContent, 0, sizeof(szContent));
		sprintf(szContent, "Comm Err:%d\n, Pls try again later", res);
		PubMsgDlg(NULL, szContent, 1, 30);
		nRet = APP_FAIL;
	}
	curl_easy_cleanup(pCurl);
	curl_global_cleanup();
	PubDebug("\n\n %s  %d \n\n", __FUNCTION__, __LINE__);
	return nRet;
}

