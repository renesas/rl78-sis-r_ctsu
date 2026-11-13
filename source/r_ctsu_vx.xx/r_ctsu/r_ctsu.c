/**********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
* THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
* Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
*********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#ifdef QE_TOUCH_CONFIGURATION
 #include "qe_touch_define.h"
#endif
#include "r_ctsu.h"
#if (CTSU_CFG_DTC_USE_SC == 1)
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_DTC.h"
#endif

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** "CTSU" in ASCII, used to determine if device is open. */
#define CTSU_OPEN                          (0x43545355U)

/* Macro definitions for register setting */
#define CTSU_PON_OFF                       (0)    // CTSU hardware macro power off
#define CTSU_PON_ON                        (1)    // CTSU hardware macro power on
#define CTSU_CSW_OFF                       (0)    // Capacitance switch turned off
#define CTSU_CSW_ON                        (1)    // Capacitance switch turned on

#define CTSU_CR1_MODIFY_BIT                (0xC8) // MD1, MD0, ATUNE1
#define CTSU_SOVF                          (0x20) // Overflow bit
#define CTSU_CORRECTION_AVERAGE            (32)
#define CTSU_SHIFT_AMOUNT                  (15)
#define CTSU_COUNT_MAX                     (0xFFFF)
#define CTSU_PCLKB_FREQ_MHZ                (1000000)
#define CTSU_PCLKB_FREQ_RANGE1             (32)
#define CTSU_PCLKB_FREQ_RANGE2             (64)
#define CTSU_PCLKB_FREQ_RANGE3             (128)
#define CTSU_WAIT_TIME                     (500)

/* Macro definitions for initial offset tuning */
#define CTSU_TUNING_MAX                    (0x03FF)
#define CTSU_TUNING_MIN                    (0x0000)
#define CTSU_TUNING_VALUE_SELF             (15360)
#define CTSU_TUNING_VALUE_MUTUAL           (10240)
#define CTSU_TUNING_OT_COUNT               (25)

#define CTSU_SST_RECOMMEND                 (0x1F) // The recommend value of SST
#define CTSU_SST_RECOMMEND_CURRENT         (0x3F) // The recommend value of SST with current
#define CTSU_SNUM_RECOMMEND                (0x07) // The value of SNUM should be fixed
#define CTSU_SNUM_MAX                      (0xFF) // The maximum value of SNUM
#define CTSU_ICOMP0                        (0x80) // ICOMP0 bit
#define CTSU_ICOMP1                        (0x40) // ICOMP1 bit
#define CTSU_ICOMPRST                      (0x20) // ICOMPRST bit
#define CTSU_CR0_MODIFY_BIT                (0xC0) // TXVSEL
#define CTSU_CR2_MODIFY_BIT                (0x33) // POSEL, ATUNE2, MD2
#define CTSU_SUADJ_MAX                     (0xFF) // The maximum value of SUADJx
#define CTSU_SUADJ_SSCNT_ADJ               (0x20) // The value of Adjusting SCADJx by SSCNT
#define CTSU_MUTUAL_BUF_SIZE               (CTSU_CFG_NUM_SUMULTI * 2)

/* Macro definitions for correction */
#if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
 #define CTSU_CORRECTION_STD_VAL           (19200) // 20UC standard value
 #define CTSU_CORRECTION_STD_UNIT          (1920)  // 2UC value
  #define CTSU_CORRECTION_STD_EXREG          (14400) // External registance standard value
 #define CTSU_CORRECTION_OFFSET_UNIT       (120)   // (7680 / 64)
#else
  #define CTSU_CORRECTION_STD_VAL          (15360) // 20UC standard value * 0.8
  #define CTSU_CORRECTION_STD_UNIT         (1536)  // 2UC value * 0.8
  #define CTSU_CORRECTION_STD_EXREG          (11520) // External registance standard value
  #define CTSU_CORRECTION_OFFSET_UNIT      (96)    // (7680 / 64) * 0.8
#endif
#define CTSU_CORRECTION_SUMULTI            (0x20)  // SUMULTI step
#define CTSU_CORRECTION_TRIMB_MAX          (0xFF)
#define CTSU_CORRECTION_TRIMB_SIGN_BIT     (0x80)
#define CTSU_CORRECTION_RTRIM_THRESHOLD1   (0xA0)
#define CTSU_CORRECTION_RTRIM_THRESHOLD2   (0x50)
#define CTSU_CORRECTION_TRIMB_THRESHOLD1   (0xC0)
#define CTSU_CORRECTION_TRIMB_THRESHOLD2   (0x3F)
 #define CTSU_CORRECTION_BIT16               (0x10000)
#define CTSU_CORRECTION_BIT10              (0x0400)
#define CTSU_CORRECTION_BIT9               (0x0200)
#define CTSU_CORRECTION_BIT8               (0x0100)
#define CTSU_CORRECTION_BIT7               (0x0080)

#if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
 #if (CTSU_CFG_LOW_VOLTAGE_MODE == 0)
  #define CTSU_CALIB_REF                   ((6144000 * 10) / CTSU_CFG_VCC_MV) // 1.5V Reference value (4096 * 1500 * 10)
 #else
  #define CTSU_CALIB_REF                   ((4915200 * 10) / CTSU_CFG_VCC_MV) // 1.2V Reference value (4096 * 1200 * 10)
#endif
 #define CTSU_CALIB_AVERAGE_TIME           (64)                               // ADC average time
 #define CTSU_CALIB_THRESHOLD              ((0x1000 * 4) / CTSU_CFG_VCC_MV)   // RTRIM calib threshold
 #define CTSU_CALIB_CTSUSO                 (0x3C0)                            // 150uA offset
 #endif


/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef struct st_ctsu_correction_calc
{
    ctsu_range_t range;
    uint16_t     snum;
    uint16_t     sdpa;
    ctsu_md_t    md;
} ctsu_correction_calc_t;

