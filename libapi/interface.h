#ifndef __INTERFACE__
#define __INTERFACE__

#define   MAX_DDOL_LEN                        (252)     /**< DDOL Maximum length */
#define   MAX_TDOL_LEN                        (252)     /**< TDOL Maximum length */

/*
 * emvparam._status
 * The identification of network testing project categories
 * Prior to the implementation of the transaction, you must assign a label to test the transaction type
 * Default PBOC2_ENB in China, No special requirements set 0
 */
#define   BCTC_TEST_ENB                       (0x01)    /**< Bank card testing center of China */
#define   PBOC2_ENB                           (0x02)    /**< PBOC */
#define   VISA_ENB                            (0x04)    /**< visa */
#define   MASTERCARD_ENB                      (0x08)    /**< master card */
#define   JCB_ENB                             (0x10)    /**< jcb */
#define   AMEX_ENB                            (0x20)    /**< amex */
#define   BCTC_QUICS_ENB                      (0x40)    /**< QUICS */
#define   DISCOVER_ENB                        (0x80)    /**< discoverÈëÍø */
#define   MCCS_ENB                            (0x81)    /**< mccsÈëÍø */



/*=========================struct emv_opt =========================================*/
/*  emv_opt._seq_to (emv Transaction sequence number),That requires EMV to perform to where it ends.
    EMV_PROC_CONTINUE, means executing full EMV process*/
typedef enum
{
    EMV_PROC_TO_APPSEL_INIT,                              /**< Application selection initialization */
    EMV_PROC_TO_READAPPDATA,                              /**< Read application data */
    EMV_PROC_TO_OFFLINEAUTH,                              /**< Offline data authentication */
    EMV_PROC_TO_RESTRITCT,                                /**< Processing restrictions */
    EMV_PROC_TO_CV,                                       /**< Cardholder verification */
    EMV_PROC_TO_RISKMANA,                                 /**< Terminal risk management */
    EMV_PROC_TO_1GENAC,                                   /**< First GAC */
    EMV_PROC_TO_2GENAC,                                   /**< Second GAC */
    EMV_PROC_CONTINUE                                     /**< PBOC executing full EMV process */
} emv_seq;


/*
 * @struct STEMVPARAM EMV Terminal parameters
 * support = 1, not support = 0;no used set 0
 */
typedef struct
{
	unsigned char _tac_default[5];                /**< default Terminal Action Code */
	unsigned char _tac_denial[5];                 /**< denial Terminal Action Code */
	unsigned char _tac_online[5];                 /**< online Terminal Action Code */
	unsigned char _target_percent;                /**< target percent */
	unsigned char _max_target_percent;            /**< max target percent */
	unsigned char _threshold_value[4];            /**< thresold value */
	unsigned char _trans_ref_conv[4];             /**< transaction reference currency convert, default 0 */
	unsigned char _script_dev_limit;              /**< script length limit ,default 0 */
	unsigned char _ics[7];                        /**< ICS (Implementation Comformance Statement)Each bit represents a setting */
	unsigned char _status;                        /**< Test type indicator*/
	unsigned char _ec_indicator;                  /**< Electronic cash identification _EMV_TAG_9F7A_TM_SUPPEC */
	unsigned char _type;                          /**< Terminal type _EMV_TAG_9F35_TM_TERMTYPE */
	unsigned char _cap[3];                        /**< Terminal capability _EMV_TAG_9F33_TM_CAP */
	unsigned char _add_cap[5];                    /**< Additional terminal capability, _EMV_TAG_9F40_TM_CAP_AD */
	unsigned char _aid[16];                       /**< AID, 4F(ICC), 9F06(Terminal), b, 5-16 bytes */
	unsigned char _aid_len;                       /**< length of AID*/
	unsigned char _app_ver[2];                    /**< Terminal application version, _EMV_TAG_9F09_TM_APPVERNO */
	unsigned char _pos_entry;                     /**< POS access mode, _EMV_TAG_9F39_TM_POSENTMODE */
	unsigned char _floorlimit[4];                 /**< Terminal transaction limit, _EMV_TAG_9F1B_TM_FLOORLMT */
	unsigned char _acq_id[6];                     /**< acquiring bank, _EMV_TAG_9F01_TM_ACQID */
	unsigned char _mer_category_code[2];          /**< Terminal type code, _EMV_TAG_9F15_TM_MCHCATCODE */
	unsigned char _merchant_id[15];               /**< Merchant ID, _EMV_TAG_9F16_TM_MCHID */
	unsigned char _trans_curr_code[2];            /**< Transaction currency code, _EMV_TAG_5F2A_TM_CURCODE */
	unsigned char _trans_curr_exp;                /**< Trading currency index, _EMV_TAG_5F36_TM_CUREXP */
	unsigned char _trans_ref_curr_code[2];        /**< Transaction reference currency code, _EMV_TAG_9F3C_TM_REFCURCODE */
	unsigned char _trans_ref_curr_exp;            /**< Transaction reference currency index, _EMV_TAG_9F3D_TM_REFCUREXP */
	unsigned char _term_country_code[2];          /**< Terminal country code, _EMV_TAG_9F1A_TM_CNTRYCODE */
	unsigned char _ifd_serial_num[8];             /**< Interface device serial number, _EMV_TAG_9F1E_TM_IFDSN */
	unsigned char _terminal_id[8];                /**< Terminal ID, _EMV_TAG_9F1C_TM_TERMID*/
	unsigned char _default_ddol_len;              /**< the length of the default ddol following the merchant name and location */
	unsigned char _default_tdol_len;              /**< the length of the default todl following the default ddol */
	unsigned char _default_ddol[MAX_DDOL_LEN];    /**< default ddol */
	unsigned char _default_tdol[MAX_TDOL_LEN - 29];    /**< default tdol */
	unsigned char ucRiskManaDataLen;
	unsigned char usRiskManaData[8];   
	unsigned char _merchant_name[20];             /**< Merchant Name*/
	unsigned char _app_sel_indicator;             /**< application select indicator
                                                       when ics support and this byte is 0x01(!0)
                                                       application select support partial match */
	unsigned char _fallback_posentry;             /**< fallback pos entry */
	unsigned char _limit_exist;                   /**< limist exist?(Identify whether the following limits exist)
                                                       bit 1    =1    EC limint exsit
                                                       bit 2    =1    contactless limit exsit  
                                                       bit 3    =1    contactless offline limit exsit 
                                                       bit 4    =1    cvm limit  exsit  */
	unsigned char _ec_limit[6];                   /**< EC limint _EMV_TAG_9F7B_TM_EC_LMT_IN */
	unsigned char _cl_limit[6];                   /**< contactless limit n12  6bytes */
	unsigned char _cl_offline_limit[6];           /**< contactless offline limit n12  6bytes */
	unsigned char _cvm_limit[6];                  /**< cvm limit    n12  6bytes */
	unsigned char _trans_prop[4];                 /**< Terminal transaction property _EMV_TAG_9F66_TM_TRANSPTY_IN */
	unsigned char _status_check;                  /**< contactless state inspection Default 0*/
	unsigned char _appid;                         /**< APP ID */
	unsigned char _resv[2];                       /**< Reserve*/
} emvparam;

/**
* @Define emvparam._appid  Corresponding support application bit
*/
#define   EMV_APPID_PBOC               (0x20)     /**< AID support PBOC */
#define   EMV_APPID_UPCARD             (0x08)
#define   EMV_APPID_QPBOC              (0x04)
#define   EMV_APPID_MSD                (0x02)
#define   EMV_APPID_EC                 (0x01)

/**
* emvparam._limit_exist
* Used to determine whether the terminal parameters define the appropriate limits
* 1      exist
* 0      not exist
*/
#define   EC_LIMIT_EXIST(x)            ((x) & 0x01)     /**< EC limint exsits */
#define   CL_LIMIT_EXIST(x)            ((x) & 0x02)     /**< contactless limit exsits */
#define   CLOFFLINE_LIMIT_EXIST(x)     ((x) & 0x04)     /**< contactless offline limit exsits */
#define   CVM_LIMIT_EXIST(x)           ((x) & 0x08)     /**< cvm limit exsits */


