
#pragma once 

#include<iostream>
#include<string>
using namespace std;

//template <typename RC4_TYPE>
typedef BYTE RC4_TYPE ;
class RC4
{
public :
	enum eTABLE_SIZE
	{
		TABLE_SIZE = 256,
	};
	RC4_TYPE enc_s[TABLE_SIZE];
	RC4_TYPE dec_s[TABLE_SIZE];

	RC4_TYPE t[TABLE_SIZE];

	static void swap(RC4_TYPE* s1,RC4_TYPE* s2)
	{
		RC4_TYPE temp=*s1;
		*s1=*s2;
		*s2=temp;
	}

	void RC4_init(RC4_TYPE s[TABLE_SIZE],const char *pKey,int keylen)
	{
		//初始化s和t数组
		for(int i=0;i<TABLE_SIZE;i++)
		{
			s[i]=i;
			t[i]=pKey[i % keylen];
		}
		int j=0;
		for(int i=0;i<TABLE_SIZE;i++)
		{
			j=(j+s[j]+t[j])%TABLE_SIZE;
			swap(s+i,s+j);
		}
	}

	static void RC4_encode0(RC4_TYPE s[TABLE_SIZE],RC4_TYPE* src,int srclen,RC4_TYPE* dst,int dstlen)
	{
		int pos=0;
		int i=0,j=0;
		while(pos++<srclen)
		{
			i=(i+1)%256;
			j=(j+s[i])%256;
			swap(s+i,s+j);
			int temp=(s[i]+s[j])%256;
			RC4_TYPE k=s[temp];
			*dst=(*src)^k;
			dst++;
			src++;
		}
	}
	//加密
	void RC4_BeginEncode(const char *pKey,int keylen)
	{
		RC4_init(enc_s ,pKey,keylen);
	}

	void RC4_encode(RC4_TYPE* src,int srclen,RC4_TYPE* dst,int dstlen)
	{
		RC4_encode0(enc_s , src, srclen, dst, dstlen);
	}

	//解密
	void RC4_BeginDecode(const char *pKey,int keylen)
	{
		RC4_init(dec_s ,pKey,keylen);
	}
	void RC4_decode(RC4_TYPE* src,int srclen,RC4_TYPE* dst,int dstlen)
	{
		RC4_encode0(dec_s , src,srclen, dst,dstlen);
	}
#ifdef _TEST_RC4
	static void print(RC4_TYPE* a,int length,string startMessage="")
	{
		cout<<startMessage<<" ";
		for(int i=0;i<length;i++){
			cout<<*(a+i)<<" ";
		}
		cout<<endl;
	}

	void main_rc4()
	{
//		const char key[]=" s s ssdfdsf5678901234sdfasfafadfafafsadf";
		const char key[]="8jel5y";
		int keylen = sizeof(key) - 1;

		//加密器
		RC4_init(enc_s ,key,keylen);
		RC4_TYPE a[10]={0,1,2,3,4,5,6,7,8,9};
		print(a,10,"a原数组");
		RC4_encode(a,10,a,10);
		print(a,10,"a加密后");

		RC4_TYPE b[10]={4,5,0,1,2,3,6,7,8,9};
		print(b,10,"b原数组");
		RC4_encode(b,10,b,10);
		print(b,10,"b加密后");

		RC4_TYPE c[10]={5,6,7,8,9,0,1,2,3,4};
		print(c,10,"c原数组");
		RC4_encode(c,10,c,10);
		print(c,10,"c加密后");

		//解密器
		//RC4_init(dec_s ,key,keylen);
		//RC4_decode(a,10,a,10);
		BaseRC4::setPassword(key);
		BaseRC4::Decrypt((char*)a, 10);
		print(a,10,"a解密后");

		//RC4_decode(b,10,b,10);
		BaseRC4::Decrypt((char*)b, 10);
		print(b,10,"b解密后");

		//RC4_decode(c,10,c,10);
		BaseRC4::Decrypt((char*)c, 10);
		print(c,10,"c解密后");

		getchar();
	}
#endif
};