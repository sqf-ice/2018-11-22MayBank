#ifndef __INTERFACE_CLSS__
#define __INTERFACE_CLSS__

#include "interface.h"

#define MAX_UDOL_LEN 252        /*Unpredictable Number Data Object List (UDOL) Maximum length*/

typedef struct {
    unsigned char _drl_app_id_len; 
    unsigned char _drl_app_id[16]; /*9F5A application ID 1-4 */
    unsigned char _cl_limit_exist; /* limist exist? 
                                    bit 1    =1    
                                    bit 2    =1    contactless limit exist
                                    bit 3    =1    contactless offline limit exist
                                    bit 4    =1    cvm limit  exist 
                                    RFU
                                    bit 7    =1    support status check
                                    bit 8    =1    zero amount allow*/
    unsigned char _cl_trans_limit[6];          /* clss transaction limit n12  6bytes */
    unsigned char _cl_offline_limit[6];        /* contactless offline limit n12  6bytes */
    unsigned char _cl_cvm_limit[6];            /* cvm limit   n12  6bytes */
}drl_param;

/*        *********   Terminal parameters and AID    Parameter structure *******        
        **    support = 1, not support = 0;
        **    no used set 0*/

typedef struct {
    unsigned char usTacDefault[5];          /*default Terminal Action Code*/
    unsigned char usTacDenial[5];           /*denial Terminal Action Code*/
    unsigned char usTacOnline[5];           /*online Terminal Action Code*/
    unsigned char ucTargetPercent;          /*target percent */
    unsigned char ucMaxTargetPercent;      /*max target percent*/
    unsigned char usThresholdValue[4];      /*thresold value*/
    unsigned char usTransRefConv[4];       /*transaction reference currency convert, default 0 */
    unsigned char ucScriptDevLimit;        /*script length limit ,default 0*/
    
    unsigned char usIcs[7];                  /*    ICS    ( Implementation Comformance Statement )
                                            Each bit represents a set of specific, see the following 
                                            [STPARAM.usIcs ICS-related bits] macro definition
                                            e.g.:        (1)Set to support cardholder authentication
                                                        ics_opt_set( AS_Support_CardHolder_Confirm, emvparm.usIcs)
                                                        (2)Set to do't support cardholder authentication
                                                        ics_opt_unset( AS_Support_CardHolder_Confirm, emvparm.usIcs)¡¡*/
    unsigned char ucStatus;                  /* Test type indicator */
    unsigned char ucEcIndicator;            /* 9F7A if Terminal  support EC?
                                                Or paywave: supprt wave2 ?
                                                support  = 1 */
                                            
    unsigned char ucType;                    /* 9F35(Terminal), n2, 1 */
    unsigned char usCap[3];                  /* 9F33(Terminal), b,  3 */
    unsigned char usAddCap[5];              /* 9F40(Terminal), b,  5 */
    unsigned char usAid[16];                 /* 4F(ICC), 9F06(Terminal), b, 5-16 bytes */
    unsigned char ucAidLen;                 /* length of AID */
    unsigned char usAppVer[2];              /* 9F09(Terminal), b, 2 bytes */
    unsigned char ucPosEntry;               /* 9F39(Terminal), n2, 1 bytes */
    unsigned char usFloorlimit[4];           /* 9F1B(Terminal), b, 4 bytes */
    unsigned char usAcqId[6];               /* 9F01(Terminal), n6-11, 6 bytes */
    unsigned char usMerCategoryCode[2];    /* 9F15(Terminal), n4, 2 bytes */
    unsigned char usMerchantId[15];         /* 9F16(Terminal), ans15, 15 bytes */
    unsigned char usTransCurrCode[2];      /* 5F2A(Terminal), n3, 2 bytes */
    unsigned char ucTransCurrExp;          /* 5F36(Terminal), n1, 1 bytes */
    unsigned char usTransRefCurrCode[2];  /* 9F3C(Terminal), n3, 2 bytes */
    unsigned char ucTransRefCurrExp;      /* 9F3D(Terminal), n1, 1 bytes */
    unsigned char usTermCountryCode[2];    /* 9F1A(Terminal), n3, 2 bytes */
    unsigned char usIfdSerialNum[8];       /* 9F1E(Terminal), an8, 8 bytes */
    unsigned char usTerminalId[8];          /* 9F1C(Terminal), an8, 8 bytes */
    unsigned char ucDefaultDdolLen;        /* the length of the default ddol following the merchant name and location */
    unsigned char ucDefaultTdolLen;        /* the length of the default todl following the default ddol */

    unsigned char usDefaultDdol[MAX_DDOL_LEN];        /*default ddol */
    unsigned char usDefaultTdol[MAX_TDOL_LEN-9];        /* default tdol     */
	unsigned char ucRiskManaDataLen;
	unsigned char usRiskManaData[8];    
    unsigned char ucAppSelIndicator;                /*application select indicator 
                                                      when ics support and this byte is 0x01(!0)
                                                      application select support partial match*/
    unsigned char ucFallBackPosentry;                /* fallback pos entry*/
    
    unsigned char ucLimitExist;    /* limist exist?(To determine the identity of the following limit exists)
                                bit 1    =1    EC limint exist
                                bit 2    =1    contactless limit exist
                                bit 3    =1    contactless offline limit exist
                                bit 4    =1    cvm limit  exist*/
    unsigned char usEcLimit[6];             /* 9F7B Electronic cash terminal limit n12  6bytes*/
    unsigned char usClLimit[6];             /* contactless terminal transaction limit n12  6bytes */
    unsigned char usClOfflineLimit[6];     /* contactless terminal offline minimum limit n12  6bytes */
    unsigned char usCvmLimit[6];            /* terminal implement CVM Limit n12  6bytes */


    unsigned char usTransProp[4];           /* 9F66 Terminal transaction attribute b32  4bytes*/
    unsigned char ucStatusCheck;            /*The default of contactless status check is 0*/
    unsigned char ucAppId;                   /* Application ID which identifies that the application supports which transaction 
                                            terminal applications that support the terminal configuration
                                            (To maintain compatibility, full support for all 0)
                                            */
    unsigned char usResvOld[2];             /*Padded 4 bytes*/
    
/************************* The following is version 4.2C new parameter *******************************/
#if 1 //The 4.2C unique field, 4.3A useless reserved
    unsigned char usMerchantName[20];           /* 9F4E(Terminal), ans20, 20 bytes  new 20121207*/
//    unsigned char usResv42c[56];                /*Padded 4 bytes*/
    unsigned char ucPPTlvLen;                
    unsigned char usPPTlv[55];               
    /*Reserved bits are considered for paypass amount limits of different trade types*/
#endif
/************************* The following is Paypass new parameter ******************************/
    unsigned char ucDefaultUdolLen;            /* the length of the default todl following the default ddol (paypass+)*/
    unsigned char usDefaultUdol[MAX_UDOL_LEN];  /*default ddol (paypass+)*/
    unsigned char ucMagStripeIndicator;        /*(paypass+) support mag stripe ?  1 support ;  0 not support [paypass]*/
    unsigned char usMagAppVer[2];              /*paypass mag stripe application version 9F6D*/
    unsigned char ucDataExchangeSupport;       /*support data exchang or not*/
    //unsigned char _on_device_cv_support;       /*On device cardholder verification supported*/
    unsigned char ucKernelConfig;            /* b8  Only EMV mode transactions supported 
                                                b7  Only mag-stripe mode transactions supported 
                                                b6  On device cardholder verification supported 
                                                b5-1  RFU */
    unsigned char ucMaxNumTornLog;            /* Max Number of Torn Transaction Log Records !=0 support Torn*/
    unsigned char ucBalanceReadFlag;           /*00,NO; bit1:Balance Read Before GAC; bit2:Balance Read After GAC,*/

/************************* The following is Paywave new parameter ******************************/
    unsigned char usPwConfig[2];             /* paywave config
                                            byte 1   bit1     (AID different)
                                                        0    when Amount zero, require Online Cryptogram
                                                        1    when Amount zero, Set the Contactless Application Not Allowed
                                                     bit2    RFU
                                                        [1     deactive floor limit check] no use
                                                     bit3    active / disable Amount Zero Check
                                                         1    active Amount zero action
                                                         0   deactive Amount zero action
                                                     bit4 
                                                         1   Domestic Transactions Only
                                                     
                                            byte2    bit1     (termianl ICS config)
                                                        1    support JCB Wave1
                                                     bit2
                                                        1    support JCB Wave2(should support )
                                                     bit3
                                                        1    support JCB Wave3
                                                     bit 4
                                                        1     support Issuer Update Processing
                                                     bit 5
                                                         1    display offline Funds
                                                     bit 6
                                                         1    support dCVV
                                                     bit 7
                                                        1    MSI
                                            */
    unsigned char ucCvmReq;                    /*    paywave2 terminal require reader execute CVM when
                                                    Transaction Amount execeed the CVM Limit 
                                                    01    Signature 
                                                    02    Online Pin */
    unsigned char ucDdaVer;                    /*    0x00    support all dda version
                                                0x01    support only version 0x01 */
    unsigned char usResvPw[4];              /*Padded 4 bytes*/
                        
    /*******Entry Point Configuration Data per Combination A-Table 5-2* ***/
    unsigned char usKernelId[8];//entry point
    unsigned char ucVisaTtqPresent; /*mark TTQ usTransProp status*/

    //ucStatusCheck 
    unsigned char ucStatusCheckSupport;//Status check of CVM online PIN
    unsigned char ucZeroAmountAllow;
    unsigned char ucExtendAidSupport; //Reserved, support AID extend
    unsigned char ucClssCardholderVerifyAllow;/*if needed to confirm contactless APP AID*/
    
    /******** qVSDC support DRL application ID[4]**********/
    unsigned char ucDrlStatus; /*0x00 Deactivated; 0x01 Activated*/
    unsigned char usDrlData[8*36]; /*DRL Data len = 8*36    8 DRL data for original configure file*/
    /* Can not define structure, may lead to a extra 4-bytes length that cause error*/

/************************* Paypass structure *******************************/
    unsigned char ucMagStripeCvm;//Mag-stripe CVM Capability - CVM Required 
    unsigned char usMaxLifetimeTornLog[2]; //Max Lifetime of Torn Transaction Log Record  0x012c
    unsigned char ucMobileSupportIndicator;
    unsigned char ucMagStripeNoCvm;//Mag-stripe CVM Capability - No CVM Required 
    unsigned char ucCapNoCvm;//CVM Capability - no CVM Required

/************************* ExpressPay structure *******************************/
    unsigned char ucEXTerminalCap;/*expresspay 3.0 Terminal capability 9F6D*/
    unsigned char ucEXRandomScope; /*expresspay 3.0 Range of random number*/
    unsigned char ucEXTimeExpire; /*expresspay 3.0 Timeout*/

    /************************************ usReserve for future use ******************************/
    unsigned char usResv[41];
    /*Padded 4 bytes*/
}STCLSSPARAM;

