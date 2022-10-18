/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the XMC MCU: CCU8 Complementary PWM
*              Example for ModusToolbox.
*              One CCU8 slice is configured to generate complementary PWM in the
*              project. Deadtime insertion is also done between the
*              complementary PWM waves.
*
* Related Document: See README.md
*
******************************************************************************
*
* Copyright (c) 2015-2022, Infineon Technologies AG
* All rights reserved.
*
* Boost Software License - Version 1.0 - August 17th, 2003
*
* Permission is hereby granted, free of charge, to any person or organization
* obtaining a copy of the software and accompanying documentation covered by
* this license (the "Software") to use, reproduce, display, distribute,
* execute, and transmit the Software, and to prepare derivative works of the
* Software, and to permit third-parties to whom the Software is furnished to
* do so, all subject to the following:
*
* The copyright notices in the Software and this entire statement, including
* the above license grant, this restriction and the following disclaimer,
* must be included in all copies of the Software, in whole or in part, and
* all derivative works of the Software, unless such copies or derivative
* works are solely in the form of machine-executable object code generated by
* a source language processor.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/
#include <stdio.h>
#include "cybsp.h"
#include "cy_utils.h"
#include "cy_retarget_io.h"
#include "xmc_common.h"
#include "xmc_ccu8.h"
#include "xmc_gpio.h"


/*******************************************************************************
* Defines
*******************************************************************************/
/* Define macro to enable/disable printing of debug messages */
#define ENABLE_XMC_DEBUG_PRINT              (0)

/* Define macro to set the loop count before printing debug messages */
#if ENABLE_XMC_DEBUG_PRINT
#define DEBUG_LOOP_COUNT_MAX                (1U)
#endif

#define    PWM_PERIOD                     720U
#define    PWM_COMPARE                    360U

#if (UC_SERIES == XMC14)

#define CCU8_MODULE_PTR                   CCU80
#define PWM_SLICE_PTR                     CCU80_CC80
#define PWM_SLICE_NUMBER                  (0U)
#define PWM_OUTPUT_GPIO_PORT              (XMC_GPIO_PORT_t *) PORT0_BASE
#define PWM_OUTPUT_GPIO_PIN               0U
#define PWM_INV_OUTPUT_GPIO_PORT          (XMC_GPIO_PORT_t *) PORT0_BASE
#define PWM_INV_OUTPUT_GPIO_PIN           1U
#define PWM_SHADOW_TRANSFER_MASK          XMC_CCU8_SHADOW_TRANSFER_SLICE_0
#define COMPARE_CHANNEL                   XMC_CCU8_SLICE_COMPARE_CHANNEL_1
#define SLICE_STATUS_CHANNEL              XMC_CCU8_SLICE_STATUS_CHANNEL_1

#endif

#if (UC_SERIES == XMC47)
#define CCU8_MODULE_PTR                   CCU80
#define PWM_SLICE_PTR                     CCU80_CC82
#define PWM_SLICE_NUMBER                  (2U)
#define PWM_OUTPUT_GPIO_PORT              (XMC_GPIO_PORT_t *) PORT0_BASE
#define PWM_OUTPUT_GPIO_PIN               3U
#define PWM_INV_OUTPUT_GPIO_PORT          (XMC_GPIO_PORT_t *) PORT0_BASE
#define PWM_INV_OUTPUT_GPIO_PIN           0U
#define PWM_SHADOW_TRANSFER_MASK          XMC_CCU8_SHADOW_TRANSFER_SLICE_2
#define COMPARE_CHANNEL                   XMC_CCU8_SLICE_COMPARE_CHANNEL_1
#define SLICE_STATUS_CHANNEL              XMC_CCU8_SLICE_STATUS_CHANNEL_1
#endif

/*******************************************************************************
* Global Data
*******************************************************************************/
/*
 * CCU8 slice configuration structure to generate PWM output
 */
