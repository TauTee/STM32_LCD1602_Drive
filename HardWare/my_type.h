/**
  ******************************************************************************
  *名称：my_foc
    *描述：空间矢量控制在永磁同步电机上的实现
    *主控：STM32F405RGT6
    *日期：2020年8月19日
    *作者：张凯强
  ******************************************************************************
  *描述：自定义的一些数据类型
    *日期：2020年8月19日
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

#endif /* 定义了一些类型 */
    