typedef struct st_ctsu_correction_multi
{
    uint16_t pri[CTSU_CFG_NUM_SUMULTI];
    uint16_t snd[CTSU_CFG_NUM_SUMULTI];
    uint32_t offset[CTSU_CFG_NUM_SUMULTI];
    int32_t  offset_error[CTSU_CFG_NUM_SUMULTI];
    bool     judge_disable;
} ctsu_correction_multi_t;

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
static fsp_err_t ctsu_transfer_open(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_transfer_close(ctsu_instance_ctrl_t * const p_instance_ctrl);
static fsp_err_t ctsu_transfer_configure(ctsu_instance_ctrl_t * const p_instance_ctrl);

#endif
static void ctsu_initial_offset_tuning(ctsu_instance_ctrl_t * const p_instance_ctrl);
static void ctsu_moving_average(uint16_t * p_average, uint16_t new_data, uint16_t average_num);
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
static void ctsu_correction_calc(uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc);
static void ctsu_correction_exec(ctsu_instance_ctrl_t * const p_instance_ctrl);

static void ctsu_correction_multi(ctsu_correction_multi_t * p_multi, uint16_t * p_pri, uint16_t * p_snd);
static void ctsu_delay(uint16_t delay_us);

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
static void ctsu_correction_scan_start(void);
static void ctsu_correction_data_get(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data);

  #if (CTSU_CFG_CALIB_RTRIM_SUPPORT == 1)
static void ctsu_correction_calib_rtrim(ctsu_instance_ctrl_t * const p_instance_ctrl, uint16_t * p_data);

  #endif
 #endif

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/** Version data structure used by error logger macro. */
static const fsp_version_t g_ctsu_version =
{
    .api_version_minor  = CTSU_API_VERSION_MINOR,
    .api_version_major  = CTSU_API_VERSION_MAJOR,
    .code_version_major = CTSU_CODE_VERSION_MAJOR,
    .code_version_minor = CTSU_CODE_VERSION_MINOR
};
static uint16_t      g_ctsu_element_index = 0;
static uint8_t       g_ctsu_tuning_count[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS];
static int32_t       g_ctsu_tuning_diff[CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS];
static ctsu_ctsuwr_t g_ctsu_ctsuwr[(CTSU_CFG_NUM_SELF_ELEMENTS + CTSU_CFG_NUM_MUTUAL_ELEMENTS) * CTSU_CFG_NUM_SUMULTI];
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
static uint16_t        g_ctsu_self_element_index = 0;
static ctsu_self_buf_t g_ctsu_self_raw[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t        g_ctsu_self_data[CTSU_CFG_NUM_SELF_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
static uint16_t          g_ctsu_mutual_element_index = 0;
static ctsu_mutual_buf_t g_ctsu_mutual_raw[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_MUTUAL_BUF_SIZE];
static uint16_t          g_ctsu_mutual_pri_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
static uint16_t          g_ctsu_mutual_snd_data[CTSU_CFG_NUM_MUTUAL_ELEMENTS * CTSU_CFG_NUM_SUMULTI];
#endif
static ctsu_correction_info_t g_ctsu_correction_info;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
const ctsu_api_t g_ctsu_on_ctsu =
{
    .open        = R_CTSU_Open,
    .scanStart   = R_CTSU_ScanStart,
    .dataGet     = R_CTSU_DataGet,
    .close       = R_CTSU_Close,
    .callbackSet = R_CTSU_CallbackSet,
    .versionGet  = R_CTSU_VersionGet,
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
#pragma address dtc_vectortable = 0x0FFD00U
uint8_t __near dtc_vectortable[40U];

#pragma address dtc_controldata_22 = 0x0FFDF0U
st_dtc_data __near dtc_controldata_22;

#pragma address dtc_controldata_23 = 0x0FFDF8U
st_dtc_data __near dtc_controldata_23;
  #elif defined(__ICCRL78__)
#pragma location=0x0FFD00U
uint8_t __near dtc_vectortable[40U];

#pragma location=0x0FFDF0U
st_dtc_data __near dtc_controldata_22;

#pragma location=0x0FFDF8U
st_dtc_data __near dtc_controldata_23;
  #elif defined(__llvm__)
uint8_t dtc_vectortable[40U] __attribute__ ((section (".dtc_vectortable")));
st_dtc_data_t dtc_controldata_22 __attribute__ ((section (".dtc_controldata_22")));
st_dtc_data_t dtc_controldata_23 __attribute__ ((section (".dtc_controldata_23")));
  #endif
  #else
extern st_dtc_data_t __near dtc_controldata_22;
extern st_dtc_data_t __near dtc_controldata_23;
 #endif
#endif

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
    uint16_t i;
    uint32_t pclkb_mhz;
    uint16_t suadj[CTSU_CFG_NUM_SUMULTI];
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

    /* Save configurations. */
    p_instance_ctrl->p_ctsu_cfg = p_cfg;

    /* Initialize driver control structure (address setting) */
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        p_instance_ctrl->p_self_raw   = &g_ctsu_self_raw[g_ctsu_self_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_self_data  = &g_ctsu_self_data[g_ctsu_self_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->num_elements = p_cfg->num_rx;
        g_ctsu_self_element_index     = (uint8_t) (g_ctsu_self_element_index + p_instance_ctrl->num_elements);
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_cfg->md))
    {
        p_instance_ctrl->p_mutual_raw      = &g_ctsu_mutual_raw[g_ctsu_mutual_element_index * CTSU_MUTUAL_BUF_SIZE];
        p_instance_ctrl->p_mutual_pri_data =
            &g_ctsu_mutual_pri_data[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->p_mutual_snd_data =
            &g_ctsu_mutual_snd_data[g_ctsu_mutual_element_index * CTSU_CFG_NUM_SUMULTI];
        p_instance_ctrl->num_elements = (uint8_t) (p_cfg->num_rx * p_cfg->num_tx);
        g_ctsu_mutual_element_index   = (uint8_t) (g_ctsu_mutual_element_index + p_instance_ctrl->num_elements);
    }
#endif
    p_instance_ctrl->p_tuning_count = &g_ctsu_tuning_count[g_ctsu_element_index];
    p_instance_ctrl->p_tuning_diff  = &g_ctsu_tuning_diff[g_ctsu_element_index];
    p_instance_ctrl->p_ctsuwr       = &g_ctsu_ctsuwr[g_ctsu_element_index * CTSU_CFG_NUM_SUMULTI];
    g_ctsu_element_index            = (uint8_t) (g_ctsu_element_index + p_instance_ctrl->num_elements);

    /* Set Value */
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

    if (CTSU_MODE_CURRENT_SCAN == p_cfg->md)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }

    p_instance_ctrl->ctsucr1 = (uint8_t) ((p_cfg->atune12 & 0x01) << 3);
    p_instance_ctrl->ctsucr1 |= (uint8_t) (p_cfg->md << 6);
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
    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        p_instance_ctrl->p_tuning_count[element_id] = 0;
        p_instance_ctrl->p_tuning_diff[element_id]  = 0;
        element_cfgs = (p_cfg->p_elements + element_id);
        if (CTSU_MODE_CURRENT_SCAN == p_cfg->md)
        {
            /* Current scan does not run multiple frequency */
            p_instance_ctrl->p_ctsuwr[element_id].ctsuso =
                (uint32_t) (((uint32_t)element_cfgs->sdpa << (uint32_t)24) | ((uint32_t)element_cfgs->snum << (uint32_t)10) | element_cfgs->so);
        }
        else
        {
            p_instance_ctrl->p_ctsuwr[element_id * CTSU_CFG_NUM_SUMULTI].ctsuso =
                (uint32_t) (((uint32_t)element_cfgs->sdpa << (uint32_t)24) | ((uint32_t)element_cfgs->snum << (uint32_t)10) | element_cfgs->so);

            for (i = 1; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI) + i].ctsuso =
                    p_instance_ctrl->p_ctsuwr[element_id * CTSU_CFG_NUM_SUMULTI].ctsuso;
            }
        }
    }

    /* Enable interrupts for CTSUWR, CTSURD, CTSUFN */
    CTSUWRMK = 0;
    CTSURDMK = 0;
    CTSUFNMK = 0;
    CTSUWRPR0 = (CTSU_CFG_INTCTSUWR_PRIORITY_LEVEL & 0x01);
    CTSURDPR0 = (CTSU_CFG_INTCTSURD_PRIORITY_LEVEL & 0x01);
    CTSUFNPR0 = (CTSU_CFG_INTCTSUFN_PRIORITY_LEVEL & 0x01);
    CTSUWRPR1 = ((CTSU_CFG_INTCTSUWR_PRIORITY_LEVEL & 0x02) >> 1);
    CTSURDPR1 = ((CTSU_CFG_INTCTSURD_PRIORITY_LEVEL & 0x02) >> 1);
    CTSUFNPR1 = ((CTSU_CFG_INTCTSUFN_PRIORITY_LEVEL & 0x02) >> 1);
    gp_ctsu_isr_context = p_instance_ctrl;

    if (p_instance_ctrl->num_elements == g_ctsu_element_index)
    {
        /* TSCAP discharge process */
#if (CTSU_CFG_TSCAP_PORT == 0x0300)
        P3_bit.no0 = 0;
        PM3_bit.no0 = 0;
        ctsu_delay(10);
        PM3_bit.no0 = 1;
        PMCT3_bit.no0 = 1;
#endif

        /* Get CTSU out of stop state (supply power/clock) */
        CTSUEN = 1;

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_open(p_instance_ctrl);
#endif
        CTSUCRAL |= CTSU_CFG_LOW_VOLTAGE_MODE << CTSUCRA_ATUNE0_Pos; // ATUNE0
        CTSUCRAL |= CTSUCRA_PUMPON_Msk; // PUMPON

        pclkb_mhz = R_BSP_GetFclkFreqHz() / CTSU_PCLKB_FREQ_MHZ;

        if (CTSU_PCLKB_FREQ_RANGE1 >= pclkb_mhz)
        {
            CTSUCR3  = (uint8_t) (pclkb_mhz - 1);
        }
        else if ((CTSU_PCLKB_FREQ_RANGE1 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE2 >= pclkb_mhz))
        {
            CTSUCRAH |= 1 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 1) - 1);
        }
        else if ((CTSU_PCLKB_FREQ_RANGE2 < pclkb_mhz) && (CTSU_PCLKB_FREQ_RANGE3 >= pclkb_mhz))
        {
            CTSUCRAH |= 2 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 2) - 1);
        }
        else
        {
            CTSUCRAH |= 3 << CTSUCRA_CLK_Pos;
            CTSUCR3  = (uint8_t) ((pclkb_mhz >> 3) - 1);
        }
    }

    /* Set power on */
    CTSUCRAL |= CTSUCRA_PON_Msk;
    CTSUCRAL |= CTSUCRA_CSW_Msk;
    ctsu_delay(30);

    /* High resolution pulse mode  */
    CTSUCRAH  |= CTSUCRA_SDPSEL_Msk;
    CTSUCRAH  |= CTSUCRA_PCSEL_Msk;
    CTSUSST   = CTSU_SST_RECOMMEND;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCLK_Msk;
    CTSUDBGR0 &= ~CTSUCALIB_SUCLKEN_Msk;

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

    CTSUCRAH  |= 1 << CTSUCRA_LOAD_Pos;
    CTSUCRBH  &= ~CTSUCRB_SSCNT_Msk;
    CTSUCRBH  |= 1 << CTSUCRB_SSCNT_Pos;
    CTSUDBGR1 &= ~CTSUCALIB_SUCARRY_Msk;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCALIB_Msk;
    CTSUDBGR1 |= CTSUCALIB_CCOCLK_Msk;
    CTSUDBGR0 &= ~CTSUCALIB_TSOC_Msk;
    CTSUDBGR0 |= CTSUCALIB_SUCLKEN_Msk;
    
    /* Read SUADJD byte */
    suadj[0] = SUADJD;

    /* Adjust multi freq */
    suadj[1] = (uint16_t) ((suadj[0] * (CTSU_CFG_SUMULTI1 + 1)) / (CTSU_CFG_SUMULTI0 + 1));
    suadj[2] = (uint16_t) ((suadj[0] * (CTSU_CFG_SUMULTI2 + 1)) / (CTSU_CFG_SUMULTI0 + 1));

    /* Adjust SSCNT setting */
    suadj[0] = (uint16_t) (suadj[0] - (CTSU_SUADJ_SSCNT_ADJ));
    suadj[1] = (uint16_t) (suadj[1] - (CTSU_SUADJ_SSCNT_ADJ));
    suadj[2] = (uint16_t) (suadj[2] - (CTSU_SUADJ_SSCNT_ADJ));

    /* Set CTSUSUCLK register */
    CTSUCRAH  &= ~CTSUCRA_SDPSEL_Msk;
    CTSUSUCLK0 = (uint16_t) (CTSU_CFG_SUMULTI0 << 8) | suadj[0];
    CTSUSUCLK1 = (uint16_t) (CTSU_CFG_SUMULTI1 << 8) | suadj[1];
    CTSUSUCLK2 = (uint16_t) (CTSU_CFG_SUMULTI2 << 8) | suadj[2];
    CTSUCRAH  |= CTSUCRA_SDPSEL_Msk;

    p_instance_ctrl->p_correction_info = &g_ctsu_correction_info;
    p_instance_ctrl->rd_index          = 0;
    p_instance_ctrl->wr_index          = 0;
    p_instance_ctrl->state             = CTSU_STATE_IDLE;

    p_instance_ctrl->p_callback        = p_cfg->p_callback;
    p_instance_ctrl->p_context         = p_cfg->p_context;

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
    uint8_t txvsel;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->p_ctsu_cfg->cap)
    {
        /* Can be checked if the previous measurement was a software trigger */
        if (0 == (0x02 & CTSUCR0))
        {
            FSP_ERROR_RETURN(0x01 != (CTSUCR0 & 0x01), FSP_ERR_CTSU_SCANNING);
        }
    }

    FSP_ERROR_RETURN(CTSU_STATE_SCANNED != p_instance_ctrl->state, FSP_ERR_CTSU_NOT_GET_DATA);

    gp_ctsu_isr_context = p_instance_ctrl;

