/**
  ******************** (C) COPYRIGHT 2012 DJI **********************************
  *
  * @Project Name       : BL_WKM2_MAIN.uvproj
  * @File Name          : cfg_handler.c
  * @Environment        : keil mdk4.12/LPC1765/100M cclock
  * @Author&Date        : 2012-05-28 
  * @Version            : 1.00
  ******************************************************************************
  * @Description
  *	    Begginning of application   
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "../../cstartup/type.h"
#include "../../IAP/sbl_iap.h"
#include "../../IAP/sbl_config.h"
//#include "../cm_at88/cm_app.h"
#include "../aes/aes.h"
#include "../../drivers/drivers.h"
#include "../md5/MF_MD5.h"
#include "../crc/MF_CRC8.h"
#include "../crc/MF_CRC16.h"
#include "cfg_inc.h"
#include "../../can/can_inc.h"
#include "../../usb/vcom_app.h"
#include <math.h>
//#define FLOW_CALI_TEST
static uint8_t pwm_present_value = 0;
int32_t g_cfg_end_point = EP_NULL;

static int32_t cfg_package_index_front = 0;
static uint32_t cfg_write_offset = 0;
static uint32_t cfg_file_size = 0;
static uint8_t tmpBuf[512] __attribute__((aligned(4)));
extern uint8_t get_MedeRunoutFlag(void);
extern volatile uint32_t g_device_status;

void cmd_headler_send_command_v1( uint8_t *p_buf, uint16_t len )
{
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	if( p_cmd->vl.length != len ) return;
	Append_CRC8_Check_Sum( p_buf, 4 );
	Append_CRC16_Check_Sum( p_buf, p_cmd->vl.length );

    switch( g_cfg_end_point ) {
#if __USB_ENABLE__
    case EP_USB:
        VCOM_sendpackage( ( const char * )p_buf, p_cmd->vl.length );
        break;
#endif    
#if __CAN1_ENABLE__
    case EP_CAN1:
        CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, p_buf, p_cmd->vl.length );
        break;
    case EP_CAN1_LED:
        CAN_sendbytes_View( LPC_CAN1, &can1LEDTxRingBuf, p_buf, p_cmd->vl.length );
        break;
    case EP_CAN1_LB:
        CAN_sendbytes_View( LPC_CAN1, &can1TxRingBuf, p_buf, p_cmd->vl.length );
        break;
#endif
#if __CAN2_ENABLE__
    case EP_CAN2:
        CAN_sendbytes_View( LPC_CAN2, &can2TxRingBuf, p_buf, p_cmd->vl.length );
        break;
#endif
#if __UART0_ENABLE__
		case EP_UART0:
				UART0_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
#if __UART1_ENABLE__
		case EP_UART1:
				UART1_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
#if __UART2_ENABLE__
		case EP_UART2: 
			UART2_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
#if __UART3_ENABLE__
case EP_UART3: 
			UART3_send_pack( p_buf , p_cmd->vl.length );
		break ;
#endif		
    default:
        break;
    }
}

void cmd_handler_init_header_v1( uint8_t *p_buf, uint16_t len )
{
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	
	uint8_t tmp0 = p_cmd->sender.sender_index;
	uint8_t tmp1 = p_cmd->sender.sender_id;
	
	p_cmd->sender.sender_index = p_cmd->receiver.receiver_index;
	p_cmd->sender.sender_id = p_cmd->receiver.receiver_id;
	p_cmd->receiver.receiver_index = tmp0;
	p_cmd->receiver.receiver_id = tmp1;
	
	p_cmd->type.cmd_type = 1;
	p_cmd->type.cmd_ack = 0;
}

void cmd_headler_send_command_v0( uint8_t *p_buf, uint16_t len )
{
	cmd_header_v0_t *p_cmd = ( cmd_header_v0_t * )p_buf;
	if( p_cmd->vl.length != len ) return;
	Append_CRC8_Check_Sum( p_buf, 4 );
	Append_CRC16_Check_Sum( p_buf, p_cmd->vl.length );

	    switch( g_cfg_end_point ) {
#if __USB_ENABLE__
    case EP_USB:
        VCOM_sendpackage( ( const char * )p_buf, p_cmd->vl.length );
        break;
#endif    
#if __CAN1_ENABLE__
    case EP_CAN1:
        CAN_sendbytes_View( LPC_CAN1, &can1TxRingBuf, p_buf, p_cmd->vl.length );
        break;
#endif
#if __CAN2_ENABLE__
    case EP_CAN2:
        CAN_sendbytes_View( LPC_CAN2, &can2TxRingBuf, p_buf, p_cmd->vl.length );
        break;
#endif
#if __UART0_ENABLE__
		case EP_UART0:
				UART0_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
#if __UART1_ENABLE__
		case EP_UART1:
				UART1_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
#if __UART3_ENABLE__
		case EP_UART3:
				UART3_send_pack( p_buf , p_cmd->vl.length );
			break ;
#endif
    default:
        break;
    }
}

uint32_t debug_foc_version = 0;
void cmd_handler_device_info( uint8_t *p_buf, uint16_t len )
{
  uint8_t result = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_device_info_ack_t *p_info = ( cmd_device_info_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_cmd->type.cmd_type == 0 ){
		p_info->command_version.major = 0;
		p_info->command_version.minor = 1;
		
	 hardware_id_read_flash( p_info->hardware_ver, 16 );
		p_info->loader_ver = sbl_get_loader_version();
			if( p_info->loader_ver == 0xFFFFFFFF ) {
					result = 0xFF;
			}

		p_info->firmware_ver = ArgBoardFirmwareVersion;
		p_info->command_set = 0x00000001;

		if( p_cmd->type.cmd_ack != 0 ) {
			cmd_handler_init_header_v1( p_buf, len );
			
			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_device_info_ack_t ) + 2;
			p_info->result = result;

			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
            
            uart_printf( 0 ,"send ack:%d\r\n",g_current_tick);
		}
	}else if( p_cmd->type.cmd_type == 1 ){
		//如果是电调回应的
		if( p_cmd->sender.sender_id == 12 && ( p_cmd->sender.sender_index == 0x07 )){
			//判断得到的电调的版本号如果不是25，就认为不是本公司的电调，禁止启动
			
			 debug_foc_version  = p_info->firmware_ver;
//			 if( ((p_info->firmware_ver & 0x0000ff00)>>8) == 25 ){
//						g_MotorCheckFlag = 1;
			 uart_printf( 0 ,"fcc ver success\r\n" );
//			 }else{
//					uart_printf( 0 ,"fcc ver error\r\n" );
//			 }
		}
	}
}

void cmd_handler_entry_upgrade( uint8_t *p_buf, uint16_t len )
{
	uint8_t result = 0;
	uint32_t i = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_entry_upgrade_req_t *p_eur = ( cmd_entry_upgrade_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_entry_upgrade_ack_t *p_eua = ( cmd_entry_upgrade_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_eur->encrypt != 0 ) return;
	
	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
		
		erase_programming_done_flag();// 擦除标志位
		if( *( ( uint32_t * )PROGRAM_FLAG_ADDR ) == 0x0 )
		{
			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_entry_upgrade_ack_t ) + 2;
			p_eua->result = result;
			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );	
			uart_printf(0,"enter upgrade\r\n");
			i = 50000;
			while(i--);
			WDT_Feed();  /*看门狗溢出时间同步*/
			//关中断,停止喂狗,看门狗复位进入bootloader模式
			while(1);
		}
	}
}

