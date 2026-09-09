/*
 * INA226AIDGSR.c
 *
 *  Created on: Jul 15, 2026
 *      Author: SRIVATHSJAYAN
 */

/*
 * Parameters to be configured in Header File:
 *FLT_DETECT.FAULT_Port = GPIOB;
  FLT_DETECT.FAULT_Pin = GPIO_PIN_7;
  FLT_DETECT.hi2c = hi2c1;
  FLT_DETECT.Init.i2c_addr = (0x40<<1);
  FLT_DETECT.Init.op_mode = INA226_OP_MODE_SH_BUS_VT_CONT;
  FLT_DETECT.Init.vshct = INA226_VBUSCT_332US;
  FLT_DETECT.Init.vbusct = INA226_VBUSCT_332US;
  FLT_DETECT.Init.avg = INA226_AVG_4;

  FLT_DETECT.Init.max_cur_exp_A = 0.5;
  FLT_DETECT.Init.shunt_res_Ohm = 0.1;

  Set_Calib
  Set_Mask_En
  Set_Alert_Val
 * */
#include "INA226AIDGSR.h"

static HAL_StatusTypeDef INA226_WriteReg(INA226_Handle_TypeDef_t *hfault,INA226_Register_t Reg,uint16_t Data);
/**
 * @brief  Performs a software reset on the INA226 device.
 * @details Sets the reset bit (RST, bit 15) in the Configuration Register (00h)
 *          over the I2C interface, restoring all internal registers to their
 *          default power-on reset values.
 *
 * @param[in] hfault Pointer to the INA226 handle structure containing device and I2C configuration.
 *
 * @retval HAL_OK       Reset command was successfully transmitted.
 * @retval HAL_ERROR    I2C communication failure or transmission error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Reset(INA226_Handle_TypeDef_t *hfault)
{
	uint16_t reset_bit = 0x01 << INA226_RST_POS;

	return INA226_WriteReg(hfault, INA226_REG_CONFIG,reset_bit);
}
/**
 * @brief  Initializes the INA226 device with the configured operating settings.
 * @details Issues a software reset to restore the device to its power-on reset state,
 *          validates the initialization structure parameters (operating mode,
 *          averaging count, bus voltage conversion time, and shunt voltage conversion time),
 *          and writes the assembled bitfield value to the Configuration Register (00h).
 *
 * @param[in] hfault Pointer to the INA226 handle structure containing initialization parameters
 *                   and I2C configuration.
 *
 * @retval HAL_OK       Device initialization and configuration successful.
 * @retval HAL_ERROR    Invalid parameter configuration detected or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Init(INA226_Handle_TypeDef_t *hfault)
{
	uint16_t reg=0;

	INA226_Reset(hfault);

	if(hfault->Init.op_mode > INA226_OP_MODE_SH_BUS_VT_CONT)
	{
		return HAL_ERROR;
	}
	if(hfault->Init.op_mode == INA226_OP_MODE_PWR_DWN_0)
	{
		return HAL_ERROR;
	}
	if(hfault->Init.op_mode == INA226_OP_MODE_PWR_DWN_4)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.op_mode << INA226_OP_MODE_POS;

	if(hfault->Init.avg > INA226_AVG_1024)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.avg << INA226_AVG_POS;

	if(hfault->Init.vbusct > INA226_VBUSCT_8_244MS)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.vbusct << INA226_VBUSCT_POS;

	if(hfault->Init.vshct > INA226_VSHCT_8_244MS)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vshct << INA226_VSHCT_POS;

	return INA226_WriteReg(hfault,INA226_REG_CONFIG,reg);

}
/**
 * @brief  Writes a 16-bit value to a specified INA226 register.
 * @details Packs the 8-bit register pointer address followed by the 16-bit data
 *          value (transmitted MSB first) into a 3-byte payload buffer and sends
 *          it to the target device over the I2C bus.
 *
 * @param[in] hfault Pointer to the INA226 handle structure containing device and I2C configuration.
 * @param[in] Reg    Target register address/command (type ::INA226_Register_t).
 * @param[in] Data   16-bit data word to write into the register.
 *
 * @retval HAL_OK       Register write completed successfully.
 * @retval HAL_ERROR    I2C communication failure or transmission error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
static HAL_StatusTypeDef INA226_WriteReg(INA226_Handle_TypeDef_t *hfault,INA226_Register_t Reg,uint16_t Data)
{
	HAL_StatusTypeDef status;
	uint8_t tx[3];
	tx[0] = (uint8_t)Reg;
	tx[1] = (uint8_t)(Data>>8);
	tx[2] = (uint8_t)Data;
	status =  HAL_I2C_Master_Transmit(hfault->hi2c,hfault->Init.dev_i2c_addr,tx, 3, HAL_MAX_DELAY);
	return status;
}
/**
 * @brief  Reads a 16-bit register from the INA226 over I2C.
 * @details Transmits the 8-bit register pointer address to the target device,
 *          waits for a 10 ms delay, and reads back 2 consecutive data bytes (16 bits)
 *          into the destination buffer (transmitted MSB first).
 *
 * @param[in]  hfault Pointer to the handle structure containing device and I2C peripheral configuration.
 * @param[in]  reg    Target register address to read (type ::INA226_Register_t).
 * @param[out] pData  Pointer to a 2-byte buffer where the received register data
 *                    will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully.
 * @retval HAL_ERROR    pData is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_ReadReg(INA226_Handle_TypeDef_t *hfault,INA226_Register_t reg,uint8_t *pData)
{
	if(pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t reg_addr = (uint8_t)reg;
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit(hfault->hi2c,hfault->Init.dev_i2c_addr, &reg_addr, 1, HAL_MAX_DELAY);
	HAL_Delay(10);
	if(status != HAL_OK)
	{
		return status;
	}
	status = HAL_I2C_Master_Receive(hfault->hi2c,hfault->Init.dev_i2c_addr, pData, 2, HAL_MAX_DELAY);

	return status;
}
/**
 * @brief  Calculates and writes the calibration register value for the INA226.
 * @details Computes the 16-bit calibration value using the user-configured maximum
 *          expected current and shunt resistance according to the formula:
 *          CAL = 0.00512 / (Current_LSB * Rshunt).
 *          Validates that the parameters are positive and that the computed calibration
 *          value falls within the valid range (1 to 32767) before writing it to the
 *          Calibration Register (05h) over I2C. Caches the calculated LSB values in the handle.
 *
 * @param[in] hfault Pointer to the INA226 handle structure containing device configuration and I2C instance.
 *
 * @retval HAL_OK       Calibration value successfully calculated and written.
 * @retval HAL_ERROR    Invalid input parameters (non-positive current/shunt), calculated
 *                      calibration value out of range, or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Set_Calib(INA226_Handle_TypeDef_t *hfault)
{

	if ((hfault->Init.max_cur_exp_A <= 0.0f) || (hfault->Init.shunt_res_Ohm <= 0.0f))
	{
		return HAL_ERROR;
	}
	float cal_f;
	HAL_StatusTypeDef status;
	float current_lsb = (hfault->Init.max_cur_exp_A / 32768.0f);

	cal_f = (float)(0.00512f / (current_lsb * hfault->Init.shunt_res_Ohm));

	if ((cal_f < 1.0f) || (cal_f > 32767.0f))
	{
		return HAL_ERROR;
	}
	uint16_t cal = (uint16_t)(cal_f + 0.5f);

	status = INA226_WriteReg(hfault,INA226_REG_CALIB,cal);
	if(status != HAL_OK)
	{
		return status;
	}
	hfault->_current_lsb_A = current_lsb;
	hfault->_power_lsb_W   = (current_lsb * 25.0f);
	return status;
}
/**
 * @brief  Reads and calculates the shunt voltage in volts from the INA226.
 * @details Reads the 16-bit signed two's complement value from the Shunt Voltage
 *          Register (01h) over I2C and applies the fixed 2.5 uV/LSB conversion
 *          factor to compute the voltage in Volts.
 *
 * @param[in]  hfault Pointer to the handle structure containing device and I2C peripheral configuration.
 * @param[out] pData  Pointer to a float variable where the calculated shunt voltage
 *                    (in Volts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Shunt voltage successfully read and calculated.
 * @retval HAL_ERROR    pData is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Get_Shunt_Vltg_V(INA226_Handle_TypeDef_t *hfault,float *pData)
{
	if(pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	int16_t shnt_vlt_raw;
	HAL_StatusTypeDef status;
	status =  INA226_ReadReg(hfault,INA226_REG_SHUNT_VLTG,rx);
	if(status != HAL_OK)
	{
		return status;
	}

	shnt_vlt_raw = (uint16_t)rx[0] << 8 | rx[1];

	float shunt_vltg = (shnt_vlt_raw * 2.5e-6f);

	*pData = shunt_vltg;

	return status;
}
/**
 * @brief  Reads and calculates the bus voltage in volts from the INA226.
 * @details Reads the 16-bit unsigned value from the Bus Voltage Register (02h)
 *          over I2C and applies the fixed 1.25 mV/LSB scaling factor to compute
 *          the voltage in Volts.
 *
 * @param[in]  hfault Pointer to the INA226 handle structure containing device and I2C configuration.
 * @param[out] pData  Pointer to a float variable where the calculated bus voltage
 *                    (in Volts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Bus voltage successfully read and calculated.
 * @retval HAL_ERROR    pData is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Get_Bus_Vltg_V(INA226_Handle_TypeDef_t *hfault,float *pData)
{
	if(pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	uint16_t bus_vlt_raw;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault,INA226_REG_BUS_VLTG,rx);
	if(status != HAL_OK)
	{
		return status;
	}

	bus_vlt_raw = (uint16_t)rx[0] << 8 | rx[1];

	float bus_vltg = (bus_vlt_raw * 1.25e-3f);

	*pData = bus_vltg;

	return status;
}
/**
 * @brief  Reads and calculates the current in amperes from the INA226.
 * @details Reads the raw 16-bit signed two's complement value from the Current
 *          Register (04h) over I2C and scales it to amperes using the cached
 *          Current_LSB value stored in the device handle.
 *
 * @param[in]  hfault Pointer to the handle structure containing INA226 configuration and I2C instance.
 * @param[out] pData  Pointer to a float variable where the calculated current
 *                    (in Amperes) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Current successfully read and calculated.
 * @retval HAL_ERROR    pData is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Get_Current_A(INA226_Handle_TypeDef_t *hfault,float *pData)
{
	if(pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	int16_t cur_raw;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault,INA226_REG_CURRENT,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	cur_raw = ((uint16_t)rx[0] << 8) | rx[1];

	*pData = (float)cur_raw * hfault->_current_lsb_A;

	return status;
}
/**
 * @brief  Reads and calculates the power in watts from the INA226.
 * @details Reads the 16-bit unsigned value from the Power Register (03h)
 *          over I2C and calculates the actual power in Watts using the cached
 *          Power_LSB value stored in the device handle.
 *
 * @param[in]  hfault Pointer to the handle structure containing INA226 configuration and I2C instance.
 * @param[out] pData  Pointer to a float variable where the calculated power
 *                    (in Watts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Power successfully read and calculated.
 * @retval HAL_ERROR    pData is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Get_Power_W(INA226_Handle_TypeDef_t *hfault,float *pData)
{
	if(pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	uint16_t power_raw;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault,INA226_REG_POWER,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	power_raw = ((uint16_t)rx[0] << 8) | rx[1];

	*pData = (float)power_raw * hfault->_power_lsb_W;

	return status;
}
/**
 * @brief  Modifies a single bit in the INA226 Mask/Enable Register (06h).
 * @details Performs a read-modify-write operation on the Mask/Enable Register over I2C.
 *          Validates that the target bit position and operation type are valid, reads
 *          the current 16-bit register value, sets or clears the specified bit, and
 *          writes the updated value back to the device.
 *
 * @param[in] hfault         Pointer to the INA226 handle structure containing device and I2C peripheral configuration.
 * @param[in] Ina226_En_msk  Target bit position to modify (type ::INA226_En_Reg_t).
 *                           Must not exceed INA226_EN_MSK_SOL (bit 15).
 * @param[in] En_Di          Bit modification state. Must be either ENABLE (0x01U) or DISABLE (0x00U).
 *
 * @retval HAL_OK       Register bit modified and written successfully.
 * @retval HAL_ERROR    Invalid bit position, invalid operation parameter, or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Modify_En_Msk(INA226_Handle_TypeDef_t *hfault,INA226_En_Reg_t Ina226_En_msk,uint8_t En_Di)
{
	if (Ina226_En_msk > INA226_EN_MSK_SOL)
	{
		return HAL_ERROR;
	}
	if (En_Di > ENABLE)
	{
	    return HAL_ERROR;
	}
	uint8_t rx[2U];
	uint16_t reg;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault, INA226_REG_MSK_EN,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	reg = (uint16_t)rx[0] << 8 | rx[1];

	if(En_Di == ENABLE)
	{
		reg |= (uint16_t)(1U << Ina226_En_msk);
	}
	else
	{
		reg &= (uint16_t)~(1U << Ina226_En_msk);
	}

	return INA226_WriteReg(hfault,INA226_REG_MSK_EN,reg);
}
/**
 * @brief  Sets the limit value in the INA226 Alert Limit Register (07h).
 * @details Writes a 16-bit threshold value to the Alert Limit Register over the I2C
 *          interface. This value is compared against the alert function selected in
 *          the Mask/Enable Register (06h) to assert the ALERT pin.
 *
 * @param[in] hfault Pointer to the INA226 handle structure containing device and I2C configuration.
 * @param[in] val    16-bit comparison threshold value to write to the Alert Limit Register.
 *
 * @retval HAL_OK       Alert limit value successfully written.
 * @retval HAL_ERROR    I2C communication failure or transmission error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA226_Set_Alert_Val(INA226_Handle_TypeDef_t *hfault,uint16_t val)
{
	return INA226_WriteReg(hfault,INA226_REG_ALERT,val);
}
