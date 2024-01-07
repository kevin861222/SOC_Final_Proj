/*
 * SPDX-FileCopyrightText: 2020 Efabless Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 */
#define DMA_wbs         0x30008000
#define DMA_offset_cfg  0x00
#define DMA_offset_addr 0x04
#define DMA_cfg  (DMA_wbs | DMA_offset_cfg)  // 0x3000_8000
#define DMA_addr (DMA_wbs | DMA_offset_addr) // 0x3000_8004

// This include is relative to $CARAVEL_PATH (see Makefile)
#include <defs.h>
#include <stub.c>
#include <user_uart.h>
#ifdef USER_PROJ_IRQ0_EN
#include <irq_vex.h>
#endif

// Matrix Multiplication
extern int* matmul();

// Quick Sort
extern int* qsort();

// FIR
extern int* fir();

// UART
extern void uart_write();
extern void uart_write_char();
extern void uart_write_string();
extern void uart_reset_write_fifo();
extern int uart_isr();
extern int uart_read();
void main()
{
	reg_wb_enable = 1;

    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_15 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_14 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_13 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_12 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_9  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_8  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_7  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_4  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_3  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_2  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_1  = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_0  = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_6  = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_5  = GPIO_MODE_USER_STD_INPUT_NOPULL;

	/* Apply configuration */
	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1);

	// Configure LA probes [31:0], [127:64] as inputs to the cpu 
	// Configure LA probes [63:32] as outputs from the cpu
	reg_la0_oenb = reg_la0_iena = 0x00000000;    // [31:0]
	reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;    // [63:32]
	reg_la2_oenb = reg_la2_iena = 0x00000000;    // [95:64]
	reg_la3_oenb = reg_la3_iena = 0x00000000;    // [127:96]


    //DMA - fir
     // DMA_addr[12:0]  = address_DMA2RAM
    // DMA_addr  = (THIS_TASK_DATA_IN_BRAM_ADDR<<0);
    // DMA_cfg[12]  = ap_busy (1 stands for DMA busy)             [Read only]
    // DMA_cfg[11]  = ap_idle (1 stands for DMA idle)             [Read only]
    // DMA_cfg[10]  = ap_start (1 stands for DMA start working)   [R/W]
    // DMA_cfg[9]   = type (mem->io=0, io->mem=1)                 [R/W]
    // DMA_cfg[8:7] = channel[1:0] (fir=0,matmul=1,sort=2)        [R/W]
    // DMA_cfg[6:0] = length[6:0]                                 [R/W]
    // FIR read parameter from BRAM to ASIC
    // DMA_cfg  |= (1<< 10) | (0 << 9) | (0 << 7) | ((64+11)<<0);
    // while(reg_mprj_datal = reg_la1_data[0] != 1); //when reg_la1_data[0] == 1 continue 
    // FIR write output from ASIC to BRAM
    // DMA_cfg  |= (1<< 10) | (1 << 9) | (0 << 7) | (64<<0);
    // Above configureation stands for
    // 1. DMA_addr,     Move data from address_src to address_dst
    // 2. DMA_cfg[6:0], data length DMA need to move
    // 3. DMA_cfg[8:7], Switch channel (fir/qsort/matmul)
    //                  DMA need to tell ASIC AXI-Stream data stands for
    // 4. DMA_cfg[9],   tell DMA that you need to start work!


	int *tmp;
    //DMA - fir
    // DMA_addr[12:0]  = address_DMA2RAM
    DMA_addr  = (THIS_TASK_DATA_IN_BRAM_ADDR<<0);
    DMA_cfg  |= (1<< 10) | (0 << 9) | (0 << 7) | ((64+11)<<0);
	// FIR
	reg_mprj_datal = reg_la1_data = 0xAB310000; // Start Flag
	tmp = fir();
	reg_mprj_datal = reg_la1_data = *tmp << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+1) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+2) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+3) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+4) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+5) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+6) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+7) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+8) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+9) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+10) << 16;	
	reg_mprj_datal = reg_la1_data = 0xAB390000; // End Flag
    while(reg_la1_data[0] == 1); //when reg_la1_data[0] == 1 continue 
    DMA_cfg  |= (1<< 10) | (1 << 9) | (0 << 7) | (64<<0);

    //DMA - matmul
     // DMA_addr[12:0]  = address_DMA2RAM
    DMA_addr  = (THIS_TASK_DATA_IN_BRAM_ADDR<<0);
    DMA_cfg  |= (1<< 10) | (0 << 9) | (1 << 7) | ((32)<<0);
	// Matrix Multiplication
	reg_mprj_datal = reg_la1_data = 0xAB110000; // Start Flag
	tmp = matmul();
	reg_mprj_datal = reg_la1_data = *tmp << 16;      // matmul[0] = 62
	reg_mprj_datal = reg_la1_data = *(tmp+1) << 16;  // matmul[1] = 68
	reg_mprj_datal = reg_la1_data = *(tmp+2) << 16;  // matmul[2] = 74
	reg_mprj_datal = reg_la1_data = *(tmp+3) << 16;  // matmul[3] = 80	
	reg_mprj_datal = reg_la1_data = 0xAB190000; // End Flag
    while(reg_mprj_datal = reg_la1_data[0] == 1); //when reg_la1_data[0] == 1 continue 
    DMA_cfg  |= (1<< 10) | (1 << 9) | (1 << 7) | (16<<0);
 
    //DMA - sort
    // DMA_addr[12:0]  = address_DMA2RAM
    DMA_addr  = (THIS_TASK_DATA_IN_BRAM_ADDR<<0);
    DMA_cfg  |= (1<< 10) | (0 << 9) | (2 << 7) | ((10)<<0);
	// Quick Sort
	reg_mprj_datal = reg_la1_data = 0xAB210000; // Start Flag
	tmp = qsort();
	reg_mprj_datal = reg_la1_data = *tmp << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+1) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+2) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+3) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+4) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+5) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+6) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+7) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+8) << 16;
	reg_mprj_datal = reg_la1_data = *(tmp+9) << 16;
	reg_mprj_datal = reg_la1_data = 0xAB290000; // End Flag
    while(reg_mprj_datal = reg_la1_data[0] == 1); //when reg_la1_data[0] == 1 continue 
    DMA_cfg  |= (1<< 10) | (1 << 9) | (2 << 7) | (10<<0);

}