#define    CL_ZERO_OPTION1_2(x)         ( (x) & 0x20 )      /*Zero option1(1) or option2(0)*/
#define    CL_STATUS_CHECK(x)           ( (x) & 0x40 )      /*Support status check*/
#define    CL_ZERO_ALLOW_CHECK(x)       ( (x) & 0x80 )      /*Zero amount allow check*/


/*STPARAM.ucKernelConfig */
/* b8  Only EMV mode transactions supported 
    b7  Only mag-stripe mode transactions supported 
    b6  On device cardholder verification supported 
    b5-1  RFU */
#define KERNEL_CONFIG_ONLY_EMV_MODE(x)          ( (x) & 0x80 )
#define KERNEL_CONFIG_ONLY_MAGSTRIPE_MODE(x)    ( (x) & 0x40 )
#define KERNEL_CONFIG_ON_DEVICE_CV(x)           ( (x) & 0x20 )

/**
 * Self-defined tags for internal usage.
 * These tag names are just defined according to
 * current hash value spread. So if the hash
 * function changes in emvbuf.c leading to
 * a different spread, these tag names may
 * be changed too.
 * Always use these macros instead of
 * any real integer.
 * (DF31 in EMV host Tools that deal with the results of  script )
 * PBOC Tag:    DF4X    (except DF41 in EMV HOST is force accept)
 * EC Tag:    DF5X    (except DF51 in EMV HOST is online PIN)
 * QPBOC Tag:    DF6X    (paywave)
 * Paypass Tag:DF7X    ()
 * Note:AMEX Express TAG
 */