/**
* @struct AID Candidate list
* @detail Select the candidate list to choose
*/
typedef struct
{
	unsigned char _aid[16];                  /**< AID */
	unsigned char _aid_len;                  /**< length of AID */
	unsigned char _lable[20];                /**< AID lable
                                                  50(ICC), ans, 1-16 bytes, we prepare 20 for some PBOC2 errors */
	unsigned char _lable_len;                /**< length of lable*/
	unsigned char _preferred_name[20];       /**< preferred name 9F12(ICC), ans, 1-16 bytes */
	unsigned char _preferred_name_len;       /**< length of preferred name */
	unsigned char _priority;                 /**< Application priority 87(ICC), b, 1 bytes */
	unsigned char  _enable;                  /**< indicate whether the candidate is enabled
                                                  0    disenabled
                                                  1    enabled */
	unsigned char _flag;                     /**< contactless limit set identification*/
	unsigned char _resv[2];                  /**< reserve bytes */
	int           _file_offset;              /**< the offset of this AID in the parameters file */
} candidate;

/**
 * Self-defined tags for internal usage.
 * These tag names are just defined according to
 * current hash value spread. So if the hash
 * function changes in emvbuf.c leading to
 * a different spread, these tag names may
 * be changed too.
 * Always use these macros instead of
 * any real integer.
 * (DF31 Issuing bank script execution results)
 * PBOC Tag:    DF4X    (except DF41 force accept)
 * EC Tag:  DF5X    (except DF51 online PIN )
 * QPBOC Tag:   DF6X    ()
 * Paypass Tag:DF7X ()
 */
#define   EMV_APPID_TAG                (0xDF42)         /**< AID._appid  */
#define   CURR_DATE_TAG                (0xDF43)         /**< Current transaction time 4 bytes:YYYYMMDD */
#define   DEF_DDOL_TAG                 (0xDF44)         /**< (DDOL) */
#define   DEF_TDOL_TAG                 (0xDF45)         /**< (TDOL) */
#define   PDOL_BUF_TAG                 (0xDF46)         /**< (PDOL) */
#define   CDOL1_BUF_TAG                (0xDF47)         /**< (CDOL1) */
#define   CDOL2_BUF_TAG                (0xDF48)         /**< (CDOL2) */
#define   AUTH_DATA                    (0xDF49)         /**< Static authentication data */
#define   ISSUER_PK_TAG                (0xDF4A)         /**< Issuer Public Key */
#define   ICC_PK_TAG                   (0xDF4B)         /**< IC Card Public Key */
#define   SCRIPT_REST_TAG              (0xDF4C)         /**< Issuing bank script execution results */
#define   ONLINE_PIN_TAG               (0xDF4D)         /**< online PIN */
#define   SCRIPT_REST_TAG2             (0xDF31)         /**< Issuing bank script execution results, Same to 0xDF4C */
#define   CARD_EXPIRE_DATE             (0xDF4F)         /**< Card expiration time(Track two data) */
#define   EC_SELECT_RESULT             (0xDF53)         /**< Select whether to execute EC results */
#define   EC_ONLINE_PIN_TAG            (0xDF54)         /**< EC online PIN(Below threshold) */
#define   CTTA_TAG                     (0xDF55)         /**< Cumulative total transaction amount */
#define   CTTAL_TAG                    (0xDF56)         /**< Cumulative total transaction limit */
#define   ONLINE_PIN_REQUEST           (0xDF57)         /**< online PIN in Terminal capability */
#define   NL_TAG_PAN_SFI                    (0xDF70)         /**<5A: The value of SFI */

/**<Self-defined tags for internal usage*/
#define _EMV_TAG_NL_0001_DEFAULT_DDOL  0x0001
#define _EMV_TAG_NL_0002_DEFAULT_TDOL  0x0002
#define _EMV_TAG_NL_0003_DEFAULT_DDOL  0x0003
#define _EMV_TAG_NL_0004_TAC_DEFAULT  0x0004
#define _EMV_TAG_NL_0005_TAC_DENIAL  0x0005
#define _EMV_TAG_NL_0006_TAC_ONLINE  0x0006
#define _EMV_TAG_NL_0007_TARGET_PERCENT  0x0007
#define _EMV_TAG_NL_0008_MAX_TARGET_PERCENT  0x0008
#define _EMV_TAG_NL_0009_THRESHOLD_VALUE  0x0009
#define _EMV_TAG_NL_000A_TRANS_REF_CONV  0x000A
#define _EMV_TAG_NL_000B_ICS  0x000B
#define _EMV_TAG_NL_000C_TEST_TYPE_INDICATOR 0x000C
#define _EMV_TAG_NL_000D_TRANS_MODE 0x000D
#define _EMV_TAG_NL_000E_TRANS_BRANCH 0x000E
#define _EMV_TAG_NL_000F_EMV_CODE 0x000F
#define _EMV_TAG_NL_0010_CARD_TYPE 0x0010
#define _EMV_TAG_NL_0011_TAG_DUPLICATE 0x0011
#define _EMV_TAG_NL_0012_SM_SUPPORT 0x0012
#define _EMV_TAG_NL_0013_EC_SUPPORT 0x0013
#define _EMV_TAG_NL_0014_CURRENTTIME 0x0014
#define _EMV_TAG_NL_0015_RAND 0x0015



/**********************************emvparam._ics   ICS ************************************************/
/*
 * AS   : Application Selection
 * Macro:
   AS_Support_PSE                 : Support PSE selection method
   AS_Support_CardHolder_Confirm  : Support Cardholder confirmation
   AS_Support_Prefferd_Order      : Have a preferred order of displaying applications
   AS_Support_Partial_AID         : Does the terminal perform partial AID selection
   AS_Support_Multi_Language      : Does the terminal have multi language support
   AS_Support_Common_Charset      : Does the terminal support Common Character Set as
                                    defined in "Annex B table 20 Book 4"

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define AS_Support_PSE                     (0x0080)
#define AS_Support_CardHolder_Confirm      (0x0040)
#define AS_Support_Preferred_Order         (0x0020)
#define AS_Support_Partial_AID             (0x0010)
#define AS_Support_Multi_Language          (0x0008)
#define AS_Support_Common_Charset          (0x0004)

/*
 * DA   : Data Authentication
 * IPKC : Issuer Public Key Certificate
 * CAPK : Certification Authority Public Key
 * Macro:
     DA_Support_IPKC_Revoc_Check      : During DA, does the terminal check the revocation of IPKC
     DA_Support_Default_DDOL          : Does the terminal contain a default DDOL
     DA_Support_CAPKLoad_Fail_Action  : Is operation action required when loading CAPK fails
     DA_Support_CAPK_Checksum         : Is CAPK verified with CAPK checksum

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define DA_Support_IPKC_Revoc_Check        (0x0180)
#define DA_Support_Default_DDOL            (0x0140)
#define DA_Support_CAPKLoad_Fail_Action    (0x0120)
#define DA_Support_CAPK_Checksum           (0x0110)


/**
 * CV   : Cardholder Verification
 * CVM   : Cardholder Verification Methods
 * Macro:
 	 CV_Support_Bypass_PIN            : Terminal supports bypass PIN entry
	 CV_Support_Subsequent_Bypass_PIN : Terminal supports subsequent bypass PIN entry
 	 CV_Support_PIN_Try_Counter       : Terminal supports Get Data for PIN Try Counter
 	 CV_Support_Fail_CVM              : Terminal supports Fail CVM
 	 CV_Support_Amounts_before_CVM    : Are amounts known before CVM processing

 * EMV 4.3 ICS Version 4.3c Level2
 */
#define CV_Support_Bypass_PIN              (0x0280)
#define CV_Support_PIN_Try_Counter         (0x0240)
#define CV_Support_Fail_CVM                (0x0220)
#define CV_Support_Amounts_before_CVM      (0x0210)
#define CV_Support_Bypass_ALL_PIN          (0x0208)

