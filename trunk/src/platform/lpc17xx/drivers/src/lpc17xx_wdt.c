/**
 * @file	: lpc17xx_wdt.c
 * @brief	: Contains all functions support for WDT firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 9. April. 2009
 * @author	: HieuNguyen
 *----------------------------------------------------------------------------
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup WDT
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_wdt.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"


/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc17xx_libcfg.h"
#else
#include "lpc17xx_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */


#ifdef _WDT

/* Private Functions ---------------------------------------------------------- */
/** @defgroup WDT_Private_Functions
 * @{
 */

/********************************************************************//**
 * @brief 		Set WDT time out value and WDT mode
 * @param[in]	clk_source select Clock source for WDT device
 * @param[in]	timeout value of time-out for WDT (us)
 * @return		None
 *********************************************************************/
uint8_t WDT_SetTimeOut (uint8_t clk_source, uint32_t timeout)
{

	uint32_t pclk_wdt = 0;
	uint32_t tempval = 0;

	switch ((WDT_CLK_OPT) clk_source)
    {
    case WDT_CLKSRC_IRC:
    	pclk_wdt = 4000000;
    	// Calculate TC in WDT
    	tempval  = (((pclk_wdt) / WDT_US_INDEX) * (timeout / 4));
    	// Check if it valid
    	if ((tempval >= WDT_TIMEOUT_MIN) && (tempval <= WDT_TIMEOUT_MAX))
    	{
    		WDT->WDTC = tempval;
    		return	SUCCESS;
    	}

    	break;

    case WDT_CLKSRC_PCLK:

    	// Get WDT clock with CCLK divider = 4
		pclk_wdt = SystemFrequency / 4;
		// Calculate TC in WDT
		tempval  = (((pclk_wdt) / WDT_US_INDEX) * (timeout / 4));

		if ((tempval >= WDT_TIMEOUT_MIN) && (tempval <= WDT_TIMEOUT_MAX))
		{
			CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_WDT, CLKPWR_PCLKSEL_CCLK_DIV_4);
			WDT->WDTC = (uint32_t) tempval;
			return SUCCESS;
		}

		// Get WDT clock with CCLK divider = 2
		pclk_wdt = SystemFrequency / 2;
		// Calculate TC in WDT
		tempval  = (((pclk_wdt) / WDT_US_INDEX) * (timeout / 4));

		if ((tempval >= WDT_TIMEOUT_MIN) && (tempval <= WDT_TIMEOUT_MAX))
		{
			CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_WDT, CLKPWR_PCLKSEL_CCLK_DIV_2);
			WDT->WDTC = (uint32_t) tempval;
			return	SUCCESS;
		}

		// Get WDT clock with CCLK divider = 1
		pclk_wdt = SystemFrequency;
		// Calculate TC in WDT
		tempval  = (((pclk_wdt) / WDT_US_INDEX) * (timeout / 4));

		if ((tempval >= WDT_TIMEOUT_MIN) && (tempval <= WDT_TIMEOUT_MAX))
		{
			CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_WDT, CLKPWR_PCLKSEL_CCLK_DIV_1);
			WDT->WDTC = (uint32_t) tempval;
			return	SUCCESS;
		}
		break ;


    case WDT_CLKSRC_RTC:
		pclk_wdt = 32768;
		// Calculate TC in WDT
		tempval  = (((pclk_wdt) / WDT_US_INDEX) * (timeout / 4));
		// Check if it valid
		if ((tempval >= WDT_TIMEOUT_MIN) && (tempval <= WDT_TIMEOUT_MAX))
		{
			WDT->WDTC = (uint32_t) tempval;
			return	SUCCESS;
		}

		break;

// Error parameter
		default:
			break;
}

	return ERROR;
}

/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup WDT_Public_Functions
 * @{
 */


/*********************************************************************//**
* @brief 		Initial for Watchdog function
* 					Clock source = RTC ,
*
* @param[in]	ClkSrc  Select clock source
* 				-	0:Clock source from Internal RC oscillator
* 				-	1: Selects the APB peripheral clock (PCLK)
* 				-	2:Selects the RTC oscillator
*
* @param[in]	WDTMode WDT mode
* 				-	0: Use WDT to generate interrupt only
* 				-	1:WDT_MODE_RESET
* @return 		None
 **********************************************************************/
void WDT_Init (uint32_t ClkSrc, uint32_t WDTMode)
{
	CHECK_PARAM(PARAM_WDT_CLK_OPT(ClkSrc));
	CHECK_PARAM(PARAM_WDT_MODE_OPT(WDTMode));
	CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_WDT, CLKPWR_PCLKSEL_CCLK_DIV_4);

	//Set clock source
	WDT->WDCLKSEL &= ~WDT_WDCLKSEL_MASK;
	WDT->WDCLKSEL |= ClkSrc;
	//Set WDT mode
	if (WDTMode == WDT_MODE_RESET){
		WDT->WDMOD |= WDT_WDMOD(WDTMode);
	}
}


/**
 * @brief		Start WDT activity with given timeout value
 * @param[in]   TimeOut WDT reset after timeout if it is not feed
 * @return		None
 */
void WDT_Start(uint32_t TimeOut)
{
	uint32_t ClkSrc;

	ClkSrc = WDT->WDCLKSEL;
	ClkSrc &=WDT_WDCLKSEL_MASK;
	WDT_SetTimeOut(ClkSrc,TimeOut);
	//enable watchdog
	WDT->WDMOD |= WDT_WDMOD_WDEN;
	WDT_Feed();
}


/********************************************************************//**
 * @brief 		Read WDT Time out flag
 * @param		None
 * @return		Time out flag status of WDT
 *********************************************************************/
FlagStatus WDT_ReadTimeOutFlag (void)
{
	return ((WDT->WDMOD & WDT_WDMOD_WDTOF) >>2);
}

/********************************************************************//**
 * @brief 		Clear WDT Time out flag
 * @param		None
 * @return		None
 *********************************************************************/
void WDT_ClrTimeOutFlag (void)
{
	WDT->WDMOD &=~WDT_WDMOD_WDTOF;
}

/********************************************************************//**
 * @brief 		Update WDT timeout value and feed
 * @param[in]	TimeOut	TimeOut value to be updated
 * @return		None
 *********************************************************************/
void WDT_UpdateTimeOut ( uint32_t TimeOut)
{
	uint32_t ClkSrc;
	ClkSrc = WDT->WDCLKSEL;
	ClkSrc &=WDT_WDCLKSEL_MASK;
	WDT_SetTimeOut(ClkSrc,TimeOut);
	WDT_Feed();
}


/********************************************************************//**
 * @brief 		After set WDTEN, call this function to start Watchdog
 * 				or reload the Watchdog timer
 * @param	None
 *
 * @return		None
 *********************************************************************/
void WDT_Feed (void)
{
	// Disable irq interrupt
	__disable_irq();
	WDT->WDFEED = 0xAA;
	WDT->WDFEED = 0x55;
	// Then enable irq interrupt
	__enable_irq();
}

/********************************************************************//**
 * @brief 		Get the current value of WDT
 * @param		None
 * @return		current value of WDT
 *********************************************************************/
uint32_t WDT_GetCurrentCount(void)
{
	return WDT->WDTV;
}

/**
 * @}
 */

#endif /* _WDT */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
