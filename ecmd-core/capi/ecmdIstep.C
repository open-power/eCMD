#include <ecmdIstep.H>

//Updated this table as per P10_IPL_Flow_v0.89.17. 
//HB isteps need to revisit
const ecmdIPLTable::ecmdIStep_t ecmdIPLTable::cv_ecmdIStepTable[] =
{
    /****************************************************************************/
    /* !!! --- THIS LIST MUST BE IN ORDER THAT THEY'RE CALLED IN AN IPL --- !!! */
    /****************************************************************************/

    /****************************************************************************/
    /* NOTE: The istep is executed by the self boot engine/the host code/       */
    /*       the attached service processor depending upon the destination type.*/
    /*       Options:                                                           */
    /*       ECMD_ISTEP_HOST - The istep is performed by the host               */
    /*       ECMD_ISTEP_SBE  - The istep is performed by the self boot engine   */
    /*       ECMD_ISTEP_SP   - The istep is performed by the service processor  */
    /*       ECMD_ISTEP_NOOP - The istep is No OP                               */
    /*                                                                          */
    /****************************************************************************/

    /****************************************************************************/
    /* Warning : The following constants are defined based on the values of this*/
    /*           table. Any changes to this table requires examination of the   */
    /*           values assigned to these constants.                            */
    /*                                                                          */
    /*       ECMD_FIRST_ISTEP_NUM   = 0                                         */
    /*       ECMD_LAST_ISTEP_NUM    = 21                                        */
    /*       ECMD_INVALID_ISTEP_NUM = 0xFFFF                                    */
    /*       ECMD_INVALID_POSITION  = 0xFFFF                                     */
    /****************************************************************************/

//major | minor |                          istep name   |                     destination   | 
//number| number|                                       |                                   |
  { 0,   2,                                   "poweron",                   ECMD_ISTEP_NOOP},
  { 0,   3,                                  "startipl",                   ECMD_ISTEP_NOOP},
  { 0,   4,                              "disableattns",                   ECMD_ISTEP_NOOP},
  { 0,   5,                             "updatehwmodel",                   ECMD_ISTEP_NOOP},
  { 0,   6,                           "alignment_check",                   ECMD_ISTEP_NOOP},
  { 0,   7,                             "set_ref_clock",                   ECMD_ISTEP_NOOP},
  { 0,   8,                           "proc_clock_test",                   ECMD_ISTEP_NOOP},
  { 0,   9,                             "proc_prep_ipl",                   ECMD_ISTEP_NOOP},
  { 0,  10,                               "edramrepair",                   ECMD_ISTEP_NOOP},
  { 0,  11,                          "asset_protection",                   ECMD_ISTEP_NOOP},
  { 0,  12,                   "proc_select_boot_master",                   ECMD_ISTEP_NOOP},
  { 0,  13,                          "hb_config_update",                   ECMD_ISTEP_NOOP},
  { 0,  14,                         "sbe_config_update",                   ECMD_ISTEP_NOOP},
  { 0,  15,                                 "sbe_start",                   ECMD_ISTEP_SP},
  { 1,   1,                   "proc_sbe_enable_seeprom",                   ECMD_ISTEP_NOOP},
  { 2,   1,                         "proc_sbe_ld_image",                   ECMD_ISTEP_NOOP},
  { 2,   2,                       "proc_sbe_attr_setup",                   ECMD_ISTEP_SBE},
  { 2,   3,                 "proc_sbe_tp_chiplet_reset",                   ECMD_ISTEP_SBE},
  { 2,   4,               "proc_sbe_tp_gptr_time_initf",                   ECMD_ISTEP_SBE},
  { 2,   5,                "proc_sbe_dft_probe_setup_1",                   ECMD_ISTEP_SBE},
  { 2,   6,                       "proc_sbe_npll_initf",                   ECMD_ISTEP_SBE},
  { 2,   7,                        "proc_sbe_rcs_setup",                   ECMD_ISTEP_SBE},
  { 2,   8,                  "proc_sbe_tp_switch_gears",                   ECMD_ISTEP_SBE},
  { 2,   9,                       "proc_sbe_npll_setup",                   ECMD_ISTEP_SBE},
  { 2,  10,                     "proc_sbe_tp_repr_intf",                   ECMD_ISTEP_SBE},
  { 2,  11,                   "proc_sbe_setup_tp_abist",                   ECMD_ISTEP_SBE},
  { 2,  12,                     "proc_sbe_tp_arrayinit",                   ECMD_ISTEP_SBE},
  { 2,  13,                          "proc_sbe_tp_intf",                   ECMD_ISTEP_SBE},
  { 2,  14,                 "proc_sbe_dft_probesetup_2",                   ECMD_ISTEP_SBE},
  { 2,  15,                  "proc_sbe_tp_chiplet_init",                   ECMD_ISTEP_SBE},
  { 3,   1,                    "proc_sbe_chiplet_setup",                   ECMD_ISTEP_SBE},
  { 3,   2,               "proc_sbe_chiplet_clk_config",                   ECMD_ISTEP_SBE},
  { 3,   3,                    "proc_sbe_chiplet_reset",                   ECMD_ISTEP_SBE},
  { 3,   4,                  "proc_sbe_gptr_time_initf",                   ECMD_ISTEP_SBE},
  { 3,   5,                "proc_sbe_chiplet_pll_initf",                   ECMD_ISTEP_SBE},
  { 3,   6,                "proc_sbe_chiplet_pll_setup",                   ECMD_ISTEP_SBE},
  { 3,   7,                        "proc_sbe_repr_intf",                   ECMD_ISTEP_SBE},
  { 3,   8,                      "proc_sbe_abist_setup",                   ECMD_ISTEP_SBE},
  { 3,   9,                        "proc_sbe_arrayinit",                   ECMD_ISTEP_SBE},
  { 3,  10,                            "proc_sbe_lbist",                   ECMD_ISTEP_SBE},
  { 3,  11,                            "proc_sbe_initf",                   ECMD_ISTEP_SBE},
  { 3,  12,                      "proc_sbe_startclocks",                   ECMD_ISTEP_SBE},
  { 3,  13,                     "proc_sbe_chiplet_init",                   ECMD_ISTEP_SBE},
  { 3,  14,                 "proc_sbe_chiplet_fir_init",                   ECMD_ISTEP_SBE},
  { 3,  15,                         "proc_sbe_dts_init",                   ECMD_ISTEP_SBE},
  { 3,  16,                "proc_sbe_skew_adjust_setup",                   ECMD_ISTEP_SBE},
  { 3,  17,                 "proc_sbe_nest_enable_ridi",                   ECMD_ISTEP_SBE},
  { 3,  18,                         "proc_sbe_scominit",                   ECMD_ISTEP_SBE},
  { 3,  19,                              "proc_sbe_lpc",                   ECMD_ISTEP_SBE},
  { 3,  20,                       "proc_sbe_fabricinit",                   ECMD_ISTEP_SBE},
  { 3,  21,                     "proc_sbe_check_master",                   ECMD_ISTEP_SBE},
  { 3,  22,                        "proc_sbe_mcs_setup",                   ECMD_ISTEP_SBE},
  { 3,  23,                        "proc_sbe_select_ex",                   ECMD_ISTEP_SBE},
  { 4,   1,                    "proc_hcd_cache_poweron",                   ECMD_ISTEP_SBE},
  { 4,   2,                      "proc_hcd_cache_reset",                   ECMD_ISTEP_SBE},
  { 4,   3,            "proc_hcd_cache_gptr_time_initf",                   ECMD_ISTEP_SBE},
  { 4,   4,               "proc_hcd_cache_repair_initf",                   ECMD_ISTEP_SBE},
  { 4,   5,                  "proc_hcd_cache_arrayinit",                   ECMD_ISTEP_SBE},
  { 4,   6,                      "proc_hcd_cache_initf",                   ECMD_ISTEP_SBE},
  { 4,   7,                "proc_hcd_cache_startclocks",                   ECMD_ISTEP_SBE},
  { 4,   8,                   "proc_hcd_cache_scominit",                   ECMD_ISTEP_SBE},
  { 4,   9,             "proc_hcd_cache_scom_customize",                   ECMD_ISTEP_SBE},
  { 4,  10,           "proc_hcd_cache_ras_runtime_scom",                   ECMD_ISTEP_SBE},
  { 4,  11,                     "proc_hcd_core_poweron",                   ECMD_ISTEP_SBE},
  { 4,  12,               "proc_hcd_core_chiplet_reset",                   ECMD_ISTEP_SBE},
  { 4,  13,             "proc_hcd_core_gptr_time_initf",                   ECMD_ISTEP_SBE},
  { 4,  14,                "proc_hcd_core_repair_initf",                   ECMD_ISTEP_SBE},
  { 4,  15,                   "proc_hcd_core_arrayinit",                   ECMD_ISTEP_SBE},
  { 4,  16,                       "proc_hcd_core_initf",                   ECMD_ISTEP_SBE},
  { 4,  17,                 "proc_hcd_core_startclocks",                   ECMD_ISTEP_SBE},
  { 4,  18,                    "proc_hcd_core_scominit",                   ECMD_ISTEP_SBE},
  { 4,  19,              "proc_hcd_core_scom_customize",                   ECMD_ISTEP_SBE},
  { 4,  20,            "proc_hcd_core_ras_runtime_scom",                   ECMD_ISTEP_SBE},
  { 5,   1,                  "proc_sbe_load_bootloader",                   ECMD_ISTEP_SBE},
  { 5,   2,                   "proc_sbe_core_spr_setup",                   ECMD_ISTEP_SBE},
  { 5,   3,                   "proc_sbe_instruct_start",                   ECMD_ISTEP_SBE},
  { 6,   1,                           "host_bootloader",                   ECMD_ISTEP_HOST},
  { 6,   2,                                "host_setup",                   ECMD_ISTEP_HOST},
  { 6,   3,                         "host_istep_enable",                   ECMD_ISTEP_HOST},
  { 6,   4,                        "host_init_bmc_pcie",                   ECMD_ISTEP_HOST},
  { 6,   5,                             "host_init_fsi",                   ECMD_ISTEP_HOST},
  { 6,   6,                        "host_set_ipl_parms",                   ECMD_ISTEP_HOST},
  { 6,   7,                     "host_discover_targets",                   ECMD_ISTEP_HOST},
  { 6,   8,                    "host_update_master_tpm",                   ECMD_ISTEP_HOST},
  { 6,   9,                                 "host_gard",                   ECMD_ISTEP_HOST},
  { 6,  10,                 "host_revert_sbe_mcs_setup",                   ECMD_ISTEP_HOST},
  { 6,  11,              "host_start_occ_xstop_handler",                   ECMD_ISTEP_HOST},
  { 6,  12,                       "host_voltage_config",                   ECMD_ISTEP_HOST},
  { 7,   1,                     "host_mss_attr_cleanup",                   ECMD_ISTEP_HOST},
  { 7,   2,                                  "mss_volt",                   ECMD_ISTEP_HOST},
  { 7,   3,                                  "mss_freq",                   ECMD_ISTEP_HOST},
  { 7,   4,                            "mss_eff_config",                   ECMD_ISTEP_HOST},
  { 7,   5,                           "mss_attr_update",                   ECMD_ISTEP_HOST},
  { 8,   1,                     "host_slave_sbe_config",                   ECMD_ISTEP_HOST},
  { 8,   2,                            "host_setup_sbe",                   ECMD_ISTEP_HOST},
  { 8,   3,                            "host_cbs_start",                   ECMD_ISTEP_HOST},
  { 8,   4,     "proc_check_slave_sbe_seeprom_complete",                   ECMD_ISTEP_HOST},
  { 8,   5,                      "host_attnlisten_proc",                   ECMD_ISTEP_HOST},
  { 8,   6,                    "host_p9_fbc_eff_config",                   ECMD_ISTEP_HOST},
  { 8,   7,                  "host_p9_eff_config_links",                   ECMD_ISTEP_HOST},
  { 8,   8,                          "proc_attr_update",                   ECMD_ISTEP_HOST},
  { 8,   9,              "proc_chiplet_fabric_scominit",                   ECMD_ISTEP_HOST},
  { 8,  10,                        "proc_xbus_scominit",                   ECMD_ISTEP_HOST},
  { 8,  11,                     "proc_xbus_enable_ridi",                   ECMD_ISTEP_HOST},
  { 8,  12,                         "host_set_voltages",                   ECMD_ISTEP_HOST},
  { 9,   1,                            "fabric_erepair",                   ECMD_ISTEP_HOST},
  { 9,   2,                           "fabric_io_dccal",                   ECMD_ISTEP_HOST},
  { 9,   3,                       "fabric_pre_trainadv",                   ECMD_ISTEP_HOST},
  { 9,   4,                    "fabric_io_run_training",                   ECMD_ISTEP_HOST},
  { 9,   5,                      "fabric_post_trainadv",                   ECMD_ISTEP_HOST},
  { 9,   6,                       "proc_smp_link_layer",                   ECMD_ISTEP_HOST},
  { 9,   7,                          "proc_fab_iovalid",                   ECMD_ISTEP_HOST},
  { 9,   8,             "host_fbc_eff_config_aggregate",                   ECMD_ISTEP_HOST},
  {10,   1,                            "proc_build_smp",                   ECMD_ISTEP_HOST},
  {10,   2,                     "host_slave_sbe_update",                   ECMD_ISTEP_HOST},
  {10,   3,                   "host_set_voltages_dummy",                   ECMD_ISTEP_NOOP},
  {10,   4,                   "proc_cen_ref_clk_enable",                   ECMD_ISTEP_HOST},
  {10,   5,                       "proc_enable_osclite",                   ECMD_ISTEP_HOST},
  {10,   6,                     "proc_chiplet_scominit",                   ECMD_ISTEP_HOST},
  {10,   7,                        "proc_abus_scominit",                   ECMD_ISTEP_HOST},
  {10,   8,                        "proc_obus_scominit",                   ECMD_ISTEP_HOST},
  {10,   9,                         "proc_npu_scominit",                   ECMD_ISTEP_HOST},
  {10,  10,                        "proc_pcie_scominit",                   ECMD_ISTEP_HOST},
  {10,  11,                "proc_scomoverride_chiplets",                   ECMD_ISTEP_HOST},
  {10,  12,                  "proc_chiplet_enable_ridi",                   ECMD_ISTEP_HOST},
  {10,  13,                             "host_rng_bist",                   ECMD_ISTEP_HOST},
  {10,  14,                 "host_update_redundant_tpm",                   ECMD_ISTEP_HOST},
  {11,   1,                       "host_prd_hwreconfig",                   ECMD_ISTEP_HOST},
  {11,   2,                      "cen_tp_chiplet_init1",                   ECMD_ISTEP_HOST},
  {11,   3,                             "cen_pll_initf",                   ECMD_ISTEP_HOST},
  {11,   4,                             "cen_pll_setup",                   ECMD_ISTEP_HOST},
  {11,   5,                      "cen_tp_chiplet_init2",                   ECMD_ISTEP_HOST},
  {11,   6,                          "cen_tp_arrayinit",                   ECMD_ISTEP_HOST},
  {11,   7,                      "cen_tp_chiplet_init3",                   ECMD_ISTEP_HOST},
  {11,   8,                          "cen_chiplet_init",                   ECMD_ISTEP_HOST},
  {11,   9,                             "cen_arrayinit",                   ECMD_ISTEP_HOST},
  {11,   10,                                "cen_initf",                   ECMD_ISTEP_HOST},
  {11,   11,                      "cen_do_manual_inits",                   ECMD_ISTEP_HOST},
  {11,   12,                          "cen_startclocks",                   ECMD_ISTEP_HOST},
  {11,   13,                            "cen_scominits",                   ECMD_ISTEP_HOST},
  {12,   1,                               "mss_getecid",                   ECMD_ISTEP_HOST},
  {12,   2,                           "dmi_attr_update",                   ECMD_ISTEP_HOST},
  {12,   3,                        "proc_dmi_scom_init",                   ECMD_ISTEP_HOST},
  {12,   4,                          "cen_dmi_scominit",                   ECMD_ISTEP_HOST},
  {12,   5,                               "dmi_erepair",                   ECMD_ISTEP_HOST},
  {12,   6,                              "dmi_io_dccal",                   ECMD_ISTEP_HOST},
  {12,   7,                          "dmi_pre_trainadv",                   ECMD_ISTEP_HOST},
  {12,   8,                       "dmi_io_run_training",                   ECMD_ISTEP_HOST},
  {12,   9,                         "dmi_post_trainadv",                   ECMD_ISTEP_HOST},
  {12,  10,                        "proc_cen_framelock",                   ECMD_ISTEP_HOST},
  {12,  11,                         "host_startprd_dmi",                   ECMD_ISTEP_HOST},
  {12,  12,                      "host_attnlisten_memb",                   ECMD_ISTEP_HOST},
  {12,  13,                       "cen_set_inband_addr",                   ECMD_ISTEP_HOST},
  {13,   1,                      "host_disable_memvolt",                   ECMD_ISTEP_HOST},
  {13,   2,                             "mem_pll_reset",                   ECMD_ISTEP_HOST},
  {13,   3,                             "mem_pll_initf",                   ECMD_ISTEP_HOST},
  {13,   4,                             "mem_pll_setup",                   ECMD_ISTEP_HOST},
  {13,   5,                       "proc_mcs_skewadjust",                   ECMD_ISTEP_HOST},
  {13,   6,                           "mem_startclocks",                   ECMD_ISTEP_HOST},
  {13,   7,                       "host_enable_memvolt",                   ECMD_ISTEP_HOST},
  {13,   8,                              "mss_scominit",                   ECMD_ISTEP_HOST},
  {13,   9,                         "mss_ddr_phy_reset",                   ECMD_ISTEP_HOST},
  {13,  10,                              "mss_draminit",                   ECMD_ISTEP_HOST},
  {13,  11,                     "mss_draminit_training",                   ECMD_ISTEP_HOST},
  {13,  12,                     "mss_draminit_trainadv",                   ECMD_ISTEP_HOST},
  {13,  13,                           "mss_draminit_mc",                   ECMD_ISTEP_HOST},
  {14,   1,                               "mss_memdiag",                   ECMD_ISTEP_HOST},
  {14,   2,                          "mss_thermal_init",                   ECMD_ISTEP_HOST},
  {14,   3,                          "proc_pcie_config",                   ECMD_ISTEP_HOST},
  {14,   4,                         "mss_power_cleanup",                   ECMD_ISTEP_HOST},
  {14,   5,                           "proc_setup_bars",                   ECMD_ISTEP_HOST},
  {14,   6,                            "proc_htm_setup",                   ECMD_ISTEP_HOST},
  {14,   7,                 "proc_exit_cache_contained",                   ECMD_ISTEP_HOST},
  {14,   8,                        "host_mpipl_service",                   ECMD_ISTEP_HOST},
  {15,   1,                     "host_build_stop_image",                   ECMD_ISTEP_HOST},
  {15,   2,                    "proc_set_pba_homer_bar",                   ECMD_ISTEP_HOST},
  {15,   3,                 "host_establish_ex_chiplet",                   ECMD_ISTEP_HOST},
  {15,   4,                    "host_start_stop_engine",                   ECMD_ISTEP_HOST},
  {16,   1,                      "host_activate_master",                   ECMD_ISTEP_HOST},
  {16,   2,                 "host_activate_slave_cores",                   ECMD_ISTEP_HOST},
  {16,   3,                           "host_secure_rng",                   ECMD_ISTEP_HOST},
  {16,   4,                                 "mss_scrub",                   ECMD_ISTEP_HOST},
  {16,   5,                          "host_load_io_ppe",                   ECMD_ISTEP_HOST},
  {16,   6,                         "host_ipl_complete",                   ECMD_ISTEP_HOST},
  {17,   1,                           "collect_drawers",                   ECMD_ISTEP_SP},
  {17,   2,                              "micro_update",                   ECMD_ISTEP_SP},
  {17,   3,                              "proc_psiinit",                   ECMD_ISTEP_SP},
  {17,   4,                                  "psi_diag",                   ECMD_ISTEP_SP},
  {18,   1,                 "sys_proc_eff_config_links",                   ECMD_ISTEP_SP},
  {18,   2,                 "sys_proc_chiplet_scominit",                   ECMD_ISTEP_SP},
  {18,   3,                        "sys_fabric_erepair",                   ECMD_ISTEP_SP},
  {18,   4,                   "sys_fabric_pre_trainadv",                   ECMD_ISTEP_SP},
  {18,   5,                       "sys_fabric_io_dccal",                   ECMD_ISTEP_SP},
  {18,   6,                "sys_fabric_io_run_training",                   ECMD_ISTEP_SP},
  {18,   7,                   "sys_proc_smp_link_layer",                   ECMD_ISTEP_SP},
  {18,   8,                  "sys_fabric_post_trainadv",                   ECMD_ISTEP_SP},
  {18,   9,                        "host_coalesce_host",                   ECMD_ISTEP_SP},
  {18,  10,                            "proc_tod_setup",                   ECMD_ISTEP_SP},
  {18,  11,                             "proc_tod_init",                   ECMD_ISTEP_SP},
  {18,  12,                          "cec_ipl_complete",                   ECMD_ISTEP_SP},
  {18,  13,                           "startprd_system",                   ECMD_ISTEP_SP},
  {18,  14,                            "attn_listenall",                   ECMD_ISTEP_SP},
  {19,   1,                                 "prep_host",                   ECMD_ISTEP_SP},
  {20,   1,                         "host_load_payload",                   ECMD_ISTEP_SP},
  {20,   2,                        "host_load_complete",                   ECMD_ISTEP_SP},
  {21,   1,                        "host_runtime_setup",                   ECMD_ISTEP_HOST},
  {21,   2,                          "host_verify_hdat",                   ECMD_ISTEP_HOST},
  {21,   3,                        "host_start_payload",                   ECMD_ISTEP_HOST},
  {21,   4,                   "host_post_start_payload",                   ECMD_ISTEP_SP},
  {21,   5,                                 "switchbcu",                   ECMD_ISTEP_SP},
  {21,   6,                               "completeipl",                   ECMD_ISTEP_SP},
}; // end - array initialization

