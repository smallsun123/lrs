#include "LRS_amf.h"

static const AMFObjectProperty AMFProp_Invalid = { {0, 0}, AMF_INVALID };
static const AMFObject AMFObj_Invalid = { 0, 0 };
static const AVal AV_empty = { 0, 0 };

//Big-Endian
char *AMF_Encode_Int16(char *output, char *outend, short nVal)
{
    if (output + 2 > outend)
        return NULL;

    *output++ = (nVal >> 8) & 0xff;
    *output++ = nVal & 0xff;
    
    return output;
}

//Big-Endian
char *AMF_Encode_Int24(char *output, char *outend, int nVal)
{
    if (output + 3 > outend)
        return NULL;

    *output++ = (nVal >> 16) & 0xff;
    *output++ = (nVal >> 8) & 0xff;
    *output++ = nVal & 0xff;
    
    return output;
}

static int lrs_encoder_int24(struct LRS_BUFFER *buffer, int nVal)
{
    *buffer->pwrite++ = (nVal >> 16) & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    *buffer->pwrite++ = (nVal >> 8) & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    *buffer->pwrite++ = nVal & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    
    return 0;
}

//Big-Endian
char *AMF_Encode_Int32(char *output, char *outend, int nVal)
{
    if (output + 4 > outend)
        return NULL;

    *output++ = (nVal >> 24) & 0xff;
    *output++ = (nVal >> 16) & 0xff;
    *output++ = (nVal >> 8) & 0xff;
    *output++ = nVal & 0xff;
    
    return output;
}

static int lrs_encode_int32(struct LRS_BUFFER *buffer, int nVal)
{
    *buffer->pwrite++ = (nVal >> 24) & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    *buffer->pwrite++ = (nVal >> 16) & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    *buffer->pwrite++ = (nVal >> 8) & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    *buffer->pwrite++ = nVal & 0xff;
    if(buffer->pwrite == buffer->end){
        buffer->pwrite = buffer->start;
    }
    
    return 0;
}

/*
    number == type + data
*/
char *AMF_Encode_Number(char *output, char *outend, double dVal)
{
    if (output+1+8 > outend)
		return NULL;

    *output++ = AMF_NUMBER;	        /* type: Number */

#if __FLOAT_WORD_ORDER == __BYTE_ORDER
{
    #if __BYTE_ORDER == __BIG_ENDIAN
    {
    	memcpy(output, &dVal, 8);
    }
    #elif __BYTE_ORDER == __LITTLE_ENDIAN
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
    #endif
}
#else
{
    #if __BYTE_ORDER == __LITTLE_ENDIAN
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[3];
		co[1] = ci[2];
		co[2] = ci[1];
		co[3] = ci[0];
		co[4] = ci[7];
		co[5] = ci[6];
		co[6] = ci[5];
		co[7] = ci[4];
	}
    #else
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[4];
		co[1] = ci[5];
		co[2] = ci[6];
		co[3] = ci[7];
		co[4] = ci[0];
		co[5] = ci[1];
		co[6] = ci[2];
		co[7] = ci[3];
	}
    #endif
}
#endif

	return output+8;
}


/*
    boolean == type + data
*/
char *AMF_Encode_Boolean(char *output, char *outend, int bVal)
{
    if (output+2 > outend)
		return NULL;

	*output++ = AMF_BOOLEAN;
	*output++ = bVal ? 0x01 : 0x00;

	return output;
}


/*
    string == type + lenght + value
*/
char *AMF_Encode_String(char *output, char *outend, const AVal *str)
{
    if ((str->av_len < 65536 && output + 1 + 2 + str->av_len > outend) ||
		output + 1 + 4 + str->av_len > outend)
		return NULL;

	if (str->av_len < 65536)
	{
		*output++ = AMF_STRING;	                                    // type
		output = AMF_Encode_Int16(output, outend, str->av_len);	    // lenght
	}
	else
	{
		*output++ = AMF_LONG_STRING;
		output = AMF_Encode_Int32(output, outend, str->av_len);
	}
	memcpy(output, str->av_val, str->av_len);		                // value
	output += str->av_len;

	return output;
}


/*
    null == type (AMF_NULL)
*/
char *AMF_Encode_Null(char *output, char *outend)
{
    if (output+1 > outend)
		return NULL;

	*output++ = AMF_NULL;
	return output;
}


/*
    object-start == type (AMF_OBJECT)
*/
char *AMF_Encode_Object_Start(char *output, char *outend)
{
    if (output+1 > outend)
		return NULL;

	*output++ = AMF_OBJECT;
	return output;
}