/*
 * TRM  : Terminal Risk Management
 * Macro:
   TRM_Support_FloorLimit     : Floor Limit Checking,
                                Mandatory for terminal with offline capability
   TRM_Support_RandomSelect   : Random Transaction Selections,
                                Mandatory for offline terminal with online capability,
                                except when cardholder controlled
   TRM_Support_VelocityCheck  : Velocity checking,
                                Mandatory for for terminal with offline capability
   TRM_Support_TransLog       : Support transaction log
   TRM_Support_ExceptionFile  : Support exception file
   TRM_Support_AIPBased       : Performance of TRM based on AIP setting
   TRM_Use_EMV_LogPolicy      : EMV has a different log policy with PBOC2, marked here

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define TRM_Support_FloorLimit            (0x0380)
#define TRM_Support_RandomSelect          (0x0340)
#define TRM_Support_VelocityCheck         (0x0320)
#define TRM_Support_TransLog              (0x0310)
#define TRM_Support_ExceptionFile         (0x0308)
#define TRM_Support_AIPBased              (0x0304)
#define TRM_Use_EMV_LogPolicy             (0x0302)

/*
 * TAA  : Terminal Action Analysis
 * (x)  : the var of struct STEMVCONFIG
 * TAC  : Terminal Action Codes
 * DAC  : Default Action Codes
 * Macro:
   TAA_Support_TAC                  : Does the terminal support Terminal Action Codes
   TAA_Support_DAC_before_1GenAC    : Does the terminal process DAC prior to first GenAC
   TAA_Support_DAC_after_1GenAC     : Does the terminal process DAC after first GenAC
   TAA_Support_Skip_DAC_OnlineFail  : Does the terminal skip DAC processing and automatically
                                      request an AAC when unable to go online
   TAA_Support_DAC_OnlineFail       : Does the terminal process DAC as normal
                                      when unable to go online
   TAA_Support_CDAFail_Detected     : Device capable of detecting CDA Failure before TAA
   TAA_Support_CDA_Always_in_ARQC   : CDA always requested in a first Gen AC, ARQC request
   TAA_Support_CDA_Never_in_ARQC    : CDA never requested in a first Gen AC, ARQC request
   TAA_Support_CDA_Alawys_in_2TC    : CDA always requested in a second Gen AC when successful
                                      host response is received, with TC request
   TAA_Support_CDA_Never_in_2TC     : CDA never requested in a second Gen AC when successful
                                      host response is received, with TC request
 * EMV 4.1 ICS Version 3.9 Level2
 */
#define TAA_Support_TAC                    (0x0480)
#define TAA_Support_DAC_before_1GenAC      (0x0440)
#define TAA_Support_DAC_after_1GenAC       (0x0420)
#define TAA_Support_Skip_DAC_OnlineFail    (0x0410)
#define TAA_Support_DAC_OnlineFail         (0x0408)
#define TAA_Support_CDAFail_Detected       (0x0404)
#define TAA_Support_CDA_Always_in_ARQC     (0x0402)
#define TAA_Support_CDA_Alawys_in_2TC      (0x0401)

/*
 * CP  : Completion Process
 * (x)  : the var of struct STEMVCONFIG
 * Macro:
   CP_Support_Force_Online         : Transaction forced Online capability
   CP_Support_Force_Accept         : Transaction forced Acceptance capability
   CP_Support_Advices              : Does the terminal support advices
   CP_Support_Issuer_VoiceRef      : Does the terminal support Issuer Initiated Voice Referrals
   CP_Support_Batch_Data_Capture   : Does the terminal support Batch Data Capture
   CP_Support_Online_Data_capture  : Does the terminal support Online Data Capture
   CP_Support_Default_TDOL         : Does the terminal support a default TDOL

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define CP_Support_Force_Online            (0x0580)
#define CP_Support_Force_Accept            (0x0540)
#define CP_Support_Advices                 (0x0520)
#define CP_Support_Issuer_VoiceRef         (0x0510)
#define CP_Support_Batch_Data_Capture      (0x0508)
#define CP_Support_Online_Data_capture     (0x0504)
#define CP_Support_Default_TDOL            (0x0502)

/*
 * MISC : Miscellaneous
 * (x)  : the var of struct emvconfig
 * Macro:
   MISC_Support_Account_Select         : Does the terminal support account type selection
   MISC_Support_ISDL_Greater_than_128  : Is Issuer Script Device Limit greater than 128 bytes
   MISC_Support_Internal_Date_Mana     : Does the terminal support internal date management

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define MISC_Support_Account_Select        (0x0680)
#define MISC_Support_ISDL_Greater_than_128 (0x0640)
#define MISC_Support_Internal_Date_Mana    (0x0620)

#define MISC_PP_Support_Default_UDOL            (0x0602)    /* default UDOL (paypass only)*/
#define MISC_MISC_PP_Support_MagAppVer          (0x0601)    /* mag stripe application version (paypass only) */
/*
* Used to determine the function of ICS support,
* 1     support
* 0     unsupport
* nr    Refers to the above ICS support related functions of the macro definition:XXX_Support_XXX
* _ics  Incoming ICS pointer
*/
#define ics_opt_get(nr, _ics) \
        ((_ics)[((unsigned int)(nr)) >> 8] & (((unsigned int)(nr)) & 0x00FF))

/**< Set ICS support function */
#define ics_opt_set(nr, _ics) \
        ((_ics)[((unsigned int)(nr)) >> 8] |= (((unsigned int)(nr)) & 0x00FF))

/**< Unset ICS support function */
#define ics_opt_unset(nr, _ics) \
        ((_ics)[((unsigned int)(nr)) >> 8] &= ~(((unsigned int)(nr)) & 0x00FF))

/*
* RESV_Terminal_Present_DefaultTAC
* RESV_Terminal_Present_DenialTAC
* RESV_Terminal_Present_OnlineTAC: to indicate whether the terminal provide these TACs
* For external parameter tools to retain the use of TAC related assignments
*/
#define   RESV_Terminal_Present_DefaultTAC    (0x0008)
#define   RESV_Terminal_Present_DenialTAC     (0x0004)
#define   RESV_Terminal_Present_OnlineTAC     (0x0002)

#define   _tac_status_get(nr, _status)        ics_opt_get(nr, _status)
#define   _tac_status_set(nr, _status)        ics_opt_set(nr, _status)
#define   _tac_status_unset(nr, _status)      ics_opt_unset(nr, _status)

/**
 * TT : Terminal Type(emvparam._type)
 */
#define   TT_Unattended(x)                    ((*(x) & 0x0F) > 3)        /**< Terminal unattended,For example:ATM */
#define   TT_Attended(x)                      ((*(x) & 0x0F) < 4)        /**< Terminal someone on duty */

/**
 * TC : Terminal Capabilities(emvparam._cap)
 */
#define   TC_Manual_Key_Entry                 (0x0080)          /**< Manual keyboard input */
#define   TC_Magnetic_Stripe                  (0x0040)          /**< Magnetic stripe card */
#define   TC_IC_With_Contacts                 (0x0020)          /**< Contact IC card */
#define   TC_Plaintext_PIN                    (0x0180)          /**< Plain text PIN verification */
#define   TC_Enciphered_PIN_Online            (0x0140)          /**< Online encrypted PIN verification */
#define   TC_Signature_Paper                  (0x0120)          /**< signature on paper */
#define   TC_Enciphered_PIN_Offline           (0x0110)          /**< Offline encrypted PIN verification */
#define   TC_No_CVM_Required                  (0x0108)          /**< NO CVM */
#define   TC_Cardholder_Cert                  (0x0101)          /**< Cardholder certificate */
#define   TC_SDA                              (0x0280)          /**< Static data authentication */
#define   TC_DDA                              (0x0240)          /**< Dynamic data authentication */
#define   TC_Card_Capture                     (0x0220)          /**< draw back */
#define   TC_CDA                              (0x0208)          /**< Composite dynamic data authentication */

/**
 * Function to determine the capability of the terminal.
 *  1     support
 *  0     unsupport
 * nr:    A variety of functions defined above.
 * cap:   Incoming terminal capability pointer
 */
#define   terminal_cap(nr, cap)               ics_opt_get(nr, cap)
/**< Set terminal capability support function */
#define   terminal_cap_set(nr, cap)           ics_opt_set(nr, cap)
/**< Unset terminal capability support function */
#define   terminal_cap_unset(nr, cap)         ics_opt_unset(nr, cap)
/**< Whether the terminal supports offline PIN */
#define   terminal_offline_pin(cap)           (*(cap + 1) & 0x90)

