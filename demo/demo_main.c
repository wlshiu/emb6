/*
 * --- License --------------------------------------------------------------*
 */
/*
 * emb6 is licensed under the 3-clause BSD license. This license gives everyone
 * the right to use and distribute the code, either in binary or source code
 * format, as long as the copyright license is retained in the source code.
 *
 * The emb6 is derived from the Contiki OS platform with the explicit approval
 * from Adam Dunkels. However, emb6 is made independent from the OS through the
 * removal of protothreads. In addition, APIs are made more flexible to gain
 * more adaptivity during run-time.
 *
 * The license text is:

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright (c) 2016,
 * Hochschule Offenburg, University of Applied Sciences
 * Institute of reliable Embedded Systems and Communications Electronics.
 * All rights reserved.
 */

/*
 *  --- Module Description ---------------------------------------------------*
 */
/**
 *  \file       demo_main.h
 *  \author     Institute of reliable Embedded Systems
 *              and Communication Electronics
 *  \date       $Date$
 *  \version    $Version$
 *
 *  \brief      Main file used for all demo applications.
 *
 *              This file provides the entry for all demo applications. depending
 *              on the configuration it initializes the stack and the according
 *              modules before it starts operation.
 */


/*
 *  --- Includes -------------------------------------------------------------*
 */
#include "emb6.h"
#include "board_conf.h"
#include "bsp.h"
#include "etimer.h"

#define LOGGER_ENABLE         LOGGER_MAIN
#include "logger.h"

#if DEMO_USE_UDP
#include "demo_udp.h"
#endif /* #if DEMO_USE_UDP */

#if DEMO_USE_UDP_SOCKET
#include "demo_udp_socket.h"
#endif /* #if DEMO_USE_UDP_SOCKET */

#if DEMO_USE_UDP_SOCKET_SIMPLE
#include "demo_udp_socket_simple.h"
#endif /* #if DEMO_USE_UDP_SOCKET_SIMPLE */

#if DEMO_USE_LWM2M
#if CONF_USE_SERVER
#else
#include "demo_lwm2m_cli.h"
#endif /* #if CONF_USE_SERVER */
#endif /* #if DEMO_USE_LWM2M */

#if DEMO_USE_COAP
#if CONF_USE_SERVER
#include "demo_coap_srv.h"
#else
#include "demo_coap_cli.h"
#endif /* #if CONF_USE_SERVER */
#endif /* #if DEMO_USE_COAP */

#if DEMO_USE_MDNS
#if CONF_USE_SERVER
#include "demo_mdns_srv.h"
#else
#include "demo_mdns_cli.h"
#endif /* #if CONF_USE_SERVER */
#endif /* #if DEMO_USE_MDNS */


#if DEMO_USE_UDPALIVE
#include "demo_udp_alive.h"
#endif /* #if DEMO_USE_UDPALIVE */

#if DEMO_USE_APTB
#include "demo_aptb.h"
#endif /* #if DEMO_USE_APTB */

#if DEMO_USE_TESTSUITE
#include "demo_tsemb6.h"
#endif /* #if DEMO_USE_TESTSUITE */

#if DEMO_USE_EXTIF
#include "slip_radio.h"
#include "slip.h"
#endif /* #if DEMO_USE_EXTIF */

#if DEMO_USE_DTLS
#if CONF_USE_SERVER
#include "demo_dtls_srv.h"
#else
#include "demo_dtls_cli.h"
#endif /* #if CONF_USE_SERVER */
#endif /* #if DEMO_USE_DTLS */

#if UIP_CONF_IPV6_RPL
#include "rpl.h"
#endif /* #if UIP_CONF_IPV6_RPL */

#if USE_FREERTOS
 /* Scheduler includes. */
#include "FreeRTOS.h"
#include "croutine.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif /* #ifndef EMB6_PROC_DELAY */


/*
 * --- Macro Definitions --------------------------------------------------- *
 */

/** default delay for running the emb6 process in microseconds */
#ifndef EMB6_PROC_DELAY
#define EMB6_PROC_DELAY                     500
#endif /* #ifndef EMB6_PROC_DELAY */

#if USE_FREERTOS
/* Task priorities. */
#define mainEMB6_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )
#define mainLED_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#endif /* #if USE_FREERTOS */


/*
 *  --- Type Definitions -----------------------------------------------------*
 */

/**
 * emb6 task parameters
 */