char *AMF_Encode_Array_Start(char *output, char *outend)
{
    if (output+1 > outend)
		return NULL;

	*output++ = AMF_STRICT_ARRAY;
	return output;
}

char *AMF_Encode_Ecma_Array_Start(char *output, char *outend)
{
    if (output+1 > outend)
		return NULL;

	*output++ = AMF_ECMA_ARRAY;
	return output;
}


/*
    object-end == type (AMF_OBJECT_END)
*/
char *AMF_Encode_Object_End(char *output, char *outend)
{
    if (output+3 > outend)
		return NULL;

    *output++ = 0x00;
    *output++ = 0x00;
	*output++ = AMF_OBJECT_END;
	return output;
}


/*
    key == lenght + data (string)
*/
char *AMF_Encode_Key_String(char *output, char *outend, const AVal *strName)
{
    if (output+2+strName->av_len > outend)
		return NULL;
    
	output = AMF_Encode_Int16(output, outend, strName->av_len);

	memcpy(output, strName->av_val, strName->av_len);
	output += strName->av_len;

	return output;
}

char *AMF_Encode_key_Value_Number(char *output, char *outend, const AVal *strName, double dVal)
{
    output = AMF_Encode_Key_String(output, outend, strName);
    return AMF_Encode_Number(output, outend, dVal);
}

char *AMF_Encode_key_Value_Boolean(char *output, char *outend, const AVal *strName, int bVal)
{
    output = AMF_Encode_Key_String(output, outend, strName);
    return AMF_Encode_Boolean(output, outend, bVal);
}

char *AMF_Encode_key_Value_String(char *output, char *outend, const AVal *strName, const AVal *strValue)
{
    output = AMF_Encode_Key_String(output, outend, strName);
    return AMF_Encode_String(output, outend, strValue);
}


char *AMF_Encode_Object_Property(AMFObjectProperty *prop, char *pBuffer, char *pBufEnd)
{
    if (prop->p_type == AMF_INVALID)
		return NULL;

    if (prop->p_type != AMF_NULL && pBuffer + prop->p_name.av_len + 2 + 1 >= pBufEnd)
		return NULL;

    if (prop->p_type != AMF_NULL && prop->p_name.av_len)
	{
	    // name
        pBuffer = AMF_Encode_Key_String(pBuffer, pBufEnd, prop->p_name);
	}

    switch (prop->p_type)
	{
	case AMF_NUMBER:
		pBuffer = AMF_Encode_Number(pBuffer, pBufEnd, prop->p_vu.p_number);
		break;
	case AMF_BOOLEAN:
		pBuffer = AMF_Encode_Boolean(pBuffer, pBufEnd, prop->p_vu.p_number != 0);
		break;
	case AMF_STRING:
		pBuffer = AMF_Encode_String(pBuffer, pBufEnd, &prop->p_vu.p_aval);
		break;
	case AMF_NULL:
        pBuffer = AMF_Encode_Null(pBuffer, pBufEnd);
		break;
	case AMF_OBJECT:
		pBuffer = AMF_Encode_Object(&prop->p_vu.p_object, pBuffer, pBufEnd);
		break;
	case AMF_ECMA_ARRAY:
		pBuffer = AMF_Encode_Ecma_Array(&prop->p_vu.p_object, pBuffer, pBufEnd);
		break;
	case AMF_STRICT_ARRAY:
		pBuffer = AMF_Encode_Array(&prop->p_vu.p_object, pBuffer, pBufEnd);
		break;
	default:
		//RTMP_Log(RTMP_LOGERROR, "%s, invalid type. %d", __FUNCTION__, prop->p_type);
		pBuffer = NULL;
	};
	return pBuffer;
}


char *AMF_Encode_Object(AMFObject * obj, char *pBuffer, char *pBufEnd)
{
    int i;

	if (pBuffer+4 >= pBufEnd)
		return NULL;

    pBuffer = AMF_Encode_Object_Start(pBuffer, pBufEnd);

    for (i = 0; i < obj->o_num; i++)
	{
		char *res = AMF_Encode_Object_Property(&(obj->o_props + i), pBuffer, pBufEnd);
		if (res == NULL)
		{
			//RTMP_Log(RTMP_LOGERROR, "AMF_Encode - failed to encode property in index %d",i);
			break;
		}
		else
		{
			pBuffer = res;
		}
	}

    pBuffer = AMF_Encode_Object_End(pBuffer, pBufEnd);

    return pBuffer;
}

