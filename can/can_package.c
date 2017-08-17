/**
  ******************** (C) COPYRIGHT 2011 DJI **********************************
  *
  * @Project Name       : BL_WKM2_MAIN.uvproj
  * @File Name          : can_package.c
  * @Environment        : keil mdk4.12/LPC1765/100M cclock
  * @Author&Date        : 2011-01-27 
  * @Version            : 1.10
  ******************************************************************************
  * @Description
  *	          
  */

/* Includes ------------------------------------------------------------------*/
#include "../cstartup/type.h"
#include "../drivers/drivers.h"
#include "../app/cfg/cfg_inc.h"
#include "can_inc.h"

/* public variables ----------------------------------------------------------*/
#if __CAN1_ENABLE__
CPU_INT08U        can1RxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};	   //
CAN_RING_BUF_Type can1RxRingBuf          = {0};

CPU_INT08U        can1Rx2Buf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};	   //
CAN_RING_BUF_Type can1Rx2RingBuf          = {0};

CPU_INT08U        can1RxUpgradeBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};	   //
CAN_RING_BUF_Type can1RxUpgradeRingBuf          = {0};

CPU_INT08U        can1RxLEDBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};	   //
CAN_RING_BUF_Type can1RxLEDRingBuf          = {0};

CPU_INT08U        can1TxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can1TxRingBuf          = {0};

CPU_INT08U        can1Tx2Buf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can1Tx2RingBuf          = {0};

CPU_INT08U        can1Tx3Buf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can1Tx3RingBuf          = {0};

CPU_INT08U        can1UpgradeAckTxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can1UpgradeAckTxRingBuf          = {0};

CPU_INT08U        can1LEDTxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can1LEDTxRingBuf          = {0};
#endif

#if __CAN2_ENABLE__
CPU_INT08U        can2RxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};	   //
CAN_RING_BUF_Type can2RxRingBuf          = {0};

CPU_INT08U        can2TxBuf[CAN_BUFSIZE] __attribute__ ((aligned (4))) = {0};
CAN_RING_BUF_Type can2TxRingBuf          = {0};
#endif