typedef struct
{
  /** MAC address */
  uint16_t ui_macAddr;

}s_emb6_startup_params_t;

#if USE_FREERTOS
/**
 * LED task parameters
 */
typedef struct
{
    int en;
    int delay;

}s_led_task_param_t;
#endif /* #if USE_FREERTOS */


/*
 *  --- Local Variables ---------------------------------------------------- *
 */

/** parameters for emb6 startup */
static s_emb6_startup_params_t emb6_startupParams;

#if USE_FREERTOS
/** parameters for the LED Taks */
static s_led_task_param_t ledTaskParams;
#endif /* #if USE_FREERTOS */


/*
 *  --- Local Function Prototypes ------------------------------------------ *
 */

/* Parse a string including a mac address. For more information please refer to the
 * function definition. */
static uint16_t loc_parseMac( const char* mac, uint16_t defaultMac );

/* Configures the stack. For more information please refer to the
 * function definition. */
static void loc_stackConf(uint16_t mac_addr_word);

/* Configure demo applications. For more information please refer to the
 * function definition. */
static void loc_demoAppsConf(s_ns_t* ps_ns, e_nsErr_t *p_err);

/* Initialize the demo applications. For more information please refer to the
 * function definition. */
static int8_t loc_demoAppsInit(void);

/* Main emb6 task. For more information please refer to the
 * function definition. */
static void emb6_task( void* p_params );

#if USE_FREERTOS
/* FreeRTOS LED task. For more information please refer to the
 * function definition. */
static void vLEDTask( void *pvParameters );

/* Configure the hardware as required by the demo. For more information please
 * refer to the function definition. */
static void prvSetupHardware( void );

/* Put the CPU into the least low power low power mode. For more
 * information please refer to the function definition. */
static void prvLowPowerMode1( void );
#endif /* #if USE_FREERTOS */


/*
 *  --- Local Functions ---------------------------------------------------- *
 */


/**
 * \brief   Parse a string including a mac address.
 *
 *          This function parses a string containing a mac address in the
 *          format "0xAA" or "AA".
 *
 * \param   mac         String to parse.
 * \param   defaultMac  Default mac to use if string can not be parsed.
 *
 * \return  The parsed mac address on success or the defaul address on error.
 */
static uint16_t loc_parseMac( const char* mac, uint16_t defaultMac )
{
    int mac_addr_word;

    if( !mac )
      return defaultMac;

    if( sscanf(mac, "0x%X", &mac_addr_word) )
        return (uint16_t)mac_addr_word;

    if( sscanf(mac, "%X", &mac_addr_word) )
        return (uint16_t)mac_addr_word;

    return defaultMac;
}


/**
 * \brief   Configure the stack from board configuration parameters.
 *
 *          This function uses the parameters from the board configuration
 *          or from the compile command to configure the stack.
 *
 * \param   mac_addr_word   Last two bytes of the mac address.
 */
static void loc_stackConf( uint16_t mac_addr_word )
{
    /* set last byte of mac address */
    mac_phy_config.mac_address[7] = (uint8_t)mac_addr_word;
    mac_phy_config.mac_address[6] = (uint8_t)(mac_addr_word >> 8);

    /* initial TX Power Output in dBm */
    mac_phy_config.init_power = TX_POWER;

    /* initial RX Sensitivity in dBm */
    mac_phy_config.init_sensitivity = RX_SENSITIVITY;

    /* initial wireless mode  */
    mac_phy_config.modulation = MODULATION;
}


/**
 * \brief   Configure the demo applications.
 *
 *          This function configures the demo applications according
 *          to the given selection. Depending which demos were selected
 *          the according configuration functions will be called.
 *
 * \param   ps_ns   Pointer to the stack structure to initialize.
 * \param   p_err   Pointer to store error status to.
 */