#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
    err = ctsu_transfer_configure(p_instance_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
#endif

    if (CTSU_CAP_SOFTWARE == p_instance_ctrl->p_ctsu_cfg->cap)
    {
        CTSUCR0 &= (uint8_t) ~0x06; // specify Software trigger usage
    }
    else
    {
        CTSUCR0 |= 0x06;            // specify external trigger usage and enable power saving
        CTSUCR0 &= (uint8_t) ~0x01; // To write CTSUCR1
    }

    /* Write CTSU Control Register 1 and save mode */
    if (CTSU_MODE_CORRECTION_SCAN != p_instance_ctrl->p_ctsu_cfg->md)
    {
        txvsel =
            (uint8_t) ((p_instance_ctrl->p_ctsu_cfg->txvsel2 << 6) | (p_instance_ctrl->p_ctsu_cfg->txvsel << 7));
        temp            = (uint8_t) (CTSUCR0 & ~(CTSU_CR0_MODIFY_BIT));
        CTSUCR0 = (uint8_t) (temp | (txvsel & CTSU_CR0_MODIFY_BIT));                   // TXVSEL
        temp            = (uint8_t) (CTSUCR1 & ~(CTSU_CR1_MODIFY_BIT));
        CTSUCR1 = (uint8_t) (temp | (p_instance_ctrl->ctsucr1 & CTSU_CR1_MODIFY_BIT)); // MD1, MD0, ATUNE1
        temp            = (uint8_t) (CTSUCR2 & ~(CTSU_CR2_MODIFY_BIT));
        CTSUCR2 = (uint8_t) (temp | (p_instance_ctrl->ctsucr2 & CTSU_CR2_MODIFY_BIT)); // POSEL, ATUNE2, MD2
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
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
        CTSUCHAC0  = p_instance_ctrl->p_ctsu_cfg->ctsuchac0;
        CTSUCHAC1  = p_instance_ctrl->p_ctsu_cfg->ctsuchac1;
        CTSUCHAC2  = p_instance_ctrl->p_ctsu_cfg->ctsuchac2;
        CTSUCHAC3  = p_instance_ctrl->p_ctsu_cfg->ctsuchac3;
        CTSUCHAC4  = p_instance_ctrl->p_ctsu_cfg->ctsuchac4;
        CTSUCHTRC0 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc0;
        CTSUCHTRC1 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc1;
        CTSUCHTRC2 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc2;
        CTSUCHTRC3 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc3;
        CTSUCHTRC4 = p_instance_ctrl->p_ctsu_cfg->ctsuchtrc4;
    }

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    else
    {
        CTSUSST = CTSU_SST_RECOMMEND;
        ctsu_correction_scan_start();
    }
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
 **********************************************************************************************************************/
fsp_err_t R_CTSU_DataGet (ctsu_ctrl_t * const p_ctrl, uint16_t * p_data)
{
    fsp_err_t              err             = FSP_SUCCESS;
    ctsu_instance_ctrl_t * p_instance_ctrl = (ctsu_instance_ctrl_t *) p_ctrl;
    uint16_t               element_id;

#if (CTSU_CFG_PARAM_CHECKING_ENABLE == 1)
    FSP_ASSERT(p_instance_ctrl);
    FSP_ASSERT(p_data);
    FSP_ERROR_RETURN(CTSU_OPEN == p_instance_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    FSP_ERROR_RETURN(CTSU_STATE_SCANNING != p_instance_ctrl->state, FSP_ERR_CTSU_SCANNING);

 #if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
    {
        ctsu_correction_data_get(p_instance_ctrl, p_data);
        p_instance_ctrl->state = CTSU_STATE_IDLE;

        return err;
    }
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
            if ((CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md) ||
                (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->p_ctsu_cfg->md))
            {
                ctsu_initial_offset_tuning(p_instance_ctrl);
            }
        }

        p_instance_ctrl->state = CTSU_STATE_IDLE;
    }

    FSP_ERROR_RETURN(0 < p_instance_ctrl->average, FSP_ERR_CTSU_INCOMPLETE_TUNING);

#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
    {
        if (true == p_instance_ctrl->p_ctsu_cfg->judge_multifreq_disable)
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI); element_id++)
            {
                *p_data = *(p_instance_ctrl->p_self_data + element_id);
                p_data++;
            }
        }
        else
        {
            for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
            {
                *p_data = *(p_instance_ctrl->p_self_data + (element_id * CTSU_CFG_NUM_SUMULTI));
                p_data++;
            }
        }
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
    {
        if (true == p_instance_ctrl->p_ctsu_cfg->judge_multifreq_disable)
        {
            for (element_id = 0; element_id < (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI); element_id++)
            {
                *p_data = *(p_instance_ctrl->p_mutual_pri_data + element_id);
                p_data++;
                *p_data = *(p_instance_ctrl->p_mutual_snd_data + element_id);
                p_data++;
            }
        }
        else
        {
            for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
            {
                *p_data = *(p_instance_ctrl->p_mutual_pri_data + (element_id * CTSU_CFG_NUM_SUMULTI));
                p_data++;
                *p_data = *(p_instance_ctrl->p_mutual_snd_data + (element_id * CTSU_CFG_NUM_SUMULTI));
                p_data++;
            }
        }
    }