char *AMF_Encode_Array(AMFObject *obj, char *pBuffer, char *pBufEnd)
{
    int i;

	if (pBuffer+4 >= pBufEnd)
		return NULL;

    pBuffer = AMF_Encode_Array_Start(pBuffer, pBufEnd);

    pBuffer = AMF_Encode_Int32(pBuffer, pBufEnd, obj->o_num);

    for (i = 0; i < obj->o_num; i++)
	{
		char *res = AMF_Encode_Object_Property(&(obj->o_props + i), pBuffer, pBufEnd);
		if (res == NULL)
		{
			//RTMP_Log(RTMP_LOGERROR, "AMF_Encode - failed to encode property in index %d",i);
			break;
		}
		else
		{
			pBuffer = res;
		}
	}

    pBuffer = AMF_Encode_Object_End(pBuffer, pBufEnd);

    return pBuffer;
}

static int lrs_AMF_Encode_Array(LRS_AMF *amf, struct LRS_BUFFER *buffer)
{
    buffer->pread = AMF_Encode_Array(amf->amf, buffer->pread, buffer->pwrite);

    return buffer->pread - buffer->start;
}

char *AMF_Encode_Ecma_Array(AMFObject *obj, char *pBuffer, char *pBufEnd)
{
    int i;

	if (pBuffer+4 >= pBufEnd)
		return NULL;

    pBuffer = AMF_Encode_Ecma_Array_Start(pBuffer, pBufEnd);

    pBuffer = AMF_Encode_Int32(pBuffer, pBufEnd, obj->o_num);

    for (i = 0; i < obj->o_num; i++)
	{
		char *res = AMF_Encode_Object_Property(&(obj->o_props + i), pBuffer, pBufEnd);
		if (res == NULL)
		{
			//RTMP_Log(RTMP_LOGERROR, "AMF_Encode - failed to encode property in index %d",i);
			break;
		}
		else
		{
			pBuffer = res;
		}
	}

    pBuffer = AMF_Encode_Object_End(pBuffer, pBufEnd);

    return pBuffer;
}


AMFDataType AMF_Property_GetType(AMFObjectProperty * prop)
{
    return prop->p_type;
}

void AMF_Property_GetName(AMFObjectProperty * prop, AVal * name)
{
    *name = prop->p_name;
}
void AMF_Property_SetName(AMFObjectProperty * prop, AVal * name)
{
    prop->p_name = *name;
}
double AMF_Property_GetNumber(AMFObjectProperty * prop)
{
    return prop->p_vu.p_number;
}
int AMF_Property_GetBoolean(AMFObjectProperty * prop)
{
    return prop->p_vu.p_number != 0;
}

void AMF_Property_GetString(AMFObjectProperty * prop, AVal * str)
{
    if (prop->p_type == AMF_STRING)
		*str = prop->p_vu.p_aval;
	else
		*str = AV_empty;
}

void AMF_Property_GetObject(AMFObjectProperty * prop, AMFObject * obj)
{
    if (prop->p_type == AMF_OBJECT)
		*obj = prop->p_vu.p_object;
	else
		*obj = AMFObj_Invalid;
}

int AMF_Property_IsValid(AMFObjectProperty * prop)
{
    return prop->p_type != AMF_INVALID;
}

uint16_t AMF_Decode_Int16(const char *data)
{
    uint16_t val = 0;
    unsigned char *c = (unsigned char *) data;

    val = *c++;
    val = (val << 8) | *c++;

    return val;
}

uint32_t AMF_Decode_Int24(const char *data)
{
    uint32_t val = 0;
    unsigned char *c = (unsigned char *) data;
    
    val = *c++;
    val = (val << 8) | *c++;
    val = (val << 8) | *c++;

    return val;
}

static uint32_t lrs_decode_int24(struct LRS_BUFFER *buffer)
{
    uint32_t val = 0;
    
    val = *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    val = (val << 8) | *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    val = (val << 8) | *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    return val;
}


uint32_t AMF_Decode_Int32(const char *data)
{
    uint32_t val = 0;
    unsigned char *c = (unsigned char *) data;
    
    val = *c++;
    val = (val << 8) | *c++;
    val = (val << 8) | *c++;
    val = (val << 8) | *c++;

    return val;
}

static uint32_t lrs_decode_int32(struct LRS_BUFFER *buffer)
{
    uint32_t val = 0;
    
    val = *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    val = (val << 8) | *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    val = (val << 8) | *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }
    val = (val << 8) | *buffer->pread++;
    if(buffer->pread == buffer->end){
        buffer->pread = buffer->start;
    }

    return val;
}


