#include <iostream>
#include <cstring>
#include "SM3_class"
#include <fstream>

#include <vector>
#include <iomanip>
#include <memory>
#include <stdint.h>
#include <ctime>
#include <ratio>

#include <chrono>
#include <stdlib.h>
using namespace std;


//�����ļ�˵��,ʹ��SM3�������ı����м���
#define MAX_NUM 1024*1024
#define MAXSIZE 1024*MAX_NUM

//�趨�����ļ�������ֽ���Ϊ4KB
//�������ֽ������ó�����Զ����зֿ�
//��Ϊ�����ı�Ƭ�ν��зֱ����


unsigned int hash_result = 0;
//�ܵ���Ϣ��
unsigned int Rate_of_hash = 0;
//��ǰ�Ѿ��������hash���ı����ݱ���



static const int endianTest = 1;
#define IsLittleEndian() (*(char *)&endianTest == 1)
//���ȳ�����Ҫ�ж����л����Ƿ�ΪС��
#define LeftRotate(word, bits) ( (word) << (bits) | (word) >> (32 - (bits)) )
//��ѭsm3�ı�׼���ܷ���������ѭ����λ��ͬʱ��ת���ֽ������ֽ���

unsigned int* Reverse_word(unsigned int* word)
{
	unsigned char* byte, temp;

	byte = (unsigned char*)word;
	temp = byte[0];
	byte[0] = byte[3];
	byte[3] = temp;

	temp = byte[1];
	byte[1] = byte[2];
	byte[2] = temp;
	return word;

}
//�������ֱ�ʵ��sm3�ĸ�������
//T
unsigned int T(int i)
{
	if (i >= 0 && i <= 15)
		return 0x79CC4519;
	else if (i >= 16 && i <= 63)
		return 0x7A879D8A;
	else
		return 0;
}

//FF
unsigned int FF(unsigned int X, unsigned int Y, unsigned int Z, int i)
{
	if (i >= 0 && i <= 15)
		return X ^ Y ^ Z;
	else if (i >= 16 && i <= 63)
		return (X & Y) | (X & Z) | (Y & Z);
	else
		return 0;
}

//GG
unsigned int GG(unsigned int X, unsigned int Y, unsigned int Z, int i)
{
	if (i >= 0 && i <= 15)
		return X ^ Y ^ Z;
	else if (i >= 16 && i <= 63)
		return (X & Y) | (~X & Z);
	else
		return 0;
}

//P0
unsigned int P0(unsigned int X)
{
	return X ^ LeftRotate(X, 9) ^ LeftRotate(X, 17);
}

//P1
unsigned int P1(unsigned int X)
{
	return X ^ LeftRotate(X, 15) ^ LeftRotate(X, 23);
}

//��sm3���г�ʼ������
void SM3_Init(SM3::SM3Context* context) {
	context->intermediateHash[0] = 0x7380166F;
	context->intermediateHash[1] = 0x4914B2B9;
	context->intermediateHash[2] = 0x172442D7;
	context->intermediateHash[3] = 0xDA8A0600;
	context->intermediateHash[4] = 0xA96F30BC;
	context->intermediateHash[5] = 0x163138AA;
	context->intermediateHash[6] = 0xE38DEE4D;
	context->intermediateHash[7] = 0xB0FB0E4E;
}