XMC_CCU8_SLICE_COMPARE_CONFIG_t ccu80_slice_pwm_config =
{
        .timer_mode            = (uint32_t)XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,          /* Edge Aligned Mode */
        .monoshot              = (uint32_t)XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,     /* Continous Mode */
        .shadow_xfer_clear     = 0U,                                                    /* Shadow tranfer disabled on timer clear */
        .dither_timer_period   = 0U,                                                    /* Dithering not enabled*/
        .dither_duty_cycle     = 0U,

        .prescaler_mode        = (uint32_t)XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,        /* Normal prescaler mode */

        .mcm_ch1_enable        = 0U,                                                    /* Multi Channel Mode Disabled  for Ch1*/
        .mcm_ch2_enable        = 0U,                                                    /* Multi Channel Mode Disabled for Ch2*/

        .slice_status          = (uint32_t)SLICE_STATUS_CHANNEL,                        /* SLICE_STATUS_CHANNEL drives slice status output */

        .passive_level_out0    = (uint32_t)XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,     /* Out0 passive level set low */
        .passive_level_out1    = (uint32_t)XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,     /* Out1 passive level set low */
        .passive_level_out2    = (uint32_t)XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,     /* Out2 passive level set low */
        .passive_level_out3    = (uint32_t)XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,     /* Out3 passive level set low */

        .asymmetric_pwm        = 0U,                                                    /* Symmetric PWM */
#if !defined(CCU8V3)
        .invert_out0           = 0U,                                                    /* Direct connection to Out0 */
        .invert_out1           = 1U,                                                    /* Inverted connection to Out1 */
        .invert_out2           = 0U,                                                    /* Direct connection to Out2 */
        .invert_out3           = 1U,                                                    /* Inverted connection to Out3 */
#else
        .selector_out0         = XMC_CCU8_SOURCE_OUT0_ST1,                              /* Direct connection to Out0 */
        .selector_out1         = XMC_CCU8_SOURCE_OUT1_INV_ST1,                          /* Inverted connection to Out1 */
        .selector_out2         = XMC_CCU8_SOURCE_OUT2_ST2,                              /* Direct connection to Out2 */
        .selector_out3         = XMC_CCU8_SOURCE_OUT3_INV_ST2,                          /* Inverted connection to Out3 */
#endif
        .prescaler_initval     = 1U,                                                    /* Prescaler divider  value */
        .float_limit           = 0U,                                                    /*  */
        .dither_limit          = 0U,                                                    /* Dithering Range */
        .timer_concatenation   = 0U,                                                    /* Disable concatenation of timer */
};

/*
 * GPIO configuration for PWM outputs (Direct and Invert outputs)
 */
#if (UC_SERIES == XMC14)

const XMC_GPIO_CONFIG_t  ccu8_pwm_gpio1_config    =
{
   .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT5,                          /* Alternate Mode 5 to connect to CCU8 output */
   .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,                                    /* Output level of pin */
   .input_hysteresis    = XMC_GPIO_INPUT_HYSTERESIS_STANDARD                            /* Pad hysteresis of pin */
};
const XMC_GPIO_CONFIG_t  ccu8_pwm_gpio2_config    =
{
   .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT5,                          /* Alternate Mode 5 to connect to CCU8 output */
   .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,                                    /* Output level of pin */
   .input_hysteresis    = XMC_GPIO_INPUT_HYSTERESIS_STANDARD                            /* Pad hysteresis of pin */
};
#endif
#if (UC_SERIES == XMC47)
const XMC_GPIO_CONFIG_t  ccu8_pwm_gpio1_config    =
{
   .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT3,                          /* Alternate Mode 3 to connect to CCU8 output */
   .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,                                    /* Output level of pin */
   .output_strength     = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM                               /* Pad driver mode */
};
const XMC_GPIO_CONFIG_t  ccu8_pwm_gpio2_config    =
{
   .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT3,                          /* Alternate Mode 3 to connect to CCU8 output */
   .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,                                    /* Output level of pin */
   .output_strength     = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM                               /* Pad driver mode */
};
#endif

