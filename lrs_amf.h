#ifndef __LRS_AMF__H__
#define __LRS_AMF__H__

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifdef _MSC_VER
#ifndef _STDINT
typedef __int32             int32_t;
typedef unsigned __int32    uint32_t;
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
#endif
#else
#include <stdint.h>
#endif

#define AMF3_INTEGER_MAX	268435455
#define AMF3_INTEGER_MIN	-268435456

typedef enum
{ 	
	AMF_NUMBER 			= 0, 
	AMF_BOOLEAN 		= 1, 
	AMF_STRING 			= 2, 
	AMF_OBJECT 			= 3,
	AMF_MOVIECLIP 		= 4,	/* reserved, not used */
	AMF_NULL 			= 5, 
	AMF_UNDEFINED 		= 6, 
	AMF_REFERENCE 		= 7, 
	AMF_ECMA_ARRAY 		= 8, 
	AMF_OBJECT_END 		= 9,
	AMF_STRICT_ARRAY 	= 10, 
	AMF_DATE 			= 11, 
	AMF_LONG_STRING 	= 12, 
	AMF_UNSUPPORTED 	= 13,
	AMF_RECORDSET 		= 14,	/* reserved, not used */
	AMF_XML_DOC 		= 15, 
	AMF_TYPED_OBJECT 	= 16,
	AMF_AVMPLUS 		= 17,	/* switch to AMF3 */
	AMF_INVALID 		= 0xff
} AMFDataType;

typedef enum
{ 
	AMF3_UNDEFINED 		= 0, 
	AMF3_NULL			= 1, 
	AMF3_FALSE			= 2, 
	AMF3_TRUE			= 3,
	AMF3_INTEGER		= 4, 
	AMF3_DOUBLE			= 5, 
	AMF3_STRING			= 6, 
	AMF3_XML_DOC		= 7, 
	AMF3_DATE			= 8,
	AMF3_ARRAY			= 9, 
	AMF3_OBJECT			= 10, 
	AMF3_XML			= 11, 
	AMF3_BYTE_ARRAY		= 12
} AMF3DataType;


/* Data is Big-Endian  大端数据存放格式 */

/*
	string 类型     	AMF_STRING/AMF_LONG_STRING + len + data
	number 类型		AMF_NUMBER + data
	bool   类型		AMF_BOOLEAN + data(0x01 : 0x00)
*/

typedef struct AVal
{
    char *av_val;
    int av_len;
} AVal;

#define AVC(str) \
    {str, sizeof(str)-1}

#define AVMATCH(a1,a2) \
    (((a1)->av_len == (a2)->av_len) && !memcmp((a1)->av_val,(a2)->av_val,(a1)->av_len))

struct AMFObject;

typedef struct AMFObjectProperty
{
	AVal p_name;                //name
    
	AMFDataType p_type;         //type
	union
	{
		double p_number;
		AVal p_aval;
		AMFObject p_object;
	} p_vu;                     //value
	
	int16_t p_UTCoffset;
} AMFObjectProperty;

typedef struct AMFObject
{
	int o_num;
	
	struct AMFObjectProperty *o_props;
} AMFObject;

typedef struct AMF3ClassDef
{
	AVal cd_name;
	char cd_externalizable;
	char cd_dynamic;
	int  cd_num;
	AVal *cd_props;
} AMF3ClassDef;

/*
**
**  API function
**
*/

typedef uint32_t (*lrs_decoder_int24)(struct LRS_BUFFER*);
typedef uint32_t (*lrs_decode_int32)(struct LRS_BUFFER*);
typedef int (*lrs_encoder_int24)(struct LRS_BUFFER*, int);
typedef int (*lrs_encode_int32)(struct LRS_BUFFER*, int);

typedef int (*amf_encoder_func)(LRS_AMF*, struct LRS_BUFFER*);
typedef int (*amf_decoder_func)(AMFObject*, const char*, int, int);
typedef void (*amf_reset_func)(AMFObject*);

typedef struct LRS_AMF{
    AMFObject       *amf;
    amf_encoder_func amf_encoder;
    amf_decoder_func amf_decoder;
    amf_reset_func   amf_reset;

    lrs_decoder_int24 dint24;
    lrs_decode_int32 dint32;
    lrs_encoder_int24 eint24;
    lrs_encode_int32 eint32;
    
} LRS_AMF;


struct LRS_AMF* amf_alloc();
void amf_free(struct LRS_AMF* amf);