#define ICC_PK_CLSS_TAG           0xDF58        /*IC card public key tag 0xDF4B defined by paypass*/


#define READER_CLSS_TRANS_LIMIT 0xDF57 /*Reader Contactless Transaction Limit*/

#define QPBOC_EXIST_TAG         0xDF63    /*QPBOC floot limit exist tag*/
#define PW2_9F35                0xDF64    /*Domestic Transactions Only*/
#define QPBOC_ICCERR            0xDF65    /*read card fail F1 readcard fail
                                                         F2 card unrecoverable error(stop card)*/
#define PW2_CONFIG              0xDF66    /*paywave config*/                                                             
#define READCARD_RESULT         0xDF67    /*whether finish readcard  0x01 read card fail*/
#define PW_TRACK1               0xDF68    /*paywave Track1 data*/
#define PW_TERMINAL_ENTRY_CAP 0xDF69  /*paywave terminal entry capability*/

#define PP_TRACK1_DD            0xDF71    /*paypass discretionary data of the track1 data returned by card */
#define PP_TRACK2_DD            0xDF72    /*paypass discretionary data of the track2 data returned by card */
#define PP_NUN                  0xDF73    /*paypass N.un = K.track2 - T.track2 (unsigend char 1bytes)*/
#define DEF_UDOL_TAG            0xDF74    /*paypass unknow data object list (UDOL)TAG*/
#define UDOL_BUF_TAG            0xDF75    /*paypass UDOL buf store */
#define PP_MAG_INDICATOR        0xDF76    /*paypass mag indicator 1bytes*/
#define PP_TRANS_CVM            0xDF77    /*paypass transaction CVM 3bytes*/
#define PP_READCARD_RESULT      0xDF78    /*paypass transaction read card result 1byte .the last command if return sucsses decide how to light or not LED and beep*/



/*    
  *Terminal transaction attribute 9F66 (emvparam._trans_prop)
      ==== Paypass only temporary support Mag Stripe (equivalent domestic MSD) and M / CHIP (domestic QPBOC equivalent)====
  */
#define EMV_PROP_VSDCCLSS       0x0040              /*1:clss VSDC supported 0:clss VSDC not supported*/
#define EMV_PROP_QVSDC          0x0020              /*1:support QVSDC*/
#define EMV_PROP_VSDC           0x0010              /*1:support contact VSDC*/

/*byte1 bit1 reserve */

/*    RFU bit be set 0 */
#define EMV_PROP_OFFLINEPIN         0x0120        /*1:support Contact offline PIN(VCPS Contact chip)*/

#define EMV_PROP_IUP                0x0280        /*1:support Issuer Update Processing*/
#define EMV_PROP_CDCVM              0x0240        /*1:support Consumer Device CVM ( mobile )
                                                    VCPS should support*/
/*other bits reserve */



/*PCII: POS Cardholder Interaction Information DF4B Byte3
Byte1:Version Number**/
#define PCII_OFFLINE_PIN_VER_SUCC       0x0110
#define PCII_CONTEXT_IS_CONFLICT        0x0108
#define PCII_OFFLINE_CHANGE_PIN_REQ     0x0104
#define PCII_ACK_REQ                    0x0102
#define PCII_PIN_REQ                    0x0101

#define pcii_info(nr, usTransProp)        ics_opt_get(nr, usTransProp)


/*Entry Point Execution order from the beginning of a step to the end*/
typedef enum {
    START_A,            //pre_processing
    START_B,            //protocol activation
    START_PPSE_SEL,
    START_C,            //select the combination
    START_FINAL_APP,
    START_D,            //kernel activation
    START_OUTCOME,      //outcome processing
    START_END = 0xFF    /*Deal with the end of the Entry Point does not require*/
}entry_point_seq;

/*contactless active card fail error value*/
#define EMV_TRANS_INTERCARD             -16            /*International Card, please swipe*/
#define EMV_TRANS_DOWNCARD              -15            /*powerdown card GPO return 6986*/
#define EMV_TRANS_TRYOTHER              -14            /*trun to other interface transaction*/

/*  ******** emv_opt.nTransRet && emv_opt.nOnlineResult  *********/

/*  ******** RF Transaction Return value  and emv_opt._trans_ret ******** */
#define EMV_TRANS_RF_TERMINATE         -1        /*Transaction terminate*/
#define EMV_TRANS_RF_MCHIP_ACCEPT      11        /*RF M/CHIP transaction succ */
#define EMV_TRANS_RF_MCHIP_DENIAL      12        /*RF M/CHIP transaction denial */
#define EMV_TRANS_RF_MCHIP_GOONLINE    13        /*RF M/CHIP transaction go online */
#define EMV_TRANS_RF_MAG_GOONLINE      14        /*RF Mag Stripe transaction go online  */
#define EMV_TRANS_RF_MAG_ACCEPT        15        /*RF Mag Stripe transaction aproved (refund transaction)  */

#define EMV_TRANS_RF_ACTIVE_CARD       16        /*RF active card*/
#define EMV_TRANS_RF_TRYOTHERINT       17        /*RF try another interface*/
#define EMV_TRANS_RF_ACTIVE_KERNEL     18        /**/
#define EMV_TRANS_RF_SELECT_NEXT_AID   19        /*exit kernel and select next aid*/
#define EMV_TRANS_RF_MAG_DENIAL        20
/*expresspay*/
#define EMV_TRANS_RF_CONTACTLESS_NOTPER 21