#endif

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

    /* Store callback and context */
    p_ctrl->p_callback = p_callback;
    p_ctrl->p_context         = p_context;

    return FSP_SUCCESS;
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
    if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
    {
        g_ctsu_self_element_index = (uint8_t) (g_ctsu_self_element_index - p_instance_ctrl->num_elements);
    }
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
    {
        g_ctsu_mutual_element_index = (uint8_t) (g_ctsu_mutual_element_index - p_instance_ctrl->num_elements);
    }
#endif
    if (0 == g_ctsu_element_index)
    {
#if (CTSU_CFG_DTC_SUPPORT_ENABLE == 1)
        err = ctsu_transfer_close(p_instance_ctrl);
#endif
    }

    p_instance_ctrl->state = CTSU_STATE_INIT;
    p_instance_ctrl->open  = false;

    return err;
}

/***********************************************************************************************************************
 * DEPRECATED Return CTSU HAL driver version. Implements @ref ctsu_api_t::versionGet.
 *
 * @retval FSP_SUCCESS             Version information successfully read.
 * @retval FSP_ERR_ASSERTION       Null pointer passed as a parameter
 **********************************************************************************************************************/
fsp_err_t R_CTSU_VersionGet (fsp_version_t * const p_version)
{
#if CTSU_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(p_version);
#endif

    p_version->version_id = g_ctsu_version.version_id;

    return FSP_SUCCESS;
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
    DTCBAR = 0xFDU;
    /* Set DTCD22 */
    dtc_vectortable[33] = 0xF0U;
    /* Set DTCD23 */
    dtc_vectortable[34] = 0xF8U;
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
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
            dtc_controldata_22.dtcct = p_instance_ctrl->num_elements;
        }
        else
        {
            dtc_controldata_22.dtcct = (uint16_t) (p_instance_ctrl->num_elements * CTSU_CFG_NUM_SUMULTI);
        }

        dtc_controldata_22.dtsar = (uint16_t)p_instance_ctrl->p_ctsuwr;
    }

    DTCEN4 |= 0x40; //CTSUWR

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
    else
    {
        if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
            dtc_controldata_23.dtcct = p_instance_ctrl->num_elements;
        }
        else if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
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
        if (CTSU_MODE_MUTUAL_FULL_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
        {
            dtc_controldata_23.dtdar = (uint16_t) p_instance_ctrl->p_mutual_raw;
            dtc_controldata_23.dtcct = (uint16_t) (dtc_controldata_23.dtcct * 2); ///< Primary and Secondary
        }
 #endif
    }

    DTCEN4 |= 0x20; //CTSURD

    return FSP_SUCCESS;
}
#endif

/***********************************************************************************************************************
 * ctsu_initial_offset_tuning
 ***********************************************************************************************************************/
