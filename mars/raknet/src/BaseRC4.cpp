#include "BaseRC4.h"
#include "string.h"
#include "stdio.h"

BYTE BaseRC4::mKeys[KEYS_LEN];
//const char* BaseRC4::szPassWord = "111111";

char BaseRC4::szPassWord[PASSWROD_LEN+8] = {0};


BaseRC4::~BaseRC4()
{
}

void BaseRC4::setPassword( const char* szPass )
{

	if ( strlen(szPass) > 64 )
		return;

	strcpy( szPassWord, szPass );

}


//º”√‹
void BaseRC4::Encrypt(char* szSource,UINT len)
{
	if (szSource == NULL) return;

	if(RC4((PBYTE)szSource,len)==0)
		return;
}

//Ω‚√‹
void BaseRC4::Decrypt(char* szSource,UINT len)
{
	if (szSource == NULL) return;

	if(RC4((PBYTE)szSource,len)==0) return;
}

int BaseRC4::RC4(PBYTE data,UINT data_len)
{
	if(data == 0) return 0;

	if(GetKey((PBYTE)szPassWord,strlen(szPassWord),mKeys) == 0)
		return 0;

	int x = 0;
	int y = 0;

	for (UINT k = 0; k < data_len; k++)
	{
		x = (x + 1) % KEYS_LEN;
		y = (mKeys[x] + y) % KEYS_LEN;
		swap_byte(&mKeys[x],&mKeys[y]);
		data[k] = data[k] ^ mKeys[(mKeys[x] + mKeys[y]) % KEYS_LEN];
	}

	return -1;
}

int BaseRC4::GetKey(const PBYTE pass, UINT pass_len, PBYTE out)
{
	if(pass == 0 || out == 0) 
		return 0;

	int i;

	for(i = 0; i < KEYS_LEN; i++)
		out[i] = i;

	int j = 0;
	for (i = 0; i< KEYS_LEN; i++)
	{
		j = (pass[i%pass_len] + out[i] + j) % KEYS_LEN;
		swap_byte(&out[i], &out[j]);
	}
	return -1;
}

void BaseRC4::swap_byte(PBYTE a, PBYTE b)
{
	BYTE swapByte = *a;
	*a = *b;
	*b = swapByte;
}