/*
**
**  AMF --- Encoder
**
*/
char *AMF_Encode_Int16(char *output, char *outend, short nVal);
char *AMF_Encode_Int24(char *output, char *outend, int nVal);
char *AMF_Encode_Int32(char *output, char *outend, int nVal);


char *AMF_Encode_Number(char *output, char *outend, double dVal);
char *AMF_Encode_Boolean(char *output, char *outend, int bVal);
char *AMF_Encode_String(char *output, char *outend, const AVal *str);
char *AMF_Encode_Null(char *output, char *outend);
char *AMF_Encode_Object_Start(char *output, char *outend);
char *AMF_Encode_Array_Start(char *output, char *outend);
char *AMF_Encode_Ecma_Array_Start(char *output, char *outend);
char *AMF_Encode_Object_End(char *output, char *outend);

/*
    key --- value

    key : length + data(string)

    value : Number / Boolean / String / Object / Null 
*/
char *AMF_Encode_Key_String(char *output, char *outend, const AVal *strName);
char *AMF_Encode_key_Value_Number(char *output, char *outend, const AVal *strName, double dVal);
char *AMF_Encode_key_Value_Boolean(char *output, char *outend, const AVal *strName, int bVal);
char *AMF_Encode_key_Value_String(char *output, char *outend, const AVal *strName, const AVal *strValue);

/*
    Object - property
*/
char *AMF_Encode_Object_Property(AMFObjectProperty *prop, char *pBuffer, char *pBufEnd);
char *AMF_Encode_Object(AMFObject * obj, char *pBuffer, char *pBufEnd);
char *AMF_Encode_Array(AMFObject *obj, char *pBuffer, char *pBufEnd);
char *AMF_Encode_Ecma_Array(AMFObject *obj, char *pBuffer, char *pBufEnd);

/*
**
**  AMF --- Property
**
*/
AMFDataType AMF_Property_GetType(AMFObjectProperty * prop);
void AMF_Property_SetNumber(AMFObjectProperty * prop, double dval);
void AMF_Property_SetBoolean(AMFObjectProperty * prop, int bflag);
void AMF_Property_SetString(AMFObjectProperty * prop, AVal * str);
void AMF_Property_SetObject(AMFObjectProperty * prop, AMFObject * obj);

void AMF_Property_GetName(AMFObjectProperty * prop, AVal * name);
void AMF_Property_SetName(AMFObjectProperty * prop, AVal * name);
double AMF_Property_GetNumber(AMFObjectProperty * prop);
int AMF_Property_GetBoolean(AMFObjectProperty * prop);
void AMF_Property_GetString(AMFObjectProperty * prop, AVal * str);
void AMF_Property_GetObject(AMFObjectProperty * prop, AMFObject * obj);

int AMF_Property_IsValid(AMFObjectProperty * prop);

/*
**
**  AMF --- Decoder
**
*/
uint16_t AMF_Decode_Int16(const char *data);
uint32_t AMF_Decode_Int24(const char *data);
uint32_t AMF_Decode_Int32(const char *data);
double AMF_Decode_Number(const char *data);
int AMF_Decode_Boolean(const char *data);
void AMF_Decode_String(const char *data, AVal *strValue);
void AMF_Decode_Long_String(const char *data, AVal *strValue);

void AMF_Object_Add_Property(AMFObject *obj, const AMFObjectProperty *prop);
int AMF_Object_Count_Property(AMFObject *obj);
AMFObjectProperty *AMF_Object_Get_Property(AMFObject * obj, const AVal * name, int nIndex);
void AMF_Object_Property_Reset(AMFObjectProperty *prop);


int AMF_Decode_Object(AMFObject *obj, const char *pBuffer, int nSize, int bDecodeName);
int AMF_Decode_Object_Property(AMFObjectProperty *prop, const char *pBuffer, int nSize, int bDecodeName);
int AMF_Decode_Array(AMFObject *obj, const char *pBuffer, int nSize, int nArrayLen, int bDecodeName);
void AMF_Object_Reset(AMFObject * obj);


int AMF3_Read_Integer(const char *data, int32_t *valp);
int AMF3_Read_String(const char *data, AVal *str);
void AMF3_CD_Object_Add_Property(AMF3ClassDef *cd, AVal *prop);
AVal *AMF3_CD_Object_Get_Property(AMF3ClassDef *cd, int nIndex);

int AMF3_Decode_Object(AMFObject *obj, const char *pBuffer, int nSize, int bAMFData);
int AMF3_Decode_Object_Property(AMFObjectProperty *prop, const char *pBuffer, int nSize, int bDecodeName);

#endif