/*
 * ATC : Additional Terminal Capabilities(emvparam._add_cap)
 */
#define   ATC_Cash                            (0x0080)          /**< Cash */
#define   ATC_Goods                           (0x0040)          /**< Goods */
#define   ATC_Services                        (0x0020)          /**< Services */
#define   ATC_Cashback                        (0x0010)          /**< Cashback */
#define   ATC_Inquiry                         (0x0008)          /**< Inquiry */
#define   ATC_Transfer                        (0x0004)          /**< Transfer */
#define   ATC_Payment                         (0x0002)          /**< Payment */
#define   ATC_Administrative                  (0x0001)          /**< Administrative */
#define   ATC_Cash_Deposit                    (0x0180)          /**< Cash_Deposit */
#define   ATC_Numeric_Keys                    (0x0280)          /**< Numeric_Keys */
#define   ATC_Alphabetic_Special_Keys         (0x0240)          /**< Alphabetic_Special_Keys */
#define   ATC_Command_Keys                    (0x0220)          /**< Command_Keys */
#define   ATC_Function_Keys                   (0x0210)          /**< Function_Keys */
#define   ATC_Print_Attendant                 (0x0380)          /**< Print to Attendant */
#define   ATC_Print_Cardholder                (0x0340)          /**< Print to Cardholder */
#define   ATC_Display_Attendant               (0x0320)          /**< Display to Attendant */
#define   ATC_Display_Cardholder              (0x0310)          /**< Display to Cardholder */
#define   ATC_Code_Table_10                   (0x0302)          /**< Code_Table_10 */
#define   ATC_Code_Table_9                    (0x0301)          /**< Code_Table_9 */
#define   ATC_Code_Table_8                    (0x0480)          /**< Code_Table_8 */
#define   ATC_Code_Table_7                    (0x0440)          /**< Code_Table_7 */
#define   ATC_Code_Table_6                    (0x0420)          /**< Code_Table_6 */
#define   ATC_Code_Table_5                    (0x0410)          /**< Code_Table_5 */
#define   ATC_Code_Table_4                    (0x0408)          /**< Code_Table_4 */
#define   ATC_Code_Table_3                    (0x0404)          /**< Code_Table_3 */
#define   ATC_Code_Table_2                    (0x0402)          /**< Code_Table_2 */
#define   ATC_Code_Table_1                    (0x0401)          /**< Code_Table_1 */

/**
 * Function to determine the additional capability of the terminal.
 *  1        support
 *  0        unsupport
 * nr:       A variety of functions defined above.
 * addcap:   Incoming additional terminal capability pointer
 */
#define   add_terminal_cap(nr, addcap)        ics_opt_get(nr, addcap)

/**< Set additional terminal capability support function */
#define   add_terminal_cap_set(nr, addcap)    ics_opt_set(nr, addcap)

/**< Unset additional terminal capability support function */
#define   add_terminal_cap_unset(nr, addcap)  ics_opt_unset(nr, addcap)

/*
  *Terminal transaction Property 9F66 (emvparam._trans_prop)
  */
#define   EMV_PROP_MSD                        (0x0080)          /**< 1:support Magnetic stripe card */
#define   EMV_PROP_PBOCCLSS                   (0x0040)          /**< 1:support contactless PBOC */
#define   EMV_PROP_QPBOC                      (0x0020)          /**< 1:support contactless qPBOC */
#define   EMV_PROP_PBOC                       (0x0010)          /**< 1:support contact PBOC */
#define   EMV_PROP_OFFLINE_ONLY               (0x0008)          /**< 1:support offline only  0:support online */
#define   EMV_PROP_ONLINEPIN                  (0x0004)          /**< 1:support online PIN */
#define   EMV_PROP_SIGNATURE                  (0x0002)          /**< 1:support signature */
/**< byte1 bit1 reserve */

/**< The following macros and reserved bits are set to 0 by default */
#define   EMV_PROP_ONLINEAC                   (0x0180)          /**< Request online */
#define   EMV_PROP_CVM                        (0x0140)          /**< Request CVM */
#define   EMV_PROP_01VERSUPPORT               (0x0380)
/*other bits reserve */

/**
 * Function to get the Terminal transaction Property
 *  1           support
 *  0           unsupport
 * nr:          A variety of functions defined above.
 * transprop:   Incoming Terminal transaction Property pointer
 */
#define   trans_prop(nr, transprop)           ics_opt_get(nr, transprop)
/**< Set Terminal transaction Property support function */
#define   trans_prop_set(nr, transprop)       ics_opt_set(nr, transprop)
/**< Unset Terminal transaction Property support function */
#define   trans_prop_unset(nr, transprop)     ics_opt_unset(nr, transprop)




/**< emv_opt._trans_ret && emv_opt._online_result(Return value of transaction result) */
#define   EMV_TRANS_CANCEL                 (-13)          /**< Transaction cancel */
#define   EMV_TRANS_NOCARD                 (-12)          /**< No card */
#define   EMV_TRANS_MORECARD               (-11)          /**< More Card */
#define   EMV_TRANS_FALLBACK                (-2)          /**< fallback */
#define   EMV_TRANS_TERMINATE               (-1)          /**< Transaction TERMINATE */
#define   EMV_TRANS_ACCEPT                   (1)          /**< Transaction ACCEPT */
#define   EMV_TRANS_DENIAL                   (2)          /**< Transaction DENIAL */
#define   EMV_TRANS_GOONLINE                 (3)          /**< Transaction GOONLINE */
#define   EMV_TRANS_2GAC_AAC                 (4)          /**< Second GAC return AAC */
#define   EMV_TRANS_ONLINEFAIL               (5)          /**< emv_opt._online_result Online failure */
#define   EMV_TRANS_ONLINESUCC_ACCEPT        (6)          /**< emv_opt._online_result The online trading success and acceptance */
#define   EMV_TRANS_ONLINESUCC_DENIAL        (7)          /**< emv_opt._online_result Online success and rejection of reference */
#define   EMV_TRANS_ONLINESUCC_ISSREF        (8)          /**< emv_opt._online_result Online success and return to reference */
#define   EMV_TRANS_GOON_PBOC2LOG            (9)          /**< Successfully obtain PBOC2 log */
#define   EMV_TRANS_GOON_ECLOADLOG          (10)          /**< Successfully obtain transference log */
#define   EMV_TRANS_EC_GOON_AMOUNT          (12)          /**< Successfully obtain EC balance */
#define   EMV_TRANS_QPBOC_ACCEPT            (13)          /**< QPBOC Transaction ACCEPT */
#define   EMV_TRANS_QPBOC_DENIAL            (14)          /**< QPBOC Transaction DENIAL */
#define   EMV_TRANS_QPBOC_GOONLINE          (15)          /**< QPBOC Transaction GOONLINE */
#define   EMV_TRANS_MSD_GOONLINE            (16)          /**< MSD Transaction GOONLINE */
#define   EMV_TRANS_RF_GOON_AMOUNT          (17)          /**< Successfully obtain QPBOC balance */
#define   EMV_TRANS_RF_ACTIVECARD           (18)          /**< Please activate the RF card */
#define   EMV_TRANS_SLECT_NEXTAID           (19)          /**< Request next AID*/
#define	  EMV_TRANS_DIRECT_GPO				(20)			/**< only execute to GPO*/
#define   EMV_TRANS_FINAL_SELECTION			(21)			/**< only execute to Final Selection*/


/**< emv_opt._trans_type(Type of request transaction) */
#define   EMV_TRANS_GOODS                 (0x01)          /**< GOODS */
#define   EMV_TRANS_SERVICES              (0x02)          /**< SERVICES */
#define   EMV_TRANS_CASH                  (0x03)          /**< CASH */
#define   EMV_TRANS_CASHBACK              (0x04)          /**< CASHBACK */
#define   EMV_TRANS_INQUIRY               (0x05)          /**< INQUIRY */
#define   EMV_TRANS_TRANFER               (0x06)          /**< TRANFER */
#define   EMV_TRANS_ADMIN                 (0x07)          /**< ADMIN */
#define   EMV_TRANS_CASHDEPOSIT           (0x08)          /**< CASHDEPOSIT */
#define   EMV_TRANS_PAYMENT               (0x09)          /**< PAYMENT */
#define   EMV_TRANS_PBOCLOG               (0x0A)          /**< Get PBOCLOG
(Only EMV_Start() return EMV_TRANS_GOON_PBOC2LOG,Successfully) */
#define   EMV_TRANS_SALE                  (0x0B)          /**< SALE */
#define   EMV_TRANS_PREAUTH               (0x0C)          /**< PREAUTH */
#define   EMV_TRANS_BALANCE               (0x0D)          /**< BALANCE */
#define   EMV_TRANS_ECLOADLOG             (0x0E)          /**< ECLOADLOG */