void cmd_handler_start_upgrade( uint8_t *p_buf, uint16_t len )
{
	uint8_t result = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_start_upgrade_req_t *p_sur = ( cmd_start_upgrade_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_start_upgrade_ack_t *p_sua = ( cmd_start_upgrade_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_sur->encrypt != 0 ) return;
	
	if( p_sur->firmware_size > USER_FLASH_END - USER_FLASH_START ) {
		result = 0xF1;
	} else {
		cfg_package_index_front = -1;
		cfg_write_offset = 0;
		cfg_file_size = p_sur->firmware_size;
		
		aesDecInit();
		if( !erase_user_area() ) {
			result = 0xF3;
		}
	}

	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
		
		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_start_upgrade_ack_t ) + 2;
		p_sua->result = result;
		p_sua->data_size = 256;
		
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

void cmd_handler_data_upgrade( uint8_t *p_buf, uint16_t len )
{
	uint8_t result = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_data_upgrade_req_t *p_dur = ( cmd_data_upgrade_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_data_upgrade_ack_t *p_dua = ( cmd_data_upgrade_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_dur->encrypt != 0 && p_dur->encrypt != 1 ) return;
	
	if( p_dur->package_index != cfg_package_index_front + 1 ) 
	{
		result = 0xF0;
	} 
	else 
	{ // write data to flash
		if( p_dur->package_length != 256 ) {
			result = 0xF7;
		} 
		else 
		{
			if( cfg_write_offset > cfg_file_size )
			{
					result = 0xF1;
			} 
			else 
			{
				memcpy( &tmpBuf[0], &p_dur->data[0], p_dur->package_length );
				if( p_dur->encrypt == 1 )
				{
					aesDecryptBlock( &tmpBuf[0], p_dur->package_length );
				}
				
				if( write_flash( ( uint32_t * )( USER_FLASH_START + cfg_write_offset ), &tmpBuf[0], p_dur->package_length ) != 0 ) {
					result = 0xF4;
				}	
				
				if( !compare_data( ( uint32_t )( USER_FLASH_START + cfg_write_offset ), ( uint32_t )&tmpBuf[0], p_dur->package_length ) ) {
					result = 0xF4;
				}
				
				if( result == 0 ) {
					cfg_write_offset += p_dur->package_length;
					cfg_package_index_front += 1;
				}
			}
		}
	}
	
	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
	
		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_data_upgrade_ack_t ) + 2;
		p_dua->result = result;
		if( p_dua->result != 0 ) {
			p_dua->package_index = cfg_package_index_front;
		}
	
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

void cmd_handler_end_upgrade( uint8_t *p_buf, uint16_t len )
{
	uint8_t result = 0, md5Digest[16], i = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_end_upgrade_req_t *p_eur = ( cmd_end_upgrade_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_end_upgrade_ack_t *p_eua = ( cmd_end_upgrade_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_eur->encrypt != 0 ) return;
	
	MD5Init( &md5 );
	MD5Update( &md5, ( uint8_t * )USER_FLASH_START, cfg_file_size );
	MD5Final( &md5, (void *)md5Digest );

	for( i = 0; i < 16; i++ ) {
		if( md5Digest[i] != p_eur->md5[i] ) {
			result = 0xF2;
			break;
		}
	}

	if( i == 16 ) {
		write_programming_done_flag();
	}

	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );

		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_end_upgrade_ack_t ) + 2;
		p_eua->result = result;
	
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

void cmd_handler_reboot( uint8_t *p_buf, uint16_t len )
{
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_reboot_req_t *p_rr = ( cmd_reboot_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_reboot_ack_t *p_ra = ( cmd_reboot_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );

    if( p_rr->type == 0 || p_rr->type == 1 ) { 
        WDT_UpdateTimeOut( p_rr->delay_ms );
        WDT_Lock();
    }
	
	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
	
		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_reboot_ack_t ) + 2;
		p_ra->result = 0;
	
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

void cmd_handler_status_report( uint8_t *p_buf, uint16_t len )
{
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
//	cmd_status_report_req_t *p_srr = ( cmd_status_report_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_status_report_ack_t *p_sra = ( cmd_status_report_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
	
		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_status_report_ack_t ) + 2;
		p_sra->result = 0;
		
		p_sra->ver.major = 0;
		p_sra->ver.minor = 0;
		
		p_sra->status = g_device_status;
		
		{
			uint8_t cnt ;
		for( cnt = 1; cnt<5 ; cnt++ ){
//			if( stFocSetAddr[cnt].access_flag != 0 ){
//				MoterTogger(stFocSetAddr[cnt].id);
//			}
		}
		}
		
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

void cmd_handler_set_version( uint8_t *p_buf, uint16_t len )
{
  uint8_t result = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_set_version_req_t *p_svr = ( cmd_set_version_req_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	cmd_set_version_ack_t *p_sva = ( cmd_set_version_ack_t * )( p_buf + sizeof( cmd_header_v1_t ) );
	
    if( !hardware_id_write_flash( p_svr->hardware_ver, 16 ) ) {
        result = 0xF4;
    }
    
	if( p_cmd->type.cmd_ack != 0 ) {
		cmd_handler_init_header_v1( p_buf, len );
	
		p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_set_version_ack_t ) + 2;
		p_sva->result = result;
		
		p_sva->ver.major = 0;
		p_sva->ver.minor = 0;
		
        hardware_id_read_flash( p_sva->hardware_ver, 16 );
	
		cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	}
}

uint8_t RecordKeyPress ;
void cmd_hander_motor_process( uint8_t *p_buf, uint16_t len )
{
//	static uint32_t get_tick =0;
//	uint8_t cnt ;
	cmd_camera_remote_req_t *data  ;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	data = ( cmd_camera_remote_req_t * )((&p_cmd->id) + 1) ;
	//如果是录像按键按下了	
	if( data->cmdtype == 2 )
	{
		RecordKeyPress = 1 ;
		uart_printf( 0 ,"carmer press\r\n" );
	}
	else if( data->cmdtype == 7 ){
		uart_printf( 0 ,"carmer release\r\n" );
	}
}

void set_pwm_present(spray_sys_esc_status_t foc_status)
{
  float pwm_value = (float)(foc_status.flow_speed - MIN_FLOW)*0.05*100;
  if(foc_status.enable_flag == 1)
		pwm_present_value = pwm_value;
	else
		pwm_present_value = 0;
}

uint8_t get_pwm_present_value(void)
{
  return pwm_present_value;
}


//uint8_t debug_flow_value = 5;
uint8_t flow_cali_result = flow_cali_idle;
void set_flow_cali_result(uint8_t value)
{
   flow_cali_result = value;
}
//得到飞控对水泵的控制
uint16_t Last_Capacity;
uint8_t STOP_SPRAY_FLAG = 0;
spray_sys_esc_status_t FOC_1_STATUS;
spray_sys_esc_status_t FOC_2_STATUS;
extern uint16_t idx_7_freq;
//uint8_t spary_speed = 0;


uint8_t debug_idx_7_freq;
uint8_t foc1_e,foc1_v;
uint8_t foc2_e,foc2_v;

extern spray_system_flow_cali_para_t spray_system_flow_cali_para1;
extern uint8_t flow_cali_enbale_flag;
uint8_t level_status_by_radar = 0; // 默认低电平

//#define Normal_Work     0
//#define Pump_Blocked    1
//#define Pipe_With_Air   2
//#define Pressure_Error  3
//#define Flowmeter_Error 4
// 向飞控推送农机的喷洒系统的状态
 void spary_sys_status_push( void )
{
	uint8_t tmpBuf[40] = {0};
	static uint8_t cali_count = 0;
	static uint16_t seq=0,count = 0;
	cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
	cmd_spary_sys_status_push_t *p_data = (cmd_spary_sys_status_push_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x03; // 飞控
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 0;
	p_cmd->type.cmd_ack = 0;
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0x05;
	p_cmd->id = 0x41;
	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_spary_sys_status_push_t)+ CRC_16_LENGTH;
	
  //电调状态	
  p_data->pump_esc_status[0].health_flag = ESC_Normal;
	p_data->pump_esc_status[1].health_flag = ESC_Normal; 
  //其余电调状态保留
	
	//水泵的工作状态
	p_data->pump_presure_state.pump1_state = get_pump1_state(); 
	p_data->pump_presure_state.pump2_state = get_pump2_state();
	
	
	if(flow_cali_result == flow_cali_complted ||flow_cali_result == flow_cali_failed)	
	{
		cali_count++; //推送三秒标定结果后状态清0
		if(cali_count > 15)
		{  // 重置状态
			 flow_cali_result = flow_cali_idle; 
			 cali_count = 15;
		}
	}
	else
	{
			cali_count = 0;
	}

	if(FOC_1_STATUS.enable_flag == 1)	//流量参数
		p_data->flow_para[0].cur_flow = FOC_1_STATUS.flow_speed * 0.2;//流量计精度计算 
	else
		p_data->flow_para[0].cur_flow = 0;
	
	p_data->flow_para[0].cali_result = flow_cali_result;
	
	if(FOC_2_STATUS.enable_flag == 1)	//流量参数
	 p_data->flow_para[1].cur_flow =  FOC_2_STATUS.flow_speed * 0.2;
	else
	 p_data->flow_para[1].cur_flow = 0;
	
	p_data->flow_para[1].cali_result = flow_cali_result;
	
	p_data->plug_temp_level.XT100= Get_XT100_State();
  p_data->plug_temp_level.XT90 = Get_XT90_State();
	
	//剩余容量
	if(get_MedeRunoutFlag() == 1)
	{
	   p_data->remaining_capacity = 0;
	}
	else // 没触发液位传感器，给已固定值
	{
		if(get_current_capacity_d()*0.01 < 15)
		{
		 	 p_data->remaining_capacity = 15; //将已经知道喷洒的赋值 单位 0.1L   remain:1.5L get_current_capacity_d()*0.01
		}
		else
		{
			p_data->remaining_capacity = get_current_capacity_d()*0.01;
			if(p_data->remaining_capacity <= 15)
				p_data->remaining_capacity = 15;
		}
  }
	
//  /************ APP调试信息打印 ****************/
//	p_data->debug_esc_status[0].enable_flag =  FOC_1_STATUS.enable_flag;
//	if(FOC_1_STATUS.enable_flag == 1)
//	 p_data->debug_esc_status[0].flow_speed  =  FOC_1_STATUS.flow_speed;
//	else
//	 p_data->debug_esc_status[0].flow_speed = 0;
//	
//	p_data->debug_esc_status[1].enable_flag =  FOC_2_STATUS.enable_flag;
//	if(FOC_2_STATUS.enable_flag == 1)
//	 p_data->debug_esc_status[1].flow_speed  = FOC_2_STATUS.flow_speed;
//	else
//	 p_data->debug_esc_status[1].flow_speed  = 0;
//	
//	
//	
//	p_data->debug_pump1_foc_value = get_foc_throttle_value(FOC_1);
//	p_data->debug_pump1_press_value = get_press_value(Press_Sensor_0);

//	p_data->debug_pump2_foc_value = get_foc_throttle_value(FOC_2);
//	p_data->debug_pump2_press_value = get_press_value(Press_Sensor_1);
//  p_data->debug_flow_freq = get_cali_flow_freq();
//	p_data->level_status = level_status_by_radar;

	count++;
	p_data->count = count;
	Append_CRC8_Check_Sum( tmpBuf, 4 );
	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
//	uart_printf( 2, "p_data->remaining_capacity = %d \r\n",p_data->remaining_capacity);
}


uint8_t  FOC1_MUTEX_FLAG = 1,FOC2_MUTEX_FLAG = 1;;
void SET_FOC1_MUTEX_FLAG(uint8_t state)
{
  FOC1_MUTEX_FLAG = state;
}

void SET_FOC2_MUTEX_FLAG(uint8_t state)
{
  FOC2_MUTEX_FLAG = state;
}

//得到飞控对水泵的控制
extern void set_MedeRunoutFlag(uint8_t state);
uint8_t residual_volume_warning = 0;//默认是剩药的
void set_residual_volume_warning_mode(uint8_t mode)//默认是剩药
{
  residual_volume_warning = mode;
}

spray_sys_esc_status_t FOC_1_STATUS;
spray_sys_esc_status_t FOC_2_STATUS;

cmd_spary_sys_esc_ctrl_t debug_sys_esc_ctrl;


void cmd_handler_get_fc_statu(  uint8_t *p_buf, uint16_t len )
{
	cmd_spary_sys_esc_ctrl_t *p_data = ( cmd_spary_sys_esc_ctrl_t*)( p_buf + sizeof( cmd_header_v1_t ) );

  FOC_1_STATUS.enable_flag =  p_data->spray_cmd[0].enable_flag;
  FOC_1_STATUS.flow_speed  =  p_data->spray_cmd[0].flow_speed; // *0.02 表示实际流量
	
  FOC_2_STATUS.enable_flag =  p_data->spray_cmd[3].enable_flag;
  FOC_2_STATUS.flow_speed  =  p_data->spray_cmd[3].flow_speed;// 
	
  #ifdef FLOW_CALI_TEST
		FOC_1_STATUS.valid_spray =  1;//p_data->valid_spray;
		FOC_2_STATUS.valid_spray =  1;// p_data->valid_spray;
	#else
		FOC_1_STATUS.valid_spray =  p_data->valid_spray;//p_data->valid_spray;
		FOC_2_STATUS.valid_spray =  p_data->valid_spray;// p_data->valid_spray;
	#endif
	memcpy(&debug_sys_esc_ctrl,p_data,sizeof(cmd_spary_sys_esc_ctrl_t));

  // 如果没有泄压前，以同一速度泄压
  if(get_capacity_flag_1L() == 1) // 触发发液位开关
	{
	  if(get_current_capacity_d() - Last_Capacity > 450)//触发后再喷洒350ml执行无药报警
		{
		  STOP_SPRAY_FLAG = 1;
		}
	}
	else
	{
		 set_MedeRunoutFlag(0); // 浮子浮上来默认有药
	   STOP_SPRAY_FLAG = 0;
	   Last_Capacity = get_current_capacity_d();
	}
	


	if(STOP_SPRAY_FLAG == 0 || residual_volume_warning == No_Liquid) // 触发液位开关后停止喷洒
	{		
		if( FOC_1_STATUS.enable_flag ==  FOC_2_STATUS.enable_flag 
			 && FOC_1_STATUS.flow_speed ==  FOC_2_STATUS.flow_speed) // 四个同时喷洒
		{
		   if(FOC1_MUTEX_FLAG == 1 && FOC2_MUTEX_FLAG == 1)
				 Set_Foc_Value_By_Flow_Double_Pump(FOC_1_STATUS);
		}
		else // 两个泵作业
		{
			if( FOC_1_STATUS.enable_flag == STOP_SPARY || FOC_1_STATUS.flow_speed == 0)
			{
				if(FOC2_MUTEX_FLAG == 1)
				{
					Set_Foc_Value_By_Flow_Single_Pump(FOC_2,FOC_2_STATUS); //两个泵工
			
				}
			}
			else 
			{
				if(FOC1_MUTEX_FLAG == 1)
				{
					Set_Foc_Value_By_Flow_Single_Pump(FOC_1,FOC_1_STATUS);
	
				}
			}
		}
	}
	else if(residual_volume_warning == Small_Liquid) //液位报警模式是剩余药量的模式
	{
		Set_Throttle_Value_1(MIN_CNT);
		Set_Throttle_Value_2(MIN_CNT);
		set_MedeRunoutFlag(1);// 无药报警
	}
	

	
//	uart_printf( 0, "FOC_1_flag = %d,FOC_1_speed = %d,FOC_2_flag  = %d,FOC_2_speed = %d\r\n" ,FOC_1_STATUS.enable_flag,FOC_1_STATUS.flow_speed,FOC_2_STATUS.enable_flag,FOC_2_STATUS.flow_speed );
}


//uint8_t get_cali_capacity(void)
//{
//  return capacity_app_set;
//}


void cmd_handler_flow_cali(  uint8_t *p_buf, uint16_t len)
{
  uint16_t capacity_app_set = 0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_flow_cali_t *p_rr = ( cmd_flow_cali_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  cmd_flow_cali_ack_t *p_ra = (cmd_flow_cali_ack_t *)( p_buf + sizeof( cmd_header_v1_t ) );
	
	capacity_app_set = p_rr->cali_capacity*1000; //得到标定的流量 ml
	set_cali_capaciy(capacity_app_set);
	//应答数据
	#ifdef FLOW_CALI_TEST
	//应答数据
	if((p_rr->cali_capacity != 0 )
	    || p_rr->cali_capacity == 0)	
	#else
		if(( FOC_1_STATUS.valid_spray == 0 && p_rr->cali_capacity != 0 ) 
	    || p_rr->cali_capacity == 0)	
	#endif	
	 {	
		 if(p_rr->cali_capacity != 0)
		 {
		   set_flow_cali_flag(START_FLOW_CALI);
			 flow_cali_result = flow_cali_working;
		 }
		 else 	//如果标定流量为 0,取消标定
		 {
			 set_flow_cali_flag(CANCEL_FLOW_CALI);
			 flow_cali_result = flow_cali_idle;
		 }
		 
		if( p_cmd->type.cmd_ack != 0 ) 
		{
			cmd_handler_init_header_v1( p_buf, len );
			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_flow_cali_ack_t ) + CRC_16_LENGTH;
			p_ra->ack_code = 0;
			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
		}
  }
	uart_printf( 2, "capacity_app_set =  %d \r\n" ,capacity_app_set);
}

uint8_t get_level_status_by_radar(void)
{
  return level_status_by_radar;
}

uint8_t level_init_flag = 0,level_init_count = 0;
void cmd_handler_level_switch(  uint8_t *p_buf, uint16_t len)  // 通过雷达液位数据
{
//	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;

	cmd_level_switch_t *p_rr = ( cmd_level_switch_t*)( p_buf + sizeof( cmd_header_v1_t ) );
	level_status_by_radar = p_rr->level_status; //
	// 第一次上电，根据浮子位置判断是否有药,低电平为无药
	if(level_init_flag == 0)
	{
		if(level_status_by_radar == 0)
		{
		  level_init_count++;
			if(level_init_count >= 3)
			{	  
				set_MedeRunoutFlag(1);
        level_init_flag = 1;				
			}
		}
    else
		{
			level_init_count = 0;
			level_init_flag = 1;
		}
	}
	uart_printf( 2, "cmd_handler_level_switch =  %d \r\n" ,level_status_by_radar);
}


void cmd_handler_residual_volume_warning(  uint8_t *p_buf, uint16_t len)  // 
{
	uint8_t buf[1] = {0};
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_residual_volume_t *p_rr = ( cmd_residual_volume_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  cmd_residual_volume_ack_t *p_ra = (cmd_residual_volume_ack_t *)( p_buf + sizeof( cmd_header_v1_t ) );
	
	if(p_rr->read_or_write == 0)
	{
	  //从flash读取 并应答给APP
		 cali_volume_para_read_flash(buf, 1); 
		 if(buf[0] == 0xff)
		   buf[0]  = Small_Liquid; // 剩余药量
		 p_ra->ack_waring_mode = buf[0];
	}	
  else
	{
		buf[0] = p_rr->waring_mode;
		set_residual_volume_warning_mode(p_rr->waring_mode);
		residual_volume_para_write_flash(buf,1);
	  p_ra->ack_waring_mode = 0xff;// 表示无效数据
	}		
	 
	if( p_cmd->type.cmd_ack != 0 ) 
	{
			cmd_handler_init_header_v1( p_buf, len );
			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_residual_volume_ack_t ) + CRC_16_LENGTH;
			p_ra->ack_code = 0;
			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	 }
}

cmd_flowmeter_K_control_t debug_control;
void cmd_handler_flowmeter_K_control(uint8_t *p_buf, uint16_t len)
{
	uint8_t buf[1] = {0};
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	cmd_flowmeter_K_control_t *p_rr = ( cmd_flowmeter_K_control_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  cmd_flowmeter_ack_t *p_ra = (cmd_flowmeter_ack_t *)( p_buf + sizeof( cmd_header_v1_t ) );
	memcpy(&debug_control,p_rr,sizeof(cmd_flowmeter_K_control_t));
  if(p_rr->read_or_write == 0)
	{
	 // 从flash读取 并应答给APP
		flowmeter_para_read_flash(buf, 1);
		if(buf[0] == 0xff)
		   buf[0]  = 0; 
    p_ra->ack_flowmeter_value = buf[0];
	}	
  else
	{
		buf[0] = p_rr->flowmeter_value;
    flowmeter_para_write_flash(buf,1);
	}
	if( p_cmd->type.cmd_ack != 0 ) 
	{
			cmd_handler_init_header_v1( p_buf, len );
			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_flowmeter_ack_t ) + CRC_16_LENGTH;
			p_ra->ack_code = 0;
			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
	 }	
}


//cmd_spray_results_t debug_spray_results;
//void cmd_handler_spray_results_corrected(uint8_t *p_buf, uint16_t len) 
//{
//	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
//	cmd_spray_results_t *p_rr = (cmd_spray_results_t *)( p_buf + sizeof( cmd_header_v1_t ) );
//  cmd_spray_results_ack_t *p_ra = (cmd_spray_results_ack_t *)( p_buf + sizeof( cmd_header_v1_t ) );
//	
//		 
//	memcpy(&debug_spray_results,p_rr,sizeof(debug_spray_results));
//	 
//	if( p_cmd->type.cmd_ack != 0 ) 
//	{
//			cmd_handler_init_header_v1( p_buf, len );
//			p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_residual_volume_ack_t ) + CRC_16_LENGTH;
//			p_ra->ack_code = 0;
//			cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
//	 }
//	 uart_printf( 2, "cmd_handler_residual_volume_warning  \r\n" );
//}

//// 向APP 推送流量计的标定结果
//void flow_cali_result_push( uint8_t cali_result)
//{
//	uint8_t tmpBuf[ sizeof(cmd_header_v1_t) + sizeof(cmd_flow_cali_completed_t)+ CRC_16_LENGTH] = { 0 };
//	static uint16_t seq=0;
//	cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
//	cmd_flow_cali_completed_t *p_data = (cmd_flow_cali_completed_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

//	p_cmd->sof = 0x55;
//	p_cmd->vl.version = 1;
//	p_cmd->sender.sender_id = DEVICE_ID;
//	p_cmd->sender.sender_index = DEVICE_INDEX;
//	p_cmd->receiver.receiver_id = 0x02; // 飞控
//	p_cmd->receiver.receiver_index = 0;
//	p_cmd->seqnum = seq++;
//	p_cmd->type.cmd_type = 0;
//	p_cmd->type.cmd_ack = 0;
//	p_cmd->type.encrypt_type = 0;
//	p_cmd->set = 0x09;
//	p_cmd->id = 0x44;
//	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_flow_cali_completed_t)+ CRC_16_LENGTH;
//  //电调状态	
//	p_data->result = cali_result;
//	Append_CRC8_Check_Sum( tmpBuf, 4 );
//	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
//	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
//	
//	 uart_printf( 2, "***********flow_cali_result_push**********\r\n" );
//}

uint16_t g_u16GimbalCtl =1024;
volatile uint32_t g_u32GimbalGettime = 0;
void cmd_handler_gimbal_ctl( uint8_t *p_buf, uint16_t len  )
{
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )p_buf;
	g_u16GimbalCtl = *(uint16_t*)((uint8_t*)(&(p_cmd->id ))+ 1);  //发送给云台（控制水泵速度）//1024+-660                     
	g_u32GimbalGettime = g_current_tick ;//时间32111113
}

void cmd_handler_foc_ver_request( uint8_t idx )
{
	uint8_t tmpBuf[24] = { 0 };
	static uint16_t seq=0;
	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )tmpBuf;

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x0c; 
	p_cmd->receiver.receiver_index = 0x07;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 1;
	p_cmd->type.cmd_ack = 0;
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0;
	p_cmd->id = 0x01;

	p_cmd->vl.length = sizeof( cmd_header_v1_t ) + 2;
	if(idx == FOC_1)
		g_cfg_end_point = EP_UART1;
	if(idx == FOC_2)
		g_cfg_end_point = EP_UART3;
	
	cmd_headler_send_command_v1( tmpBuf, p_cmd->vl.length );	
	uart_printf( 0 ,"req foc ver\r\n" );
}

//#define MIN_FLOW_ESC_VALUE 1200
uint16_t Throttle_Value_1 = MIN_CNT;
uint16_t Throttle_Value_2 = MIN_CNT;
uint8_t feckbackID = 0;


void  Set_Throttle_Value_1(uint16_t value)
{
	 Throttle_Value_1 = value;
}

void  Set_Throttle_Value_2(uint16_t value)
{
	 Throttle_Value_2 = value;
}

uint16_t get_pump_map_speed(uint8_t foc_id,uint8_t flow)
{
	uint16_t speed = 0;
	float value = 0;
		if(flow > MAX_FLOW )
			flow = MAX_FLOW;
		if(flow <= MIN_FLOW)
			flow = MIN_FLOW;
		
		value = (float)(flow - MIN_FLOW)*0.05264*(Get_Spary_Limit_Value(foc_id,MAX_SPRAY_ID)-Get_Spary_Limit_Value(foc_id,MIN_SPRAY_ID))+Get_Spary_Limit_Value(foc_id,MIN_SPRAY_ID); //    1/(MAX_FLOW - MIN_FLOW) = 0.05264
    speed = value;
		return speed;
}


void set_foc_throttle_value(uint8_t foc_id, spray_sys_esc_status_t foc_status)
{
	uint16_t value = 0;
	if(foc_status.flow_speed != 0) // 1 = 0.1L/min
		value	= get_pump_map_speed(foc_id,foc_status.flow_speed);
	else
		value = MIN_CNT;
	
	if(value < MIN_CNT)
		value = MIN_CNT;
	else if(value > MAX_CNT)
		value = MAX_CNT;
	else
		value = value; // 不变
		
	if(foc_id == FOC_1 )
	{
		if(foc_status.enable_flag == 1)
		 Set_Throttle_Value_1(value);
		else
		 Set_Throttle_Value_1(MIN_CNT);
	}
	else if(foc_id == FOC_2)
	{
		if(foc_status.enable_flag == 1)
			Set_Throttle_Value_2(value);
		else
		  Set_Throttle_Value_2(MIN_CNT);
	}
}


uint16_t get_foc_throttle_value(uint8_t foc_id)
{
	if(Throttle_Value_1 > User_Max_Throttle)
		Throttle_Value_1 = User_Max_Throttle;
	if(Throttle_Value_2 > User_Max_Throttle)
		Throttle_Value_2 = User_Max_Throttle;
	
	if(foc_id == FOC_1)
		return Throttle_Value_1;
	else if(foc_id == FOC_2)
		return Throttle_Value_2;
	else
		return MIN_CNT;	
}

uint16_t debug_throttle_value_1,debug_throttle_value_2;
//uint8_t ack_debug_value;
void cmd_hander_foc_request( uint8_t cmd , uint8_t foc_id )
{
	uint8_t tmpBuf[24] = { 0 };
	cmd_header_v0_t *p_cmd = ( cmd_header_v0_t * )tmpBuf;
	cmd_foc_req_t * p_data = ( cmd_foc_req_t* )( tmpBuf + sizeof( cmd_header_v0_t ) );
	feckbackID++;
	if(feckbackID > 3)
	{
	  feckbackID = 0;
	}
	p_cmd->sof = 0x55 ;
	p_cmd->vl.version=0;
	p_cmd->id = 0x00A0 ;
	p_data->cmd.cmd = 0 ;
	p_data->cmd.foc_encoder=0;
	p_data->type.feckbackID = feckbackID ;
	p_data->type.youmen_type = Throttle_Value_Type;

	
	if(foc_id == FOC_1) // 前泵 EP_UART3
	{
		g_cfg_end_point = EP_UART3;
		p_data->foc0.acc_value = get_foc_throttle_value(FOC_1);
		p_data->foc1.acc_value = get_foc_throttle_value(FOC_1);
		p_data->foc2.acc_value = get_foc_throttle_value(FOC_1);
		p_data->foc3.acc_value = get_foc_throttle_value(FOC_1);
		
		debug_throttle_value_1 = get_foc_throttle_value(FOC_1);
	}
	
	if(foc_id == FOC_2)
	{
		g_cfg_end_point = EP_UART1;
		p_data->foc0.acc_value = get_foc_throttle_value(FOC_2);
		p_data->foc1.acc_value = get_foc_throttle_value(FOC_2);
		p_data->foc2.acc_value = get_foc_throttle_value(FOC_2);
		p_data->foc3.acc_value = get_foc_throttle_value(FOC_2);
		debug_throttle_value_2 = get_foc_throttle_value(FOC_2);
	}
	

	 p_cmd->vl.length = sizeof( cmd_header_v0_t ) + sizeof( cmd_foc_req_t ) + 2;
	//memcpy(&debug_cmd_foc_req_t,p_data,sizeof( cmd_foc_req_t ));
	 cmd_headler_send_command_v0( tmpBuf, p_cmd->vl.length );
	//uart_printf( 0 ,"cmd_hander_foc_request\r\n" );
}

void Rank(int16_t adbuf[],uint8_t num)//排序法
{
    int16_t u16Tmp=0;
    uint8_t u8i=0,u8j=0;
    for(u8i=0;u8i<num-1;u8i++)
    {
        for(u8j=u8i+1;u8j<num;u8j++)
        {
            if(adbuf[u8i]>adbuf[u8j])
            {
                u16Tmp=adbuf[u8i];
                adbuf[u8i]=adbuf[u8j];
                adbuf[u8j]=u16Tmp;
            }
        }
    }
}
int16_t AD_Filter(int16_t adbuf[],uint8_t num)
{
    uint8_t u8i=0;
    int16_t u16Filter;
    int32_t s32ValTmp=0;
    Rank(adbuf,num);
    if(num>7)
    {
        for(u8i=3;u8i<num-3;u8i++)
        {
            s32ValTmp+=adbuf[u8i];
        }
        u16Filter=s32ValTmp/(num-6);
    }
    else
    {
        for(u8i=0;u8i<num;u8i++)
        {
            s32ValTmp+=adbuf[u8i];
        }
        u16Filter=s32ValTmp/num;
    }
    return u16Filter;
}



cmd_foc_ack_t  debug_stFOC_Value_1;
cmd_foc_ack_t  debug_stFOC_Value_3;

void cmd_hander_foc_ack( uint8_t *p_buf, uint16_t len )
{
	cmd_foc_ack_t *p_data  = ( cmd_foc_ack_t * )( p_buf + sizeof( cmd_header_v0_t )) ;
	
//	if(ack_debug_value == EP_UART1)
//	{
	  memcpy(&debug_stFOC_Value_1 , p_data , sizeof(cmd_foc_ack_t) );
//	}
//	
//	
//	if(ack_debug_value == EP_UART3)
//	{
 	  memcpy(&debug_stFOC_Value_3 , p_data , sizeof(cmd_foc_ack_t) );
//	}
	
}

radar_data_t g_stRadarData;
void cmd_hander_rardar_push( uint8_t *p_buf, uint16_t len )
{
//	cmd_radar_push_t *p_data  = ( cmd_radar_push_t * )( p_buf + sizeof( cmd_header_v0_t )) ;
//  
//	g_Distance=p_data->distance;
//	g_SonarGetTime = g_current_tick;
//	
//	g_stRadarData.radar_data.distance=p_data->distance;
//	g_stRadarData.radar_data.flag=p_data->flag;
//	g_stRadarData.data_flash=1;
//	
//	uart_printf(0,"flag:%d  dis:%d",p_data->flag,p_data->distance);
}

const handler_pair_v0_t cmd_handler_array_v0[] = {
	{ cmd_hander_foc_ack ,   0x01A0},
	{ cmd_hander_rardar_push,0x1007},
	{ NULL, NULL }
};


const handler_pair_v1_t cmd_handler_array_v1[] = {
	{ cmd_handler_device_info, 		0x01, 0 },
	{ cmd_handler_entry_upgrade, 	0x07, 0 },
	{ cmd_handler_start_upgrade, 	0x08, 0 },
	{ cmd_handler_data_upgrade, 	0x09, 0 },
	{ cmd_handler_end_upgrade, 		0x0A, 0 },
	{ cmd_handler_reboot,		      0x0B, 0 },
	{ cmd_handler_status_report,	0x0C, 0 },
  { cmd_handler_set_version,      0x0D, 0 },
	{ cmd_handler_get_fc_statu ,  0x4e ,  0x03  },//飞机状态 //只使用Vx Vy Vz 是否喷水状态  
	{ cmd_hander_motor_process,   0x7c , 0x02},  //拍照按键 - 喷水
	{ cmd_handler_flow_cali ,     0x42 , 0x09},
	{ cmd_handler_level_switch,   0x43 , 0x09},
	{ cmd_handler_residual_volume_warning,0x44 , 0x09},
	{ cmd_handler_flowmeter_K_control,0x45 , 0x09},
	{ cmd_handler_Block_ID_ack,0x01  , 0xe},
	{ cmd_handler_iosd_status_ack,0x02  , 0xe},
	{ cmd_handler_iosd_config_ack,0x21 , 0xe},
	{ cmd_handler_data_push_ack,0x22 , 0xe},
	
	{ cmd_handler_gimbal_ctl,     0x01 , 0x04},   //云台俯仰- 喷水速度 
	{ NULL, NULL, NULL }
};


uint8_t get_command_version( uint8_t *p_buf, uint8_t len )
{
	uint16_t vl = *( uint16_t * )&p_buf[1];
	return ( uint8_t )( vl >> 10 );
}

void command_process_v0( uint8_t *p_buf, uint16_t len )
{
	int i = 0;
	cmd_header_v0_t	*p_cmd  = ( cmd_header_v0_t * )p_buf;

	if( len < sizeof( cmd_header_v0_t ) ) {
		return;
	}

//	uart_printf( 0, "[%s, %d] command id: 0x%.4X\r\n", __FILE__, __LINE__, p_cmd->id );
	for( i = 0; cmd_handler_array_v0[i].handler != NULL; i++ ) {
		if( p_cmd->id == cmd_handler_array_v0[i].id ) {
			cmd_handler_array_v0[i].handler( p_buf, len );
		}
	}
}

uint8_t u8BatteryUpgradeFlag=0;
cmd_header_v1_t	*p_cmd = NULL;
void command_process_v1( uint8_t *p_buf, uint16_t len )
{
	int i = 0;
	cmd_header_v0_t	*p_cmd_v0  = ( cmd_header_v0_t * )p_buf;
	p_cmd = ( cmd_header_v1_t * )p_buf;

	
	if( len < sizeof( cmd_header_v1_t ) ) {
		return;
	}

	if(p_cmd->vl.version==0)
	{
			for( i = 0; cmd_handler_array_v0[i].handler != NULL; i++ ) 
		 {
		  if( p_cmd_v0->id == cmd_handler_array_v0[i].id )
 			{
				cmd_handler_array_v0[i].handler( p_buf, len );
		  }
	  }
	}
	else if(p_cmd->vl.version==1)
	{			
//			uart_printf( 0, "[%s, %d] command id: %d %d %d %d %d\r\n", __FILE__, __LINE__, p_cmd->set, p_cmd->type.encrypt_type, p_cmd->type.cmd_type, p_cmd->receiver.receiver_id, p_cmd->receiver.receiver_index );
			if(  ( ( p_cmd->receiver.receiver_id != DEVICE_ID || p_cmd->receiver.receiver_index != DEVICE_INDEX  ) && p_cmd->receiver.receiver_id != 31 && p_cmd->receiver.receiver_id !=0x01 && p_cmd->receiver.receiver_id !=0x04) ) 
			{
				if( DJI_PRO_DEVICE_TYPE_SMART_BATTERY==p_cmd->receiver.receiver_id  )
				{
					g_cfg_end_point = EP_UART2;
					
                    if((p_cmd->set==0)&&(p_cmd->id == 8)){
                        u8BatteryUpgradeFlag=1;
                    }
                    if(u8BatteryUpgradeFlag==1){
                        if((p_cmd->set==0)&&((p_cmd->id == 0x08)||(p_cmd->id == 0x09)||(p_cmd->id == 0x0a))){
                            cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
                        }
                    }else{
                        cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
                    }
				}else if(DJI_PRO_DEVICE_TYPE_FLIGHT_CONTROL == p_cmd->receiver.receiver_id
 				         || DJI_PRO_DEVICE_TYPE_APP == p_cmd->receiver.receiver_id )
				{
					g_cfg_end_point = EP_CAN1;
                    cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
				}else if(DJI_PRO_DEVICE_TYPE_PC==p_cmd->receiver.receiver_id)
				{
                    g_cfg_end_point = EP_CAN1;
                    cmd_headler_send_command_v1( p_buf, p_cmd->vl.length );
                    if((p_cmd->set==0)&&(p_cmd->id == 0x0a))
										{
                        u8BatteryUpgradeFlag=0;
                    }
                }
			}else if(p_cmd->receiver.receiver_id == DEVICE_ID ) {
	//	uart_printf( 0, "[%s, %d] command id: 0x%.2X\r\n", __FILE__, __LINE__, p_cmd->id );
				for( i = 0; cmd_handler_array_v1[i].handler != NULL; i++ ) {
					if( p_cmd->id == cmd_handler_array_v1[i].id && p_cmd->set == cmd_handler_array_v1[i].set ) {
						cmd_handler_array_v1[i].handler( p_buf, len );
					}
				}
		}
	}
}

void cfg_status_report( void )
{
	uint8_t tmpBuf[128] = { 0 };

	cmd_header_v1_t *p_cmd = ( cmd_header_v1_t * )tmpBuf;
	cmd_status_report_ack_t *p_sra = ( cmd_status_report_ack_t * )( tmpBuf + sizeof( cmd_header_v1_t ) );

	if( g_device_status == 0 ) return; 

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 31; // broadcast
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = 0xFFFF;
	p_cmd->type.cmd_type = 1;
	p_cmd->type.cmd_ack = 0;
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0;
	p_cmd->id = 0x0C;

	p_sra->result = 0;
	p_sra->ver.major = 0;
	p_sra->ver.minor = 0;
	p_sra->status = g_device_status;
	
	p_cmd->vl.length = sizeof( cmd_header_v1_t ) + sizeof( cmd_status_report_ack_t ) + 2;
	
	cmd_headler_send_command_v1( tmpBuf, p_cmd->vl.length );
}


void can_upgrade_ack( uint8_t * buf , uint16_t len )
{
	CAN_sendbytes_View( LPC_CAN1, &can1UpgradeAckTxRingBuf, buf, len );
}

/*
描述： 喷洒系统数据记录相关 
CMD SET: 14 
*/
#define Item_Count 13
uint8_t const spray_data_name[data_name_length] = 
{
0x61, 'P', 'E', 'N', 'S', 'A', 0x0a,
0x62, 'e', 'n', 'b', 'l', 'e', '1', 0x0a,
0x6a, 'f', 'l', 'o', 'w', '1', 0x0a,
0x62, 'e', 'n', 'b', 'l', 'e', '2', 0x0a,
0x6a, 'f', 'l', 'o', 'w', '2', 0x0a,	
0x63, 'f', 'o', 'c', '1', 0x0a,
0x63, 'p', 'r', 'e', 's', 's', '1', 0x0a,
0x63, 'f', 'o', 'c', '2', 0x0a,
0x63, 'p', 'r', 'e', 's', 's', '2', 0x0a,
0x62, 'f', 'r', 'e', 'q', 0x0a, 
0x62, 'l', 'e', 'v', 'e', 'l', 0x0a,	 
0x63, 'c', 'a', 'l', 'i', '1', 0x0a,
0x63, 'c', 'a', 'l', 'i', '2', 0x0a
};

/********************************Block ID 申请**************************************************/
//uint16_t smm_1,smm_2,smm_3,smm_4;
uint16_t Block_ID = 0;
cmd_iosd_status_ack_t cmd_iosd_status_ack; 
void IOSD_Block_ID_Request( void )
{
	uint8_t tmpBuf[20] = {0};
	static uint16_t seq=0;
  cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
  cmd_iosd_req_t *p_data = (cmd_iosd_req_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x03; // 飞控
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 0;
	p_cmd->type.cmd_ack =  1; // 需要应答
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0xe;
	p_cmd->id = 0x01;
	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_iosd_req_t)+ CRC_16_LENGTH;
  
	cmd_iosd_status_ack.status_flag = 1;//默认非空闲状态
	
  //smm_1++;
	p_data->tag = MY_DEVICE_TAG;	
	Append_CRC8_Check_Sum( tmpBuf, 4 );
	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
}

cmd_iosd_ack_t cmd_iosd_ack; 
void cmd_handler_Block_ID_ack(uint8_t *p_buf, uint16_t len) 
{
	cmd_iosd_ack_t *p_rr = ( cmd_iosd_ack_t*)( p_buf + sizeof( cmd_header_v1_t ) );
	Block_ID = p_rr->Block_ID;
  memcpy(&cmd_iosd_ack,p_rr,sizeof(cmd_iosd_ack_t));
}

/******************************** 状态查询 **************************************************/

void IOSD_status_inquire(void)
{
	uint8_t tmpBuf[20] = {0};
	static uint16_t seq=0;
  cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
  cmd_iosd_status_inquire_t *p_data = (cmd_iosd_status_inquire_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x03; // 飞控
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 0;
	p_cmd->type.cmd_ack =  1; // 需要应答
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0xe;
	p_cmd->id = 0x02;
	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_iosd_status_inquire_t)+ CRC_16_LENGTH;
  p_data->reserve = 0;
 // smm_2++;
	Append_CRC8_Check_Sum( tmpBuf, 4 );
	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
}

void cmd_handler_iosd_status_ack(uint8_t *p_buf, uint16_t len) 
{
	cmd_iosd_status_ack_t *p_rr = (cmd_iosd_status_ack_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  memcpy(&cmd_iosd_status_ack,p_rr,sizeof(cmd_iosd_status_ack_t));
}
/******************************** 配置信息推送 **************************************************/

void IOSD_Config_Info_Push(void)
{
	uint8_t tmpBuf[120] = {0};
	static uint16_t seq=0;
  cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
  cmd_iosd_config_t *p_data = (cmd_iosd_config_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x03; // 飞控
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 0;
	p_cmd->type.cmd_ack =  1; // 需要应答
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0xe;
	p_cmd->id = 0x21;
	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_iosd_config_t)+ CRC_16_LENGTH;
  	  
//smm_3++;
	p_data->block_id = Block_ID;
	p_data->number_of_packet_entries = Item_Count; 
	p_data->config_data_length = data_name_length;
  memcpy(p_data->data_name,spray_data_name,data_name_length);
	Append_CRC8_Check_Sum( tmpBuf, 4 );
	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
}

cmd_iosd_config_ack_t cmd_iosd_config_ack; 
void cmd_handler_iosd_config_ack(uint8_t *p_buf, uint16_t len) 
{
	cmd_iosd_config_ack_t *p_rr = (cmd_iosd_config_ack_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  memcpy(&cmd_iosd_config_ack,p_rr,sizeof(cmd_iosd_config_ack_t));
}

/******************************** 数据包推送 **************************************************/

void spray_system_data_Push(void)
{
	uint8_t tmpBuf[50] = {0};
	static uint16_t seq=0;
  cmd_header_v1_t *p_cmd = (cmd_header_v1_t *)tmpBuf;
  cmd_iosd_data_push_t *p_data = (cmd_iosd_data_push_t *)(tmpBuf + sizeof( cmd_header_v1_t ));

	p_cmd->sof = 0x55;
	p_cmd->vl.version = 1;
	p_cmd->sender.sender_id = DEVICE_ID;
	p_cmd->sender.sender_index = DEVICE_INDEX;
	p_cmd->receiver.receiver_id = 0x03; // 飞控
	p_cmd->receiver.receiver_index = 0;
	p_cmd->seqnum = seq++;
	p_cmd->type.cmd_type = 0;
	p_cmd->type.cmd_ack =  1; // 需要应答
	p_cmd->type.encrypt_type = 0;
	p_cmd->set = 0xe;
	p_cmd->id = 0x22;
	p_cmd->vl.length = sizeof(cmd_header_v1_t) + sizeof(cmd_iosd_data_push_t)+ CRC_16_LENGTH;
  
	p_data->block_id =  Block_ID;
  p_data->iosd_data_length = sizeof(spray_system_para_save_iosd_t);
  
	p_data->spray_system_para.control_pump_status[0].enable_flag = FOC_1_STATUS.enable_flag;;
	p_data->spray_system_para.control_pump_status[0].flow_speed = FOC_1_STATUS.flow_speed*0.02f;
	p_data->spray_system_para.control_pump_status[1].enable_flag = FOC_2_STATUS.enable_flag;
	p_data->spray_system_para.control_pump_status[1].flow_speed  = FOC_2_STATUS.flow_speed*0.02f;
	p_data->spray_system_para.flow_freq = get_cali_flow_freq();
	p_data->spray_system_para.level_status = level_status_by_radar;
	p_data->spray_system_para.foc1_value = get_foc_throttle_value(FOC_1);
	p_data->spray_system_para.foc2_value = get_foc_throttle_value(FOC_2);
	p_data->spray_system_para.press1_value = get_press_value(Press_Sensor_0);
	p_data->spray_system_para.press2_value = get_press_value(Press_Sensor_1);
	p_data->spray_system_para.cali_capacity = spray_system_flow_cali_para1.capacity;
	p_data->spray_system_para.cali_pulses = spray_system_flow_cali_para1.cali_pulses;

	Append_CRC8_Check_Sum( tmpBuf, 4 );
	Append_CRC16_Check_Sum( tmpBuf, p_cmd->vl.length );
	CAN_sendbytes_View( LPC_CAN1, &can1Tx3RingBuf, tmpBuf, p_cmd->vl.length );
}

cmd_iosd_data_push_ack_t debug_cmd_iosd_data_push_ack; 
void cmd_handler_data_push_ack(uint8_t *p_buf, uint16_t len) 
{
	cmd_iosd_data_push_ack_t *p_rr = (cmd_iosd_data_push_ack_t*)( p_buf + sizeof( cmd_header_v1_t ) );
  memcpy(&debug_cmd_iosd_data_push_ack,p_rr,sizeof(cmd_iosd_data_push_ack_t));
}



// 10 HZ
void spray_system_para_recoder(void)
{
	static uint16_t run_count = 0;
	static uint8_t  status = 0;
  run_count++;
  switch(status)
	{
		case 0: // 申请id
			if(run_count%10 == 0)
			  IOSD_Block_ID_Request();
			if(cmd_iosd_ack.Block_ID != 0)
			{
				if(cmd_iosd_ack.result == 0 || cmd_iosd_ack.result == 4)
					status = 1; 
		  }
	  break;
		
		case 1:// 查询是否在空闲
		  if(run_count%30 == 0)
			  IOSD_status_inquire();
		  if(cmd_iosd_status_ack.status_flag == 0)// 状态空闲
				status = 2;
	  break;
		
		case 2:
			if(run_count%10 == 0)
		   IOSD_Config_Info_Push();
			if( cmd_iosd_config_ack.block_id != 0)
			{
				if(cmd_iosd_config_ack.config_result == 0 ||cmd_iosd_config_ack.config_result == 7)
				  status = 3;
		  }
	  break;
		
		case 3:
			 spray_system_data_Push();
	  break;
		
	  default:
			
	  break;
	}
}

/*******************  (C) COPYRIGHT 2014 DJI ************END OF FILE***********/
