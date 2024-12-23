#ifndef _DMA_H
#define _DMA_H

#include <exec/types.h>
#include <stdint.h>
#define DMA_BASE          (ARM_BASE + 0x7000)
#define DMA_CH_CS(x)      ((volatile uint32_t *)(DMA_BASE + 0x00 + (0x100 * (x))))
#define DMA_CH_CB_ADDR(x) ((volatile uint32_t *)(DMA_BASE + 0x04 + (0x100 * (x))))
#define DMA_CH_DEBUG(x)   ((volatile uint32_t *)(DMA_BASE + 0x20 + (0x100 * (x))))

#define DMA_TI_PERMAP_HDMI (17)
// rpi4 HDMI0: 10, HDMI1: 17

/* dma control block - ti */
// https://github.com/Wallacoloo/Raspberry-Pi-DMA-Example/blob/master/dma-example.c
   //31:27 unused
        //26    NO_WIDE_BURSTS
        //21:25 WAITS; number of cycles to wait between each DMA read/write operation
        //16:20 PERMAP; peripheral number to be used for DREQ signal (pacing). set to 0 for unpaced DMA.
        //12:15 BURST_LENGTH
        //11    SRC_IGNORE; set to 1 to not perform reads. Used to manually fill caches
        //10    SRC_DREQ; set to 1 to have the DREQ from PERMAP gate requests.
        //9     SRC_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        //8     SRC_INC;   set to 1 to automatically increment the source address after each read (you'll want this if you're copying a range of memory)
        //7     DEST_IGNORE; set to 1 to not perform writes.
        //6     DEST_DREG; set to 1 to have the DREQ from PERMAP gate *writes*
        //5     DEST_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        //4     DEST_INC;   set to 1 to automatically increment the destination address after each read (Tyou'll want this if you're copying a range of memory)
        //3     WAIT_RESP; make DMA wait for a response from the peripheral during each write. Ensures multiple writes don't get stacked in the pipeline
        //2     unused (0)
        //1     TDMODE; set to 1 to enable 2D mode
        //0     INTEN;  set to 1 to generate an interrupt upon completion

#define DMA_TI_NO_WIDE   (1 << 26)
#define DMA_TI_PERMAP(x) (x << 16)
#define DMA_TI_SRC_DREQ  (1 << 10)
#define DMA_TI_SRC_INC   (1 << 8)
#define DMA_TI_DST_DREQ  (1 << 6)
#define DMA_TI_DST_INC   (1 << 4)
#define DMA_TI_WAIT_RESP (1 << 3)
#define DMA_TI_BURST(x)  ((x & 0xf) << 12)
#define DMA_TI_WAITS(x)  ((x & 0x1f) << 21)

/* dma - cs */
        //31    RESET; set to 1 to reset DMA
        //30    ABORT; set to 1 to abort current DMA control block (next one will be loaded & continue)
        //29    DISDEBUG; set to 1 and DMA won't be paused when debug signal is sent
        //28    WAIT_FOR_OUTSTANDING_WRITES; set to 1 and DMA will wait until peripheral says all writes have gone through before loading next CB
        //24-27 reserved
        //20-23 PANIC_PRIORITY; 0 is lowest priority
        //16-19 PRIORITY; bus scheduling priority. 0 is lowest
        //9-15  reserved
        //8     ERROR; read as 1 when error is encountered. error can be found in DEBUG register.
        //7     reserved
        //6     WAITING_FOR_OUTSTANDING_WRITES; read as 1 when waiting for outstanding writes
        //5     DREQ_STOPS_DMA; read as 1 if DREQ is currently preventing DMA
        //4     PAUSED R; 1 if DMA is paused
        //3     DREQ; copy of the data request signal from the peripheral, if DREQ is enabled. reads as 1 if data is being requested, else 0
        //2     INT; set when current CB ends and its INTEN=1. Write a 1 to this register to clear it
        //1     END; set when the transfer defined by current CB is complete. Write 1 to clear.
        //0     ACTIVE; write 1 to activate DMA (load the CB before hand)
#define DMA_CHANNEL_RESET (1 << 31)
#define DMA_CHANNEL_ABORT (1 << 30)
#define DMA_DISDEBUG (1 << 29)
#define DMA_WAIT_ON_WRITES (1 << 28)
#define DMA_PANIC_PRIORITY(x) ((x) << 20)
#define DMA_PRIORITY(x) ((x) << 16)
#define DMA_INTERRUPT_STATUS (1 << 2)
#define DMA_END_FLAG (1 << 1)
#define DMA_ACTIVE (1 << 0)

struct dcb
{
    uint32_t ti;
    uint32_t src;
    uint32_t dst;
    uint32_t len;
    uint32_t stride;
    uint32_t next_cb;
    uint32_t debug;
    uint32_t reserved;
} __attribute__((__packed__));

int dma_init(void);
void dma_free (void);
APTR dma_get_mem(size_t size);
int dma_reset (uint32_t ch);
void dma_setup (struct dcb *cb, uint32_t src, uint32_t dst, uint32_t size);
void dma_start (uint32_t ch, struct dcb *cb);
void dma_wait (uint32_t ch);

#endif /* _DMA_H */
