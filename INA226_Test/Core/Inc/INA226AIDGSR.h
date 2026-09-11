/*
 * INA226AIDGSR.h
 *
 *  Created on: Jul 15, 2026
 *      Author: SRIVATHSJAYAN
 */

#ifndef INA226AIDGSR_H_
#define INA226AIDGSR_H_

#include "main.h"

typedef enum
{
	INA226_REG_CONFIG           = 0x00,
	INA226_REG_SHUNT_VLTG       = 0x01,
	INA226_REG_BUS_VLTG         = 0x02,
	INA226_REG_POWER            = 0x03,
	INA226_REG_CURRENT          = 0x04,
	INA226_REG_CALIB            = 0x05,
	INA226_REG_MSK_EN           = 0x06,
	INA226_REG_ALERT            = 0x07,

	// 0x08 - 0xFD Reserved

	INA226_REG_MANF_ID          = 0xFE,
	INA226_REG_DIE_ID           = 0xFF
} INA226_Register_t;

typedef enum
{
    INA226_EN_MSK_LEN  = 0x00,    // Conversion Ready / Alert Enable
    INA226_EN_MSK_APOL = 0x01,    // Alert Polarity
    INA226_EN_MSK_OVF  = 0x02,    // Math Overflow Flag
    INA226_EN_MSK_CVRF = 0x03,    // Conversion Ready Flag
    INA226_EN_MSK_AFF  = 0x04,    // Alert Function Flag

	// 0x05 to 0x09 Reserved

	INA226_EN_MSK_CNRV = 0x0A,    // Conversion Ready
    INA226_EN_MSK_POL  = 0x0B,    // Power Limit
    INA226_EN_MSK_BUL  = 0x0C,    // Bus Voltage Under-Limit
    INA226_EN_MSK_BOL  = 0x0D,    // Bus Voltage Over-Limit
    INA226_EN_MSK_SUL  = 0x0E,    // Shunt Voltage Under-Limit
    INA226_EN_MSK_SOL  = 0x0F     // Shunt Voltage Over-Limit
} INA226_En_Reg_t;
typedef struct
{
	uint8_t avg;
	uint8_t vbusct;
	uint8_t vshct;
	uint8_t op_mode;
	float max_cur_exp_A;
	float shunt_res_Ohm;
	uint16_t dev_i2c_addr;
	INA226_Register_t reg;

} INA226_InitTypeDef_t;
typedef struct
{
	I2C_HandleTypeDef *hi2c;

	GPIO_TypeDef *FAULT_Port;
	uint16_t FAULT_Pin;

	INA226_InitTypeDef_t Init;

	float _current_lsb_A;
	float _power_lsb_W;
} INA226_Handle_TypeDef_t;
// @avg
#define INA226_AVG_1                0x00U
#define INA226_AVG_4                0x01U
#define INA226_AVG_16               0x02U
#define INA226_AVG_64               0x03U
#define INA226_AVG_128              0x04U
#define INA226_AVG_256              0x05U
#define INA226_AVG_512              0x06U
#define INA226_AVG_1024             0x07U

// @vbusct
#define INA226_VBUSCT_140US         0x00U
#define INA226_VBUSCT_204US         0x01U
#define INA226_VBUSCT_332US         0x02U
#define INA226_VBUSCT_588US         0x03U
#define INA226_VBUSCT_1_1MS         0x04U
#define INA226_VBUSCT_2_116MS       0x05U
#define INA226_VBUSCT_4_156MS       0x06U
#define INA226_VBUSCT_8_244MS       0x07U

// @vshct
#define INA226_VSHCT_140US          0x00U
#define INA226_VSHCT_204US          0x01U
#define INA226_VSHCT_332US          0x02U
#define INA226_VSHCT_588US          0x03U
#define INA226_VSHCT_1_1MS          0x04U
#define INA226_VSHCT_2_116MS        0x05U
#define INA226_VSHCT_4_156MS        0x06U
#define INA226_VSHCT_8_244MS        0x07U

// @op_mode
#define INA226_OP_MODE_PWR_DWN_0      0x00U
#define INA226_OP_MODE_SH_VT_TRG      0x01U
#define INA226_OP_MODE_BUS_VT_TRG     0x02U
#define INA226_OP_MODE_SH_BUS_VT_TRG  0x03U
#define INA226_OP_MODE_PWR_DWN_4      0x04U
#define INA226_OP_MODE_SH_VT_CONT     0x05U
#define INA226_OP_MODE_BUS_VT_CONT    0x06U
#define INA226_OP_MODE_SH_BUS_VT_CONT 0x07U

#define _INA226_OP_MODE_POS   0x00U
#define _INA226_VSHCT_POS     0x03U
#define _INA226_VBUSCT_POS    0x06U
#define _INA226_AVG_POS       0x09U
#define _INA226_RST_POS       0x0FU

//#define _INA226_I2C_REG_POS       0x10U
//#define _INA226_I2C_DEVICE_ID_POS 0x19U
//#define _INA226_I2C_W_R_EN_POS    0x18U
//#define INA226_READ_EN_BIT   0x01U
//#define INA226_WRITE_EN_BIT  0x00U

#define ENABLE 0x01U
#define DISABLE 0x00U

HAL_StatusTypeDef INA226_Init(INA226_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA226_Reset(INA226_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA226_ReadReg(INA226_Handle_TypeDef_t *hfault, INA226_Register_t reg, uint16_t *pData);
HAL_StatusTypeDef INA226_Get_Shunt_Vltg_V(INA226_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA226_Get_Bus_Vltg_V(INA226_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA226_Get_Power_W(INA226_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA226_Get_Current_A(INA226_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA226_Set_Calib(INA226_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA226_Modify_En_Msk(INA226_Handle_TypeDef_t *hfault,INA226_En_Reg_t Ina226_En_msk,uint8_t En_Di);
HAL_StatusTypeDef INA226_Set_Alert_Val(INA226_Handle_TypeDef_t *hfault,uint16_t val);
#endif /* INA226AIDGSR_H_ */