static void loc_demoAppsConf( s_ns_t* ps_ns, e_nsErr_t *p_err )
{

    EMB6_ASSERT_FN( (p_err != NULL), emb6_errorHandler( p_err ) );
    EMB6_ASSERT_RETS( (ps_ns != NULL), ,(*p_err), NETSTK_ERR_INVALID_ARGUMENT );

#if DEMO_USE_EXTIF
    demo_extifConf( ps_ns );
#endif /* #if DEMO_USE_EXTIF */

#if DEMO_USE_LWM2M
  demo_lwm2mConf( ps_ns );
#endif /* #if DEMO_USE_LWM2M */

#if DEMO_USE_COAP
    demo_coapConf( ps_ns );
#endif /* #if DEMO_USE_COAP */

#if DEMO_USE_MDNS
    demo_mdnsConf( ps_ns );
#endif /* #if DEMO_USE_MDNS */


#if DEMO_USE_UDPALIVE
    demo_udpAliveConf( ps_ns );
#endif /* #if DEMO_USE_UDPALIVE */

#if DEMO_USE_UDP_SOCKET
    demo_udpSocketCfg(ps_ns);
#endif /* #if DEMO_USE_UDP_SOCKET */

#if DEMO_USE_UDP_SOCKET_SIMPLE
  demo_udpSocketSimpleCfg(ps_ns);
#endif /* #if DEMO_USE_UDP_SOCKET_SIMPLE */

#if DEMO_USE_APTB
    demo_aptbConf( ps_ns );
#endif /* #if DEMO_USE_APTB */

#if DEMO_USE_UDP
    demo_udpSockConf( ps_ns );
#endif /* #if DEMO_USE_UDP */

#if DEMO_USE_TESTSUITE
    demo_testsuiteConf( ps_ns );
#endif /* #if DEMO_USE_TESTSUITE */

#if DEMO_USE_DTLS
    demo_dtlsConf( ps_ns );
#endif /* #if DEMO_USE_DTLS */

    /* set returned error code */
    *p_err = NETSTK_ERR_NONE;
}


/**
 * \brief   Initialize the demo applications.
 *
 *          This function initializes the demo applications according
 *          to the given selection. Depending which demos were selected
 *          the according initialization functions will be called.
 *
 * \return  0 on success or negative value on error.
 */
static int8_t loc_demoAppsInit( void )
{
#if DEMO_USE_EXTIF
    if( !demo_extifInit() )
        return -1;
#endif /* #if DEMO_USE_EXTIF */

#if DEMO_USE_COAP
    if( !demo_coapInit() )
        return -1;
#endif /* #if DEMO_USE_COAP */

#if DEMO_USE_LWM2M
    if( !demo_lwm2mInit() )
          return -1;
#endif /* #if DEMO_USE_LWM2M */

#if DEMO_USE_MDNS
    if( !demo_mdnsInit() )
        return -1;
#endif /* #if DEMO_USE_MDNS */


#if DEMO_USE_UDPALIVE
    if( !demo_udpAliveInit() )
        return -1;
#endif /* #if DEMO_USE_UDPALIVE */

#if DEMO_USE_UDP_SOCKET
    if( !demo_udpSocketInit() )
        return -1;
#endif /* #if DEMO_USE_UDP_SOCKET */

#if DEMO_USE_UDP_SOCKET_SIMPLE
  if( !demo_udpSocketSimpleInit() )
        return -1;
#endif /* #if DEMO_USE_UDP_SOCKET_SIMPLE */

#if DEMO_USE_APTB
    if( !demo_aptbInit() )
        return -1;
#endif /* #if DEMO_USE_APTB */

#if DEMO_USE_UDP
    if( !demo_udpSockInit() )
        return -1;
#endif /* #if DEMO_USE_UDP */

#if DEMO_USE_TESTSUITE
    if( !demo_testsuiteInit() )
        return -1;
#endif /* #if DEMO_USE_TESTSUITE */

#if DEMO_USE_DTLS
    if( !demo_dtlsInit() )
	      return -1;
#endif /* #if DEMO_USE_DTLS */

    return 0;
}


/**
 * \brief   Main emb6 task.
 *
 *          This function represents the main emb6 tasks. FIrst of all
 *          the board support package will be initialized to provide
 *          a well working hardware. As next steps the stack and demo
 *          applications will be configured before the stack is
 *          initialized. Finally the demo applications will be initialized
 *          and the stack is operating.
 *
 * \param   p_params      Parameters used to execute the stack.
 */