/*
* Dead Time insertion configuration
*/
const XMC_CCU8_SLICE_DEAD_TIME_CONFIG_t  dt_config =
    {
      .enable_dead_time_channel1         = 1U,                                          /* Dead time insertion enabled for Ch1 */
      .enable_dead_time_channel2         = 0U,                                          /* Dead time insertion disabled for Ch2 */
      .channel1_st_path                  = 1U,                                          /* Dead time inserted for direct output of Ch1 */
      .channel1_inv_st_path              = 1U,                                          /* Dead time inserted for invert output of Ch1 */
      .channel2_st_path                  = 0U,                                          /* Dead time inserted for direct output of Ch2 */
      .channel2_inv_st_path              = 0U,                                          /* Dead time inserted for invert output of Ch2 */
      .div                               = (uint32_t)XMC_CCU8_SLICE_DTC_DIV_1,          /* Dead time pre-scaler divider value*/

      .channel1_st_rising_edge_counter   = 22U,                                         /* Dead time inserted to rising edge of compare Ch1 */
      .channel1_st_falling_edge_counter  = 22U,                                         /* Dead time inserted to falling edge of compare Ch1 */

      .channel2_st_rising_edge_counter   = 0U,                                          /* Dead time inserted to rising edge of compare Ch2 */
      .channel2_st_falling_edge_counter  = 0U,                                          /* Dead time inserted to falling edge of compare Ch2 */
    };

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. This configures the CCU8 to generate complementary
* PWM outputs. The GPIOs are configured for the outputs. Additionally
* dead time insertion configuration is also done for the complementary outputs.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    #if ENABLE_XMC_DEBUG_PRINT
    /* Assign false to disable printing of debug messages*/
    static volatile bool debug_printf = true;
    /* Initialize the current loop count to zero */
    static uint32_t debug_loop_count = 0;
    #endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_HW);

    #if ENABLE_XMC_DEBUG_PRINT
    printf("Initialization done\r\n");
    #endif

    /* Ensure fCCU reaches CCU8 module */
    XMC_CCU8_SetModuleClock(CCU8_MODULE_PTR, XMC_CCU8_CLOCK_SCU);

    /* Initialize CCU8 module */
    XMC_CCU8_Init(CCU8_MODULE_PTR, XMC_CCU8_SLICE_MCMS_ACTION_TRANSFER_PR_CR);

    /* Start the prescaler and restore clocks to slices */
    XMC_CCU8_StartPrescaler(CCU8_MODULE_PTR);

    /* Get the slices out of idle mode */
    XMC_CCU8_EnableClock(CCU8_MODULE_PTR, PWM_SLICE_NUMBER);

    /* Initialize the Slices */
    XMC_CCU8_SLICE_CompareInit(PWM_SLICE_PTR, &ccu80_slice_pwm_config);

    /* Program timer period for both ccu8 slices */
    XMC_CCU8_SLICE_SetTimerPeriodMatch(PWM_SLICE_PTR, PWM_PERIOD);

    /* Program initial compare match value for ccu8 slice */
    XMC_CCU8_SLICE_SetTimerCompareMatch(PWM_SLICE_PTR, COMPARE_CHANNEL, PWM_COMPARE);

    /* Configure GPIO outputs for the ccu8 slice */
    XMC_GPIO_Init(PWM_OUTPUT_GPIO_PORT, PWM_OUTPUT_GPIO_PIN, &ccu8_pwm_gpio1_config);
    XMC_GPIO_Init(PWM_INV_OUTPUT_GPIO_PORT, PWM_INV_OUTPUT_GPIO_PIN, &ccu8_pwm_gpio2_config);

    /* Enable shadow transfer */
    XMC_CCU8_EnableShadowTransfer(CCU8_MODULE_PTR, PWM_SHADOW_TRANSFER_MASK);

    /* Configure Dead time insertion between transitions */
    XMC_CCU8_SLICE_DeadTimeInit(PWM_SLICE_PTR, &dt_config);

    /* Start timer */
    XMC_CCU8_SLICE_StartTimer(PWM_SLICE_PTR);

    for (;;)
    {
        #if ENABLE_XMC_DEBUG_PRINT
        debug_loop_count++;

        if (debug_printf && debug_loop_count == DEBUG_LOOP_COUNT_MAX)
        {
            debug_printf = false;
            /* Print message after the loop has run DEBUG_LOOP_COUNT_MAX times */
            printf("Generated complementary PWM waves\r\n");
        }
        #endif
    }
}

/* [] END OF FILE */