// Calculate the number of isteps in the IPL Table
const uint16_t ecmdIPLTable::ECMD_NUMBER_OF_ISTEPS = ( sizeof(cv_ecmdIStepTable) /
                                                       sizeof(ecmdIPLTable::ecmdIStep_t));



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ecmdIPLTable::getIStepNumber(const std::string & i_istepName,
                                  uint16_t & o_majorNum,
                                  uint16_t & o_minorNum)
{

  bool l_status = false;
  o_majorNum  = ECMD_INVALID_ISTEP_NUM;
  o_minorNum  = ECMD_INVALID_ISTEP_NUM;

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    // Check whether the istep name in the IPL table matches with the
    // input istep name
    if ( 0 == i_istepName.compare(cv_ecmdIStepTable[l_rowNumber].istepName) )
    {
      o_majorNum = cv_ecmdIStepTable[l_rowNumber].majorNum;
      o_minorNum = cv_ecmdIStepTable[l_rowNumber].minorNum;
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ecmdIPLTable::isValid(const std::string & i_istepName)
{
  bool l_status = false;

  //search if this istep name is available in the IPL table
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    //  Check whether the istep name in the IPL table matches with the
    //  input istep name
    if ( 0 == i_istepName.compare(cv_ecmdIStepTable[l_rowNumber].istepName) )
    {
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ecmdIPLTable::isValid(uint16_t i_majorNum)
{
  if ( i_majorNum > ECMD_LAST_ISTEP_NUM )
  {
    return false;
  }

  uint16_t l_status = false;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    if ( cv_ecmdIStepTable[l_rowNumber].majorNum == i_majorNum )
    {
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool ecmdIPLTable::getIStepNameOf(uint16_t i_position,std::string & o_istepName)
{
  bool l_status = false;
   // If there are 100 rows in the table, then the index of the last row is 99
   // Hence, the array index should always be 1 less than the table size.
   // If the array index is greater than or equal to the table index, then it is
   // out of range and return false.
  if ( i_position < ECMD_NUMBER_OF_ISTEPS )
  {
    o_istepName = cv_ecmdIStepTable[i_position].istepName;
    l_status = true;
  }

  return l_status;
}

/*************************************************************************
 * This function returns the position of the first minor number of the
 * specified major number.
 * Example:
 * Row Number       Major    Minor
 *    0               1        0
 *    1               1        1
 *    2               1        2
 *    3               1        3
 *    4               1        5
 *    5               1        7
 *    6               2        0
 *    7               3        1
 *    8               3        8
 *  Row Number is nothing but an array index.
 *  The position of first minor number of the major number 1 is 0.
 *  The position of first minor number of the major number 2 is 6.
 *  The position of first minor number of the major number 3 is 7.
 ************************************************************************/


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t ecmdIPLTable::getPosFirstMinorNumber(uint16_t i_majorNum)
{
  if ( i_majorNum > ECMD_LAST_ISTEP_NUM )
  {
    return ECMD_INVALID_POSITION;
  }

  uint16_t l_position = ECMD_INVALID_POSITION;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if ( cv_ecmdIStepTable[l_rowNumber].majorNum == i_majorNum )
    {
      l_position = l_rowNumber;
      break;
    }
  }

  return l_position;
}

/*************************************************************************
 * This function returns the position of the last minor number of the
 * specified major number.
 * Example:
 * Row Number       Major    Minor
 *    0               1        0
 *    1               1        1
 *    2               1        2
 *    3               1        3
 *    4               1        5
 *    5               1        7
 *    6               2        0
 *    7               3        1
 *    8               3        8
 *  Row Number is nothing but an array index.
 *  The position of last minor number of the major number 1 is 5.
 *  The position of last minor number of the major number 2 is 6.
 *  The position of last minor number of the major number 3 is 8.
 ************************************************************************/
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t ecmdIPLTable::getPosLastMinorNumber(uint16_t i_majorNum)
{
  if ( i_majorNum > ECMD_LAST_ISTEP_NUM )
  {
    return ECMD_INVALID_POSITION;
  }

  uint16_t l_position = ECMD_INVALID_POSITION;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    if ( cv_ecmdIStepTable[l_rowNumber].majorNum == i_majorNum)
    {
      // If the table has 100 rows, then the last row number is 99 as the
      // row number starts from 0.
      // To know whether we are at the last row, add 1 to the current
      // row number and compare it with the table size. If it matches, then it
      // indicates that we are at the last row. The last row's minor number is
      // always the last minor number of the last major number.
      if ( ( (l_rowNumber+1) == ECMD_NUMBER_OF_ISTEPS)           ||
         ( cv_ecmdIStepTable[l_rowNumber+1].majorNum > i_majorNum ) )
      {
        l_position = l_rowNumber;
        break;
      }
    }
  }

  return l_position;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t ecmdIPLTable::getPosition(const std::string & i_istepName)
{
  uint16_t l_position = ECMD_INVALID_POSITION;

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    // Check whether the istep name in the IPL table matches
    // with the input istep name
    if ( 0 == i_istepName.compare(cv_ecmdIStepTable[l_rowNumber].istepName) )
    {
      l_position = l_rowNumber;
      break;
    }
  }

  return l_position;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t ecmdIPLTable::getIStepNumber(uint16_t i_position)
{
  uint16_t l_istepNum = ecmdIPLTable::ECMD_INVALID_ISTEP_NUM;

  // check whether the array index is out of range
  if ( i_position < ECMD_NUMBER_OF_ISTEPS )
  {
    l_istepNum = cv_ecmdIStepTable[i_position].majorNum;
  }

  return l_istepNum;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t ecmdIPLTable::getIStepMinorNumber(uint16_t i_position)
{
  uint16_t l_istepMinor = ecmdIPLTable::ECMD_INVALID_ISTEP_NUM;

  // check whether the array index is out of range
  if ( i_position < ECMD_NUMBER_OF_ISTEPS )
  {
    l_istepMinor = cv_ecmdIStepTable[i_position].minorNum;
  }

  return l_istepMinor;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ecmdIPLTable::ecmdIStepDestination_t
          ecmdIPLTable::getDestination( uint16_t i_majorNum,
                                        uint16_t i_minorNum)
{

  ecmdIStepDestination_t  l_destination = ECMD_ISTEP_INVALID_DESTINATION;

    // iterate through the array and return the destination corresponding to the
    // matching major and minor number
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if (  cv_ecmdIStepTable[l_rowNumber].majorNum == i_majorNum &&
          cv_ecmdIStepTable[l_rowNumber].minorNum == i_minorNum )
    {
      l_destination = cv_ecmdIStepTable[l_rowNumber].destination;
      break;
    }
  }

  return l_destination;
}

// modified the parameter passing from reference to value
// for the parameters i_majorNum and i_minorNum to make it
// uniform
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool ecmdIPLTable::getIStepName(const uint16_t i_majorNum,
                                const uint16_t i_minorNum,
                                std::string & o_istepName)
{

  bool l_status = false;

  // iterate through the array and return the istep name corresponding to the
  // matching major and minor number

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < ECMD_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if (  (cv_ecmdIStepTable[l_rowNumber].majorNum == i_majorNum) &&
          (cv_ecmdIStepTable[l_rowNumber].minorNum == i_minorNum) )
    {
      o_istepName = cv_ecmdIStepTable[l_rowNumber].istepName;
      l_status = true;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ecmdIPLTable::ecmdIStepDestination_t ecmdIPLTable::getDestination(uint16_t i_position)
{
  ecmdIStepDestination_t  l_destination = ECMD_ISTEP_INVALID_DESTINATION;
   // If there are 100 rows in a table, the last position is 99.
   // ie. the last position in the table is table size - 1.
   // If the given position is greater than or equal to the last position,
   // then it is out of range
  if ( i_position < ECMD_NUMBER_OF_ISTEPS )
  {
    l_destination = cv_ecmdIStepTable[i_position].destination;
  }

  return l_destination;
}