double AMF_Decode_Number(const char *data)
{
    double dVal;
#if __FLOAT_WORD_ORDER == __BYTE_ORDER
{
	#if __BYTE_ORDER == __BIG_ENDIAN
    {   
		memcpy(&dVal, data, 8);
    }
	#elif __BYTE_ORDER == __LITTLE_ENDIAN
    {   
		unsigned char *ci, *co;
		ci = (unsigned char *)data;
		co = (unsigned char *)&dVal;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
    }
	#endif
}
#else
{
	#if __BYTE_ORDER == __LITTLE_ENDIAN
    {   
		unsigned char *ci, *co;
		ci = (unsigned char *)data;
		co = (unsigned char *)&dVal;
		co[0] = ci[3];
		co[1] = ci[2];
		co[2] = ci[1];
		co[3] = ci[0];
		co[4] = ci[7];
		co[5] = ci[6];
		co[6] = ci[5];
		co[7] = ci[4];
    }
	#else 
    {   
		unsigned char *ci, *co;
		ci = (unsigned char *)data;
		co = (unsigned char *)&dVal;
		co[0] = ci[4];
		co[1] = ci[5];
		co[2] = ci[6];
		co[3] = ci[7];
		co[4] = ci[0];
		co[5] = ci[1];
		co[6] = ci[2];
		co[7] = ci[3];
    }
	#endif
}
#endif
	return dVal;
}

int AMF_Decode_Boolean(const char *data)
{
    return *data != 0;
}

void AMF_Decode_String(const char *data, AVal *strValue)
{
    strValue->av_len = AMF_Decode_Int16(data);
	strValue->av_val = (strValue->av_len > 0) ? (char *)data + 2 : NULL;
}

void AMF_Decode_Long_String(const char *data, AVal *strValue)
{
    strValue->av_len = AMF_Decode_Int32(data);
	strValue->av_val = (strValue->av_len > 0) ? (char *)data + 4 : NULL;
}

void AMF_Object_Add_Property(AMFObject *obj, const AMFObjectProperty *prop)
{
    if (!(obj->o_num & 0x0f))
		obj->o_props = realloc(obj->o_props, (obj->o_num + 16) * sizeof(AMFObjectProperty));
	
	memcpy(&(obj->o_props + obj->o_num++), prop, sizeof(AMFObjectProperty));
}

int AMF_Object_Count_Property(AMFObject *obj)
{
    return obj->o_num;
}

AMFObjectProperty *AMF_Object_Get_Property(AMFObject * obj, const AVal * name, int nIndex)
{
    if (nIndex >= 0)
	{
		if (nIndex < obj->o_num)
			return &(obj->o_props + nIndex);
	}
	else
	{
		int n;
		for (n = 0; n < obj->o_num; n++)
		{
			if (AVMATCH(&((obj->o_props + n)->p_name), name))
				return &(obj->o_props + n);
		}
	}

	return (AMFObjectProperty *)&AMFProp_Invalid;
}

void AMF_Object_Property_Reset(AMFObjectProperty *prop)
{
    if (prop->p_type == AMF_OBJECT || prop->p_type == AMF_ECMA_ARRAY ||
		prop->p_type == AMF_STRICT_ARRAY)
		AMF_Object_Reset(&prop->p_vu.p_object);
	else
	{
		prop->p_vu.p_aval.av_len = 0;
		prop->p_vu.p_aval.av_val = NULL;
	}
	prop->p_type = AMF_INVALID;
}

int AMF_Decode_Object(AMFObject *obj, const char *pBuffer, int nSize, int bDecodeName)
{
    int nOriginalSize = nSize;
	int bError = FALSE;

    obj->o_num = 0;
	obj->o_props = NULL;
    
    while (nSize > 0)
    {
        AMFObjectProperty prop;
        int nRes;

        if (nSize >=3 && AMF_Decode_Int24(pBuffer) == AMF_OBJECT_END)
        {
            nSize -= 3;
            bError = FALSE;
            break;
        }

        if (bError)
        {
            //RTMP_Log(RTMP_LOGERROR,"DECODING ERROR, IGNORING BYTES UNTIL NEXT KNOWN PATTERN!");
            nSize--;
            pBuffer++;
            continue;
        }

        nRes = AMF_Decode_Object_Property(&prop, pBuffer, nSize, bDecodeName);
        if (nRes == -1)
        {
            bError = TRUE;
            break;
        }
        else
        {
            nSize -= nRes;
            if (nSize < 0)
            {
                bError = TRUE;
                break;
            }
            pBuffer += nRes;
            AMF_Object_Add_Property(obj, &prop);
        }
    }

    if (bError)
		return -1;

	return nOriginalSize - nSize;
}

