/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup Timer_Example main.c
 * @{
 * @ingroup Timer_example
 * @brief Timer_example Application main file.
 *
 * This file contains the source code for a simple application that prints a counter value that increments every 509 ms
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "boards.h"

#define TIMEOUT_US          509 * 1000 

const nrf_drv_timer_t TIMER_COUNTER = NRF_DRV_TIMER_INSTANCE(0);
uint8_t signal =0;

/**
 * @brief Handler for timer events.
 * @param[in] event_type Channel 0 is set so, verifying that event only
 * @param[in] p_context It has the value till that timer should run
 * @return none
 */
void increment_count_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    static uint32_t usCount=0;

    /* Increment the count as every microseconds interrupt will come */
    if(event_type == NRF_TIMER_EVENT_COMPARE0)
        usCount++;

    /* Compared count with value to give signal to main thread*/
    if(usCount == (uint32_t)p_context)
    {
        usCount = 0;
        signal = 1;
    }
}

/*
 * @brief Function for Timer to stop
 * @param[in] none
 * @return none
 */
void timer_stop()
{
    /* Timer Stop */
    nrfx_timer_disable(&TIMER_COUNTER);
}

/*
 * @brief Function for Timer to start with given value
 * @param[in] time_us Value till that timer should run
 * @param[in] event_handler function pointer to call after the timer interrupt occurs
 * @return none
 */
void timer_start(uint32_t time_us, void *event_handler)
{
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;
    
    /* Configure TIMER_COUNTER for incrementing the timer 
       Frequency = 1MHz
       Mode = Timer
       Bit Width = 16 */
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    
    /* Setting the ms value so in handler can access it */
    timer_cfg.p_context = (void *)time_us;
    
    /* Initializing timer init */
    err_code = nrf_drv_timer_init(&TIMER_COUNTER, &timer_cfg, event_handler);
    APP_ERROR_CHECK(err_code);

    /* Converting ms to ticks */
    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_COUNTER, time_us);

    /* Configuring Channel 0 */
    nrf_drv_timer_compare(
         &TIMER_COUNTER, NRF_TIMER_CC_CHANNEL0, time_ticks, true);

    /* Timer start */
    nrf_drv_timer_enable(&TIMER_COUNTER);
}

/**
 * @brief Function for application main entry.
 * it prints the count after the signal from event handler is set
 */
int main(void)
{
    uint32_t counter = 1;          /* Initialising to 1, as after the ms it will print */
    NRF_LOG_INFO("Timer started\n");

    /* Starting the timer */
    timer_start(TIMEOUT_US, &increment_count_event_handler);

    /* Continuously running */
    while (true)
    {
        /* Wait till signal is set from timer event */
        while(!signal);
      
        /* Clear the signal and print the counter */
        signal = 0;
        NRF_LOG_INFO("%d\n",counter++);
    }
}