void ctsu_initial_offset_tuning (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;
    int32_t  diff          = 0;
    uint32_t complete_flag = 0;
    uint32_t num_complete  = 0;
    uint16_t i;
    uint16_t element_top;
    uint16_t corr_data[CTSU_CFG_NUM_SUMULTI];
    uint16_t target_val;
    int32_t  ctsuso;
    uint32_t snum;

    if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
    {
        target_val = (CTSU_TUNING_VALUE_SELF / 2);
    }
    else
    {
        target_val = (CTSU_TUNING_VALUE_MUTUAL / 2);
    }

    /* element_id through each element for control block */
    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        if (CTSU_TUNING_OT_COUNT != *(p_instance_ctrl->p_tuning_count + element_id))
        {
            element_top = (uint16_t) (element_id * CTSU_CFG_NUM_SUMULTI);
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
                {
                    corr_data[i] = p_instance_ctrl->p_self_data[element_top + i];
                }
                else
                {
                    corr_data[i] = p_instance_ctrl->p_mutual_pri_data[(element_top + i)];
                }

                snum = (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI)].ctsuso >> 10) &
                       CTSU_SNUM_MAX;
                corr_data[i] = (uint16_t) (((uint32_t) corr_data[i] * (CTSU_SNUM_RECOMMEND + 1)) / (snum + 1));

                /* Calculate CTSUSO equivalent difference between current value and target value */
                diff = (int32_t) ((int32_t) corr_data[i] - (int32_t) target_val) /
                       (CTSU_CORRECTION_OFFSET_UNIT >> p_instance_ctrl->range);

                ctsuso  = (int32_t) (p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso & CTSU_TUNING_MAX);
                ctsuso += diff;

                /* If the CTSUSO exceeds the minimum value or the maximum value, tuning complete */
                if (ctsuso < 0)
                {
                    ctsuso = 0;
                    complete_flag++;
                }
                else if (ctsuso > CTSU_TUNING_MAX)
                {
                    ctsuso = CTSU_TUNING_MAX;
                    complete_flag++;
                }
                else
                {
                    /* If the difference is large, tuning value may not be able to match, so create the next opportunity */
                    if (0 == diff)
                    {
                        complete_flag++;
                    }
                    else
                    {
                        (*(p_instance_ctrl->p_tuning_count + element_id))++;
                    }
                }

                /* Set the result of the calculated CTSUSO */
                p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso &= (uint32_t) (~CTSU_TUNING_MAX);
                p_instance_ctrl->p_ctsuwr[element_top + i].ctsuso |= (uint32_t) ctsuso;
            }
        }
        else
        {
            complete_flag = CTSU_CFG_NUM_SUMULTI;
        }

        if (CTSU_CFG_NUM_SUMULTI == complete_flag)
        {
            complete_flag = 0;
            num_complete++;
            *(p_instance_ctrl->p_tuning_count + element_id) = CTSU_TUNING_OT_COUNT;
        }
    }

    if (num_complete == p_instance_ctrl->num_elements)
    {
        p_instance_ctrl->tuning = CTSU_TUNING_COMPLETE;
    }
}

/***********************************************************************************************************************
 * ctsu_moving_average
 ***********************************************************************************************************************/