int AMF_Decode_Object_Property(AMFObjectProperty *prop, const char *pBuffer, int nSize, int bDecodeName)
{
    int nOriginalSize = nSize;
	int nRes;

	prop->p_name.av_len = 0;
	prop->p_name.av_val = NULL;

    if (nSize == 0 || !pBuffer)
	{
		//RTMP_Log(RTMP_LOGDEBUG, "%s: Empty buffer/no buffer pointer!", __FUNCTION__);
		return -1;
	}

    if (bDecodeName && nSize < 4)
	{				/* at least name (length + at least 1 byte) and 1 byte of data */
		//RTMP_Log(RTMP_LOGDEBUG, "%s: Not enough data for decoding with name, less than 4 bytes!",__FUNCTION__);
		return -1;
	}

    if (bDecodeName)
	{
		unsigned short nNameSize = AMF_Decode_Int16(pBuffer);
		if (nNameSize > nSize - 2)
		{
			//RTMP_Log(RTMP_LOGDEBUG,"%s: Name size out of range: namesize (%d) > len (%d) - 2",__FUNCTION__, nNameSize, nSize);
			return -1;
		}

		AMF_Decode_String(pBuffer, &prop->p_name);
		nSize -= 2 + nNameSize;
		pBuffer += 2 + nNameSize;
	}

    if (nSize == 0)
	{
		return -1;
	}

    prop->p_type = *pBuffer++;
    nSize--;

    switch (prop->p_type)
    {
    case AMF_NUMBER:
        if (nSize < 8)
            return -1;
        prop->p_vu.p_number = AMF_Decode_Number(pBuffer);
        nSize -= 8;
        break;
    case AMF_BOOLEAN:
        if (nSize < 1)
            return -1;
        prop->p_vu.p_number = (double)AMF_Decode_Boolean(pBuffer);
        nSize--;
        break;
    case AMF_STRING:
        {
            unsigned short nStringSize = AMF_Decode_Int16(pBuffer);

            if (nSize < (long)nStringSize + 2)
                return -1;
            AMF_Decode_String(pBuffer, &prop->p_vu.p_aval);
            nSize -= (2 + nStringSize);
            break;
        }
    case AMF_OBJECT:
        {
            int nRes = AMF_Decode_Object(&prop->p_vu.p_object, pBuffer, nSize, TRUE);
            if (nRes == -1)
                return -1;
            nSize -= nRes;
            break;
        }
    case AMF_MOVIECLIP:
        {
            //RTMP_Log(RTMP_LOGERROR, "AMF_MOVIECLIP reserved!");
            return -1;
            break;
        }
    case AMF_NULL:
    case AMF_UNDEFINED:
    case AMF_UNSUPPORTED:
        prop->p_type = AMF_NULL;
        break;
    case AMF_REFERENCE:
        {
            //RTMP_Log(RTMP_LOGERROR, "AMF_REFERENCE not supported!");
            return -1;
            break;
        }
    case AMF_ECMA_ARRAY:
        {
            nSize -= 4;
            /* next comes the rest, mixed array has a final 0x000009 mark and names, so its an object */
            nRes = AMF_Decode_Object(&prop->p_vu.p_object, pBuffer + 4, nSize, TRUE);
            if (nRes == -1)
                return -1;
            nSize -= nRes;
            break;
        }
    case AMF_OBJECT_END:
        {
            return -1;
            break;
        }
    case AMF_STRICT_ARRAY:
        {
            unsigned int nArrayLen = AMF_Decode_Int32(pBuffer);
            nSize -= 4;

            nRes = AMF_Decode_Array(&prop->p_vu.p_object, pBuffer + 4, nSize, nArrayLen, FALSE);
            if (nRes == -1)
                return -1;
            nSize -= nRes;
            break;
        }
    case AMF_DATE:
        {
            //RTMP_Log(RTMP_LOGDEBUG, "AMF_DATE");

            if (nSize < 10)
                return -1;

            prop->p_vu.p_number = AMF_Decode_Number(pBuffer);
            prop->p_UTCoffset = AMF_Decode_Int16(pBuffer + 8);

            nSize -= 10;
            break;
        }
    case AMF_LONG_STRING:
    case AMF_XML_DOC:
        {
            unsigned int nStringSize = AMF_Decode_Int32(pBuffer);
            if (nSize < (long)nStringSize + 4)
                return -1;
            AMF_Decode_Long_String(pBuffer, &prop->p_vu.p_aval);
            nSize -= (4 + nStringSize);
            if (prop->p_type == AMF_LONG_STRING)
                prop->p_type = AMF_STRING;
            break;
        }
    case AMF_RECORDSET:
        {
            //RTMP_Log(RTMP_LOGERROR, "AMF_RECORDSET reserved!");
            return -1;
            break;
        }
    case AMF_TYPED_OBJECT:
        {
            //RTMP_Log(RTMP_LOGERROR, "AMF_TYPED_OBJECT not supported!");
            return -1;
            break;
        }
    case AMF_AVMPLUS:
        {
            int nRes = AMF3_Decode_Object(&prop->p_vu.p_object, pBuffer, nSize, TRUE);
            if (nRes == -1)
                return -1;
            nSize -= nRes;
            prop->p_type = AMF_OBJECT;
            break;
        }
    default:
        //RTMP_Log(RTMP_LOGDEBUG, "%s - unknown datatype 0x%02x, @%p", __FUNCTION__,prop->p_type, pBuffer - 1);
        return -1;
    }

    return nOriginalSize - nSize;
}