/**< +++ Types of electronic cash transactions+++ */
#define   EMV_TRANS_EC_GOODS              (EMV_TRANS_GOODS)         /**< EC_GOODS */
#define   EMV_TRANS_EC_SERVICES           (EMV_TRANS_SERVICES)      /**< EC_SERVICES */
#define   EMV_TRANS_EC_SALE               (EMV_TRANS_SALE)          /**< EC_SALE */
#define   EMV_TRANS_EC_BINDLOAD           (0x21)                    /**< EC_BINDLOAD */
#define   EMV_TRANS_EC_NOBINDLOAD         (0x22)                    /**< EC_NOBINDLOAD */
#define   EMV_TRANS_EC_CASHLOAD           (0x23)                    /**< CASHLOAD */
#define   EMV_TRANS_EC_UPLOAD             (0x24)                    /**< EC_UPLOAD */
#define   EMV_TRANS_EC_INQUIRE_LOG        (EMV_TRANS_PBOCLOG)       /**< EC_INQUIRE_LOG */
#define   EMV_TRANS_EC_INQUIRE_AMOUNT     (0x25)                    /**< EC_INQUIRE_AMOUNT
(Only EMV_Start() return TRANS_EC_GOON_AMOUNT,Successfully) */
#define   EMV_TRANS_EC_CASHLOAD_VOID      (0x26)                    /**< EC_CASHLOAD_VOID */

/**< +++Types of QPBOC /MSD transactions+++ */
#define   EMV_TRANS_RF_START              (0x30)                    /**< *<RF_START */
#define   EMV_TRANS_RF_GOODS              (EMV_TRANS_GOODS)         /**< QPBOC/MSD GOODS */
#define   EMV_TRANS_RF_SERVICES           (EMV_TRANS_SERVICES)      /**< QPBOC/MSD SERVICES */
#define   EMV_TRANS_RF_SALE               (EMV_TRANS_SALE)          /**< QPBOC/MSD SALE */
#define   EMV_TRANS_RF_BINDLOAD           (0x31)                    /**< RF_BINDLOAD */
#define   EMV_TRANS_RF_NOBINDLOAD         (0x32)                    /**< RF_NOBINDLOAD */
#define   EMV_TRANS_RF_CASHLOAD           (0x33)                    /**< RF_CASHLOAD */
#define   EMV_TRANS_RF_INQUIRE_AMOUNT     (0x34)                    /**< RF_INQUIRE_AMOUNT
(Only EMV_rf_start() return TRANS_RF_GOON_AMOUNT,Successfully) */
#define   EMV_TRANS_RF_UPLOAD             (0x35)                    /**< RF_UPLOAD */
#define   EMV_TRANS_RF_CASHLOAD_VOID      (0x36)                    /**< RF_CASHLOAD_VOID */
#define   EMV_TRANS_RF_PBOCLOG            (0x37)                    /**< RF_PBOCLOG */
#define   EMV_TRANS_RF_UPTCARDINFO        (0x38)                    /**< RF_UPTCARDINFO */
#define   EMV_TRANS_RF_PBOC_SALE          (0x39)
#define   EMV_TRANS_RF_ECLOADLOG          (0x40)                    /**< RF_ECLOADLOG */
/**< STEMVOPTION.nRequestAmount :   When to enter the amount */
#define   EMV_TRANS_REQAMT_NO             (0)                       /**< No input amount */
#define   EMV_TRANS_REQAMT_APS            (1)                       /**< Input in application selection */
#define   EMV_TRANS_REQAMT_DDA            (2)                       /**< Input in Data authentication */
#define   EMV_TRANS_REQAMT_RFPRECESS      (3)                       /**< RF:Input in Preprocess */

/*  EMV Trading options structure*/
typedef struct
{
	unsigned char  _trans_type;              /**< in, transaction type, see above */
	emv_seq        _seq_to;                  /**< in, when to terminate the session */
	int            _request_amt;             /**< in, whether to request the the amount, before PAN */
	/**< if the terminal ICS support the below 3 options */
	int            _force_online_enable;     /**< in, whether the force online option opened */
	int            _account_type_enable;     /**< in, whether the account type selection opened */
	unsigned char* _online_pin;              /**< out, string with '\0' if online pin is entered */
	unsigned char* _iss_script_res;          /**< out, if issuer script result exists */
	int            _iss_sres_len;
	int            _advice_req;              /**< out, if advice is required (must be supported by ics) */
	int            _force_accept_supported;  /**< out, if ICS support it */
	int            _signature_req;           /**< out, if the CVM finally request a signature */
	unsigned char* _auth_resp_code;          /**< in, 8A from the host */
	unsigned char* _field55;                 /**< in, field55 or tlv decoded data from the host */
	int            _field55_len;
	int            _online_result;           /**< in, the online result */
	int            _trans_ret;               /**< transaction return */
} emv_opt;

/*
 * EMV Files name structure
 */
#define   EMV_PATH_NAME_MAX             (26 + 1)          /**< EMV Longest path name */
#define   EMV_FILE_NAME_MAX             (12 + 1)          /**< EMV Longest file name */

/**
* @struct  emv_file Emv File path and file name(EMV Files name structure)
*/
typedef struct
{
	char  _file_path[EMV_PATH_NAME_MAX];                   /**< path */
	char  _conf_name[EMV_FILE_NAME_MAX];                   /**< Terminal configuration parameters */
	char  _capk_name[EMV_FILE_NAME_MAX];                   /**< Public key file */
	char  _card_blk[EMV_FILE_NAME_MAX];                    /**< Card black list file */
	char  _cert_blk[EMV_FILE_NAME_MAX];                    /**< Cert black list file */
	char  _emv_log[EMV_FILE_NAME_MAX];                     /**< EMV LOG file */
} emv_file;


#define   EMV_CARD_IC                    (0x00)            /**< contact IC Card */
#define   EMV_CARD_IC2                   (0x01)            /**< contact IC Card 2 */
#define   EMV_CARD_QIC                   (0xa1)            /**< contactless IC Card */

/**< emv Operating function pointer */
typedef struct
{
	int          (*emv_get_transamt)(unsigned char transtype, unsigned long long* cash, unsigned long long* cashback);
	int          (*emv_get_pinentry)(int type, char* pinentry);
	int          (*iss_ref)(unsigned char* pan, int panlen);
	int          (*acctype_sel)(void);
	int          (*inc_tsc)(void);
	int          (*cert_confirm)(unsigned char type, unsigned char* pcon, int len);
	int          (*lcd_msg)(char* title, unsigned char* msg, int len,  int yesno, int waittime);
	int          (*candidate_sel)(candidate* pcan, int amt, int times);
	int          (*emv_asc_2_bcd)(unsigned char* ascstr, int asclen, unsigned char* bcdstr, int align);
	int          (*emv_bcd_2_asc)(unsigned char* bcd, int asclen, unsigned char* asc, int align);
	unsigned int (*emv_c4_2_uint)(unsigned char* c4);
	void         (*emv_uint_2_c4)(unsigned int num, unsigned char* c4);
	unsigned long long  (*bcd_2_uint64)(unsigned char*bcd,  int bcd_len);
	int          (*emv_icc_rw)(int cardno, unsigned char* inbuf, int inlen, unsigned char* outbuf, int olen);
	int          (*emv_rf_powerdown)(int cardno);
	int          (*emv_ec_switch)(void);
	int          (*emv_icc_powerup)(int* cardno);
	int          (*emv_get_bcdamt)(unsigned char ucTransType, unsigned char *pusBCDCash, unsigned char *pusBCDCashBack);
} emv_oper;

