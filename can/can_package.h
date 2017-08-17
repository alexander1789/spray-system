/**
  ******************** (C) COPYRIGHT 2011 DJI **********************************
  *
  * @Project Name       : WKM2_CAN_LOADER.uvproj
  * @File Name          : can_package.h
  * @Environment        : keil mdk4.12/LPC1765/100M cclock
  * @Author&Date        : 2011-10-14 
  * @Version            : 1.10
  ******************************************************************************
  * @Description
  *	           
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_PACKAGE_H__
#define __CAN_PACKAGE_H__

/* Includes ------------------------------------------------------------------*/
/* Standlib */

/* Private define ------------------------------------------------------------*/


/* Function declaration ------------------------------------------------------*/
/* 环形缓冲区读写结构 */
uint8_t CAN_RING_BUF_RD(CAN_RING_BUF_Type *ring_buf);
void CAN_RING_BUF_WR(CAN_RING_BUF_Type *ring_buf, uint8_t DataIn);
uint32_t CAN_RING_BUF_COUNT(CAN_RING_BUF_Type *ring_buf); 
uint32_t CAN_RING_BUF_IS_FULL(CAN_RING_BUF_Type *ring_buf);
uint32_t CAN_RING_BUF_IS_EMPTY(CAN_RING_BUF_Type *ring_buf);
void CAN_RING_BUF_CLEAR(CAN_RING_BUF_Type *ring_buf);

void CAN_RING_BUF_Init(void);
CPU_INT32U CAN_RING_BUF_WR_BLOCK(CAN_RING_BUF_Type *dst,CPU_INT08U *src,CPU_INT32U cnt);
CPU_INT32U CAN_RING_BUF_RD_BLOCK(CPU_INT08U *dst,CAN_RING_BUF_Type *src,CPU_INT32U cnt);
void CAN_tx_function(LPC_CAN_TypeDef *pCANx, CAN_RING_BUF_Type *buf,CPU_INT32U index_TBS);
void CAN_SendPackage(CPU_INT08U to, CPU_INT16U len, CPU_INT16U cmd,CPU_INT08U *p_package);
void CAN_SendPackage_View(CPU_INT08U to, CPU_INT16U len,CPU_INT08U *p_package);

void CAN_rx_function_ctrl(void);

/* can send bytes */
void CAN_sendbytes_View( LPC_CAN_TypeDef *LPC_CANx, CAN_RING_BUF_Type *dst, CPU_INT08U *src, CPU_INT16U length );

/*******************  (C) COPYRIGHT 2011 DJI ************END OF FILE***********/
#endif