int AMF_Decode_Array(AMFObject *obj, const char *pBuffer, int nSize, int nArrayLen, int bDecodeName)
{
    int nOriginalSize = nSize;
	int bError = FALSE;

	obj->o_num = 0;
	obj->o_props = NULL;
	while (nArrayLen > 0)
	{
		AMFObjectProperty prop;
		int nRes;
		nArrayLen--;

		if (nSize <= 0)
		{
			bError = TRUE;
			break;
		}
		nRes = AMF_Decode_Object_Property(&prop, pBuffer, nSize, bDecodeName);
		if (nRes == -1)
		{
			bError = TRUE;
			break;
		}
		else
		{
			nSize -= nRes;
			pBuffer += nRes;
			AMF_Object_Add_Property(obj, &prop);
		}
	}
	if (bError)
		return -1;

	return nOriginalSize - nSize;
}

void AMF_Object_Reset(AMFObject * obj)
{
    int n;
	for (n = 0; n < obj->o_num; n++)
	{
		AMF_Object_Property_Reset(&(obj->o_props + n));
	}
	free(obj->o_props);
	obj->o_props = NULL;
	obj->o_num = 0;
}


int AMF3_Read_Integer(const char *data, int32_t *valp)
{
    int i = 0;
	int32_t val = 0;

	while (i <= 2)
	{				/* handle first 3 bytes */
		if (data[i] & 0x80)
		{			/* byte used */
			val <<= 7;		/* shift up */
			val |= (data[i] & 0x7f);	/* add bits */
			i++;
		}
		else
		{
			break;
		}
	}

	if (i > 2)
	{				/* use 4th byte, all 8bits */
		val <<= 8;
		val |= data[3];

		/* range check */
		if (val > AMF3_INTEGER_MAX)
			val -= (1 << 29);
	}
	else
	{				/* use 7bits of last unparsed byte (0xxxxxxx) */
		val <<= 7;
		val |= data[i];
	}

	*valp = val;

	return i > 2 ? 4 : i + 1;
}


int AMF3_Read_String(const char *data, AVal *str)
{
    int32_t ref = 0;
	int len;
	assert(str != 0);

	len = AMF3_Read_Integer(data, &ref);
	data += len;

	if ((ref & 0x1) == 0)
	{				/* reference: 0xxx */
		uint32_t refIndex = (ref >> 1);
		//RTMP_Log(RTMP_LOGDEBUG,"%s, string reference, index: %d, not supported, ignoring!",__FUNCTION__, refIndex);
		str->av_val = NULL;
		str->av_len = 0;
		return len;
	}
	else
	{
		uint32_t nSize = (ref >> 1);

		str->av_val = (char *)data;
		str->av_len = nSize;

		return len + nSize;
	}
	return len;
}

void AMF3_CD_Object_Add_Property(AMF3ClassDef *cd, AVal *prop)
{
    if (!(cd->cd_num & 0x0f))
		cd->cd_props = realloc(cd->cd_props, (cd->cd_num + 16) * sizeof(AVal));
    
	*(cd->cd_props + cd->cd_num++) = *prop;
}

AVal *AMF3_CD_Object_Get_Property(AMF3ClassDef *cd, int nIndex)
{
    if (nIndex >= cd->cd_num)
		return (AVal *)&AV_empty;
	return &(cd->cd_props + nIndex);
}

