#ifndef _ENCRYPT_BASERC4_
#define  _ENCRYPT_BASERC4_

typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef BYTE *PBYTE;

#define KEYS_LEN 256
#define PASSWROD_LEN 64

class BaseRC4{
private:

	static BYTE mKeys[KEYS_LEN];
	//static const char* szPassWord;
	static char szPassWord[PASSWROD_LEN+8];

public:

	~BaseRC4();

	static int GetKey(const PBYTE pass, UINT pass_len, PBYTE out);
	static int RC4(const PBYTE data,UINT data_len);

	static void setPassword( const char* szPass );

	static void swap_byte(PBYTE a, PBYTE b);

	static void Encrypt(char* szSource, UINT len);
	static void Decrypt(char* szSource, UINT len);

};

#endif //#ifndef _ENCRYPT_BASERC4_