/*  ******** Entry Point Transaction Type 
Now: paypass paywave expresspay use this four transaction type
            emv_opt._trans_type(Request the type of transaction)     ******** */
#define EMV_TRANS_EP_PURCHASE             0x00      /*Entry Point good servic*/
#define EMV_TRANS_EP_CASH_ADVANCE         0x01      /*Entry Point cash*/
#define EMV_TRANS_EP_PURCHASE_CASHBACK    0x09      /*Entry Point */
#define EMV_TRANS_EP_REFUND               0x20      /*Entry Point refund*/
#define EMV_TRANS_EP_MANUAL_CASH          0x12      /*manual cash*/

/*    define emv_opt._request_amt :when input amount*/


/*
entry_point_opt defined transaction processing flag variables, such as the steps of the transaction execution, and display the information variable outcome paramter
TRF_transData defined transaction related data, such as the amount of pretreatment flag group variable pre-processing indicators
*/
typedef struct {
    unsigned char       _trans_type;    /* in, transaction type, see above */
    emv_seq     _seq_to;                /* in,kernel control when to terminate the session */
    entry_point_seq     _seq_start;     /* in, Entry Point control start from the process  */
    int                 _request_amt;   /* in, whether to request the the amount*/
    unsigned char       _card_no;       /*in, which card type for transaction,RF use EMV_CARD_RFIC= 0xA1*/
    unsigned char       _restart;       /*in, after online sucess,restart again*/
    /* if the terminal ICS support the below 3 options */
    int      _force_online_enable;      /* in, whether the force online option opened */
    int      _account_type_enable;      /* in, whether the account type selection opened */

    unsigned char *  _online_pin;       /* out, string with '\0' if online pin is entered */
    
    unsigned char *  _iss_script_res;   /* out, if issuer script result exists */
    int      _iss_sres_len;
    int      _advice_req;               /* out, if advice is required (must be supported by ics) */
    int      _force_accept_supported;   /* out, if ICS support it */
    int      _signature_req;            /* out, if the CVM finally request a signature */

    unsigned char *  _auth_resp_code;   /* in, 8A from the host */
    unsigned char *  _field55;          /* in, field55 or tlv decoded data from the host  */
    int      _field55_len;
    int      _online_result;            /* in, the online result */
    int      _trans_ret;                /* transaction return */

    /*********************Entry Point Data***************************/
    /*****User Interface Request Data --DF8116 Len=23*****/
    unsigned char _UI_message_id;          /*UI Request on Outcome Present*/
    unsigned char _UI_status;              /*UI Request on Outcome Present*/
    unsigned char _UI_hold_time[3];
    unsigned char _UI_language_len;//Ã”Ã¶Â¼Ã“Ã’Â»Â¸Ã¶Ã“Ã¯Ã‘Ã”ÂµÃ„Â³Â¤Â¶ÃˆÂ£Â¬2-8
    unsigned char _UI_language_preference[8];
    unsigned char _UI_value_qualifier;
    unsigned char _UI_value[6];
    unsigned char _UI_currency_code[2];

    /*Outcome Parameter Set --DF8129 maybe can use op_set[8]*/
    unsigned char _OP_status;
    unsigned char _OP_start;               /*Tag Entry Ponint began the position*/
    unsigned char _OP_online_response_data;/*Online response data*/
    unsigned char _OP_cvm;                 /*CVM authentication method*/

    unsigned char _OP_ui_request_on_outcome_present;   /*Request OUTCOME tag exists*/
    unsigned char _OP_ui_request_on_restart_present;   /*Online mark whether the request to re-swipe*/
    unsigned char _OP_data_record_present;             /*Authorization request or liquidation data exist*/
    unsigned char _OP_discretionary_data_present;      /*Custom data, such as the balance of the cardholder's name*/
    unsigned char _OP_receipt;                         /*signatrue 0:N/A 1:YES*/

    unsigned char _OP_alternate_interface_preference;  /*try another interface Contact chip or Mag-stripe*/
    unsigned char _OP_field_off_request;               /*Antenna Close does not allow credit card*/
    unsigned char _OP_removal_timeout;                 /*(Or shift card wait time) online timeout, timeout and allowing the shift card and re-under a deal*/

    /*****'FF8106'  10  Discretionary Data   
                        'DF8115'  6  Error Indication  *****/
    unsigned char _ER_L1_indication; //_OP_discretionary_data_present
    unsigned char _ER_L2_indication;
    unsigned char _ER_L3_indication;
    unsigned char _ER_SW1;
    unsigned char _ER_SW2;
    unsigned char _ER_MSG_ON_ERROR;//see msg ID in A.1.177

    unsigned char _refund_request_aac;//paywave for refund transaction request AAC . TC-0X40 ARQC-0X80 AC-OTHER 
    unsigned char _ctrl;
   	unsigned char _rfu[5];
}STENTRYPOINTOPT,entry_point_opt;