/* public Function -----------------------------------------------------------*/
/****************************** can ring buffer process ***************************/
void CAN_RING_BUF_Init(void)
{
#if __CAN1_ENABLE__	
	can1RxRingBuf.size    = CAN_BUFSIZE;
  can1RxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1RxRingBuf.wrIdx   = 0;
	can1RxRingBuf.rdIdx   = 0;
	can1RxRingBuf.pBuf    = &can1RxBuf[0];
	can1RxRingBuf.id      = CAN1_RX_ID;	   /* 接收缓冲区可以不写 */
	can1RxRingBuf.state   = 0;	   /* first state */
//CAN1 的第二个BUF
	can1Rx2RingBuf.size    = CAN_BUFSIZE;
  can1Rx2RingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1Rx2RingBuf.wrIdx   = 0;
	can1Rx2RingBuf.rdIdx   = 0;
	can1Rx2RingBuf.pBuf    = &can1Rx2Buf[0];
	can1Rx2RingBuf.id      = CAN1_RX_ID2;	   /* 接收缓冲区可以不写 */
	can1Rx2RingBuf.state   = 0;	   /* first state */

//CAN1 的第三个BUF
	can1RxUpgradeRingBuf.size    = CAN_BUFSIZE;
  can1RxUpgradeRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1RxUpgradeRingBuf.wrIdx   = 0;
	can1RxUpgradeRingBuf.rdIdx   = 0;
	can1RxUpgradeRingBuf.pBuf    = &can1RxUpgradeBuf[0];
	can1RxUpgradeRingBuf.id      = CAN1_UPGRADE_ID;	   /* 接收缓冲区可以不写 */
	can1RxUpgradeRingBuf.state   = 0;	   /* first state */
//CAN1 的第四个buf
	can1RxLEDRingBuf.size    = CAN_BUFSIZE;
  can1RxLEDRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1RxLEDRingBuf.wrIdx   = 0;
	can1RxLEDRingBuf.rdIdx   = 0;
	can1RxLEDRingBuf.pBuf    = &can1RxLEDBuf[0];
	can1RxLEDRingBuf.id      = CAN1_RX_RADAR;	   /* 接收缓冲区可以不写 */
	can1RxLEDRingBuf.state   = 0;	   /* first state */

	can1TxRingBuf.size	  = CAN_BUFSIZE;
	can1TxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1TxRingBuf.wrIdx   = 0;
	can1TxRingBuf.rdIdx   = 0;
	can1TxRingBuf.pBuf    = &can1TxBuf[0];
	can1TxRingBuf.id      = CAN1_TX_ID;     /* 中转设备接收被升级的CAN设备通道 */
	can1TxRingBuf.state   = 0;
	
	can1Tx2RingBuf.size	  = CAN_BUFSIZE;
	can1Tx2RingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1Tx2RingBuf.wrIdx   = 0;
	can1Tx2RingBuf.rdIdx   = 0;
	can1Tx2RingBuf.pBuf    = &can1Tx2Buf[0];
	can1Tx2RingBuf.id      = CAN1_TX_ID2;     /* 中转设备接收被升级的CAN设备通道 */
	can1Tx2RingBuf.state   = 0;
	
	can1Tx3RingBuf.size	  = CAN_BUFSIZE;
	can1Tx3RingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1Tx3RingBuf.wrIdx   = 0;
	can1Tx3RingBuf.rdIdx   = 0;
	can1Tx3RingBuf.pBuf    = &can1Tx3Buf[0];
	can1Tx3RingBuf.id      = CAN1_TX_ID3;     /* 中转设备接收被升级的CAN设备通道 */
	can1Tx3RingBuf.state   = 0;
	
	can1UpgradeAckTxRingBuf.size	  = CAN_BUFSIZE;
	can1UpgradeAckTxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1UpgradeAckTxRingBuf.wrIdx   = 0;
	can1UpgradeAckTxRingBuf.rdIdx   = 0;
	can1UpgradeAckTxRingBuf.pBuf    = &can1UpgradeAckTxBuf[0];
	can1UpgradeAckTxRingBuf.id      = CAN1_UPGRADE_ACK_ID;     /* can升级时返回的CAN ID */
	can1UpgradeAckTxRingBuf.state   = 0;
    
	can1LEDTxRingBuf.size	  = CAN_BUFSIZE;
	can1LEDTxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can1LEDTxRingBuf.wrIdx   = 0;
	can1LEDTxRingBuf.rdIdx   = 0;
	can1LEDTxRingBuf.pBuf    = &can1LEDTxBuf[0];
	can1LEDTxRingBuf.id      = CAN1_TX_LED_ID;     /* can升级时返回的CAN ID */
	can1LEDTxRingBuf.state   = 0;    
#endif
#if __CAN2_ENABLE__	
	can2RxRingBuf.size    = CAN_BUFSIZE;
    can2RxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can2RxRingBuf.wrIdx   = 0;
	can2RxRingBuf.rdIdx   = 0;
	can2RxRingBuf.pBuf    = &can2RxBuf[0];
	can2RxRingBuf.id      = CAN2_RX_ID;	   /* 接收缓冲区可以不写 */
	can2RxRingBuf.state   = 0;	   /* first state */

	can2TxRingBuf.size	  = CAN_BUFSIZE;
	can2TxRingBuf.mask    = ( CPU_INT32U )( CAN_BUFSIZE - 1 );
	can2TxRingBuf.wrIdx   = 0;
	can2TxRingBuf.rdIdx   = 0;
	can2TxRingBuf.pBuf    = &can2TxBuf[0];
	can2TxRingBuf.id      = CAN2_TX_ID;     /* 中转设备接收被升级的CAN设备通道 */
	can2TxRingBuf.state   = 0;
#endif	
}

/* 环形缓冲区操作 */
uint8_t CAN_RING_BUF_RD(CAN_RING_BUF_Type *ring_buf)
{
    uint8_t tmp;
	tmp = (*ring_buf).pBuf[(*ring_buf).rdIdx & (*ring_buf).mask];
	(*ring_buf).rdIdx++;

	return tmp;
}

void CAN_RING_BUF_WR(CAN_RING_BUF_Type *ring_buf, uint8_t DataIn)
{
	(*ring_buf).pBuf[(*ring_buf).wrIdx&(*ring_buf).mask] = DataIn;
	(*ring_buf).wrIdx++;
}

uint32_t CAN_RING_BUF_IS_FULL(CAN_RING_BUF_Type *ring_buf)
{ 
	/* 注意1为逻辑真值,0为逻辑假值,这里要与DEF_TRUE和DEF_FALSE的值相区别 */
	return ( (((*ring_buf).wrIdx+1)&(*ring_buf).mask) == ((*ring_buf).rdIdx&(*ring_buf).mask) );
}

