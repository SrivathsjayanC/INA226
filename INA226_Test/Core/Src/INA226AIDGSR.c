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

static HAL_StatusTypeDef INA226_WriteReg(INA226_Handle_TypeDef_t *hfault, INA226_Register_t Ina226_Reg, uint16_t Data);
static HAL_StatusTypeDef _INA226_ReadReg_Signed(INA226_Handle_TypeDef_t *hfault, INA226_Register_t Ina226_Reg, int16_t *pData);
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
	uint16_t reset_bit = 0x01U << __INA226_RST_POS;

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
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	uint16_t reg=0;
	HAL_StatusTypeDef status;
	status = INA226_Reset(hfault);
	if(status != HAL_OK)
	{
		return HAL_ERROR;
	}
	HAL_Delay(2);
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

	reg |= hfault->Init.op_mode << __INA226_OP_MODE_POS;

	if(hfault->Init.avg > INA226_AVG_1024)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.avg << __INA226_AVG_POS;

	if(hfault->Init.vbusct > INA226_VBUSCT_8_244MS)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.vbusct << __INA226_VBUSCT_POS;

	if(hfault->Init.vshct > INA226_VSHCT_8_244MS)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vshct << __INA226_VSHCT_POS;

	return INA226_WriteReg(hfault,INA226_REG_CONFIG,reg);

}
/**
 * @brief  Writes a 16-bit value to a specified INA226 register over I2C.
 * @details Splits the 16-bit data value into MSB and LSB, and writes them
 *          to the target device register using the HAL I2C memory write API.
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
static HAL_StatusTypeDef INA226_WriteReg(INA226_Handle_TypeDef_t *hfault, INA226_Register_t Ina226_Reg, uint16_t Data)
{
	HAL_StatusTypeDef status;
	uint8_t tx[2];
	tx[0] = (uint8_t)(Data>>8);
	tx[1] = (uint8_t)Data;

	status = HAL_I2C_Mem_Write(hfault->hi2c, hfault->Init.dev_i2c_addr, Ina226_Reg, 1, tx, 2, HAL_MAX_DELAY);

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
HAL_StatusTypeDef INA226_ReadReg(INA226_Handle_TypeDef_t *hfault, INA226_Register_t Ina226_Reg,uint16_t *pData)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status;
	uint8_t rx_buff[2];
	
	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Ina226_Reg, 1, rx_buff, 2, HAL_MAX_DELAY);
	if(status != HAL_OK)
	{
		return status;
	}
	*pData = (((uint16_t)rx_buff[0]<<8) | (uint16_t)rx_buff[1]);

	return status;
}
/**
 * @brief  Reads a 16-bit signed register from the INA226 over I2C.
 * @details Uses the I2C memory read sequence to fetch 2 consecutive data bytes
 *          from the specified target register and reconstructs them into a
 *          signed 16-bit integer (MSB first). This is used specifically for
 *          two's complement registers such as Shunt Voltage and Current.
 *
 * @param[in]  hfault Pointer to the INA226 handle structure containing the
 *                    I2C peripheral instance and target device address.
 * @param[in]  reg    Target register address to read from (type ::INA226_Register_t).
 * @param[out] pData  Pointer to a signed 16-bit variable where the reconstructed
 *                    register value will be written. Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully over I2C.
 * @retval HAL_ERROR    The pData pointer is NULL or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
static HAL_StatusTypeDef _INA226_ReadReg_Signed(INA226_Handle_TypeDef_t *hfault, INA226_Register_t Ina226_Reg,int16_t *pData)
{
	HAL_StatusTypeDef status;
	uint8_t rx_buff[2];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Ina226_Reg, 1, rx_buff, 2, HAL_MAX_DELAY);
	if(status != HAL_OK)
	{
		return status;
	}
	*pData = (((uint16_t)rx_buff[0]<<8) | (uint16_t)rx_buff[1]);

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
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	if ((hfault->Init.max_cur_exp_A <= 0.0f) || (hfault->Init.shunt_res_Ohm <= 0.0f))
	{
		return HAL_ERROR;
	}
	float cal_f;
	HAL_StatusTypeDef status;
	float current_lsb = (hfault->Init.max_cur_exp_A / __INA226_CURRENT_LSB_DIVISOR);

	cal_f = (float)(__INA226_CALIBRATION_CONSTANT / (current_lsb * hfault->Init.shunt_res_Ohm));

	if ((cal_f < 1.0f) || (cal_f > __INA226_CURRENT_LSB_DIVISOR))
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
	hfault->_power_lsb_W   = (current_lsb * __INA226_POWER_LSB_MULTIPLIER);
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
HAL_StatusTypeDef INA226_Get_Shunt_Vltg_V(INA226_Handle_TypeDef_t *hfault, float *pData)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t shnt_vlt_raw;
	HAL_StatusTypeDef status;
	status =  _INA226_ReadReg_Signed(hfault,INA226_REG_SHUNT_VLTG,&shnt_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	float shunt_vltg = (shnt_vlt_raw * __INA226_SHUNT_VOLTAGE_LSB_V);

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
HAL_StatusTypeDef INA226_Get_Bus_Vltg_V(INA226_Handle_TypeDef_t *hfault, float *pData)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t bus_vlt_raw;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault,INA226_REG_BUS_VLTG,&bus_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	float bus_vltg = (bus_vlt_raw * __INA226_BUS_VOLTAGE_LSB_V);

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
HAL_StatusTypeDef INA226_Get_Current_A(INA226_Handle_TypeDef_t *hfault, float *pData)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t alert;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault, INA226_REG_MSK_EN, &alert);
	if(status != HAL_OK)
	{
		return status;
	}
	if(alert & __INA226_OVF_BIT)
	{
		return __INA226_STATUS_OVF;
	}
	int16_t cur_raw;
	status = _INA226_ReadReg_Signed(hfault,INA226_REG_CURRENT,&cur_raw);
	if(status != HAL_OK)
	{
		return status;
	}
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
HAL_StatusTypeDef INA226_Get_Power_W(INA226_Handle_TypeDef_t *hfault, float *pData)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint16_t alert;
	status = INA226_ReadReg(hfault, INA226_REG_MSK_EN, &alert);
	if(status != HAL_OK)
	{
		return status;
	}
	if(alert & __INA226_OVF_BIT)
	{
		return __INA226_STATUS_OVF;
	}
	uint16_t power_raw;
	status = INA226_ReadReg(hfault,INA226_REG_POWER,&power_raw);
	if(status != HAL_OK)
	{
		return status;
	}

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
HAL_StatusTypeDef INA226_Modify_En_Msk(INA226_Handle_TypeDef_t *hfault, INA226_En_Reg_t Ina226_En_msk, uint8_t En_Di)
{
	if (Ina226_En_msk > INA226_EN_MSK_SOL ||
			(Ina226_En_msk >= 0x02 && Ina226_En_msk <= 0x04 ) ||
			(Ina226_En_msk >= 0x05U && Ina226_En_msk <= 0x09))
	{
		return HAL_ERROR;
	}
	if (En_Di > ENABLE)
	{
	    return HAL_ERROR;
	}
	uint16_t reg;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault, INA226_REG_MSK_EN,&reg);
	if(status != HAL_OK)
	{
		return status;
	}

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
HAL_StatusTypeDef INA226_Set_Alert_Val(INA226_Handle_TypeDef_t *hfault, uint16_t val)
{
	return INA226_WriteReg(hfault,INA226_REG_ALERT,val);
}
/**
 * @brief  Reads the status of a specific diagnostic or fault flag from the INA226.
 * @details Reads the 16-bit Mask/Enable Register (06h) over I2C and isolates the
 *          requested flag bit to check if a specific condition (e.g., Math Overflow
 *          or Alert Function Flag) has been asserted.
 *
 * @note    Reading the Mask/Enable Register automatically clears the latched
 *          read-only flags (such as OVF and AFF) on the INA226 silicon.
 *
 * @param[in,out] hfault     Pointer to the INA226 handle structure containing the
 *                           I2C peripheral instance and target device address.
 * @param[in]     Ina226_Flag Target status flag bit position to inspect
 *                           (type ::INA226_Status_Flag_t).
 *
 * @retval Non-zero Bitmask value corresponding to the asserted flag bit.
 * @retval 0        The specified flag is cleared (inactive).
 * @retval HAL_ERROR Input validation failed (NULL pointer, I2C address is 0,
 *                   or flag position is out of range).
 * @retval 0x99     An I2C communication error occurred while reading the register.
 */
uint8_t INA226_Get_Flag_Status(INA226_Handle_TypeDef_t *hfault, INA226_Status_Flag_t Ina226_Flag)
{
	if(hfault == NULL || hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			Ina226_Flag > INA226_FLAG_AFF || Ina226_Flag < INA226_FLAG_OVF)
	{
		return HAL_ERROR;
	}
	uint16_t status_data;
	HAL_StatusTypeDef status;
	status = INA226_ReadReg(hfault, INA226_REG_MSK_EN, &status_data);
	if(status != HAL_OK)
	{
		return 0x99;
	}
	return ((status_data & 1U << Ina226_Flag));
}