int AMF3_Decode_Object(AMFObject *obj, const char *pBuffer, int nSize, int bAMFData)
{
    int nOriginalSize = nSize;
	int32_t ref;
	int len;

	obj->o_num = 0;
	obj->o_props = NULL;
	if (bAMFData)
	{
		//if (*pBuffer != AMF3_OBJECT)
			//RTMP_Log(RTMP_LOGERROR,"AMF3 Object encapsulated in AMF stream does not start with AMF3_OBJECT!");
		pBuffer++;
		nSize--;
	}

	ref = 0;
	len = AMF3_Read_Integer(pBuffer, &ref);
	pBuffer += len;
	nSize -= len;

	if ((ref & 1) == 0)
	{				/* object reference, 0xxx */
		uint32_t objectIndex = (ref >> 1);
		//RTMP_Log(RTMP_LOGDEBUG, "Object reference, index: %d", objectIndex);
	}
	else				/* object instance */
    {
    	int32_t classRef = (ref >> 1);

    	AMF3ClassDef cd = { {0, 0} };
    	AMFObjectProperty prop;

    	if ((classRef & 0x1) == 0)
    	{			/* class reference */
    		uint32_t classIndex = (classRef >> 1);
    		//RTMP_Log(RTMP_LOGDEBUG, "Class reference: %d", classIndex);
    	}
    	else
    	{
    		int32_t classExtRef = (classRef >> 1);
    		int i, cdnum;

    		cd.cd_externalizable = (classExtRef & 0x1) == 1;
    		cd.cd_dynamic = ((classExtRef >> 1) & 0x1) == 1;

    		cdnum = classExtRef >> 2;

    		/* class name */

    		len = AMF3_Read_String(pBuffer, &cd.cd_name);
    		nSize -= len;
    		pBuffer += len;

    		/*std::string str = className; */

    		//RTMP_Log(RTMP_LOGDEBUG,
    			//"Class name: %s, externalizable: %d, dynamic: %d, classMembers: %d",
    			//cd.cd_name.av_val, cd.cd_externalizable, cd.cd_dynamic,cd.cd_num);

    		for (i = 0; i < cdnum; i++)
    		{
    			AVal memberName;
    			if (nSize <=0)
    			{
invalid:
                    //RTMP_Log(RTMP_LOGDEBUG, "%s, invalid class encoding!",__FUNCTION__);
    				return nOriginalSize;
    			}
    			len = AMF3_Read_String(pBuffer, &memberName);
    			//RTMP_Log(RTMP_LOGDEBUG, "Member: %s", memberName.av_val);
    			AMF3_CD_Object_Add_Property(&cd, &memberName);
    			nSize -= len;
    			pBuffer += len;
    		}
    	}

		/* add as referencable object */
        if (cd.cd_externalizable)
    	{
    		int nRes;
    		AVal name = AVC("DEFAULT_ATTRIBUTE");

    		//RTMP_Log(RTMP_LOGDEBUG, "Externalizable, TODO check");

    		nRes = AMF3_Decode_Object_Property(&prop, pBuffer, nSize, FALSE);
    		if (nRes == -1)
    			//RTMP_Log(RTMP_LOGDEBUG, "%s, failed to decode AMF3 property!",__FUNCTION__);
    		else
    		{
    			nSize -= nRes;
    			pBuffer += nRes;
    		}

    		AMF_Property_SetName(&prop, &name);
    		AMF_Object_Add_Property(obj, &prop);
    	}
    	else
        {
    		int nRes, i;
    		for (i = 0; i < cd.cd_num; i++)	/* non-dynamic */
    		{
    			if (nSize <=0)
    				goto invalid;
    			nRes = AMF3_Decode_Object_Property(&prop, pBuffer, nSize, FALSE);
    			if (nRes == -1)
    				//RTMP_Log(RTMP_LOGDEBUG, "%s, failed to decode AMF3 property!",__FUNCTION__);

    			AMF_Property_SetName(&prop, AMF3_CD_Object_Get_Property(&cd, i));
    			AMF_Object_Add_Property(obj, &prop);

    			pBuffer += nRes;
    			nSize -= nRes;
    		}
    		if (cd.cd_dynamic)
    		{
    			int len = 0;

    			do
    			{
    				if (nSize <=0)
    					goto invalid;
    				nRes = AMF3_Decode_Object_Property(&prop, pBuffer, nSize, TRUE);
    				AMF_Object_Add_Property(obj, &prop);

    				pBuffer += nRes;
    				nSize -= nRes;

    				len = prop.p_name.av_len;
    			}
    			while (len > 0);
    		}
	    }
		//RTMP_Log(RTMP_LOGDEBUG, "class object!");
	}
	return nOriginalSize - nSize;
}


