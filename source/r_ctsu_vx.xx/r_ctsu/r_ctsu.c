/*
* Copyright (c) 2021 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#ifdef QE_TOUCH_CONFIGURATION
 #include "qe_touch_define.h"
#endif
#include "r_ctsu.h"
#include "r_cg_userdefine.h"
#if (CTSU_CFG_DTC_USE_SC == 1)
#include "r_cg_macrodriver.h"
#include "Config_DTC.h"
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
  #include "r_cg_macrodriver.h"
  #if defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
   #include "Config_S12AD0.h"
  #elif defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
   #include "Config_ADC.h"
  #endif
 #endif
#endif
#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
  #include "r_ctsu_sms_asm.h"
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
  #include "r_ctsu_sms_asm_mutual.h"
#endif
#endif

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** "CTSU" in ASCII, used to determine if device is open. */
#define CTSU_OPEN                            (0x43545355U)

/* Macro definitions for register setting */
#define CTSU_PON_OFF                         (0)    // CTSU hardware macro power off
#define CTSU_PON_ON                          (1)    // CTSU hardware macro power on
#define CTSU_CSW_OFF                         (0)    // Capacitance switch turned off
#define CTSU_CSW_ON                          (1)    // Capacitance switch turned on

#define CTSU_CR1_MODIFY_BIT                  (0xC8) // MD1, MD0, ATUNE1
#define CTSU_SOVF                            (0x20) // Overflow bit
#define CTSU_CORRECTION_AVERAGE              (32)
#define CTSU_SHIFT_AMOUNT                    (15)

#define CTSU_PCLKB_FREQ_MHZ                  (1000000)
#define CTSU_PCLKB_FREQ_RANGE1               (32)
#define CTSU_PCLKB_FREQ_RANGE2               (64)
#define CTSU_PCLKB_FREQ_RANGE3               (128)
#define CTSU_WAIT_TIME                       (500)

/* Macro definitions for initial offset tuning */
#define CTSU_TUNING_MAX                      (0x03FF)
#define CTSU_TUNING_MIN                      (0x0000)
#define CTSU_TUNING_VALUE_SELF               (15360)
#define CTSU_TUNING_VALUE_MUTUAL             (10240)

#define CTSU_CFG_DECIMAL_POINT               (16)
#define CTSU_CFG_DECIMAL_POINT_MASK          (0x0000FFFF)

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #define CTSU_SST_RECOMMEND                  (0x1F) // The recommend value of SST
 #define CTSU_SST_RECOMMEND_CURRENT          (0x3F) // The recommend value of SST with current
 #define CTSU_SNUM_RECOMMEND                 (0x07) // The value of SNUM should be fixed
 #define CTSU_SNUM_MAX                       (0xFF) // The maximum value of SNUM
 #define CTSU_ICOMP0                         (0x80) // ICOMP0 bit
 #define CTSU_ICOMP1                         (0x40) // ICOMP1 bit
 #define CTSU_ICOMPRST                       (0x20) // ICOMPRST bit
 #define CTSU_CR0_MODIFY_BIT                 (0xC0) // TXVSEL
 #define CTSU_CR2_MODIFY_BIT                 (0x33) // POSEL, ATUNE2, MD2
 #define CTSU_SUADJ_MAX                      (0xFF) // The maximum value of SUADJx
 #define CTSU_SUADJ_SSCNT_ADJ                (0x20) // The value of Adjusting SCADJx by SSCNT
 #define CTSU_MUTUAL_BUF_SIZE                (CTSU_CFG_NUM_SUMULTI * 2)
 #define CTSU_SUMULTI_BASE                   (0x3F) // SUCLK base value (32MHz)

 #if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
  #if BSP_MCU_GROUP_RL78L23
   #define CTSU_DTC_ACTIVATION_SOURCE_NUM    (56)   // DTC activation source number
  #else
   #define CTSU_DTC_ACTIVATION_SOURCE_NUM    (40)   // DTC activation source number
  #endif
 #endif

/* Macro definitions for correction */
 #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
  #define CTSU_CORRECTION_STD_VAL            (19200) // 20UC standard value
  #define CTSU_CORRECTION_STD_UNIT           (1920)  // 2UC value
  #define CTSU_CORRECTION_STD_EXREG          (14400) // External registance standard value
  #define CTSU_CORRECTION_OFFSET_UNIT        (120)   // (7680 / 64)
 #else
  #define CTSU_CORRECTION_STD_VAL            (15360) // 20UC standard value * 0.8
  #define CTSU_CORRECTION_STD_UNIT           (1536)  // 2UC value * 0.8
  #define CTSU_CORRECTION_STD_EXREG          (11520) // External registance standard value
  #define CTSU_CORRECTION_OFFSET_UNIT        (96)    // (7680 / 64) * 0.8
 #endif
 #define CTSU_CORRECTION_SUMULTI             (0x20)  // SUMULTI step
 #define CTSU_CORRECTION_TRIMB_MAX           (0xFF)
 #define CTSU_CORRECTION_TRIMB_SIGN_BIT      (0x80)
 #define CTSU_CORRECTION_RTRIM_THRESHOLD1    (0xA0)
 #define CTSU_CORRECTION_RTRIM_THRESHOLD2    (0x50)
 #define CTSU_CORRECTION_TRIMB_THRESHOLD1    (0xC0)
 #define CTSU_CORRECTION_TRIMB_THRESHOLD2    (0x3F)
 #define CTSU_CORRECTION_BIT16               (0x10000)
 #define CTSU_CORRECTION_BIT10               (0x0400)
 #define CTSU_CORRECTION_BIT9                (0x0200)
 #define CTSU_CORRECTION_BIT8                (0x0100)
 #define CTSU_CORRECTION_BIT7                (0x0080)
 #define CTSU_CORRECTION_BIT6_0              (0x007F)
 #define CTSU_CORRECTION_DIV_PRECISION       (12)

 #if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #if BSP_MCU_GROUP_RL78G22
    #define CTSU_CALIB_REF                   ((1536000 * 10) / CTSU_CFG_VCC_MV) // 1.5V Reference value (1024 * 1500 * 10)
   #else
    #define CTSU_CALIB_REF                   ((6144000 * 10) / CTSU_CFG_VCC_MV) // 1.5V Reference value (4096 * 1500 * 10)
   #endif
  #else
   #define CTSU_CALIB_REF                    ((4915200 * 10) / CTSU_CFG_VCC_MV) // 1.2V Reference value (4096 * 1200 * 10)
  #endif
  #define CTSU_CALIB_AVERAGE_TIME            (64)                               // ADC average time
  #if BSP_MCU_GROUP_RL78G22
   #define CTSU_CALIB_THRESHOLD              ((0x400 * 4) / CTSU_CFG_VCC_MV)   // RTRIM calib threshold (1024 * 4)
  #else
   #define CTSU_CALIB_THRESHOLD              ((0x1000 * 4) / CTSU_CFG_VCC_MV)   // RTRIM calib threshold (4096 * 4)
  #endif
  #define CTSU_CALIB_CTSUSO                  (0x3C0)                            // 150uA offset
  #define CTSU_CALIB_ADSSTRL                 (0x3F)                             // Sampling time
 #endif
#endif

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
 #define CTSU_SET_LOW(a)                     (uint8_t) ((uint16_t) (a) & 0xFF);
 #define CTSU_SET_HI(a)                      (uint8_t) (((uint16_t) (a) >> 8) & 0xFF);

 #define CTSU_NUM_ELEMENTS                   (CTSU_CFG_SMS_ELEMENT_NUM)

 #define CTSU_CFG_SMS_NUM_ELEMENTS           (8)

 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
  #define CTSU_SMS_PAGE_NUM_SELF             (7)
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
  #define CTSU_SMS_PAGE_NUM_MUTUAL           (8)
 #endif

 #define CTSU_SMS_WORK_NUM                   (8)

 #define CTSU_SMS_REF_AVERAGE                (7)

 #define CTSU_SMS_INST_SIZE                  (32)
 #define CTSU_SMS_GREG_SIZE                  (16)

 #define CTSU_SMS_VARIABLE_INST_SIZE         (29)
 #define CTSU_SMS_VARIABLE_GREG_SIZE         (5)

#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
 #define CTSU_TXVSEL                         (0x80) // TXVSEL bit
 #define CTSU_SST_RECOMMEND                  (0x10) // The value of SST should be fixed to 00010000b
 #define CTSU_SNUM_MAX                       (0x3F) // The maximum value of SNUM
 #define CTSU_SDPA_MAX                       (0x1F) // The maximum value of SDPA
 #define CTSU_PRRATIO_RECOMMEND              (3)    // Recommended setting value
 #define CTSU_PRMODE_62_PULSES               (2)    // 62 pulses (recommended setting value)
 #define CTSU_SOFF_ON                        (0)    // High-pass noise reduction function turned on
 #define CTSU_SSMOD                          (0)    // The value of SSMOD should be fixed to 00b
 #define CTSU_SSCNT                          (3)    // The value of SSCNT should be fixed to 11b
 #define CTSU_RICOA_RECOMMEND                (0x0F) // Recommended setting value
 #define CTSU_ICOG_100                       (0)    // ICOG = 100%
 #define CTSU_ICOG_66                        (1)    // ICOG = 66%
 #define CTSU_ICOG_50                        (2)    // ICOG = 50%
 #define CTSU_ICOG_40                        (3)    // ICOG = 40%
 #define CTSU_MUTUAL_BUF_SIZE                (1)

/* Macro definitions for correction */
 #if (CTSU_CFG_MCU_PROCESS_MF3 == 1)
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_CORRECTION_1ST_STD_VAL       (40960UL)       // ICOG = 66%
   #define CTSU_CORRECTION_2ND_STD_VAL       (24824)         // ICOG = 40%, (x = 40960 * 40 / 66)
  #else
   #define CTSU_CORRECTION_1ST_STD_VAL       (32768UL)       // ICOG = 66%
   #define CTSU_CORRECTION_2ND_STD_VAL       (19859)         // ICOG = 40%, (x = 40960 * 40 / 66)
  #endif
  #define CTSU_WAFER_PARAMETER               (0.96523525)
  #define CTSU_ICOG_RECOMMEND                (CTSU_ICOG_66)  // Recommended setting value
 #endif
 #if (CTSU_CFG_MCU_PROCESS_40N_PHASE2 == 1)
  #define CTSU_CORRECTION_1ST_STD_VAL        (27306UL)       // ICOG = 66%, (x = 40960 * 66 / 100)
  #define CTSU_CORRECTION_2ND_STD_VAL        (16384)         // ICOG = 40%, (x = 40960 * 40 / 100)
  #define CTSU_WAFER_PARAMETER               (1)
  #define CTSU_ICOG_RECOMMEND                (CTSU_ICOG_100) // Recommended setting value
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

  #define CTSU_DIAG_DAC_1UC               (0x10)  // 0x10 for so dac value
  #define CTSU_DIAG_DAC_2UC               (0x20)  // 0x20 for so dac value
  #define CTSU_DIAG_DAC_4UC               (0x40)  // 0x40 for so dac value
  #define CTSU_DIAG_DAC_8UC               (0x80)  // 0x80 for so dac value
  #define CTSU_DIAG_DAC_16UC              (0x100) // 0x100 for so dac value

  #define CTSU_DIAG_DAC_SO_MAX            (0x3FF) // so dac max

  #define CTSU_DIAG_TSOC_BIT              (0x80)  // TSOC bit 1
  #define CTSU_DIAG_CLKSEL1_BIT           (0x40)  // clksel bit 1
  #define CTSU_DIAG_DAC_INIT_VALUE        (241)   // SO value of dac test
  #define CTSU_DIAG_DAC_TARGET_VALUE      (15360) // 6UC value dac test target
  #define CTSU_DIAG_DAC_START_VALUE       (0x100) // so value dac test tuning
  #define CTSU_DAC_TEST_ATUNE1            (0x08)  // ATUNE1 bit 1

  #define CTSU_DIAG_DAC_INIT_0            (0x0)    // so dac Lower current source0 init
  #define CTSU_DIAG_DAC_INIT_1            (0x100)  // so dac Lower current source1 init
  #define CTSU_DIAG_DAC_INIT_2            (0x200)  // so dac Lower current source2 init
  #define CTSU_DIAG_DAC_INIT_3            (0x300)  // so dac Lower current source3 init

  #define CTSU_LDO_RESISTOR_MASK          (0x0700U)
  #define CTSU_LDO_RESISTOR_1             (0x0100U)
  #define CTSU_LDO_RESISTOR_2             (0x0200U)
  #define CTSU_LDO_RESISTOR_3             (0x0400U)

  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_CFG_DIAG_CCO_HIGH_MAX     (54888)
   #define CTSU_CFG_DIAG_CCO_HIGH_MIN     (29062)
   #define CTSU_CFG_DIAG_CCO_LOW_MAX      (3269)
   #define CTSU_CFG_DIAG_CCO_LOW_MIN      (705)
   #define CTSU_CFG_DIAG_SSCG_MAX         (21813)
   #define CTSU_CFG_DIAG_SSCG_MIN         (11782)
   #define CTSU_CFG_DIAG_DAC1_MAX         (20422)
   #define CTSU_CFG_DIAG_DAC2_MAX         (21532)
   #define CTSU_CFG_DIAG_DAC3_MAX         (23015)
   #define CTSU_CFG_DIAG_DAC4_MAX         (25923)
   #define CTSU_CFG_DIAG_DAC5_MAX         (32016)
   #define CTSU_CFG_DIAG_DAC6_MAX         (45795)
   #define CTSU_CFG_DIAG_DAC1_MIN         (12492)
   #define CTSU_CFG_DIAG_DAC2_MIN         (14053)
   #define CTSU_CFG_DIAG_DAC3_MIN         (15322)
   #define CTSU_CFG_DIAG_DAC4_MIN         (18070)
   #define CTSU_CFG_DIAG_DAC5_MIN         (23262)
   #define CTSU_CFG_DIAG_DAC6_MIN         (33446)
   #define CTSU_DIAG_TSCAP_RANGE_LOW      (1096)
   #define CTSU_DIAG_TSCAP_RANGE_HIGH     (1305)
   #define CTSU_DIAG_TSCAP_MES_VOLTAGE    (5500)
  #else
   #define CTSU_CFG_DIAG_CCO_HIGH_MAX     (43910)
   #define CTSU_CFG_DIAG_CCO_HIGH_MIN     (23249)
   #define CTSU_CFG_DIAG_CCO_LOW_MAX      (2615)
   #define CTSU_CFG_DIAG_CCO_LOW_MIN      (564)
   #define CTSU_CFG_DIAG_SSCG_MAX         (21813)
   #define CTSU_CFG_DIAG_SSCG_MIN         (11782)
   #define CTSU_CFG_DIAG_DAC1_MAX         (16599)
   #define CTSU_CFG_DIAG_DAC2_MAX         (17226)
   #define CTSU_CFG_DIAG_DAC3_MAX         (18412)
   #define CTSU_CFG_DIAG_DAC4_MAX         (20738)
   #define CTSU_CFG_DIAG_DAC5_MAX         (25613)
   #define CTSU_CFG_DIAG_DAC6_MAX         (36636)
   #define CTSU_CFG_DIAG_DAC1_MIN         (9994)
   #define CTSU_CFG_DIAG_DAC2_MIN         (11242)
   #define CTSU_CFG_DIAG_DAC3_MIN         (12258)
   #define CTSU_CFG_DIAG_DAC4_MIN         (14456)
   #define CTSU_CFG_DIAG_DAC5_MIN         (18610)
   #define CTSU_CFG_DIAG_DAC6_MIN         (26757)
   #define CTSU_DIAG_TSCAP_RANGE_LOW      (2513)
   #define CTSU_DIAG_TSCAP_RANGE_HIGH     (2991)
   #define CTSU_DIAG_TSCAP_MES_VOLTAGE    (2400)
  #endif
  #define CTSU_CFG_DIAG_DAC1_TYP          (1085)
  #define CTSU_CFG_DIAG_DAC2_TYP          (1085)
  #define CTSU_CFG_DIAG_DAC3_TYP          (1085)
  #define CTSU_CFG_DIAG_DAC4_TYP          (949)
  #define CTSU_CFG_DIAG_DAC5_TYP          (814)
  #define CTSU_CFG_DIAG_DAC6_TYP          (678)
  #define CTSU_CFG_DIAG_DAC7_TYP          (407)
  #define CTSU_CFG_DIAG_DAC8_TYP          (1152)
  #define CTSU_CFG_DIAG_DAC_HDIFF         (114)
  #define CTSU_CFG_DIAG_DAC_LDIFF         (91)
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
  #if defined(BSP_MCU_GROUP_RL78G22)
/* Convert 10bit ADC error to 12bit to unify the threshold calculation */
   #define CTSU_DIAG_ADC_MEASUREMENT_ERROR                       ((CTSU_DIAG_ADC_OFFSET_ERR_10B + \
                                                                   CTSU_DIAG_ADC_ABS_ACC_ERR_10B) << 2)
  #else
   #define CTSU_DIAG_ADC_MEASUREMENT_ERROR                       (CTSU_DIAG_ADC_OFFSET_ERR_12B + \
                                                                  CTSU_DIAG_ADC_ABS_ACC_ERR_12B)
  #endif

  #define CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT           (0x0)    /* Select low  unit of DAC lower current source for diagnosis */
  #define CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT          (0x100)  /* Select high unit of DAC lower current source for diagnosis */
  #define CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT           (0x200)  /* Select low  unit of DAC upper current source for diagnosis */
  #define CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT          (0x300)  /* Select high unit of DAC upper current source for diagnosis */

  #define CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_DATA_MAX         (0x0FF)  /* Max value of low  unit of DAC lower current source for diagnosis */
  #define CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_DATA_MAX        (0x003)  /* Max value of high unit of DAC lower current source for diagnosis */

  #define CTSU_DIAG_SUCLK_LOWER_CURRENT_SELECT                  (0x80)   /* Select lower current source for SUCLK gain diagnosis */
  #define CTSU_DIAG_SUCLK_UPPER_CURRENT_SELECT                  (0x0)    /* Select upper current source for SUCLK gain diagnosis */

  #define CTSU_DIAG_CLOCK_RECOVERY_SST_RECOMMEND                (0x3F)   /* Recommended setting value of SST for clock recovery diagnosis */

  #define CTSU_DIAG_CCODAC_GAIN_1_UNIT                          (0x1F)   /* Turn on 1 unit of CCODAC current source */
  #define CTSU_DIAG_CCODAC_GAIN_8_UNIT                          (0xFF)   /* Turn on 8 unit of CCODAC current source */

  #define CTSU_DIAG_OUTPUT_VOLTAGE_TEST_NUM                     (8)      /* Number of diagnostic tests for output voltage */
  #define CTSU_DIAG_OVER_VOLTAGE_TEST_NUM                       (2)      /* Number of diagnostic tests for over voltage */
  #define CTSU_DIAG_OVER_CURRENT_TEST_NUM                       (2)      /* Number of diagnostic tests for over current */
  #define CTSU_DIAG_LOAD_RESISTANCE_TEST_NUM                    (5)      /* Number of diagnostic tests for load resistance */
  #define CTSU_DIAG_CURRENT_SOURCE_TEST_NUM                     (27)     /* Number of diagnostic tests for current source */
  #define CTSU_DIAG_SENSCLK_TEST_NUM                            (12)     /* Number of diagnostic tests for sensclk */
  #define CTSU_DIAG_SUCLK_TEST_NUM                              (24)     /* Number of diagnostic tests for suclk */

  #if defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM                    (4)      /* Number of diagnostic tests for clock recovery (NM mode) */
  #elif defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM                    (3)      /* Number of diagnostic tests for clock recovery */
  #endif

  #define CTSU_DIAG_AVERAGE_SHIFT_NUM                           (2)      /* Number of shifts required to achieve a 4-time average */
  #define CTSU_DIAG_AVERAGE_NUM                                 (4)      /* Number of samples used for averaging in CTSU diagnosis */
  #define CTSU_DIAG_PERCENT_BASE                                (100)    /* Reference value for percentage calculation */

/* Output voltage diagnosis macro */
  #if defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
   #define ADC_CHANNEL_VOLT                                     (ADINTERREFVOLT)
   #define ADC_CHANNEL_TSCAP_VOLT                               (ADCHANNEL29)
  #elif defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
   #define ADC_CHANNEL_VOLT                                     (ADINTERREFVOLT)
   #define ADC_CHANNEL_TSCAP_VOLT                               (ADTSCAP)
  #endif

/* Over voltage diagnosis macro */
  #define CTSU_DIAG_OFFSET_CURRENT_80UA                         (0x0200) /* SO[9:0] value for 80uA offset current */

/* Over voltage and Over current diagnosis macro */
  #define CTSU_DIAG_OFFSET_CURRENT_0UA                          (0x0000) /* SO[9:0] value for 0uA offset current */

/* TSCAP output voltage diagnosis macro */
  #define CTSU_DIAG_INTERNAL_VREF_TYP_MV                        (1480L)  /* Internal reference voltage theoretical value [mV] */

  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_DIAG_TSCAP_VREF_TYP_MV                          (1500L)  /* TSCAP voltage theoretical value [mV] (NM mode) */
  #else
   #define CTSU_DIAG_TSCAP_VREF_TYP_MV                          (1200L)  /* TSCAP voltage theoretical value [mV] (LV mode) */
  #endif

/* Load resistance diagnosis macro */
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_DIAG_LOAD_RESISTANCE_THEORETICAL_COUNT_VALUE    (19200L) /* Theoretical count value of load resistance (NM mode)  */
   #define CTSU_DIAG_IDLE_CURRENT_THEORETICAL_COUNT_VALUE       (1920L)  /* Theoretical count value of IDLE current (NM mode)  */
  #else
   #define CTSU_DIAG_LOAD_RESISTANCE_THEORETICAL_COUNT_VALUE    (15360L) /* Theoretical count value of load resistance (LV mode) */
   #define CTSU_DIAG_IDLE_CURRENT_THEORETICAL_COUNT_VALUE       (1536L)  /* Theoretical count value of IDLE current (LV mode) */
  #endif
  #define CTSU_DIAG_LOAD_RESISTANCE_VARIATION_SCALE_SHIFT       (9)

/* Current source diagnosis macro */
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_DIAG_UPPER_CURRENT_SOURCE_DIFF_VALUE            (7680L)
   #define CTSU_DIAG_LOWER_CURRENT_SOURCE_DIFF_VALUE            (960L)
  #else
   #define CTSU_DIAG_UPPER_CURRENT_SOURCE_DIFF_VALUE            (6144L)
   #define CTSU_DIAG_LOWER_CURRENT_SOURCE_DIFF_VALUE            (768L)
  #endif

/* SENSCLK gain and SUCLK gain (DWA) macro */
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)

/* Minimum threshold for differential value (NM mode) */
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MIN              (1048)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MIN              (1004)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MIN              (964)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MIN              (928)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MIN              (890)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MIN              (854)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MIN              (812)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MIN              (770)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MIN             (714)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MIN            (656)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MIN            (598)

/* Maximum threshold for differential value (NM mode) */
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MAX              (3218)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MAX              (3018)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MAX              (2922)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MAX              (2788)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MAX              (2696)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MAX              (2600)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MAX              (2530)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MAX              (2436)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MAX             (2350)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MAX            (2262)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MAX            (2180)

  #else

/* Minimum threshold for differential value (LV mode) */
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MIN              (834)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MIN              (815)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MIN              (794)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MIN              (772)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MIN              (749)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MIN              (724)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MIN              (699)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MIN              (671)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MIN             (643)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MIN            (613)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MIN            (582)

/* Maximum threshold for differential value (LV mode)*/
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MAX              (2600)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MAX              (2494)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MAX              (2400)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MAX              (2318)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MAX              (2244)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MAX              (2178)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MAX              (2118)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MAX              (2062)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MAX             (2010)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MAX            (1958)
   #define CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MAX            (1905)
  #endif

/* SUCLK gain (NO_DWA) macro */
  #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
   #define CTSU_DIAG_NO_DWA_SUCLK_GAIN_MAX_VALUE            (3709L) /* Maximum threshold without DWA */
   #define CTSU_DIAG_NO_DWA_SUCLK_GAIN_MIN_VALUE            (763L)  /* Minimum threshold without DWA */
  #else
   #define CTSU_DIAG_NO_DWA_SUCLK_GAIN_MAX_VALUE            (3142L) /* Maximum threshold without DWA */
   #define CTSU_DIAG_NO_DWA_SUCLK_GAIN_MIN_VALUE            (547L)  /* Minimum threshold without DWA */
  #endif

/* SUCLK clock recovery macro */
  #define CTSU_DIAG_CLOCK_RECOVERY_FREQUENCY_32MHZ_SHIFT    (5)     /* Number of shifts required to calculate the frequency ratio for 32 MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST1_FREQUENCY_MHZ      (16)    /* Clock recovery TEST1 frequency: 16MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST2_FREQUENCY_MHZ      (24)    /* Clock recovery TEST2 frequency: 24MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST1_FREQUENCY          (31)    /* Clock recovery TEST1 with SUCLK frequency set to 16MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST2_FREQUENCY          (47)    /* Clock recovery TEST2 with SUCLK frequency set to 24MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST1_MIN_VALUE          (4092)  /* Minimum threshold for clock recovery TEST1 at 16MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST1_MAX_VALUE          (4100)  /* Maximum threshold for clock recovery TEST1 at 16MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST2_MIN_VALUE          (6140)  /* Minimum threshold for clock recovery TEST2 at 24MHz */
  #define CTSU_DIAG_CLOCK_RECOVERY_TEST2_MAX_VALUE          (6148)  /* Maximum threshold for clock recovery TEST2 at 24MHz */

  #if defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY_MHZ     (32)    /* Clock recovery TEST3 frequency: 32MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY         (63)    /* Clock recovery TEST3 with SUCLK frequency set to 32MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_MIN_VALUE         (8188)  /* Minimum threshold for clock recovery TEST3 at 32MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_MAX_VALUE         (8196)  /* Maximum threshold for clock recovery TEST3 at 32MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST4_FREQUENCY_MHZ     (38)    /* Clock recovery TEST4 frequency: 38MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST4_FREQUENCY         (75)    /* Clock recovery TEST4 with SUCLK frequency set to 38MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST4_MIN_VALUE         (9722)  /* Minimum threshold for clock recovery TEST4 at 38MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST4_MAX_VALUE         (9734)  /* Maximum threshold for clock recovery TEST4 at 38MHz */
  #elif defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY_MHZ     (30)    /* Clock recovery TEST3 frequency: 30MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY         (59)    /* Clock recovery TEST3 with SUCLK frequency set to 30MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_MIN_VALUE         (7676)  /* Minimum threshold for clock recovery TEST3 at 30MHz */
   #define CTSU_DIAG_CLOCK_RECOVERY_TEST3_MAX_VALUE         (7684)  /* Maximum threshold for clock recovery TEST3 at 30MHz */
  #endif

 #endif
#endif


#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
 #define CTSU_SERIAL_TUNING_ELEMENT              (1) // To avoid reading outside the array in the case of mutual 1 x N configuration
#else
 #define CTSU_SERIAL_TUNING_ELEMENT              (0) // No additional buffer is required
#endif

#if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
 #define CTSU_AUTO_JUDGE_ELEMENT_NUM             (CTSU_CFG_NUM_AUTOJUDGE_SELF_ELEMENTS + \
                                                  CTSU_CFG_NUM_AUTOJUDGE_MUTUAL_ELEMENTS)
 #define CTSU_AUTO_FINAL_JUDGE_BIT               (4)          // Definition for bit-shifting the final judgement bit of the automatic judgement result register
 #define CTSU_AUTO_MINUS_BIT_MASK                (0x0000FFFF) // Mask to assign to the CTSUAJTHR register

 #define CTSU_AUTO_JUDGE_BLOCK_SIZE              (5)          // Block size of automatic judgement register to be transferred by DTC
 #define CTSU_AUTO_CTSUSO_BLOCK_SIZE             (1)          // Block size of CTSUSO register to be transferred by DTC for automatic judgement(same for CTSUSCNT register)
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
  #define CTSU_AUTO_MCACT_BLOCK_SIZE             (1)          // Block size of CTSUMCACT register to be transferred by DTC
 #endif
#endif

#if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
 #define CTSU_AUTO_DECIMAL_POINT_NUM             (12)         // Number of bits after the decimal point of the sensor counter auto correction table register
#endif

#if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
 #define CTSU_AUTO_CURRENT_MODE_20UA             (20)
 #define CTSU_AUTO_CURRENT_MODE_40UA             (40)
 #define CTSU_AUTO_CURRENT_MODE_80UA             (80)
 #define CTSU_AUTO_CURRENT_MODE_160UA            (160)
 #define CTSU_AUTO_REF_COEFFICIENT               (15)  // RefFullScaleCount / OffsetDAC Resolution (15360 / 1024)
 #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
  #define CTSU_AUTO_CORRECTION_OFFSET_DAC_MAX    (160) // OffsetDAC maximum output current [uA]
 #else
  #define CTSU_AUTO_CORRECTION_OFFSET_DAC_MAX    (128) // 160 * 0.8
 #endif
 #define CTSU_AUTO_INT32_OVERFLOW_VALUE          (2147483648)
 #define CTSU_AUTO_INT16_OVERFLOW_VALUE          (32767)
 #define CTSU_AUTO_INT16_UNDERFLOW_VALUE         (-32768)
#endif

#define CTSU_LWORD_SET(_lword, _upper, _lower)  \
    {                                           \
        ((uint16_t *) &(_lword))[0] = _lower;   \
        ((uint16_t *) &(_lword))[1] = _upper;   \
    }
#define CTSU_LWORD_GET(_upper, _lower, _lword)  \
    {                                           \
        _lower = (uint16_t) _lword;             \
        _upper = (uint16_t) (_lword >> 16);     \
    }

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef struct st_ctsu_correction_calc
{
    ctsu_range_t range;
    uint16_t     snum;
    uint16_t     sdpa;
    uint16_t     cfc;
    ctsu_md_t    md;
} ctsu_correction_calc_t;

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
/* Define SMS Data Structure */
typedef struct st_sms_transfer
{
    uint16_t inst[CTSU_SMS_VARIABLE_INST_SIZE];
    uint16_t greg[CTSU_SMS_VARIABLE_GREG_SIZE];
} ctsu_sms_transfer_t;

typedef struct st_ctsu_sms_correction
{
    uint16_t scntsctl;
    uint16_t scntscth;
} ctsu_sms_correction_t;

typedef struct st_ctsu_sms_threshold
{
    int16_t threshold_h;
    int16_t threshold_l;
} ctsu_sms_threshold_t;

typedef struct st_ctsu_sms_result
{
    uint16_t ajmmarl;
    uint16_t ajmmarh;
    uint16_t ajblactl;
    uint16_t ajblacth;
    uint16_t ajblarl;
    uint16_t ajblarh;
    uint16_t ajrrl;
    uint16_t sjccr;
} ctsu_sms_result_t;

typedef struct st_ctsu_sms_ctsuwr
{
    uint16_t so0;
    uint16_t so1;
} ctsu_sms_ctsuwr_t;

typedef struct st_ctsu_sms_work
{
    uint8_t eb0;
    uint8_t eb1;
    uint8_t eb2;
    uint8_t eb3;
    uint8_t eb4;
    uint8_t eb5;
    uint8_t eb6;
    uint8_t eb7;
} ctsu_sms_work_t;

#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
typedef struct st_ctsu_diag_save_reg
{
    uint8_t  ctsucr0;
    uint8_t  ctsucr1;
    uint8_t  ctsusdprs;
    uint8_t  ctsusst;
    uint8_t  ctsuchac0;
    uint8_t  ctsuchac1;
    uint8_t  ctsuchac2;
    uint8_t  ctsuchtrc0;
    uint8_t  ctsuchtrc1;
    uint8_t  ctsuchtrc2;
    uint8_t  ctsudclkc;
    uint16_t ctsuerrs;
} ctsu_diag_save_reg_t;
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
typedef struct st_ctsu_diag_save_reg
{
    uint32_t ctsucra;
    uint32_t ctsucrb;
    uint32_t ctsucalib;
    uint32_t ctsusuclka;
    uint32_t ctsuopt;
} ctsu_diag_save_reg_t;
 #endif
#endif

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else
 #if defined(__ARMCC_VERSION) || defined(__ICCARM__)
typedef void (BSP_CMSE_NONSECURE_CALL * ctsu_prv_ns_callback)(ctsu_callback_args_t * p_args);
 #elif defined(__GNUC__)
typedef BSP_CMSE_NONSECURE_CALL void (*volatile ctsu_prv_ns_callback)(ctsu_callback_args_t * p_args);
 #endif
#endif

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
static fsp_err_t ctsu_transfer_open(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_transfer_close(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_transfer_configure(ctsu_instance_ctrl_t * const p_instance_ctrl);


static void ctsu_transfer_normal(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_transfer_normal_ctsuwr(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_transfer_normal_ctsurd(ctsu_instance_ctrl_t * const p_instance_ctrl);

  #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
static void ctsu_transfer_autojudge(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_transfer_autojudge_ctsuwr(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_transfer_autojudge_ctsurd(ctsu_instance_ctrl_t * const p_instance_ctrl);
  #endif
#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
static fsp_err_t ctsu_sms_transfer_configure (ctsu_instance_ctrl_t * const p_instance_ctrl);
  #endif

#endif
static void ctsu_initial_offset_tuning(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_moving_average(ctsu_data_t * p_average, uint16_t new_data, uint16_t average_num);
static void ctsu_end_interrupt(ctsu_instance_ctrl_t * const p_instance_ctrl);

#if defined(__CCRL__)
 void        ctsu_write_isr(void);
 void        ctsu_read_isr(void);
 void        ctsu_end_isr(void);
#elif defined(__ICCRL78__)
 __interrupt void ctsu_write_isr(void);
 __interrupt void ctsu_read_isr(void);
 __interrupt void ctsu_end_isr(void);
#elif defined(__llvm__)
 void ctsu_write_isr(void) __attribute__ ((interrupt));
 void ctsu_read_isr(void) __attribute__ ((interrupt));
 void ctsu_end_isr(void) __attribute__ ((interrupt));
#endif
static void ctsu_correction_process(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_correction_measurement(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * data);

static void ctsu_correction_exec(ctsu_instance_ctrl_t * const p_instance_ctrl);

#if ((CTSU_CFG_AUTO_CORRECTION_ENABLE == 0) || (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1))
static void ctsu_correction_calc(uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc);

#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
static void ctsu_correction_ctsu1_exec(ctsu_instance_ctrl_t * const p_instance_ctrl);

#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
static void ctsu_correction_ctsu2_exec(ctsu_instance_ctrl_t * const p_instance_ctrl);

#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
static void ctsu_correction_freq(uint16_t * p_corr, uint32_t * p_so_value, uint16_t * p_mfc);

 #endif
static uint8_t ctsu_correction_multi(uint16_t * p_pri_mfc,
                                     uint16_t * p_snd_mfc,
                                     uint16_t * p_pri_data,
                                     uint16_t * p_snd_data);
#endif
static void ctsu_delay(uint16_t delay_us);
#if (BSP_FEATURE_CTSU_VERSION == 2)

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
static void      ctsu_correction_scan_start(void);
static void      ctsu_correction_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data);

  #if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
static void      ctsu_correction_calib_rtrim(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data);

  #endif
 #endif

 #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
static void ctsu_auto_correction_register_set(ctsu_instance_ctrl_t * const p_instance_ctrl);

 #endif

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
static void ctsu_auto_judge_threshold_calc(ctsu_instance_ctrl_t * const p_instance_ctrl);

 #endif

 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
static void    ctsu_multiclock_auto_correction_register_set(ctsu_instance_ctrl_t * const p_instance_ctrl);
static int16_t ctsu_multiclock_auto_correction_calc(ctsu_instance_ctrl_t * const p_instance_ctrl,
                                                    uint16_t                     element_id,
                                                    uint8_t                      freq);

 #endif
#endif

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
void ctsu_sms_init (ctsu_instance_ctrl_t * const p_instance_ctrl,
                    uint16_t * p_threshold,
                    uint16_t * p_hysteresis,
                    uint16_t count_filter);
void ctsu_sms_start(void);
void ctsu_sms_stop(void);
void ctsu_sms_work_init (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t count_filter);
void ctsu_sms_parameter_init (ctsu_instance_ctrl_t * const p_instance_ctrl,
                              uint16_t * p_threshold,
                              uint16_t * p_hysteresis,
                              uint16_t count_filter);
void ctsu_sms_correction_data_copy(ctsu_instance_ctrl_t * const p_instance_ctrl);
void ctsu_sms_program_setup(ctsu_instance_ctrl_t * const p_instance_ctrl);
void ctsu_sms_elc_init(void);
#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

  #if BSP_MCU_GROUP_RL78G16
  #else
static void ctsu_diag_dac_initial_tuning(void);
  #endif

  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_output_voltage_scan_start(void);
  #endif
static void ctsu_diag_ldo_over_voltage_scan_start(void);
static void ctsu_diag_oscillator_high_scan_start(void);
static void ctsu_diag_oscillator_low_scan_start(void);
static void ctsu_diag_sscg_scan_start(void);
  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_dac_woTS_scan_start(void);
  #else
static void ctsu_diag_dac_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
  #endif

  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_output_voltage_data_get(void);
  #endif
static void ctsu_diag_ldo_over_voltage_data_get(void);
static void ctsu_diag_oscillator_high_data_get(void);
static void ctsu_diag_oscillator_low_data_get(void);
static void ctsu_diag_sscg_data_get(void);
  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_dac_woTS_data_get(void);
  #else
static void ctsu_diag_dac_data_get(void);
  #endif

  #if BSP_MCU_GROUP_RL78G16
static fsp_err_t ctsu_diag_output_voltage_result(void);
  #endif
static fsp_err_t ctsu_diag_ldo_over_voltage_result(void);
static fsp_err_t ctsu_diag_oscillator_high_result(void);
static fsp_err_t ctsu_diag_oscillator_low_result(void);
static fsp_err_t ctsu_diag_sscg_result(void);
  #if BSP_MCU_GROUP_RL78G16
static fsp_err_t ctsu_diag_dac_woTS_result(void);
  #else
static fsp_err_t ctsu_diag_dac_result(void);
  #endif

static void      ctsu_diag_scan_start1(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_diag_data_get1(void);

 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

static void ctsu_diag_regi_store2(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_regi_restore2(void);

static void      ctsu_diag_output_voltage_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void      ctsu_diag_adc_measure_average(uint16_t channel, uint16_t *adc_measured_value);
static uint16_t ctsu_diag_calc_threshold_output_voltage(bool threshold_type, uint16_t internal_vref_value);

static void ctsu_diag_over_voltage_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_over_voltage_data_get(void);

static void ctsu_diag_over_current_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_over_current_data_get(void);

static void ctsu_diag_load_resistance_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_load_resistance_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl);

static void ctsu_diag_current_source_scan_start(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_current_source_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl);

static void ctsu_diag_sensclk_gain_scan_start(void);
static void ctsu_diag_sensclk_gain_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_cco_gain_config_test(void);

static void ctsu_diag_suclk_gain_scan_start(void);
static void ctsu_diag_suclk_gain_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_diag_suclk_gain_config_test(void);

static void ctsu_diag_clock_recovery_scan_start(void);
static void ctsu_diag_clock_recovery_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl);

static void      ctsu_diag_scan_start2(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_diag_data_get2(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_diag_adc_open_check(void);

 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1) && (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
static void      ctsu_adc_init(void);
 #endif
#endif

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

static uint16_t g_ctsu_element_index = 0;
static uint8_t  g_ctsu_element_complete_flag[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS];
#if (BSP_FEATURE_CTSU_VERSION == 2)
static uint8_t g_ctsu_frequency_complete_flag[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS];
#endif
static int32_t       g_ctsu_tuning_diff[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS];
#if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
 #if defined(__CCRL__)
#pragma address      g_ctsu_ctsuwr = CTSU_CFG_CTSUWR_ADDRESS
 #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_CTSUWR_ADDRESS
 #endif
#endif
static ctsu_ctsuwr_t g_ctsu_ctsuwr[(CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS +
                                    CTSU_SERIAL_TUNING_ELEMENT) * CTSU_CFG_NUM_SUMULTI];
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
static uint16_t        g_ctsu_self_element_index = 0;
#if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
 #if defined(__CCRL__)
#pragma address        g_ctsu_self_raw = CTSU_CFG_SELF_RAW_ADDRESS
 #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_SELF_RAW_ADDRESS
 #endif
#endif
static ctsu_self_buf_t g_ctsu_self_raw[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t        g_ctsu_self_corr[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t        g_ctsu_self_mfc[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static ctsu_data_t     g_ctsu_self_data[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_MAJORITY_MODE_ELEMENTS];
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
static uint16_t          g_ctsu_mutual_element_index = 0;
#if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
 #if defined(__CCRL__)
#pragma address          g_ctsu_mutual_raw = CTSU_CFG_MUTUAL_RAW_ADDRESS
 #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_MUTUAL_RAW_ADDRESS
 #endif
#endif
static ctsu_mutual_buf_t g_ctsu_mutual_raw[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_MUTUAL_BUF_SIZE];
static uint16_t          g_ctsu_mutual_pri_corr[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t          g_ctsu_mutual_snd_corr[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t          g_ctsu_mutual_pri_mfc[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t          g_ctsu_mutual_snd_mfc[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static ctsu_data_t       g_ctsu_mutual_pri_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_MAJORITY_MODE_ELEMENTS];
static ctsu_data_t       g_ctsu_mutual_snd_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_MAJORITY_MODE_ELEMENTS];
#endif
static ctsu_correction_info_t g_ctsu_correction_info;

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
 #if BSP_MCU_GROUP_RL78G22
#pragma address g_ctsu_sms_trigger_bit_clr = 0x0FF900U
uint8_t __near g_ctsu_sms_trigger_bit_clr;
#pragma address g_ctsu_sms_trigger_bit_set = 0x0FFA00U
uint8_t __near g_ctsu_sms_trigger_bit_set;
 #endif
 #if defined(__CCRL__)
#pragma address g_ctsu_sms_transfer = CTSU_CFG_SMS_TRANSFER_ADDRESS
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_MUTUAL];
  #elif (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_SELF];
  #endif
#pragma address g_ctsu_sms_ctsuwr = CTSU_CFG_SMS_CTSUWR_ADDRESS
ctsu_sms_ctsuwr_t      g_ctsu_sms_ctsuwr[CTSU_NUM_ELEMENTS][CTSU_CFG_NUM_SUMULTI];
 #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_SMS_TRANSFER_ADDRESS
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_MUTUAL];
  #elif (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_SELF];
  #endif
#pragma location=CTSU_CFG_SMS_CTSUWR_ADDRESS
ctsu_sms_ctsuwr_t      g_ctsu_sms_ctsuwr[CTSU_NUM_ELEMENTS][CTSU_CFG_NUM_SUMULTI];
 #elif defined(__llvm__)
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_MUTUAL];
  #elif (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
ctsu_sms_transfer_t    g_ctsu_sms_transfer[CTSU_SMS_PAGE_NUM_SELF];
  #endif
ctsu_sms_ctsuwr_t      g_ctsu_sms_ctsuwr[CTSU_NUM_ELEMENTS][CTSU_CFG_NUM_SUMULTI];
 #endif

ctsu_sms_correction_t  g_ctsu_sms_correction[CTSU_CORRECTION_POINT_NUM+1];
ctsu_sms_result_t      g_ctsu_sms_result[CTSU_NUM_ELEMENTS][CTSU_CFG_NUM_SUMULTI];
ctsu_sms_threshold_t   g_ctsu_sms_threshold[CTSU_NUM_ELEMENTS][CTSU_CFG_NUM_SUMULTI];
ctsu_sms_work_t        g_ctsu_sms_work[CTSU_SMS_WORK_NUM];

 /* SMS Program */
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
const uint16_t g_ctsu_sms_program[CTSU_SMS_PAGE_NUM_SELF][CTSU_SMS_INST_SIZE] =
{
    {sms_prog_page1}, {sms_prog_page2}, {sms_prog_page3}, {sms_prog_page4},
    {sms_prog_page5}, {sms_prog_page6}, {sms_prog_page7}
};

 /* SMS General Register Data */
const uint16_t g_ctsu_sms_greg[CTSU_SMS_PAGE_NUM_SELF][CTSU_SMS_GREG_SIZE] =
{
    {sms_greg_page1}, {sms_greg_page2}, {sms_greg_page3}, {sms_greg_page4},
    {sms_greg_page5}, {sms_greg_page6}, {sms_greg_page7}
};
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
const uint16_t g_ctsu_sms_mutual_program[CTSU_SMS_PAGE_NUM_MUTUAL][CTSU_SMS_INST_SIZE] =
{
    {sms_prog_mutual_page1}, {sms_prog_mutual_page2}, {sms_prog_mutual_page3}, {sms_prog_mutual_page4},
    {sms_prog_mutual_page5}, {sms_prog_mutual_page6}, {sms_prog_mutual_page7}, {sms_prog_mutual_page8}
};

 /* SMS General Register Data */
const uint16_t g_ctsu_sms_mutual_greg[CTSU_SMS_PAGE_NUM_MUTUAL][CTSU_SMS_GREG_SIZE] =
{
    {sms_greg_mutual_page1}, {sms_greg_mutual_page2}, {sms_greg_mutual_page3}, {sms_greg_mutual_page4},
    {sms_greg_mutual_page5}, {sms_greg_mutual_page6}, {sms_greg_mutual_page7}, {sms_greg_mutual_page8}
};
 #endif
#endif /* CTSU_CFG_SMS_SUPPORT_ENABLE */

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
static ctsu_diag_info_t     g_ctsu_diag_info;
static ctsu_diag_save_reg_t g_ctsu_diag_reg;
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
static ctsu_diag_info_t     g_ctsu_diag_info;
static ctsu_diag_save_reg_t g_ctsu_diag_reg;
 #endif

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
  #if defined(__CCRL__)
#pragma address          g_ctsu_ajthr = CTSU_CFG_AJTHR_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_AJTHR_ADDRESS
  #endif
static uint32_t g_ctsu_ajthr[CTSU_AUTO_JUDGE_ELEMENT_NUM * CTSU_MAJORITY_MODE_ELEMENTS];
  #if defined(__CCRL__)
#pragma address          g_ctsu_ajmmar = CTSU_CFG_AJMMAR_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_AJMMAR_ADDRESS
  #endif
static uint32_t g_ctsu_ajmmar[CTSU_AUTO_JUDGE_ELEMENT_NUM * CTSU_MAJORITY_MODE_ELEMENTS];
  #if defined(__CCRL__)
#pragma address          g_ctsu_ajblact = CTSU_CFG_AJBLACT_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_AJBLACT_ADDRESS
  #endif
static uint32_t g_ctsu_ajblact[CTSU_AUTO_JUDGE_ELEMENT_NUM * CTSU_MAJORITY_MODE_ELEMENTS];
  #if defined(__CCRL__)
#pragma address          g_ctsu_ajblar = CTSU_CFG_AJBLAR_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_AJBLAR_ADDRESS
  #endif
static uint32_t g_ctsu_ajblar[CTSU_AUTO_JUDGE_ELEMENT_NUM * CTSU_MAJORITY_MODE_ELEMENTS];
  #if defined(__CCRL__)
#pragma address          g_ctsu_ajrr = CTSU_CFG_AJRR_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_AJRR_ADDRESS
  #endif
static uint32_t g_ctsu_ajrr[CTSU_AUTO_JUDGE_ELEMENT_NUM * CTSU_MAJORITY_MODE_ELEMENTS];
 #endif
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
  #if defined(__CCRL__)
#pragma address g_ctsu_mcact1 = CTSU_CFG_MCACT1_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_MCACT1_ADDRESS
  #endif
static uint32_t g_ctsu_mcact1[(CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS) * CTSU_CFG_NUM_SUMULTI];
  #if defined(__CCRL__)
#pragma address g_ctsu_mcact2 = CTSU_CFG_MCACT2_ADDRESS
  #elif defined(__ICCRL78__)
#pragma location=CTSU_CFG_MCACT2_ADDRESS
  #endif
static uint32_t g_ctsu_mcact2[(CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS) * CTSU_CFG_NUM_SUMULTI];
 #endif

 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
uint8_t g_ctsu_selected_freq_self[CTSU_CFG_NUM_SELF_ELEMENTS];
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
uint8_t g_ctsu_selected_freq_mutual[CTSU_CFG_NUM_MUTUAL_ELEMENTS];
 #endif
#endif


#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
  #if BSP_MCU_GROUP_RL78G16
uint16_t dac_wots_table[8];
  #else
static const uint16_t dac_oscil_table[6][2] =
{
    {CTSU_CFG_DIAG_DAC1_MAX, CTSU_CFG_DIAG_DAC1_MIN},
    {CTSU_CFG_DIAG_DAC2_MAX, CTSU_CFG_DIAG_DAC2_MIN},
    {CTSU_CFG_DIAG_DAC3_MAX, CTSU_CFG_DIAG_DAC3_MIN},
    {CTSU_CFG_DIAG_DAC4_MAX, CTSU_CFG_DIAG_DAC4_MIN},
    {CTSU_CFG_DIAG_DAC5_MAX, CTSU_CFG_DIAG_DAC5_MIN},
    {CTSU_CFG_DIAG_DAC6_MAX, CTSU_CFG_DIAG_DAC6_MIN},
};
  #endif
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

static const uint16_t cco_gain_threshold_diff_table[11][2] =
{
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_2_1_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_3_2_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_4_3_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_5_4_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_6_5_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_7_6_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_8_7_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MIN,   CTSU_CFG_DIAG_CCO_GAIN_DIFF_9_8_MAX        },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MIN,  CTSU_CFG_DIAG_CCO_GAIN_DIFF_10_9_MAX       },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MIN, CTSU_CFG_DIAG_CCO_GAIN_DIFF_11_10_MAX      },
    {CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MIN, CTSU_CFG_DIAG_CCO_GAIN_DIFF_12_11_MAX      },
};

static const uint16_t clock_recovery_threshold_table[CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM][2] =
{
    {CTSU_DIAG_CLOCK_RECOVERY_TEST1_MIN_VALUE, CTSU_DIAG_CLOCK_RECOVERY_TEST1_MAX_VALUE},
    {CTSU_DIAG_CLOCK_RECOVERY_TEST2_MIN_VALUE, CTSU_DIAG_CLOCK_RECOVERY_TEST2_MAX_VALUE},
    {CTSU_DIAG_CLOCK_RECOVERY_TEST3_MIN_VALUE, CTSU_DIAG_CLOCK_RECOVERY_TEST3_MAX_VALUE},
  #if (CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM == 4)
    {CTSU_DIAG_CLOCK_RECOVERY_TEST4_MIN_VALUE, CTSU_DIAG_CLOCK_RECOVERY_TEST4_MAX_VALUE},
  #endif
};
 #endif
#endif

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
const ctsu_api_t g_ctsu_on_ctsu =
{
    .open                 = R_CTSU_Open,
    .scanStart            = R_CTSU_ScanStart,
    .dataGet              = R_CTSU_DataGet,
    .smsSet               = R_CTSU_SmsSet,
    .scanStop             = R_CTSU_ScanStop,
    .diagnosis            = R_CTSU_Diagnosis,
    .close                = R_CTSU_Close,
    .callbackSet          = R_CTSU_CallbackSet,
    .specificDataGet      = R_CTSU_SpecificDataGet,
    .dataInsert           = R_CTSU_DataInsert,
    .offsetTuning         = R_CTSU_OffsetTuning,
};

ctsu_instance_ctrl_t * gp_ctsu_isr_context;

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
 #if (CTSU_CFG_DTC_USE_SC == 0)
typedef struct
{
    uint8_t  dtccr;
    uint8_t  dtbls;
    uint8_t  dtcct;
    uint8_t  dtrld;
    uint16_t dtsar;
    uint16_t dtdar;
}st_dtc_data;

  #if defined(__CCRL__)
   #if BSP_MCU_GROUP_RL78G22
#pragma address dtc_vectortable = 0x0FFB00U
uint8_t __near dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM];

    #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
#pragma address dtc_controldata_20 = 0x0FFBE0U
st_dtc_data __near dtc_controldata_20;

#pragma address dtc_controldata_23 = 0x0FFBF8U
st_dtc_data __near dtc_controldata_23;
    #endif

#pragma address dtc_controldata_21 = 0x0FFBE8U
st_dtc_data __near dtc_controldata_21;

#pragma address dtc_controldata_22 = 0x0FFBF0U
st_dtc_data __near dtc_controldata_22;

   #else
#pragma address dtc_vectortable = 0x0FFD00U
uint8_t __near dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM];

    #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
#pragma address dtc_controldata_21 = 0x0FFDE8U
st_dtc_data __near dtc_controldata_21;
    #endif
 
    #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
     #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
#pragma address dtc_controldata_10 = 0x0FFD90U
st_dtc_data __near dtc_controldata_10;

#pragma address dtc_controldata_11 = 0x0FFD98U
st_dtc_data __near dtc_controldata_11;
     #endif

#pragma address dtc_controldata_12 = 0x0FFDA0U
st_dtc_data __near dtc_controldata_12;

#pragma address dtc_controldata_13 = 0x0FFDA8U
st_dtc_data __near dtc_controldata_13;

#pragma address dtc_controldata_14 = 0x0FFDB0U
st_dtc_data __near dtc_controldata_14;

#pragma address dtc_controldata_15 = 0x0FFDB8U
st_dtc_data __near dtc_controldata_15;

#pragma address dtc_controldata_16 = 0x0FFDC0U
st_dtc_data __near dtc_controldata_16;

#pragma address dtc_controldata_17 = 0x0FFDC8U
st_dtc_data __near dtc_controldata_17;

#pragma address dtc_controldata_18 = 0x0FFDD0U
st_dtc_data __near dtc_controldata_18;

#pragma address dtc_controldata_19 = 0x0FFDD8U
st_dtc_data __near dtc_controldata_19;

#pragma address dtc_controldata_20 = 0x0FFDE0U
st_dtc_data __near dtc_controldata_20;

#pragma address dtc_controldata_21 = 0x0FFDE8U
st_dtc_data __near dtc_controldata_21;
    #endif

#pragma address dtc_controldata_22 = 0x0FFDF0U
st_dtc_data __near dtc_controldata_22;

#pragma address dtc_controldata_23 = 0x0FFDF8U
st_dtc_data __near dtc_controldata_23;
   #endif

  #elif defined(__ICCRL78__)
   #if BSP_MCU_GROUP_RL78G22
#pragma location=0x0FFB00U
uint8_t __near dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM];

    #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
#pragma location=0x0FFBE0U
st_dtc_data __near dtc_controldata_20;

#pragma location=0x0FFBF8U
st_dtc_data __near dtc_controldata_23;
    #endif

#pragma location=0x0FFBE8U
st_dtc_data __near dtc_controldata_21;

#pragma location=0x0FFBF0U
st_dtc_data __near dtc_controldata_22;

   #else
#pragma location=0x0FFD00U
uint8_t __near dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM];

    #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
#pragma location=0x0FFDE8U
st_dtc_data __near dtc_controldata_21;
    #endif

    #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
     #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
#pragma location=0x0FFD90U
st_dtc_data __near dtc_controldata_10;

#pragma location=0x0FFD98U
st_dtc_data __near dtc_controldata_11;
     #endif

#pragma location=0x0FFDA0U
st_dtc_data __near dtc_controldata_12;

#pragma location=0x0FFDA8U
st_dtc_data __near dtc_controldata_13;

#pragma location=0x0FFDB0U
st_dtc_data __near dtc_controldata_14;

#pragma location=0x0FFDB8U
st_dtc_data __near dtc_controldata_15;

#pragma location=0x0FFDC0U
st_dtc_data __near dtc_controldata_16;

#pragma location=0x0FFDC8U
st_dtc_data __near dtc_controldata_17;

#pragma location=0x0FFDD0U
st_dtc_data __near dtc_controldata_18;

#pragma location=0x0FFDD8U
st_dtc_data __near dtc_controldata_19;

#pragma location=0x0FFDE0U
st_dtc_data __near dtc_controldata_20;

#pragma location=0x0FFDE8U
st_dtc_data __near dtc_controldata_21;
    #endif

#pragma location=0x0FFDF0U
st_dtc_data __near dtc_controldata_22;

#pragma location=0x0FFDF8U
st_dtc_data __near dtc_controldata_23;
   #endif

  #elif defined(__llvm__)
uint8_t dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM] __attribute__ ((section (".dtc_vectortable")));
   #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
    #if BSP_MCU_GROUP_RL78G22
st_dtc_data dtc_controldata_20 __attribute__ ((section (".dtc_controldata_20")));
st_dtc_data dtc_controldata_23 __attribute__ ((section (".dtc_controldata_23")));
    #else
st_dtc_data dtc_controldata_21 __attribute__ ((section (".dtc_controldata_21")));
    #endif
   #endif
   #if BSP_MCU_GROUP_RL78G22
st_dtc_data dtc_controldata_21 __attribute__ ((section (".dtc_controldata_21")));
st_dtc_data dtc_controldata_22 __attribute__ ((section (".dtc_controldata_22")));
   #else

    #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
     #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
st_dtc_data dtc_controldata_10 __attribute__ ((section (".dtc_controldata_10")));
st_dtc_data dtc_controldata_11 __attribute__ ((section (".dtc_controldata_11")));
     #endif
st_dtc_data dtc_controldata_12 __attribute__ ((section (".dtc_controldata_12")));
st_dtc_data dtc_controldata_13 __attribute__ ((section (".dtc_controldata_13")));
st_dtc_data dtc_controldata_14 __attribute__ ((section (".dtc_controldata_14")));
st_dtc_data dtc_controldata_15 __attribute__ ((section (".dtc_controldata_15")));
st_dtc_data dtc_controldata_16 __attribute__ ((section (".dtc_controldata_16")));
st_dtc_data dtc_controldata_17 __attribute__ ((section (".dtc_controldata_17")));
st_dtc_data dtc_controldata_18 __attribute__ ((section (".dtc_controldata_18")));
st_dtc_data dtc_controldata_19 __attribute__ ((section (".dtc_controldata_19")));
st_dtc_data dtc_controldata_20 __attribute__ ((section (".dtc_controldata_20")));
st_dtc_data dtc_controldata_21 __attribute__ ((section (".dtc_controldata_21")));
    #endif

st_dtc_data dtc_controldata_22 __attribute__ ((section (".dtc_controldata_22")));
st_dtc_data dtc_controldata_23 __attribute__ ((section (".dtc_controldata_23")));
   #endif
  #endif

 #else /* CTSU_CFG_DTC_USE_SC == 0 */
  #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
extern uint8_t __near     dtc_vectortable[CTSU_DTC_ACTIVATION_SOURCE_NUM];
   #if BSP_MCU_GROUP_RL78G22
extern st_dtc_data_t __near dtc_controldata_20;
extern st_dtc_data_t __near dtc_controldata_23;
   #else
extern st_dtc_data_t __near dtc_controldata_21;
   #endif
  #endif
  #if BSP_MCU_GROUP_RL78G22
extern st_dtc_data_t __near dtc_controldata_21;
extern st_dtc_data_t __near dtc_controldata_22;
  #else

   #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
extern st_dtc_data_t __near dtc_controldata_20;
extern st_dtc_data_t __near dtc_controldata_21;
   #endif

extern st_dtc_data_t __near dtc_controldata_22;
extern st_dtc_data_t __near dtc_controldata_23;
  #endif
 #endif
#endif /* CTSU_CFG_DTC_SUPPORT_ENABLE == 1 */

/*******************************************************************************************************************//**
 * @addtogroup CTSU
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the CTSU driver module. Implements @ref ctsu_api_t::open.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_Open
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 * @retval FSP_ERR_INVALID_ARGUMENT Configuration parameter error.
 *
 * @note In the first Open, measurement for correction works, and it takes several tens of milliseconds.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_Open (ctsu_ctrl_t * const p_ctrl, ctsu_cfg_t const * const p_cfg)
{
    ctsu_instance_ctrl_t     * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    fsp_err_t                  err             = FSP_SUCCESS;
    uint16_t                   element_id;
    const ctsu_element_cfg_t * element_cfgs;
#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
  #if BSP_MCU_GROUP_RL78G16
    uint16_t correction_factor;
  #endif
 #endif
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
    uint16_t i;
    uint32_t pclkb_mhz;
    uint16_t suadj[CTSU_CFG_NUM_SUMULTI];
#endif

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_cfg);
#endif
    FSP_ERROR_RETURN(CTSU_OPEN != p_instance_ctrl->open, FSP_ERR_ALREADY_OPEN);

    /* Check element number */
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        FSP_ERROR_RETURN(CTSU_CFG_NUM_SELF_ELEMENTS >=
                         (uint8_t) (g_ctsu_self_element_index + p_cfg->num_rx),
                         FSP_ERR_INVALID_ARGUMENT);
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        FSP_ERROR_RETURN(CTSU_CFG_NUM_MUTUAL_ELEMENTS >=
                         (uint8_t) (g_ctsu_mutual_element_index + (p_cfg->num_rx * p_cfg->num_tx)),
                         FSP_ERR_INVALID_ARGUMENT);
    }
#endif
    p_instance_ctrl->state = CTSU_STATE_INIT;

    /* Save configurations */
    p_instance_ctrl->p_ctsu_cfg = p_cfg;

    /* Initialize driver control structure (address setting) */
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        p_instance_ctrl->p_self_raw   = &g_ctsu_self_raw[g_ctsu_self_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_self_corr  = &g_ctsu_self_corr[g_ctsu_self_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_self_mfc   = &g_ctsu_self_mfc[g_ctsu_self_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_self_data  = &g_ctsu_self_data[g_ctsu_self_element_index * CTSU_MAJORITY_MODE_ELEMENTS];
        p_instance_ctrl->num_elements = p_cfg->num_rx;
 #if (BSP_FEATURE_CTSU_VERSION == 2)
        p_instance_ctrl->p_selected_freq_self = &g_ctsu_selected_freq_self[g_ctsu_self_element_index];
 #endif
        g_ctsu_self_element_index        = (uint8_t) (g_ctsu_self_element_index + p_instance_ctrl->num_elements);
        p_instance_ctrl->self_elem_index = g_ctsu_self_element_index;

 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        p_instance_ctrl->p_mutual_raw      = &g_ctsu_mutual_raw[0];
        p_instance_ctrl->p_mutual_pri_corr = &g_ctsu_mutual_pri_corr[0];
        p_instance_ctrl->p_mutual_snd_corr = &g_ctsu_mutual_snd_corr[0];
        p_instance_ctrl->p_mutual_pri_mfc  = &g_ctsu_mutual_pri_mfc[0];
        p_instance_ctrl->p_mutual_snd_mfc  = &g_ctsu_mutual_snd_mfc[0];
        p_instance_ctrl->p_mutual_pri_data = &g_ctsu_mutual_pri_data[0];
        p_instance_ctrl->p_mutual_snd_data = &g_ctsu_mutual_snd_data[0];
 #endif
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        p_instance_ctrl->p_mutual_raw      = &g_ctsu_mutual_raw[g_ctsu_mutual_element_index * CTSU_MUTUAL_BUF_SIZE];
        p_instance_ctrl->p_mutual_pri_corr =
            &g_ctsu_mutual_pri_corr[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_mutual_snd_corr =
            &g_ctsu_mutual_snd_corr[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_mutual_pri_mfc =
            &g_ctsu_mutual_pri_mfc[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_mutual_snd_mfc =
            &g_ctsu_mutual_snd_mfc[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_mutual_pri_data =
            &g_ctsu_mutual_pri_data[g_ctsu_mutual_element_index * CTSU_MAJORITY_MODE_ELEMENTS];
        p_instance_ctrl->p_mutual_snd_data =
            &g_ctsu_mutual_snd_data[g_ctsu_mutual_element_index * CTSU_MAJORITY_MODE_ELEMENTS];
        p_instance_ctrl->num_elements = (uint8_t) (p_cfg->num_rx * p_cfg->num_tx);
 #if (BSP_FEATURE_CTSU_VERSION == 2)
        p_instance_ctrl->p_selected_freq_mutual = &g_ctsu_selected_freq_mutual[g_ctsu_mutual_element_index];
 #endif
        g_ctsu_mutual_element_index        = (uint8_t) (g_ctsu_mutual_element_index + p_instance_ctrl->num_elements);
        p_instance_ctrl->mutual_elem_index = g_ctsu_mutual_element_index;

 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        p_instance_ctrl->p_self_raw  = &g_ctsu_self_raw[0];
        p_instance_ctrl->p_self_corr = &g_ctsu_self_corr[0];
        p_instance_ctrl->p_self_mfc  = &g_ctsu_self_mfc[0];
        p_instance_ctrl->p_self_data = &g_ctsu_self_data[0];
 #endif
    }
#endif

    p_instance_ctrl->p_element_complete_flag = &g_ctsu_element_complete_flag[g_ctsu_element_index];
#if (BSP_FEATURE_CTSU_VERSION == 2)
    p_instance_ctrl->p_frequency_complete_flag = &g_ctsu_frequency_complete_flag[g_ctsu_element_index];
#endif
    p_instance_ctrl->p_tuning_diff = &g_ctsu_tuning_diff[g_ctsu_element_index];
    p_instance_ctrl->p_ctsuwr      = &g_ctsu_ctsuwr[g_ctsu_element_index * CTSU_CFG_NUM_SUMULTI];
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)

    /* Secure DTC transfer area for automatic judgement, store start address and transfer count */
    p_instance_ctrl->p_ajthr       = g_ctsu_ajthr;
    p_instance_ctrl->p_ajmmar      = g_ctsu_ajmmar;
    p_instance_ctrl->p_ajblact     = g_ctsu_ajblact;
    p_instance_ctrl->p_ajblar      = g_ctsu_ajblar;
    p_instance_ctrl->p_ajrr        = g_ctsu_ajrr;
    /* Copy to ctrl variable for setting change for automatic judgement */
    p_instance_ctrl->ajmmat = p_cfg->ajmmat;
    p_instance_ctrl->ajbmat = p_cfg->ajbmat;
 #endif
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
    p_instance_ctrl->p_mcact1   = &g_ctsu_mcact1[g_ctsu_element_index * CTSU_CFG_NUM_SUMULTI];
    p_instance_ctrl->p_mcact2   = &g_ctsu_mcact2[g_ctsu_element_index * CTSU_CFG_NUM_SUMULTI];
    p_instance_ctrl->mcact_flag = 0;
 #endif
#endif
    g_ctsu_element_index             = (uint8_t) (g_ctsu_element_index + p_instance_ctrl->num_elements);
    p_instance_ctrl->ctsu_elem_index = g_ctsu_element_index;

    /* Set Value */
    p_instance_ctrl->cap                = p_cfg->cap;
    p_instance_ctrl->num_moving_average = p_cfg->num_moving_average;
    p_instance_ctrl->average            = 0;
    if (true == p_cfg->tunning_enable)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_INCOMPLETE;
    }
    else
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }

#if (BSP_FEATURE_CTSU_VERSION == 2)
    if (CTSU_MODE_CURRENT_SCAN == p_cfg->md)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }
#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
    p_instance_ctrl->ctsucr1 = (uint8_t) (p_cfg->atune1 << 3);
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
    p_instance_ctrl->ctsucr1 = (uint8_t) ((p_cfg->atune12 & 0x01) << 3);
#endif
    p_instance_ctrl->ctsucr1 |= (uint8_t) (p_cfg->md << 6);

    p_instance_ctrl->txvsel  = p_instance_ctrl->p_ctsu_cfg->txvsel;
    p_instance_ctrl->txvsel2 = p_instance_ctrl->p_ctsu_cfg->txvsel2;

    p_instance_ctrl->ctsuchac0 = p_instance_ctrl->p_ctsu_cfg->ctsuchac0;
    p_instance_ctrl->ctsuchac1 = p_instance_ctrl->p_ctsu_cfg->ctsuchac1;
    p_instance_ctrl->ctsuchac2 = p_instance_ctrl->p_ctsu_cfg->ctsuchac2;
    p_instance_ctrl->ctsuchac3 = p_instance_ctrl->p_ctsu_cfg->ctsuchac3;
    p_instance_ctrl->ctsuchac4 = p_instance_ctrl->p_ctsu_cfg->ctsuchac4;

    p_instance_ctrl->ctsuchtrc0 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc0;
    p_instance_ctrl->ctsuchtrc1 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc1;
    p_instance_ctrl->ctsuchtrc2 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc2;
    p_instance_ctrl->ctsuchtrc3 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc3;
    p_instance_ctrl->ctsuchtrc4 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc4;

    p_instance_ctrl->md = p_instance_ctrl->p_ctsu_cfg->md;

#if (BSP_FEATURE_CTSU_VERSION == 2)
    p_instance_ctrl->ctsucr2  = (uint8_t) (p_cfg->atune12 & 0x02);
    p_instance_ctrl->ctsucr2 |= (uint8_t) ((p_cfg->md & 0x04) >> 2);
    p_instance_ctrl->ctsucr2 |= (uint8_t) (p_cfg->posel << 4);

    if (CTSU_ATUNE12_80UA == p_cfg->atune12)
    {
        p_instance_ctrl->range = CTSU_RANGE_80UA;
    }
    else if (CTSU_ATUNE12_40UA == p_cfg->atune12)
    {
        p_instance_ctrl->range = CTSU_RANGE_40UA;
    }
    else if (CTSU_ATUNE12_20UA == p_cfg->atune12)
    {
        p_instance_ctrl->range = CTSU_RANGE_20UA;
    }
    else if (CTSU_ATUNE12_160UA == p_cfg->atune12)
    {
        p_instance_ctrl->range = CTSU_RANGE_160UA;
    }
    else
    {
    }

 #if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)

    /* Copy the variables used for the MEC function to the ctrl instance */
    p_instance_ctrl->tsod          = p_instance_ctrl->p_ctsu_cfg->tsod;
    p_instance_ctrl->mec_ts        = p_instance_ctrl->p_ctsu_cfg->mec_ts;
    p_instance_ctrl->mec_shield_ts = p_instance_ctrl->p_ctsu_cfg->mec_shield_ts;
 #endif
#endif
    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        p_instance_ctrl->p_element_complete_flag[element_id] = 0;
#if (BSP_FEATURE_CTSU_VERSION == 2)
        p_instance_ctrl->p_frequency_complete_flag[element_id] = 0;
#endif
        p_instance_ctrl->p_tuning_diff[element_id] = 0;
        element_cfgs = (p_cfg->p_elements + element_id);
#if (BSP_FEATURE_CTSU_VERSION == 2)
        if (CTSU_MODE_CURRENT_SCAN == p_cfg->md)
        {
            /* Current scan does not run multiple frequency */
            p_instance_ctrl->p_ctsuwr[element_id].ctsuso =
                (uint32_t) (((uint32_t) element_cfgs->sdpa << (uint32_t) 24) |
                            ((uint32_t) element_cfgs->snum << (uint32_t) 10) | element_cfgs->so);
        }
        else
        {
            if (CTSU_MODE_MUTUAL_CFC_SCAN != p_cfg->md)
            {
                p_instance_ctrl->p_ctsuwr[element_id * CTSU_CFG_NUM_SUMULTI].ctsuso =
                    (uint32_t) (((uint32_t) element_cfgs->sdpa << (uint32_t) 24) |
                                ((uint32_t) element_cfgs->snum << (uint32_t) 10) | element_cfgs->so);
            }
            for (i = 1; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI) + i].ctsuso =
                    p_instance_ctrl->p_ctsuwr[element_id * CTSU_CFG_NUM_SUMULTI].ctsuso;
            }
        }
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
        p_instance_ctrl->p_ctsuwr[element_id].ctsussc = (uint16_t) (element_cfgs->ssdiv << 8);
        p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 = (uint16_t) ((uint16_t) (element_cfgs->snum << 10) | element_cfgs->so);
        p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 =
            (uint16_t) ((CTSU_ICOG_RECOMMEND << 13) | (element_cfgs->sdpa << 8) | CTSU_RICOA_RECOMMEND);
#endif
    }

    /* Enable interrupts for CTSUWR, CTSURD, CTSUFN */
    CTSUWRMK = 0;
    CTSURDMK = 0;
    CTSUFNMK = 0;
    CTSUWRPR0 =  (CTSU_CFG_INTCTSUWR_PRIORITY_LEVEL & 0x01);
    CTSURDPR0 =  (CTSU_CFG_INTCTSURD_PRIORITY_LEVEL & 0x01);
    CTSUFNPR0 =  (CTSU_CFG_INTCTSUFN_PRIORITY_LEVEL & 0x01);
    CTSUWRPR1 = ((CTSU_CFG_INTCTSUWR_PRIORITY_LEVEL & 0x02) >> 1);
    CTSURDPR1 = ((CTSU_CFG_INTCTSURD_PRIORITY_LEVEL & 0x02) >> 1);
    CTSUFNPR1 = ((CTSU_CFG_INTCTSUFN_PRIORITY_LEVEL & 0x02) >> 1);
    gp_ctsu_isr_context = p_instance_ctrl;

    p_instance_ctrl->interrupt_reverse_flag = 0;

    if (p_instance_ctrl->num_elements == g_ctsu_element_index)
    {
        /* TSCAP discharge process */
#if (CTSU_CFG_TSCAP_PORT == 0x0102)
        P1_bit.no2     = 0;
        POM1_bit.no2   = 0;
        ctsu_delay(10);
        POM1_bit.no2   = 1;
        TSPMC1_bit.no2 = 1;
#endif
#if (CTSU_CFG_TSCAP_PORT == 0x0300)
        P3_bit.no0    = 0;
        PM3_bit.no0   = 0;
        ctsu_delay(10);
        PM3_bit.no0   = 1;
        PMCT3_bit.no0 = 1;
#endif
#if (CTSU_CFG_TSCAP_PORT == 0x0507)
        PFSEG2       &= 0xF7;       /* PFSEG11 = 0 : Uses the pin as port */
        P5_bit.no7    = 0;
        PM5_bit.no7   = 0;
        ctsu_delay(10);
        PM5_bit.no7   = 1;
        PMCT5_bit.no7 = 1;
#endif
#if (CTSU_CFG_TSCAP_PORT == 0x0002)
        VTSEL        = 0x00;
        PMC0_bit.no2 = 0;
        P0_bit.no2   = 0;
        PM0_bit.no2  = 0;
        ctsu_delay(10);
        PM0_bit.no2  = 1;
#endif

        /* Get CTSU out of stop state (supply power/clock) */
        CTSUEN = 1;

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_open(p_instance_ctrl);
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
        CTSUCRAL |= CTSU_CFG_LOW_VOLTAGE_MODE << CTSUCRA_ATUNE0_Pos; // ATUNE0
        CTSUCRAL |= CTSUCRA_PUMPON_Msk; // PUMPON

        pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;

        if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
        {
            CTSUCR3  = (uint8_t) (pclkb_mhz - 1);
        }
        else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
        {
            CTSUCRAL |= 1 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 1) - 1);
        }
        else if ((CTSU_PCLKB_FREQ_RANGE2 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE3 >= pclkb_mhz))
        {
            CTSUCRAL |= 2 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 2) - 1);
        }
        else
        {
            CTSUCRAL |= 3 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 3) - 1);
        }
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)

        /* Set power on */
        CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

        /* Synchronous Noise Reduction Setting */
        CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

        /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
        CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

        /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
        CTSUSST = (CTSU_SST_RECOMMEND);

        if (CTSU_CORRECTION_INIT == g_ctsu_correction_info.status)
        {
            ctsu_correction_process(p_instance_ctrl);
        }

        /* Since CLK is rewritten by correction, set here. */
        CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);
#endif
    }

#if (BSP_FEATURE_CTSU_VERSION == 2)
    if (CTSU_MODE_MUTUAL_CFC_SCAN != p_cfg->md)
    {
        /* Set power on */
        CTSUCRAL |= CTSUCRA_PON_Msk;
        CTSUCRAL |= CTSUCRA_CSW_Msk;
        ctsu_delay(30);
    }

    /* High resolution pulse mode  */
    CTSUCRAH  |= CTSUCRA_SDPSEL_Msk;
    CTSUCRAH  |= CTSUCRA_PCSEL_Msk;
    CTSUSST   = CTSU_SST_RECOMMEND;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCLK_Msk;

    if (CTSU_MODE_MUTUAL_CFC_SCAN != p_cfg->md)
    {
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
        if (CTSU_MODE_CORRECTION_SCAN == p_cfg->md)
        {
            g_ctsu_correction_info.scan_index = CTSU_CORRECTION_POINT_NUM;
        }
 #endif
        if (CTSU_CORRECTION_INIT == g_ctsu_correction_info.status)
        {
            ctsu_correction_process(p_instance_ctrl);
        }
    }

    CTSUCRAH  |= 1 << CTSUCRA_LOAD_Pos;
    CTSUCRBH  &= ~CTSUCRB_SSCNT_Msk;
    CTSUCRBH  |= 1 << CTSUCRB_SSCNT_Pos;
    CTSUDBGR1 &= ~CTSUCALIB_SUCARRY_Msk;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCALIB_Msk;
    CTSUDBGR1 |= CTSUCALIB_CCOCLK_Msk;
    CTSUDBGR0 &= ~CTSUCALIB_TSOC_Msk;

    /* Read SUADJD byte */
    suadj[0] = (uint16_t) ((SUADJD * (CTSU_CFG_SUMULTI0 + 1)) / (CTSU_SUMULTI_BASE + 1));
 #if (CTSU_CFG_NUM_SUMULTI >= 2)

    /* Adjust multi freq */
    suadj[1] = (uint16_t) ((suadj[0] * (CTSU_CFG_SUMULTI1 + 1)) / (CTSU_CFG_SUMULTI0 + 1));
 #endif
 #if (CTSU_CFG_NUM_SUMULTI >= 3)
    suadj[2] = (uint16_t) ((suadj[0] * (CTSU_CFG_SUMULTI2 + 1)) / (CTSU_CFG_SUMULTI0 + 1));
 #endif

    /* Adjust SSCNT setting */
    suadj[0] = (uint16_t) (suadj[0] - (CTSU_SUADJ_SSCNT_ADJ));
 #if (CTSU_CFG_NUM_SUMULTI >= 2)
    suadj[1] = (uint16_t) (suadj[1] - (CTSU_SUADJ_SSCNT_ADJ));
 #endif
 #if (CTSU_CFG_NUM_SUMULTI >= 3)
    suadj[2] = (uint16_t) (suadj[2] - (CTSU_SUADJ_SSCNT_ADJ));
 #endif

    /* Set CTSUSUCLK register */
    CTSUCRAH  &= ~CTSUCRA_SDPSEL_Msk;
    CTSUSUCLK0 = (uint16_t) (CTSU_CFG_SUMULTI0 << 8) | suadj[0];
 #if (CTSU_CFG_NUM_SUMULTI >= 2)
    CTSUSUCLK1 = (uint16_t) (CTSU_CFG_SUMULTI1 << 8) | suadj[1];
 #endif
 #if (CTSU_CFG_NUM_SUMULTI >= 3)
    CTSUSUCLK2 = (uint16_t) (CTSU_CFG_SUMULTI2 << 8) | suadj[2];
 #endif
    CTSUCRAH  |= CTSUCRA_SDPSEL_Msk;

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
    {
        /* Fixed parameter setting for automatic judgement */
        CTSUAJCRL = (p_instance_ctrl->p_ctsu_cfg->thot << 8U) | p_instance_ctrl->p_ctsu_cfg->tlot;

        p_instance_ctrl->blini_flag = 1;
    }
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        p_instance_ctrl->p_diag_info = &g_ctsu_diag_info;
        g_ctsu_diag_info.state       = CTSU_DIAG_INIT;
        g_ctsu_diag_info.dac_init    = 0;
    }

  #if BSP_MCU_GROUP_RL78G16
    /* Correction Factor */
    correction_factor = CTSUTRESULT;

    if ((correction_factor & 0x80U) != 0U)
    {
        if (((correction_factor & 0x40U) != 0U) && (RTRIM >= 0x90U))
        {
            correction_factor = (correction_factor & 0x007FU) | 0x0100U;
        }
        else
        {
            correction_factor = (correction_factor & 0x007FU) | 0x0200U;
        }
    }
    else
    {
        if (((correction_factor & 0x40U) == 0U) && (RTRIM <= 0x50U))
        {
            correction_factor = (correction_factor & 0x007FU) | 0x0280U;
        }
        else
        {
            correction_factor = (correction_factor & 0x007FU) | 0x0180U;
        }
    }

    /* Set expected voltage value table */
    dac_wots_table[0U] = ((uint32_t)CTSU_CFG_DIAG_DAC1_TYP * correction_factor) >> 9U;
    dac_wots_table[1U] = ((uint32_t)CTSU_CFG_DIAG_DAC2_TYP * correction_factor) >> 9U;
    dac_wots_table[2U] = ((uint32_t)CTSU_CFG_DIAG_DAC3_TYP * correction_factor) >> 9U;
    dac_wots_table[3U] = ((uint32_t)CTSU_CFG_DIAG_DAC4_TYP * correction_factor) >> 9U;
    dac_wots_table[4U] = ((uint32_t)CTSU_CFG_DIAG_DAC5_TYP * correction_factor) >> 9U;
    dac_wots_table[5U] = ((uint32_t)CTSU_CFG_DIAG_DAC6_TYP * correction_factor) >> 9U;
    dac_wots_table[6U] = ((uint32_t)CTSU_CFG_DIAG_DAC7_TYP * correction_factor) >> 9U;
    dac_wots_table[7U] = ((uint32_t)CTSU_CFG_DIAG_DAC8_TYP * correction_factor) >> 9U;

  #endif
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
    p_instance_ctrl->p_diag_info = &g_ctsu_diag_info;
  #endif
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 0)
        p_instance_ctrl->p_diag_info = &g_ctsu_diag_info;
  #endif
        g_ctsu_diag_info.state             = CTSU_DIAG_INIT;
        g_ctsu_diag_info.test_result       = FSP_SUCCESS;
        g_ctsu_diag_info.test_count        = 0;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
        g_ctsu_diag_info.load_resistance   = 0;
        g_ctsu_diag_info.average_data_pre  = 0;
        g_ctsu_diag_info.onetime_exec_flag = 0;
  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_id     = 0;
        g_ctsu_diag_info.buffer_index = CTSU_DIAG_DEBUG_DATA_START_INDEX;
  #endif
    }
 #endif
#endif

    p_instance_ctrl->p_correction_info = &g_ctsu_correction_info;
    p_instance_ctrl->rd_index          = 0;
    p_instance_ctrl->wr_index          = 0;
    p_instance_ctrl->state             = CTSU_STATE_IDLE;

    p_instance_ctrl->p_callback        = p_cfg->p_callback;
    p_instance_ctrl->p_context         = p_cfg->p_context;
    p_instance_ctrl->p_callback_memory = NULL;

    p_instance_ctrl->serial_tuning_enable     = 0;
    p_instance_ctrl->serial_tuning_mutual_cnt = 0;

#if defined(CTSU_CFG_TARGET_VALUE_QE_SUPPORT)
    p_instance_ctrl->tuning_self_target_value   = p_cfg->tuning_self_target_value;
    p_instance_ctrl->tuning_mutual_target_value = p_cfg->tuning_mutual_target_value;
#else
    p_instance_ctrl->tuning_self_target_value   = CTSU_TUNING_VALUE_SELF;
    p_instance_ctrl->tuning_mutual_target_value = CTSU_TUNING_VALUE_MUTUAL;
#endif
#if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)
    p_instance_ctrl->tsod = p_instance_ctrl->p_ctsu_cfg->tsod;
    p_instance_ctrl->mec_ts = p_instance_ctrl->p_ctsu_cfg->mec_ts;
    p_instance_ctrl->mec_shield_ts = p_instance_ctrl->p_ctsu_cfg->mec_shield_ts;
#endif

    /* Mark driver as open */
    p_instance_ctrl->open = CTSU_OPEN;

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function should be called each time a periodic timer expires.
 * If initial offset tuning is enabled, The first several calls are used to tuning for the sensors.
 * Before starting the next scan, first get the data with R_CTSU_DataGet().
 * If a different control block scan should be run, check the scan is complete before executing.
 * Implements @ref ctsu_api_t::scanStart.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_ScanStart
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance or other.
 * @retval FSP_ERR_CTSU_NOT_GET_DATA    The previous data has not been retrieved by DataGet.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_ScanStart (ctsu_ctrl_t * const p_ctrl)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint8_t                temp;
#if (BSP_FEATURE_CTSU_VERSION == 2)
    uint8_t txvsel;
#endif

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->cap)
    {
#if (BSP_FEATURE_CTSU_VERSION == 2)
        /* Can be checked if the previous measurement was a software trigger */
        if (0 == (0x02 & CTSUCR0))
        {
            FSP_ERROR_RETURN(0x01 != (CTSUCR0 & 0x01), FSP_ERR_CTSU_SCANNING);
        }
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
        /* Can be checked if the previous measurement was a software trigger */
        if (0 == (0x02 & CTSUCR0))
        {
            FSP_ERROR_RETURN(0x01 != (CTSUCR0 & 0x01), FSP_ERR_CTSU_SCANNING);
        }
#endif
    }

    FSP_ERROR_RETURN(CTSU_STATE_SCANNED != p_instance_ctrl->state, FSP_ERR_CTSU_NOT_GET_DATA);

    gp_ctsu_isr_context = p_instance_ctrl;

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    ctsu_auto_correction_register_set(p_instance_ctrl);

    /* Sensor counter auto correction enabled */
    CTSUOPTL |= 0x0001U;    /* CCOCFEN = 1 */
 #endif
#endif

#if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
    if (((1 == p_instance_ctrl->p_ctsu_cfg->ajfen) && (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)) ||
        ((0 == p_instance_ctrl->p_ctsu_cfg->ajfen) && (0 == p_instance_ctrl->p_ctsu_cfg->majority_mode)))
    {
        if (0 == p_instance_ctrl->mcact_flag)
        {
            if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
            {
                ctsu_multiclock_auto_correction_register_set(p_instance_ctrl);
                p_instance_ctrl->mcact_flag = 1;
                CTSUOPTL |= 0x0002U;    /* MCACFEN = 1 */
                CTSUOPTL = (CTSUOPTL & ~0x0004U) | (p_instance_ctrl->p_ctsu_cfg->majirimd << 2);
            }
            else
            {
                CTSUOPTL &= ~0x0004U;    /* MAJIRIMD = 0 */
                CTSUOPTL &= ~0x0002U;    /* MCACFEN = 0 */
            }
        }
        else
        {
            CTSUOPTL |= 0x0002U;    /* MCACFEN = 1 */
            CTSUOPTL = (CTSUOPTL & ~0x0004U) | (p_instance_ctrl->p_ctsu_cfg->majirimd << 2);
        }
    }
    else
    {
        CTSUOPTL &= ~0x0004U;    /* MAJIRIMD = 0 */
        CTSUOPTL &= ~0x0002U;    /* MCACFEN = 0 */
    }
#endif

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
 #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
    if (1 == p_instance_ctrl->sms)
    {
        ctsu_sms_transfer_configure(p_instance_ctrl);
    }
    else
    {
        err = ctsu_transfer_configure(p_instance_ctrl);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
    }
 #else
    err = ctsu_transfer_configure(p_instance_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)

    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->cap)
    {
        /* specify Software trigger usage */
        CTSUCR0 &= (uint8_t) ~0x06;
    }
    else
    {
        /* specify external trigger usage and enable power saving */
        CTSUCR0 |= 0x06;
        CTSUCR0 &= (uint8_t) ~0x01; // To write CTSUCR1
    }

    /* Write CTSU Control Register 1 and save mode */

    if (CTSU_MODE_CORRECTION_SCAN != p_instance_ctrl->md)
    {
        txvsel =
            (uint8_t) ((p_instance_ctrl->txvsel2 << 6) | (p_instance_ctrl->txvsel << 7));
        temp            = (uint8_t) (CTSUCR0 & ~(CTSU_CR0_MODIFY_BIT));
        CTSUCR0 = (uint8_t) (temp | (txvsel & CTSU_CR0_MODIFY_BIT));                   // TXVSEL
        temp            = (uint8_t) (CTSUCR1 & ~(CTSU_CR1_MODIFY_BIT));
        CTSUCR1 = (uint8_t) (temp | (p_instance_ctrl->ctsucr1 & CTSU_CR1_MODIFY_BIT)); // MD1, MD0, ATUNE1
        temp            = (uint8_t) (CTSUCR2 & ~(CTSU_CR2_MODIFY_BIT));
        CTSUCR2 = (uint8_t) (temp | (p_instance_ctrl->ctsucr2 & CTSU_CR2_MODIFY_BIT)); // POSEL, ATUNE2, MD2
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            CTSUSST  = CTSU_SST_RECOMMEND_CURRENT;
            CTSUCRAH |= CTSUCRA_DCMODE_Msk;
            CTSUCRAH |= CTSUCRA_DCBACK_Msk;
            CTSUMCHH = 0x0001;
        }
        else
        {
            CTSUSST  = CTSU_SST_RECOMMEND;
            CTSUCRAH &= ~CTSUCRA_DCMODE_Msk;
            CTSUCRAH &= ~CTSUCRA_DCBACK_Msk;
            CTSUMCHH = 0x0001;
#if (CTSU_CFG_NUM_SUMULTI >= 2)
            CTSUMCHH |= 0x0002;
#endif
#if (CTSU_CFG_NUM_SUMULTI >= 3)
            CTSUMCHH |= 0x0004;
#endif
        }

        /* Write Channel setting */
        CTSUCHAC0  = p_instance_ctrl->ctsuchac0;
        CTSUCHAC1  = p_instance_ctrl->ctsuchac1;
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
        CTSUCHAC2  = p_instance_ctrl->ctsuchac2;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
        CTSUCHAC3  = p_instance_ctrl->ctsuchac3;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
        CTSUCHAC4  = p_instance_ctrl->ctsuchac4;
 #endif

        CTSUCHTRC0 = p_instance_ctrl->ctsuchtrc0;
        CTSUCHTRC1 = p_instance_ctrl->ctsuchtrc1;
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
        CTSUCHTRC2 = p_instance_ctrl->ctsuchtrc2;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
        CTSUCHTRC3 = p_instance_ctrl->ctsuchtrc3;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
        CTSUCHTRC4 = p_instance_ctrl->ctsuchtrc4;
 #endif
    }

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        ctsu_diag_scan_start2(p_instance_ctrl);
    }
 #endif
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
    {
        CTSUSST = CTSU_SST_RECOMMEND;
        ctsu_correction_scan_start();
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)

        /* Disable automatic correction during temperature correction */
        CTSUOPTL &= 0xFFFEU;        /* CCOCFEN = 0 */
  #endif
    }
 #endif

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
    {
        /* Enable / disable switching of automatic judgement */
        CTSUOPTL = (CTSUOPTL & 0xFEDFU) | (p_instance_ctrl->p_ctsu_cfg->mtucfen << 5U) | (p_instance_ctrl->p_ctsu_cfg->ajfen << 8U);

        if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
        {
            CTSUAJCRH = (CTSUAJCRH & 0x00CFU) | (p_instance_ctrl->ajbmat << 12U) | (p_instance_ctrl->ajmmat << 8U) | (p_instance_ctrl->p_ctsu_cfg->jc << 4U);
            /* Set 0 to TJR2, TJR1, TJR0 */
            CTSUAJRRL &= 0xFFF8;
        }
    }
    else
    {
        /* If tuning incomplete, auto judgement register set OFF */
        CTSUOPTL = (CTSUOPTL & 0xFEDFU);
    }

    if (1 == p_instance_ctrl->blini_flag)
    {
        /* Set blini to 1 at the first judgement of automatic judgement */
        CTSUAJCRH |= 0x0001U;

        /* Generates a temporary measurement completion interrupt during snooze mode */
        CTSUOPTL |= 0x0200U;
    }
 #endif

 #if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)

    /* Parameter setting for multiple electrode connection */
    CTSUDBGR0 &= ~CTSUCALIB_TSOD_Msk;
    CTSUDBGR0 |= (p_instance_ctrl->tsod << CTSUCALIB_TSOD_Pos);
    CTSUDBGR0 &= ~CTSUCALIB_IOC_Msk;
    if (1 == p_instance_ctrl->tsod)
    {
        /* When using MEC, MD0 bit is set to single scan mode. */
        CTSUCRAL &= ~CTSUCRA_MD0_Msk;
        CTSUMCH0 = (p_instance_ctrl->mec_ts & CTSUMCH_MCH0_Msk);

        /* Set MCH1 when using both MEC and Active Shield.*/
        if ((0 != p_instance_ctrl->ctsuchtrc0) ||
            (0 != p_instance_ctrl->ctsuchtrc1) ||
            (0 != p_instance_ctrl->ctsuchtrc2) ||
            (0 != p_instance_ctrl->ctsuchtrc3) ||
            (0 != p_instance_ctrl->ctsuchtrc4))
        {
            CTSUMCH1 = (p_instance_ctrl->mec_shield_ts & (CTSUMCH_MCH1_Msk >> CTSUMCH_MCH1_Pos));
        }
    }
 #endif
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)

    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->cap)
    {
        /* specify Software trigger usage */
        CTSUCR0 &= (uint8_t) ~0x06;
    }
    else
    {
        /* specify external trigger usage and enable power saving */
        CTSUCR0 |= 0x06;
        CTSUCR0 &= (uint8_t) ~0x01;
        CTSUSTRT = 0; // To write CTSUCR1
    }


    if (CTSU_MODE_DIAGNOSIS_SCAN != p_instance_ctrl->md)
    {
        temp            = (uint8_t) (CTSUCR1 & ~(CTSU_CR1_MODIFY_BIT));
        CTSUCR1 = (uint8_t) (temp | (p_instance_ctrl->ctsucr1 & CTSU_CR1_MODIFY_BIT)); // MD1, MD0, ATUNE1

 #if BSP_FEATURE_CTSU_HAS_TXVSEL
        CTSUCR0 =
            (uint8_t) ((CTSUCR0 & ~(CTSU_TXVSEL)) | (p_instance_ctrl->p_ctsu_cfg->txvsel & CTSU_TXVSEL));
 #endif

        /* Write Channel setting */
        CTSUCHAC0 = p_instance_ctrl->ctsuchac0;
        CTSUCHAC1 = p_instance_ctrl->ctsuchac1;
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
        CTSUCHAC2 = p_instance_ctrl->ctsuchac2;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
        CTSUCHAC3 = p_instance_ctrl->ctsuchac3;
 #endif
 #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
        CTSUCHAC4 = p_instance_ctrl->ctsuchac4;
 #endif

 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        CTSUCHTRC0 = p_instance_ctrl->ctsuchtrc0;
        CTSUCHTRC1 = p_instance_ctrl->ctsuchtrc1;
  #if (BSP_FEATURE_CTSU_CTSUCHTRC_REGISTER_COUNT >= 3)
        CTSUCHTRC2 = p_instance_ctrl->ctsuchtrc2;
  #endif
  #if (BSP_FEATURE_CTSU_CTSUCHTRC_REGISTER_COUNT >= 4)
        CTSUCHTRC3 = p_instance_ctrl->ctsuchtrc3;
  #endif
  #if (BSP_FEATURE_CTSU_CTSUCHTRC_REGISTER_COUNT >= 5)
        CTSUCHTRC4 = p_instance_ctrl->ctsuchtrc4;
  #endif
 #endif
    }

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        ctsu_diag_scan_start1(p_instance_ctrl);
    }
 #endif

 #if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)
    /* Parameter setting for multiple electrode connection */
    CTSUERRS &= ~(CTSUERRS_ALMES_Msk);
    CTSUCR0  &= ~(CTSUCR0_IOC_Msk);

    if (1 == p_instance_ctrl->tsod)
    {
        /* When using MEC, MD0 bit is set to single scan mode. */
        CTSUCR1 &= ~(CTSUCR1_MD0_Msk);
        CTSUMCH0 = (p_instance_ctrl->mec_ts & CTSUMCH_MCH0_Msk);

        /* Multiple electrode connection mode */
        CTSUERRS |= CTSUERRS_ALMES_Msk;
    }
 #endif
#endif

    p_instance_ctrl->state = CTSU_STATE_SCANNING;

    /* Set CTSU_STRT bit to start scan */
    CTSUCR0 |= 0x01;           ///< CTSU_STRT

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function gets the sensor values as scanned by the CTSU.
 * If initial offset tuning is enabled, The first several calls are used to tuning for the sensors.
 * Implements @ref ctsu_api_t::dataGet.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_DataGet
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval FSP_ERR_CTSU_INCOMPLETE_TUNING      Incomplete initial offset tuning.
 * @retval FSP_ERR_CTSU_DIAG_NOT_YET      Diagnosis of data collected no yet.
 * @retval FSP_ERR_INVALID_MODE           The mode of automatic judgement on is invalid.
 * @retval FSP_ERR_ABORTED                Operate error of Diagnosis ADC data collection ,since ADC use other
 **********************************************************************************************************************/
fsp_err_t R_CTSU_DataGet (ctsu_ctrl_t * const p_ctrl, uint16_t * p_data)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint16_t               element_id;
    uint16_t               majority_mode_id;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_data);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);
    FSP_ERROR_RETURN(0 == p_instance_ctrl->p_ctsu_cfg->ajfen, FSP_ERR_INVALID_MODE);

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
    {
        ctsu_correction_data_get(p_instance_ctrl, p_data);
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)

        /* Enable automatic correction after temperature correction is completed */
        CTSUOPTL |= 0x0001U;    /* CCOCFEN = 1 */
  #endif
        p_instance_ctrl->state = CTSU_STATE_IDLE;

        return err;
    }
 #endif

 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        err = ctsu_diag_data_get2(p_instance_ctrl);
        p_instance_ctrl->state = CTSU_STATE_IDLE;
        *p_data                = CTSU_COUNT_MAX;

        return err;
    }
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        err = ctsu_diag_data_get1();
        p_instance_ctrl->state = CTSU_STATE_IDLE;
        if (FSP_ERR_CTSU_DIAG_NOT_YET == err)
        {
            err = FSP_ERR_CTSU_DIAG_NOT_YET;
        }
        else
        {
            err = FSP_SUCCESS;
        }

        return err;
    }
 #endif
#endif

    if (CTSU_STATE_SCANNED == p_instance_ctrl->state)
    {
        if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
        {
            if (p_instance_ctrl->average == p_instance_ctrl->num_moving_average)
            {
                /* Do nothing */
            }
            else if (p_instance_ctrl->average < p_instance_ctrl->num_moving_average)
            {
                (p_instance_ctrl->average)++;
            }
            else
            {
                p_instance_ctrl->average = p_instance_ctrl->num_moving_average;
            }
        }

        ctsu_correction_exec(p_instance_ctrl);

        if (CTSU_TUNING_INCOMPLETE == p_instance_ctrl->tuning)
        {
            if ((CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md) ||
                (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md))
            {
                ctsu_initial_offset_tuning(p_instance_ctrl);
            }
        }

        p_instance_ctrl->state = CTSU_STATE_IDLE;
    }

    FSP_ERROR_RETURN(0 < p_instance_ctrl->average, FSP_ERR_CTSU_INCOMPLETE_TUNING);

#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (majority_mode_id = 0; majority_mode_id < CTSU_MAJORITY_MODE_ELEMENTS; majority_mode_id++)
            {
                *p_data =
                    (p_instance_ctrl->p_self_data + element_id * CTSU_MAJORITY_MODE_ELEMENTS +
                     majority_mode_id)->int_data;
                p_data++;
            }
        }
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        if (true == p_instance_ctrl->serial_tuning_enable)
        {
            /* Serial Tuning Phase1 */
            if ((0 == p_instance_ctrl->ctsuchtrc0) && (0 == p_instance_ctrl->ctsuchtrc1) &&
                (0 == p_instance_ctrl->ctsuchtrc2) && (0 == p_instance_ctrl->ctsuchtrc3) &&
                (0 == p_instance_ctrl->ctsuchtrc4))
            {
                return FSP_SUCCESS;
            }
        }

        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (majority_mode_id = 0; majority_mode_id < CTSU_MAJORITY_MODE_ELEMENTS; majority_mode_id++)
            {
                *p_data =
                    (p_instance_ctrl->p_mutual_pri_data + element_id * CTSU_MAJORITY_MODE_ELEMENTS +
                     majority_mode_id)->int_data;
                p_data++;
                *p_data =
                    (p_instance_ctrl->p_mutual_snd_data + element_id * CTSU_MAJORITY_MODE_ELEMENTS +
                     majority_mode_id)->int_data;
                p_data++;
            }
        }
    }
#endif

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function gets the result of automatic judgement button. Call after the scan is completed.
 * The result is 64-bit bitmaps and is stored in order of TS number for specified ctsu control.
 * After the initial judgement, the baseline initialization bit is set and the automatic judgement threshold is set.
 * This function is called only for automatic judgement.
 * Implements @ref ctsu_api_t::autoJudgementDataGet.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_AutoJudgementDataGet
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval FSP_ERR_INVALID_MODE     The mode of automatic judgement off is invalid.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_AutoJudgementDataGet (ctsu_ctrl_t * const p_ctrl, uint64_t * p_button_status)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    uint16_t element_id;
 #endif
#endif

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_button_status);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);
    FSP_ERROR_RETURN(1 == p_instance_ctrl->p_ctsu_cfg->ajfen, FSP_ERR_INVALID_MODE);

#if  ((BSP_FEATURE_CTSU_VERSION == 2) && (CTSU_CFG_AUTO_JUDGE_ENABLE == 1))
    if (1 == p_instance_ctrl->blini_flag)
    {
        p_instance_ctrl->blini_flag = 0;

        /* After the initial judgement of automatic judgement, set the baseline initialization bit to 0. */
        CTSUAJCRH &= 0xFFFEU;

        ctsu_auto_judge_threshold_calc(p_instance_ctrl);

        /* Turn off measurement completion interrupt in snooze mode */
        CTSUOPTL &= 0xFDFFU;
    }

    if (CTSU_STATE_SCANNED == p_instance_ctrl->state)
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
            if (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)
            {
                /* Reflect the final result bit of automatic judgement in buttun_status */
                if (1 == (((p_instance_ctrl->p_ajrr[element_id]) >> CTSU_AUTO_FINAL_JUDGE_BIT) & 0x1))
                {
                    *p_button_status |= ((uint64_t) 1 << element_id);
                }
                else
                {
                    *p_button_status &= ~((uint64_t) 1 << element_id);
                }
            }
            else
 #endif
            {
                /* Reflect the final result bit of automatic judgement in buttun_status */
                /* Since the final result of the automatic judgement is output to the FJR bit of the final multi-clock, */
                /* the result of the final multi-clock is taken out.                                                   */
                if (1 ==
                    (((p_instance_ctrl->p_ajrr[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                                               (CTSU_MAJORITY_MODE_ELEMENTS - 1)]) >> CTSU_AUTO_FINAL_JUDGE_BIT) & 0x1))
                {
                    *p_button_status |= ((uint64_t) 1 << element_id);
                }
                else
                {
                    *p_button_status &= ~((uint64_t) 1 << element_id);
                }
            }
        }

        p_instance_ctrl->state = CTSU_STATE_IDLE;
    }

#else
    FSP_PARAMETER_NOT_USED(p_button_status);
#endif

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function tunes the offset register(SO). Call after the measurement is completed.
 * If the return value is FSP_ERR_CTSU_INCOMPLETE_TUNING, tuning is not complete.
 * Execute the measurement and this function call routine until the return value becomes FSP_SUCCESS.
 * It is recommended to run this routine after R_CTSU_Open().
 * It can be recalled and tuned again.
 * When the automatic judgement is enabled, after the offset tuning is completed,the baseline initialization bit flag is set.
 * Implements @ref ctsu_api_t::offsetTuning.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_OffsetTuning
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval FSP_ERR_CTSU_INCOMPLETE_TUNING      Incomplete initial offset tuning.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_OffsetTuning (ctsu_ctrl_t * const p_ctrl)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint16_t               element_id;
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    uint16_t i;
 #endif
#endif

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);

    if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            /* Counter clear for re-offset tuning */
            *(p_instance_ctrl->p_element_complete_flag + element_id) = 0;
#if (BSP_FEATURE_CTSU_VERSION == 2)
            *(p_instance_ctrl->p_frequency_complete_flag + element_id) = 0;
#endif
            *(p_instance_ctrl->p_tuning_diff + element_id) = 0;
        }
    }

    p_instance_ctrl->tuning = CTSU_TUNING_INCOMPLETE;

    if (CTSU_STATE_SCANNED == p_instance_ctrl->state)
    {
        if (0 == p_instance_ctrl->p_ctsu_cfg->ajfen)
        {
            ctsu_correction_exec(p_instance_ctrl);
        }

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
        else
        {
            CTSUOPTL &= 0xFEDFU;    /* AJFEN = 0, MTUCFEN = 0 */

            for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
            {
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
                    {
                        p_instance_ctrl->p_self_corr[(element_id * CTSU_CFG_NUM_SUMULTI) +
                                                     i] =
                            p_instance_ctrl->p_self_raw[(element_id * CTSU_CFG_NUM_SUMULTI) + i];
                    }

  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
                    else
                    {
                        p_instance_ctrl->p_mutual_pri_corr[(element_id * CTSU_CFG_NUM_SUMULTI) +
                                                           i] =
                            p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2)];
                    }
  #endif
  #if (CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE)

                    /* Initial value setting of DTC transfer information for automatic judgement */
                    p_instance_ctrl->p_ajthr[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i]   = 0;
                    p_instance_ctrl->p_ajmmar[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i]  = 0;
                    p_instance_ctrl->p_ajblact[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i] = 0;
                    p_instance_ctrl->p_ajblar[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i]  = 0;
                    p_instance_ctrl->p_ajrr[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i]    = 0;
  #endif
                }

  #if ((CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE) == 0)
                p_instance_ctrl->p_ajthr[element_id]   = 0;
                p_instance_ctrl->p_ajmmar[element_id]  = 0;
                p_instance_ctrl->p_ajblact[element_id] = 0;
                p_instance_ctrl->p_ajblar[element_id]  = 0;
                p_instance_ctrl->p_ajrr[element_id]    = 0;
  #endif
            }
        }
 #endif
#endif

        if (CTSU_TUNING_INCOMPLETE == p_instance_ctrl->tuning)
        {
            if ((CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md) ||
                (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md))
            {
                ctsu_initial_offset_tuning(p_instance_ctrl);
            }
        }

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
        if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
        {
            if (CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning)
            {
                /* Initialization of baseline mean for automatic judgement */
                p_instance_ctrl->blini_flag = 1;
            }
        }
 #endif
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
        p_instance_ctrl->mcact_flag = 0;
 #endif
#endif
        p_instance_ctrl->state = CTSU_STATE_IDLE;
    }

    FSP_ERROR_RETURN(CTSU_TUNING_COMPLETE == p_instance_ctrl->tuning, FSP_ERR_CTSU_INCOMPLETE_TUNING);

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function scan stops the sensor as scanning by the CTSU.
 * Implements @ref ctsu_api_t::scanStop.
 * @retval FSP_SUCCESS              CTSU successfully scan stop.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_ScanStop (ctsu_ctrl_t * const p_ctrl)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
#if (CTSU_CFG_PARAM_CHECKING_ENABLE)
    FSP_ASSERT(p_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    if (CTSU_STATE_SCANNING == p_instance_ctrl->state)
    {
        CTSUCR0 = (uint8_t) ((CTSUCR0 & 0xFE) | 0x10);  ///< STRT=0, INIT=1

        CTSUWRIF = 0;
        CTSURDIF = 0;
        CTSUFNIF = 0;
        p_instance_ctrl->state    = CTSU_STATE_IDLE;
        p_instance_ctrl->wr_index = 0;
        p_instance_ctrl->rd_index = 0;
    }

    return err;
}

/*******************************************************************************************************************//**
 * Updates the user callback and has option of providing memory for callback structure.
 * Implements ctsu_api_t::callbackSet
 *
 * @retval  FSP_SUCCESS                  Callback updated successfully.
 * @retval  FSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  FSP_ERR_NOT_OPEN             The control block has not been opened.
 * @retval  FSP_ERR_NO_CALLBACK_MEMORY   p_callback is non-secure and p_callback_memory is either secure or NULL.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_CallbackSet (ctsu_ctrl_t * const          p_api_ctrl,
                              void (                     * p_callback)(ctsu_callback_args_t *),
                              void const * const           p_context,
                              ctsu_callback_args_t * const p_callback_memory)
{
    ctsu_instance_ctrl_t * p_ctrl = (ctsu_instance_ctrl_t *) p_api_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE)
    FSP_ASSERT(p_ctrl);
    FSP_ASSERT(p_callback);
    FSP_ERROR_RETURN(CTSU_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

#if BSP_TZ_SECURE_BUILD

    /* Get security state of p_callback */
    bool callback_is_secure =
        (NULL == cmse_check_address_range((void *) p_callback, sizeof(void *), CMSE_AU_NONSECURE));

 #if CTSU_CFG_PARAM_CHECKING_ENABLE

    /* In secure projects, p_callback_memory must be provided in non-secure space if p_callback is non-secure */
    ctsu_callback_args_t * const p_callback_memory_checked = cmse_check_pointed_object(p_callback_memory,
                                                                                       CMSE_AU_NONSECURE);
    FSP_ERROR_RETURN(callback_is_secure || (NULL != p_callback_memory_checked), FSP_ERR_NO_CALLBACK_MEMORY);
 #endif
#endif

    /* Store callback and context */
#if BSP_TZ_SECURE_BUILD
    p_ctrl->p_callback = callback_is_secure ? p_callback :
                         (void (*)(ctsu_callback_args_t *))cmse_nsfptr_create(p_callback);
#else
    p_ctrl->p_callback = p_callback;
#endif
    p_ctrl->p_context         = p_context;
    p_ctrl->p_callback_memory = p_callback_memory;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Setup measurement by SMS. Implements @ref ctsu_api_t::smsSet.
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_SmsSet (ctsu_ctrl_t * const p_ctrl,
                         uint16_t          * p_threshold,
                         uint16_t          * p_hysteresis,
                         uint16_t            count_filter)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE)
    FSP_ASSERT(p_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#else
    FSP_PARAMETER_NOT_USED(p_instance_ctrl);
#endif

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
    ctsu_sms_init(p_instance_ctrl, p_threshold, p_hysteresis, count_filter);
    ctsu_sms_elc_init();
    ctsu_sms_start();
#else
    FSP_PARAMETER_NOT_USED(p_threshold);
    FSP_PARAMETER_NOT_USED(p_hysteresis);
    FSP_PARAMETER_NOT_USED(count_filter);
    err = FSP_ERR_NOT_ENABLED;
#endif
    return err;
}

/*******************************************************************************************************************//**
 * @brief Disables specified CTSU control block. Implements @ref ctsu_api_t::close.
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_Close (ctsu_ctrl_t * const p_ctrl)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

    FSP_ASSERT(p_instance_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);

    g_ctsu_element_index = (uint8_t) (g_ctsu_element_index - p_instance_ctrl->num_elements);
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        g_ctsu_self_element_index = (uint8_t) (g_ctsu_self_element_index - p_instance_ctrl->num_elements);
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        g_ctsu_mutual_element_index = (uint8_t) (g_ctsu_mutual_element_index - p_instance_ctrl->num_elements);
    }
#endif
    if (0 == g_ctsu_element_index)
    {
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_close(p_instance_ctrl);
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
        CTSUCRAL &= 0xFFFEU;
#else
        CTSUCR0 &= 0xFFFEU;
#endif
    }

    p_instance_ctrl->state = CTSU_STATE_INIT;
    p_instance_ctrl->open  = false;

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function gets the sensor specific data values as scanned by the CTSU.
 * Call this function after calling the R_CTSU_DataGet() function.
 *
 * By setting the third argument to CTSU_SPECIFIC_RAW_DATA,
 * RAW data can be output from the second argument.
 *
 * By setting the third argument to CTSU_SPECIFIC_CCO_CORRECTION_DATA,
 * the cco corrected data can be output from the second argument.
 *
 * By setting the third argument to CTSU_SPECIFIC_CORRECTION_DATA,
 * the frequency corrected data can be output from the second argument.
 *
 * By setting the third argument to CTSU_SPECIFIC_SELECTED_FREQ,
 * Get bitmap of the frequency values used in majority decision from the second argument.(CTSU2 Only)
 * The bitmap is shown as follows.
 *
 * | 2bit                | 1bit                | 0bit                |
 * |---------------------|---------------------|---------------------|
 * | 3rd frequency value | 2nd frequency value | 1st frequency value |
 *
 * Implements @ref ctsu_api_t::specificDataGet.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_SpecificDataGet
 *
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval FSP_ERR_CTSU_INCOMPLETE_TUNING      Incomplete initial offset tuning.
 * @retval FSP_ERR_NOT_ENABLED      CTSU_SPECIFIC_SELECTED_FREQ is not enabled in CTSU1.(CTSU2 Only)
 **********************************************************************************************************************/
fsp_err_t R_CTSU_SpecificDataGet (ctsu_ctrl_t * const       p_ctrl,
                                  uint16_t                * p_specific_data,
                                  ctsu_specific_data_type_t specific_data_type)
{
    fsp_err_t              err = FSP_SUCCESS;
    uint16_t               element_id;
    uint16_t               i;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_specific_data);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);
    FSP_ERROR_RETURN(CTSU_TUNING_INCOMPLETE != p_instance_ctrl->tuning, FSP_ERR_CTSU_INCOMPLETE_TUNING);

#if (BSP_FEATURE_CTSU_VERSION == 1)
    FSP_ERROR_RETURN(CTSU_SPECIFIC_SELECTED_FREQ != specific_data_type, FSP_ERR_NOT_ENABLED);
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE)
    FSP_ERROR_RETURN(CTSU_SPECIFIC_SELECTED_FREQ != specific_data_type, FSP_ERR_NOT_ENABLED);
    FSP_ERROR_RETURN(CTSU_SPECIFIC_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
 #endif
 #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    FSP_ERROR_RETURN(CTSU_SPECIFIC_RAW_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
 #endif

 #if (CTSU_CFG_MAJORITY_MODE & (CTSU_JUDGEMENT_MAJORITY_MODE | CTSU_VALUE_MAJORITY_MODE))
    if (p_instance_ctrl->p_ctsu_cfg->majority_mode == 1)
    {
        FSP_ERROR_RETURN(CTSU_SPECIFIC_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
    }
 #endif

 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
    if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
    {
        if (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)
        {
            FSP_ERROR_RETURN(CTSU_SPECIFIC_CCO_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
            FSP_ERROR_RETURN(CTSU_SPECIFIC_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
            FSP_ERROR_RETURN(CTSU_SPECIFIC_SELECTED_FREQ != specific_data_type, FSP_ERR_NOT_ENABLED);
        }
    }
    else
    {
        if (0 == p_instance_ctrl->p_ctsu_cfg->majority_mode)
        {
            FSP_ERROR_RETURN(CTSU_SPECIFIC_CCO_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
        }
    }
 #endif

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
    {
        if (0 == p_instance_ctrl->p_ctsu_cfg->majirimd)
        {
            FSP_ERROR_RETURN(CTSU_SPECIFIC_CORRECTION_DATA != specific_data_type, FSP_ERR_NOT_ENABLED);
            FSP_ERROR_RETURN(CTSU_SPECIFIC_SELECTED_FREQ != specific_data_type, FSP_ERR_NOT_ENABLED);
        }
    }
 #endif
#endif

    if (CTSU_SPECIFIC_RAW_DATA == specific_data_type)
    {
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
            {
 #if (BSP_FEATURE_CTSU_VERSION == 1)
                *p_specific_data = (p_instance_ctrl->p_self_raw + element_id)->sen;
                p_specific_data++;
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 2)
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    *p_specific_data = *(p_instance_ctrl->p_self_raw + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                    p_specific_data++;
                }
 #endif
            }
        }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
            {
 #if (BSP_FEATURE_CTSU_VERSION == 1)
                *p_specific_data = (p_instance_ctrl->p_mutual_raw + element_id)->pri_sen;
                p_specific_data++;
                *p_specific_data = (p_instance_ctrl->p_mutual_raw + element_id)->snd_sen;
                p_specific_data++;
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 2)
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    *p_specific_data =
                        *(p_instance_ctrl->p_mutual_raw + (element_id * CTSU_CFG_NUM_SUMULTI * 2) + (i * 2));
                    p_specific_data++;
                    *p_specific_data =
                        *(p_instance_ctrl->p_mutual_raw + (element_id * CTSU_CFG_NUM_SUMULTI * 2) + (i * 2) + 1);
                    p_specific_data++;
                }
 #endif
            }
        }
#endif
    }
    else if (CTSU_SPECIFIC_CCO_CORRECTION_DATA == specific_data_type)
    {
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
 #if (BSP_FEATURE_CTSU_VERSION == 2)
  #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
                    if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
                    {
                        *p_specific_data =
                            *(p_instance_ctrl->p_self_raw + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                        p_specific_data++;
                    }
                    else
  #endif
 #endif
                    {
                        *p_specific_data =
                            *(p_instance_ctrl->p_self_corr + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                        p_specific_data++;
                    }
                }
            }
        }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
 #if (BSP_FEATURE_CTSU_VERSION == 2)
  #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
                    if (1 == p_instance_ctrl->p_ctsu_cfg->ajfen)
                    {
                        *p_specific_data =
                            *(p_instance_ctrl->p_mutual_raw +
                              ((element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2)));
                        p_specific_data++;
                        *p_specific_data =
                            *(p_instance_ctrl->p_mutual_raw +
                              ((element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2) + 1));
                        p_specific_data++;
                    }
                    else
  #endif
 #endif
                    {
                        *p_specific_data =
                            *(p_instance_ctrl->p_mutual_pri_corr + (element_id * CTSU_CFG_NUM_SUMULTI) +
                              i);
                        p_specific_data++;
                        *p_specific_data =
                            *(p_instance_ctrl->p_mutual_snd_corr + (element_id * CTSU_CFG_NUM_SUMULTI) +
                              i);
                        p_specific_data++;
                    }
                }
            }
        }
#endif
    }

#if (BSP_FEATURE_CTSU_VERSION == 2)
    else if (CTSU_SPECIFIC_CORRECTION_DATA == specific_data_type)
    {
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    *p_specific_data = *(p_instance_ctrl->p_self_mfc + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                    p_specific_data++;
                }
            }
        }
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    *p_specific_data = *(p_instance_ctrl->p_mutual_pri_mfc + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                    p_specific_data++;
                    *p_specific_data = *(p_instance_ctrl->p_mutual_snd_mfc + (element_id * CTSU_CFG_NUM_SUMULTI) + i);
                    p_specific_data++;
                }
            }
        }
 #endif
    }
    else if (CTSU_SPECIFIC_SELECTED_FREQ == specific_data_type)
    {
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                *p_specific_data = *(p_instance_ctrl->p_selected_freq_self + element_id);
                p_specific_data++;
            }
        }
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements); element_id++)
            {
                *p_specific_data = *(p_instance_ctrl->p_selected_freq_mutual + element_id);
                p_specific_data++;
            }
        }
 #endif
    }
#endif
    else
    {
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function inserts the value of the second argument as the measurement result value.
 * Call this function after calling the R_CTSU_DataInsert() function.
 * Implements @ref ctsu_api_t::dataInsert.
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_DataInsert
 *
 * @retval FSP_SUCCESS              CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 * @retval FSP_ERR_CTSU_SCANNING    Scanning this instance.
 * @retval FSP_ERR_CTSU_INCOMPLETE_TUNING      Incomplete initial offset tuning.
 **********************************************************************************************************************/
fsp_err_t R_CTSU_DataInsert (ctsu_ctrl_t * const p_ctrl, uint16_t * p_insert_data)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint16_t               element_id;
    uint16_t               majority_mode_elem_id;
#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_insert_data);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);
    FSP_ERROR_RETURN(CTSU_TUNING_INCOMPLETE != p_instance_ctrl->tuning, FSP_ERR_CTSU_INCOMPLETE_TUNING);

#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        /* Data output */
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (majority_mode_elem_id = 0;
                 majority_mode_elem_id < CTSU_MAJORITY_MODE_ELEMENTS;
                 majority_mode_elem_id++)
            {
                (p_instance_ctrl->p_self_data + (element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                 majority_mode_elem_id)->int_data = *p_insert_data;
                p_insert_data++;
            }
        }
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (majority_mode_elem_id = 0;
                 majority_mode_elem_id < CTSU_MAJORITY_MODE_ELEMENTS;
                 majority_mode_elem_id++)
            {
                (p_instance_ctrl->p_mutual_pri_data + (element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                 majority_mode_elem_id)->int_data = *p_insert_data;
                p_insert_data++;
                (p_instance_ctrl->p_mutual_snd_data + (element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                 majority_mode_elem_id)->int_data = *p_insert_data;
                p_insert_data++;
            }
        }
    }
#endif

    return err;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup CTSU)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Internal ctsu private function.
 **********************************************************************************************************************/

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_transfer_open
 ***********************************************************************************************************************/
fsp_err_t ctsu_transfer_open (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if (CTSU_CFG_DTC_USE_SC == 0)
    /* Enable input clock supply */
    DTCEN = 1U;
    /* Disable all DTC channels operation */
    DTCEN0 = 0x00U;
    DTCEN1 = 0x00U;
    DTCEN2 = 0x00U;
    DTCEN3 = 0x00U;
    DTCEN4 = 0x00U;
    /* Set base address */
 #if BSP_MCU_GROUP_RL78G22
    DTCBAR = 0xFBU;
 #else
    DTCBAR = 0xFDU;
 #endif
 #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
  #if BSP_MCU_GROUP_RL78G22
    /* Set DTCD20 */
    dtc_vectortable[38] = 0xE0U;
  #elif BSP_MCU_GROUP_RL78L23
    /* Set DTCD21 */
    dtc_vectortable[48] = 0xE8U;
  #else
    /* Set DTCD21 */
    dtc_vectortable[38] = 0xE8U;
  #endif
 #endif
 #if BSP_MCU_GROUP_RL78G22
    /* Set DTCD21 */
    dtc_vectortable[33] = 0xE8U;
    /* Set DTCD22 */
    dtc_vectortable[34] = 0xF0U;
 #elif (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    /* Set DTCD22 */
    dtc_vectortable[16] = 0xF0U;
    /* Set DTCD23 */
    dtc_vectortable[17] = 0xF8U;
 #elif BSP_MCU_GROUP_RL78L23
    /* Set DTCD22 */
    dtc_vectortable[43] = 0xF0U;
    /* Set DTCD23 */
    dtc_vectortable[44] = 0xF8U;
 #else
    /* Set DTCD22 */
    dtc_vectortable[33] = 0xF0U;
    /* Set DTCD23 */
    dtc_vectortable[34] = 0xF8U;
 #endif
#endif

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * ctsu_transfer_close
 ***********************************************************************************************************************/
fsp_err_t ctsu_transfer_close (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * ctsu_transfer_configure
 ***********************************************************************************************************************/
fsp_err_t ctsu_transfer_configure (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* After the initial offset tuning is completed, set the DTC for automatic judgement.                              */
    /* In addition, the method for which automatic judgement is disabled does not set the DTC for automatic judgement. */
    /* Alternatively, DTC for automatic judgment will not be set during correction measurement.                        */
    if ((CTSU_TUNING_INCOMPLETE == p_instance_ctrl->tuning) || (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status) || (0 == p_instance_ctrl->p_ctsu_cfg->ajfen))
    {
        ctsu_transfer_normal(p_instance_ctrl);
    }
    
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    else
    {
        /* DTC setting for automatic judgement */
        ctsu_transfer_autojudge(p_instance_ctrl);
    }
 #endif

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * ctsu_transfer_normal
 ***********************************************************************************************************************/
void ctsu_transfer_normal (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ctsu_transfer_normal_ctsuwr(p_instance_ctrl);

    ctsu_transfer_normal_ctsurd(p_instance_ctrl);
}

/***********************************************************************************************************************
 * ctsu_transfer_normal_ctsuwr
 ***********************************************************************************************************************/
void ctsu_transfer_normal_ctsuwr (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    DTCEN2 &= 0x7F; //CTSUWR
#elif BSP_MCU_GROUP_RL78L23
    DTCEN5 &= 0xDF; //CTSUWR
#else
    DTCEN4 &= 0xBF; //CTSUWR
#endif

#if BSP_MCU_GROUP_RL78G22
    /* CTSUWR setting */
    /* TRANSFER_MODE_NORMAL | DATA_SIZE_16BITS | SOURCE_ADDR_INCREMENTED | DEST_ADDR_FIXED | CHAIN_TRANSFER_DISABLE */
    dtc_controldata_21.dtccr = 0x44;
    /* 4byte */
    dtc_controldata_21.dtbls = 2;
    dtc_controldata_21.dtrld = 0x00U;
    dtc_controldata_21.dtdar = (uint16_t) &CTSUSO0;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        dtc_controldata_21.dtcct = 1;
        dtc_controldata_21.dtsar = (uint16_t)(&(g_ctsu_correction_info.ctsuwr));
    }
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        dtc_controldata_21.dtcct  = 1;
        dtc_controldata_21.dtsar = (uint16_t)p_instance_ctrl->p_ctsuwr;
    }
 #endif
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_21.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_21.dtcct = p_instance_ctrl->num_elements;
        }
        else
        {
            dtc_controldata_21.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        }

        dtc_controldata_21.dtsar = (uint16_t)p_instance_ctrl->p_ctsuwr;
    }
#else
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    dtc_vectortable[16] = 0xF0U;
 #endif

    /* CTSUWR setting */
    /* TRANSFER_MODE_NORMAL | DATA_SIZE_16BITS | SOURCE_ADDR_INCREMENTED | DEST_ADDR_FIXED | CHAIN_TRANSFER_DISABLE */
    dtc_controldata_22.dtccr = 0x44;

    /* 4byte */
    dtc_controldata_22.dtbls = 2;
    dtc_controldata_22.dtrld = 0x00U;
    dtc_controldata_22.dtdar = (uint16_t) &CTSUSO0;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        dtc_controldata_22.dtcct = 1;
        dtc_controldata_22.dtsar = (uint16_t)(&(g_ctsu_correction_info.ctsuwr));
    }
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        dtc_controldata_22.dtcct  = 1;
        dtc_controldata_22.dtsar = (uint16_t)p_instance_ctrl->p_ctsuwr;
    }
 #endif
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else
        {
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE != 0)
            if ((0 == p_instance_ctrl->p_ctsu_cfg->majority_mode) && (CTSU_TUNING_INCOMPLETE != p_instance_ctrl->tuning))
            {
                dtc_vectortable[16] = 0xE8U;
                dtc_controldata_21.dtccr = 0x54;
                dtc_controldata_21.dtbls = 2;
                dtc_controldata_21.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
                dtc_controldata_21.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
                dtc_controldata_21.dtsar = (uint16_t) p_instance_ctrl->p_mcact1;
                dtc_controldata_21.dtdar = (uint16_t) &CTSUMACT1L;

                dtc_controldata_22.dtccr = 0x44;
                dtc_controldata_22.dtbls = 2;
                dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
                dtc_controldata_22.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
                dtc_controldata_22.dtsar = (uint16_t) p_instance_ctrl->p_ctsuwr;
                dtc_controldata_22.dtdar = (uint16_t) &CTSUSO0;
            }
            else
 #endif
            {
                dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
            }
        }

        dtc_controldata_22.dtsar = (uint16_t)p_instance_ctrl->p_ctsuwr;
    }
#endif

#if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    DTCEN2 |= 0x80; //CTSUWR
#elif BSP_MCU_GROUP_RL78L23
    DTCEN5 |= 0x10; //CTSUWR
#else
    DTCEN4 |= 0x40; //CTSUWR
#endif
}

/***********************************************************************************************************************
 * ctsu_transfer_normal_ctsurd
 ***********************************************************************************************************************/
void ctsu_transfer_normal_ctsurd (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    DTCEN2 &= 0xBF; //CTSURD
#elif BSP_MCU_GROUP_RL78L23
    DTCEN5 &= 0xF7; //CTSURD
#else
    DTCEN4 &= 0xDF; //CTSURD
#endif

#if BSP_MCU_GROUP_RL78G22
    /* CTSURD setting */
    /* TRANSFER_MODE_NORMAL | DATA_SIZE_16BITS | SOURCE_ADDR_FIXED | DEST_ADDR_INCREMENTED | CHAIN_TRANSFER_DISABLE */
    dtc_controldata_22.dtccr = 0x48U;
    /* 2byte */
    dtc_controldata_22.dtbls = 1;
    dtc_controldata_22.dtrld = 0x00U;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        dtc_controldata_22.dtcct = 1;
        dtc_controldata_22.dtdar = (uint16_t) &g_ctsu_correction_info.scanbuf;
        dtc_controldata_22.dtsar = (uint16_t) &CTSUSC;
    }
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        if((CTSU_DIAG_SUCLK          == g_ctsu_diag_info.state) ||
           (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state))
        {
            dtc_controldata_22.dtsar = (uint16_t) &CTSUUC;
        }
        else
        {
            dtc_controldata_22.dtsar = (uint16_t) &CTSUSC;
        }

        dtc_controldata_22.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
        dtc_controldata_22.dtcct = 1;
    }
 #endif
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else
        {
            dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        }

        dtc_controldata_22.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
        dtc_controldata_22.dtsar = (uint16_t) &CTSUSC;
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            dtc_controldata_22.dtdar = (uint16_t) p_instance_ctrl->p_mutual_raw;
            dtc_controldata_22.dtcct = (uint16_t) (dtc_controldata_22.dtcct * 2); ///< Primary and Secondary
        }
 #endif
    }
#else
 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)
    dtc_vectortable[17] = 0xF8U;
 #endif

    /* CTSURD setting */
    /* TRANSFER_MODE_NORMAL | DATA_SIZE_16BITS | SOURCE_ADDR_FIXED | DEST_ADDR_INCREMENTED | CHAIN_TRANSFER_DISABLE */
    dtc_controldata_23.dtccr = 0x48U;
    /* 2byte */
    dtc_controldata_23.dtbls = 1;
    dtc_controldata_23.dtrld = 0x00U;
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        dtc_controldata_23.dtcct = 1;
        dtc_controldata_23.dtdar = (uint16_t) &g_ctsu_correction_info.scanbuf;
        dtc_controldata_23.dtsar = (uint16_t) &CTSUSC;
    }
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        if((CTSU_DIAG_SUCLK          == g_ctsu_diag_info.state) ||
           (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state))
        {
            dtc_controldata_23.dtsar = (uint16_t) &CTSUUC;
        }
        else
        {
            dtc_controldata_23.dtsar = (uint16_t) &CTSUSC;
        }

        dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
        dtc_controldata_23.dtcct = 1;
    }
 #endif
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_23.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
        {
            dtc_controldata_23.dtcct = p_instance_ctrl->num_elements;
        }
        else
        {
            dtc_controldata_23.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        }

        dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
        dtc_controldata_23.dtsar = (uint16_t) &CTSUSC;
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_mutual_raw;
            dtc_controldata_23.dtcct = (uint16_t) (dtc_controldata_23.dtcct * 2); ///< Primary and Secondary
        }
 #endif
    }
#endif

#if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    DTCEN2 |= 0x40; //CTSURD
#elif BSP_MCU_GROUP_RL78L23
    DTCEN5 |= 0x08; //CTSURD
#else
    DTCEN4 |= 0x20; //CTSURD
#endif
}

#if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_transfer_autojudge
 ***********************************************************************************************************************/
void ctsu_transfer_autojudge (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ctsu_transfer_autojudge_ctsuwr(p_instance_ctrl);

    ctsu_transfer_autojudge_ctsurd(p_instance_ctrl);
}

/***********************************************************************************************************************
 * ctsu_transfer_autojudge_ctsuwr
 ***********************************************************************************************************************/
void ctsu_transfer_autojudge_ctsuwr (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    DTCEN2 &= 0x7F; //CTSUWR

#if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
    if (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)
    {
        /* Set DTCD11 */
        dtc_vectortable[16U] = 0x98U;
        dtc_controldata_11.dtccr = 0x53;
        dtc_controldata_11.dtbls = 2;
        dtc_controldata_11.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_11.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_11.dtsar = (uint16_t) p_instance_ctrl->p_ajthr;
        dtc_controldata_11.dtdar = (uint16_t) &CTSUAJTHRL;

        /* Set DTCD12 */
        dtc_controldata_12.dtccr = 0x53;
        dtc_controldata_12.dtbls = 2;
        dtc_controldata_12.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_12.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_12.dtsar = (uint16_t) p_instance_ctrl->p_ajmmar;
        dtc_controldata_12.dtdar = (uint16_t) &CTSUAJMMARL;

        /* Set DTCD13 */
        dtc_controldata_13.dtccr = 0x53;
        dtc_controldata_13.dtbls = 2;
        dtc_controldata_13.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_13.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_13.dtsar = (uint16_t) p_instance_ctrl->p_ajblact;
        dtc_controldata_13.dtdar = (uint16_t) &CTSUAJBLACL;

        /* Set DTCD14 */
        dtc_controldata_14.dtccr = 0x53;
        dtc_controldata_14.dtbls = 2;
        dtc_controldata_14.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_14.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_14.dtsar = (uint16_t) p_instance_ctrl->p_ajblar;
        dtc_controldata_14.dtdar = (uint16_t) &CTSUAJBLARL;

        /* Set DTCD15 */
        dtc_controldata_15.dtccr = 0x53;
        dtc_controldata_15.dtbls = 2;
        dtc_controldata_15.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_15.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_15.dtsar = (uint16_t) p_instance_ctrl->p_ajrr;
        dtc_controldata_15.dtdar = (uint16_t) &CTSUAJRRL;

        /* Set DTCD16 */
        dtc_controldata_16.dtccr = 0x53;
        dtc_controldata_16.dtbls = 2;
        dtc_controldata_16.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_16.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_16.dtsar = (uint16_t) p_instance_ctrl->p_mcact1;
        dtc_controldata_16.dtdar = (uint16_t) &CTSUMACT1L;

        /* Set DTCD17 */
        dtc_controldata_17.dtccr = 0x53;
        dtc_controldata_17.dtbls = 2;
        dtc_controldata_17.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_17.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_17.dtsar = (uint16_t) p_instance_ctrl->p_mcact2;
        dtc_controldata_17.dtdar = (uint16_t) &CTSUMACT2L;

        /* Set DTCD18 */
        dtc_controldata_18.dtccr = 0x43;
        dtc_controldata_18.dtbls = 2;
        dtc_controldata_18.dtcct = (uint16_t) (p_instance_ctrl->num_elements);
        dtc_controldata_18.dtrld = (uint16_t) (p_instance_ctrl->num_elements);
        dtc_controldata_18.dtsar = (uint16_t) p_instance_ctrl->p_ctsuwr;
        dtc_controldata_18.dtdar = (uint16_t) &CTSUSO0;
    }
    else
#endif
    {
        /* Set DTCD13 */
        dtc_vectortable[16U] = 0xA8U;
        dtc_controldata_13.dtccr = 0x53;
        dtc_controldata_13.dtbls = 2;
        dtc_controldata_13.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_13.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_13.dtsar = (uint16_t) p_instance_ctrl->p_ajthr;
        dtc_controldata_13.dtdar = (uint16_t) &CTSUAJTHRL;

        /* Set DTCD14 */
        dtc_controldata_14.dtccr = 0x53;
        dtc_controldata_14.dtbls = 2;
        dtc_controldata_14.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_14.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_14.dtsar = (uint16_t) p_instance_ctrl->p_ajmmar;
        dtc_controldata_14.dtdar = (uint16_t) &CTSUAJMMARL;

        /* Set DTCD15 */
        dtc_controldata_15.dtccr = 0x53;
        dtc_controldata_15.dtbls = 2;
        dtc_controldata_15.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_15.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_15.dtsar = (uint16_t) p_instance_ctrl->p_ajblact;
        dtc_controldata_15.dtdar = (uint16_t) &CTSUAJBLACL;

        /* Set DTCD16 */
        dtc_controldata_16.dtccr = 0x53;
        dtc_controldata_16.dtbls = 2;
        dtc_controldata_16.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_16.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_16.dtsar = (uint16_t) p_instance_ctrl->p_ajblar;
        dtc_controldata_16.dtdar = (uint16_t) &CTSUAJBLARL;

        /* Set DTCD17 */
        dtc_controldata_17.dtccr = 0x53;
        dtc_controldata_17.dtbls = 2;
        dtc_controldata_17.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_17.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_17.dtsar = (uint16_t) p_instance_ctrl->p_ajrr;
        dtc_controldata_17.dtdar = (uint16_t) &CTSUAJRRL;

        /* Set DTCD18 */
        dtc_controldata_18.dtccr = 0x43;
        dtc_controldata_18.dtbls = 2;
        dtc_controldata_18.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_18.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_18.dtsar = (uint16_t) p_instance_ctrl->p_ctsuwr;
        dtc_controldata_18.dtdar = (uint16_t) &CTSUSO0;
    }

    DTCEN2 |= 0x80; //CTSUWR
}

/***********************************************************************************************************************
 * ctsu_transfer_autojudge_ctsurd
 ***********************************************************************************************************************/
void ctsu_transfer_autojudge_ctsurd (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    DTCEN2 &= 0xBF; //CTSURD

 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
    if (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)
    {
        /* Set DTCD19 */
        dtc_vectortable[17U] = 0xD8U;
        dtc_controldata_19.dtccr = 0x51;
        dtc_controldata_19.dtbls = 2;
        dtc_controldata_19.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_19.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_19.dtsar = (uint16_t) &CTSUAJMMARL;
        dtc_controldata_19.dtdar = (uint16_t) p_instance_ctrl->p_ajmmar;

        /* Set DTCD20 */
        dtc_controldata_20.dtccr = 0x51;
        dtc_controldata_20.dtbls = 2;
        dtc_controldata_20.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_20.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_20.dtsar = (uint16_t) &CTSUAJBLACL;
        dtc_controldata_20.dtdar = (uint16_t) p_instance_ctrl->p_ajblact;

        /* Set DTCD21 */
        dtc_controldata_21.dtccr = 0x51;
        dtc_controldata_21.dtbls = 2;
        dtc_controldata_21.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_21.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_21.dtsar = (uint16_t) &CTSUAJBLARL;
        dtc_controldata_21.dtdar = (uint16_t) p_instance_ctrl->p_ajblar;

        /* Set DTCD22 */
        dtc_controldata_22.dtccr = 0x51;
        dtc_controldata_22.dtbls = 2;
        dtc_controldata_22.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_22.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_22.dtsar = (uint16_t) &CTSUAJRRL;
        dtc_controldata_22.dtdar = (uint16_t) p_instance_ctrl->p_ajrr;

        /* Set DTCD23 */
        dtc_controldata_23.dtccr = 0x41;
        dtc_controldata_23.dtbls = 1;
        dtc_controldata_23.dtcct = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_23.dtrld = (uint16_t) p_instance_ctrl->num_elements;
        dtc_controldata_23.dtsar = (uint16_t) &CTSUSC;
        dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
    }
    else
 #endif
    {
        /* Set DTCD19 */
        dtc_vectortable[17U] = 0xD8U;
        dtc_controldata_19.dtccr = 0x51;
        dtc_controldata_19.dtbls = 2;
        dtc_controldata_19.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_19.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_19.dtsar = (uint16_t) &CTSUAJMMARL;
        dtc_controldata_19.dtdar = (uint16_t) p_instance_ctrl->p_ajmmar;

        /* Set DTCD20 */
        dtc_controldata_20.dtccr = 0x51;
        dtc_controldata_20.dtbls = 2;
        dtc_controldata_20.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_20.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_20.dtsar = (uint16_t) &CTSUAJBLACL;
        dtc_controldata_20.dtdar = (uint16_t) p_instance_ctrl->p_ajblact;

        /* Set DTCD21 */
        dtc_controldata_21.dtccr = 0x51;
        dtc_controldata_21.dtbls = 2;
        dtc_controldata_21.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_21.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_21.dtsar = (uint16_t) &CTSUAJBLARL;
        dtc_controldata_21.dtdar = (uint16_t) p_instance_ctrl->p_ajblar;

        /* Set DTCD22 */
        dtc_controldata_22.dtccr = 0x51;
        dtc_controldata_22.dtbls = 2;
        dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_22.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_22.dtsar = (uint16_t) &CTSUAJRRL;
        dtc_controldata_22.dtdar = (uint16_t) p_instance_ctrl->p_ajrr;

        /* Set DTCD23 */
        dtc_controldata_23.dtccr = 0x41;
        dtc_controldata_23.dtbls = 1;
        dtc_controldata_23.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_23.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        dtc_controldata_23.dtsar = (uint16_t) &CTSUSC;
        dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_self_raw;
    }
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
    {
        dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_mutual_raw;
    }
 #endif

    DTCEN2 |= 0x40; //CTSURD
}

#endif

#if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
/***********************************************************************************************************************
 * ctsu_sms_transfer_configure
 ***********************************************************************************************************************/
fsp_err_t ctsu_sms_transfer_configure (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if BSP_MCU_GROUP_RL78G22
    g_ctsu_sms_trigger_bit_clr = 0;
    g_ctsu_sms_trigger_bit_set = CTSU_CFG_SMS_EXTRIGGER_BIT;

    /* DATA_SIZE_16BITS | REPEAT_INT_DISABLE | CHAIN_TRANSFER_DISABLE */
    /* DEST_ADDR_FIXED | DTC_SOURCE_ADDR_INCREMENTED | REPEAT_AREA_SOURCE | DTC_TRANSFER_MODE_REPEAT */
    dtc_controldata_20.dtccr = 0x47;
    dtc_controldata_20.dtbls = (uint8_t) (CTSU_SMS_VARIABLE_INST_SIZE + CTSU_SMS_VARIABLE_GREG_SIZE);
    dtc_controldata_20.dtsar = (uint16_t) CTSU_CFG_SMS_TRANSFER_ADDRESS;
    dtc_controldata_20.dtdar = (uint16_t) &SMSI3;
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
    dtc_controldata_20.dtcct = CTSU_SMS_PAGE_NUM_SELF;
    dtc_controldata_20.dtrld = CTSU_SMS_PAGE_NUM_SELF;
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
    dtc_controldata_20.dtcct = CTSU_SMS_PAGE_NUM_MUTUAL;
    dtc_controldata_20.dtrld = CTSU_SMS_PAGE_NUM_MUTUAL;
    }
#endif
#else
    /* DATA_SIZE_16BITS | REPEAT_INT_DISABLE | CHAIN_TRANSFER_DISABLE */
    /* DEST_ADDR_FIXED | DTC_SOURCE_ADDR_INCREMENTED | REPEAT_AREA_SOURCE | DTC_TRANSFER_MODE_REPEAT */
    dtc_controldata_21.dtccr = 0x47;
    dtc_controldata_21.dtbls = (uint8_t) (CTSU_SMS_VARIABLE_INST_SIZE + CTSU_SMS_VARIABLE_GREG_SIZE);
    dtc_controldata_21.dtsar = (uint16_t) CTSU_CFG_SMS_TRANSFER_ADDRESS;
    dtc_controldata_21.dtdar = (uint16_t) &SMSI3;
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
    dtc_controldata_21.dtcct = CTSU_SMS_PAGE_NUM_SELF;
    dtc_controldata_21.dtrld = CTSU_SMS_PAGE_NUM_SELF;
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
    dtc_controldata_21.dtcct = CTSU_SMS_PAGE_NUM_MUTUAL;
    dtc_controldata_21.dtrld = CTSU_SMS_PAGE_NUM_MUTUAL;
    }
#endif
#endif
#if BSP_MCU_GROUP_RL78L23
    DTCEN6 |= 0x80;
#else
    DTCEN4 |= 0x02;
#endif

#if BSP_MCU_GROUP_RL78G22
    /* DATA_SIZE_16BITS | REPEAT_INT_DISABLE | CHAIN_TRANSFER_DISABLE */
    /* DEST_ADDR_FIXED | REPEAT_AREA_SOURCE | TRANSFER_MODE_REPEAT */
    dtc_controldata_21.dtccr = 0x43;
    dtc_controldata_21.dtbls = 0x02;
    dtc_controldata_21.dtsar = (uint16_t) CTSU_CFG_SMS_CTSUWR_ADDRESS;
    dtc_controldata_21.dtdar = (uint16_t) &CTSUSO0;

    dtc_controldata_21.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
    dtc_controldata_21.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
#else
    /* DATA_SIZE_16BITS | REPEAT_INT_DISABLE | CHAIN_TRANSFER_DISABLE */
    /* DEST_ADDR_FIXED | REPEAT_AREA_SOURCE | TRANSFER_MODE_REPEAT */
    dtc_controldata_22.dtccr = 0x43;
    dtc_controldata_22.dtbls = 0x02;
    dtc_controldata_22.dtsar = (uint16_t) CTSU_CFG_SMS_CTSUWR_ADDRESS;
    dtc_controldata_22.dtdar = (uint16_t) &CTSUSO0;

    dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
    dtc_controldata_22.dtrld = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
#endif
#if BSP_MCU_GROUP_RL78L23
    DTCEN5 |= 0x10; //CTSUWR
#else
    DTCEN4 |= 0x40; //CTSUWR
#endif
#if BSP_MCU_GROUP_RL78G22
    dtc_controldata_22.dtccr = 0x13U;
    dtc_controldata_22.dtbls = 0x01;
    dtc_controldata_22.dtsar = (uint16_t)&g_ctsu_sms_trigger_bit_set;
    dtc_controldata_22.dtdar = (uint16_t)&CTSU_CFG_SMS_EXTRIGGER_PORT;

    dtc_controldata_22.dtcct = 1;
    dtc_controldata_22.dtrld = 1;

    dtc_controldata_23.dtccr = 0x03U;
    dtc_controldata_23.dtbls = 0x01;
    dtc_controldata_23.dtsar = (uint16_t)&g_ctsu_sms_trigger_bit_clr;
    dtc_controldata_23.dtdar = (uint16_t)&CTSU_CFG_SMS_EXTRIGGER_PORT;

    dtc_controldata_23.dtcct = 1;
    dtc_controldata_23.dtrld = 1;

    DTCEN4 |= 0x20; //CTSURD
#endif

    return FSP_SUCCESS;
}
#endif /* CTSU_CFG_SMS_SUPPORT_ENABLE */
#endif /* CTSU_CFG_DTC_SUPPORT_ENABLE */

/***********************************************************************************************************************
 * ctsu_initial_offset_tuning
 ***********************************************************************************************************************/
void ctsu_initial_offset_tuning (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;
    int32_t  diff          = 0;
    uint32_t complete_flag = 0;
    uint32_t num_complete  = 0;
    uint32_t target_val;
#if (BSP_FEATURE_CTSU_VERSION == 1)
    uint16_t               ctsuso;
    ctsu_correction_calc_t calc;
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
    uint16_t i;
    uint16_t element_top;
    uint16_t corr_data[CTSU_CFG_NUM_SUMULTI];
    int32_t  ctsuso;
    uint32_t snum;
    int32_t  offset_unit;
#endif

    /* element_id through each element for control block */
    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        if (0 == *(p_instance_ctrl->p_element_complete_flag + element_id))
        {
#if (BSP_FEATURE_CTSU_VERSION == 1)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                target_val = (p_instance_ctrl->tuning_self_target_value);
            }
            else
            {
                target_val = (p_instance_ctrl->tuning_mutual_target_value);
            }

            calc.snum  = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 >> 10) & CTSU_SNUM_MAX;
            calc.sdpa  = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 >> 8) & CTSU_SDPA_MAX;
            target_val = (uint32_t) (target_val * (uint32_t) ((calc.snum + 1) * (calc.sdpa + 1)) /
                                     g_ctsu_correction_info.ctsu_clock);
            if (CTSU_COUNT_MAX < target_val)
            {
                target_val = CTSU_COUNT_MAX;
            }

 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                diff = (int32_t) ((p_instance_ctrl->p_self_data + element_id)->int_data - target_val);
            }
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
            {
                diff = (int32_t) ((p_instance_ctrl->p_mutual_pri_data + element_id)->int_data - target_val);
            }
 #endif
            ctsuso = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 & CTSU_TUNING_MAX);
            if (0 < diff)
            {
                if (*(p_instance_ctrl->p_tuning_diff + element_id) < 0)
                {
                    if ((-diff) > *(p_instance_ctrl->p_tuning_diff + element_id))
                    {
                        ctsuso++;      ///< Decrease count
                    }

                    complete_flag = 1;
                }
                else
                {
                    if (CTSU_TUNING_MAX == ctsuso) /* CTSUSO limit check    */
                    {
                        complete_flag = 1;
                    }
                    else
                    {
                        ctsuso++;                                                ///< Decrease count
                        (*(p_instance_ctrl->p_tuning_diff + element_id)) = diff; ///< Plus
                    }
                }
            }
            else if (0 == diff)
            {
                complete_flag = 1;
            }
            else
            {
                if (*(p_instance_ctrl->p_tuning_diff + element_id) > 0)
                {
                    if ((-diff) > *(p_instance_ctrl->p_tuning_diff + element_id))
                    {
                        ctsuso--;      ///< Increase count
                    }

                    complete_flag = 1;
                }
                else
                {
                    if (CTSU_TUNING_MIN == ctsuso) /* CTSUSO limit check    */
                    {
                        complete_flag = 1;
                    }
                    else
                    {
                        ctsuso--;                                                ///< Increase count
                        (*(p_instance_ctrl->p_tuning_diff + element_id)) = diff; ///< Minus
                    }
                }
            }

            p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 &= (uint16_t) (~CTSU_TUNING_MAX);
            p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 |= ctsuso;
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
            element_top = (uint16_t) (element_id * CTSU_CFG_NUM_SUMULTI);
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                /* Adjust only frequencies for which offset tuning is not completed */
                if (0 == (p_instance_ctrl->p_frequency_complete_flag[element_id] & (1 << i)))
                {
                    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
                    {
                        target_val = (p_instance_ctrl->tuning_self_target_value / 2);
                    }
                    else
                    {
                        target_val = (p_instance_ctrl->tuning_mutual_target_value / 2);
                    }

                    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
                    {
                        corr_data[i] = p_instance_ctrl->p_self_corr[element_top + i];
                    }
                    else
                    {
                        corr_data[i] = p_instance_ctrl->p_mutual_pri_corr[element_top + i];
                    }

                    snum = (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI)].ctsuso >> 10) &
                           CTSU_SNUM_MAX;
                    target_val = (uint32_t) ((target_val * (snum + 1)) / (CTSU_SNUM_RECOMMEND + 1));
                    if (CTSU_COUNT_MAX < target_val)
                    {
                        target_val = CTSU_COUNT_MAX;
                    }

                    offset_unit = (int32_t) ((CTSU_CORRECTION_OFFSET_UNIT * (snum + 1)) / (CTSU_SNUM_RECOMMEND + 1));

                    /* Calculate CTSUSO equivalent difference between current value and target value */
                    diff = (int32_t) ((int32_t) corr_data[i] - (int32_t) target_val) /
                           (offset_unit >> p_instance_ctrl->range);

                    ctsuso  = (int32_t) (p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso & CTSU_TUNING_MAX);
                    ctsuso += diff;

                    /* If the CTSUSO exceeds the minimum value or the maximum value, tuning complete */
                    if (ctsuso < 0)
                    {
                        ctsuso = 0;
                        p_instance_ctrl->p_frequency_complete_flag[element_id] += (uint8_t) (1 << i);
                    }
                    else if (ctsuso > CTSU_TUNING_MAX)
                    {
                        ctsuso = CTSU_TUNING_MAX;
                        p_instance_ctrl->p_frequency_complete_flag[element_id] += (uint8_t) (1 << i);
                    }
                    else
                    {
                        /* If the difference is large, tuning value may not be able to match, so create the next opportunity */
                        if (0 == diff)
                        {
                            p_instance_ctrl->p_frequency_complete_flag[element_id] += (uint8_t) (1 << i);
                        }
                    }

                    /* Set the result of the calculated CTSUSO */
                    p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso &= (uint32_t) (~CTSU_TUNING_MAX);
                    p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso |= (uint32_t) ctsuso;
                }

                /* Add completion status for each frequency */
                complete_flag += ((p_instance_ctrl->p_frequency_complete_flag[element_id] >> i) & 1);
            }
#endif
        }
        else
        {
            complete_flag = CTSU_CFG_NUM_SUMULTI;
        }

        if (CTSU_CFG_NUM_SUMULTI == complete_flag)
        {
            num_complete++;
            *(p_instance_ctrl->p_element_complete_flag + element_id) = 1;
        }

        complete_flag = 0;
    }

    if (num_complete == p_instance_ctrl->num_elements)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }
}

/***********************************************************************************************************************
 * ctsu_moving_average
 ***********************************************************************************************************************/
void ctsu_moving_average (ctsu_data_t * p_average, uint16_t new_data, uint16_t average_num)
{
    uint32_t work;

    work  = (uint32_t) (((uint32_t) p_average->int_data << CTSU_CFG_DECIMAL_POINT) + p_average->decimal_point_data);
    work -= (uint32_t) (work / average_num);
    work += (uint32_t) (((uint32_t) new_data << CTSU_CFG_DECIMAL_POINT) / average_num);

    p_average->int_data           = (uint16_t) (work >> CTSU_CFG_DECIMAL_POINT);
    p_average->decimal_point_data = (uint16_t) (work & CTSU_CFG_DECIMAL_POINT_MASK);
}

/***********************************************************************************************************************
 * CTSUWR interrupt handler. This service routine sets the tuning for the next element to be scanned by hardware.
 ***********************************************************************************************************************/
#if defined(__CCRL__)
 #pragma interrupt ctsu_write_isr(vect=INTCTSUWR)
 void ctsu_write_isr (void)
#elif defined(__ICCRL78__)
 #pragma vector=INTCTSUWR_vect
 __interrupt void ctsu_write_isr (void)
#elif defined(__llvm__)
 void ctsu_write_isr (void)
#endif
{
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 0)
    ctsu_instance_ctrl_t * p_instance_ctrl = gp_ctsu_isr_context;

    /* Write settings for current element */
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
 #if (BSP_FEATURE_CTSU_VERSION == 2)
        CTSUSO0 = (uint16_t) g_ctsu_correction_info.ctsuwr.ctsuso;
        CTSUSO1 = (uint16_t) (g_ctsu_correction_info.ctsuwr.ctsuso >> 16);
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 1)
        CTSUSSC = g_ctsu_correction_info.ctsuwr.ctsussc;
        CTSUSO0 = g_ctsu_correction_info.ctsuwr.ctsuso0;
        CTSUSO1 = g_ctsu_correction_info.ctsuwr.ctsuso1;
 #endif
    }
 #if (BSP_FEATURE_CTSU_VERSION == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
  #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
        CTSUSSC = g_ctsu_diag_info.ctsuwr.ctsussc;
        CTSUSO0 = g_ctsu_diag_info.ctsuwr.ctsuso0;
        CTSUSO1 = g_ctsu_diag_info.ctsuwr.ctsuso1;
  #endif
    }
 #endif
    else
    {
 #if (BSP_FEATURE_CTSU_VERSION == 2)
        CTSUSO0 = (uint16_t) p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso;
        CTSUSO1 = (uint16_t) (p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso >> 16);
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 1)
        CTSUSSC = p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsussc;
        CTSUSO0 = p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso0;
        CTSUSO1 = p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso1;
 #endif
        p_instance_ctrl->wr_index++;
    }
#endif
}

/***********************************************************************************************************************
 * CTSURD interrupt handler. This service routine reads the sensor count and reference counter for
 * the current element and places the value in the scan data buffer. Note that the reference counter
 * does not work properly but is saved anyway for backward compatibility and potential future use.
 * Additionally, the SC register cannot be read again until RC is read.
 ***********************************************************************************************************************/
#if defined(__CCRL__)
 #pragma interrupt ctsu_read_isr(vect=INTCTSURD)
 void ctsu_read_isr (void)
#elif defined(__ICCRL78__)
 #pragma vector=INTCTSURD_vect
 __interrupt void ctsu_read_isr (void)
#elif defined(__llvm__)
 void ctsu_read_isr (void)
#endif
{
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 0)
    ctsu_instance_ctrl_t * p_instance_ctrl = gp_ctsu_isr_context;

    /* read current channel/element value */
    /* Store the reference counter for possible future use. Register must be read or scan will hang. */

 #if (BSP_FEATURE_CTSU_VERSION == 1)
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        g_ctsu_correction_info.scanbuf.sen = CTSUSC;
        g_ctsu_correction_info.scanbuf.ref = CTSURC;
    }

  #if (BSP_FEATURE_CTSU_VERSION == 1)
   #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        g_ctsu_diag_info.scanbuf.sen = CTSUSC;
        g_ctsu_diag_info.scanbuf.ref = CTSURC;
    }
   #endif
  #endif
    else
    {
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            (p_instance_ctrl->p_self_raw + p_instance_ctrl->rd_index)->sen = CTSUSC;
            (p_instance_ctrl->p_self_raw + p_instance_ctrl->rd_index)->ref = CTSURC;
            p_instance_ctrl->rd_index++;
        }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            if (false == p_instance_ctrl->serial_tuning_enable)
            {
                if (1 == ((CTSUST & 0x80) >> 7))
                {
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->pri_sen = CTSUSC;
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->pri_ref = CTSURC;
                }
                else
                {
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->snd_sen = CTSUSC;
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->snd_ref = CTSURC;
                    p_instance_ctrl->rd_index++;
                }
            }
            else
            {
                p_instance_ctrl->serial_tuning_mutual_cnt++;
                if (p_instance_ctrl->serial_tuning_mutual_cnt % 2)
                {
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->pri_sen = CTSUSC;
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->pri_ref = CTSURC;
                }
                else
                {
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->snd_sen = CTSUSC;
                    (p_instance_ctrl->p_mutual_raw + p_instance_ctrl->rd_index)->snd_ref = CTSURC;
                    p_instance_ctrl->rd_index++;
                }
            }
        }
  #endif
    }
 #endif

 #if (BSP_FEATURE_CTSU_VERSION == 2)
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        g_ctsu_correction_info.scanbuf = CTSUSC;
    }
  #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    else if ((CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md) &&
             ((CTSU_DIAG_SUCLK == g_ctsu_diag_info.state) ||
              (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
             ))
    {
        p_instance_ctrl->p_self_raw[p_instance_ctrl->rd_index] = CTSUUC;
        p_instance_ctrl->rd_index++;
    }
  #endif
    else
    {
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->md))
        {
            p_instance_ctrl->p_self_raw[p_instance_ctrl->rd_index] = CTSUSC;
            p_instance_ctrl->rd_index++;
        }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            p_instance_ctrl->p_mutual_raw[p_instance_ctrl->rd_index] = CTSUSC;
            p_instance_ctrl->rd_index++;
        }
  #endif
    }
 #endif
    if (1 == p_instance_ctrl->interrupt_reverse_flag)
    {
        p_instance_ctrl->interrupt_reverse_flag = 0;

        ctsu_end_interrupt(p_instance_ctrl);
    }
#endif
}

/***********************************************************************************************************************
 * CTSUFN interrupt handler. This service routine occurs when all elements have been scanned (finished).
 * The user's callback function is called if available.
 ***********************************************************************************************************************/
#if defined(__CCRL__)
 #pragma interrupt ctsu_end_isr(vect=INTCTSUFN)
 void ctsu_end_isr (void)
#elif defined(__ICCRL78__)
 #pragma vector=INTCTSUFN_vect
 __interrupt void ctsu_end_isr (void)
#elif defined(__llvm__)
 void ctsu_end_isr (void)
#endif
{
    ctsu_instance_ctrl_t * p_instance_ctrl = gp_ctsu_isr_context;
    uint16_t               rd_index;

    /* In CTSU1 and CTSU2 self-capacity and current measurement mode and diagnostic mode, */
    /* rd_index has the same value as wr_index.                                           */
    rd_index = p_instance_ctrl->rd_index;

#if (BSP_FEATURE_CTSU_VERSION == 1)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        if (true == p_instance_ctrl->serial_tuning_enable)
        {
            if (0 == ((p_instance_ctrl->ctsucr1 >> 7) & 0x01))
            {
                rd_index = p_instance_ctrl->serial_tuning_mutual_cnt;
            }
        }
    }
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        if (true == p_instance_ctrl->serial_tuning_enable)
        {
            if (0 == ((p_instance_ctrl->ctsucr1 >> 7) & 0x01))
            {
                rd_index = p_instance_ctrl->rd_index;
            }
            else
            {
                rd_index = p_instance_ctrl->rd_index / 2;
            }
        }
        else
        {
            /* In the mutual capacity of CTSU2, the value of rd_index is twice the value of wr_index. */
            rd_index = p_instance_ctrl->rd_index / 2;
        }
    }
 #endif
#endif

    /* Countermeasure for the problem that RD interrupt and FN interrupt are reversed. */
    if (rd_index != p_instance_ctrl->wr_index)
    {
        p_instance_ctrl->interrupt_reverse_flag = 1;

        return;
    }

    ctsu_end_interrupt(p_instance_ctrl);
}

void ctsu_end_interrupt (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    ctsu_callback_args_t args;

    /* Store callback arguments in memory provided by user if available.  This allows callback arguments to be
     * stored in non-secure memory so they can be accessed by a non-secure callback function. */
    ctsu_callback_args_t * p_args = p_instance_ctrl->p_callback_memory;
    if (NULL == p_args)
    {
        /* Store on stack */
        p_args = &args;
    }
    else
    {
        /* Save current arguments on the stack in case this is a nested interrupt. */
        args = *p_args;
    }

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        if (((CTSUERRS & 0x8000) >> 15) == 1)
        {
            g_ctsu_diag_info.icomp = 1;
        }
    }
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_MODE_DIAGNOSIS_SCAN == p_instance_ctrl->md)
    {
        if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
        {
            if (0 == g_ctsu_diag_info.test_count)
            {
                /* TEST1: Check ICOMP0 in normal voltage condition */
                if (0 != (CTSUSRL & CTSUSR_ICOMP0_Msk))
                {
                    /* FAIL: Set diagnostic error */
                    g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_VOLTAGE;
                }

                /* PASS: Check is skipped because ICOMP0 value is expected.
                 * Condition: 0 == R_CTSU->CTSUSR_b.ICOMP0 */
            }
            else
            {
                /* TEST2: Check ICOMP0 in over voltage condition */
                if (0 == (CTSUSRL & CTSUSR_ICOMP0_Msk))
                {
                    /* FAIL: Set diagnostic error */
                    g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_VOLTAGE;
                }
                else
                {
                    /* PASS: Reset ICOMP0 */
                    CTSUSRL  |= CTSUSR_ICOMPRST_Msk;
                    if (0 != (CTSUSRL & CTSUSR_ICOMP0_Msk))
                    {
                        /* FAIL: Set diagnostic error */
                        g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_VOLTAGE;
                    }

                    /* PASS: Check is skipped because ICOMP0 value is expected.
                     * Condition: 0 == R_CTSU->CTSUSR_b.ICOMP0 */
                }
            }
        }
        else if (CTSU_DIAG_OVER_CURRENT == g_ctsu_diag_info.state)
        {
            if (0 == g_ctsu_diag_info.test_count)
            {
                /* TEST1: Check ICOMP1 in normal current condition */
                if (0 != (CTSUSRL & CTSUSR_ICOMP1_Msk))
                {
                    /* FAIL: Set error result */
                    g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_CURRENT;
                }

                /* PASS: Check is skipped because ICOMP1 value is expected.
                 * Condition: 0 == R_CTSU->CTSUSR_b.ICOMP1 */
            }
            else
            {
                /* TEST2: Check ICOMP1 in over current condition */
                if (0 == (CTSUSRL & CTSUSR_ICOMP1_Msk))
                {
                    /* FAIL: Set error result */
                    g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_CURRENT;
                }
                else
                {
                    /* PASS: Reset ICOMP1 */
                    CTSUSRL  |= CTSUSR_ICOMPRST_Msk;
                    if (0 != (CTSUSRL & CTSUSR_ICOMP1_Msk))
                    {
                        /* FAIL: Set error result */
                        g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OVER_CURRENT;
                    }

                    /* PASS: Check is skipped because ICOMP1 value is expected.
                     * Condition: 0 == R_CTSU->CTSUSR_b.ICOMP1 */
                }
            }
        }
        else
        {
            /* Do nothing */
        }
    }
 #endif
#endif

    p_args->event = CTSU_EVENT_SCAN_COMPLETE;

    if (CTSUST & CTSU_SOVF)
    {
        p_args->event  |= CTSU_EVENT_OVERFLOW;
        CTSUST &= (uint8_t) (~CTSU_SOVF);
    }

#if (BSP_FEATURE_CTSU_VERSION == 2)
    if (CTSUSRL & (CTSU_ICOMP0 | CTSU_ICOMP1))
    {
        if (CTSUSRL & CTSU_ICOMP0)
        {
            p_args->event |= CTSU_EVENT_ICOMP;
        }
        if (CTSUSRL & CTSU_ICOMP1)
        {
            p_args->event |= CTSU_EVENT_ICOMP1;
        }

        CTSUSRL |= CTSU_ICOMPRST;
    }
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
    if (((CTSUERRS & 0x8000) >> 15) == 1)
    {
        CTSUCR1 &= (uint8_t) (~0x01);
        BSP_NOP();
        BSP_NOP();
        CTSUCR1 |= 0x01;
        p_args->event   |= CTSU_EVENT_ICOMP;
    }
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    if (CTSU_CORRECTION_RUN != g_ctsu_correction_info.status)
    {
        if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->md)
        {
            CTSUCRAH   &= ~CTSUCRA_SDPSEL_Msk;
            CTSUSUCLK0 = (uint16_t) ((CTSU_CFG_SUMULTI0 << 8) | g_ctsu_correction_info.suadj0);
            CTSUCRBH   &= ~CTSUCRB_SSCNT_Msk;
            CTSUCRBH   |= 1 << CTSUCRB_SSCNT_Pos;
            CTSUDBGR1  &= ~CTSUCALIB_SUCARRY_Msk;
            CTSUDBGR1  &= ~CTSUCALIB_CCOCALIB_Msk;
            CTSUDBGR1  |= CTSUCALIB_CCOCLK_Msk;
            CTSUDBGR0  &= ~CTSUCALIB_TSOC_Msk;
            CTSUCRAH   |= CTSUCRA_SDPSEL_Msk;
        }
    }
 #endif
#endif

#if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)
    if ((1 == p_instance_ctrl->tsod) && (CTSU_CAP_EXTERNAL == p_instance_ctrl->cap))
    {
        /* When using MEC, MCH0 and MCH1 are cleared, and when using external trigger, */
        /* the next R_CTSU_ScanStart() is not called, so MCH0 and MCH1 are set again here. */
        CTSUMCH0 = (p_instance_ctrl->mec_ts & CTSUMCH_MCH0_Msk);

        /* Set MCH1 when using both MEC and Active Shield.*/
        if ((0 != p_instance_ctrl->ctsuchtrc0) ||
            (0 != p_instance_ctrl->ctsuchtrc1) ||
            (0 != p_instance_ctrl->ctsuchtrc2) ||
            (0 != p_instance_ctrl->ctsuchtrc3) ||
            (0 != p_instance_ctrl->ctsuchtrc4))
        {
            CTSUMCH1 = (p_instance_ctrl->mec_shield_ts & (CTSUMCH_MCH1_Msk >> CTSUMCH_MCH1_Pos));
        }
    }
#endif

    p_instance_ctrl->state        = CTSU_STATE_SCANNED;
    p_instance_ctrl->error_status = args.event;
    p_args->p_context             = p_instance_ctrl->p_context;

    /* If a callback was provided, call it with the argument */
    if (NULL != p_instance_ctrl->p_callback)
    {
#if BSP_TZ_SECURE_BUILD

        /* p_callback can point to a secure function or a non-secure function. */
        if (!cmse_is_nsfptr(p_instance_ctrl->p_callback))
        {
            /* If p_callback is secure, then the project does not need to change security state. */
            p_instance_ctrl->p_callback(p_args);
        }
        else
        {
            /* If p_callback is Non-secure, then the project must change to Non-secure state in order to call the callback. */
            ctsu_prv_ns_callback p_callback = (ctsu_prv_ns_callback) (p_instance_ctrl->p_callback);
            p_callback(p_args);
        }

#else

        /* If the project is not Trustzone Secure, then it will never need to change security state in order to call the callback. */
        p_instance_ctrl->p_callback(p_args);
#endif
    }

    if (NULL != p_instance_ctrl->p_callback_memory)
    {
        /* Restore callback memory in case this is a nested interrupt. */
        *p_instance_ctrl->p_callback_memory = args;
    }

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
    if (CTSU_CORRECTION_RUN != g_ctsu_correction_info.status)
    {
        if (CTSU_CAP_EXTERNAL == p_instance_ctrl->cap)
        {
            CTSUCR0 &= (uint8_t) ~0x04; // Clear SNZ bit
            CTSUCR0 |= (uint8_t ) 0x04; // Set SNZ bit
            ctsu_transfer_configure(p_instance_ctrl);
        }
    }
#endif

#if BSP_MCU_GROUP_RL78G16
    if (CTSU_CAP_EXTERNAL == p_instance_ctrl->cap)
    {
        CTSUCR0  = (uint8_t) (CTSUCR0 & 0xFE) | 0x10;   /* CTSUSTRT=0 & CTSUINIT=1 */
        CTSUCR0 &= (uint8_t) ~0x04;                     /* CTSUSNZ=0 */
    }
#endif

    /* reset indexes */
    p_instance_ctrl->wr_index = 0;
    p_instance_ctrl->rd_index = 0;
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    p_instance_ctrl->serial_tuning_mutual_cnt = 0;
#endif
}

/***********************************************************************************************************************
 * ctsu_correction_process
 ***********************************************************************************************************************/
void ctsu_correction_process (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if (BSP_FEATURE_CTSU_VERSION == 2)
    uint32_t i;
    uint32_t j;
    uint32_t trimb;
    uint8_t  rtrim;
    uint16_t x1;
    uint16_t x0;
    uint16_t y0;
    uint16_t y1;
    uint16_t dac2vdc;
    uint8_t  trimb_byte;
    uint16_t dac_value;
    uint32_t ref_value;
    uint16_t c0;
    uint16_t c1;

    g_ctsu_correction_info.status = CTSU_CORRECTION_RUN;

    CTSUCRAL |= CTSUCRA_MD0_Msk;
    CTSUCRAL &= ~CTSUCRA_MD1_Msk;
    CTSUCRAH &= ~CTSUCRA_MD2_Msk;
    CTSUMCHH = 0x0001;
    CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
    CTSUCRAH |= 1 << CTSUCRA_LOAD_Pos;

    /* Setting time of measurement */
    g_ctsu_correction_info.ctsuwr.ctsuso = (CTSU_SNUM_RECOMMEND << 10);

    /* Measure the current input to the ICO by passing current through the internal resistance in each range. */
    /* The theoretical value of the current is 12.5uA. */
    CTSUCHACAL = 1;
#if BSP_MCU_GROUP_RL78F22
    /* nothing */
#elif BSP_MCU_GROUP_RL78F25
    CTSUCHACAH = 0;
#else
    CTSUCHACAH = 0;
    CTSUCHACBL = 0;
#endif

    CTSUDBGR0 |= CTSUCALIB_TSOC_Msk;
    for (i = 0; i < CTSU_RANGE_NUM; i++)
    {
        if (CTSU_RANGE_20UA == i)
        {
            CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
            CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
        }
        else if (CTSU_RANGE_40UA == i)
        {
            CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
            CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
        }
        else if (CTSU_RANGE_80UA == i)
        {
            CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
            CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
        }
        else
        {
            CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
            CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
        }

        CTSUCRAH |= 3 << CTSUCRA_LOAD_Pos;
        ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.base_value[i]);
        CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
        CTSUCRAH |= 1 << CTSUCRA_LOAD_Pos;
    }

    /* Measure by inputting each constant current from internal DAC to ICO. */
    CTSUCRBH  &= ~CTSUCRB_SSCNT_Msk;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCLK_Msk;
    CTSUDBGR1 |= CTSUCALIB_CCOCALIB_Msk;

    /* 2.5uA * (j + 1), SUCARRY and SSCNT = 3 are required for greater than 20uA */
    for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
    {
        CTSUCRAH  &= ~CTSUCRA_PCSEL_Msk;
        if (8 > j)
        {
            CTSUSUCLK0 = (uint16_t) (((j + 1) * CTSU_CORRECTION_SUMULTI) - 1);
        }
        else
        {
            CTSUCRBH |= CTSUCRB_SSCNT_Msk;
            CTSUDBGR1 |= CTSUCALIB_SUCARRY_Msk;
            CTSUSUCLK0          = (uint16_t) (((j - 3) * CTSU_CORRECTION_SUMULTI) - 1);
        }

        CTSUCRAH  |= CTSUCRA_PCSEL_Msk;

        // store CCODAC path value to tha last array of dac_value
        ctsu_correction_measurement(p_instance_ctrl, &(g_ctsu_correction_info.dac_value[CTSU_RANGE_NUM - 1][j]));
    }

    /* Calculate the coefficient between step2 and step3 in each range. */
    trimb = (uint32_t) CTSUTRIM3 << 16;
    trimb = trimb | (uint32_t) (CTSUTRIM2);
    rtrim = (uint8_t) (CTSUTRIM0 & 0x00FF);

    for (i = 0; i < CTSU_RANGE_NUM; i++)
    {
        /* Get resistance value from TRIMB register. */
        /* Error rate calculation on VCD path as UQ1.9 */
        trimb_byte = (uint8_t) ((trimb >> ((CTSU_RANGE_NUM - 1 - i) * 8)) & CTSU_CORRECTION_TRIMB_MAX);

        switch (trimb_byte & CTSU_CORRECTION_TRIMB_THRESHOLD1)
        {
            case CTSU_CORRECTION_TRIMB_THRESHOLD1:
            {
                if (rtrim > CTSU_CORRECTION_RTRIM_THRESHOLD1)
                {
                    x1 = CTSU_CORRECTION_BIT8; /* 0.05 */
                }
                else
                {
                    x1 = CTSU_CORRECTION_BIT9; /* 1.00 */
                }

                break;
            }

            case 0x00:
            {
                if (rtrim < CTSU_CORRECTION_RTRIM_THRESHOLD2)
                {
                    x1 = (CTSU_CORRECTION_BIT7 | CTSU_CORRECTION_BIT8); /* 0.25 and 0.50 */
                }
                else
                {
                    x1 = (CTSU_CORRECTION_BIT7 | CTSU_CORRECTION_BIT9); /* 0.25 and 1.00 */
                }

                break;
            }

            case CTSU_CORRECTION_TRIMB_SIGN_BIT:
            {
                x1 = CTSU_CORRECTION_BIT9;                          /* 1.00 */
                break;
            }

            default:                                                /* 0x40 */
            {
                x1 = (CTSU_CORRECTION_BIT7 | CTSU_CORRECTION_BIT8); /* 0.25 and 0.50 */
                break;
            }
        }

        g_ctsu_correction_info.error_rate[i] = x1 + (trimb_byte & CTSU_CORRECTION_BIT6_0);

        // searching for upper index of CCODAC path value containing VDC path value
        // Possibility of base_value > g_ctsu_correction_info.dac_value[][CTSU_CORRECTION_POINT_NUM - 1]

        // Theoretical value at VDC path measurement
        x1 = (uint16_t) (((uint32_t) CTSU_CORRECTION_STD_VAL << 9) / g_ctsu_correction_info.error_rate[i]);

        j = x1 / CTSU_CORRECTION_STD_UNIT;              // upper index of theoretical value

        x0 = (uint16_t) (CTSU_CORRECTION_STD_UNIT * j); // lower side Theoretical value
        y0 = g_ctsu_correction_info.dac_value[CTSU_RANGE_NUM - 1][j - 1];
        y1 = g_ctsu_correction_info.dac_value[CTSU_RANGE_NUM - 1][j];

        // Calculate CCODAC path value on VDC path current by linear interpolation
        dac2vdc = (uint16_t) (y0 + ((uint32_t) (y1 - y0) * (x1 - x0)) / CTSU_CORRECTION_STD_UNIT);

        // calculate VDC path conversion ratio
        dac2vdc =
            (uint16_t) (((int32_t) g_ctsu_correction_info.base_value[i] << CTSU_CORRECTION_DIV_PRECISION) / dac2vdc);

        // Correct CCODAC path value using conversion ratio, and make correction coefficients
        for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
        {
            dac_value =
                (uint16_t) (((uint32_t) g_ctsu_correction_info.dac_value[CTSU_RANGE_NUM - 1][j] * dac2vdc) >>
                            CTSU_CORRECTION_DIV_PRECISION);
            ref_value = (CTSU_CORRECTION_STD_UNIT * (j + 1));

            g_ctsu_correction_info.dac_value[i][j] = dac_value;
            g_ctsu_correction_info.coef[i][j]      =
                (uint16_t) ((ref_value << CTSU_CORRECTION_DIV_PRECISION) / dac_value);
        }

 #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
        /* Store edge data for SMS correction calculation */       
        g_ctsu_correction_info.dac_value_edge[i][0] = g_ctsu_correction_info.dac_value[i][0];
        g_ctsu_correction_info.dac_value_edge[i][1] = g_ctsu_correction_info.dac_value[i][11];
 #endif

        // expand correction range
        c0 = g_ctsu_correction_info.coef[i][0];
        c1 = g_ctsu_correction_info.coef[i][1];
        x0 = g_ctsu_correction_info.dac_value[i][0];
        x1 = g_ctsu_correction_info.dac_value[i][1];

        g_ctsu_correction_info.coef[i][0] =
            (uint16_t) ((int32_t) c1 - (((int32_t) c1 - (int32_t) c0) * x1) / (x1 - x0));
        g_ctsu_correction_info.dac_value[i][0] = 0;

        c0 = g_ctsu_correction_info.coef[i][10];
        c1 = g_ctsu_correction_info.coef[i][11];
        x0 = g_ctsu_correction_info.dac_value[i][10];
        x1 = g_ctsu_correction_info.dac_value[i][11];

        g_ctsu_correction_info.coef[i][11] =
            (uint16_t) (c1 + (((int32_t) c1 - (int32_t) c0) * (CTSU_COUNT_MAX - x1)) / (x1 - x0));
        g_ctsu_correction_info.dac_value[i][11] = CTSU_COUNT_MAX;
    }

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    for (i = 0; i < (CTSU_RANGE_NUM - 1); i++)
    {
        g_ctsu_correction_info.range_ratio[i] =
            (uint16_t) (((uint32_t) g_ctsu_correction_info.error_rate[i] << CTSU_SHIFT_AMOUNT) /
                        g_ctsu_correction_info.error_rate[CTSU_RANGE_160UA]);
    }
 #endif

 #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    ctsu_auto_correction_register_set (p_instance_ctrl);

    /* Sensor counter auto correction enabled */
    CTSUOPTL |= 0x0001U;        /* CCOCFEN = 1 */
 #endif
    g_ctsu_correction_info.status = CTSU_CORRECTION_COMPLETE;
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
    uint16_t second_std_val;
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz;

 #if BSP_FEATURE_CTSU_HAS_TRMR
    uint8_t ctsutrimr_def;
 #endif

    g_ctsu_correction_info.status = CTSU_CORRECTION_RUN;

    pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        CTSUCR1 |= (0 << 4);
        ctsu_sdpa        = pclkb_mhz - 1;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        CTSUCR1 |= (1 << 4);
        ctsu_sdpa        = (pclkb_mhz / 2) - 1;
    }
    else
    {
        CTSUCR1 |= (2 << 4);
        ctsu_sdpa        = (pclkb_mhz / 4) - 1;
    }

    g_ctsu_correction_info.ctsu_clock = pclkb_mhz >> CTSU_CFG_PCLK_DIVISION;

    CTSUCR1    |= (CTSU_MODE_SELF_MULTI_SCAN << 6);
    CTSUCHAC0 = 0x01;

    g_ctsu_correction_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_correction_info.ctsuwr.ctsuso0 = 0x0000;

    /* Set CTSUSO1 */
    g_ctsu_correction_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | (ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    CTSUERRS = 0x0082;
    
    ctsu_delay(CTSU_WAIT_TIME);

    /* First value measurement */
    ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.first_val);

    /* Second standard value create */
 #if BSP_FEATURE_CTSU_HAS_TRMR
    uint32_t work;

    /* Current trimming value storage */
    ctsutrimr_def = RTRIM;

    /*                       ctsutrimr_def + 273            ((ctsutrimr_def + 273) * 2560 * 128)      */
    /*    second_std_val  =  ------------------- * 40960 =  ------------------------------------ + 64 */
    /*                               528                                   33 * 128                   */

    work           = (uint32_t) (((uint32_t) ctsutrimr_def + 273) * 9930 + 64);
    second_std_val = (uint16_t) (work >> 7UL);

    /* 0xFF set in the current trimming register */
    RTRIM          = 0xFF;
 #else
    second_std_val = (uint16_t) (CTSU_CORRECTION_2ND_STD_VAL * CTSU_WAFER_PARAMETER);
    g_ctsu_correction_info.ctsuwr.ctsuso1 |= (uint16_t) (CTSU_ICOG_40 << 13); /* ICO gain  66% -> 40% */
 #endif

    /* Second value measurement */
    ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.second_val);
 #if BSP_FEATURE_CTSU_HAS_TRMR

    /* Return the current trimming register to the initial value */
    RTRIM         = ctsutrimr_def;
 #endif

    /* Normal measurement setting */
    CTSUERRS = 0;

    ctsu_delay(CTSU_WAIT_TIME);

    if ((0 != g_ctsu_correction_info.first_val) && (0 != g_ctsu_correction_info.second_val))
    {
        if (g_ctsu_correction_info.second_val < g_ctsu_correction_info.first_val)
        {
            /* 1st coefficient create */
            g_ctsu_correction_info.first_coefficient = (CTSU_CORRECTION_1ST_STD_VAL << CTSU_SHIFT_AMOUNT) /
                                                       g_ctsu_correction_info.first_val;

            /* 2nd coefficient create */
            g_ctsu_correction_info.second_coefficient = ((uint32_t) second_std_val << CTSU_SHIFT_AMOUNT) /
                                                        g_ctsu_correction_info.second_val;

            g_ctsu_correction_info.status = CTSU_CORRECTION_COMPLETE;
        }
        else
        {
            g_ctsu_correction_info.status = CTSU_CORRECTION_ERROR;
        }
    }
    else
    {
        g_ctsu_correction_info.status = CTSU_CORRECTION_ERROR;
    }
#endif
}

/***********************************************************************************************************************
 * ctsu_correction_measurement
 ***********************************************************************************************************************/
void ctsu_correction_measurement (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * data)
{
    uint16_t i;
    uint32_t sum = 0;

    for (i = 0; i < CTSU_CORRECTION_AVERAGE; i++)
    {
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        ctsu_transfer_configure(p_instance_ctrl);
#else
        FSP_PARAMETER_NOT_USED(p_instance_ctrl);
#endif
        p_instance_ctrl->state = CTSU_STATE_SCANNING;
#if (BSP_FEATURE_CTSU_VERSION == 2)
        CTSUCR0 |= 0x01;
        while (p_instance_ctrl->state != CTSU_STATE_SCANNED)
        {
        }

        sum += g_ctsu_correction_info.scanbuf;
#endif
#if (BSP_FEATURE_CTSU_VERSION == 1)
        CTSUCR0       |= 0x01;
        while (p_instance_ctrl->state != CTSU_STATE_SCANNED)
        {
        }
        sum += g_ctsu_correction_info.scanbuf.sen;
#endif
    }

    *data = (uint16_t) (sum / CTSU_CORRECTION_AVERAGE);
}

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)

/***********************************************************************************************************************
 * ctsu_correction_scan_start
 ***********************************************************************************************************************/
void ctsu_correction_scan_start (void)
{
    CTSUCRAL |= CTSUCRA_MD0_Msk;
    CTSUCRAL &= ~CTSUCRA_MD1_Msk;
    CTSUCRAH &= ~CTSUCRA_MD2_Msk;
    CTSUMCHH = 0x0001;

    /* Setting time of measurement */
    g_ctsu_correction_info.ctsuwr.ctsuso = (CTSU_SNUM_RECOMMEND << 10);
    g_ctsu_correction_info.suadj0        = (uint8_t) (CTSUSUCLK0 & CTSU_SUADJ_MAX);

    if (g_ctsu_correction_info.scan_index < CTSU_CORRECTION_POINT_NUM)
    {
        /* Dummy setting */
        CTSUCHACAL = 1;
  #if BSP_MCU_GROUP_RL78F22
        /* nothing */
  #elif BSP_MCU_GROUP_RL78F25
        CTSUCHACAH = 0;
  #else
        CTSUCHACAH = 0;
        CTSUCHACBL = 0;
  #endif

        /* Step3 : Measure by inputting each constant current from internal DAC to ICO. */
        CTSUCRBH  &= ~CTSUCRB_SSCNT_Msk;
        CTSUDBGR1 &= ~CTSUCALIB_CCOCLK_Msk;
        CTSUDBGR1 |= CTSUCALIB_CCOCALIB_Msk;
        CTSUDBGR0 |= CTSUCALIB_TSOC_Msk;

        CTSUCRAH  &= ~CTSUCRA_SDPSEL_Msk;
        if (8 > g_ctsu_correction_info.scan_index)
        {
            CTSUSUCLK0 = (uint16_t) (((g_ctsu_correction_info.scan_index + 1) * CTSU_CORRECTION_SUMULTI) - 1);
        }
        else
        {
            /* SUCARRY is required for greater than 10uA */
            CTSUCRBH |= CTSUCRB_SSCNT_Msk;
            CTSUDBGR1 |= CTSUCALIB_SUCARRY_Msk;
            CTSUSUCLK0          =
                (uint16_t) (((g_ctsu_correction_info.scan_index - 3) * CTSU_CORRECTION_SUMULTI) - 1);
        }

        CTSUCRAH  |= CTSUCRA_SDPSEL_Msk;
    }
    else
    {
        /* Step2-b : Measure the current input to the ICO by passing current through the external resistance. */
        /*           The theoretical value of the current is 9.375uA. */
        CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
        CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
 #if (CTSU_CFG_TEMP_CORRECTION_TS < 16)
        CTSUCHACAL = (uint16_t) (1 << CTSU_CFG_TEMP_CORRECTION_TS);
  #if BSP_MCU_GROUP_RL78F22
        /* nothing */
  #elif BSP_MCU_GROUP_RL78F25
        CTSUCHACAH = 0;
  #else
        CTSUCHACAH = 0;
        CTSUCHACBL = 0;
  #endif
 #elif (CTSU_CFG_TEMP_CORRECTION_TS >= 16) && (CTSU_CFG_TEMP_CORRECTION_TS < 32)
        CTSUCHACAL = 0;
  #if BSP_MCU_GROUP_RL78F22
        /* nothing */
  #elif BSP_MCU_GROUP_RL78F25
        CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
  #else
        CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
        CTSUCHACBL = 0;
  #endif
 #else
        CTSUCHACAL = 0;
        CTSUCHACAH = 0;
        CTSUCHACBL = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 32));
 #endif
        CTSUCRAH |= CTSUCRA_DCMODE_Msk;
        CTSUCRAH |= CTSUCRA_DCBACK_Msk;
        CTSUSO0      = 0;
    }
}

/***********************************************************************************************************************
 * ctsu_correction_data_get
 ***********************************************************************************************************************/
void ctsu_correction_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data)
{
    uint32_t    i;
    uint32_t    j;
    uint16_t    x0;
    uint16_t    x1;
    uint16_t    y0;
    uint16_t    y1;
    uint16_t    dac2vdc;
    uint16_t    dac_value;
    uint32_t    ref_value;
    uint16_t    c0;
    uint16_t    c1;
    ctsu_data_t temp_avg_data = {0, 0};

    if (g_ctsu_correction_info.scan_index < CTSU_CORRECTION_POINT_NUM)
    {
        temp_avg_data.int_data = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][g_ctsu_correction_info.scan_index];
        ctsu_moving_average(&temp_avg_data, *p_instance_ctrl->p_self_raw, 4);
        g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][g_ctsu_correction_info.scan_index] = temp_avg_data.int_data;
        g_ctsu_correction_info.scan_index++;
    }
    else
    {
        if (0 == g_ctsu_correction_info.ex_base_value)
        {
            g_ctsu_correction_info.ex_base_value  = *p_instance_ctrl->p_self_raw;
            g_ctsu_correction_info.update_counter = CTSU_CFG_TEMP_CORRECTION_TIME;
        }
        else
        {
            temp_avg_data.int_data = g_ctsu_correction_info.ex_base_value;
            ctsu_moving_average(&temp_avg_data, *p_instance_ctrl->p_self_raw, 4);
            g_ctsu_correction_info.ex_base_value = temp_avg_data.int_data;
        }

        g_ctsu_correction_info.scan_index = 0;
        g_ctsu_correction_info.update_counter++;
    }

    /* Step4 : Calculate the coefficient between step2 and step3. */
    if (g_ctsu_correction_info.update_counter > CTSU_CFG_TEMP_CORRECTION_TIME)
    {
        // Theoretical value at VDC path measurement
        x1 =
            (uint16_t) (((uint32_t) CTSU_CORRECTION_STD_VAL << 9) /
                        g_ctsu_correction_info.error_rate[CTSU_RANGE_160UA]);

        j = x1 / CTSU_CORRECTION_STD_UNIT;              // upper index of theoretical value

        x0 = (uint16_t) (CTSU_CORRECTION_STD_UNIT * j); // lower side Theoretical value
        y0 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][j - 1];
        y1 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][j];

        // Calculate CCODAC path value on VDC path current by linear interpolation
        dac2vdc = (uint16_t) (y0 + ((uint32_t) (y1 - y0) * (x1 - x0)) / CTSU_CORRECTION_STD_UNIT);

        // calculate VDC path conversion ratio
        dac2vdc =
            (uint16_t) (((int32_t) g_ctsu_correction_info.base_value[CTSU_RANGE_160UA] <<
                         CTSU_CORRECTION_DIV_PRECISION) /
                        dac2vdc);

        // Correct CCODAC path value using conversion ratio, and make correction coefficients
        for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
        {
            dac_value =
                (uint16_t) (((uint32_t) g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][j] * dac2vdc) >>
                            CTSU_CORRECTION_DIV_PRECISION);
            ref_value = (CTSU_CORRECTION_STD_UNIT * (j + 1));

            g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][j] = dac_value;
            g_ctsu_correction_info.coef[CTSU_RANGE_160UA][j]      =
                (uint16_t) ((ref_value << CTSU_CORRECTION_DIV_PRECISION) / dac_value);
        }

        // expand correction range
        c0 = g_ctsu_correction_info.coef[CTSU_RANGE_160UA][0];
        c1 = g_ctsu_correction_info.coef[CTSU_RANGE_160UA][1];
        x0 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][0];
        x1 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][1];

        g_ctsu_correction_info.coef[CTSU_RANGE_160UA][0] =
            (uint16_t) ((int32_t) c1 - (((int32_t) c1 - (int32_t) c0) * x1) / (x1 - x0));
        g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][0] = 0;

        c0 = g_ctsu_correction_info.coef[CTSU_RANGE_160UA][10];
        c1 = g_ctsu_correction_info.coef[CTSU_RANGE_160UA][11];
        x0 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][10];
        x1 = g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][11];

        g_ctsu_correction_info.coef[CTSU_RANGE_160UA][11] =
            (uint16_t) (c1 + (((int32_t) c1 - (int32_t) c0) * (CTSU_COUNT_MAX - x1)) / (x1 - x0));
        g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][11] = CTSU_COUNT_MAX;

        for (j = 1; j < (CTSU_CORRECTION_POINT_NUM - 1); j++)
        {
            for (i = 0; i < (CTSU_RANGE_NUM - 1); i++)
            {
                g_ctsu_correction_info.dac_value[i][j] =
                    (uint16_t) (((uint32_t) g_ctsu_correction_info.dac_value[CTSU_RANGE_160UA][j] *
                                 (uint32_t) g_ctsu_correction_info.range_ratio[i]) >> CTSU_SHIFT_AMOUNT);
                g_ctsu_correction_info.coef[i][j] =
                    (uint16_t) (((uint32_t) g_ctsu_correction_info.coef[CTSU_RANGE_160UA][j] *
                                 (uint32_t) g_ctsu_correction_info.range_ratio[i]) >> CTSU_SHIFT_AMOUNT);
            }
        }

  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
        ctsu_auto_correction_register_set(p_instance_ctrl);
  #endif

        g_ctsu_correction_info.update_counter = 0;
  #if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
        ctsu_correction_calib_rtrim(p_instance_ctrl, p_data);
  #endif
    }
    else
    {
        /* Indicates that ADC measurement was not performed. */
        *p_data = CTSU_COUNT_MAX;
    }
}

  #if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)

/***********************************************************************************************************************
 * ctsu_correction_calib_rtrim
 ***********************************************************************************************************************/
void ctsu_correction_calib_rtrim (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data)
{
    uint16_t               i;
    uint16_t               adctdr_ave;
    uint32_t               adctdr_sum;
    int16_t                diff;
    int16_t                dir  = 0;
    uint16_t               comp = 0;

    ctsu_adc_init();

    /* Self single scan mode */
    CTSUCRAH |= 1 << CTSUCRA_LOAD_Pos;
    CTSUCRAL &= ~CTSUCRA_MD0_Msk;
    CTSUCRAL &= ~CTSUCRA_MD1_Msk;
    CTSUCRAH &= ~CTSUCRA_MD2_Msk;

    /* Set Channel */
   #if (CTSU_CFG_TEMP_CORRECTION_TS < 16)
      CTSUCHACAL = (uint16_t) (1 << CTSU_CFG_TEMP_CORRECTION_TS);
    #if BSP_MCU_GROUP_RL78F22
        /* nothing */
    #elif BSP_MCU_GROUP_RL78F25
      CTSUCHACAH = 0;
    #else
      CTSUCHACAH = 0;
      CTSUCHACBL = 0;
    #endif
   #elif (CTSU_CFG_TEMP_CORRECTION_TS >= 16) && (CTSU_CFG_TEMP_CORRECTION_TS < 32)
      CTSUCHACAL = 0;
    #if BSP_MCU_GROUP_RL78F22
        /* nothing */
    #elif BSP_MCU_GROUP_RL78F25
      CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
    #else
      CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
      CTSUCHACBL = 0;
    #endif
   #else
      CTSUCHACAL = 0;
    #if BSP_MCU_GROUP_RL78F22
        /* nothing */
    #elif BSP_MCU_GROUP_RL78F25
      CTSUCHACAH = 0;
    #else
      CTSUCHACAH = 0;
      CTSUCHACBL = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 32));
    #endif
   #endif
      CTSUMCH0 = CTSU_CFG_TEMP_CORRECTION_TS;

    /* 150uA current measurement */
    CTSUCRAH |= CTSUCRA_DCMODE_Msk;
    CTSUCRAH |= CTSUCRA_DCBACK_Msk;
    CTSUSO0  = CTSU_CALIB_CTSUSO;
    CTSUCRAL &= ~CTSUCRA_CSW_Msk;
    CTSUDBGR0 |= CTSUCALIB_DRV_Msk;

    /* ADC scan */
    while (!comp)
    {
        adctdr_sum = 0;
        for (i = 0; i < CTSU_CALIB_AVERAGE_TIME; i++)
        {
   #if BSP_MCU_GROUP_RL78G22
            ADCE = 1U;
            ctsu_delay(1);
            ADIF = 0;
   #endif
   #if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
            ADWINR = 0x00U;
            ADCSR |= 0x8000U;
   #else
            ADCS = 1;
   #endif

            /* Wait for conversion to complete. */
   #if BSP_MCU_GROUP_RL78G22
            while (0 == ADIF)
            {
            }
   #elif (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
            ADWINR = 0x00U;

            while (0 != (ADCSR & 0x8000U))
            {
            }
   #else
            while (1 == ADCS)
            {
            }
   #endif

   #if BSP_MCU_GROUP_RL78G22
            adctdr_sum += ((ADCR >> 6) & 0x3FF);
   #elif (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
            ADWINR      = 0x05U;
            adctdr_sum += ADDR29;
   #else
            adctdr_sum += ADCR;
   #endif
        }

        adctdr_ave = (uint16_t) ((adctdr_sum * 10) / CTSU_CALIB_AVERAGE_TIME);
        diff       = (int16_t) ((adctdr_ave - CTSU_CALIB_REF) / 10);

        /* The change unit of the voltage by the RTRIM register is about 4mV (4096 * 4) = 16.384 */
        if (diff > CTSU_CALIB_THRESHOLD)
        {
            if (0 <= dir)
            {
                RTRIM = RTRIM + 1;
                dir = 1;
            }
            else
            {
                comp = 1;
            }
        }
        else if (diff < -(CTSU_CALIB_THRESHOLD))
        {
            if (0 >= dir)
            {
                RTRIM = RTRIM - 1;
                dir = -1;
            }
            else
            {
                comp = 1;
            }
        }
        else
        {
            comp = 1;
        }
    }

    /* Restore register settings */
    CTSUDBGR0 &= ~CTSUCALIB_DRV_Msk;
    CTSUCRAL |= CTSUCRA_CSW_Msk;
    CTSUCRAH &= ~CTSUCRA_DCMODE_Msk;
    CTSUCRAH &= ~CTSUCRA_DCBACK_Msk;
    
    /* Indicates that ADC measurement was performed  */
    *p_data = RTRIM;
}

  #endif
 #endif
#endif

#if ((CTSU_CFG_AUTO_CORRECTION_ENABLE == 0) || (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1))

/***********************************************************************************************************************
 * ctsu_correction_calc
 ***********************************************************************************************************************/
void ctsu_correction_calc (uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc)
{
    uint32_t answer = 0;
    uint32_t cmp_data;
    uint8_t  calc_flag = 0;
 #if (BSP_FEATURE_CTSU_VERSION == 1)
    uint16_t diff_val;
    int32_t  diff_coefficient;
    int32_t  mul_diffcoff_diff1valsval;
    uint32_t mul_coff1val_diffcorr;
    uint16_t coefficient;
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 2)
    int32_t  y0 = 0;
    int32_t  y1 = 0;
    int32_t  x0 = 0;
    int32_t  x1 = 0;
    uint16_t i;
    uint32_t scaled_data;
    uint32_t coef;
    bool     flag;
 #endif

 #if (BSP_FEATURE_CTSU_VERSION == 2)
    if (CTSU_CORRECTION_COMPLETE == g_ctsu_correction_info.status)
    {
        calc_flag = 1;
    }
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 1)
    if (CTSU_CORRECTION_COMPLETE == g_ctsu_correction_info.status)
    {
        calc_flag = 1;
    }
 #endif

    if (calc_flag)
    {
 #if (BSP_FEATURE_CTSU_VERSION == 1)

        /* Since the correction coefficient table is created with the recommended measurement time, */
        /* If the measurement time is different, adjust the value level. */
        cmp_data = (uint32_t) ((raw_data * g_ctsu_correction_info.ctsu_clock) /
                               (uint32_t) ((p_calc->snum + 1) * (p_calc->sdpa + 1)));

        /*               g_mul_coff1val_diffcorr - g_diff_cofficient * (g_ctsu_correction_info.first_val - raw_data) */
        /*  coefficient= ------------------------------------------------------------------------------------------  */
        /*                                      g_diff_correct_val                                                   */
        /*                                                                                                           */

        diff_val = (uint16_t) (g_ctsu_correction_info.first_val - g_ctsu_correction_info.second_val);

        /* Get multiplication of g_ctsu_correction_info.first_coefficient and difference of Correction value */
        mul_coff1val_diffcorr = g_ctsu_correction_info.first_coefficient * diff_val;

        /* Get difference of Correction coefficient */
        diff_coefficient =
            (int32_t) (g_ctsu_correction_info.first_coefficient - g_ctsu_correction_info.second_coefficient);

        /* Get multiplication of  g_diff_cofficient  and (g_ctsu_correction_info.first_val - raw_data_coff) */
        mul_diffcoff_diff1valsval = (diff_coefficient * (int32_t) (g_ctsu_correction_info.first_val - cmp_data));

        /* Get correction coefficient of scan data */
        coefficient = (uint16_t) (((int32_t) mul_coff1val_diffcorr - mul_diffcoff_diff1valsval) / diff_val);

        /* Get output count data */
        answer = (uint32_t) (((uint32_t) raw_data * (uint32_t) coefficient) >> CTSU_SHIFT_AMOUNT);
 #endif
 #if (BSP_FEATURE_CTSU_VERSION == 2)

        // Normalization measurement data to recommended measurement timed
        cmp_data = (uint32_t) raw_data * (CTSU_SNUM_RECOMMEND + 1);

        /* searching for upper index of measurement value containing raw_data by scaled measurement value   */
        /* if without scaling, should use binary search                                                     */
        /* index search must be stopped when i = 11 because dac_value[11] = 65535                           */

        if (CTSU_MODE_MUTUAL_CFC_SCAN != p_calc->md)
        {
            for (i = 1; i < CTSU_CORRECTION_POINT_NUM - 1; i++)
            {
                scaled_data = (uint32_t) (g_ctsu_correction_info.dac_value[p_calc->range][i]) * (p_calc->snum + 1);
                if (scaled_data > cmp_data)
                {
                    break;
                }
            }

            x0 = g_ctsu_correction_info.dac_value[p_calc->range][i - 1];
            x1 = g_ctsu_correction_info.dac_value[p_calc->range][i];

            // coef
            y0 = g_ctsu_correction_info.coef[p_calc->range][i - 1];
            y1 = g_ctsu_correction_info.coef[p_calc->range][i];

            x0 = (x0 * (p_calc->snum + 1)) / (CTSU_SNUM_RECOMMEND + 1);
            x1 = (x1 * (p_calc->snum + 1)) / (CTSU_SNUM_RECOMMEND + 1);
            {
                coef = (uint32_t) (((y1 - y0) * (raw_data - x0)) / (x1 - x0) + y0);
                flag = false;

                if (p_calc->snum < CTSU_SNUM_RECOMMEND) // 1st condition
                {
                    if (raw_data < coef)
                    {
                        answer = (coef >> CTSU_CORRECTION_DIV_PRECISION) * raw_data;
                    }
                    else
                    {
                        answer = coef * (raw_data >> CTSU_CORRECTION_DIV_PRECISION);
                    }

                    if (answer > CTSU_COUNT_MAX) // 2nd condition
                    {
                        flag   = true;
                        answer = CTSU_COUNT_MAX;
                        g_ctsu_correction_info.calculation_error = 1;
                    }
                }

                if (false == flag)
                {
                    answer = (uint32_t) (coef * raw_data) >> CTSU_CORRECTION_DIV_PRECISION;
                }
            }
        }
 #endif

        /* Value Overflow Check */
        if (CTSU_COUNT_MAX < answer)
        {
            *correction_data = CTSU_COUNT_MAX;
            g_ctsu_correction_info.calculation_error = 1;
        }
        else
        {
            *correction_data = (uint16_t) answer;
        }
    }
    else
    {
        *correction_data = raw_data;
    }
}

#endif

/***********************************************************************************************************************
 * ctsu_correction_exec
 ***********************************************************************************************************************/
void ctsu_correction_exec (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
#if (BSP_FEATURE_CTSU_VERSION == 1)
    ctsu_correction_ctsu1_exec(p_instance_ctrl);
#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
    ctsu_correction_ctsu2_exec(p_instance_ctrl);
#endif
}

#if (BSP_FEATURE_CTSU_VERSION == 1)
void ctsu_correction_ctsu1_exec (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;

    ctsu_correction_calc_t calc;
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    ctsu_data_t * p_self_data;
    ctsu_data_t   average_self = {0, 0};
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    ctsu_data_t * p_pri_data;
    ctsu_data_t * p_snd_data;
    ctsu_data_t   average_pri = {0, 0};
    ctsu_data_t   average_snd = {0, 0};
 #endif

    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        calc.snum = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso0 >> 10) & CTSU_SNUM_MAX;
        calc.sdpa = (p_instance_ctrl->p_ctsuwr[element_id].ctsuso1 >> 8) & CTSU_SDPA_MAX;
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
            p_self_data  = (p_instance_ctrl->p_self_data + element_id);
            average_self = *p_self_data;
            ctsu_correction_calc((p_instance_ctrl->p_self_corr + element_id),
                                 (p_instance_ctrl->p_self_raw + element_id)->sen,
                                 &calc);
            p_self_data->int_data = *(p_instance_ctrl->p_self_corr + element_id);
            if (1 < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_self, p_self_data->int_data, p_instance_ctrl->average);
                *p_self_data = average_self;
            }
        }
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        p_pri_data = (p_instance_ctrl->p_mutual_pri_data + element_id);
        p_snd_data = (p_instance_ctrl->p_mutual_snd_data + element_id);
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
            average_pri = *p_pri_data;
            average_snd = *p_snd_data;
            ctsu_correction_calc((p_instance_ctrl->p_mutual_pri_corr + element_id),
                                 (p_instance_ctrl->p_mutual_raw + element_id)->pri_sen, &calc);
            ctsu_correction_calc((p_instance_ctrl->p_mutual_snd_corr + element_id),
                                 (p_instance_ctrl->p_mutual_raw + element_id)->snd_sen, &calc);
            p_pri_data->int_data = *(p_instance_ctrl->p_mutual_pri_corr + element_id);
            p_snd_data->int_data = *(p_instance_ctrl->p_mutual_snd_corr + element_id);

            if (1 < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_pri, p_pri_data->int_data, p_instance_ctrl->average);
                ctsu_moving_average(&average_snd, p_snd_data->int_data, p_instance_ctrl->average);
                *p_pri_data = average_pri;
                *p_snd_data = average_snd;
            }
        }
 #endif
    }
}

#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
void ctsu_correction_ctsu2_exec (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t i;
    uint16_t element_id;
    uint16_t majority_mode_element_num;
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
    ctsu_correction_calc_t calc;
    uint32_t               ctsuso;
    uint32_t               snum;
    uint32_t               so_value[CTSU_CFG_NUM_SUMULTI];
 #else
    uint16_t j;
 #endif

 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    uint16_t    * p_self_corr;
    uint16_t    * p_self_mfc;
    ctsu_data_t * p_self_data;
    ctsu_data_t   average_self;
 #endif
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    uint16_t    * p_pri_corr;
    uint16_t    * p_snd_corr;
    uint16_t    * p_pri_mfc;
    uint16_t    * p_snd_mfc;
    ctsu_data_t * p_pri_data;
    ctsu_data_t * p_snd_data;
    ctsu_data_t   average_pri;
    ctsu_data_t   average_snd;
 #endif
 #if (CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE)
    if (1 == p_instance_ctrl->p_ctsu_cfg->majority_mode)
    {
        majority_mode_element_num = CTSU_MAJORITY_MODE_ELEMENTS;
    }
    else
 #endif
    {
        majority_mode_element_num = 1;
    }

 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
    calc.range = p_instance_ctrl->range;
    calc.md    = p_instance_ctrl->md;
 #endif

    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
        calc.snum = (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI)].ctsuso >> 10) & CTSU_SNUM_MAX;
        for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
        {
            ctsuso =
                (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI) + i].ctsuso & CTSU_TUNING_MAX);
            snum = (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI)].ctsuso >> 10) &
                   CTSU_SNUM_MAX;
            so_value[i] = (ctsuso * CTSU_CORRECTION_OFFSET_UNIT * (snum + 1)) >> calc.range;
        }
 #endif

        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
        {
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            p_self_corr = p_instance_ctrl->p_self_corr + (element_id * CTSU_CFG_NUM_SUMULTI);
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 0)
                ctsu_correction_calc(&p_self_corr[i],
                                     p_instance_ctrl->p_self_raw[(element_id * CTSU_CFG_NUM_SUMULTI) + i], &calc);
  #else
                *(p_self_corr + i) =
                    p_instance_ctrl->p_self_raw[(element_id * CTSU_CFG_NUM_SUMULTI) + i];
  #endif
            }

            for (i = 0; i < majority_mode_element_num; i++)
            {
                if (0 != p_instance_ctrl->average)
                {
                    p_self_data = (p_instance_ctrl->p_self_data + (element_id * majority_mode_element_num) + i);

                    /* Store last moving averaged data */
                    average_self.int_data           = p_self_data->int_data;
                    average_self.decimal_point_data = p_self_data->decimal_point_data;
  #if (CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE)
                    if (p_instance_ctrl->p_ctsu_cfg->majority_mode == 1)
                    {
                        /* Skip the ctsu_correction_multi at Software JMM */
                        p_self_data->int_data = p_self_corr[i];
                    }
                    else
  #endif
                    {
                        p_self_mfc = (p_instance_ctrl->p_self_mfc + (element_id * CTSU_CFG_NUM_SUMULTI));

                        /* Matching values */
  #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
                        ctsu_correction_freq(p_self_corr, so_value, p_self_mfc);
  #else
                        for (j = 0; j < CTSU_CFG_NUM_SUMULTI; j++)
                        {
                            p_self_mfc[j] = p_self_corr[j];
                        }
  #endif
                        *(p_instance_ctrl->p_selected_freq_self + element_id) =
                            ctsu_correction_multi(p_self_mfc, NULL, &(p_self_data->int_data), NULL);
                    }

                    /* Update moving averaged data */
                    ctsu_moving_average(&average_self, p_self_data->int_data, p_instance_ctrl->average);
                    *p_self_data = average_self;
                }
            }
        }
        else if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->md)
        {
            p_self_data = (p_instance_ctrl->p_self_data + (element_id * CTSU_CFG_NUM_SUMULTI));

            /* Store last moving averaged data */
            average_self = *p_self_data;

            /* Correction */
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 0)
            ctsu_correction_calc(&(p_self_data->int_data), p_instance_ctrl->p_self_raw[element_id], &calc);
  #else
            p_self_data->int_data = p_instance_ctrl->p_self_raw[element_id];
  #endif

            /* Update moving averaged data */
            if (1 < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_self, p_self_data->int_data, p_instance_ctrl->average);
                *p_self_data = average_self;
            }
 #endif
        }
        else if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
        {
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            p_pri_corr = p_instance_ctrl->p_mutual_pri_corr + (element_id * CTSU_CFG_NUM_SUMULTI);
            p_snd_corr = p_instance_ctrl->p_mutual_snd_corr + (element_id * CTSU_CFG_NUM_SUMULTI);
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                if (true == p_instance_ctrl->serial_tuning_enable)
                {
                    if ((0 == p_instance_ctrl->ctsuchtrc0) && (0 == p_instance_ctrl->ctsuchtrc1) &&
                        (0 == p_instance_ctrl->ctsuchtrc2) && (0 == p_instance_ctrl->ctsuchtrc3) &&
                        (0 == p_instance_ctrl->ctsuchtrc4))
                    {
                        /* Serial tuning Phase1 */
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 0)
                        ctsu_correction_calc(&p_instance_ctrl->p_mutual_pri_corr[element_id],
                                             p_instance_ctrl->p_mutual_raw[element_id * CTSU_CFG_NUM_SUMULTI],
                                             &calc);
  #else
                        p_instance_ctrl->p_mutual_pri_corr[element_id] =
                            p_instance_ctrl->p_mutual_raw[element_id * CTSU_CFG_NUM_SUMULTI];
  #endif
                        break;
                    }
                }

  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 0)
                ctsu_correction_calc(&p_pri_corr[i],
                                     p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2)],
                                     &calc);
  #else
                p_pri_corr[i] =
                    p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2)];
  #endif

  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 0)
                ctsu_correction_calc(&p_snd_corr[i],
                                     p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2) + 1],
                                     &calc);
  #else
                p_snd_corr[i] =
                    p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2) + 1];
  #endif
            }

            for (i = 0; i < majority_mode_element_num; i++)
            {
                if (true == p_instance_ctrl->serial_tuning_enable)
                {
                    /* Serial Tuning Phase1 */
                    if ((0 == p_instance_ctrl->ctsuchtrc0) && (0 == p_instance_ctrl->ctsuchtrc1) &&
                        (0 == p_instance_ctrl->ctsuchtrc2) && (0 == p_instance_ctrl->ctsuchtrc3) &&
                        (0 == p_instance_ctrl->ctsuchtrc4))
                    {
                        break;
                    }
                }

                if (0 != p_instance_ctrl->average)
                {
                    p_pri_data = (p_instance_ctrl->p_mutual_pri_data + (element_id * majority_mode_element_num) + i);
                    p_snd_data = (p_instance_ctrl->p_mutual_snd_data + (element_id * majority_mode_element_num) + i);

                    /* Store last moving averaged data */
                    average_pri = *p_pri_data;
                    average_snd = *p_snd_data;
  #if (CTSU_CFG_MAJORITY_MODE & CTSU_JUDGEMENT_MAJORITY_MODE)
                    if (p_instance_ctrl->p_ctsu_cfg->majority_mode == 1)
                    {
                        /* Skip the ctsu_correction_multi at Software JMM */
                        p_pri_data->int_data = p_pri_corr[i];
                        p_snd_data->int_data = p_snd_corr[i];
                    }
                    else
  #endif
                    {
                        p_pri_mfc = (p_instance_ctrl->p_mutual_pri_mfc + (element_id * CTSU_CFG_NUM_SUMULTI));
                        p_snd_mfc = (p_instance_ctrl->p_mutual_snd_mfc + (element_id * CTSU_CFG_NUM_SUMULTI));

                        /* Matching values */
  #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)
                        ctsu_correction_freq(p_pri_corr, so_value, p_pri_mfc);
                        ctsu_correction_freq(p_snd_corr, so_value, p_snd_mfc);
  #else
                        for (j = 0; j < CTSU_CFG_NUM_SUMULTI; j++)
                        {
                            p_pri_mfc[j] = p_pri_corr[j];
                            p_snd_mfc[j] = p_snd_corr[j];
                        }
  #endif
                        *(p_instance_ctrl->p_selected_freq_mutual + element_id) =
                            ctsu_correction_multi(p_pri_mfc, p_snd_mfc, &(p_pri_data->int_data),
                                                  &(p_snd_data->int_data));
                    }

                    /* Update moving averaged data */
                    ctsu_moving_average(&average_pri, p_pri_data->int_data, p_instance_ctrl->average);
                    *p_pri_data = average_pri;
                    ctsu_moving_average(&average_snd, p_snd_data->int_data, p_instance_ctrl->average);
                    *p_snd_data = average_snd;
                }
            }
 #endif
        }
        else
        {
        }
    }
}

#endif
#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 0)

/***********************************************************************************************************************
 * ctsu_correction_freq
 ***********************************************************************************************************************/
void ctsu_correction_freq (uint16_t * p_corr, uint32_t * p_so_value, uint16_t * p_mfc)
{
    uint32_t i;
    uint32_t sumulti[CTSU_CFG_NUM_SUMULTI];
    int32_t  calc[CTSU_CFG_NUM_SUMULTI];
    uint32_t total;

    sumulti[0] = CTSU_CFG_SUMULTI0 + 1;
  #if CTSU_CFG_NUM_SUMULTI >= 2
    sumulti[1] = CTSU_CFG_SUMULTI1 + 1;
  #endif
  #if CTSU_CFG_NUM_SUMULTI >= 3
    sumulti[2] = CTSU_CFG_SUMULTI2 + 1;
  #endif

    p_mfc[0] = p_corr[0];

    for (i = 1; i < CTSU_CFG_NUM_SUMULTI; i++)
    {
        total = (((uint32_t) p_corr[i] * (CTSU_SNUM_RECOMMEND + 1) + p_so_value[i]) * sumulti[0]) / sumulti[i];

        if (total > p_so_value[0])
        {
            calc[i] = (int32_t) ((total - p_so_value[0]) / (CTSU_SNUM_RECOMMEND + 1));
            if (calc[i] < CTSU_COUNT_MAX)
            {
                p_mfc[i] = (uint16_t) calc[i];
            }
            else
            {
                /* over flow */
                p_mfc[i] = CTSU_COUNT_MAX;
            }
        }
        else
        {
            /* under flow */
            calc[i]  = 0;
            p_mfc[i] = 0;
        }
    }
}

 #endif

/***********************************************************************************************************************
 * ctsu_correction_multi
 ***********************************************************************************************************************/
uint8_t ctsu_correction_multi (uint16_t * p_pri_mfc, uint16_t * p_snd_mfc, uint16_t * p_pri_data, uint16_t * p_snd_data)
{
    uint8_t selected_freq;
    int32_t add_pri;
    int32_t add_snd;
 #if CTSU_CFG_NUM_SUMULTI >= 3
    uint32_t i;
    int32_t  pri_calc[CTSU_CFG_NUM_SUMULTI];
    int32_t  snd_calc[CTSU_CFG_NUM_SUMULTI];
    int32_t  diff[CTSU_CFG_NUM_SUMULTI];
 #endif

 #if CTSU_CFG_NUM_SUMULTI == 1
    add_pri = p_pri_mfc[0];
    if (NULL != p_snd_mfc)
    {
        add_snd = p_snd_mfc[0];
    }
 #endif
 #if CTSU_CFG_NUM_SUMULTI == 2
    add_pri = p_pri_mfc[0] + p_pri_mfc[1];
    if (NULL != p_snd_mfc)
    {
        add_snd = p_snd_mfc[0] + p_snd_mfc[1];
    }
 #endif
 #if CTSU_CFG_NUM_SUMULTI >= 3
    for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
    {
        pri_calc[i] = (int32_t) p_pri_mfc[i];
        if (NULL == p_snd_mfc)
        {
            snd_calc[i] = 0;
        }
        else
        {
            snd_calc[i] = (int32_t) p_snd_mfc[i];
        }
    }

    diff[0] = (snd_calc[1] - pri_calc[1]) - (snd_calc[0] - pri_calc[0]);
    diff[1] = (snd_calc[2] - pri_calc[2]) - (snd_calc[0] - pri_calc[0]);
    diff[2] = (snd_calc[2] - pri_calc[2]) - (snd_calc[1] - pri_calc[1]);

    for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
    {
        if (diff[i] < 0)
        {
            diff[i] = -diff[i];
        }
    }

    /* Normally select freq0 and freq1 addition */
    /* If the following conditions are false, it is estimated to be a noise environment. */
    /* Compare with the combination with the other frequency difference (including margin). */
    if ((diff[0] < (diff[1] * 2)) && (diff[0] < ((diff[2] * 3) / 2)))
    {
        add_pri = pri_calc[0] + pri_calc[1];
        add_snd = snd_calc[0] + snd_calc[1];

        selected_freq = 0x3;
    }
    else
    {
        if (diff[1] < diff[2])
        {
            add_pri = pri_calc[0] + pri_calc[2];
            add_snd = snd_calc[0] + snd_calc[2];

            selected_freq = 0x5;
        }
        else
        {
            add_pri = pri_calc[1] + pri_calc[2];
            add_snd = snd_calc[1] + snd_calc[2];

            selected_freq = 0x6;
        }
    }
 #endif

    if (CTSU_COUNT_MAX < add_pri)
    {
        *p_pri_data = CTSU_COUNT_MAX;
    }
    else
    {
        *p_pri_data = (uint16_t) add_pri;
    }

    if (NULL != p_snd_data)
    {
        if (CTSU_COUNT_MAX < add_snd)
        {
            *p_snd_data = CTSU_COUNT_MAX;
        }
        else
        {
            *p_snd_data = (uint16_t) add_snd;
        }
    }

    return selected_freq;
}
#endif

/***********************************************************************************************************************
 * ctsu_delay
 ***********************************************************************************************************************/
void ctsu_delay(uint16_t delay_us)
{
    uint32_t i;
    uint32_t loops_required;
    uint16_t clock_mhz;

    clock_mhz = (uint16_t)(R_BSP_GetFclkFreqHz() / 1000000);
    if (0 == clock_mhz)
    {
        clock_mhz = 1;
    }

    loops_required = ((uint32_t)delay_us * (uint32_t)clock_mhz);
    loops_required /= 20;
    for (i = 0; i < loops_required; i++)
    {
        BSP_NOP();
    }
}

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_SMS_SUPPORT_ENABLE == 1)
/***********************************************************************************************************************
* Function Name: ctsu_sms_init
* Description  : This function initializes the SMS for CTSU.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ctsu_sms_init (ctsu_instance_ctrl_t * const p_instance_ctrl,
                    uint16_t * p_threshold,
                    uint16_t * p_hysteresis,
                    uint16_t count_filter)
{
    uint16_t i;
    uint16_t * dst = (uint16_t *) 0x0380; /* Address of SMSI0 */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        static const uint16_t iregs[SIZEOF_sms_init_program] = {
                sms_init_program
        };
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        static const uint16_t iregs_mutual[SIZEOF_sms_init_mutual_program] = {
                sms_init_mutual_program
        };
  #endif

    CTSUFNMK = 1;

    p_instance_ctrl->sms = 1;

    ctsu_sms_work_init(p_instance_ctrl, count_filter);

    ctsu_sms_parameter_init(p_instance_ctrl, p_threshold, p_hysteresis, count_filter);

    ctsu_sms_correction_data_copy(p_instance_ctrl);

    ctsu_sms_program_setup(p_instance_ctrl);

    SMSEN = 1U;

  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        /* Copy the SMS instruction */
        for (i = 0; i < SIZEOF_sms_init_program; ++i) {
            *dst = iregs[i];
            ++dst;
        }
        /* Copy the SMS General */
        dst = (uint16_t *) 0x03C0; /* Address of SMSG0 */
        for (i = 0; i < CTSU_SMS_GREG_SIZE; ++i) {
            *dst = g_ctsu_sms_greg[0][i];
            ++dst;
        }
    }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        /* Copy the SMS instruction */
        for (i = 0; i < SIZEOF_sms_init_mutual_program; ++i) {
            *dst = iregs_mutual[i];
            ++dst;
        }
        /* Copy the SMS General */
        dst = (uint16_t *) 0x03C0; /* Address of SMSG0 */
        for (i = 0; i < CTSU_SMS_GREG_SIZE; ++i) {
            *dst = g_ctsu_sms_mutual_greg[0][i];
            ++dst;
        }
    }
  #endif

  #if BSP_MCU_GROUP_RL78L23
    SMSC |= 0x0B;    /* SMS start trigger is INTCTSURD */
  #else
    SMSC |= 0x05;    /* SMS start trigger is ELC output */
  #endif
}

/***********************************************************************************************************************
* Function Name: ctsu_sms_start
* Description  : This function starts SMS module operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ctsu_sms_start(void)
{
    SMSEMK = 1U; /* disable INTSMSE interrupt */
    SMSEIF = 1U; /* set INTSMSE interrupt flag */
    SMSSTART = 1U;
}

/***********************************************************************************************************************
* Function Name: ctsu_sms_stop
* Description  : This function stops SMS module operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ctsu_sms_stop(void)
{
    SMSSTOP = 1;
    SMSEMK = 1U; /* disable INTSMSE interrupt */
    SMSEIF = 0U; /* clear INTSMSE interrupt flag */
}

/***********************************************************************************************************************
* ctsu_sms_work_init
***********************************************************************************************************************/
void ctsu_sms_work_init (ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t count_filter)
{
    uint16_t total_elements;
    uint16_t sms_num_moving_average;
    uint16_t sms_set_moving_average=0;

    total_elements = (uint16_t)(p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);

    sms_num_moving_average = (uint16_t)(p_instance_ctrl->num_moving_average >>1);
    while ( sms_num_moving_average != 0)
    {
        sms_num_moving_average = sms_num_moving_average >>1;
        sms_set_moving_average++;
    }

    /* Set Work RAM Parameter Work1 to Work8 */
    /* Work RAM1 */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        g_ctsu_sms_work[0].eb0 = 0x00;                              /* Auto judgement result (internal variable) */
        g_ctsu_sms_work[0].eb1 = CTSU_CFG_NUM_SUMULTI;              /* Set Multi Clock Measurement Count */
        g_ctsu_sms_work[0].eb2 = CTSU_SET_LOW(count_filter);        /* Set Continus Over Threshold Count for Upper */
        g_ctsu_sms_work[0].eb3 = CTSU_SET_HI(count_filter);         /* Set Continus Over Threshold Count for lower */
        g_ctsu_sms_work[0].eb4 = CTSU_CFG_NUM_SUMULTI -1;           /* Set Number of multiclocks above threshold */
        g_ctsu_sms_work[0].eb5 = sms_set_moving_average;            /* Set SMS Moving Average Count */
        g_ctsu_sms_work[0].eb6 = CTSU_SMS_REF_AVERAGE;              /* Set SMS BaseLine Average Count */
        g_ctsu_sms_work[0].eb7 = CTSU_CFG_NUM_SUMULTI;              /* For update the Multi Clock Judgement Counter */
    }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        g_ctsu_sms_work[0].eb0 = 0x00;                              /* Auto judgement result (internal variable) */
        g_ctsu_sms_work[0].eb1 = CTSU_CFG_NUM_SUMULTI *2;           /* Set Multi Clock Measurement Count, *2=P/S */
        g_ctsu_sms_work[0].eb2 = CTSU_SET_LOW(count_filter);        /* Set Continus Over Threshold Count for Upper */
        g_ctsu_sms_work[0].eb3 = CTSU_SET_HI(count_filter);         /* Set Continus Over Threshold Count for lower */
        g_ctsu_sms_work[0].eb4 = CTSU_CFG_NUM_SUMULTI -1;           /* Set Number of multiclocks above threshold */
        g_ctsu_sms_work[0].eb5 = sms_set_moving_average;            /* Set SMS Moving Average Count */
        g_ctsu_sms_work[0].eb6 = CTSU_SMS_REF_AVERAGE;              /* Set SMS BaseLine Average Count */
        g_ctsu_sms_work[0].eb7 = CTSU_CFG_NUM_SUMULTI *2;           /* For update the Multi Clock Judgement Counter, *2=P/S */
    }
  #endif

    /* Work RAM2 */
    g_ctsu_sms_work[1].eb0 = 0x00;                              /* immediate Value */
    g_ctsu_sms_work[1].eb1 = 0x00;                              /* immediate Value */
    g_ctsu_sms_work[1].eb2 = 0x01;                              /* immediate Value */
    g_ctsu_sms_work[1].eb3 = 0x00;                              /* immediate Value */
    g_ctsu_sms_work[1].eb4 = 0x00;                              /* immediate Value */
    g_ctsu_sms_work[1].eb5 = 0x80;                              /* immediate Value */
    g_ctsu_sms_work[1].eb6 = 0x00;                              /* Store Correction Result (immediate Value) */
    g_ctsu_sms_work[1].eb7 = 0x00;                              /* Store Correction Result (immediate Value) */

    /* Work RAM3 */
    g_ctsu_sms_work[2].eb0 = CTSU_SET_LOW(0x8000 >> CTSU_SMS_REF_AVERAGE); /* Baseline Counter add Value (internal variable) */
    g_ctsu_sms_work[2].eb1 = CTSU_SET_HI(0x8000 >> CTSU_SMS_REF_AVERAGE); /* Baseline Counter add Value (internal variable) */
    g_ctsu_sms_work[2].eb2 = 0x00;                                  /* Number of multiclocks above threshold */
    g_ctsu_sms_work[2].eb3 = 0x00;                                  /* Number of multiclocks above threshold */
    g_ctsu_sms_work[2].eb4 = CTSU_SET_LOW(&g_ctsu_sms_threshold);   /* Store Threshold Data address pointer */
    g_ctsu_sms_work[2].eb5 = CTSU_SET_HI(&g_ctsu_sms_threshold);    /* Store Threshold Data address pointer */
    g_ctsu_sms_work[2].eb6 = CTSU_SET_LOW(&g_ctsu_sms_result);      /* Store Judgement Data address pointer */
    g_ctsu_sms_work[2].eb7 = CTSU_SET_HI(&g_ctsu_sms_result);       /* Store Judgement Data address pointer */

    /* Work RAM4 */
    g_ctsu_sms_work[3].eb0 = 0x08;                          /* immediate Value */
    g_ctsu_sms_work[3].eb1 = 0x00;                          /* immediate Value */
    g_ctsu_sms_work[3].eb2 = 0x04;                          /* immediate Value */
    g_ctsu_sms_work[3].eb3 = 0x00;                          /* immediate Value */
    g_ctsu_sms_work[3].eb4 = CTSU_SET_LOW(total_elements);  /* Judgement number (tx_ch * rx_ch * multi) */
    g_ctsu_sms_work[3].eb5 = CTSU_SET_HI(total_elements);   /* Judgement number (tx_ch * rx_ch * multi) */
    g_ctsu_sms_work[3].eb6 = CTSU_SET_LOW(total_elements);  /* Update Judgement number (tx_ch * rx_ch * multi) */
    g_ctsu_sms_work[3].eb7 = CTSU_SET_HI(total_elements);   /* update Judgement number (tx_ch * rx_ch * multi) */

    /* Work RAM5 */
    g_ctsu_sms_work[4].eb0 = CTSU_SET_LOW(&CTSUSRL);        /* CTCUSRL Address pointer */
    g_ctsu_sms_work[4].eb1 = CTSU_SET_HI(&CTSUSRL);         /* CTSUSRL Address pointer */
    g_ctsu_sms_work[4].eb2 = CTSU_SET_LOW(&ITLS0);          /* ITLS0 Address pointer */
    g_ctsu_sms_work[4].eb3 = CTSU_SET_HI(&ITLS0);           /* ITLS0 Address pointer */
    g_ctsu_sms_work[4].eb4 = CTSU_SET_LOW(&CTSUCRAL);       /* CTSUCRAL Address pointer */
    g_ctsu_sms_work[4].eb5 = CTSU_SET_HI(&CTSUCRAL);        /* CTSUCRAL Address pointer */
    g_ctsu_sms_work[4].eb6 = 0x00;                          /* Write back MCH0 Value at the Snooze was finished (Only Single Scan) */
    g_ctsu_sms_work[4].eb7 = 0x00;                          /* Write back MCH1 Value at the Snooze was finished (Only Single Scan) */
  #if (CTSU_CFG_MULTIPLE_ELECTRODE_CONNECTION_ENABLE == 1)
    if ((1 == p_instance_ctrl->tsod) && (CTSU_CAP_EXTERNAL == p_instance_ctrl->cap))
    {
        g_ctsu_sms_work[4].eb6 = (p_instance_ctrl->mec_ts & CTSUMCH_MCH0_Msk);  /* Write back MCH0 Value at the Snooze was finished (Only Single Scan) */

        /* Set MCH1 when using both MEC and Active Shield.*/
        if((0 != p_instance_ctrl->ctsuchtrc0)
            || (0 != p_instance_ctrl->ctsuchtrc1)
            || (0 != p_instance_ctrl->ctsuchtrc2)
            || (0 != p_instance_ctrl->ctsuchtrc3)
            || (0 != p_instance_ctrl->ctsuchtrc4))
        {
        g_ctsu_sms_work[4].eb7 = (p_instance_ctrl->mec_shield_ts & (CTSUMCH_MCH1_Msk >> CTSUMCH_MCH1_Pos));
    }
    }
  #endif

    /* Work RAM6 */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
   #if BSP_MCU_GROUP_RL78L23
        g_ctsu_sms_work[5].eb0 = dtc_vectortable[48U];              /* DTC Vector table for SMS */
   #else
        g_ctsu_sms_work[5].eb0 = dtc_vectortable[38U];              /* DTC Vector table for SMS */
   #endif
        g_ctsu_sms_work[5].eb1 = DTCBAR;                            /* DTC Base Address */
        g_ctsu_sms_work[5].eb2 = CTSU_SET_LOW(&g_ctsu_sms_transfer);/* Store the SMS Program Address pointer */
        g_ctsu_sms_work[5].eb3 = CTSU_SET_HI(&g_ctsu_sms_transfer); /* Store the SMS Program Address pointer */
        g_ctsu_sms_work[5].eb4 = CTSU_SMS_PAGE_NUM_SELF;            /* DTC Translate for SMS (Set SMS Program Page Number) */
        g_ctsu_sms_work[5].eb5 = CTSU_SMS_PAGE_NUM_SELF;            /* DTC Translate for SMS (Set SMS Program Page Number) */
        g_ctsu_sms_work[5].eb6 = 0x1E;                              /* ITF00 clr data */
        g_ctsu_sms_work[5].eb7 = 0x00;                              /* ITF00 clr data */
    }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
   #if BSP_MCU_GROUP_RL78L23
        g_ctsu_sms_work[5].eb0 = dtc_vectortable[48U];              /* DTC Vector table for SMS */
   #else
        g_ctsu_sms_work[5].eb0 = dtc_vectortable[38U];              /* DTC Vector table for SMS */
   #endif
        g_ctsu_sms_work[5].eb1 = DTCBAR;                            /* DTC Base Address */
        g_ctsu_sms_work[5].eb2 = CTSU_SET_LOW(&g_ctsu_sms_transfer);/* Store the SMS Program Address pointer */
        g_ctsu_sms_work[5].eb3 = CTSU_SET_HI(&g_ctsu_sms_transfer); /* Store the SMS Program Address pointer */
        g_ctsu_sms_work[5].eb4 = CTSU_SMS_PAGE_NUM_MUTUAL;          /* DTC Translate for SMS (Set SMS Program Page Number) */
        g_ctsu_sms_work[5].eb5 = CTSU_SMS_PAGE_NUM_MUTUAL;          /* DTC Translate for SMS (Set SMS Program Page Number) */
        g_ctsu_sms_work[5].eb6 = 0x1E;                              /* ITF00 clr data */
        g_ctsu_sms_work[5].eb7 = 0x00;                              /* ITF00 clr data */
    }
  #endif

    /* Work RAM7 */
    g_ctsu_sms_work[6].eb0 = CTSU_SET_LOW(&MK0H);                   /* MK0H Address pointer */
    g_ctsu_sms_work[6].eb1 = CTSU_SET_HI(&MK0H);                    /* MK0H Address pointer */
    g_ctsu_sms_work[6].eb2 = 0x04;                                  /* immediate Value */
    g_ctsu_sms_work[6].eb3 = 0x00;                                  /* immediate Value */
    g_ctsu_sms_work[6].eb4 = CTSU_SET_LOW(&g_ctsu_sms_correction[CTSU_CORRECTION_POINT_NUM]);           /* CTSU Correction Table Data Address pointer */
    g_ctsu_sms_work[6].eb5 = CTSU_SET_HI(&g_ctsu_sms_correction[CTSU_CORRECTION_POINT_NUM]);            /* CTSU Correction Table Data Address pointer */
    g_ctsu_sms_work[6].eb6 = CTSU_SET_LOW(&CTSUSC);                 /* CTSUCSC Address pointer */
    g_ctsu_sms_work[6].eb7 = CTSU_SET_HI(&CTSUSC);                  /* CTSUCSC Address pointer */

    /* Work RAM8 */
    g_ctsu_sms_work[7].eb0 = 0x0E;                      /* subtraction number for division1 (immediate Value) */
    g_ctsu_sms_work[7].eb1 = 0x00;                      /* subtraction number for division1 (immediate Value) */
    g_ctsu_sms_work[7].eb2 = 0x0E;                      /* addition number for multiply1 (immediate Value) */
    g_ctsu_sms_work[7].eb3 = 0x00;                      /* addition number for multiply1 (immediate Value) */
    g_ctsu_sms_work[7].eb4 = 0x00;                      /* Reserve */
    g_ctsu_sms_work[7].eb5 = 0x00;                      /* Reserve */
    g_ctsu_sms_work[7].eb6 = 0xFF;                      /* for overflow operation (immediate Value) */
    g_ctsu_sms_work[7].eb7 = 0xFF;                      /* for overflow operation (immediate Value) */
}

/***********************************************************************************************************************
* ctsu_sms_parameter_init
***********************************************************************************************************************/
void ctsu_sms_parameter_init (ctsu_instance_ctrl_t * const p_instance_ctrl,
                              uint16_t * p_threshold,
                              uint16_t * p_hysteresis,
                              uint16_t count_filter)
{
    uint16_t element;
    uint16_t multi;
    int16_t threshold_l;
    int16_t threshold_h;
    int32_t int_data;

    /* Set CTSU Threshold Data & Sensor Parameter & Initial moving Average data */
    for (element = 0; element < p_instance_ctrl->num_elements; element++)
    {
        for (multi = 0; multi < CTSU_CFG_NUM_SUMULTI; multi++)
        {
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                threshold_l = p_threshold[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi] -
                              p_hysteresis[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi];
                threshold_h = p_threshold[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi];
            }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
            {
                threshold_l = -(p_threshold[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi] -
                                p_hysteresis[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi]);
                threshold_h = -(p_threshold[(element * CTSU_MAJORITY_MODE_ELEMENTS) + multi]);
            }
  #endif
            g_ctsu_sms_threshold[element][multi].threshold_l = threshold_l;
            g_ctsu_sms_threshold[element][multi].threshold_h = threshold_h;

            /* Store CTSUSO Parameter */
            g_ctsu_sms_ctsuwr[element][multi].so0 = (uint16_t) p_instance_ctrl->p_ctsuwr[(element * CTSU_CFG_NUM_SUMULTI) + multi].ctsuso;
            g_ctsu_sms_ctsuwr[element][multi].so1 = (uint16_t) (p_instance_ctrl->p_ctsuwr[(element * CTSU_CFG_NUM_SUMULTI) + multi].ctsuso >> 16);

            /* Set Initial moving average, the others are set 0 */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
            {
                int_data = (p_instance_ctrl->p_self_data + (element * CTSU_MAJORITY_MODE_ELEMENTS) + multi)->int_data;
            }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
            {
                int_data = (p_instance_ctrl->p_mutual_snd_data + (element * CTSU_MAJORITY_MODE_ELEMENTS) + multi)->int_data -
                           (p_instance_ctrl->p_mutual_pri_data + (element * CTSU_MAJORITY_MODE_ELEMENTS) + multi)->int_data;
                if (0 > int_data)
                {
                    int_data = 0;
                }
            }
  #endif
            g_ctsu_sms_result[element][multi].ajmmarl = 0;
            g_ctsu_sms_result[element][multi].ajmmarh = (uint16_t) int_data;
            g_ctsu_sms_result[element][multi].ajblactl = 0;
            g_ctsu_sms_result[element][multi].ajblacth = 0;
            g_ctsu_sms_result[element][multi].ajblarl = 0;
            g_ctsu_sms_result[element][multi].ajblarh = (uint16_t) int_data;
            g_ctsu_sms_result[element][multi].ajrrl = 0;
            g_ctsu_sms_result[element][multi].sjccr = CTSU_SET_HI(count_filter);
        }
    }
}

/***********************************************************************************************************************
* ctsu_sms_correction_data_copy
***********************************************************************************************************************/
void ctsu_sms_correction_data_copy(ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t i;
    uint16_t dac_value;

    for (i = 0; i < CTSU_CORRECTION_POINT_NUM; i++)
    {
        if (0 == i)
        {
            dac_value = p_instance_ctrl->p_correction_info->dac_value_edge[p_instance_ctrl->range][0];
        }
        else if ((CTSU_CORRECTION_POINT_NUM - 1) == i)
        {
            dac_value = p_instance_ctrl->p_correction_info->dac_value_edge[p_instance_ctrl->range][1];
        }
        else
        {
            dac_value = p_instance_ctrl->p_correction_info->dac_value[p_instance_ctrl->range][i];
        }
        g_ctsu_sms_correction[i + 1].scntsctl = (uint16_t) (CTSU_CORRECTION_STD_UNIT * (i + 1));
        g_ctsu_sms_correction[i + 1].scntscth = dac_value;
    }

    /* The first coefficient is 0,0 */
    g_ctsu_sms_correction[0].scntsctl = 0;
    g_ctsu_sms_correction[0].scntscth = 0;
}

/***********************************************************************************************************************
* ctsu_sms_program_setup
***********************************************************************************************************************/
void ctsu_sms_program_setup(ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint8_t page_cnt;
    uint8_t i;

    /* Copy Variable SMS Program & General Register Data */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        for (page_cnt = 0; page_cnt < CTSU_SMS_PAGE_NUM_SELF; page_cnt++)
        {
            for (i = 0; i < CTSU_SMS_VARIABLE_INST_SIZE; i++)
            {
                g_ctsu_sms_transfer[page_cnt].inst[i] = g_ctsu_sms_program[page_cnt][3 + i];
            }

            for (i = 0; i < CTSU_SMS_VARIABLE_GREG_SIZE; i++)
            {
                g_ctsu_sms_transfer[page_cnt].greg[i] = g_ctsu_sms_greg[page_cnt][i];
            }
        }
    }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        for (page_cnt = 0; page_cnt < CTSU_SMS_PAGE_NUM_MUTUAL; page_cnt++)
        {
            for (i = 0; i < CTSU_SMS_VARIABLE_INST_SIZE; i++)
            {
                g_ctsu_sms_transfer[page_cnt].inst[i] = g_ctsu_sms_mutual_program[page_cnt][3 + i];
            }

            for (i = 0; i < CTSU_SMS_VARIABLE_GREG_SIZE; i++)
            {
                g_ctsu_sms_transfer[page_cnt].greg[i] = g_ctsu_sms_mutual_greg[page_cnt][i];
            }
        }
    }
  #endif
}

/***********************************************************************************************************************
* Function Name: ctsu_sms_isr
* Description  : This function is INTSMSE interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
  #if defined(__CCRL__)
#pragma interrupt ctsu_sms_isr(vect=INTSMSE)
void ctsu_sms_isr (void)
  #elif defined(__ICCRL78__)
#pragma vector=INTSMSE_vect
__interrupt void ctsu_sms_isr (void)
  #elif defined(__llvm__)
void ctsu_sms_isr (void)
  #endif
{
    uint16_t                i;
    ctsu_instance_ctrl_t  * p_instance_ctrl = gp_ctsu_isr_context;
    ctsu_callback_args_t    args;
    uint16_t                element_id;

    SMSEMK = 1U;
    SMSEIF = 1U;

    args.event = CTSU_EVENT_SCAN_COMPLETE;

    if (CTSUST & CTSU_SOVF)
    {
        args.event     |= CTSU_EVENT_OVERFLOW;
        CTSUST &= (uint8_t) (~CTSU_SOVF);
    }

    if (CTSUSRL & (CTSU_ICOMP0 | CTSU_ICOMP1))
    {
        if (CTSUSRL & CTSU_ICOMP0)
        {
            args.event |= CTSU_EVENT_ICOMP;
        }

        if (CTSUSRL & CTSU_ICOMP1)
        {
            args.event |= CTSU_EVENT_ICOMP1;
        }

        CTSUSRL |= CTSU_ICOMPRST;
    }

    p_instance_ctrl->state = CTSU_STATE_IDLE;
    args.p_context         = p_instance_ctrl->p_context;

    /* Store the result of measurement in SMS */
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->md)
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                p_instance_ctrl->p_self_data[(element_id * CTSU_CFG_NUM_SUMULTI) + i].int_data =
                        g_ctsu_sms_result[element_id][i].ajmmarh;
            }
        }
    }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->md)
    {
        for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
        {
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                p_instance_ctrl->p_mutual_pri_data[(element_id * CTSU_CFG_NUM_SUMULTI) + i].int_data = 0;
                p_instance_ctrl->p_mutual_snd_data[(element_id * CTSU_CFG_NUM_SUMULTI) + i].int_data =
                        g_ctsu_sms_result[element_id][i].ajmmarh;
            }
        }
    }
  #endif

    /* If a callback was provided, call it with the argument */
    if (NULL != p_instance_ctrl->p_callback)
    {
        p_instance_ctrl->p_callback(&args);
    }

    /* Clear SMS flag */
    p_instance_ctrl->sms = 0;

    /* Event Link Output Enable EOL6 */
  #if BSP_MCU_GROUP_RL78G22
    ELSELR10 = 0x06;
  #else
    ELOENCTL = 0x40;
  #endif

    /* Enable CTSUFNINT */
    CTSUFNIF = 0;
    CTSUFNMK = 0;

    /* Stop SMS */
    SMSSTOP = 1;
    SMSEN   = 0;
    SMSRES  = 1;
    SMSRES  = 0;
}

/***********************************************************************************************************************
* ctsu_sms_elc_init
***********************************************************************************************************************/
void ctsu_sms_elc_init(void)
{
    /* Start ELC setting*/
  #if BSP_MCU_GROUP_RL78G22
    /* ITL    -> CTSU Event Link Setting */
    ELSELR10 = 0x06; /* ELCITL0 select CTSU start */

    /* INTP0 -> SMS Event Link Setting */
   #if CTSU_CFG_SMS_ELC_INTP == 0
    ELSELR00 = 0x04; /* INTP0 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 1
    /* INTP1 -> SMS Event Link Setting */
    ELSELR01 = 0x04; /* INTP1 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 2
    /* INTP2 -> SMS Event Link Setting */
    ELSELR02 = 0x04; /* INTP2 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 3
    /* INTP3 -> SMS Event Link Setting */
    ELSELR03 = 0x04; /* INTP3 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 4
    /* INTP4 -> SMS Event Link Setting */
    ELSELR04 = 0x04; /* INTP4 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 5
    /* INTP5 -> SMS Event Link Setting */
    ELSELR05 = 0x04; /* INTP5 select SMS start */
   #endif

   #if CTSU_CFG_SMS_ELC_INTP == 6
    /* INTP6 -> SMS Event Link Setting */
    ELSELR06 = 0x04; /* INTP6 select SMS start */
   #endif
  #else
    /* ITL    -> CTSU Event Link Setting */
    ELISEL7  = 0x17; /* INTC6 Select(INTITL) */
    ELL1SEL0 = 0x08; /* Signal selected by input signal select register 7 is to be linked (inverted level) */
    ELL1CTL  = 0x00; /* Pass-through is selected */
    ELL1LNK0 = 0x01; /* Linked to input 0 of logic cell 0 in logic cell block L1 */
    ELOSEL6  = 0x01; /* Output signal 0 from logic cell block L1 (output of logic cell 0) is selected */

   #if BSP_MCU_GROUP_RL78L23
    /* CTSURD -> SMS Event Link not Use */

    /* Event Link Output Enable EOL6 */
    ELOENCTL = 0x40;
   #else
    /* CTSURD -> SMS Event Link Setting */
    ELISEL10 = 0x1a; /* INTC9 Select(INTCTSURD) */
    ELL2SEL0 = 0x0b; /* Signal selected by input signal select register 10 is to be linked */
    ELL2CTL  = 0x00; /* Pass-through is selected */
    ELL2LNK0 = 0x01; /* Linked to input 0 of logic cell 0 in logic cell block L2 */
    ELOSEL2  = 0x06; /* Output signal 0 from logic cell block L2 (output of logic cell 0) is selected */

    /* Event Link Output Enable EOL2 & EOL6 */
    ELOENCTL = 0x44;
   #endif
  #endif
}
 #endif /* CTSU_CFG_SMS_SUPPORT_ENABLE == 1 */


 #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_auto_correction_register_set
 ***********************************************************************************************************************/
void ctsu_auto_correction_register_set (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t j;
    int16_t  range;

    /* Initialization of sensor counter auto correction table register number */
    CTSUOPTH &= 0xFFF0U;        /* SCACTB = 0 */

    /* Enter the correction factor and dac_value in CTSUSCNTACT for 12 correction tables */
    range = (int16_t) p_instance_ctrl->range;
    for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
    {
        CTSUSCTACTL = g_ctsu_correction_info.coef[range][j];
        CTSUSCTACTH = g_ctsu_correction_info.dac_value[range][j];
    }
}

 #endif

 #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_multiclock_auto_correction_register_set
 ***********************************************************************************************************************/
void ctsu_multiclock_auto_correction_register_set (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;
    int16_t  offsetcoeff_1;
    int16_t  offsetcoeff_2;
    uint16_t elem_w_sumulti;

    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        elem_w_sumulti = element_id * CTSU_CFG_NUM_SUMULTI;

        /* Calculating the Second Frequency */
        offsetcoeff_1 = ctsu_multiclock_auto_correction_calc(p_instance_ctrl, element_id, 2);

        /* Calculating the Third Frequency */
        offsetcoeff_2 = ctsu_multiclock_auto_correction_calc(p_instance_ctrl, element_id, 3);

        if (0 == p_instance_ctrl->p_ctsu_cfg->ajfen)
        {
            p_instance_ctrl->p_mcact1[elem_w_sumulti + 0] = 0;
            p_instance_ctrl->p_mcact1[elem_w_sumulti + 1] = ((uint32_t) offsetcoeff_1 << 16) |
                                                            (p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 1].ctsuso &
                                                             CTSU_TUNING_MAX);

            p_instance_ctrl->p_mcact1[elem_w_sumulti + 2] = ((uint32_t) offsetcoeff_2 << 16) |
                                                            (p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 2].ctsuso &
                                                             CTSU_TUNING_MAX);
        }
        else
        {
            p_instance_ctrl->p_mcact1[element_id] = ((uint32_t) offsetcoeff_1 << 16) |
                                                    (p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 1].ctsuso &
                                                     CTSU_TUNING_MAX);

            p_instance_ctrl->p_mcact2[element_id] = ((uint32_t) offsetcoeff_2 << 16) |
                                                    (p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 2].ctsuso &
                                                     CTSU_TUNING_MAX);
        }
    }
}

/***********************************************************************************************************************
 * ctsu_multiclock_auto_correction_calc
 ***********************************************************************************************************************/
int16_t ctsu_multiclock_auto_correction_calc (ctsu_instance_ctrl_t * const p_instance_ctrl,
                                              uint16_t                     element_id,
                                              uint8_t                      freq)
{
    uint8_t  current_mode;
    uint8_t  first_sumulti;
    uint8_t  freq_sumulti;
    uint16_t first_so;
    uint16_t freq_so;
    uint8_t  snum_recommend;
    uint8_t  snum;
    int32_t  numerator_1;              // ((SUMULTIn + 1)SO0 - (SUMULTI0 + 1)SOn)
    int32_t  numerator_2;              // ((Gcco * 20uA * 128us) / 1024)(OffsetDACmax / CurrentMode))
    int32_t  numerator_3;              // (SNUM + 1)
    int32_t  denominator;              // ((SUMULTIn + 1)(SNUM0 + 1)
    int16_t  offsetcoeff;
    int32_t  offsetcoeff_tmp;
    uint16_t elem_w_sumulti;
    elem_w_sumulti = element_id * CTSU_CFG_NUM_SUMULTI;

    if (CTSU_ATUNE12_80UA == p_instance_ctrl->p_ctsu_cfg->atune12)
    {
        current_mode = CTSU_AUTO_CURRENT_MODE_80UA;
    }
    else if (CTSU_ATUNE12_40UA == p_instance_ctrl->p_ctsu_cfg->atune12)
    {
        current_mode = CTSU_AUTO_CURRENT_MODE_40UA;
    }
    else if (CTSU_ATUNE12_20UA == p_instance_ctrl->p_ctsu_cfg->atune12)
    {
        current_mode = CTSU_AUTO_CURRENT_MODE_20UA;
    }
    else                               // (CTSU_ATUNE12_160UA == p_instance_ctrl->p_ctsu_cfg->atune12)
    {
        current_mode = CTSU_AUTO_CURRENT_MODE_160UA;
    }

    if (2 == freq)
    {
        freq_sumulti = CTSU_CFG_SUMULTI1 + 1;
        freq_so      = p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 1].ctsuso & CTSU_TUNING_MAX;
    }
    else                               // 3 == freq
    {
        freq_sumulti = CTSU_CFG_SUMULTI2 + 1;
        freq_so      = p_instance_ctrl->p_ctsuwr[elem_w_sumulti + 2].ctsuso & CTSU_TUNING_MAX;
    }

    first_so       = p_instance_ctrl->p_ctsuwr[elem_w_sumulti].ctsuso & CTSU_TUNING_MAX;
    first_sumulti  = CTSU_CFG_SUMULTI0 + 1;
    snum_recommend = CTSU_SNUM_RECOMMEND + 1;
    snum           = (p_instance_ctrl->p_ctsuwr[elem_w_sumulti].ctsuso >> 10) & CTSU_SNUM_MAX;

    numerator_1 = ((int32_t) freq_sumulti * first_so) - ((int32_t) first_sumulti * freq_so);
    numerator_2 = (int32_t) (CTSU_AUTO_REF_COEFFICIENT * CTSU_AUTO_CORRECTION_OFFSET_DAC_MAX / current_mode);
    numerator_3 = (int32_t) snum + 1;
    denominator = (int32_t) (freq_sumulti * snum_recommend);

    /* overflow check int32 */
    if ((CTSU_AUTO_INT32_OVERFLOW_VALUE / numerator_2) <= (numerator_1 * numerator_3))
    {
        offsetcoeff = CTSU_AUTO_INT16_OVERFLOW_VALUE;
        p_instance_ctrl->p_correction_info->calculation_error = 1;
    }
    else if (-(CTSU_AUTO_INT32_OVERFLOW_VALUE / numerator_2) >= (numerator_1 * numerator_3))
    {
        offsetcoeff = CTSU_AUTO_INT16_UNDERFLOW_VALUE;
        p_instance_ctrl->p_correction_info->calculation_error = 1;
    }
    else
    {
        /* calculate offset coefficient with int32 */
        offsetcoeff_tmp = (numerator_1 * numerator_2 * numerator_3 / denominator);

        /* overflow check int16 */
        if (CTSU_AUTO_INT16_OVERFLOW_VALUE <= offsetcoeff_tmp)
        {
            offsetcoeff = CTSU_AUTO_INT16_OVERFLOW_VALUE;
            p_instance_ctrl->p_correction_info->calculation_error = 1;
        }
        else if (CTSU_AUTO_INT16_UNDERFLOW_VALUE >= offsetcoeff_tmp)
        {
            offsetcoeff = CTSU_AUTO_INT16_UNDERFLOW_VALUE;
            p_instance_ctrl->p_correction_info->calculation_error = 1;
        }
        else
        {
            offsetcoeff = (int16_t) offsetcoeff_tmp;
        }
    }

    return offsetcoeff;
}

 #endif

 #if (CTSU_CFG_AUTO_JUDGE_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_auto_judge_threshold_calc
 ***********************************************************************************************************************/
void ctsu_auto_judge_threshold_calc (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint8_t element_id;
    uint8_t i;
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    int16_t ajthr_h;
    int16_t ajthr_l;
  #endif

    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
  #if (CTSU_CFG_AUTO_MULTI_CLOCK_CORRECTION_ENABLE == 1)
        if (1 == p_instance_ctrl->p_ctsu_cfg->majirimd)
        {
   #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
            {
   #endif
                p_instance_ctrl->p_ajthr[element_id] =
                    ((uint32_t) p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].threshold << 16) |
                    (p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].
                     threshold - p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].hysteresis);
   #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            }
            else
            {
                ajthr_h =
                    (int16_t) (-(p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].threshold) +
                               p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].hysteresis);
                ajthr_l = (int16_t) (-(p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[element_id].threshold));
                p_instance_ctrl->p_ajthr[element_id] = ((uint32_t) ajthr_h << 16) |
                                                       (ajthr_l & CTSU_AUTO_MINUS_BIT_MASK);
            }
   #endif
        }
        else
  #endif
        {
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
                if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
                {
  #endif
                    p_instance_ctrl->p_ajthr[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i] =
                        ((uint32_t) p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id *
                                                                                      CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                                     i].threshold << 16) |
                        (p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                          i].threshold -
                         p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                          i].hysteresis);
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
                }
                else
                {
                    ajthr_h =
                        (int16_t) (-(p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id *
                                                                                       CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                                      i].threshold) +
                                   p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id *
                                                                                     CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                                    i].hysteresis);
                    ajthr_l =
                        (int16_t) (-(p_instance_ctrl->p_ctsu_cfg->p_ctsu_auto_buttons[(element_id *
                                                                                       CTSU_MAJORITY_MODE_ELEMENTS) +
                                                                                      i].threshold));
                    p_instance_ctrl->p_ajthr[(element_id * CTSU_MAJORITY_MODE_ELEMENTS) + i] =
                        ((uint32_t) ajthr_h << 16) | (ajthr_l & CTSU_AUTO_MINUS_BIT_MASK);
                }
  #endif
            }
        }
    }
}
 #endif

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1) && (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
/***********************************************************************************************************************
 * ctsu_adc_init
 ***********************************************************************************************************************/
static void ctsu_adc_init(void)
{
    ADCEN = 1U;  /* supply AD clock */

  #if (defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22))
    ADCKS  = 0x00U;         /* A/D conversion clock no devide setting */
    ADWINR = 0x00U;         /* Access window setting for the ADCSR register (0) */
    ADCSR &= (~0x8000U);    /* Stop A/D conversion */
    ADMK   = 1U;            /* disable INTAD interrupt */
    ADIF   = 0U;            /* clear INTAD interrupt flag */

    /* Set INTAD low priority (level 3) */
    ADPR1  = 1U;
    ADPR0  = 1U;

    /* Set transition analog block to operation mode */
    ADWINR = 0x08U;
    ADSLP  = 0U;    /* Normal operation mode */

    ctsu_delay(1U); /* wait for 1uS */

    /* AD configuration */
    ADWINR      = 0x00U;    /* Access window setting for the ADCSR register (0) */
    ADCSR       = 0x0000U;  /* Single scan mode, started by trigger disable and scan end interrupt enable */
    ADWINR      = 0x00U;    /* Access window setting for the ADANSA1 register (0) */
    ADANSA1     = 0x2000U;  /* ANI29 (TSCAP) used */
    ADWINR      = 0x08U;    /* Access window setting for the ADHVREFCNT register (8) */
    ADHVREFCNT |= 0x11U;    /* AVREFM and AVREFP are selected */
    ADWINR      = 0x00U;    /* Access window setting for the ADCER register (0) */
    ADCER       = 0x0000U;  /* Right aligned is selected, self-diag disable and auto clearing disable */
    ADWINR      = 0x07U;    /* Access window setting for the ADDISCR register (7) */
    ADDISCR     = 0x00U;    /* Disables the disconnection detection assist function */
    ADWINR      = 0x00U;    /* Access window setting for the ADADC register (0) */
    ADADC       = 0x00U;    /* Addition mode is selected and 1-time conversion */
    ADWINR      = 0x00U;    /* Access window setting for the ADSSTRL register (0) */
    ADSSTRL     = 0x20U;    /* Sampling time for ANI16-ANI29 setting */
  #else
    ADM0 = 0x00U;   /* disable AD conversion and clear ADM0 register */
    ADMK = 1U;      /* disable INTAD interrupt */
    ADIF = 0U;      /* clear INTAD interrupt flag */
    ///* The reset status of ADPC is analog input, so it's unnecessary to set. */
    ADM0 = 0x00U;   /* AD_CONVERSION_CLOCK_64 | AD_TIME_MODE_NORMAL_1 | AD_OPERMODE_SELECT */
    ADM1 = 0x20U;   /* AD_TRIGGER_SOFTWARE | AD_CONVMODE_ONESELECT */
   #if BSP_MCU_GROUP_RL78G22
    ADM2 = 0x00U;   /* AD_POSITIVE_VDD | AD_NEGATIVE_VSS | AD_AREA_MODE_1 | AD_RESOLUTION_10BIT */
   #else
    ADM2 = 0x02U;   /* AD_POSITIVE_VDD | AD_NEGATIVE_VSS | AD_AREA_MODE_1 | AD_RESOLUTION_12BIT */
   #endif
    ADUL = 0xFFU;   /* AD_ADUL_VALUE */
    ADLL = 0x00U;   /* AD_ADLL_VALUE */
    ADS  = 0x1EU;   /* CH30 */
  #endif
}
 #endif
#endif /* BSP_FEATURE_CTSU_VERSION == 2 */

/*******************************************************************************************************************//**
 * @brief Diagnosis the CTSU peripheral.
 * Implements @ref ctsu_api_t::diagnosis.
 *
 *
 * Example:
 * @snippet r_ctsu_example.c R_CTSU_Diagnosis
 *
 * @retval FSP_SUCCESS                             CTSU successfully configured.
 * @retval FSP_ERR_ASSERTION                       Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN                        Module is not open.
 * @retval FSP_ERR_CTSU_NOT_GET_DATA               The previous data has not been retrieved by DataGet.
 * @retval FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE        Diagnosis of LDO output voltage failed.
 * @retval FSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE      Diagnosis of LDO over voltage failed.
 * @retval FSP_ERR_CTSU_DIAG_CCO_HIGH              Diagnosis of CCO into 19.2uA failed.
 * @retval FSP_ERR_CTSU_DIAG_CCO_LOW               Diagnosis of CCO into 2.4uA failed.
 * @retval FSP_ERR_CTSU_DIAG_SSCG                  Diagnosis of SSCG frequency failed.
 * @retval FSP_ERR_CTSU_DIAG_DAC                   Diagnosis of non-touch count value failed.
 * @retval FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE        Diagnosis of LDO output voltage failed.
 * @retval FSP_ERR_CTSU_DIAG_OVER_VOLTAGE          Diagnosis of over voltage detection circuit failed.
 * @retval FSP_ERR_CTSU_DIAG_OVER_CURRENT          Diagnosis of over current detection circuit failed.
 * @retval FSP_ERR_CTSU_DIAG_LOAD_RESISTANCE       Diagnosis of LDO internal resistance value failed.
 * @retval FSP_ERR_CTSU_DIAG_CURRENT_SOURCE        Diagnosis of LDO internal resistance value failed.
 * @retval FSP_ERR_CTSU_DIAG_SENSCLK_GAIN          Diagnosis of SENSCLK frequency gain failed.
 * @retval FSP_ERR_CTSU_DIAG_SUCLK_GAIN            Diagnosis of SUCLK frequency gain failed.
 * @retval FSP_ERR_CTSU_DIAG_CLOCK_RECOVERY        Diagnosis of SUCLK clock recovery function failed.
 * @retval FSP_ERR_CTSU_DIAG_CFC_GAIN              Diagnosis of CFC oscillator gain failed.
 **********************************************************************************************************************/

fsp_err_t R_CTSU_Diagnosis (ctsu_ctrl_t * const p_ctrl)
{
#if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    fsp_err_t diag_err;
#endif
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNED != p_instance_ctrl->state, FSP_ERR_CTSU_NOT_GET_DATA);

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
  #if BSP_MCU_GROUP_RL78G16
        diag_err = ctsu_diag_output_voltage_result();

        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE;
        }
  #endif
        diag_err = ctsu_diag_ldo_over_voltage_result();
        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE;
        }

        diag_err = ctsu_diag_oscillator_high_result();
        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_CCO_HIGH;
        }

        diag_err = ctsu_diag_oscillator_low_result();
        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_CCO_LOW;
        }

        diag_err = ctsu_diag_sscg_result();
        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_SSCG;
        }

  #if BSP_MCU_GROUP_RL78G16
        diag_err = ctsu_diag_dac_woTS_result();
  #else
        diag_err = ctsu_diag_dac_result();
  #endif
        if (FSP_SUCCESS != diag_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_INIT;

            return FSP_ERR_CTSU_DIAG_DAC;
        }

        g_ctsu_diag_info.state = CTSU_DIAG_INIT;
    }
 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)
    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
        diag_err = g_ctsu_diag_info.test_result;
        g_ctsu_diag_info.test_result = FSP_SUCCESS;
        g_ctsu_diag_info.test_count  = 0;
        g_ctsu_diag_info.state       = CTSU_DIAG_INIT;
  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.buffer_index = CTSU_DIAG_DEBUG_DATA_START_INDEX;
        if (CTSU_DIAG_DEBUG_PROGRESS == g_ctsu_diag_info.debug_state)
        {
            g_ctsu_diag_info.debug_state = CTSU_DIAG_DEBUG_COMPLETE;
            g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_HEADER_INDEX]  = CTSU_DIAG_DEBUG_HEADER_VALUE;
            g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_COMMAND_INDEX] = CTSU_DIAG_DEBUG_COMMAND_DATA_SEND;

            if (FSP_SUCCESS == diag_err)
            {
                g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_RESULT_INDEX] = FSP_SUCCESS;
            }
            else
            {
                g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_RESULT_INDEX] = CTSU_DIAG_DEBUG_ERROR_VALUE;
            }

            g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_FIRST_ID_INDEX]    = g_ctsu_diag_info.debug_id;
            g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_LAST_ID_INDEX]     = g_ctsu_diag_info.debug_id;
            g_ctsu_diag_info.debug_buffer[CTSU_DIAG_DEBUG_DATA_FOOTER_INDEX] = CTSU_DIAG_DEBUG_FOOTER_VALUE;
        }
  #endif

        return diag_err;
    }
 #endif
#endif

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}

#if (BSP_FEATURE_CTSU_VERSION == 1)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_diag_scan_start1
 ***********************************************************************************************************************/
static void ctsu_diag_scan_start1 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    if (CTSU_DIAG_INIT == g_ctsu_diag_info.state)
    {
  #if BSP_MCU_GROUP_RL78G16
        g_ctsu_diag_info.state = CTSU_DIAG_OUTPUT_VOLTAGE;
  #else
        g_ctsu_diag_info.state = CTSU_DIAG_OVER_VOLTAGE;
  #endif
    }

    /* ctsu normal scan register save */
    g_ctsu_diag_reg.ctsucr0    = CTSUCR0;
    g_ctsu_diag_reg.ctsucr1    = CTSUCR1;
    g_ctsu_diag_reg.ctsusdprs  = CTSUSDPRS;
    g_ctsu_diag_reg.ctsusst    = CTSUSST;
    g_ctsu_diag_reg.ctsuchac0  = CTSUCHAC0;
    g_ctsu_diag_reg.ctsuchac1  = CTSUCHAC1;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    g_ctsu_diag_reg.ctsuchac2  = CTSUCHAC2;
#endif
    g_ctsu_diag_reg.ctsuchtrc0 = CTSUCHTRC0;
    g_ctsu_diag_reg.ctsuchtrc1 = CTSUCHTRC1;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    g_ctsu_diag_reg.ctsuchtrc2 = CTSUCHTRC2;
#endif
    g_ctsu_diag_reg.ctsudclkc  = CTSUDCLKC;
    g_ctsu_diag_reg.ctsuerrs   = CTSUERRS;

    /* scan register setting */
  #if BSP_MCU_GROUP_RL78G16
    if (CTSU_DIAG_OUTPUT_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_output_voltage_scan_start();
    }
  #endif
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_ldo_over_voltage_scan_start();
    }

    if (CTSU_DIAG_CCO_HIGH == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_high_scan_start();
    }

    if (CTSU_DIAG_CCO_LOW == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_low_scan_start();
    }

    if (CTSU_DIAG_SSCG == g_ctsu_diag_info.state)
    {
        ctsu_diag_sscg_scan_start();
    }

    if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
  #if BSP_MCU_GROUP_RL78G16
        ctsu_diag_dac_woTS_scan_start();
  #else
        ctsu_diag_dac_scan_start(p_instance_ctrl);
  #endif
    }
}

/***********************************************************************************************************************
 * ctsu_diag_data_get1
 ***********************************************************************************************************************/
static fsp_err_t ctsu_diag_data_get1 (void)
{
    fsp_err_t err;

    /* data get */
  #if BSP_MCU_GROUP_RL78G16
    if (CTSU_DIAG_OUTPUT_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_output_voltage_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_OVER_VOLTAGE;
    }
    else
  #endif
    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_ldo_over_voltage_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_CCO_HIGH;
    }
    else if (CTSU_DIAG_CCO_HIGH == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_high_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_CCO_LOW;
    }
    else if (CTSU_DIAG_CCO_LOW == g_ctsu_diag_info.state)
    {
        ctsu_diag_oscillator_low_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_SSCG;
    }
    else if (CTSU_DIAG_SSCG == g_ctsu_diag_info.state)
    {
        ctsu_diag_sscg_data_get();

        g_ctsu_diag_info.state = CTSU_DIAG_DAC;
    }
    else if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
  #if BSP_MCU_GROUP_RL78G16
        ctsu_diag_dac_woTS_data_get();
        g_ctsu_diag_info.state = CTSU_DIAG_COMPLETE;
  #else
        ctsu_diag_dac_data_get();
        if (CTSU_TUNING_INCOMPLETE == g_ctsu_diag_info.tuning)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_DAC;
        }
        else
        {
            g_ctsu_diag_info.loop_count++;
            if (6 <= g_ctsu_diag_info.loop_count)
            {
                g_ctsu_diag_info.state      = CTSU_DIAG_COMPLETE;
                g_ctsu_diag_info.loop_count = 0;
            }
        }
  #endif
    }
    else
    {
    }

    /* register restore */
    CTSUCR0      = g_ctsu_diag_reg.ctsucr0;
    CTSUCR1      = g_ctsu_diag_reg.ctsucr1;
    CTSUSDPRS    = g_ctsu_diag_reg.ctsusdprs;
    CTSUSST      = g_ctsu_diag_reg.ctsusst;
    CTSUCHAC0    = g_ctsu_diag_reg.ctsuchac0;
    CTSUCHAC1    = g_ctsu_diag_reg.ctsuchac1;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHAC2    = g_ctsu_diag_reg.ctsuchac2;
#endif
    CTSUCHTRC0   = g_ctsu_diag_reg.ctsuchtrc0;
    CTSUCHTRC1   = g_ctsu_diag_reg.ctsuchtrc1;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2   = g_ctsu_diag_reg.ctsuchtrc2;
#endif
    CTSUDCLKC    = g_ctsu_diag_reg.ctsudclkc;
    CTSUERRS     = 0;

    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
        err = FSP_SUCCESS;
    }
    else
    {
        err = FSP_ERR_CTSU_DIAG_NOT_YET;
    }

    return err;
}

  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_output_voltage_scan_start (void)
{
    uint8_t k;
    uint8_t j;
    uint16_t adc_value;

    ADMK  = 1U;     /* Disable INTAD interrupt */
    ADMK  = 1U;     /* Disable INTAD interrupt */
    ADCEN = 1U;     /* Supply AD clock */
    ADM0  = 0x00U;  /* Disable AD conversion and clear ADM0 register */
    ADIF  = 0U;     /* Clear INTAD interrupt flag */

    ///* The reset status of ADPC is analog input, so it's unnecessary to set. */

    /* ADM0 = | ADCS | ADMD | FR2 | FR1 | FR0 | LV1 | LV0 | ADCE | G22 */
    /*        | ADCS |  0   |  0  | FR1 | FR0 |  0  | LV0 | ADCE | G16 */
    /* ADSC 0:disable conversion, 1:enable conversion
       ADMD 0:select mode 1:scan mode
       FR2/1/0,LV1/0 sampling clock division ratio setting
       ADCE 0:disable comparator, 1: enable comparator
       */
    ADM0 = 0U;

    /* RL78/G16 does not have AMD1 register */
    /* ADM1 = | ADTMD1 | ADTMD0 | ADSCM |  0  |  0  |  0  | ADTRS1 | ADTRS0 | G22 */
    /* ADTMD1/0 Software trigger mode
       ADSCM    continuously conversion mode
       ADTRS1/0 [no effect in software trigger mode](timer01 interrupt) */

    /* ADM2 = | ADREFP1 | ADREFP0 | ADREFM |  0  | ADRCK | AWC |  0  | ADTYP | G22 */
    /*        |    0    |    0    |    0   |  0  |   0   |  0  |  0  | ADTYP | G16 */
    ADM2 = 0U;      /* AD resolution 10 bit */

    /* RL78/G16 does not have ADUL ADLL register */

    ADS = 0x0BU;    /* Select CTSU TSCAP voltage */

    /* CTSU setting */
    CTSUCR1 |= (CTSUCR1_CSW_Msk | CTSUCR1_PON_Msk);     /* CTSU power on */

    for (k = 0U; k < 4U; k++)
    {
        switch (k)
        {
            case 0U:
                {
                    /* R02 (30kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_2;

                    /* 40uA */
                    CTSUCR1 &= ~CTSUCR1_ATUNE1_Msk;
                }
                break;
            case 1U:
                {
                    /* R01 (15kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_1;

                    /* 40uA */
                    CTSUCR1 &= ~CTSUCR1_ATUNE1_Msk;
                }
                break;
            case 2U:
                {
                    /* R02 (30kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_2;

                    /* 80uA */
                    CTSUCR1 |= CTSUCR1_ATUNE1_Msk;
                }
                break;
            case 3U:
                {
                    /* R01 (15kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_1;

                    /* 80uA */
                    CTSUCR1 |= CTSUCR1_ATUNE1_Msk;
                }
                break;
            default:
                break;
        }

        ADCE = 1U;
        ctsu_delay(1U);

        adc_value = 0U;

        for (j = 0U; j < 16U; j++)
        {
            /* Software trigger start scan */
            ADCS = 1U;

            /* Wait for conversion to complete */
            while (1U == ADCS)
            {
            }

            adc_value += (ADCR >> 6U);
        }

        adc_value >>= 2U;

        /* Read A/D data then scan normal end */
        g_ctsu_diag_info.output_voltage_cnt[k] = (uint16_t)((uint32_t)adc_value * CTSU_CFG_VCC_MV / CTSU_DIAG_TSCAP_MES_VOLTAGE );
    }

    /* ADC Close */
    ADCEN = 0U; /* stop supply of AD clock */
    ADIF  = 0U; /* clear INTAD interrupt flag */
}

static void ctsu_diag_output_voltage_data_get(void)
{
    /* Do nothing */
}

static fsp_err_t ctsu_diag_output_voltage_result(void)
{
    uint8_t k;

    for (k = 0U; k < 4U; k++)
    {
        if ((g_ctsu_diag_info.output_voltage_cnt[k] <= CTSU_DIAG_TSCAP_RANGE_LOW) ||
            (g_ctsu_diag_info.output_voltage_cnt[k] >= CTSU_DIAG_TSCAP_RANGE_HIGH))
        {
            return FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE;
        }
    }

    return FSP_SUCCESS;
}
  #endif

static void ctsu_diag_ldo_over_voltage_scan_start (void)
{
    uint32_t pclkb_mhz;
    uint32_t ctsu_sdpa;

    /* Set power on */
    CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

    /* Synchronous Noise Reduction Setting */
    CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

    /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
    CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

    /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
    CTSUSST = (CTSU_SST_RECOMMEND);

    /* Since CLK is rewritten by correction, set here. */
    CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);

    pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        CTSUCR1 |= (0 << 4);
        ctsu_sdpa        = pclkb_mhz - 1;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        CTSUCR1 |= (1 << 4);
        ctsu_sdpa        = (pclkb_mhz / 2) - 1;
    }
    else
    {
        CTSUCR1 |= (2 << 4);
        ctsu_sdpa        = (pclkb_mhz / 4) - 1;
    }

    CTSUCR1   |= (CTSU_MODE_SELF_MULTI_SCAN << 6);
    CTSUCHAC0  = 0x01;
    CTSUCHAC1  = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHAC2  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHAC3  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHAC4  = 0x00;
#endif
    CTSUCHTRC0 = 0x00;
    CTSUCHTRC1 = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHTRC3 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHTRC4 = 0x00;
#endif

    /* Correction measurement setting */
    CTSUERRS = 0x0080;
    g_ctsu_diag_info.icomp = 0;

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = CTSU_DIAG_DAC_SO_MAX;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | (ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);
}

static void ctsu_diag_ldo_over_voltage_data_get (void)
{
    /* Nothing */
}

static fsp_err_t ctsu_diag_ldo_over_voltage_result (void)
{
    if (1 != g_ctsu_diag_info.icomp)
    {
        return FSP_ERR_CTSU_DIAG_LDO_OVER_VOLTAGE;
    }

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}

static void ctsu_diag_oscillator_high_scan_start (void)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz;

    /* Set power on */
    CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

    /* Synchronous Noise Reduction Setting */
    CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

    /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
    CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

    /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
    CTSUSST = (CTSU_SST_RECOMMEND);

    /* Since CLK is rewritten by correction, set here. */
    CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);

    pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        CTSUCR1  |= (0 << 4);
        ctsu_sdpa        = pclkb_mhz - 1;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        CTSUCR1  |= (1 << 4);
        ctsu_sdpa        = (pclkb_mhz / 2) - 1;
    }
    else
    {
        CTSUCR1  |= (2 << 4);
        ctsu_sdpa        = (pclkb_mhz / 4) - 1;
    }

    CTSUCR1   |= (CTSU_MODE_SELF_MULTI_SCAN << 6);
    CTSUCHAC0  = 0x01;
    CTSUCHAC1  = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHAC2  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHAC3  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHAC4  = 0x00;
#endif
    CTSUCHTRC0 = 0x00;
    CTSUCHTRC1 = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHTRC3 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHTRC4 = 0x00;
#endif

    CTSUSO0 = 0;

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = 0x0000;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | (ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    CTSUERRS = 0x0082;
    ctsu_delay(10000);
}

static void ctsu_diag_oscillator_high_data_get (void)
{
    g_ctsu_diag_info.cco_high = g_ctsu_diag_info.scanbuf.sen;
}

static fsp_err_t ctsu_diag_oscillator_high_result (void)
{
    if ((g_ctsu_diag_info.cco_high < CTSU_CFG_DIAG_CCO_HIGH_MAX) &&
        (g_ctsu_diag_info.cco_high > CTSU_CFG_DIAG_CCO_HIGH_MIN))
    {
    }
    else
    {
        return FSP_ERR_CTSU_DIAG_CCO_HIGH;
    }

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}

static void ctsu_diag_oscillator_low_scan_start (void)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz;

    /* Set power on */
    CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

    /* Synchronous Noise Reduction Setting */
    CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

    /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
    CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

    /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
    CTSUSST = (CTSU_SST_RECOMMEND);

    /* Since CLK is rewritten by correction, set here. */
    CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);

    pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        CTSUCR1  |= (0 << 4);
        ctsu_sdpa        = pclkb_mhz - 1;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        CTSUCR1  |= (1 << 4);
        ctsu_sdpa        = (pclkb_mhz / 2) - 1;
    }
    else
    {
        CTSUCR1  |= (2 << 4);
        ctsu_sdpa        = (pclkb_mhz / 4) - 1;
    }

    CTSUCR1   |= (CTSU_MODE_SELF_MULTI_SCAN << 6);
    CTSUCHAC0  = 0x01;
    CTSUCHAC1  = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHAC2  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHAC3  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHAC4  = 0x00;
#endif
    CTSUCHTRC0 = 0x00;
    CTSUCHTRC1 = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHTRC3 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHTRC4 = 0x00;
#endif

    CTSUSO0 = 0;

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = 0x0000;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | (ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    CTSUERRS = 0x0080;
    ctsu_delay(10000);
}

static void ctsu_diag_oscillator_low_data_get (void)
{
    g_ctsu_diag_info.cco_low = g_ctsu_diag_info.scanbuf.sen;
}

static fsp_err_t ctsu_diag_oscillator_low_result (void)
{
    if ((g_ctsu_diag_info.cco_low < CTSU_CFG_DIAG_CCO_LOW_MAX) &&
        (g_ctsu_diag_info.cco_low > CTSU_CFG_DIAG_CCO_LOW_MIN))
    {
    }
    else
    {
        return FSP_ERR_CTSU_DIAG_CCO_LOW;
    }

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}

static void ctsu_diag_sscg_scan_start (void)
{
    uint32_t ctsu_sdpa;
    uint32_t pclkb_mhz;

    /* Set power on */
    CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

    /* Synchronous Noise Reduction Setting */
    CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

    /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
    CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

    /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
    CTSUSST = (CTSU_SST_RECOMMEND);

    /* Since CLK is rewritten by correction, set here. */
    CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);

    pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;
    if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
    {
        CTSUCR1 |= (0 << 4);
        ctsu_sdpa        = pclkb_mhz - 1;
    }
    else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
    {
        CTSUCR1 |= (1 << 4);
        ctsu_sdpa        = (pclkb_mhz / 2) - 1;
    }
    else
    {
        CTSUCR1 |= (2 << 4);
        ctsu_sdpa        = (pclkb_mhz / 4) - 1;
    }

    CTSUCR1 |= (CTSU_MODE_SELF_MULTI_SCAN << 6);

    CTSUCHAC0  = 0x01;
    CTSUCHAC1  = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHAC2  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHAC3  = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHAC4  = 0x00;
#endif
    CTSUCHTRC0 = 0x00;
    CTSUCHTRC1 = 0x00;
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHTRC3 = 0x00;
#endif
#if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHTRC4 = 0x00;
#endif

    CTSUSO0 = 0;

    g_ctsu_diag_info.ctsuwr.ctsussc = (CTSU_SSDIV_0500 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso0 = 0x0000;
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_66 << 13) | (ctsu_sdpa << 8) | CTSU_RICOA_RECOMMEND);

    /* Correction measurement setting */
    CTSUERRS = 0x0080;

    CTSUERRS = 0x00C0;
}

static void ctsu_diag_sscg_data_get (void)
{
    g_ctsu_diag_info.sscg = g_ctsu_diag_info.scanbuf.ref;
}

static fsp_err_t ctsu_diag_sscg_result (void)
{
    if ((g_ctsu_diag_info.sscg > CTSU_CFG_DIAG_SSCG_MAX) || (g_ctsu_diag_info.sscg < CTSU_CFG_DIAG_SSCG_MIN))
    {
        return FSP_ERR_CTSU_DIAG_SSCG;
    }

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}

  #if BSP_MCU_GROUP_RL78G16
static void ctsu_diag_dac_woTS_scan_start(void)
{
    uint8_t k;
    uint8_t j;
    uint16_t adc_value;

    ADMK  = 1U;     /* Disable INTAD interrupt */
    ADCEN = 1U;     /* Supply AD clock */
    ADM0  = 0x00U;  /* Disable AD conversion and clear ADM0 register */
    ADIF  = 0U;     /* Clear INTAD interrupt flag */

    ///* The reset status of ADPC is analog input, so it's unnecessary to set. */

    /* ADM0 = | ADCS | ADMD | FR2 | FR1 | FR0 | LV1 | LV0 | ADCE | G22 */
    /*        | ADCS |  0   |  0  | FR1 | FR0 |  0  | LV0 | ADCE | G16 */
    /* ADSC 0:disable conversion, 1:enable conversion
       ADMD 0:select mode 1:scan mode
       FR2/1/0,LV1/0 sampling clock division ratio setting
       ADCE 0:disable comparator, 1: enable comparator
       */
    ADM0 = 0U;

    /* RL78/G16 does not have AMD1 register */
    /* ADM1 = | ADTMD1 | ADTMD0 | ADSCM |  0  |  0  |  0  | ADTRS1 | ADTRS0 | G22 */
    /* ADTMD1/0 Software trigger mode
       ADSCM    continuously conversion mode
       ADTRS1/0 [no effect in software trigger mode](timer01 interrupt) */

    /* ADM2 = | ADREFP1 | ADREFP0 | ADREFM |  0  | ADRCK | AWC |  0  | ADTYP | G22 */
    /*        |    0    |    0    |    0   |  0  |   0   |  0  |  0  | ADTYP | G16 */
    ADM2 = 0U;      /* AD resolution 10 bit */

    /* RL78/G16 does not have ADUL ADLL register */

    ADS = 0x0BU;    /* Select CTSU TSCAP voltage */

    CTSUINIT = 1U;  /* Initializing CTSU controller and registers */

    CTSUCR1 |= (CTSUCR1_CSW_Msk | CTSUCR1_PON_Msk);     /* CTSU power on */
    CTSUCR1 &= ~(CTSUCR1_MD1_Msk | CTSUCR1_MD0_Msk);    /* Self single scan mode */

    CTSUERRS |= CTSUERRS_DRV_Msk;                       /* Calibration setting 1 */
    CTSUERRS  = (CTSUERRS & ~CTSUERRS_SPMD1_Msk) | CTSUERRS_SPMD0_Msk;  /* LDO off */

    for (k = 0U; k < 8U; k++)
    {
        switch (k)
        {
            case 0U:
                {
                    /* SO[9:0] = 0b1000000000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0200U;

                    /* R01 (15kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_1;
                }
                break;
            case 1U:
                {
                    /* SO[9:0] = 0b0100000000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0100U;

                    /* R02 (30kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_2;
                }
                break;
            case 2U:
                {
                    /* SO[9:0] = 0b0010000000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0080U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            case 3U:
                {
                    /* SO[9:0] = 0b0001110000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0070U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            case 4U:
                {
                    /* SO[9:0] = 0b0001100000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0060U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            case 5U:
                {
                    /* SO[9:0] = 0b0001010000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0050U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            case 6U:
                {
                    /* SO[9:0] = 0b0000110000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0030U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            case 7U:
                {
                    /* SO[9:0] = 0b0010001000 */
                    CTSUSO0 = (CTSUSO0 & 0xFC00U) | 0x0088U;

                    /* R03 (60kohm) */
                    CTSUERRS = (CTSUERRS & ~CTSU_LDO_RESISTOR_MASK) | CTSU_LDO_RESISTOR_3;
                }
                break;
            default:
                break;
        }

        ADCE = 1U;
        ctsu_delay(3500U);

        adc_value = 0U;

        for (j = 0U; j < 16U; j++)
        {
            /* Software trigger start scan */
            ADCS = 1U;

            /* Wait for conversion to complete */
            while (1U == ADCS)
            {
            }

            adc_value += (ADCR >> 6U);
        }

        adc_value >>= 2U;

        /* Read A/D data then scan normal end */
        g_ctsu_diag_info.dac_voltage_value[k] = (uint16_t)(((uint32_t)adc_value * CTSU_CFG_VCC_MV) / 4095UL);
    }

    /* ADC Close */
    ADCEN = 0U; /* stop supply of AD clock */
    ADIF  = 0U; /* clear INTAD interrupt flag */
}

static void ctsu_diag_dac_woTS_data_get(void)
{
    /* Do nothing */
}

static fsp_err_t ctsu_diag_dac_woTS_result(void)
{
    uint8_t  k;
    uint16_t vdiff;

    for (k = 0U; k < 8U; k++)
    {
        /* Voltage error between value of logical and measurement */
        vdiff = (uint16_t)((uint32_t)(g_ctsu_diag_info.dac_voltage_value[k] * 100UL) / (uint32_t)dac_wots_table[k]);

        if ((vdiff < CTSU_CFG_DIAG_DAC_LDIFF) ||
            (vdiff > CTSU_CFG_DIAG_DAC_HDIFF))
        {
            return FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE;
        }
    }

    return FSP_SUCCESS;
}

  #else
static void ctsu_diag_dac_initial_tuning (void)
{
    int32_t  diff          = 0;
    uint32_t complete_flag = 0;
    uint16_t ctsuso;

    diff = (int32_t) g_ctsu_diag_info.correct_data - (int32_t) CTSU_DIAG_DAC_TARGET_VALUE;

    ctsuso = g_ctsu_diag_info.ctsuwr.ctsuso0 & CTSU_TUNING_MAX;
    if (0 < diff)
    {
        if (g_ctsu_diag_info.tuning_diff < 0)
        {
            if ((-diff) > g_ctsu_diag_info.tuning_diff)
            {
                ctsuso++;
            }

            complete_flag = 1;
        }
        else
        {
            if (CTSU_TUNING_MAX == ctsuso)
            {
                complete_flag = 1;
            }
            else
            {
                ctsuso++;
                g_ctsu_diag_info.tuning_diff = diff;
            }
        }
    }
    else if (0 == diff)
    {
        complete_flag = 1;
    }
    else
    {
        if (g_ctsu_diag_info.tuning_diff > 0)
        {
            if ((-diff) > g_ctsu_diag_info.tuning_diff)
            {
                ctsuso--;
            }

            complete_flag = 1;
        }
        else
        {
            complete_flag = 1;
        }
    }

    g_ctsu_diag_info.ctsuwr.ctsuso0 &= (uint16_t) (~CTSU_TUNING_MAX);
    g_ctsu_diag_info.ctsuwr.ctsuso0 |= ctsuso;

    if (CTSU_CFG_NUM_SUMULTI == complete_flag)
    {
        g_ctsu_diag_info.tuning_diff = 0;
        g_ctsu_diag_info.so0_4uc_val = ctsuso;
        g_ctsu_diag_info.dac_init    = 3;
        g_ctsu_diag_info.tuning      = CTSU_TUNING_COMPLETE;
        g_ctsu_diag_info.loop_count  = 0;
        g_ctsu_diag_info.dac_cnt[0]  = g_ctsu_diag_info.correct_data;
    }
}

static void ctsu_diag_dac_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint8_t temp;

    /* Set power on */
    CTSUCR1 = ((CTSU_CFG_LOW_VOLTAGE_MODE << 2) | (CTSU_CSW_ON << 1) | CTSU_PON_ON);

    /* Synchronous Noise Reduction Setting */
    CTSUSDPRS = ((CTSU_SOFF_ON << 6) | (CTSU_PRMODE_62_PULSES << 4) | CTSU_PRRATIO_RECOMMEND);

    /* High Pass Noise Reduction- ALWAYS 0x30 as per HW Manual */
    CTSUDCLKC = ((CTSU_SSCNT << 4) | CTSU_SSMOD);

    /* Sensor Stabilization- ALWAYS 0x10 as per HW Manual */
    CTSUSST = (CTSU_SST_RECOMMEND);

    /* Since CLK is rewritten by correction, set here. */
    CTSUCR1 |= (uint8_t) (CTSU_CFG_PCLK_DIVISION << 4);

    temp            = (uint8_t) (CTSUCR1 & ~(CTSU_CR1_MODIFY_BIT));
    CTSUCR1 = (uint8_t) (temp | (p_instance_ctrl->ctsucr1 & CTSU_CR1_MODIFY_BIT) | CTSU_DAC_TEST_ATUNE1); // MD1, MD0, ATUNE1=1
  #if BSP_FEATURE_CTSU_HAS_TXVSEL
    CTSUCR0 =
        (uint8_t) ((CTSUCR0 & ~(CTSU_TXVSEL)) | (p_instance_ctrl->p_ctsu_cfg->txvsel & CTSU_TXVSEL));
  #endif

    /* Write Channel setting */
  #if (CTSU_CFG_DIAG_DAC_TS >= 0) && (CTSU_CFG_DIAG_DAC_TS < 8)
    CTSUCHAC0 = 1 << CTSU_CFG_DIAG_DAC_TS;
  #else
    CTSUCHAC0 = 0;
  #endif
  #if (CTSU_CFG_DIAG_DAC_TS >= 8) && (CTSU_CFG_DIAG_DAC_TS < 16)
    CTSUCHAC1 = 1 << (CTSU_CFG_DIAG_DAC_TS - 8);
  #else
    CTSUCHAC1 = 0;
  #endif
  #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
  #if (CTSU_CFG_DIAG_DAC_TS >= 16) && (CTSU_CFG_DIAG_DAC_TS < 24)
    CTSUCHAC2 = 1 << (CTSU_CFG_DIAG_DAC_TS - 16);
  #else
    CTSUCHAC2 = 0;
  #endif
  #endif
  #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
  #if (CTSU_CFG_DIAG_DAC_TS >= 24) && (CTSU_CFG_DIAG_DAC_TS < 32)
    CTSUCHAC3 = 1 << (CTSU_CFG_DIAG_DAC_TS - 24);
  #else
    CTSUCHAC3 = 0;
   #endif
  #endif
  #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
  #if (CTSU_CFG_DIAG_DAC_TS >= 32)
    CTSUCHAC4  = 1 << (CTSU_CFG_DIAG_DAC_TS - 32);
  #else
    CTSUCHAC4  = 0;
   #endif
  #endif
    CTSUCHTRC0 = 0;
    CTSUCHTRC1 = 0;
   #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 3)
    CTSUCHTRC2 = 0;
   #endif
   #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 4)
    CTSUCHTRC3 = 0;
   #endif
   #if (BSP_FEATURE_CTSU_CTSUCHAC_REGISTER_COUNT >= 5)
    CTSUCHTRC4 = 0;
   #endif

    g_ctsu_diag_info.ctsuwr.ctsussc = (uint16_t) (CTSU_SSDIV_1330 << 8);
    g_ctsu_diag_info.ctsuwr.ctsuso1 = (uint16_t) ((CTSU_ICOG_RECOMMEND << 13) | (7 << 8) | CTSU_RICOA_RECOMMEND);

    if (g_ctsu_diag_info.dac_init > 2)
    {
        /* Apply DAC current */
        if (0 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 = (uint16_t) ((3 << 10) | g_ctsu_diag_info.so0_4uc_val);
        }
        else if (1 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) ((3 << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_1UC));
        }
        else if (2 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) ((3 << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_2UC));
        }
        else if (3 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) ((3 << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_4UC));
        }
        else if (4 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) ((3 << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_8UC));
        }
        else if (5 == g_ctsu_diag_info.loop_count)
        {
            g_ctsu_diag_info.ctsuwr.ctsuso0 =
                (uint16_t) ((3 << 10) | (g_ctsu_diag_info.so0_4uc_val - CTSU_DIAG_DAC_16UC));
        }
        else
        {
        }
    }

    if (g_ctsu_diag_info.dac_init == 0)
    {
        g_ctsu_diag_info.dac_init       = 1;
        g_ctsu_diag_info.so0_4uc_val    = 0;
        g_ctsu_diag_info.tuning_diff    = 0;
        g_ctsu_diag_info.ctsuwr.ctsuso0 = (3 << 10) + CTSU_DIAG_DAC_START_VALUE;
    }
}

static void ctsu_diag_dac_data_get (void)
{
    ctsu_correction_calc_t calc;

    calc.snum = (g_ctsu_diag_info.ctsuwr.ctsuso0 >> 10) & CTSU_SNUM_MAX;

    if (CTSU_DIAG_DAC == g_ctsu_diag_info.state)
    {
        calc.snum = 3;
    }

    calc.sdpa = (g_ctsu_diag_info.ctsuwr.ctsuso1 >> 8) & CTSU_SDPA_MAX;

    /* Correction process */
    ctsu_correction_calc(&g_ctsu_diag_info.correct_data, g_ctsu_diag_info.scanbuf.sen, &calc);

    if (g_ctsu_diag_info.tuning == CTSU_TUNING_COMPLETE)
    {
        g_ctsu_diag_info.dac_cnt[g_ctsu_diag_info.loop_count] = g_ctsu_diag_info.correct_data;
    }
    else
    {
        ctsu_diag_dac_initial_tuning();
    }
}

static fsp_err_t ctsu_diag_dac_result (void)
{
    uint8_t k;
    if (g_ctsu_diag_info.tuning == CTSU_TUNING_COMPLETE)
    {
        for (k = 0; k < 6; k++)
        {
            if ((g_ctsu_diag_info.dac_cnt[k] > dac_oscil_table[k][0]) ||
                (g_ctsu_diag_info.dac_cnt[k] < dac_oscil_table[k][1]))
            {
                return FSP_ERR_CTSU_DIAG_DAC;
            }
        }
    }

    /* if all checks passed to this point, return success */
    return FSP_SUCCESS;
}
  #endif

 #endif
#endif

#if (BSP_FEATURE_CTSU_VERSION == 2)
 #if (CTSU_CFG_DIAG_SUPPORT_ENABLE == 1)

/***********************************************************************************************************************
 * ctsu_diag_scan_start2
 ***********************************************************************************************************************/
static void ctsu_diag_scan_start2 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    fsp_err_t adc_err = FSP_SUCCESS;

    if ((CTSU_DIAG_INIT == g_ctsu_diag_info.state) || (CTSU_DIAG_ADC_ERROR == g_ctsu_diag_info.state))
    {
        adc_err = ctsu_diag_adc_open_check();
        if (FSP_SUCCESS == adc_err)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_OUTPUT_VOLTAGE;
        }
        else
        {
            g_ctsu_diag_info.state = CTSU_DIAG_ADC_ERROR;
        }
    }

    /* ctsu normal scan register save */
    ctsu_diag_regi_store2(p_instance_ctrl);

    /* scan register setting */
    if (CTSU_DIAG_OUTPUT_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_output_voltage_scan_start(p_instance_ctrl);
        if (FSP_SUCCESS == g_ctsu_diag_info.test_result)
        {
            if (0 == g_ctsu_diag_info.onetime_exec_flag)
            {
                g_ctsu_diag_info.state = CTSU_DIAG_OVER_VOLTAGE;
            }
            else
            {
                g_ctsu_diag_info.state = CTSU_DIAG_LOAD_RESISTANCE;
            }
        }
        else
        {
            g_ctsu_diag_info.state = CTSU_DIAG_COMPLETE;
            ctsu_diag_regi_restore2();
        }
    }

    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_over_voltage_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_OVER_CURRENT == g_ctsu_diag_info.state)
    {
        ctsu_diag_over_current_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_LOAD_RESISTANCE == g_ctsu_diag_info.state)
    {
        ctsu_diag_load_resistance_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_CURRENT_SOURCE == g_ctsu_diag_info.state)
    {
        ctsu_diag_current_source_scan_start(p_instance_ctrl);
    }

    if (CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state)
    {
        ctsu_diag_sensclk_gain_scan_start();
    }

    if (CTSU_DIAG_SUCLK == g_ctsu_diag_info.state)
    {
        ctsu_diag_suclk_gain_scan_start();
    }

    if (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
    {
        ctsu_diag_clock_recovery_scan_start();
    }
}

/***********************************************************************************************************************
 * ctsu_diag_data_get2
 ***********************************************************************************************************************/
static fsp_err_t ctsu_diag_data_get2 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    fsp_err_t err;

    if (CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state)
    {
        ctsu_diag_over_voltage_data_get();
    }

    if (CTSU_DIAG_OVER_CURRENT == g_ctsu_diag_info.state)
    {
        ctsu_diag_over_current_data_get();
    }

    if (CTSU_DIAG_LOAD_RESISTANCE == g_ctsu_diag_info.state)
    {
        ctsu_diag_load_resistance_data_get(p_instance_ctrl);
    }

    if (CTSU_DIAG_CURRENT_SOURCE == g_ctsu_diag_info.state)
    {
        ctsu_diag_current_source_data_get(p_instance_ctrl);
    }

    if (CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state)
    {
        ctsu_diag_sensclk_gain_data_get(p_instance_ctrl);
    }

    if (CTSU_DIAG_SUCLK == g_ctsu_diag_info.state)
    {
        ctsu_diag_suclk_gain_data_get(p_instance_ctrl);
    }

    if (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
    {
        ctsu_diag_clock_recovery_data_get(p_instance_ctrl);
    }

    /* register restore */
    ctsu_diag_regi_restore2();

    /* Diagnosis state transition */
    if ((CTSU_DIAG_OVER_VOLTAGE == g_ctsu_diag_info.state) &&
        (CTSU_DIAG_OVER_VOLTAGE_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        g_ctsu_diag_info.state      = CTSU_DIAG_OVER_CURRENT;
    }
    else if ((CTSU_DIAG_OVER_CURRENT == g_ctsu_diag_info.state) &&
             (CTSU_DIAG_OVER_CURRENT_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        g_ctsu_diag_info.state      = CTSU_DIAG_LOAD_RESISTANCE;
    }
    else if ((CTSU_DIAG_LOAD_RESISTANCE == g_ctsu_diag_info.state) &&
             (CTSU_DIAG_LOAD_RESISTANCE_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        if (0 == g_ctsu_diag_info.onetime_exec_flag)
        {
            g_ctsu_diag_info.state = CTSU_DIAG_CURRENT_SOURCE;
        }
        else
        {
            g_ctsu_diag_info.state = CTSU_DIAG_SENSCLK;
        }
    }
    else if ((CTSU_DIAG_CURRENT_SOURCE == g_ctsu_diag_info.state) &&
             (CTSU_DIAG_CURRENT_SOURCE_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count        = 0;
        g_ctsu_diag_info.state             = CTSU_DIAG_SENSCLK;
        g_ctsu_diag_info.onetime_exec_flag = 1;
    }
    else if ((CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state) &&
             (CTSU_DIAG_SENSCLK_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        g_ctsu_diag_info.state      = CTSU_DIAG_SUCLK;
    }
    else if ((CTSU_DIAG_SUCLK == g_ctsu_diag_info.state) && (CTSU_DIAG_SUCLK_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        g_ctsu_diag_info.state      = CTSU_DIAG_CLOCK_RECOVERY;
    }
    else if ((CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state) &&
             (CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM <= g_ctsu_diag_info.test_count))
    {
        g_ctsu_diag_info.test_count = 0;
        g_ctsu_diag_info.state = CTSU_DIAG_COMPLETE;
    }
    else
    {
        /* No processing */
    }

    if (FSP_SUCCESS != g_ctsu_diag_info.test_result)
    {
        g_ctsu_diag_info.state = CTSU_DIAG_COMPLETE;
    }

    if (CTSU_DIAG_COMPLETE == g_ctsu_diag_info.state)
    {
        err = FSP_SUCCESS;
    }
    else if (CTSU_DIAG_ADC_ERROR == g_ctsu_diag_info.state)
    {
        err = FSP_ERR_ABORTED;
    }
    else
    {
        err = FSP_ERR_CTSU_DIAG_NOT_YET;
    }

    return err;
}

static void ctsu_diag_regi_store2 (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Store registers settings */
    CTSU_LWORD_SET(g_ctsu_diag_reg.ctsucra, CTSUCRAH, CTSUCRAL);
    if ((CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_SUCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
        )
    {
        CTSU_LWORD_SET(g_ctsu_diag_reg.ctsucrb, CTSUCRBH, CTSUCRBL);
    }
    CTSU_LWORD_SET(g_ctsu_diag_reg.ctsucalib, CTSUDBGR1, CTSUDBGR0);
    CTSU_LWORD_SET(g_ctsu_diag_reg.ctsusuclka, CTSUSUCLK1, CTSUSUCLK0);
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    CTSU_LWORD_SET(g_ctsu_diag_reg.ctsuopt, CTSUOPTH, CTSUOPTL);
  #endif

    /* Preconfigure registers */
    if ((CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_SUCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
        )
    {
        CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
    }

    CTSUMCHH |= 1 << CTSUMCH_MCA0_Pos;
    CTSUMCHH &= ~CTSUMCH_MCA1_Msk;
    CTSUMCHH &= ~CTSUMCH_MCA2_Msk;
    CTSUMCHH &= ~CTSUMCH_MCA3_Msk;

    CTSU_LWORD_GET(CTSUDBGR1   , CTSUDBGR0   , 0UL );

    if (CTSU_DIAG_OVER_CURRENT != g_ctsu_diag_info.state)
    {
        CTSUDBGR0 |= 1 << CTSUCALIB_TSOC_Pos;
    }

  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    CTSUOPTL &= ~CTSUOPTL_CCOCFEN_Msk;
  #endif

    if ((CTSU_DIAG_OVER_VOLTAGE != g_ctsu_diag_info.state) &&
        (CTSU_DIAG_OVER_CURRENT != g_ctsu_diag_info.state) &&
        (CTSU_DIAG_CURRENT_SOURCE != g_ctsu_diag_info.state))
    {
        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (uint32_t) (CTSU_SNUM_RECOMMEND << 10);
    }
}

/* This function must be called when the state is the same as that of ctsu_diag_regi_store2(). */
static void ctsu_diag_regi_restore2 (void)
{
    /* register restore */
    CTSU_LWORD_GET(CTSUCRAH, CTSUCRAL, g_ctsu_diag_reg.ctsucra);
    if ((CTSU_DIAG_SENSCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_SUCLK == g_ctsu_diag_info.state) ||
        (CTSU_DIAG_CLOCK_RECOVERY == g_ctsu_diag_info.state)
        )
    {
        CTSU_LWORD_GET(CTSUCRBH, CTSUCRBL, g_ctsu_diag_reg.ctsucrb);
    }

    CTSU_LWORD_GET(CTSUDBGR1, CTSUDBGR0, g_ctsu_diag_reg.ctsucalib);
    CTSUCRAH &= ~CTSUCRA_SDPSEL_Msk;
    CTSU_LWORD_GET(CTSUSUCLK1, CTSUSUCLK0, g_ctsu_diag_reg.ctsusuclka);
    CTSUCRAH |= 1 << CTSUCRA_SDPSEL_Pos;
  #if (CTSU_CFG_AUTO_CORRECTION_ENABLE == 1)
    CTSU_LWORD_GET(CTSUOPTH, CTSUOPTL, g_ctsu_diag_reg.ctsuopt);
  #endif
}

static fsp_err_t ctsu_diag_adc_open_check (void)
{
    /* Check the register if hardware trigger is set or if conversion is in progress. */
#if defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
    if ((1U == ADCE) || (1U == ADCS))
#elif defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
    if ((_8000_AD_CONVERSION_START | _0200_AD_SYNCASYNCTRG_ENABLE) & ADCSR)
#endif
    {
        return FSP_ERR_ALREADY_OPEN;
    }

    return FSP_SUCCESS;
}

static void ctsu_diag_output_voltage_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Initialize local variable */
    uint16_t internal_vref_value = 0;
    uint16_t output_voltage_value = 0;
    uint16_t upper_threshold_value = 0;
    uint16_t lower_threshold_value = 0;

    /* SO must be set to 0 */
    CTSUSO0 = (CTSUSO0 & 0xFC00);

    /* 1. Measure Internal Reference Voltage (VREF) */
    /* Use ADC to measure VREF as a basis for threshold calculation */
    ctsu_diag_adc_measure_average(ADC_CHANNEL_VOLT, &internal_vref_value);

    /* 2. Calculate the upper and lower threshold values */
    upper_threshold_value = ctsu_diag_calc_threshold_output_voltage(true, internal_vref_value);
    lower_threshold_value = ctsu_diag_calc_threshold_output_voltage(false, internal_vref_value);

    /* 3. Test case configuration */
    for (g_ctsu_diag_info.test_count = 0;
         g_ctsu_diag_info.test_count < CTSU_DIAG_OUTPUT_VOLTAGE_TEST_NUM;
         g_ctsu_diag_info.test_count++)
    {
        switch (g_ctsu_diag_info.test_count)
        {
            case 0:
            {
                /* TEST1: IDLE current (NM/LV = 2.5uA/2uA), Current range = 20uA */
                CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
                CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
                CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 1:
            {
                /* TEST2: IDLE current (NM/LV = 2.5uA/2uA), Current range = 40uA */
                CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
                CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
                CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 2:
            {
                /* TEST3: IDLE current (NM/LV = 2.5uA/2uA), Current range = 80uA */
                CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
                CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
                CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 3:
            {
                /* TEST4: IDLE current (NM/LV = 2.5uA/2uA), Current range = 160uA */
                CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
                CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
                CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 4:
            {
                /* TEST5: Resistance Load Mode, Load Resistance = 60kOhm, Current range = 20uA */
                CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
                CTSUCRAH |= CTSUCRA_LOAD_Msk;
                CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
                CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 5:
            {
                /* TEST6: Resistance Load Mode, Load Resistance = 30kOhm, Current range = 40uA */
                CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
                CTSUCRAH |= CTSUCRA_LOAD_Msk;
                CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
                CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 6:
            {
                /* TEST7: Resistance Load Mode, Load Resistance = 15kOhm, Current range = 80uA */
                CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
                CTSUCRAH |= CTSUCRA_LOAD_Msk;
                CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
                CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
                break;
            }

            case 7:
            {
                /* TEST8: Resistance Load Mode, Load Resistance = 7.5kOhm, Current range = 160uA */
                CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
                CTSUCRAH |= CTSUCRA_LOAD_Msk;
                CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
                CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
                break;
            }

            /* All tests are complete. */
            default:
            {
                break;
            }
        }

        /* Select power supply calibration mode */
        CTSUDBGR0 |= CTSUCALIB_DRV_Msk;

        /* 4. Measure TSCAP Voltage */
        ctsu_diag_adc_measure_average(ADC_CHANNEL_TSCAP_VOLT, &output_voltage_value);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (output_voltage_value & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((output_voltage_value >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);
  #endif

        /* 5. Compare and process result */
        if ((output_voltage_value <= lower_threshold_value) || (output_voltage_value >= upper_threshold_value))
        {
            /* FAIL: Set diagnostic error */
            g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_OUTPUT_VOLTAGE;
        }

        /* PASS: Check is skipped because the measured value is within the expected range.
         * Condition: (output_voltage_value < upper_threshold_value) && (output_voltage_value > lower_threshold_value) */

        /* Reset for the next measurement */
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }

    /* Reset test count */
    g_ctsu_diag_info.test_count = 0;
}

/* Performs repeated ADC measurements (for RL78/G23, RL78/G22, RL78/L23) or a single averaged ADC measurement
   (for RL78/F25, RL78/F22) and calculates the final average value */
static void ctsu_diag_adc_measure_average (uint16_t channel, uint16_t *adc_measured_value)
{
#if defined(BSP_MCU_GROUP_RL78G23) || defined(BSP_MCU_GROUP_RL78G22) || defined(BSP_MCU_GROUP_RL78L23)
    uint8_t dummy_count = 0;

    /* Set Internal VREF measurement */
    R_Config_ADC_Create();

    /* Change AD channel if measure TSCAP */
    if (ADC_CHANNEL_TSCAP_VOLT == channel)    
    {
        R_Config_ADC_Set_ADChannel((e_ad_channel_t) channel);
    }

    g_ctsu_diag_info.measurement_sum = 0;

    /* To disable the first to third Internal VREF measurement */
    if (ADC_CHANNEL_VOLT == channel)
    {
        dummy_count = 3;
    }

    /* Measure 4times and average the measured data */
    for (g_ctsu_diag_info.measurement_count = 0;
         g_ctsu_diag_info.measurement_count < (CTSU_DIAG_AVERAGE_NUM + dummy_count);
         g_ctsu_diag_info.measurement_count++)
    {
        *adc_measured_value = 0;

        /* Start (Software trigger no-wait mode) */
        R_Config_ADC_Set_OperationOn();
        R_Config_ADC_Start();

        /* Wait for completion */
        while (1U == ADCS)
        {
        }
        /* Read measured value */
#if BSP_MCU_GROUP_RL78G22
        R_Config_ADC_Get_Result_10bit(adc_measured_value);
        *adc_measured_value = *adc_measured_value << 2;
#else
        R_Config_ADC_Get_Result_12bit(adc_measured_value);
#endif

        /* Discard the first to third Internal VREF measurement */
        if (!((ADC_CHANNEL_VOLT == channel) && (3 > g_ctsu_diag_info.measurement_count)))
        {
            /* Accumulate measurement value */
            g_ctsu_diag_info.measurement_sum += *adc_measured_value;
        }
    }

    /* Calculate the average value */
    *adc_measured_value = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

#elif defined(BSP_MCU_GROUP_RL78F25) || defined(BSP_MCU_GROUP_RL78F22)
    /* Set Internal VREF measurement */
    R_Config_S12AD0_Create();

    /* Change AD channel if measure TSCAP */
    if (ADC_CHANNEL_TSCAP_VOLT == channel)
    {
        ADWINR  = 0x00U;    /* Access window setting for the ADANSA1 register (0) */
        ADANSA1 = 0x2000U;  /* ANI29 (TSCAP) used */
        ADWINR  = 0x0DU;    /* Access window setting for the ADSSTRL register (D) */
        ADSSTRL = 0x21U;    /* Sampling time for ANI16-ANI29 setting */
    }

    /* Start (Software trigger) */
    R_Config_S12AD0_Start();

    /* Wait for completion */
    while (0 != (ADCSR & _8000_AD_CONVERSION_START))
    {
    }

    /* Read measured value */
    R_Config_S12AD0_Get_ValueResult((e_ad_channel_t) channel , adc_measured_value);
#endif

    /* ADC Close for System */
    ADCEN = 0;
}

/**
 * Description: Calculates the TSCAP output voltage threshold (upper or lower)
 *              based on the measured ADC value of the Internal VREF.
 *              This function computes either the upper or lower threshold value.
 *
 * Parameter: threshold_type : Specify which threshold to calculate
 *                  - true  : upper threshold
 *                  - false : lower threshold
 * Parameter: internal_vref_value : The measured internal reference voltage in ADC counts
 *
 * Return: The calculated threshold value (upper or lower threshold).
 *
 * Example usage:
 *   uint16_t upper_threshold_value = ctsu_diag_calc_threshold_output_voltage(true, internal_vref_value);
 *   uint16_t lower_threshold_value = ctsu_diag_calc_threshold_output_voltage(false, internal_vref_value);
 */
static uint16_t ctsu_diag_calc_threshold_output_voltage (bool threshold_type, uint16_t internal_vref_value)
{
    /* Initialize local variable */
    uint32_t threshold;

    /* Threshold calculation */
    if (threshold_type)
    {
        /* Upper threshold value = (((V_tscaptyp * (1 + Err_tscap) * (AD_measured + Err_adc)) /
         * (V_base * (1 - Err_base))) + Err_adc) * (1 + margin) */
        threshold = (uint32_t)
                    (((CTSU_DIAG_TSCAP_VREF_TYP_MV *
                       (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_TSCAP_VOLT_TOL_MAX_PCT) *
                       (internal_vref_value + CTSU_DIAG_ADC_MEASUREMENT_ERROR))
                      /
                      (CTSU_DIAG_INTERNAL_VREF_TYP_MV *
                       (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_INTERNAL_VOLT_TOL_PCT)) +
                      CTSU_DIAG_ADC_MEASUREMENT_ERROR) *
                     (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_OUTPUT_VOLTAGE_SELF_TEST_MARGIN_PCT)) /
                    CTSU_DIAG_PERCENT_BASE;
    }
    else
    {
        /* Lower threshold value = (((V_tscaptyp * (1 - Err_tscap) * (AD_measured - Err_adc)) /
         * (V_base * (1 + Err_base))) - Err_adc) * (1 - margin) */
        threshold = (uint32_t)
                    (((CTSU_DIAG_TSCAP_VREF_TYP_MV *
                       (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_TSCAP_VOLT_TOL_MIN_PCT) *
                       (internal_vref_value - CTSU_DIAG_ADC_MEASUREMENT_ERROR))
                      /
                      (CTSU_DIAG_INTERNAL_VREF_TYP_MV *
                       (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_INTERNAL_VOLT_TOL_PCT)) -
                      CTSU_DIAG_ADC_MEASUREMENT_ERROR) *
                     (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_OUTPUT_VOLTAGE_SELF_TEST_MARGIN_PCT)) /
                    CTSU_DIAG_PERCENT_BASE;
    }

    return (uint16_t) (threshold);
}

static void ctsu_diag_over_voltage_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Initialize local variable */
    uint32_t offset_current_value = 0;

    /* Pre-configuration for diagnosis */
    CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
    CTSUCRAH |= CTSUCRA_LOAD_Msk;
    CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
    CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;

    /* Test case configuration */
    if (0 == g_ctsu_diag_info.test_count)
    {
        /* TEST1: Normal voltage setting. Set Offset current = 0uA (SO = 0x0000).*/
        offset_current_value = CTSU_DIAG_OFFSET_CURRENT_0UA;
    }
    else
    {
        /* TEST2: Over voltage setting. Set Offset current = 80uA (SO = 0x0200). */
        offset_current_value = CTSU_DIAG_OFFSET_CURRENT_80UA;
    }

    /* Set CTSUSO value: Combine SO and SNUM. */
    p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso =
        offset_current_value | (CTSU_SNUM_RECOMMEND << 10);
}

static void ctsu_diag_over_voltage_data_get (void)
{
    /* Increment test count */
    g_ctsu_diag_info.test_count++;
}

static void ctsu_diag_over_current_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Initialize local variable */
    uint8_t sdpa_value = 0;

    /* Pre-configuration for diagnosis */
    CTSUDBGR0  = 0x0000U;
    CTSUDBGR1  = 0x0000U;

    /* Test case configuration */
    if (0 == g_ctsu_diag_info.test_count)
    {
        /* TEST1: Normal voltage setting */
        CTSUCRAL  |= CTSUCRA_ATUNE1_Msk;              /* Current range = 160uA */
        CTSUCRAH  |= CTSUCRA_ATUNE2_Msk;
        sdpa_value               = CTSU_CFG_SUMULTI0; /* Configure SDPA based on SUCLK multiplier */
    }
    else
    {
        /* TEST2: Over voltage setting */
        CTSUCRAL  &= ~CTSUCRA_ATUNE1_Msk;                                         /* Current range = 20uA */
        CTSUCRAH  |= CTSUCRA_ATUNE2_Msk;
        sdpa_value               = p_instance_ctrl->p_ctsu_cfg->p_elements->sdpa; /* Use SDPA from QE configuration structure */
    }

    /* Set CTSUSO value: Combine SO, SNUM, and SDPA (SUCLK Divider) */
    p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso =
        (CTSU_DIAG_OFFSET_CURRENT_0UA | (CTSU_SNUM_RECOMMEND << 10) | ((uint32_t) sdpa_value << 24));
}

static void ctsu_diag_over_current_data_get (void)
{
    /* Increment test count */
    g_ctsu_diag_info.test_count++;
}

static void ctsu_diag_load_resistance_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Test case configuration */

    if (g_ctsu_diag_info.test_count <= 3)
    {
        /* Load resistance ON for TEST1 through TEST4 */
        CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
        CTSUCRAH |= CTSUCRA_LOAD_Msk;
    }

    switch (g_ctsu_diag_info.test_count)
    {
        case 0:
        {
            /* TEST1: Load resistance = 60kOhm, Current range = 20uA */
            CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
            CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
            p_instance_ctrl->range   = CTSU_RANGE_20UA;
            break;
        }

        case 1:
        {
            /* TEST2: Load resistance = 30kOhm, Current range = 40uA */
            CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
            CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
            p_instance_ctrl->range   = CTSU_RANGE_40UA;
            break;
        }

        case 2:
        {
            /* TEST3: Load resistance = 15kOhm, Current range = 80uA */
            CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
            CTSUCRAH &= ~CTSUCRA_ATUNE2_Msk;
            p_instance_ctrl->range   = CTSU_RANGE_80UA;
            break;
        }

        case 3:
        {
            /* TEST4: Load resistance = 7.5kOhm, Current range = 160uA */
            CTSUCRAL |= CTSUCRA_ATUNE1_Msk;
            CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
            p_instance_ctrl->range   = CTSU_RANGE_160UA;
            break;
        }

        case 4:
        {
            /* TEST5: IDLE current, Current range = 20uA */
            CTSUCRAH &= ~CTSUCRA_LOAD_Msk;

            CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
            CTSUCRAH |= CTSUCRA_ATUNE2_Msk;
            p_instance_ctrl->range   = CTSU_RANGE_20UA;
            break;
        }

        /* All tests are complete. */
        default:
        {
            break;
        }
    }
}

static void ctsu_diag_load_resistance_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Initialize local variable */
    uint16_t load_resistance_value       = 0;
    uint16_t upper_threshold_value       = 0;
    uint16_t lower_threshold_value       = 0;
    uint32_t theoretical_value_variation = 0;

    /* Get raw count value and apply correction */
    ctsu_correction_calc_t calc;
    calc.snum  = CTSU_SNUM_RECOMMEND;
    calc.range = p_instance_ctrl->range;
    calc.md    = CTSU_MODE_DIAGNOSIS_SCAN;
    ctsu_correction_calc(&load_resistance_value, (uint16_t) (*(p_instance_ctrl->p_self_raw)), &calc);

    /* Accumulate sum and count */
    g_ctsu_diag_info.measurement_sum += load_resistance_value;
    g_ctsu_diag_info.measurement_count++;

    /* Check if enough samples have been collected for averaging */
    if (g_ctsu_diag_info.measurement_count >= CTSU_DIAG_AVERAGE_NUM)
    {
        /* 1. Calculate the average value */
        load_resistance_value = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (load_resistance_value & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((load_resistance_value >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);
  #endif

        /* 2. Store the first measurement value */
        if (0 == g_ctsu_diag_info.test_count)
        {
            g_ctsu_diag_info.load_resistance = load_resistance_value;
        }

        /* 3. Calculate thresholds */
        if (4 == g_ctsu_diag_info.test_count) /* TEST5: For IDLE current test */
        {
            upper_threshold_value = (uint16_t) ((CTSU_DIAG_IDLE_CURRENT_THEORETICAL_COUNT_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_IDLE_CURRENT_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_IDLE_CURRENT_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));

            lower_threshold_value = (uint16_t) ((CTSU_DIAG_IDLE_CURRENT_THEORETICAL_COUNT_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_IDLE_CURRENT_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_IDLE_CURRENT_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
        }
        else                           /* TEST1 through TEST4: For load resistance test */
        {
            theoretical_value_variation =
                (uint32_t) ((CTSU_DIAG_LOAD_RESISTANCE_THEORETICAL_COUNT_VALUE <<
                             CTSU_DIAG_LOAD_RESISTANCE_VARIATION_SCALE_SHIFT) /
                            g_ctsu_correction_info.error_rate[g_ctsu_diag_info.test_count]);

            upper_threshold_value = (uint16_t) ((theoretical_value_variation *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_LOAD_RESISTANCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_LOAD_RESISTANCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));

            lower_threshold_value = (uint16_t) ((theoretical_value_variation *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_LOAD_RESISTANCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_LOAD_RESISTANCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
        }

        /* 4. Compare and process result */
        if ((load_resistance_value <= lower_threshold_value) || (load_resistance_value >= upper_threshold_value))
        {
            /* FAIL: Set diagnostic error */
            g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_LOAD_RESISTANCE;
        }

        /* PASS: Check is skipped because the measured value is within the expected range.
         * Condition: (load_resistance_value < upper_threshold_value) && (load_resistance_value > lower_threshold_value) */

        /* Increment test cycle count and Reset for the next measurement */
        g_ctsu_diag_info.test_count++;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }

    /* Wait for enough samples, exit function */
}

static void ctsu_diag_current_source_scan_start (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* DAC initial setting */
    CTSUDBGR1 |= CTSUCALIB_DACMSEL_Msk;
    CTSUDBGR1 |= CTSUCALIB_DACCARRY_Msk;

    /* LDO Nch setting (20uAmode) */
    CTSUCRAH  = (CTSUCRAH & ~CTSUCRA_LOAD_Msk) | 1 << CTSUCRA_LOAD_Pos;
    CTSUCRAH |= CTSUCRA_LOAD_Msk;
    CTSUCRAL &= ~CTSUCRA_ATUNE1_Msk;
    CTSUCRAH |= CTSUCRA_ATUNE2_Msk;

    /* TEST1 through TEST8 */
    if (g_ctsu_diag_info.test_count <= 7)
    {
        /* Upper Current source setting (10uA) */
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT | (1 << g_ctsu_diag_info.test_count);
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;

        /* Setting time of measurement */
        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (CTSU_SNUM_RECOMMEND << 10) |
                                                                      CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;
    }
    /* TEST9 through TEST16 */
    else if (g_ctsu_diag_info.test_count <= 15)
    {
        /* Upper Current source setting (10uA) */
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT | (1 << (g_ctsu_diag_info.test_count - 8));

        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (CTSU_SNUM_RECOMMEND << 10) |
                                                                      (CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT |
                                                                       (1 << (g_ctsu_diag_info.test_count - 8)));
    }
    /* TEST17 */
    else if (16 == g_ctsu_diag_info.test_count)
    {
        /* Lower Current source setting (1.25uA) */
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT | CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_DATA_MAX;
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT | CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_DATA_MAX;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;

        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (CTSU_SNUM_RECOMMEND << 10) |
                                                                      CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;
    }
    /* TEST18 through TEST25 */
    else if (g_ctsu_diag_info.test_count <= 24)
    {
        /* Lower Current source setting (1.25uA) */
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT |
                  (CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_DATA_MAX & ~(1 << (g_ctsu_diag_info.test_count - 17)));
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT | CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_DATA_MAX;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;

        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (CTSU_SNUM_RECOMMEND << 10) |
                                                                      CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;
    }
    /* TEST26 through TEST27 */
    else
    {
        /* Lower Current source setting (1.25uA) */
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_SELECT | CTSU_DIAG_DAC_LOWER_CURRENT_LOW_UNIT_DATA_MAX;
        CTSUSO0 = CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_SELECT |
                  (CTSU_DIAG_DAC_LOWER_CURRENT_HIGH_UNIT_DATA_MAX & ~(1 << (g_ctsu_diag_info.test_count - 25)));
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_LOW_UNIT_SELECT;
        CTSUSO0 = CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;

        p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso = (CTSU_SNUM_RECOMMEND << 10) |
                                                                      CTSU_DIAG_DAC_UPPER_CURRENT_HIGH_UNIT_SELECT;
    }
}

static void ctsu_diag_current_source_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t current_source_value;
    uint16_t average_data;
    uint16_t current_source_diff_value = 0;
    uint16_t upper_threshold_value;
    uint16_t lower_threshold_value;

    ctsu_correction_calc_t calc;
    calc.snum  = CTSU_SNUM_RECOMMEND;
    calc.range = CTSU_RANGE_20UA;
    calc.md    = CTSU_MODE_DIAGNOSIS_SCAN;
    ctsu_correction_calc(&current_source_value, *(p_instance_ctrl->p_self_raw), &calc);

    g_ctsu_diag_info.measurement_sum   += current_source_value;
    g_ctsu_diag_info.measurement_count += 1;

    if (CTSU_DIAG_AVERAGE_NUM <= g_ctsu_diag_info.measurement_count)
    {
        average_data = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (average_data & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((average_data >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);
  #endif

        /* Upper current source test */
        if (15 >= g_ctsu_diag_info.test_count)
        {
            if (g_ctsu_diag_info.load_resistance > average_data)
            {
                current_source_diff_value = g_ctsu_diag_info.load_resistance - average_data;
            }

            upper_threshold_value = (uint16_t) ((CTSU_DIAG_UPPER_CURRENT_SOURCE_DIFF_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_UPPER_CURRENT_SOURCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_UPPER_CURRENT_SOURCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
            lower_threshold_value = (uint16_t) ((CTSU_DIAG_UPPER_CURRENT_SOURCE_DIFF_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_UPPER_CURRENT_SOURCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_UPPER_CURRENT_SOURCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
        }
        else if (16 == g_ctsu_diag_info.test_count)
        {
            g_ctsu_diag_info.average_data_pre = average_data;
        }
        /* Lower current source test */
        else
        {
            if (average_data > g_ctsu_diag_info.average_data_pre)
            {
                current_source_diff_value = average_data - g_ctsu_diag_info.average_data_pre;
            }

            upper_threshold_value = (uint16_t) ((CTSU_DIAG_LOWER_CURRENT_SOURCE_DIFF_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_LOWER_CURRENT_SOURCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE +
                                                  CTSU_DIAG_LOWER_CURRENT_SOURCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
            lower_threshold_value = (uint16_t) ((CTSU_DIAG_LOWER_CURRENT_SOURCE_DIFF_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_LOWER_CURRENT_SOURCE_SELF_TEST_MARGIN_PCT) *
                                                 (CTSU_DIAG_PERCENT_BASE -
                                                  CTSU_DIAG_LOWER_CURRENT_SOURCE_SPEC_MARGIN_PCT)) /
                                                (CTSU_DIAG_PERCENT_BASE * CTSU_DIAG_PERCENT_BASE));
        }

        if (16 != g_ctsu_diag_info.test_count)
        {
            if ((current_source_diff_value <= lower_threshold_value) ||
                (current_source_diff_value >= upper_threshold_value))
            {
                g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_CURRENT_SOURCE;
            }
        }

        g_ctsu_diag_info.test_count       += 1;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }
}

static void ctsu_diag_sensclk_gain_scan_start (void)
{
    /* Pre-configuration for diagnosis */
    CTSUDBGR1 |= CTSUCALIB_CCOCALIB_Msk;

    /* Test case configuration */
    ctsu_diag_cco_gain_config_test();
}

static void ctsu_diag_sensclk_gain_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    /* Initialize local variable */
    uint16_t average_data;
    uint16_t sensclk_gain_diff_value = 0;
    uint16_t upper_threshold_value;
    uint16_t lower_threshold_value;

    /* Get raw count value, accumulate sum and count */
    g_ctsu_diag_info.measurement_sum += (uint16_t) (*(p_instance_ctrl->p_self_raw));
    g_ctsu_diag_info.measurement_count++;

    /* Check if enough samples have been collected for averaging */
    if (g_ctsu_diag_info.measurement_count >= CTSU_DIAG_AVERAGE_NUM)
    {
        /* 1. Calculate the average value */
        average_data = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (average_data & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((average_data >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);
  #endif

        if (0 != g_ctsu_diag_info.test_count)
        {
            /* 2. Calculate thresholds difference value */
            if (average_data > g_ctsu_diag_info.average_data_pre)
            {
                sensclk_gain_diff_value = average_data - g_ctsu_diag_info.average_data_pre;
            }

            /* 3. Calculate thresholds */
            upper_threshold_value = (uint16_t) (
                ((uint32_t) cco_gain_threshold_diff_table[(g_ctsu_diag_info.test_count - 1)][1] *
                 (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_SENSCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                CTSU_DIAG_PERCENT_BASE);

            lower_threshold_value = (uint16_t) (
                ((uint32_t) cco_gain_threshold_diff_table[(g_ctsu_diag_info.test_count - 1)][0] *
                 (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_SENSCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                CTSU_DIAG_PERCENT_BASE);

            /* 4. Compare and process result */
            if ((sensclk_gain_diff_value >= upper_threshold_value) ||
                (sensclk_gain_diff_value <= lower_threshold_value))
            {
                /* FAIL: Set diagnostic error */
                g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_SENSCLK_GAIN;
            }

            /* PASS: Check is skipped because the threshold difference value is within the expected range.
             * Condition: (sensclk_gain_diff_value < upper_threshold_value) && (sensclk_gain_diff_value > lower_threshold_value) */
        }

        /* Store the measurement value */
        g_ctsu_diag_info.average_data_pre = average_data;

        /* Increment test cycle count and Reset for the next measurement */
        g_ctsu_diag_info.test_count++;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }

    /* Wait for enough samples, exit function */
}

static void ctsu_diag_cco_gain_config_test (void)
{
    /* Test case configuration */
    CTSUCRBH &= ~CTSUCRB_SSCNT_Msk;
    CTSUCRAH &= ~CTSUCRA_SDPSEL_Msk;

    if (g_ctsu_diag_info.test_count < 8)
    {
        /* TEST1 through TEST8: Configure CCODAC Gain from 1 unit (0x1F) up to 8 units (0xFF)
         * Value = 0x1F | (count << 5) (Value = 0x1F + count * 0x20) */
        CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) |
            (uint8_t) (CTSU_DIAG_CCODAC_GAIN_1_UNIT | (g_ctsu_diag_info.test_count << 5));
    }
    else
    {
        /* Apply CCODAC Gain 8 units (0xFF) for all remaining TEST9 ~ TEST12.
         * The switch only handles auxiliary register configurations (SSCNT|SUCARRY) */
        CTSUSUCLK0 |= CTSU_DIAG_CCODAC_GAIN_8_UNIT;

        switch (g_ctsu_diag_info.test_count)
        {
            case 8:
            {
                /* TEST9: CCODAC Gain 8 units. Add 0x20 to SUADJD by setting SSCNT = 1. */
                CTSUCRBH = (CTSUCRBH & ~CTSUCRB_SSCNT_Msk) | (1 << CTSUCRB_SSCNT_Pos);
                break;
            }

            case 9:
            {
                /* TEST10: CCODAC Gain 8 units. Add 0x40 to SUADJD by setting SSCNT = 2. */
                CTSUCRBH = (CTSUCRBH & ~CTSUCRB_SSCNT_Msk) | (2 << CTSUCRB_SSCNT_Pos);
                break;
            }

            case 10:
            {
                /* TEST11: CCODAC Gain 8 units. Add 0x60 to SUADJD by setting SSCNT = 3. */
                CTSUCRBH = (CTSUCRBH & ~CTSUCRB_SSCNT_Msk) | (3 << CTSUCRB_SSCNT_Pos);
                break;
            }

            case 11:
            {
                /* TEST12: CCODAC Gain 8 units. Add 0x80 to SUADJD (SSCNT=3 for 0x60 + SUCARRY=1 for 0x20). */
                CTSUCRBH = (CTSUCRBH & ~CTSUCRB_SSCNT_Msk) | (3 << CTSUCRB_SSCNT_Pos);
                CTSUDBGR1 |= CTSUCALIB_SUCARRY_Msk;
                break;
            }

            /* All tests are complete. */
            default:
            {
                break;
            }
        }
    }

    CTSUCRAH |= CTSUCRA_SDPSEL_Msk;
}

static void ctsu_diag_suclk_gain_scan_start (void)
{
    if (11 >= g_ctsu_diag_info.test_count)
    {
        CTSUDBGR1 &= ~CTSUCALIB_SUMSEL_Msk;
        ctsu_diag_cco_gain_config_test();
    }
    else
    {
        CTSUDBGR1 |= CTSUCALIB_SUCARRY_Msk;
        CTSUDBGR1 |= CTSUCALIB_SUMSEL_Msk;
        ctsu_diag_suclk_gain_config_test();
    }
}

static void ctsu_diag_suclk_gain_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t average_data;
    uint16_t suclk_gain_diff_value = 0;
    uint16_t upper_threshold_value;
    uint16_t lower_threshold_value;

    g_ctsu_diag_info.measurement_sum   += *(p_instance_ctrl->p_self_raw);
    g_ctsu_diag_info.measurement_count += 1;

    if (CTSU_DIAG_AVERAGE_NUM <= g_ctsu_diag_info.measurement_count)
    {
        average_data = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (average_data & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((average_data >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);
  #endif

        if (0 == g_ctsu_diag_info.test_count)
        {
            g_ctsu_diag_info.average_data_pre = average_data;
        }
        else if (g_ctsu_diag_info.test_count <= 11)
        {
            if (average_data > g_ctsu_diag_info.average_data_pre)
            {
                suclk_gain_diff_value = average_data - g_ctsu_diag_info.average_data_pre;
            }

            g_ctsu_diag_info.average_data_pre = average_data;

            upper_threshold_value = (uint16_t) (
                ((uint32_t) cco_gain_threshold_diff_table[(g_ctsu_diag_info.test_count - 1)][1] *
                 (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_SUCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                CTSU_DIAG_PERCENT_BASE);
            lower_threshold_value = (uint16_t) (
                ((uint32_t) cco_gain_threshold_diff_table[(g_ctsu_diag_info.test_count - 1)][0] *
                 (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_SUCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                CTSU_DIAG_PERCENT_BASE);

            if ((suclk_gain_diff_value <= lower_threshold_value) ||
                (suclk_gain_diff_value >= upper_threshold_value))
            {
                g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_SUCLK_GAIN;
            }
        }
        else
        {
            upper_threshold_value = (uint16_t) ((CTSU_DIAG_NO_DWA_SUCLK_GAIN_MAX_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_SUCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                                                CTSU_DIAG_PERCENT_BASE);
            lower_threshold_value = (uint16_t) ((CTSU_DIAG_NO_DWA_SUCLK_GAIN_MIN_VALUE *
                                                 (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_SUCLK_GAIN_SELF_TEST_MARGIN_PCT)) /
                                                CTSU_DIAG_PERCENT_BASE);

            if ((average_data <= lower_threshold_value) ||
                (average_data >= upper_threshold_value))
            {
                g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_SUCLK_GAIN;
            }
        }

        g_ctsu_diag_info.test_count       += 1;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }
}

static void ctsu_diag_suclk_gain_config_test (void)
{
    /* TEST13 through TEST18 */
    if (g_ctsu_diag_info.test_count <= 17)
    {
        CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) |
                     (CTSU_DIAG_SUCLK_LOWER_CURRENT_SELECT | (1 << (g_ctsu_diag_info.test_count - 12)));
        CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) | CTSU_DIAG_SUCLK_UPPER_CURRENT_SELECT;
    }
    /* TEST19 through TEST24 */
    else
    {
        CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) | CTSU_DIAG_SUCLK_LOWER_CURRENT_SELECT;
        CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) |
                     (CTSU_DIAG_SUCLK_UPPER_CURRENT_SELECT | (1 << (g_ctsu_diag_info.test_count - 18)));
    }
}

static void ctsu_diag_clock_recovery_scan_start (void)
{
    uint8_t sumulti  = 0;
    uint8_t freq_mhz = 0;

    CTSUDBGR1 |= CTSUCALIB_CCOCLK_Msk;
    CTSUCRBL   = (CTSUCRBL & ~CTSUCRB_SST_Msk) | (CTSU_DIAG_CLOCK_RECOVERY_SST_RECOMMEND << CTSUCRB_SST_Pos);
    CTSUCRAH  &= ~CTSUCRA_SDPSEL_Msk;

    switch (g_ctsu_diag_info.test_count)
    {
        case 0:
        {
            CTSUCRBH &= ~CTSUCRB_SSCNT_Msk;
            sumulti   = CTSU_DIAG_CLOCK_RECOVERY_TEST1_FREQUENCY;
            freq_mhz  = CTSU_DIAG_CLOCK_RECOVERY_TEST1_FREQUENCY_MHZ;
            break;
        }

        case 1:
        {
            CTSUCRBH &= ~CTSUCRB_SSCNT_Msk;
            sumulti   = CTSU_DIAG_CLOCK_RECOVERY_TEST2_FREQUENCY;
            freq_mhz  = CTSU_DIAG_CLOCK_RECOVERY_TEST2_FREQUENCY_MHZ;
            break;
        }

        case 2:
        {
            CTSUCRBH &= ~CTSUCRB_SSCNT_Msk;
            sumulti   = CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY;
            freq_mhz  = CTSU_DIAG_CLOCK_RECOVERY_TEST3_FREQUENCY_MHZ;
            break;
        }

  #if (CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM == 4)
        case 3:
        {
            CTSUCRBH = (CTSUCRBH & ~CTSUCRB_SSCNT_Msk) | (1 << CTSUCRB_SSCNT_Pos);
            sumulti  = CTSU_DIAG_CLOCK_RECOVERY_TEST4_FREQUENCY;
            freq_mhz = CTSU_DIAG_CLOCK_RECOVERY_TEST4_FREQUENCY_MHZ;
            break;
        }
  #endif

        default:
        {
            break;
        }
    }

    CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUMULTI0_Msk) | (sumulti << CTSUSUCLKA_SUMULTI0_Pos);
    CTSUSUCLK0 = (CTSUSUCLK0 & ~CTSUSUCLKA_SUADJ0_Msk) |
                 ((((CTSUTRIM1 & CTSUTRIM1_SUADJD_Msk) * freq_mhz) >>
                    CTSU_DIAG_CLOCK_RECOVERY_FREQUENCY_32MHZ_SHIFT) -
                   (CTSU_SUADJ_SSCNT_ADJ * ((CTSUCRBH & CTSUCRB_SSCNT_Msk) >> CTSUCRB_SSCNT_Pos)));

    CTSUCRAH |= CTSUCRA_SDPSEL_Msk;
}

static void ctsu_diag_clock_recovery_data_get (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t average_data;
    uint16_t upper_threshold_value;
    uint16_t lower_threshold_value;

    g_ctsu_diag_info.measurement_sum   += *(p_instance_ctrl->p_self_raw);
    g_ctsu_diag_info.measurement_count += 1;

    if (CTSU_DIAG_AVERAGE_NUM <= g_ctsu_diag_info.measurement_count)
    {
        average_data = (uint16_t) (g_ctsu_diag_info.measurement_sum >> CTSU_DIAG_AVERAGE_SHIFT_NUM);

  #if (CTSU_DIAGNOSIS_DEBUG_ENABLE == 1)
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) (average_data & CTSU_DIAG_DEBUG_8BIT_MASK);
        g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] =
            (uint8_t) ((average_data >> CTSU_DIAG_DEBUG_HIGH_8BIT_SHIFT) & CTSU_DIAG_DEBUG_8BIT_MASK);

   #if (CTSU_DIAG_CLOCK_RECOVERY_TEST_NUM == 3)

        /* For MCU with 3 tests, fill the 4th data with zero. */
        if (2 == g_ctsu_diag_info.test_count)
        {
            g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] = 0x00;
            g_ctsu_diag_info.debug_buffer[g_ctsu_diag_info.buffer_index++] = 0x00;
        }
   #endif
  #endif

        upper_threshold_value = (uint16_t) (((uint32_t) clock_recovery_threshold_table[g_ctsu_diag_info.test_count][1] *
                                             (CTSU_DIAG_PERCENT_BASE + CTSU_DIAG_CLOCK_RECOVERY_SELF_TEST_MARGIN_PCT)) /
                                            CTSU_DIAG_PERCENT_BASE);
        lower_threshold_value = (uint16_t) (((uint32_t) clock_recovery_threshold_table[g_ctsu_diag_info.test_count][0] *
                                             (CTSU_DIAG_PERCENT_BASE - CTSU_DIAG_CLOCK_RECOVERY_SELF_TEST_MARGIN_PCT)) /
                                            CTSU_DIAG_PERCENT_BASE);

        if ((average_data <= lower_threshold_value) ||
            (average_data >= upper_threshold_value))
        {
            g_ctsu_diag_info.test_result = FSP_ERR_CTSU_DIAG_CLOCK_RECOVERY;
        }

        g_ctsu_diag_info.test_count       += 1;
        g_ctsu_diag_info.measurement_count = 0;
        g_ctsu_diag_info.measurement_sum   = 0;
    }
}
 #endif
#endif
