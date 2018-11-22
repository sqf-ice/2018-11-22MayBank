/***************************************************************************
** All right reserved:  Desay Copyright (c) 2016-2026  
** File name:  packet.c
** File indentifier: 
** Brief:  Packing module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
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
* Define ISO Format
*/
const static struct STISO_FIELD_DEF gstMaybankIso8583Def[] =
{
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBMsgIDpack,  PubFPBMsgIDUnpack},  /* 0   "MESSAGE TYPE INDICATOR" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT,  '\0', PubFPBBitmapPack, PubFPBBitmapUnPack}, /* 1   "BIT MAP" */
	{ 19, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 2   "PAN - PRIMARY ACCOUNT NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 3   "PROCESSING CODE" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 4   "AMOUNT, TRANSACTION" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 5   "AMOUNT, SETTLEMENT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 6   "AMOUNT, CARDHOLDER BILLING" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 7   "TRANSMISSION DATE AND TIME" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 8   "AMOUNT, CARDHOLDER BILLING FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 9   "CONVERSION RATE, SETTLEMENT" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 10  "CONVERSION RATE, CARDHOLDER BILLING" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 11  "SYSTEM TRACE AUDIT NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 12  "TIME, LOCAL TRANSACTION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 13  "DATE, LOCAL TRANSACTION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 14  "DATE, EXPIRATION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 15  "DATE, SETTLEMENT" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 16  "DATE, CONVERSION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 17  "DATE, CAPTURE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 18  "MERCHANTS TYPE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 19  "ACQUIRING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 20  "PAN EXTENDED COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 21  "FORWARDING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 22  "POINT OF SERVICE ENTRY MODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 23  "CARD SEQUENCE NUMBER" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 24  "NETWORK INTERNATIONAL IDENTIFIEER" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 25  "POINT OF SERVICE CONDITION CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 26  "POINT OF SERVICE PIN CAPTURE CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 27  "AUTHORIZATION IDENTIFICATION RESP LEN" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 28  "AMOUNT, TRANSACTION FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 29  "AMOUNT, SETTLEMENT FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 30  "AMOUNT, TRANSACTION PROCESSING FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 31  "AMOUNT, SETTLEMENT PROCESSING FEE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 32  "ACQUIRING INSTITUTION IDENT CODE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 33  "FORWARDING INSTITUTION IDENT CODE" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 34  "PAN EXTENDED" */
	{ 37, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 35  "TRACK 2 DATA" */
	{104, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 36  "TRACK 3 DATA" */
	{  12, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 37  "RETRIEVAL REFERENCE NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 38  "AUTHORIZATION IDENTIFICATION RESPONSE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 39  "RESPONSE CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 40  "SERVICE RESTRICTION CODE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 41  "CARD ACCEPTOR TERMINAL IDENTIFICACION" */
	{  15, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 42  "CARD ACCEPTOR IDENTIFICATION CODE"  */
	{ 40, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 43  "CARD ACCEPTOR NAME/LOCATION" */
	{25, LENGTH_TYPE_LLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 44  "ADITIONAL RESPONSE DATA" */
	{ 76, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 45  "TRACK 1 DATA" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 46  "ADITIONAL DATA - ISO" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 47  "ADITIONAL DATA - NATIONAL" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 48  "ADITIONAL DATA - PRIVATE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 49  "CURRENCY CODE, TRANSACTION" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 50  "CURRENCY CODE, SETTLEMENT" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 51  "CURRENCY CODE, CARDHOLDER BILLING"    */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 52  "PIN DATA"    */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 53  "SECURITY RELATED CONTROL INFORMATION" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 54  "ADDITIONAL AMOUNTS" */
	{255, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 55  "RESERVED ISO" */
	{ 999, LENGTH_TYPE_LLLVAR, ALIGN_TYPE_LEFT, '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 56  "RESERVED ISO" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 57  "RESERVED NATIONAL" */
	{255, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 58  "RESERVED NATIONAL" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 59  "RESERVED NATIONAL" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 60  "RESERVED PRIVATE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 61  "RESERVED PRIVATE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 62  "RESERVED PRIVATE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 63  "RESERVED PRIVATE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 64  "MESSAGE AUTHENTICATION CODE FIELD" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBitmapPack, PubFPBBitmapUnPack}, /* 65  "BITMAP, EXTENDED" */
	{  1, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 66  "SETTLEMENT CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 67  "EXTENDED PAYMENT CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 68  "RECEIVING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 69  "SETTLEMENT INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 70  "NETWORK MANAGEMENT INFORMATION CODE" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 71  "MESSAGE NUMBER" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 72  "MESSAGE NUMBER LAST" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 73  "DATE ACTION" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 74  "CREDITS NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 75  "CREDITS REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 76  "DEBITS NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 77  "DEBITS REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 78  "TRANSFER NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 79  "TRANSFER REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 80  "INQUIRIES NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 81  "AUTHORIZATION NUMBER" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 82  "CREDITS, PROCESSING FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 83  "CREDITS, TRANSACTION FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 84  "DEBITS, PROCESSING FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 85  "DEBITS, TRANSACTION FEE AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 86  "CREDITS, AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 87  "CREDITS, REVERSAL AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 88  "DEBITS, AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 89  "DEBITS, REVERSAL AMOUNT" */
	{ 42, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 90  "ORIGINAL DATA ELEMENTS" */
	{  1, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 91  "FILE UPDATE CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 92  "FILE SECURITY CODE" */
	{  5, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 93  "RESPONSE INDICATOR" */
	{  7, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 94  "SERVICE INDICATOR" */
	{ 42, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 95  "REPLACEMENT AMOUNTS" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 96  "MESSAGE SECURITY CODE" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 97  "AMOUNT, NET SETTLEMENT" */
	{ 25, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 98  "PAYEE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 99  "SETTLEMENT INSTITUTION IDENT CODE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 100 "RECEIVING INSTITUTION IDENT CODE" */
	{ 17, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 101 "FILE NAME" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 102 "ACCOUNT IDENTIFICATION 1" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 103 "ACCOUNT IDENTIFICATION 2" */
	{100, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 104 "TRANSACTION DESCRIPTION" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 105 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 106 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 107 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 108 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 109 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 110 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 111 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 112 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 113 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 114 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 115 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 116 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 117 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 118 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 119 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 120 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 121 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 122 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 123 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 124 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 125 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 126 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 127 "RESERVED PRIVATE USE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, 	/* 128 "MAC 2" */
};


/**
* define global variable
*/
static const char gszCode[][64]={       
	{"00Approved"},
	{"01Refer to card issuer."},
	{"02Refer to card issuer.s special conditions"},
	{"03Invalid Merchant."},
	{"04Pick Up Card"},
	{"05Do Not Honour"},
	{"06Error"},
	{"07Pickup card, special condition"},
	{"08Verify id and sign"},
	{"09Accepted"},
	{"10Approved for partial amount"},
	{"11Approved (VIP)"},
	{"12Invalid Transaction"},
	{"13Invalid Amount"},
	{"14Invalid card number"},
	{"15No such Issuer"},
	{"16Approved, update tk 3"},
	{"17Customer cancellation"},
	{"18Customer dispute"},
	{"19Re-enter transaction"},
	{"20Invalid response"},
	{"21No transactions"},
	{"22Suspected malfunction"},
	{"23Unaccepted trans fee"},
	{"24File Upd not Suported"},
	{"25Unable to locate record on file"},
	{"26Dup rec,old rec rplcd"},
	{"27Field edit error"},
	{"28File Locked out"},
	{"29File update error"},
	{"30Format error"},
	{"31Bank Not Supported by Switch"},
	{"32Completed partially"},
	{"33Expired card"},
	{"34Suspected fraud"},
	{"35Contact acquirer"},
	{"36Restricted card"},
	{"37Call acq. Security"},
	{"38PIN tries Exceeded"},
	{"39No credit account"},
	{"40Func. Not Supported"},
	{"41Lost card"},
	{"42No universal account"},
	{"43Stolen card"},
	{"44No investment account"},
	{"45ISO error #45"},
	{"46ISO error #46"},
	{"47ISO error #47"},
	{"48ISO error #48"},
	{"49ISO error #49"},
	{"50ISO error #50"},
	{"51Not sufficient funds"},
	{"52No cheque acc"},
	{"53No savings acc"},
	{"54Expired card"},
	{"55Incorrect PIN"},
	{"56No card record"},
	{"57Txn not Permtd-card"},
	{"58Transaction Not Permitted in Terminal"},
	{"59Suspected fraud"},
	{"60Contact acquirer"},
	{"61Amount limit exceeded"},
	{"62Restricted card"},
	{"63Security violation"},
	{"64Org amount incorrect"},
	{"65Frequency limit exceeded,Pls Try Contact"},//for master MSD
	{"66Call acq's security"},
	{"67Hard capture"},
	{"68Resp Recvd too Late"},
	{"69ISO error #69"},
	{"70ISO error #70"},
	{"71ISO error #71"},
	{"72ISO error #72"},
	{"73ISO error #73 "},
	{"74Transaction declined due to PIN bypass not allowed"},
	{"75PIN tries exceeded"},
	{"76Invalid product codes"},
	{"77Reconcile error"},
	{"78Trace not found"},
	{"79Declined CVV2"},
	{"80Batch number not found"},
	{"81Exceed redp limit"},
	{"82No closed soc slots"},
	{"83No susp. Soc slots"},
	{"84Insufficient pts"},
	{"85Batch not found"},
	{"86Private error #86"},
	{"87Private error #87"},
	{"88Have cm call amex"},
	{"89Bad terminal Id."},
	{"90Cutoff in Process"},
	{"91Issuer or switch inoperative"},
	{"92Transaction can't be routed"},
	{"93Txn cant be Completed"},
	{"94Duplicate transmission"},
	{"95Batch upload started"},
	{"96System malfuntion"},
	{"97Resvd. For Nat. Use"},
	{"98Resvd. For Nat. Use"},
	{"99Terminal data dld"},	
	{"--NO RESPONSE CODE"},
};


/**
* define global variable
*/
static const char gszEDebitCode[][64]={       
	{"24Transaction declined due to wrong contents of DE55"},
	{"25Transaction decline due to wrong cryptogram present in DE55"},
	{"46Transaction declined, switch to other interface"},
	{"50Transaction declined, PIN required"},
	{"46Transaction declined, switch to other interface"},
//	{"74Transaction declined due to PIN bypass not allowed"},
	{"--NO RESPONSE CODE"},
};



static STISODataStru gstISOData;	/**< Iso data*/
static struct STISO_FIELD_LENGTH_CALC gstSetFieldLengthCalc = {PubFLBLengthSet, PubFLBLengthGet};

#define DISP_PACKET_ERR(pszTitle, nFieldNum)\
	char szMsg[64+1];\
	sprintf(szMsg, "[%d] Field Err", nFieldNum);\
	PubMsgDlg(pszTitle, szMsg, 3, 10);\


/**
* @brief Initialize ISO8583 format
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
int InitISO8583(void)
{
	int nRet = 0;	
	nRet = PubImport8583(gstMaybankIso8583Def );
	nRet += PubSetFieldLengthCalc(&gstSetFieldLengthCalc );

	ASSERT_FAIL(nRet);
	return APP_SUCC;
}


/**
* @brief Check MAC
* @param in const char *psBuffer 
* @param in int nDataLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CheckMac(const char *psBuffer,  int nDataLen)
{
#if 0
	char sHashValue[32+1] = {0};
	char sMac[8] = {0};
	
	ASSERT_QUIT(NDK_AlgSHA256((uchar *)psBuffer, nDataLen-8, (uchar *)sHashValue));
	ASSERT_QUIT(ClacDukptMacResponse(sHashValue, 32, sMac));
	if (memcmp(sMac, psBuffer+nDataLen-8, 8) == 0 )
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
#endif
	int nRet, nGroupNo;
	char sCalcMacBuf[8+1]={0};
	char sMacBuf[512]={0}; 
	int nMacLen=0;
	
	GetVarMainKeyNo(&nGroupNo);
	PubSetCurrentMainKeyIndex(nGroupNo);


	nRet = PubCalcMac(MAC_TYPE_X919, sMacBuf, nMacLen, sCalcMacBuf);
	TRACE_HEX((sCalcMacBuf,8,"sCalcMacBuf:"));
	TRACE_HEX(((char *)(psBuffer+nDataLen-8),8,"realmac:"));
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
	if (memcmp( sCalcMacBuf, psBuffer+nDataLen-8, 4) == 0 )
	{
		return APP_SUCC;
	}
	else
	{
		return APP_FAIL;
	}
	
}

/**
* @brief Add MAC in the data tail
* @param in char * psData
* @param  int pnDataLen 
* @param  out pnDataLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int AddMac(char * psData, int *pnDataLen, char cKeyType)
{
#if 0
	char sHashValue[64+1] = {0};
	char sMac[8] = {0};

	ASSERT_QUIT(NDK_AlgSHA256((uchar *)psData, *pnDataLen, (uchar *)sHashValue));
	ASSERT_QUIT(ClacDukptMac(sHashValue, 32, sMac));
		
	memcpy(psData+(*pnDataLen), sMac, 8);
	*pnDataLen += 8;
	return APP_SUCC;
#endif
	int nRet, nGroupNo = 0;

	GetVarMainKeyNo(&nGroupNo);
	PubSetCurrentMainKeyIndex(nGroupNo);

	nRet = PubCalcMac(MAC_TYPE_X919, psData, *pnDataLen, psData+(*pnDataLen));
	if (nRet != APP_SUCC)
	{
		return nRet;
	}
	*pnDataLen += 8;
	return APP_SUCC;
}

#if 0
/**
* @brief Add MAC in the data tail
* @param in char * psData
* @param  int pnDataLen 
* @param  out pnDataLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetMac(char * psData, int *pnDataLen, char *psMac)
{
	char sHashValue[64+1] = {0};
	char sMac[8] = {0};

	ASSERT_QUIT(NDK_AlgSHA256((uchar *)psData, *pnDataLen, (uchar *)sHashValue));
	ASSERT_QUIT(ClacDukptMac(sHashValue, 32, sMac));
		
	memcpy(psMac, sMac, 8);
	return APP_SUCC;
}
#endif


/**
* @brief Clear ISO global buffer
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
int ClrPack()
{
	return PubDelAllBit(&gstISOData);
}

/**
* @brief Set Field data to ISO global buffer
* @param in const int nFieldNum
* @param in const char *psFieldBuffer 
* @param in const int nFieldLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int SetField(const int nFieldNum , const char *psFieldBuffer, const int nFieldLen)
{
	int nRet = 0;

	nRet = PubSetBit (&gstISOData, nFieldNum, (char *)psFieldBuffer, nFieldLen);
	if (nRet != APP_SUCC)
	{
		DISP_PACKET_ERR("SET FIELD", nFieldNum);
	}
	return nRet;
}


/**
* @brief Set Field data to ISO global buffer
* @param in const int nFieldNum
* @param in const char *psFieldBuffer 
* @param in const int nFieldLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int DelField(const int nFieldNum)
{
	int nRet = 0;

	nRet = PubDelOneBit(&gstISOData, nFieldNum);
	if (nRet != APP_SUCC)
	{
		DISP_PACKET_ERR("DEL FIELD", nFieldNum);
	}
	return nRet;
}


/**
* @brief Get Field data from ISO global buffer
* @param in const int nFieldNum
* @param out const char *psFieldBuffer 
* @param out const int *pnFieldLen
* @li APP_SUCC
* @li APP_FAIL
*/
int GetField(const int nFieldNum, char *psFieldBuffer, int *pnFieldLen)
{
	int nRet = 0;

	nRet = PubGetBit (&gstISOData, nFieldNum, psFieldBuffer, pnFieldLen );
	if (nRet != APP_SUCC)
	{
		DISP_PACKET_ERR("UNPACK", nFieldNum);
	}
	return nRet;
}


/**
* @brief Pack ISO global buffer to str
* @param out const char *psPackBuffer 
* @param out const int *pnPackLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int Pack(char *psPackBuffer,  int *pnPackLen )
{
	return PubIsoToStr(psPackBuffer, &gstISOData, pnPackLen );
}	


/**
* @brief Unpack to ISO global buffer
* @param in const char *psPackBuffer
* @param in const int nPackLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int Unpack(const char *psPackBuffer, const int nPackLen)
{
	int nRes = 0, nRet = 0;

	PubDelAllBit(&gstISOData);
	nRet = PubStrToIso((char *)psPackBuffer, nPackLen, &gstISOData, &nRes);
	if (nRet != APP_SUCC)
	{
		PubClearAll();
		PubDisplayGen(1, "UNPACK ERR");
		PubDisplayStrInline(0, 2, "[%d]Field Err",nRes);
		PubUpdateWindow();
		PubGetKeyCode(3);
		return nRet;
	}
	return nRet;
}


/**
* @brief Display Error message
* @param in const char *szRespCode 
* @return ·µ»Ø
*/
void DispResp(const char *pszRespCode, int nAcqIndex)
{
	int i;
	char szContent[100];

	for (i = 0; ; i++)
	{
		if (nAcqIndex == 2)
		{
			if (i < sizeof(gszEDebitCode)/48 && (!memcmp(gszEDebitCode[i], pszRespCode, 2)))
			{
				memset(szContent, 0, sizeof(szContent));
				PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%2.2s-%s", pszRespCode, gszEDebitCode[i]+2);
				PubMsgDlg("WARNING", szContent, 3, 30);
				return;
			}
		}
		if ((!memcmp(gszCode[i], "--", 2)) ||(!memcmp(gszCode[i], pszRespCode, 2)))
		{
			memset(szContent, 0, sizeof(szContent));
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%2.2s-%s", pszRespCode, gszCode[i]+2);
			PubMsgDlg("WARNING", szContent, 3, 30);
			return;
		}
	}
}


/**
* @brief Check Respond isSuccess
* @param [in] IsChkReversal   
* @param [in] pszRespCode   
* @return
* @li APP_FAIL 
* @li APP_SUCC 
*/
int ChkRespIsSucc(YESORNO IsChkReversal,const char *pszRespCode, int nAcqIndex)
{
	if (memcmp(pszRespCode, "00", 2) != 0)
	{
		if (IsChkReversal  == YES)
		{
			SetVarIsReversal(NO);
		}
		if (memcmp(pszRespCode, "LE", 2) == 0)
		{
			DispRespEFTInfo();
		}
		else
		{
			DispResp(pszRespCode, nAcqIndex);
		}
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Do Pack and Communicaiton 
* @detail [Pack],[SEND],[RECV][UNPACK]
* @param [in] cOperFlag  see [ENM_DEALPACKCOMM]
* @param [out] pstSystem 
* @param [out] pstReversal  
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author lg
* @date 2013-3-14
*/
int DealPackAndComm(char* pszTitle, ENM_DEALPACKCOMM cOperFlag,STSYSTEM *pstSystem,STREVERSAL *pstReversal,int nInputPinNum)
{
	int nRet = 0, nPackLen = 0, nFieldLen = 0;
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char szContent[200]= {0};
	char szMsgID[4+1] = {0};
	char sMac[8] = {0};
	
#ifdef DEMO
	PubCommClear();
	
	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Sending...");
	PubUpdateWindow();
	NDK_SysDelay(5);
	PubClear2To4();
	PubDisplayStrInline(DISPLAY_MODE_CENTER, 3, "Recving...");
	PubUpdateWindow();
	NDK_SysDelay(10);
	return APP_SUCC;
#endif
	nFieldLen = 4;
	GetField(0, szMsgID, &nFieldLen);
	/**
	*Pack
	*/
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
	nPackLen -= 8;
PubDebugData("before mac: ", sPackBuf, nPackLen);
	GetMac(sPackBuf, &nPackLen, sMac);
TRACE(("====refund===="));
	ASSERT_HANGUP_FAIL(ProcessTLED(pstSystem));
TRACE(("====refund===="));
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
TRACE(("====refund===="));
	memcpy(sPackBuf+nPackLen-8, sMac, 8);

//	ASSERT_HANGUP_FAIL(PackTLEDAndMAC(sPackBuf, &nPackLen, pstSystem));
		
	if ((cOperFlag & DPC_SETREVERSAL) == DPC_SETREVERSAL)
	{
		SetVarReversalData((char *)pstReversal, sizeof(STREVERSAL));
		SetVarIsReversal(YES);
	}

	IncVarTraceNo();
//	IncVarInvoiceNo();
TRACE(("====refund===="));
	nRet = CommSend(sPackBuf, nPackLen);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		return nRet;
	}
TRACE(("====refund===="));
	nRet = CommRecv(sPackBuf, &nPackLen);
TRACE(("====refund===="));
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		return nRet;
	}

	if ((cOperFlag & DPC_SETREVERSAL) == DPC_SETREVERSAL)
	{
		strcpy(pstReversal->szResponse, "06");
		SetVarReversalData((char *)pstReversal, sizeof(STREVERSAL));
	}

	CommHangUpSocket();

	ASSERT_HANGUP_FAIL(Unpack(sPackBuf, nPackLen));
	ASSERT_HANGUP_FAIL(ChkRespMsgID(szMsgID, sPackBuf));
	ASSERT_HANGUP_FAIL(ChkRespon(pstSystem, sPackBuf + 2));

	//decrypt sentitive data
	if (APP_SUCC == ParseTLED(pstSystem))
	{
		ASSERT_FAIL(DelField(57));
		
		ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
PubDebugData("mac repack data: ", sPackBuf,nPackLen );
		
	}
	
	if (pstSystem->cMustChkMAC == 0x01)
	{
		nRet = CheckMac(sPackBuf, nPackLen);
		ASSERT(nRet);
PubDebug("==========mac result: %d", nRet);		
		nRet = APP_SUCC;
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			if ((cOperFlag & DPC_SETREVERSAL) == DPC_SETREVERSAL)
			{
				if (memcmp(pstSystem->szInputMode, "96", 2) != 0)
				{
					memset(pstReversal->szFieldAdd1, 0, sizeof(pstReversal->szFieldAdd1));
					pstReversal->nFieldAdd1Len = 0;
					EmvPackField55(EMV_REVERSAL, pstReversal->szFieldAdd1, &(pstReversal->nFieldAdd1Len), pstReversal->nAcqIndex);
				}

				if (TRANS_VOID_PREAUTH == pstSystem->cTransType
					|| TRANS_AUTHCOMP== pstSystem->cTransType
					|| TRANS_VOID_AUTHSALE == pstSystem->cTransType)
				{
					memcpy(pstReversal->stField62.sOldDate, pstSystem->szDate, 4);
				}
							
				strcpy(pstReversal->szResponse, "A0");
				SetVarReversalData((char *)pstReversal, sizeof(STREVERSAL));
			}
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "\n|CCheck MAC Error");
			PubMsgDlg(pszTitle, szContent, 3, 10);
			return APP_FAIL;
		}
	}

	ASSERT_FAIL(IncreaseKSN());
	
#if 1
	if((memcmp("50",pstSystem->szResponse,2) == 0) && (nInputPinNum < 3))
	{
		if ((cOperFlag & DPC_SETREVERSAL) == DPC_SETREVERSAL)
		{
			SetVarIsReversal(NO);
		}
		return APP_REPIN;
	}
#endif	
	PubClear2To4();
	if ((cOperFlag & DPC_NOCHECKRESP) == DPC_NOCHECKRESP)
	{
		return APP_SUCC;
	}
	
	if ((cOperFlag & DPC_SETREVERSAL) == DPC_SETREVERSAL)
	{
		nRet = ChkRespIsSucc(YES,pstSystem->szResponse,pstSystem->nAcqIndex);
	}
	else
	{
		nRet = ChkRespIsSucc(NO,pstSystem->szResponse,pstSystem->nAcqIndex);
	}
	if(nRet != APP_SUCC)
	{
		CommHangUp();
		return APP_FAIL;
	}

	if((cOperFlag & DPC_EMV) == DPC_EMV)
	{

	}
	return APP_SUCC;
}

/**
* @brief Trade Init
* @detail [Check oper],[Check switch],[Check login][Check limit]
* @param [in] pszTitle  
* @param [in] cOperFlag  
* @param [in] cTransType  
* @param [in] cAttr 
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author
* @date 2014-9-14
*/
int TradeInit(char* pszTitle, ENM_TRADEINIT cOperFlag, const char cTransType, const char cAttr, const char cClearScr)
{
	if(cOperFlag & TDI_CHECKOPER)
	{
		if(GetVarIsVoidPwd()==YES)
		{
			ASSERT_QUIT(ProCheckPwd(pszTitle, EM_TRANS_PWD));
		}
	}
	
	if (YES == cClearScr)
	{
		PubClearAll();
		PubDisplayTitle(pszTitle);
	}
	
	/**
	* Check switch
	*/
	ASSERT_QUIT(ChkTransOnOffStatus(cTransType));

	/**
	* Check if it has login
	*/
	ASSERT_QUIT(ChkLoginStatus());

	/**
	* check limit
	*/
	ASSERT_QUIT(DealPosLimit());

	return APP_SUCC;
}

/**
* @brief Trace Complete
* @detail [Save water],[Cleat reversal],[Print][Send TC]
* @param [in] pszTitle  
* @param [in] cOperFlag  
* @param [in] pstSystem 
* @param [in] pstTransRecord 
* @param [in] pszInvno 
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author lg
* @date 2014-9-14
*/
int TradeComplete(char* pszTitle, ENM_TRADECOMPLETE cOperFlag, const STSYSTEM *pstSystem, STTAANSRECORD *pstTransRecord, const char *pszInvno)
{
	int nRet;
	STTAANSRECORD stTransRecordOld;
	STREVERSAL stReversal;	
	int nLen = 0; 
	
	nRet = AppendTransRecord(pstTransRecord);
	if (nRet != APP_SUCC)
	{
	    GetVarReversalData((char *)&stReversal, &nLen);
		strcpy(stReversal.szResponse, "96");
		SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
		CommHangUp();
		return APP_FAIL;
	}

	if(!(cOperFlag & TDC_UNSETREVERSAL))
	{
		SetVarIsReversal(NO);
		memset(&stReversal, 0, sizeof(STREVERSAL));
		SetVarReversalData((char *)&stReversal, sizeof(STREVERSAL));
	}
	if(pszInvno != NULL)
	{
		/**
		* update old water
		*/
		memset(&stTransRecordOld, 0, sizeof(STTAANSRECORD));
		FindRecordWithInvno(pszInvno, &stTransRecordOld);
		stTransRecordOld.cStatus = 0x01;
		UpdateTransRecord(&stTransRecordOld);
	}

	ChangeSettle(pstSystem);

	PubClearAll();
	PubDisplayTitle(pszTitle);
	DISP_TRANS_SUCC;
	DISP_PRINTING_NOW;
	
	IncVarInvoiceNo();
	PrintWater(pstTransRecord, FIRSTPRINT);

	EmvSendTC();
	SendOffline(0);
	CommHangUp();

	return APP_SUCC;
}




int AddMccsDE55(const char *psF55, int nLen, char *psOut, int *pnLen)
{
	
	psOut[0] = 05;
	psOut[1] = nLen;

	memcpy(psOut+2, psF55, nLen);

	*pnLen = nLen + 2;

	return APP_SUCC;
	
}

/*
General Pack
*/
int PackGeneral(STSYSTEM *pstSystem, STTAANSRECORD *pstTransRecord)
{
	char szTotalAmt[13] = {0};
	char sTmpAuchCode[6+1] = {0};
	
	/**
	* Begin to pack
	*/
	ClrPack();
	
	ASSERT_HANGUP_FAIL(SetField(0, pstSystem->szMsgID, 4));
	if (YES == IsNeedToSendDE2(pstSystem))
	{
		if(strlen(pstSystem->szPan) > 0)
		{
			ASSERT_HANGUP_FAIL(SetField(2, pstSystem->szPan, strlen(pstSystem->szPan)));
		}
	}

	if (pstSystem->nAcqIndex == 1)
	{
		pstSystem->szProcCode[2] = '4';
		if (pstSystem->cTransType == TRANS_OFFLINE)
		{
			pstSystem->szProcCode[5] = '1';
		}
	}
	ASSERT_HANGUP_FAIL(SetField(3, pstSystem->szProcCode, 6));
	if (1 == pstSystem->cFlagCashback)
	{
		AmtAddAmt((uchar *)pstSystem->szAmount, (uchar *)pstSystem->szCashbackAmount, (uchar *)szTotalAmt);
		ASSERT_HANGUP_FAIL(SetField(4, szTotalAmt, 12));
	}
	else
	{
		ASSERT_HANGUP_FAIL(SetField(4, pstSystem->szAmount, 12));
	}
	ASSERT_HANGUP_FAIL(SetField(11, pstSystem->szTrace, 6));

	if (memcmp(pstSystem->szMsgID, "0320", 4) == 0
		|| memcmp(pstSystem->szMsgID, "0220", 4) == 0)
	{
		PubHexToAsc((uchar *)pstTransRecord->sTime, 6, 0, (uchar *)pstSystem->szTime);					
		PubHexToAsc((uchar *)pstTransRecord->sDate, 4, 0, (uchar *)pstSystem->szDate);				
		ASSERT_FAIL(SetField(12, pstSystem->szTime, 6));
		ASSERT_FAIL(SetField(13, pstSystem->szDate, 4));
	}

	if (YES == IsNeedToSendDE2(pstSystem))
	{
		if (ChkDigitStr(pstSystem->szExpDate, 4) == APP_SUCC)
		{
			ASSERT_HANGUP_FAIL(SetField(14, pstSystem->szExpDate, 4));
		}
	}
//	pstSystem->szInputMode[2] = '1';//force requirement
	if (pstSystem->nAcqIndex == 1)
	{
		if (pstSystem->cTransType == TRANS_OFFLINE)
			pstSystem->szInputMode[2] = '0';
	}
	ASSERT_HANGUP_FAIL(SetField(22, pstSystem->szInputMode, 3));

PubDebugData("*************SN: ", pstSystem->szCardSerialNo, 3);
	if (memcmp(pstSystem->szCardSerialNo, "\x00\x00\x00", 3) != 0)
	{
		if (pstSystem->nAcqIndex != 1 && pstSystem->cTransAttr != ATTR_MSD)
		{
			if (strlen(pstSystem->szCardSerialNo) != 3)
			{
				PubAddSymbolToStr(pstSystem->szCardSerialNo, 3, '0', ADDCH_MODE_BEFORE_STRING);	
			}
			ASSERT_FAIL(SetField(23, pstSystem->szCardSerialNo, 3));
		}
	}

	ASSERT_HANGUP_FAIL(SetField(24, pstSystem->szNii, 3));
	ASSERT_HANGUP_FAIL(SetField(25, pstSystem->szServerCode, 2));

PubDebug("1-------------%d", strlen(pstSystem->szTrack2));
	if (YES == IsNeedToSendDE35(pstSystem))
	{
PubDebug("2-------------%d", strlen(pstSystem->szTrack2));
		if (strlen(pstSystem->szTrack2) > MIN_TK2 && strlen(pstSystem->szTrack2) <= MAX_TK2)
		{
			ASSERT_HANGUP_FAIL(SetField(35, pstSystem->szTrack2, strlen(pstSystem->szTrack2)));
		}
#if 1
		if (pstSystem->cTransAttr == ATTR_MSD)
		{
			if (strlen(pstSystem->szTrack1) > MIN_TK1 && strlen(pstSystem->szTrack1) <= MAX_TK1)
			{
				ASSERT_HANGUP_FAIL(SetField(45, pstSystem->szTrack1, strlen(pstSystem->szTrack1)));
			} 
		}
#endif
	}

	if(strlen(pstSystem->szRefnum) > 0)
	{
		ASSERT_FAIL(SetField(37, pstSystem->szRefnum, 12));
	}

#if 1
	if(strlen(pstSystem->szAuthCode) > 0 && (memcmp(pstSystem->szMsgID, "0320", 4) == 0
		|| memcmp(pstSystem->szMsgID, "0220", 4) == 0))
	{
		if (strlen(pstSystem->szAuthCode) < 6)
		{
			PubAddSymbolToStr(pstSystem->szAuthCode, 6, ' ', 1);
		}
		ASSERT_FAIL(SetField(38, pstSystem->szAuthCode, 6));
	}
#endif

	if(memcmp(pstSystem->szMsgID, "0320", 4) == 0
		|| memcmp(pstSystem->szMsgID, "0220", 4) == 0)
	{
		if (strlen(pstSystem->szResponse) > 0)//offline no resonse code
		{
			ASSERT_FAIL(SetField(39, pstSystem->szResponse, 2));
		//	strcpy(pstSystem->szResponse, "00");
		}
	}
	
	ASSERT_HANGUP_FAIL(SetField(41, pstSystem->szPosID, 8));
	ASSERT_HANGUP_FAIL(SetField(42, pstSystem->szShopID, 15));

	if (pstSystem->cTransAttr == ATTR_MANUAL || pstSystem->cTransAttr == ATTR_MAG)
	{
		if (strlen(pstSystem->szCVV2) > 0 && pstSystem->nAcqIndex != 1)
		{
			ASSERT_HANGUP_FAIL(SetField(48, pstSystem->szCVV2, strlen(pstSystem->szCVV2)));
		}
	}

	if (memcmp(pstSystem->szPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) != 0)
	{
		ASSERT_HANGUP_FAIL(SetField(52, pstSystem->szPin, 8));
	}
		
	if(pstSystem->cFlagCashback == 1)
	{
		ASSERT_FAIL(SetField(54, pstSystem->szCashbackAmount, 12));
	}
	else if (pstSystem->cTipFlag == 1)
	{
		ASSERT_FAIL(SetField(54, pstSystem->szTipAmount, 12));
	}

	if (pstSystem->nAddFieldLen >0)
	{
	 	if (pstSystem->cTransAttr != ATTR_MSD)//add 20170511 
		{
			//for AMEX upload no send DE55  ----  TC
			if ((memcmp(pstSystem->szMsgID, "0320", 4) == 0) && (memcmp(pstSystem->szProcCode, "940000", 6) != 0))
			{
				if (pstSystem->nAcqIndex != 1)
				{
					ASSERT_FAIL(SetField(55, pstSystem->sField55, pstSystem->nAddFieldLen));
				}
			}
			else 
			{
				ASSERT_FAIL(SetField(55, pstSystem->sField55, pstSystem->nAddFieldLen));
			}
	 	}
	}
	else
	{
		if (pstSystem->cTransAttr == ATTR_MANUAL || pstSystem->cTransAttr == ATTR_MAG)
		{
			if (strlen(pstSystem->szCVV2) > 0 && pstSystem->nAcqIndex == 1)// 4DBC
			{
				pstSystem->nAddFieldLen = strlen(pstSystem->szCVV2);
				memcpy(pstSystem->sField55, pstSystem->szCVV2, pstSystem->nAddFieldLen);
				ASSERT_FAIL(SetField(55, pstSystem->sField55, pstSystem->nAddFieldLen));
			}
		}
	}
	
	//upload
	if ((memcmp(pstSystem->szMsgID, "0320", 4) == 0) && (memcmp(pstSystem->szProcCode, "940000", 6) != 0))
	{
		char sTmp[100] = {0};
		
		memcpy(pstSystem->stField60.sOldMTI, pstTransRecord->szMsgId, 4);
		PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)sTmp);	
		memcpy(pstSystem->stField60.sOldSTAN, sTmp, 6);
		memcpy(pstSystem->stField60.sOldRRN, pstSystem->szRefnum, 12);
		ASSERT_HANGUP_FAIL(SetField(60, (char *)&pstSystem->stField60, 22));
		
	}

	if (TRANS_INSTALLMENT == pstSystem->cTransType || TRANS_VOID_INSTALLMENT == pstSystem->cTransType)
	{
		char szDE61[100] = {0};

		if ((memcmp(pstSystem->szMsgID, "0320", 4) == 0) && (memcmp(pstSystem->szProcCode, "940000", 6) != 0))
		{
			ASSERT_HANGUP_FAIL(SetField(61, pstSystem->szInstallmentInfo, strlen(pstSystem->szInstallmentInfo)));
		}
		else
		{
			sprintf(szDE61, "001%02d", pstSystem->nInstallmentMonth);
			ASSERT_HANGUP_FAIL(SetField(61, szDE61, strlen(szDE61)));
		}

	}

	if (memcmp(pstSystem->szMsgID, "0320", 4) == 0
		|| memcmp(pstSystem->szMsgID, "0220", 4) == 0)
	{
PubDebug("water====%s", pstTransRecord->szInvoice);
		ASSERT_HANGUP_FAIL(SetField(62, pstTransRecord->szInvoice, strlen(pstTransRecord->szInvoice)));
	}
	else
	{
PubDebug("pstSystem====%s", pstSystem->szInvoice);
		ASSERT_HANGUP_FAIL(SetField(62, pstSystem->szInvoice, strlen(pstSystem->szInvoice)));
	}

	
	ASSERT_HANGUP_FAIL(SetField(64, "\x00\x00\x00\x00\x00\x00\x00\x00", 8));

	//for DCC 
	if (1 == pstSystem->cIsUseDcc)
	{
		ASSERT_HANGUP_FAIL(SetField(4, pstSystem->szLocalAmount, 12));
		ASSERT_HANGUP_FAIL(SetField(49, COUNTRYCODE, 3));
		ASSERT_HANGUP_FAIL(SetField(50, pstSystem->szDccCurrencyCode, strlen(pstSystem->szDccCurrencyCode)));
		//void no need 
		if (YES == IsNeedToSendDE63(pstSystem) && pstSystem->nField63Len > 0)
		{
			ASSERT_HANGUP_FAIL(SetField(63, pstSystem->szField63, pstSystem->nField63Len));
		}
	}

	return APP_SUCC;
	
}

YESORNO IsNeedToSendDE2(STSYSTEM *pstSystem)
{
	if (memcmp(pstSystem->szMsgID, "0400", 4) == 0
		|| (memcmp(pstSystem->szMsgID, "0320", 4) == 0)
			|| (memcmp(pstSystem->szMsgID, "0220", 4) == 0))
	{
		return YES;
	}
	if (pstSystem->cTransAttr == ATTR_MANUAL)
		//|| pstSystem->cTransAttr == ATTR_MSD)
	{
		return YES;		
	}
	return NO;
}


YESORNO IsNeedToSendDE35(STSYSTEM *pstSystem)
{
	if (YES != IsNeedToSendDE2(pstSystem)
		&& (pstSystem->cTransAttr != ATTR_MANUAL))
		return YES;

	return NO;
}

YESORNO IsNeedToSendDE63(STSYSTEM *pstSystem)
{
	if (pstSystem->cIsUseDcc != 1)
	{
		return NO;
	}
	if (TRANS_VOID == pstSystem->cTransType)
	{
		return NO;
	}
	if ((memcmp(pstSystem->szMsgID, "0320", 4) == 0) && (memcmp(pstSystem->szProcCode, "940000", 6) != 0))
	{
		return NO;
	}
	return YES;
}

int SslRepack(char *psBuf, int *pnLen)
{
#if 0
	char sTmp[2048] = {0};

	//add gprs head
	memcpy(sTmp, psBuf, *pnLen);	
	PubIntToC2(psBuf, *pnLen);
	memcpy(psBuf+2, sTmp, *pnLen);
	*pnLen+=2;
	
	//add include head
	memcpy(sTmp, psBuf, *pnLen);
	PubIntToC2(psBuf, *pnLen+2);
	memcpy(psBuf+2, sTmp, *pnLen);
	*pnLen += 2;
#endif	
	return APP_SUCC;
}

int GetSignValue(char *pszTimestamp, char *pszRequestBody, char * pszSign, char cTransType)
{
	char szBuff[MAX_PACK_SIZE] = {0};
	char szKey[32+1] = {0};
	unsigned char szMd5Hex[16 + 1] = {0}; 
	unsigned char uszMd5Ascii[32 + 1] = {0};
	char szUtf8Value[256]= {0};
	int nLen;
	char szData[1024] = {0};
	char sSign[999] = {0};

	strcpy(szData, pszTimestamp);
	if (cTransType != TRANS_QUERY)
	{
		strcat(szData, pszRequestBody);
	}
TRACE(("szData=%s", szData));
	GetRsaValue(szData, strlen(szData), sSign, &nLen);
	base64_encode(sSign, nLen, pszSign);
TRACE(("[Sign]:[%s]", pszSign));	


	return APP_SUCC;
}