void ctsu_moving_average (uint16_t * p_average, uint16_t new_data, uint16_t average_num)
{
    uint32_t work;

    work       = (uint32_t) ((uint32_t) *p_average * (uint32_t) (average_num - 1)); /* Average * (num - 1) */
    work      += new_data;                                                          /* Add Now data        */
    *p_average = (uint16_t) (work / average_num);                                   /* Average calculation */
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
        CTSUSO0 = (uint16_t)g_ctsu_correction_info.ctsuwr.ctsuso;
        CTSUSO1 = (uint16_t)(g_ctsu_correction_info.ctsuwr.ctsuso >> 16);
    }
    else
    {
        CTSUSO0 = (uint16_t)p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso;
        CTSUSO1 = (uint16_t)(p_instance_ctrl->p_ctsuwr[p_instance_ctrl->wr_index].ctsuso >> 16);
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
    if (CTSU_CORRECTION_RUN == g_ctsu_correction_info.status)
    {
        g_ctsu_correction_info.scanbuf = CTSUSC;
    }
    else
    {
  #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
        if (CTSU_MODE_SELF_MULTI_SCAN == (CTSU_MODE_MUTUAL_FULL_SCAN & p_instance_ctrl->p_ctsu_cfg->md))
        {
            p_instance_ctrl->p_self_raw[p_instance_ctrl->rd_index] = CTSUSC;
            p_instance_ctrl->rd_index++;
        }
  #endif
  #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
        if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
            p_instance_ctrl->p_mutual_raw[p_instance_ctrl->rd_index] = CTSUSC;
            p_instance_ctrl->rd_index++;
        }
  #endif
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
    ctsu_callback_args_t   args;

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
#if (CTSU_CFG_TEMP_CORRECTION_SUPPORT == 1)
    if (CTSU_CORRECTION_RUN != g_ctsu_correction_info.status)
    {
        if (CTSU_MODE_CORRECTION_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
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

    p_instance_ctrl->state = CTSU_STATE_SCANNED;
    args.p_context         = p_instance_ctrl->p_context;

    /* If a callback was provided, call it with the argument */
    if (NULL != p_instance_ctrl->p_callback)
    {
        p_instance_ctrl->p_callback(&args);
    }

    /* reset indexes */
    p_instance_ctrl->wr_index = 0;
    p_instance_ctrl->rd_index = 0;
}

/***********************************************************************************************************************
 * ctsu_correction_process
 ***********************************************************************************************************************/
void ctsu_correction_process (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint32_t i;
    uint32_t j;
    uint16_t error_registance[CTSU_RANGE_NUM];
    uint8_t  trimb_byte;
    uint16_t base_value;
    uint16_t base_conv_dac;
    uint32_t ref_include_error;
    int32_t  x0;
    int32_t  x1;
    int32_t  y0;

    g_ctsu_correction_info.status = CTSU_CORRECTION_RUN;

    CTSUCRAL |= CTSUCRA_MD0_Msk;
    CTSUCRAL &= ~CTSUCRA_MD1_Msk;
    CTSUCRAH &= ~CTSUCRA_MD2_Msk;
    CTSUMCHH = 0x0001;
    CTSUCRAH &= ~CTSUCRA_LOAD_Msk;
    CTSUCRAH |= 1 << CTSUCRA_LOAD_Pos;

    /* Setting time of measurement */
    g_ctsu_correction_info.ctsuwr.ctsuso = (CTSU_SNUM_RECOMMEND << 10);

    /* Step1 : Get resistance value from TRIMB register. */
    for (i = 0; i < CTSU_RANGE_NUM; i++)
    {
    	if (0 == i)
    	{
            trimb_byte = TRESULT0;
    	}
    	else if (1 == i)
    	{
    		trimb_byte = TRESULT1;
    	}
    	else if (2 == i)
    	{
    		trimb_byte = TRESULT2;
    	}
    	else if (3 == i)
    	{
    		trimb_byte = TRESULT3;
    	}
    	else
    	{
    	}
        error_registance[i] = (uint16_t) (trimb_byte & (uint8_t) (~CTSU_CORRECTION_TRIMB_SIGN_BIT));
        if (trimb_byte & CTSU_CORRECTION_TRIMB_SIGN_BIT)
        {
            if ((RTRIM > CTSU_CORRECTION_RTRIM_THRESHOLD1) && (trimb_byte >= CTSU_CORRECTION_TRIMB_THRESHOLD1))
            {
                /* Minus2 : bit8(0.50) set 1 */
                error_registance[i] = (error_registance[i] | CTSU_CORRECTION_BIT8);
            }
            else
            {
                /* Plus   : bit9(1.00) set 1 */
                error_registance[i] = (error_registance[i] | CTSU_CORRECTION_BIT9);
            }
        }
        else
        {
            if ((RTRIM < CTSU_CORRECTION_RTRIM_THRESHOLD2) && (trimb_byte <= CTSU_CORRECTION_TRIMB_THRESHOLD2))
            {
                /* Plus2  : bit7(0.25) and bit9(1.00) set 1 */
                error_registance[i] = (error_registance[i] | CTSU_CORRECTION_BIT7 | CTSU_CORRECTION_BIT9);
            }
            else
            {
                /* Minus : bit8(0.50) and bit7(0.25) set 1 */
                error_registance[i] = (error_registance[i] | CTSU_CORRECTION_BIT7 | CTSU_CORRECTION_BIT8);
            }
        }

        /* 6-bit left shift, 512 to 32768 */
        error_registance[i] = (uint16_t) (error_registance[i] << (CTSU_SHIFT_AMOUNT - 9));
    }

    /* Step2-a : Measure the current input to the ICO by passing current through the internal resistance */
    /*           in each range. The theoretical value of the current is 12.5uA. */
    CTSUCHACAL = 1;
    CTSUCHACAH = 0;
    CTSUCHACBL = 0;
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

    /* Step3 : Measure by inputting each constant current from internal DAC to ICO. */
    CTSUCRBH  &= ~CTSUCRB_SSCNT_Msk;
    CTSUDBGR1 &= ~CTSUCALIB_CCOCLK_Msk;
    CTSUDBGR1 |= CTSUCALIB_CCOCALIB_Msk;

    /* 1.25uA * (j + 1), SUCARRY is required for greater than 10uA */
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
        ctsu_correction_measurement(p_instance_ctrl, &g_ctsu_correction_info.dac_value[j]);
    }

    /* Step4 : Calculate the coefficient between step2 and step3. */
    for (i = 0; i < CTSU_RANGE_NUM; i++)
    {
        /* Linear interpolation calculation */
        base_value = g_ctsu_correction_info.base_value[i];
        j          = 1;
        while (1)
        {
            if ((base_value < g_ctsu_correction_info.dac_value[j]) || ((CTSU_CORRECTION_POINT_NUM - 1) == j))
            {
                y0 = (uint16_t) (CTSU_CORRECTION_STD_UNIT * (j + 1));
                x0 = g_ctsu_correction_info.dac_value[j];
                x1 = g_ctsu_correction_info.dac_value[j - 1];
                break;
            }

            j++;
        }

        base_conv_dac = (uint16_t) (y0 - ((CTSU_CORRECTION_STD_UNIT * (x0 - base_value)) / (x0 - x1)));

        /* Error rate calculation */
        ref_include_error = (uint32_t) (CTSU_CORRECTION_STD_VAL * (CTSU_CORRECTION_BIT16 - error_registance[i]));
        g_ctsu_correction_info.error_rate[i] = (uint16_t) (ref_include_error / base_conv_dac);

        for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
        {
            g_ctsu_correction_info.ref_value[i][j] =
                (uint16_t) ((CTSU_CORRECTION_STD_UNIT * (j + 1) * g_ctsu_correction_info.error_rate[i]) >>
                            CTSU_SHIFT_AMOUNT);
        }
    }

    for (i = 0; i < CTSU_RANGE_NUM - 1; i++)
    {
        g_ctsu_correction_info.range_ratio[i] =
            (uint16_t) (((uint32_t) g_ctsu_correction_info.error_rate[i] << CTSU_SHIFT_AMOUNT) /
                        g_ctsu_correction_info.error_rate[CTSU_RANGE_160UA]);
    }
    g_ctsu_correction_info.status = CTSU_CORRECTION_COMPLETE;
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
        CTSUCR0 |= 0x01;
        while (p_instance_ctrl->state != CTSU_STATE_SCANNED)
        {
        }

        sum += g_ctsu_correction_info.scanbuf;
    }

    *data = (uint16_t) (sum / CTSU_CORRECTION_AVERAGE);
}

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
        CTSUCHACAH = 0;
        CTSUCHACBL = 0;
        
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
        CTSUCHACAH = 0;
        CTSUCHACBL = 0;
  #elif (CTSU_CFG_TEMP_CORRECTION_TS >= 16) && (CTSU_CFG_TEMP_CORRECTION_TS < 32)
        CTSUCHACAL = 0;
        CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
        CTSUCHACBL = 0;
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
    uint32_t i;
    uint32_t j;
    uint16_t base_value;
    uint16_t base_conv_dac;
    int32_t  x0;
    int32_t  x1;
    int32_t  y0;

    if (g_ctsu_correction_info.scan_index < CTSU_CORRECTION_POINT_NUM)
    {
        ctsu_moving_average(&g_ctsu_correction_info.dac_value[g_ctsu_correction_info.scan_index],
                            *p_instance_ctrl->p_self_raw,
                            4);
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
            ctsu_moving_average(&g_ctsu_correction_info.ex_base_value, *p_instance_ctrl->p_self_raw, 4);
        }

        g_ctsu_correction_info.scan_index = 0;
        g_ctsu_correction_info.update_counter++;
    }

    /* Step4 : Calculate the coefficient between step2 and step3. */
    if (g_ctsu_correction_info.update_counter > CTSU_CFG_TEMP_CORRECTION_TIME)
    {
        /* Linear interpolation calculation */
        base_value = g_ctsu_correction_info.ex_base_value;
        j          = 1;
        while (1)
        {
            if ((base_value < g_ctsu_correction_info.dac_value[j]) || ((CTSU_CORRECTION_POINT_NUM - 1) == j))
            {
                y0 = (uint16_t) (CTSU_CORRECTION_STD_UNIT * (j + 1));
                x0 = g_ctsu_correction_info.dac_value[j];
                x1 = g_ctsu_correction_info.dac_value[j - 1];
                break;
            }

            j++;
        }

        base_conv_dac = (uint16_t) (y0 - ((CTSU_CORRECTION_STD_UNIT * (x0 - base_value)) / (x0 - x1)));

        /* Error rate calculation */
        g_ctsu_correction_info.error_rate[CTSU_RANGE_160UA] =
            (uint16_t) (((uint32_t) CTSU_CORRECTION_STD_EXREG << (uint32_t) CTSU_SHIFT_AMOUNT) / base_conv_dac);

        for (j = 0; j < CTSU_CORRECTION_POINT_NUM; j++)
        {
            g_ctsu_correction_info.ref_value[CTSU_RANGE_160UA][j] =
                (uint16_t) ((CTSU_CORRECTION_STD_UNIT * (j + 1) *
                             (uint32_t) g_ctsu_correction_info.error_rate[CTSU_RANGE_160UA]) >> CTSU_SHIFT_AMOUNT);
            for (i = 0; i < CTSU_RANGE_NUM - 1; i++)
            {
                g_ctsu_correction_info.ref_value[i][j] =
                    (uint16_t) (((uint32_t) g_ctsu_correction_info.ref_value[CTSU_RANGE_160UA][j] *
                                 (uint32_t) g_ctsu_correction_info.range_ratio[i]) >> CTSU_SHIFT_AMOUNT);
            }
        }

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
    uint16_t     i;
    uint16_t     adctdr_ave;
    uint32_t     adctdr_sum;
    int16_t      diff;
    int16_t      dir  = 0;
    uint16_t     comp = 0;

    ADCEN = 1U;  /* supply AD clock */
    ADM0 = 0x00; /* disable AD conversion and clear ADM0 register */
    ADMK = 1U;   /* disable INTAD interrupt */
    ADIF = 0U;   /* clear INTAD interrupt flag */
    ///* The reset status of ADPC is analog input, so it's unnecessary to set. */
    ADM0 = 0x00; /* AD_CONVERSION_CLOCK_64 | AD_TIME_MODE_NORMAL_1 | AD_OPERMODE_SELECT */
    ADM1 = 0x20; /* AD_TRIGGER_SOFTWARE | AD_CONVMODE_ONESELECT */
    ADM2 = 0x02; /* AD_POSITIVE_VDD | AD_NEGATIVE_VSS | AD_AREA_MODE_1 | AD_RESOLUTION_12BIT */
    ADUL = 0xFF; /* AD_ADUL_VALUE */
    ADLL = 0x00; /* AD_ADLL_VALUE */
    ADS = 0x1E;  /* CH30 */

    /* Self single scan mode */
    CTSUCRAH |= 1 << CTSUCRA_LOAD_Pos;
    CTSUCRAL &= ~CTSUCRA_MD0_Msk;
    CTSUCRAL &= ~CTSUCRA_MD1_Msk;
    CTSUCRAH &= ~CTSUCRA_MD2_Msk;

    /* Set Channel */
#if (CTSU_CFG_TEMP_CORRECTION_TS < 16)
      CTSUCHACAL = (uint16_t) (1 << CTSU_CFG_TEMP_CORRECTION_TS);
      CTSUCHACAH = 0;
      CTSUCHACBL = 0;
#elif (CTSU_CFG_TEMP_CORRECTION_TS >= 16) && (CTSU_CFG_TEMP_CORRECTION_TS < 32)
      CTSUCHACAL = 0;
      CTSUCHACAH = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 16));
      CTSUCHACBL = 0;