/**
* @brief Emv Initialize
* @detail Incoming EMV file path, EMV related file name, EMV related operating function pointer
* @param in  pfile  --- EMV file name
* @param in  poper  --- EMV Operating function pointer
* @return
* @li 0         SUCC
* @li <0        FAIL
*/
extern int EMV_Initialize(emv_file* pfile, emv_oper* poper);

/**
* @brief Same as EMV_Initialize, the difference is the amount of input for the BCD code
*        EMV_Initialize function will be eliminated
* @detail Incoming EMV file path, EMV related file name, EMV related operating function pointer
* @param in  pfile  --- EMV file name
* @param in  poper  --- EMV Operating function pointer
* @return
* @li 0         SUCC
* @li <0        FAIL
*/
extern int EMVL2_Initialize(emv_file * pfile, emv_oper * poper);


/***********************************IC CARD Trading function*************************/
/**
* @brief IC card transaction
* @param in out popt  --- EMV Trading Options
* @return
* @li    SUCC
*/
extern int EMV_Start(emv_opt * popt);


/**
* @brief EMV IC End of transaction execution function
* @detail Implementation of the IC card under the amount of electricity and transaction log save
* @param in nTransRes --- Final transaction result
* @return
* @li        0        SUCC
* @li       -1        FAIL
*/
extern int EMV_Suspend(int nTransRes);

/*  ****************************    RF card function  *****************************   */
/**
* @brief  RF card transaction
* @param in out pstEmvOption    --- EMV Trading Options
* @param in     transAmount     --- trans Amount 6byte BCD
* @return
* @li    EMV_TRANS_RF_ACTIVECARD    Please activate the card
* @li    <0                         FAIL
*/
extern int EMV_rf_start(emv_opt* pstEmvOption, unsigned long long transAmount);
/**
* @brief  EMV RF End of transaction execution function
* @param in nFinalFlag    ---Final transaction result
* @return
* @li       0             SUCC
* @li       -1            FAIL
*/
extern int EMV_rf_suspend(int nFinalFlag);


/******************************Other operating functions **************************/
/**
* @brief EMV version
* @return
* @li Version    
*/
extern const char* EMV_getVersion(void);

/**
* @brief EMV error code
* @return
* @li ErrorCode
*/
extern int EMV_ErrorCode(void);

/**
* @brief get lunTagName[] TLV data,tag + len + value
* @param in out   punTagName  Holds the first pointer to the tag array to get the TLV data
*        in       nTagCnt     Number of TLV data to be acquired
*        out      pusOutBuf   TLV Data
*        in       nMaxOutLen  Maximum storage space of an array
* @return
* @li    -2        Parameter is empty
* @li    <0        FAIL
* @li   n(n>0)     Total data length
*/
extern int EMV_FetchData(unsigned int* punTagName, int nTagCnt, unsigned char* pusOutBuf, int nMaxOutLen);

/**
* @brief Get the data value of TagName
* @param in  unTagName                ---    TagName
* @param out pusData                  ---    Value
* @param in  nMaxOutLen               ---    Value Maximum length limit
* @return
* @li 0        tag value does not exist
* @li >0       The length of the Value
* @li -1       Data length exceeds length limit
*/
extern int EMV_getdata(unsigned int unTagName, unsigned char *pusData, int nMaxOutLen);

/**
* @brief Set the data value of TagName
* @param in       unTagName  TagName
*        in       pusData    Value
*        in       nMaxLen    Value Length
* @return
* @li     0        SUCC
* @li    <0        FAIL
* @li    -2        No set of permissions for this Tag
*/
extern int EMV_setdata(unsigned int unTagName, unsigned char* pusData, int nMaxLen);

/*=========================================================================*/

typedef struct tlv
{
	/**< internal state */
	unsigned char* _tagptr;           /**< pointer of  'tag' field in the TLV String */
	unsigned char* _lenptr;           /**< pointer of  'len' field in the TLV String */
	unsigned int   _len;              /**< length from first pointer  to itself in its parent's string */

	/**< parsed information */
	int            parent;            /**< recored the parent position in the tlv_t structure */
	int            childnum;          /**< the num of its children (no children = -1) */

	/**< tag len value */
	unsigned int   tagname;           /**< record tagname */
	unsigned int   valuelen;          /**< record the length of value */
	unsigned char* pvalue;            /**< pointer of value in the TLV String */
} tlv_t;

/**< nDefFlag defined */
#define   SINGLE_TLVOBJ     (0x01)    /**< it is a single constructed object      (PRIMITIVE_TLV_DATA)*/
#define   STRING_TLVOBJ     (0x02)    /**< it is a tlv object string not coded in a constructed object      (CONSTRUCTED_TLV_DATA)*/
#define   DECODE_LEVEL1     (0x10)    /**< just decode the object in level one */

/**
* @brief  Parse TLV string to tlv_t  *pstTlvList
* @detail 
          pstTlvList[0].pvalue        Save TLV string pointer
          pstTlvList[0].valuelen      Save TLV string length
          pstTlvList[0].childnum      TLV string children number
* @param  in  pusTlvBuf       ---     TLV data
* @param  in  nTlvlen         ---     Tlv data length
* @param  out pstTlvList      ---     TLVLists
* @param  in  nTlvArrayNum    ---     TLVLists Array size
* @param  in  nFlag           ---     SINGLE_TLVOBJ        
*                                     STRING_TLVOBJ       
*                                     DECODE_LEVEL1        
* @return
* @li 0       SUCC
* @li <0      FAIL
*/
extern int EMV_parse_tlv(unsigned char* pusTlvBuf, int nTlvlen, tlv_t* pstTlvList, int nTlvArrayNum, int nDefFlag);

/**< level */
#define   SEARCH_ALL_DESC               (0x01)            /**< search all the descendants */
#define   SEARCH_ONLY_SON               (0x02)            /**< search only its sons  */


/**
* @brief  Get index number of TLV data of unTagName in pstTlvList structure
* @detail With EMV_parse_tlv int () function to use
* @param  in nParent       ---    Start searching from the parent node
* @param  in unTagName     ---    TagName
* @param  in pstTlvList    ---    TLVLists
* @param  in nLevel        ---    Layer of search
*                                 SEARCH_ONLY_SON   Search only in their own sub node
*                                 SEARCH_ALL_DESC   Find in all subsequent nodes
* @return
* @li     SUCC         index number
* @li     FAIL         0
*/
extern int EMV_fetch_tlv(int nParent, unsigned int unTagName, tlv_t* pstTlvList, int nLevel);

/**
* detail PBOC Transaction Logs Reading
*/
typedef enum
{
    PBOCLOG_RECNUM = -2,
    PBOCLOG_SFI,
    PBOCLOG_FMT
} EM_PBOCLOGTAG;
/**
* @brief Get PBOC LOG
* @param in  nRec        ---Number of records to read
* @param out pusOut      ---Save outgoing log data from function 
* @param in  nOutMaxLen  ---Maximum length limit for outgoing data
* @return
* @li     0              Do not get the log data
* @li    >0              outgoing log data length
* @li    <0              FAIL
*/
extern int EMV_GetPBOCLog(int nRec, unsigned char* pusOut, int nOutMaxLen);

/*
 * PBOC unload Logs Reading
 */
/**
* @brief Get EC LOG
* @param in    nRecNo      Number of records to read
*        out   pusOutBuf   Save outgoing log data from function 
*        in    nMaxOutLen  Maximum length limit for outgoing data
*
* @return
* @li   < 0            FAIL
*       > 0            outgoing log data length
*       = 0            Do not get the log data
*/
extern int EMV_GetecloadLog(int nRecNo, unsigned char* pusOutBuf, int nMaxOutLen);

/*
 * CAPK Interface
 */
#define   MAX_MODULUS_LEN               (248)             /**< Maximum length of public key */

/**
* @struct publickey (RSA Key)
*/
typedef struct
{
	unsigned char pk_modulus[MAX_MODULUS_LEN];        /**< Public key module */
	unsigned char pk_mod_len;                         /**< Public key module length */
	unsigned char pk_exponent[3];                     /**< Public key exponent */
} publickey;