/*If no Message Hold Time is provided, then a 
default value of 000013 is used by Process S. */
#define MESSAGE_HOLD_TIME   "\x00\x00\x13"  //units of 100ms
#define HOLD_TIME_VALUE     0x0D
/**A.1.110  Outcome Parameter Set  Tag:  'DF8129' Length:  8 **/
/*Byte 1 bit8-5 Status*/
#define OP_STATUS_APPROVED                  0x10
#define OP_STATUS_DECLINED                  0x20
#define OP_STATUS_ONLINE_REQUEST            0x30
#define OP_STATUS_END_APPLICATION           0x40
#define OP_STATUS_SELECT_NEXT               0x50
#define OP_STATUS_TRY_ANOTHER_INTERFACE     0x60
#define OP_STATUS_TRY_AGAIN                 0x70
#define OP_STATUS_NA                        0xF0
/*Byte 2 bit8-5 Start*/
#define OP_START_A                          0x00
#define OP_START_B                          0x10
#define OP_START_C                          0x20
#define OP_START_D                          0x30
#define OP_START_NA                         0xF0
/*Byte 3 bit8-5 Online Response Data*/
#define OP_ONLINE_RESPONSE_DATA_NA          0xF0
/*Byte 4 bit8-5 CVM*/
#define OP_NO_CVM                           0x00
#define OP_OBTAIN_SIGNATURE                 0x10
#define OP_ONLINE_PIN                       0x20
#define OP_CONFIRMATION_CODE_VERIFIED       0x30
#define OP_CVM_NA                           0xF0
/*Byte 5 bit8-4 */
#define OP_UI_REQUEST_ON_OUTCOME_PRESENT    0x80 //Mark User Interface Request Data 
#define OP_UI_REQUEST_ON_RESTART_PRESENT    0x40
#define OP_DATA_RECORD_PRSENT               0x20
#define OP_DISCRETIONARY_DATA_PRESENT       0x10
#define OP_RECEIPT                          0x40 // 0:n/a ; 1:YES
/*Byte 6 bit8-5 Alternate Interface Preference*/
#define OP_ALTERNATE_INTERFACE_PREFERENCE   0xF0
/*Byte 7 bit8-1 Field Off Request*/
#define OP_FIELD_OFF_REQUEST                0xFF // FF:N/A ; Other value:Hold time in units of 100ms
/*Byte 8 bit8-1 Removal Timeout*/
#define OP_REMOVAL_TIMEOUT                  0xFF



/**A.1.177  User Interface Request Data 
Tag:  'DF8116' Length:  22 
To application control LCD LED Beep behavior according to the corresponding parameter values**/
typedef struct {
    unsigned char message_id;       //see below
    unsigned char status;           //see below
    unsigned char hold_time[3];     //n 6
    unsigned char language_preference[8]; //an
    unsigned char value_qualifier;
    unsigned char value[6];         //n 12
    unsigned char currency_code[2]; //n 3
    unsigned char resv[6];
}UI_request_data;

/* AID STCANDIDATE list structure, used for STCANDIDATE list selection  */
typedef struct
{
	unsigned char usAid[16];                 /*AID : application id*/
	unsigned char ucAidLen;                 /*length of AID*/
	unsigned char usLable[20];               /* application lable
                                            50(ICC), ans, 1-16 bytes, we prepare 20 for some PBOC2 errors*/
	unsigned char ucLableLen;               /*length of lable*/
	unsigned char usPreferredName[20];      /* Application Preferred Name 9F12(ICC), ans, 1-16 bytes */
	unsigned char ucPreferredNameLen;      /*length of preferred name*/
	unsigned char ucPriority;                /* Application Priority Indicator 87(ICC), b, 1 bytes */
	unsigned char  ucEnable;                 /* indicate whether the STCANDIDATE is enabled
                                            0    disable
                                            1    enable*/
	//add for Entry Point
	unsigned char ucLimitFlag;              /* contactless limit amount set flag*/
	unsigned char usKernelId[8];            /*Entry Point new add*/
	unsigned char usExtendAid[16];          /*Entry Point new add*/
	unsigned char ucExtendAidLen;          /* 0: does not support extended AID name,> 0:00 that the length */
	unsigned char ucStatusType;             /* Mark the type of the AID support VISA MASTERCARD JCB the PBOC equal to para.ucStatus value */
	unsigned char ucTerminalAidNum;    /*Terminal AID number of Select AID*/
	unsigned char ucTerminalPriority; // 0 to 255; 255 is the greatest priority
	unsigned char uc9F2Aexist;//9F2A exist or not
	unsigned char ucKernelConfig;
	unsigned char usResv[4];             /* usReserve bytes */
	unsigned int  nFileOffset;         /* the offset of this AID    in the parameters file */
}ep_candidate;


/*********EMV Operation of function pointers passed 
(See EMV API functin manual.doc)********/
typedef struct {
    int (*emv_get_transamt)(unsigned char transtype, unsigned long long * cash, unsigned long long * cashback);

    /*    -1    input fail    error
        -2    no input        BYPASS
        -3    BCTC Test: Timeout
        >0    input PIN length*/
    int (*emv_get_pinentry)(int type, char * pinentry);

    int (*iss_ref)(unsigned char * pan, int panlen);
    int (*acctype_sel)(void);
    int (*inc_tsc)(void);
    int (*cert_confirm)(unsigned char type, unsigned char * pcon, int len);
    int (*lcd_msg)(char * title, unsigned char * msg, int len,  int yesno, int waittime);
    int (*outcome_msg)(UI_request_data msg); //not yet applied
    int (*candidate_sel)(ep_candidate * pcan, int amt, int times);

    int (*emv_asc_2_bcd)(unsigned char * ascstr, int asclen, unsigned char * bcdstr, int align);
    int (*emv_bcd_2_asc)(unsigned char * bcd, int asclen, unsigned char * asc, int align);
    unsigned int (*emv_c4_2_uint)(unsigned char * c4);
    void (*emv_uint_2_c4)(unsigned int num, unsigned char * c4);
    unsigned long long (*bcd_2_uint64)( unsigned char  *bcd,  int bcd_len );
    
    int (*emv_icc_rw)(  int cardno, unsigned char *inbuf, int inlen, unsigned char *outbuf, int olen);

    /*EMV RFcard IC card powen off function*/
    int (*emv_icc_powerdown)( int cardno );
    
    /*    Func: RFcard    IC Card power on function
    retrun      0   Success
                -1  Fail
                -2  Conflict*/
    int(*emv_icc_powerup)( int *cardno ); 

    /*    Func:    When terminal & card both support E-cash
        *    Return:            1        E-CASH
        *                       0        NO E-CASH
        *                      -1        USER QUIT
        *                      -3        TIMEOUT
        */
    int(*emv_ec_switch)( void );
    int(*send_msg)(STENTRYPOINTOPT opt);
  	int(*dek_det)(unsigned char ucMesType, unsigned char * pusBufOut, int *pnBufOutLen);
   /*/*outcome(message_id,status) - Use pointer?controlled by app.
      Data Exchange recall_back().*/
}STEPOPER;