int AMF3_Decode_Object_Property(AMFObjectProperty *prop, const char *pBuffer, int nSize, int bDecodeName)
{
    int nOriginalSize = nSize;
	AMF3DataType type;

	prop->p_name.av_len = 0;
	prop->p_name.av_val = NULL;

	if (nSize == 0 || !pBuffer)
	{
		//RTMP_Log(RTMP_LOGDEBUG, "empty buffer/no buffer pointer!");
		return -1;
	}
    
    /* decode name */
    if (bDecodeName)
    {
        AVal name;
        int nRes = AMF3_Read_String(pBuffer, &name);

        if (name.av_len <= 0)
            return nRes;

        nSize -= nRes;
        if (nSize <= 0)
            return -1;
        prop->p_name = name;
        pBuffer += nRes;
    }

    /* decode */
	type = *pBuffer++;
	nSize--;

	switch (type)
	{
	case AMF3_UNDEFINED:
	case AMF3_NULL:
		prop->p_type = AMF_NULL;
		break;
	case AMF3_FALSE:
		prop->p_type = AMF_BOOLEAN;
		prop->p_vu.p_number = 0.0;
		break;
	case AMF3_TRUE:
		prop->p_type = AMF_BOOLEAN;
		prop->p_vu.p_number = 1.0;
		break;
	case AMF3_INTEGER:
		{
			int32_t res = 0;
			int len = AMF3_Read_Integer(pBuffer, &res);
			prop->p_vu.p_number = (double)res;
			prop->p_type = AMF_NUMBER;
			nSize -= len;
			break;
		}
	case AMF3_DOUBLE:
		if (nSize < 8)
			return -1;
		prop->p_vu.p_number = AMF_Decode_Number(pBuffer);
		prop->p_type = AMF_NUMBER;
		nSize -= 8;
		break;
	case AMF3_STRING:
	case AMF3_XML_DOC:
	case AMF3_XML:
		{
			int len = AMF3_Read_String(pBuffer, &prop->p_vu.p_aval);
			prop->p_type = AMF_STRING;
			nSize -= len;
			break;
		}
	case AMF3_DATE:
		{
			int32_t res = 0;
			int len = AMF3_Read_Integer(pBuffer, &res);

			nSize -= len;
			pBuffer += len;

			if ((res & 0x1) == 0)
			{			/* reference */
				uint32_t nIndex = (res >> 1);
				//RTMP_Log(RTMP_LOGDEBUG, "AMF3_DATE reference: %d, not supported!", nIndex);
			}
			else
			{
				if (nSize < 8)
					return -1;

				prop->p_vu.p_number = AMF_Decode_Number(pBuffer);
				nSize -= 8;
				prop->p_type = AMF_NUMBER;
			}
			break;
		}
	case AMF3_OBJECT:
		{
			int nRes = AMF3_Decode_Object(&prop->p_vu.p_object, pBuffer, nSize, TRUE);
			if (nRes == -1)
				return -1;
			nSize -= nRes;
			prop->p_type = AMF_OBJECT;
			break;
		}
	case AMF3_ARRAY:
	case AMF3_BYTE_ARRAY:
	default:
		//RTMP_Log(RTMP_LOGDEBUG, "%s - AMF3 unknown/unsupported datatype 0x%02x, @%p",__FUNCTION__, (unsigned char)(*pBuffer), pBuffer);
		return -1;
	}
	if (nSize < 0)
		return -1;

	return nOriginalSize - nSize;
}


struct LRS_AMF* amf_alloc()
{
    struct LRS_AMF *amf = NULL;
    amf = (struct LRS_AMF*)malloc(sizeof(struct LRS_AMF));
    if(!amf){
        printf("amf_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(amf, 0, sizeof(struct LRS_AMF));
    
    amf->amf = (struct AMFObject*)malloc(sizeof(struct AMFObject));
    if(!amf->amf){
        printf("amf_alloc amf failed! error msg : %s\n", strerror(errno));
        free(amf);
        return NULL;
    }
    memset(amf->amf, 0, sizeof(struct AMFObject));
    
    amf->amf_encoder = lrs_AMF_Encode_Array;
    amf->amf_decoder = AMF_Decode_Object;
    amf->amf_reset = AMF_Object_Reset;

    amf->dint24 = lrs_decode_int24;
    amf->dint32 = lrs_decode_int32;
    amf->eint24 = lrs_encoder_int24;
    amf->eint32 = lrs_encode_int32;
    return amf;
}

void amf_free(struct LRS_AMF* amf)
{
    if(amf){
        if(amf->amf){
            amf->amf_reset(amf->amf);
            free(amf->amf);
            amf->amf = NULL;
        }

        amf->amf_encoder = NULL;
        amf->amf_decoder = NULL;
        amf->amf_reset = NULL;
    
        free(amf);
        amf = NULL;
    }
    return;
}