uint32_t CAN_RING_BUF_IS_EMPTY(CAN_RING_BUF_Type *ring_buf)
{
    /* 注意1为逻辑真值,0为逻辑假值,这里要与DEF_TRUE和DEF_FALSE的值相区别 */
	return ( ((*ring_buf).wrIdx&(*ring_buf).mask) == ((*ring_buf).rdIdx&(*ring_buf).mask) );	
}

uint32_t CAN_RING_BUF_COUNT(CAN_RING_BUF_Type *ring_buf)
{
	return ( ((*ring_buf).wrIdx-(*ring_buf).rdIdx) & (*ring_buf).mask );
}

void CAN_RING_BUF_CLEAR(CAN_RING_BUF_Type *ring_buf)
{
	ring_buf->rdIdx = ring_buf->wrIdx = 0;
}

/*
 * param[in]   dst: buf be written to
 * param[out]  src: pointer to source buf
 * param[in]   cnt: the number of data to be written
 * Return      the number really to be written
 * Notes       read data to can ringbuf
 */
uint32_t CAN_RING_BUF_WR_BLOCK(CAN_RING_BUF_Type *dst,uint8_t *src,uint32_t cnt)
{
    uint32_t tmp_cnt;

	/* 注意:计算可以写入的数量 */
	tmp_cnt = (*dst).size - CAN_RING_BUF_COUNT( dst ); 
	
	if(tmp_cnt>cnt) /* 缓冲区中至少有cnt+1个空白位置 */
	{
	    tmp_cnt = cnt;
	}
	else
	{
	    tmp_cnt = 0;
		return tmp_cnt; /* 防止缓冲区写满 */
	}

	while(tmp_cnt--)
	{
		(*dst).pBuf[(*dst).wrIdx & (*dst).mask] = *src;
	    (*dst).wrIdx++;
		src++;    
	}

	return cnt;
}

/*
 * param[in]   dst: read data form src to dst
 * param[out]  src: buf to be read
 * param[in]   cnt: the number of data to be read 
 * Return      the number really to be written
 * Notes       read data form can ringbuf
 */
uint32_t CAN_RING_BUF_RD_BLOCK(uint8_t *dst,CAN_RING_BUF_Type *src,uint32_t cnt)
{
	uint32_t tmp_cnt;

	tmp_cnt = CAN_RING_BUF_COUNT( src );
	cnt     = cnt<=tmp_cnt?cnt:tmp_cnt;

	tmp_cnt = cnt;
	while(tmp_cnt--)
	{
		*dst = (*src).pBuf[(*src).rdIdx & (*src).mask];
	   (*src).rdIdx++;
	   dst++;
	}

	return cnt;
}

/****************************** CAN send package process ***************************/

void CAN_tx_function(LPC_CAN_TypeDef *pCANx, CAN_RING_BUF_Type *buf,CPU_INT32U index_TBS)
{
	CPU_INT32U cnt = 0;
	CPU_INT32U can_status = 0;

	cnt = CAN_RING_BUF_COUNT(buf);
	if(cnt <= 0)
	{
	    return;
	}

	can_status = (pCANx->SR & 0x000c0c0c);
	if(index_TBS == 1)
	{
		if( (can_status & 0x0c) != 0x0c)
		{
			return;
		}
		CAN_tx_action(pCANx,buf,cnt,1);
	}else if( index_TBS == 2)
	{
		if( (can_status & 0x0c00) != 0x0c00)
		{
			return;
		}
		CAN_tx_action(pCANx,buf,cnt,2);
	}else if( index_TBS == 3)
	{
		if((can_status & 0x0c0000) != 0x0c0000)
		{
			return;
		}
		CAN_tx_action(pCANx,buf,cnt,3);
	}else if(index_TBS == 4) // 如果有这么多数据则一次填入三个寄存器
	{
		if((can_status & 0x040404) != 0x040404)
		{
			return;
		}
		CAN_tx_action_all(pCANx,buf,cnt);
	}
}

/**************************** VIEW send package ***********************/
void CAN_sendbytes_View(LPC_CAN_TypeDef *pCANx, CAN_RING_BUF_Type *dst, CPU_INT08U *src, CPU_INT16U length)
{

	if( CAN_RING_BUF_WR_BLOCK(dst,src,length) < length)
	{
	   // todo stuff tail data error
//	   uart_printf( "can buf over flow!\r\n" );
	}


	if( (pCANx->GSR&(1<<2)) && ((pCANx->SR&0x040404) == 0x040404) )
	{
	    CAN_tx_function(pCANx,dst,4);
	}
}

/*******************  (C) COPYRIGHT 2011 DJI ************END OF FILE***********/