#else
      CTSUCHACAL = 0;
      CTSUCHACAH = 0;
      CTSUCHACBL = (uint16_t) (1 << (CTSU_CFG_TEMP_CORRECTION_TS - 32));
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
            ADCS = 1;

            /* Wait for conversion to complete. */
            while (1 == ADCS)
            {
            }

            adctdr_sum += ADCR;
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

/***********************************************************************************************************************
 * ctsu_correction_calc
 ***********************************************************************************************************************/
void ctsu_correction_calc (uint16_t * correction_data, uint16_t raw_data, ctsu_correction_calc_t * p_calc)
{
    uint32_t answer;
    int32_t  cmp_data;
    int32_t  y0 = 0;
    int32_t  y1 = 0;
    int32_t  x0 = 0;
    int32_t  x1 = 0;
    uint16_t i;

    if (CTSU_CORRECTION_COMPLETE == g_ctsu_correction_info.status)
    {
        /* Since the correction coefficient table is created with the recommended measurement time, */
        /* If the measurement time is different, adjust the value level. */
        if (CTSU_SNUM_RECOMMEND == p_calc->snum)
        {
            cmp_data = raw_data;
        }
        else
        {
        	cmp_data = (int32_t) (((uint32_t)raw_data * (CTSU_SNUM_RECOMMEND + 1)) / (p_calc->snum + 1));
        }

        /* y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);    [y=coefficient, x=value] */
        i = 0;
        while (1)
        {
            if ((cmp_data < g_ctsu_correction_info.dac_value[i]) || ((CTSU_CORRECTION_POINT_NUM - 1) == i))
            {
                y0 = g_ctsu_correction_info.ref_value[p_calc->range][i];
                x0 = g_ctsu_correction_info.dac_value[i];
                if (0 == i)
                {
                    x1 = 0;
                    y1 = 0;
                }
                else
                {
                    x1 = g_ctsu_correction_info.dac_value[i - 1];
                    y1 = g_ctsu_correction_info.ref_value[p_calc->range][i - 1];
                }

                break;
            }

            i++;
        }

        answer = (uint32_t) (y0 - (((y0 - y1) * (x0 - cmp_data)) / (x0 - x1)));

        if (CTSU_SNUM_RECOMMEND != p_calc->snum)
        {
            answer = (uint32_t) ((answer * (uint32_t) (p_calc->snum + 1)) / (CTSU_SNUM_RECOMMEND + 1));
        }

        /* Value Overflow Check */
        if (CTSU_COUNT_MAX < answer)
        {
            *correction_data = CTSU_COUNT_MAX;
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

/***********************************************************************************************************************
 * ctsu_correction_exec
 ***********************************************************************************************************************/
void ctsu_correction_exec (ctsu_instance_ctrl_t * const p_instance_ctrl)
{
    uint16_t element_id;

    ctsu_correction_calc_t calc;
    uint16_t                i;
    uint32_t                ctsuso;
    ctsu_correction_multi_t multi;
#if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
    uint16_t * p_self_data;
    uint16_t   average_self[CTSU_CFG_NUM_SUMULTI];
#endif
#if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
    uint16_t * p_pri_data;
    uint16_t * p_snd_data;
    uint16_t   average_pri[CTSU_CFG_NUM_SUMULTI];
    uint16_t   average_snd[CTSU_CFG_NUM_SUMULTI];
#endif
    calc.range          = p_instance_ctrl->range;
    calc.md             = p_instance_ctrl->p_ctsu_cfg->md;
    multi.judge_disable = p_instance_ctrl->p_ctsu_cfg->judge_multifreq_disable;

    for (element_id = 0; element_id < p_instance_ctrl->num_elements; element_id++)
    {
        calc.snum = (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI)].ctsuso >> 10) & CTSU_SNUM_MAX;
        for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
        {
            ctsuso =
                (p_instance_ctrl->p_ctsuwr[(element_id * CTSU_CFG_NUM_SUMULTI) + i].ctsuso & CTSU_TUNING_MAX);
            multi.offset[i] = (ctsuso * (uint32_t) (CTSU_CORRECTION_OFFSET_UNIT >> calc.range));
        }

        if (CTSU_MODE_SELF_MULTI_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
 #if (CTSU_CFG_NUM_SELF_ELEMENTS != 0)
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                ctsu_correction_calc(&multi.pri[i],
                                              p_instance_ctrl->p_self_raw[(element_id * CTSU_CFG_NUM_SUMULTI) + i],
                                     &calc);
                multi.snd[i] = 0;
            }

            p_self_data = (p_instance_ctrl->p_self_data + (element_id * CTSU_CFG_NUM_SUMULTI));

            if (0 == p_instance_ctrl->average)
            {
                /* Store corrected data in p_pri_data[i] for initial offset tuning */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    p_self_data[i] = multi.pri[i];
                }
            }
            else
            {
                /* Store last moving averaged data */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    average_self[i] = p_self_data[i];
                }

                /* Matching values */
                ctsu_correction_multi(&multi, p_self_data, NULL);

                /* Update moving averaged data */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    ctsu_moving_average(&average_self[i], p_self_data[i], p_instance_ctrl->average);
                    p_self_data[i] = average_self[i];
                }
            }
        }
        else if (CTSU_MODE_CURRENT_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
            p_self_data  = (p_instance_ctrl->p_self_data + (element_id * CTSU_CFG_NUM_SUMULTI));

            /* Store last moving averaged data */
            average_self[0] = p_self_data[0];

            /* Correction */
            ctsu_correction_calc(p_self_data, p_instance_ctrl->p_self_raw[element_id], &calc);

            /* Update moving averaged data */
            if (1 < p_instance_ctrl->average)
            {
                ctsu_moving_average(&average_self[0], p_self_data[0], p_instance_ctrl->average);
                p_self_data[0] = average_self[0];
            }
 #endif
        }
        else if (CTSU_MODE_MUTUAL_FULL_SCAN == p_instance_ctrl->p_ctsu_cfg->md)
        {
 #if (CTSU_CFG_NUM_MUTUAL_ELEMENTS != 0)
            for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
            {
                ctsu_correction_calc(&multi.pri[i],
                                     p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2)],
                                     &calc);
                ctsu_correction_calc(&multi.snd[i],
                                     p_instance_ctrl->p_mutual_raw[(element_id * CTSU_MUTUAL_BUF_SIZE) + (i * 2) + 1],
                                     &calc);
            }

            p_pri_data = (p_instance_ctrl->p_mutual_pri_data + (element_id * CTSU_CFG_NUM_SUMULTI));
            p_snd_data = (p_instance_ctrl->p_mutual_snd_data + (element_id * CTSU_CFG_NUM_SUMULTI));

            if (0 == p_instance_ctrl->average)
            {
                /* Store corrected data in p_pri_data[i] for initial offset tuning */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    p_pri_data[i] = multi.pri[i];
                }
            }
            else
            {
                /* Store last moving averaged data */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    average_pri[i] = p_pri_data[i];
                    average_snd[i] = p_snd_data[i];
                }

                /* Matching values */
                ctsu_correction_multi(&multi, p_pri_data, p_snd_data);

                /* Update moving averaged data */
                for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
                {
                    ctsu_moving_average(&average_pri[i], p_pri_data[i], p_instance_ctrl->average);
                    p_pri_data[i] = average_pri[i];
                    ctsu_moving_average(&average_snd[i], p_snd_data[i], p_instance_ctrl->average);
                    p_snd_data[i] = average_snd[i];
                }
            }
 #endif
        }
        else
        {
        }
    }
}