typedef struct {
    uchar usTacDefault[5];
    uchar usTacDenial[5];
    uchar usTacOnline[5];
    uchar ucTargetPercent;
    uchar ucMaxTargetPercent;
    uchar usThresholdValue[4];
    uchar usTransRefConv[4];
    uchar ucScriptDevLimit;
    
    uchar usIcs[7];
    uchar ucStatus;          /*Test type indicator*/
    uchar ucEcIndicator;    /*E-cash indicator use terminal value, not AID*/
    /****The above are AID settings****/

    uchar ucTransMod;    /*to save trade method and type
                            bit 8    1        Contactless
                                     0        Contact
                            bit 7    1        active
                                     0        deactive
                            bit 6    RFU for American Express 
                            bit 5    PayWave( QVSDC & MSD)
                            bit 4    PayPass (M/Chip & Mag Stripe)
                            bit 3    QPBOC/MSD transaction
                            bit 2    EC transaction
                            bit 1    PBOC transaction*/
    uchar ucCardNo;        /*Actived card number*/    
    uchar ucTransBranch;   /*22 field:
                             paypass:  1:M/CHIP(07)  2:Mag Strip(91)
                             micropayment:  1:QPBOC (071) 2:MSD (92[provisional])*/
    unsigned char ucDataExchangeSupport;   /*support data exchang or not. bit1:DE,bit2:SDS,bit3:IDS*/
    //unsigned char _on_device_cv_support;    /*On device cardholder verification supported*/
    unsigned char ucKernelConfig; /*substitute for _on_device_cv_support*/
    unsigned char ucMaxNumTornLog;            /* Max Number of Torn Transaction Log Records !=0 support Torn*/
    unsigned char ucBalanceReadFlag; // Instead of using Tag value as reference, to get balance, use this flag for justification
    unsigned char ucDrlStatus; /*0x00 Deactivated; 0x01 Activated*/
    uchar usReserve[17];     /*4 bytes align*/
}STCLSSCONFIG,emvclssconfig;


/***Paypass define Message Identifier *****/
#define UI_MSGID_APPROVED                   0x03
#define UI_MSGID_DECLINED                   0x07
#define UI_MSGID_PLEASE_ENTER_YOURE_PIN     0x09
#define UI_MSGID_PROCESSING_ERROR           0x0F
#define UI_MSGID_REMOVE_CARD                0x10
#define UI_MSGID_TRY_AGAIN                  0x11
#define UI_MSGID_WELCOME                    0x14
#define UI_MSGID_PRESENT_CARD               0x15
#define UI_MSGID_PROCESSING                 0x16
#define UI_MSGID_CARD_READ_OK               0x17
#define UI_MSGID_INSERT_OR_SWIPE_CARD       0x18
#define UI_MSGID_PRESENT_ONE_CARD_ONLY      0x19
#define UI_MSGID_APPROVED_SIGN              0x1A 
#define UI_MSGID_AUTHORISING_PLEASE_WAIT    0x1B 
#define UI_MSGID_ERROR_OTHER_CARD           0x1C
#define UI_MSGID_INSERT_CARD                0x1D
#define UI_MSGID_CLEAR_DISPLAY              0x1E
#define UI_MSGID_SEE_PHONE                  0x20
#define UI_MSGID_PRESENT_CARD_AGAIN         0x21 //try again
#define UI_MSGID_NA                         0xFF
/***Paypass define Status  *****/
#define UI_STATUS_NOT_READY                 0x00
#define UI_STATUS_IDLE                      0x01
#define UI_STATUS_READY_TO_READ             0x02
#define UI_STATUS_PROCESSING                0x03
#define UI_STATUS_CARD_READ_SUCCESSFULLY    0x04
#define UI_STATUS_PROCESSING_ERROR          0x05
#define UI_STATUS_STATUS_NA                 0xFF
/***Paypass define Value Qualifier   *****/
#define UI_VALUE_QUALIFIER_NONE             0x00
#define UI_VALUE_QUALIFIER_AMOUNT           0x10
#define UI_VALUE_QUALIFIER_BALANCE          0x20

/***A.1.68 Error Indication   'DF8115'  6 ***/
#define ER_L1_OK                        0x00
#define ER_L1_TIMEOUT                   0x01
#define ER_L1_TRANSMISSION              0x02
#define ER_L1_PROTOCOL                  0x03

#define ER_L2_OK                        0x00
#define ER_L2_CARD_DATA_MISSING         0x01
#define ER_L2_CAM_FAILD                 0x02
#define ER_L2_STATUS_BYTES              0x03
#define ER_L2_PARSING_ERROR             0x04
#define ER_L2_MAX_LIMIT_EXCEEDED        0x05
#define ER_L2_CARD_DATA_ERROR           0x06
#define ER_L2_MAGSTRIPE_NOT_SUPPORTED   0x07
#define ER_L2_NO_PPSE                   0x08
#define ER_L2_PPSE_FAULT                0x09
#define ER_L2_EMPTY_CANDIDATE_LIST      0x0A
#define ER_L2_IDS_READ_ERROR            0x0B
#define ER_L2_IDS_WRITE_ERROR           0x0C
#define ER_L2_IDS_DATA_ERROR            0x0D
#define ER_L2_IDS_NO_MATCHING_AC        0x0E
#define ER_L2_TERMINAL_DATA_ERROR       0x0F

#define ER_L3_OK                        0x00
#define ER_L3_TIME_OUT                  0x01
#define ER_L3_STOP                      0x02
#define ER_L3_AMOUNT_NOT_PRESENT        0x03



/*    Define cardno while read or write IC card */
#define EMV_CARD_RFIC               0xa1        /*contactless IC card*/

/// Callback function definitions for SDK_EP_SetCallbackFunction()
#define SDK_EP_CALLBACK_SM2_VERIFY		0x06   ///<  EP_AlgSM2Verify
#define SDK_EP_CALLBACK_SM2_GENE		0x07   ///<  EP_AlgSM2GenE
#define SDK_EP_CALLBACK_SM3_COMPUTE		0x08   ///<  EP_AlgSM3Compute
#define SDK_EP_CALLBACK_DEBUG			0x09   ///<  EP_Debug
#define SDK_EP_CALLBACK_PRINT_APDU		0xFF   ///<  EP_PrintAPDU,used for paypass Test