/**
* @struct capk (Certification Authority Public Key structure)
*/
typedef struct
{
	publickey     _key;                               /**< RSA Public key */
	unsigned char _hashvalue[20];                     /**< Public key HASH value */
	unsigned char _expired_date[4];                   /**< Public key expiration time */
	unsigned char _rid[5];                            /**< RID */
	unsigned char _index;                             /**< Public key index */
	unsigned char _pk_algorithm;                      /**< Public key algorithm identifier */
	unsigned char _hash_algorithm;                    /**< HASH algorithm identifier */
	unsigned char _disable;                           /**< =1 Public key disable */
	unsigned char _resv[3];                           /**< reserved */
} capk;

/**< Public key operation error value definition */
#define   CAPKERR_BASE              (-4000)
#define   CAPKERR_FILEOPEN          (CAPKERR_BASE - 1) /**< File open error */
#define   CAPKERR_FILEWRITE         (CAPKERR_BASE - 2) /**< File write error */
#define   CAPKERR_FILEREAD          (CAPKERR_BASE - 3) /**< File read error */
#define   CAPKERR_CHKSUM            (CAPKERR_BASE - 4) /**< Public key checksum error */
#define   CAPKERR_LOST              (CAPKERR_BASE - 5) /**< This public key was not found */
#define   CAPKERR_PARAM             (CAPKERR_BASE - 6) /**< Parameter error */
#define   CAPKERR_FILELEN           (CAPKERR_BASE - 7) /**< File length error */


/**< Public key operation mode  */
#define   CAPK_RMV                  (0x01)             /**< DELETE */
#define   CAPK_UPT                  (0x02)             /**< UPDATE(ADD) */
#define   CAPK_DIS                  (0x04)             /**< DISABLE */
#define   CAPK_ENB                  (0x08)             /**< ABLE */
#define   CAPK_GET                  (0x10)             /**< GET */
#define   CAPK_CLR                  (0x20)             /**< CLEAR */

/**
* @brief    Public key operation
* @param in out pstCAPK   ---struct capk
* @param in nMode         ---Public key operation mode
* @return
* @li       0             SUCC
* @li       -1            FAIL
*/
extern int EMV_OperCAPK(capk* pstCAPK, int nMode);
/**
* @brief    Delete CAPK by RID
* @param in sRID          ---RID
* @return
* @li       0             SUCC
* @li       -1            FAIL
*/
extern int EMV_removeCAPKByRID(char sRID[5]);

/**
* @brief     Enumerate CAPK
* @param in  nStart        ---Start index
* @param in  nEnd          ---End index
* @param out lsCAPK        ---CAPK Lists
* @return
* @li        >0            Number of public key
* @li        <= 0          FAIL
*/
extern int EMV_EnumCAPK(int nStart, int nEnd, char lsCAPK[][6]);

/*=========================================================================*/
/**< AID operation error value definition */
#define	AIDERR_BASE				(-5000)
#define	AIDERR_FILEOPEN			(AIDERR_BASE - 1) /**< File open error */
#define	AIDERR_FILEWRITE			(AIDERR_BASE - 2) /**< File write error */
#define	AIDERR_FILEREAD			(AIDERR_BASE - 3) /**< File read error */
#define	AIDERR_CHKSUM				(AIDERR_BASE - 4) /**< Public key checksum error */
#define	AIDERR_LOST				(AIDERR_BASE - 5) /**< This AID was not found */
#define	AIDERR_PARAM				(AIDERR_BASE - 6) /**< Parameter error */
#define	AIDERR_FILELEN				(AIDERR_BASE - 7) /**< File length error */
#define	AIDERR_UPTAID				(AIDERR_BASE - 8) /**< Synchronous update AID failed */
#define	AIDERR_PARSE				(AIDERR_BASE - 9) /**< AID parse fail*/

/**< AID operation mode */
#define	AID_RMV					(0x01)            /**< DELETE a AID */
#define	AID_UPT					(0x02)            /**< UPDATE(ADD)*/
#define	AID_GET					(0x10)            /**< GET */
#define	AID_CONFIG_R				(0x20)            /**< Read terminal configuration */
#define	AID_CONFIG_W				(0x40)            /**< Write terminal configuration */
#define	AID_CLR						(0x80)            /**< Clear all AID */

/**
* @brief    terminal configuration or AID operation
* @detail   
* @param in out pstEmvParam  ---EMV struct AID
* @param in     nMode        ---AID operation Mode
* @return
* @li           0            SUCC
* @li           -1           FAIL
*/
extern int EMV_OperAID(emvparam* pstEmvParam, int nMode);

/**
* @brief  Enumerate all AID
* @param out lsAid    ---AID Lists
* @return
* @li        >0           Number of AID
* @li        <= 0         FAIL
*/
extern int EMV_EnumAID(char lsAid[][17]);

/**
* @brief  Re read the AID file, Build AID List 
* @return
* @li     0              SUCC
* @li     -1             FAIL
*/
extern int EMV_buildAidList(void);

/**
 *@detail struct Operation mode
 */
#define   STRUCT_DEL                    (0x01)            /**< DELETE */
#define   STRUCT_UPT                    (0x02)            /**< UPDATE(ADD) */
#define   STRUCT_GET                    (0x04)            /**< GET */
#define   STRUCT_CLR                    (0x08)            /**< CLEAR ALL STRUCT */


/*========================Certificate black=============================*/
/**
* @struct  Certificate black 
*/
typedef struct
{
	unsigned char  _rid[5];
	unsigned char  _index;
	unsigned char  _sn[3];
	unsigned char  _disable;                              /**< (0x01 : disable) */
	unsigned char  _rsv[2];
} certblk;

/**
* @brief    Certificate black operation
* @param in out pstCertBlk    ---struct Certificate black 
* @param in     nMode         ---STRUCT_DEL     DELETE
*                                STRUCT_UPT     UPDATE(ADD)
*                                STRUCT_GET     GET
*                                STRUCT_CLR     CLEAR ALL STRUCT
* @return
* @li           0          SUCC
* @li           -1         FAIL
*/
extern int EMV_oper_certblk(certblk* pstCertBlk, int nMode);

/**
* @struct  card black 
*/
typedef struct
{
	unsigned char _card_no[10];                /**< PAN (0x5A) */
	unsigned char _len;                        /**< PAN length */
	unsigned char _index;                      /**< PAN serial number(0x5F34) */
	unsigned char _disable;                    /**< 0x01: shield the entire card black list */
	unsigned char _partial_match;              /**< 0x01: Allow PAN to forward partial matches */
	unsigned char _disable_index;              /**< 0x01: (only PAN without PAN serial number)
                                                    0x00: (PAN with PAN serial number) */
	unsigned char _rsv[1];
} cardblk;

/**
* @brief  card black operation
* @param in out pstEmvCardBlack    ---struct card black 
* @param in nMode                  ---STRUCT_DEL     DELETE
*                                     STRUCT_UPT     UPDATE(ADD)
*                                     STRUCT_GET     GET
*                                     STRUCT_CLR     CLEAR ALL STRUCT
* @return
* @li       0             SUCC
* @li       -1            FAIL
*/
extern int EMV_oper_cardblk(cardblk *pstEmvCardBlack, int nMode);


typedef enum
{
    ATC,
    LASTONLINE_ATC,
    PWD_RETRY,
    LOG_FMT,
    EC_BALANCE,
    EC_RESET_THRESHOLD,
    RF_BALANCE,
    EC_BALANCE_LIMIT,
    EC_SINGLE_AMOUNT_LIMIT,
    EC_LOAD_LOG_FMT,
    EC_BALANCE_SEC,
    EC_APPCURR_CODE,
    EC_SECCURR_CODE,
    GETDATA_TOTAL
} GetData;
/**
* @brief Send GetData command, get tag value
* @param in  emGetData  ---Getdata
* @param out pusOut     ---data
* @param out pnOutLen   ---data length
* @return
* @li         0            SUCC
* @li        -1            FAIL
*/
extern int EMV_ICC_getdata(GetData emGetData, unsigned char* pusOut, int* pnOutLen);

/**
* @brief     only execute to Final Selection
* @param in  nVal          =0 Normal execution, =1 only execute to Final Selection
* @return
* @li         0            
*/
extern int EMV_run_to_finalsel(int value);

