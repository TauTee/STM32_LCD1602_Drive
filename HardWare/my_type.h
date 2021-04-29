/**
  ******************************************************************************
  *���ƣ�my_foc
    *�������ռ�ʸ������������ͬ������ϵ�ʵ��
    *���أ�STM32F405RGT6
    *���ڣ�2020��8��19��
    *���ߣ��ſ�ǿ
  ******************************************************************************
  *�������Զ����һЩ��������
    *���ڣ�2020��8��19��
  ******************************************************************************
  */
    
#ifndef __MYTYPE_H
#define __MYTYPE_H

typedef unsigned char uInt8;
typedef unsigned short int uInt16;
typedef unsigned int uInt32;
typedef char sInt8;
typedef short int sInt16;
typedef int sInt32;

typedef enum
{
    mRESET = 0,
    mSET = !mRESET,
}flagStatus;

typedef enum
{
    mFALSE = 0,
    mTRUE  = !mFALSE,
}boollen;
    
typedef enum
{
    mERROR = 0,
    mSUCCESS = !mERROR,
}errorStatus;

typedef enum
{
    mEMPTY = 0,
    mNOTEMPTY =!mEMPTY,
}emptyStatus;

#endif /* ������һЩ���� */
    