#define LV_CLOSE 0x00  //close debug
#define LV_DEBUG 0x01  //log with normal debug informaiton, normal/recommand use this value;
#define LV_ALL   0x03  //log with all of the debug information

/// End of Callback function definitions
/* Func:    initialize the EMV file path,
  *         introduce EMV-related file names  and operation of function pointers,
  *         Load AID lists and save g_emvconfig the default settings
  * Para:       *pfile       EMV file structure operation,in
  *             * Poper      EMV related operations function pointer,in
  * Return:     0   Success
  *             < 0 failure
  */
extern int SDK_Entry_Point_Initialize(emv_file * pfile, STEPOPER * poper);



/*    *****************************        Other operating functions **************************/
/*    return EMV version buffer*/
extern const char * SDK_EP_GetVersion(void);
extern const char * SDK_PP_GetVersion(void);
extern const char * SDK_PW_GetVersion(void);
extern const char * SDK_XP_GetVersion(void);
extern const char * SDK_Qpboc_GetVersion(void);
/*    return EMV error code*/
extern int SDK_EP_ErrorCode(void);

/*    return CLL2 Status buffer[40]*/
extern unsigned char *  SDK_EP_Get_CLL2_Status(void);


/* Func: Get  a series of TLV specifies data in tagname[]
        the data format of the returned data tag + len + value
   * Para:  tagname     to save the data to obtain the label TLV array pointer to the first
   *        count       the number of data to obtain the TLV
   *        obuf        save the data pointer for the TLV
            olen        obuf maximum array space constraints
   * Return:       -2   parameter is empty
                   -1   Failure
                   n    (n> 0) total length of the data obtained   
*/
extern int SDK_EP_FetchData(unsigned int * tagname, int count, unsigned char * obuf, int olen);


/* Func: request tagname value
* Para: 
        tagname     TAG name, i.e: 0x9F36
        *data       character pointer, the head pointer storing data, which can not be NULL
        dataLimit   data stored in the maximum length
* Return:   0       label value does not exist
            > 0     the length of the data obtained
            -1      Long data value exceeds the length limit data
*/
extern int SDK_EP_GetData( unsigned int tagname, unsigned char *data, int dataLimit );

/** Func: save tagname data values
  * Para: tagname you want to save the label name, such as 0x9F36
  The ** data character pointer, the head pointer stored data, the value can not be NULL
  * Datalen data storage length
  * Return: 0 tag value does not exist
  *         > 0 taken to the length of the data
  *         -1 Data value long beyond the data length limit
  */
extern int SDK_EP_SetData (unsigned int tagname,unsigned char *data, int datalen);





/* Func:    parse TLV structure string to tlv_t * pobj .
            (pobj [0] record does not store information about the TLV parsing out of the TLV object
            actual storage from pobj [1] start)

                pobj[0].pvalue      stored TLV string pointer 
                pobj[0].valuelen    save TLV length of the string
                pobj[0].childnum    save the total number of TLV nodes which is parsed

   * Para: 
            *ptlvstr    the first pointer to the string passed TLV
            *tlvlen     TLV length of the string
   *        *Pobj       store parsed data structure good TLV array pointer
   *        objspace    stored TLV limit the maximum size of pobj
   *        deflag      SINGLE_TLVOBJ   TLV data object string is a single structure
   *                    STRING_TLVOBJ   TLV object is a set of strings (tag + len + value + tag + len + value ...)
                        DECODE_LEVEL1   only child node TLV parsing out the string
                                        (If the child node for the structure of data objects will not continue to parse its child nodes)
   * Return:    0       Parse successfully
   *            <0      Parse failed 
*/
extern int SDK_EP_ParseTLV(unsigned char * ptlvstr, int tlvlen, tlv_t * pobj, int objspace, int deflag);


/* Parameter level class */
//#define SEARCH_ALL_DESC     0x01    /* search all the descendants */
//#define SEARCH_ONLY_SON     0x02    /* search only its sons  */


/* Func:  in structure tlv_t pobj get tagname of TLV data (with int EMV_ParseTLV () function is used)
   * Para:  parent       from the parent node to start the search
   *        tagname      tag you want to find
   *        * pobj       structure to search for tlv_t
   *        level        search layer degree
                   SEARCH_ONLY_SON :only to find their own child nodes
                   SEARCH_ALL_DESC :look at all subsequent nodes
   * Return:    find success    return  node number of child
                find failed     return  0
   */
extern int SDK_EP_FetchTLV(int parent,  unsigned int tagname, tlv_t * pobj, int level);








/*  Func: operation keys, including update, delete, use, remove
    * Para:  *pk    incoming or outgoing public key pointers to structures
             mode   public key operation mode
    * Return:   0   Success
                <0  failure 
*/                     
extern int SDK_EP_OperCAPK(capk * pk, int mode);


/* Func: delete all CAPK a RID
   * Para: RID      RID string to be deleted
   * Return:    0   Success
                <0  failure */
extern int SDK_EP_RemoveCAPKByRID(char RID[5]);


/* FUNC: get the public key from start to end of RID and RID index
   *Para:   start         the first of several public-key start
            end           to end several public key
            strCAPK[][6]    for each array element  the first five characters to save RID, 
                            the first six characters to save  RID index
   * Return:        > 0         to obtain the public key to return the number of successful
                    Other       failure */
extern int SDK_EP_EnumCAPK(int start, int end, char strCAPK[][6]);



/* Func:        operate the terminal configuration and AID: access, delete, update, etc.
                (Note: the terminal configuration parameters in the file should be written in the first place,
                AID from the second parameters of position to write)
   * Para:  *par    incoming or outgoing AID structure pointer
            mode    operation mode
   * Return:    0   Success
                <0  failure */