// ��input���ı����зֿ鴦��
void SM3_dealwith_MessageBlock(SM3::SM3Context* context)
{
	int i;
	unsigned int W[68];
	unsigned int W_[64];
	unsigned int A, B, C, D, E, F, G, H, SS1, SS2, TT1, TT2;

	//message extence 
	for (i = 0; i < 16; i++)
	{
		W[i] = *(unsigned int*)(context->messageBlock + i * 4);
		if (IsLittleEndian())
			ReverseWord(W + i);
	}
	for (i = 16; i < 68; i++)
	{
		//P1
		W[i] = (W[i - 16] ^ W[i - 9] ^ LeftRotate(W[i - 3], 15)) ^ LeftRotate((W[i - 16] ^ W[i - 9] ^ LeftRotate(W[i - 3], 15)), 15) ^ LeftRotate((W[i - 16] ^ W[i - 9] ^ LeftRotate(W[i - 3], 15)), 23)
			^ LeftRotate(W[i - 13], 7)
			^ W[i - 6];
	}
	for (i = 0; i < 64; i++)
	{
		W_[i] = W[i] ^ W[i + 4];
	}

	//message compression
	A = context->intermediateHash[0];
	B = context->intermediateHash[1];
	C = context->intermediateHash[2];
	D = context->intermediateHash[3];
	E = context->intermediateHash[4];
	F = context->intermediateHash[5];
	G = context->intermediateHash[6];
	H = context->intermediateHash[7];

	for (i = 0; i < 64; i++)
	{
		unsigned int SS3;

		SS1 = LeftRotate((LeftRotate(A, 12) + E + LeftRotate(T(i), i)), 7);
		SS2 = SS1 ^ LeftRotate(A, 12);
		TT1 = FF(A, B, C, i) + D + SS2 + W_[i];
		TT2 = GG(E, F, G, i) + H + SS1 + W[i];


		D = C;
		C = LeftRotate(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = LeftRotate(F, 19);
		F = E;
		E = TT2 ^ LeftRotate(TT2, 9) ^ LeftRotate(TT2, 17);
	}
	context->intermediateHash[0] ^= A;
	context->intermediateHash[1] ^= B;
	context->intermediateHash[2] ^= C;
	context->intermediateHash[3] ^= D;
	context->intermediateHash[4] ^= E;
	context->intermediateHash[5] ^= F;
	context->intermediateHash[6] ^= G;
	context->intermediateHash[7] ^= H;
}

/*
* SM3������������:
	message������Ҫ���ܵ���Ϣ�ֽڴ�;
	messagelen����Ϣ���ֽ���;
	digset��ʾ���صĹ�ϣֵ
*/
unsigned char* SM3::SM3Calc(const unsigned char* message,
	unsigned int messageLen, unsigned char digest[SM3_HASH_SIZE])
{
	SM3::SM3Context context;
	unsigned int i, remainder, bitLen;

	SM3_Init(&context);
	hash_result = messageLen / 64 + 1;
	//�����ܿ���
	remainder = messageLen % 64;
	if (remainder > 111) {
		hash_result += 1;
		//mod64֮���������111��˵��������4KB��������Ҫ����һ�������Ϣ���
		//�ܿ�����Ҫ+1
	}
	//��ǰ�����Ϣ������д��� 
	for (i = 0; i < messageLen / 64; i++)
	{
		memcpy(context.messageBlock, message + i * 64, 64);
		Rate_of_hash = i + 1;
		//ÿ����һ��512bit����Ϣ�飬���Ⱦ�+1
		SM3_dealwith_MessageBlock(&context);
	}

	//�����Ϣ���飬������ 
	bitLen = messageLen * 8;
	if (IsLittleEndian())
		ReverseWord(&bitLen);
	memcpy(context.messageBlock, message + i * 64, remainder);
	context.messageBlock[remainder] = 0x80;
	//����bit��0x1000 0000����ĩβ
	if (remainder <= 111)
	{
		//���Ȱ��մ�˷�ռ8���ֽڣ�ֻ���ǳ����� 2^32 - 1(bit)���ڵ������
		//�ʽ��� 4 ���ֽڸ�Ϊ 0 
		memset(context.messageBlock + remainder + 1, 0, 64 - remainder - 1 - 8 + 4);
		memcpy(context.messageBlock + 64 - 4, &bitLen, 4);
		Rate_of_hash += 1;//�������һ���̿�
		SM3_dealwith_MessageBlock(&context);
	}
	else
	{
		memset(context.messageBlock + remainder + 1, 0, 64 - remainder - 1);
		hash_rate += 1;
		//�����Ҷ������ӵĶ̿�
		SM3_dealwith_MessageBlock(&context);
		//���Ȱ��մ�˷�ռ8���ֽڣ�ֻ���ǳ����� 2^32 - 1(bit)���ڵ������
		//�ʽ��� 4 ���ֽڸ�Ϊ 0 
		memset(context.messageBlock, 0, 64 - 4);
		memcpy(context.messageBlock + 64 - 4, &bitLen, 4);
		Rate_of_hash += 1;
		//�������һ���̿�
		SM3_dealwith_MessageBlock(&context);
	}
	if (IsLittleEndian())
		for (i = 0; i < 8; i++)
			ReverseWord(context.intermediateHash + i);
	memcpy(digest, context.intermediateHash, SM3_HASH_SIZE);
	return digest;
}

/*
* call_hash_sm3����
	����������ļ���ַ�ַ���
	���������vector<unit32_t> hash_result(32)
*/
std::vector<uint32_t> SM3::call_hash_sm3(char* filepath)
{
	std::vector<uint32_t> hash_result(32, 0);
	std::ifstream infile;
	uint32_t FILESIZE = 0;
	//�����ļ������������ļ���Ϊ�ı����ݴ�����
	unsigned char* buffer = new unsigned char[MAXSIZE];
	unsigned char hash_output[32];
	struct _stat info;
	_stat(filepath, &info);
	FILESIZE = info.st_size;
	infile.open(filepath, std::ifstream::binary);
	infile >> buffer;

	auto start = std::chrono::high_resolution_clock::now();
	SM3::SM3Calc(buffer, FILESIZE, hash_output);
	auto end = std::chrono::high_resolution_clock::now();
	// �Ժ���Ϊ��λ����������ʱ��
	std::cout << "in millisecond time:";
	std::chrono::duration<double, std::ratio<1, 1000>> diff = end - start;
	std::cout << "Time is " << diff.count() << " ms\n";
	hash_result.assign(&hash_output[0], &hash_output[32]);
	delete[]buffer;
	return hash_result;
}

//�Ե�ǰ�Ĺ�ϣ���Ƚ��м����뷴��
double progress() {
	return (double(Rate_of_hash) / hash_result);
}

//�����̶���С���ļ�
void CreatTxt(char* pathName, int length)//����txt�ļ�
{
	ofstream fout(pathName);
	char char_list[] = "abcdefghijklmnopqrstuvwxyz";
	int n = 26;
	if (fout) { 
		for (int i = 0; i < length; i++)
		{
			fout << char_list[rand() % n]; 
			// ʹ�ú������ͬ���ķ�ʽ����д��
		}

		fout.close();  
		// ִ���������ر��ļ������
		//һ��Ҫд��һ�䣬�����´����е�ʱ����������
	}
}
int main() {
	char filepath[] = "test.txt";
	CreatTxt(filepath, MAX_NUM);
	std::vector<uint32_t> hash_result;
	hash_result = SM3::call_hash_sm3(filepath);
	for (int i = 0; i < 32; i++) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << hash_result[i];
		if (((i + 1) % 4) == 0) std::cout << " ";
	}
	std::cout << std::endl;
	double rate = progress();
	printf("\n��ǰ����: %f", rate);
	return 0;
}