/***********************************************************************************************************************
 * ctsu_correction_multi
 ***********************************************************************************************************************/
void ctsu_correction_multi (ctsu_correction_multi_t * p_multi, uint16_t * p_pri, uint16_t * p_snd)
{
    uint32_t i;
    int32_t  pri_calc[CTSU_CFG_NUM_SUMULTI];
    int32_t  snd_calc[CTSU_CFG_NUM_SUMULTI];
    int32_t  sumulti[CTSU_CFG_NUM_SUMULTI];
    int32_t  pri_total;
    int32_t  snd_total;
    int32_t  add_pri;
    int32_t  add_snd;
 #if CTSU_CFG_NUM_SUMULTI >= 3
    int32_t diff[CTSU_CFG_NUM_SUMULTI];
 #endif

    sumulti[0] = CTSU_CFG_SUMULTI0 + 1;
 #if CTSU_CFG_NUM_SUMULTI >= 2
    sumulti[1] = CTSU_CFG_SUMULTI1 + 1;
 #endif
 #if CTSU_CFG_NUM_SUMULTI >= 3
    sumulti[2] = CTSU_CFG_SUMULTI2 + 1;
 #endif

    pri_calc[0] = p_multi->pri[0];
    for (i = 1; i < CTSU_CFG_NUM_SUMULTI; i++)
    {
        pri_total   = (int32_t) (p_multi->pri[i] + p_multi->offset[i]);
        pri_calc[i] = (int32_t) (((pri_total * sumulti[0]) / sumulti[i]) - (int32_t) p_multi->offset[0]);
    }

    if (NULL == p_snd)
    {
        for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
        {
            snd_calc[i] = 0;
        }
    }
    else
    {
        snd_calc[0] = p_multi->snd[0];
        for (i = 1; i < CTSU_CFG_NUM_SUMULTI; i++)
        {
            snd_total   = (int32_t) (p_multi->snd[i] + p_multi->offset[i]);
            snd_calc[i] = (int32_t) (((snd_total * sumulti[0]) / sumulti[i]) - (int32_t) p_multi->offset[0]);
        }
    }

    if (true == p_multi->judge_disable)
    {
        for (i = 0; i < CTSU_CFG_NUM_SUMULTI; i++)
        {
            p_pri[i] = (uint16_t) pri_calc[i];
            if (NULL != p_snd)
            {
                p_snd[i] = (uint16_t) snd_calc[i];
            }
        }
    }
    else
    {
 #if CTSU_CFG_NUM_SUMULTI == 1
        add_pri = pri_calc[0];
        add_snd = snd_calc[0];
 #endif
 #if CTSU_CFG_NUM_SUMULTI == 2
        add_pri = pri_calc[0] + pri_calc[1];
        add_snd = snd_calc[0] + snd_calc[1];
 #endif
 #if CTSU_CFG_NUM_SUMULTI >= 3
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
        }
        else
        {
            if (diff[1] < diff[2])
            {
                add_pri = pri_calc[0] + pri_calc[2];
                add_snd = snd_calc[0] + snd_calc[2];
            }
            else
            {
                add_pri = pri_calc[1] + pri_calc[2];
                add_snd = snd_calc[1] + snd_calc[2];
            }
        }
 #endif

        if (CTSU_COUNT_MAX < add_pri)
        {
            *p_pri = CTSU_COUNT_MAX;
        }
        else
        {
            *p_pri = (uint16_t) add_pri;
        }

        if (NULL != p_snd)
        {
            if (CTSU_COUNT_MAX < add_snd)
            {
                *p_snd = CTSU_COUNT_MAX;
            }
            else
            {
                *p_snd = (uint16_t) add_snd;
            }
        }
    }
}

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