static void emb6_task( void* p_params )
{
    s_ns_t s_ns;
    s_emb6_startup_params_t* ps_params = p_params;
    uint8_t ret;
    e_nsErr_t err;

    /* Initialize variables */
    err = NETSTK_ERR_NONE;
    memset( &s_ns, 0, sizeof(s_ns) );

    /* Initialize BSP */
    ret = bsp_init( &s_ns );
    if( ret != 0 )
    {
        /* no recovery possible, call global error handler. */
        err = NETSTK_ERR_INIT;
        emb6_errorHandler( &err );
    }

    /* Configure stack parameters */
    loc_stackConf( ps_params->ui_macAddr );

    /* Configure applications */
    loc_demoAppsConf( &s_ns, &err );
    if( err != NETSTK_ERR_NONE )
    {
        /* no recovery possible, call global error handler. */
        emb6_errorHandler( &err );
    }

    /* Initialize stack */
    emb6_init( &s_ns, &err );
    if( err != NETSTK_ERR_NONE )
    {
        /* no recovery possible, call global error handler. */
        emb6_errorHandler(&err);
    }

    /* Show that stack has been launched */
    bsp_led(HAL_LED0, EN_BSP_LED_OP_ON);
    bsp_delayUs(2000000);
    bsp_led(HAL_LED0, EN_BSP_LED_OP_OFF);

    /* Initialize applications */
    ret = loc_demoAppsInit();
    if( ret != 0 )
    {
        LOG_ERR("Demo APP failed to initialize");
        err = NETSTK_ERR_INIT;
        emb6_errorHandler(&err);
    }

    while(1)
    {
        /* run the emb6 stack */
#if USE_FREERTOS
        emb6_process(-1);
#else
        emb6_process(EMB6_PROC_DELAY);
#endif /* #if USE_FREERTOS */
    }

    /* the program should never come here */
    return;
}

#if USE_FREERTOS
/*-----------------------------------------------------------*/
static void vLEDTask( void *pvParameters )
{
    s_led_task_param_t* p_params = (s_led_task_param_t*)pvParameters;
    while(1)
    {
        if( p_params->en )
        {
            bsp_led( HAL_LED0, EN_BSP_LED_OP_TOGGLE );
            bsp_led( HAL_LED1, EN_BSP_LED_OP_TOGGLE );
        }

        vTaskDelay( p_params->delay / portTICK_PERIOD_MS );
    }
}

/*-----------------------------------------------------------*/
void vApplicationIdleHook( void )
{
    /* Use the idle task to place the CPU into a low power mode.  Greater power
    saving could be achieved by not including any demo tasks that never block. */
    prvLowPowerMode1();
}

/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{

    /* This function will be called if a task overflows its stack, if
    configCHECK_FOR_STACK_OVERFLOW != 0.  It might be that the function
    parameters have been corrupted, depending on the severity of the stack
    overflow.  When this is the case pxCurrentTCB can be inspected in the
    debugger to find the offending task. */
    for( ;; );
}

/*-----------------------------------------------------------*/
static void prvSetupHardware( void )
{
    /* Nothing to do */
}

/*-----------------------------------------------------------*/
static void prvLowPowerMode1( void )
{
    /* not implemented */
}
#endif /* #if USE_FREERTOS */

/*==============================================================================
 main()
==============================================================================*/
#if defined(MAIN_WITH_ARGS)
int main( int argc, char **argv )
#else
int main( void )
#endif /* #if defined(MAIN_WITH_ARGS) */
{
  /* set startup parameter to zero */
  memset( &emb6_startupParams, 0, sizeof(emb6_startupParams) );

#if defined(MAIN_WITH_ARGS)
  if( argc > 1 )
  {
    emb6_startupParams.ui_macAddr = loc_parseMac(argv[1], MAC_ADDR_WORD);
  }
#endif /* #if defined(MAIN_WITH_ARGS) */
  emb6_startupParams.ui_macAddr = loc_parseMac(NULL, MAC_ADDR_WORD);

#if USE_FREERTOS
    ledTaskParams.en = 1;
    ledTaskParams.delay = 1000;

    /* Perform the necessary hardware configuration. */
    prvSetupHardware();

    /* Create EMB6 task. */
    xTaskCreate( emb6_task, "EMB6Task", 2048, &emb6_startupParams, mainEMB6_TASK_PRIORITY, NULL );


    /* Create LED task. */
    xTaskCreate( vLEDTask, "LEDTask", configMINIMAL_STACK_SIZE, &ledTaskParams, mainLED_TASK_PRIORITY, NULL );

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* The scheduler should now be running the tasks so the following code should
    never be reached.  If it is reached then there was insufficient heap space
    for the idle task to be created.  In this case the heap size is set by
    configTOTAL_HEAP_SIZE in FreeRTOSConfig.h. */
    for( ;; );
#else
    emb6_task( &emb6_startupParams );
#endif /* #if USE_FREERTOS */
}