/**
* @brief Send GetData command, get tag value
* @param in  unTagName  ---TagName 2 byte
* @param out pusOut     ---data
* @param out pnOutLen   ---data length
* @return
* @li         0            SUCC
* @li        -1            FAIL
*/
extern int EMV_ICC_GetDataByTagName(unsigned int unTagName, unsigned char * pusOut, int * pnOutLen);

/**
* @brief Get the branch of a transaction
* @param out pucTransBranch ---branch of a transaction
* @return
* @li         0            SUCC
* @li        -1            FAIL
* @
*/
extern int EMVL2_GetTransBranch(unsigned char *pucTransBranch);

/**
* @brief Set whether to detect black list.
* @param in ucNotChkBlkCard --- 0-detect,1-Not detect
* @return
* @li
* @
*/
extern void  EMVL2_SetNotChkBlkCard(const unsigned char ucNotChkBlkCard);

/**
* @brief Get whether to detect black list.
* @param void
* @return 0-detect,1-Not detect
* @li
* @
*/
extern unsigned char EMVL2_GetNotChkBlkCard(void);

/**
* @brief Set 9C transaction type
* @param in ucTransType --- 9C transaction type
* @return
* @li
* @
*/
extern void EMVL2_Set9CTransType(const unsigned char ucTransType);

/**
* @brief Get 9C transaction type
* @param void
* @return	 9C transaction type
* @li
* @
*/
extern unsigned char EMVL2_Get9CTransType(void);

/**
* @brief Set the 9C flag transaction type is set up
* @param in ucFlag --- 0-not be set,1-be set
* @return
* @li
* @
*/
extern void EMVL2_Set9CFlag(const unsigned char ucFlag);

/**
* @brief Get the 9C flag transaction type is set up
* @param void
* @return	 0-not be set,1-be set
* @li
* @
*/
 unsigned char EMVL2_Get9CFlag(void);

/**
* @brief Designated AID transaction,Parameters need to be set NULL(length 0) when you do not need to designated
* @param in pucAid --- Designated AID
* @param in nAidLen --- AID length
* @return
* @li
* @
*/
extern int EMVL2_SetBindAid(unsigned char  *pucAid, int nAidLen);

/**
* @brief Get designated AID 
* @param in pucAid --- Designated AID
* @param in nAidLen --- AID length
* @return
* @li
* @
*/
extern int EMVL2_GetBindAid(unsigned char  *pucAid, int *pnAidLen);

/**
* @brief Set double currency electronic cash switch
* @param in nDoubCurrFlag  1-open, 0-close
* @return
* @li
* @
*/
extern int EMVL2_SetDoulCurrFlag(int nDoubCurrFlag);

/**
* @brief Get double currency electronic cash switch
* @param out pnDoubCurrFlag  1-open, 0-close
* @return
* @li
* @
*/
extern int EMVL2_GetDoulCurrFlag(int *pnDoubCurrFlag);

/**
* @brief Set NLTag value
* @param in       unTagName   NLTag name
*        in       pusVal      NLTag value
*        in       nValLen     value length
* @return
* @li     0        SUCC
*/
extern int EMVL2_WriteNLTagData(const unsigned int unTagName, const unsigned char * pusVal, int nValLen);
/**
* @brief Get NLTag value
* @param in  unTagName      ---    NLTag name
* @param out  pnValLen      ---    value length
* @return    
* @li     NLTag value
* @li     NULL
*/
extern unsigned char*   EMVL2_ReadNLTagData(const unsigned int unTagName, int* pnValLen);

/**
* @brief Flash Card process
* @param emv_opt* pstEmvOpt         -- emv control struct
* @param const uchar * pusInData    -- emv tag data
* @param int nInDataLen             -- tag data length
* @return
* @li
*/
extern int EMVL2_FlashCard(emv_opt* pstEmvOpt, const unsigned char * pusInData, int nInDataLen);
/**
* @brief Flash Card process Completely
* @param emv_opt* pstEmvOpt         -- emv control struct
* @param const uchar * pusInData    -- emv tag data
* @param int nInDataLen             -- tag data length
* @return
* @li
*/
extern int EMVL2_FlashCard_Complete(emv_opt* pstEmvOpt, const unsigned char * pusInData, int nInDataLen);


/**
* @brief Pack all emv tag to TLV data
* @param out uchar * pusOutBuf  	--	TLV data
* @param in  int nMaxOutLen     	--  TLV data maximum length
* @return
* @li 						-- >0   TLV data length
* @li						-- <0	Packet data length exceeds the limit of data length
*/
extern int EMVL2_GetAllTagData(unsigned char * pusOutBuf, int nMaxOutLen);

/**
* @brief Set external has been done with the preprocess
* @param in nFlag  1-Has completed the preprocess , 0-No preprocess
* @return
* @li
* @
*/
extern int EMVL2_SetDonePreProcess(int nFlag);
/**
* @brief Get external has been done with the preprocess
* @param 
* @return      1-Has completed the preprocess , 0-No preprocess
* @li
* @
*/
extern int EMVL2_GetDonePreProcess();

/*pure Contactless application capabilities*/
/*<mccs specification(page204)>  Contactless application capabilities (20161202) author linlufeng*/
/*byte 1*/
#define PURE_AC_MODE(x)                      (*(x) & 0x20)
#define PURE_AC_CONTACT(x)                (*(x) & 0x10)
#define PURE_AC_OFFLINE(x)                 (*(x) & 0x08)
#define PURE_AC_ONLINEPIN(x)             (*(x) & 0x04)
#define PURE_AC_SIGNATURE(x)            (*(x) & 0x02)
/*byte 3*/
#define PURE_AC_ONLINE_TAP(x)          (*(x + 2) & 0x80)
#define PURE_AC_CC_TAP(x)                  (*(x + 2) & 0x40)
#define PURE_AC_ECHO(x)                      (*(x + 2) & 0x20)
#define PURE_AC_LONG_TAP(x)              (*(x + 2) & 0x10)
/*byte 4 */
#define PURE_AC_AUTH_APPLI(x)          (*(x + 3) & 0x40)
#define PURE_AC_BALANCE(x)                (*(x + 3) & 0x08)
#define PURE_AC_DECLINE(x)                 (*(x + 3) & 0x02)
#define PURE_AC_OFFLINECAM(x)          (*(x + 3) & 0x01)
/*byte 5 TIP*/
#define PURE_AC_RELIAB(x)                   (*(x + 4) & 0x80)
#define PURE_AC_SDA(x)                        (*(x + 4) & 0x40)
#define PURE_AC_DDA(x)                        (*(x + 4) & 0x20)
#define PURE_AC_CVM(x)                        (*(x + 4) & 0x10)
#define PURE_AC_TRM(x)                        (*(x + 4) & 0x08)
#define PURE_AC_CDA(x)                        (*(x + 4) & 0x01)

/*byte 1*/
#define MCCS_AC_MODE                      0x0020
#define MCCS_AC_CONTACT              		0x0010
#define MCCS_AC_OFFLINE                	0x0008
#define MCCS_AC_ONLINEPIN              	0x0004
#define MCCS_AC_SIGNATURE           		0x0002
/*byte 3*/
#define MCCS_AC_ONLINE_TAP          		0x0280
#define MCCS_AC_CC_TAP                 	0x0240
#define MCCS_AC_ECHO                     	0x0220
#define MCCS_AC_LONG_TAP         			0x0210
/*byte 4 */
#define MCCS_AC_AUTH_APPLI          		0x0340
#define MCCS_AC_BALANCE                	0x0308
#define MCCS_AC_DECLINE                	0x0302
#define MCCS_AC_OFFLINECAM          		0x0301
/*byte 5 TIP*/
#define MCCS_AC_RELIAB                  	0x0480
#define MCCS_AC_SDA                       	0x0440
#define MCCS_AC_DDA                       	0x0420
#define MCCS_AC_CVM                        0x0410
#define MCCS_AC_TRM                       	0x0408
#define MCCS_AC_CDA                        0x0401



#define   mccs_ac_set(nr, transprop)       ics_opt_set(nr, transprop)
#define   mccs_ac_unset(nr, transprop)     ics_opt_unset(nr, transprop)

#define EMV_EP_CTRL_CK_CANDIDATE_LIST        0x04


#endif