extern int SDK_EP_OperAID(STCLSSPARAM * pstClssEPParam, int mode);


/* Func: get all the number of AID and AID.
   *Para:   strAID[][17] save for the AID values
                   (The first character of the array (ie. strAID[X][0]) to save AID length;
                   From the second character (ie. strAID[X][1]) start saving AID values
   *Return:     success     Number of AID
                Failure     <0 */
extern int SDK_EP_EnumAID(char strAID[][17]);




/* Func: Operation Certificate blacklist function
   * Para in:   *blk    save blacklist structure to operate
   *            mode    operation of a unified approach to see the above documents
   * Return:    0   Success
   *            <0  failure 
*/

extern int SDK_EP_Oper_CertBlk( certblk *blk, int mode );


/* Func: Operation Card blacklist function
   * Para in:   *blk    save blacklist structure to operate
   *            mode    operation of a unified approach to see the above documents
   * Return:    0   Success
   *            <0  failure 
*/
extern int SDK_EP_Oper_CardBlk( cardblk *blk, int mode );

/*    ****************************    RF card function    *****************************    */

/*************    RF transaction input RF structure *************/
typedef struct{
    unsigned long long _amount;         /*in,transamount*/
    unsigned long long _amountOther;    /*in,transamountOther*/
    unsigned char _date[7];             /*in,date: yyyy,mm,dd,hh,mm,ss*/

    unsigned char _aid[16];             /*out,*/
    unsigned char _aid_len;             /*out,*/
    unsigned char _kernel_id[8];        /*out,kernel id*/
    unsigned int  _file_offset;         /*out,kernel config file offset */

    unsigned char *_final_aid_fci;      /*out,FCI received from select aid final,Application must assign one buffer pointer*/
    unsigned int  _fci_len;             /*out,the length of FCI*/
    unsigned char _SW12[2];             /*out,SW1 SW2 received from select aid*/

    /*Entry_Point_PreProcess_indicators*/
    unsigned char *_pre_process_indicator;      /*out,Application must assign one T_epData buffer pointer*/
    unsigned char _pre_process_indicator_len;   /*out,length of T_epData*/

    unsigned char _no_amount; /*paypass test.Not save amount*/
    unsigned char _resv[4];        /*RFU*/
}TRF_transData;



/* Func: AID re-read the file, the establishment of AID List */
/* Func: re-read AID documents establish AID List
Core configuration file is modified, you need to call this function to re-establish the practice list
*/
extern int SDK_Entry_Point_BuildAIDList(void);


extern int SDK_Entry_Point_Process( entry_point_opt * popt ,TRF_transData *pdata );

/* Func: the end of the EMV transaction execution function to execute trades flag variable emptying operation.
  * Para: trans_final the final result of a transaction
  * Return 0 is successful
              -1 Failed
  */
extern int SDK_Entry_Point_Suspend(int trans_final);
extern int SDK_EP_ClearStatus( void );

/*PAYPASS 3.0.1 Use the following functions to do business*/
/* Func: contactless transaction execution functions.
    To call SDK_Entry_Point_Process() , contactclss pre-process.

    1: The function returns <0, then the transaction aborts
    (Be sure to call SDK_PayPass_Suspend () function to the end of the transaction and related data)
    
 * Para: * popt requires incoming EMV transaction options
        * pdata incoming transaction data (if the incoming fixed transaction payment
        The amount also set popt-> _request_amt = EMV_TRANS_REQAMT_NO)
      
      Do not pass a fixed amount but by the user within the function
      Enter the amount, transAmount filled in as 0, and set
      popt-> _request_amt = EMV_TRANS_REQAMT_RFPRECESS.
      
 * Return:  return value of the transaction results
      */
extern int SDK_PayPass_Process( entry_point_opt * popt ,TRF_transData *pdata ) ;
/* Func: paypass RF card transaction processing functions.
 * Para: trans_final the final result of a transaction (transactions accepted, transactions refuse ...)
            while trans_final ==0xff, don't powerdown rf card.
 * Return:  0 success
            -1 Failed
 */  
extern int SDK_PayPass_Suspend( int trans_final );

/*************** paywave transacion function****************/
/* Func: contactless transaction execution functions.
    To call SDK_Entry_Point_Process() , contactclss pre-process.

    1: The function returns <0, then the transaction aborts
    (Be sure to call SDK_PayWave_Suspend () function to the end of the transaction and related data)
    
 * Para: * popt requires incoming EMV transaction options
        * pdata incoming transaction data (if the incoming fixed transaction payment
        The amount also set popt-> _request_amt = EMV_TRANS_REQAMT_NO)
      
      Do not pass a fixed amount but by the user within the function
      Enter the amount, transAmount filled in as 0, and set
      popt-> _request_amt = EMV_TRANS_REQAMT_RFPRECESS.
      
 * Return:  return value of the transaction results
      */
extern int SDK_PayWave_Process( entry_point_opt * popt ,TRF_transData *pdata ) ;


/* Func: paypass RF card transaction processing functions.
 * Para: trans_final the final result of a transaction (transactions accepted, transactions refuse ...)
 * Return:  0 success
            -1 Failed
 */  
extern int SDK_PayWave_Suspend( int trans_final );

extern int SDK_ExpressPay_Process( entry_point_opt * popt ,TRF_transData *pdata ) ;
extern int SDK_ExpressPay_Suspend( int trans_final );


extern int SDK_Pure_Process( entry_point_opt * popt ,TRF_transData *pdata ) ;
extern int SDK_Pure_Suspend( int trans_final );
extern int SDK_Qpboc_Process( entry_point_opt * popt ,TRF_transData *pdata ) ;
extern int SDK_Qpboc_Suspend( int trans_final );

extern int SDK_EP_SetCallbackFunction(unsigned int callbackFuncId, void *callbackFunction);

/**
* @brief Set Kernel Debug Mode
* @param cDebuglv   LV_CLOSE / LV_DEBUG / LV_ALL
* @return
* @date 2018-9-30
*/
extern void SDK_EP_SetDebugMode(char cDebuglv);
#endif


