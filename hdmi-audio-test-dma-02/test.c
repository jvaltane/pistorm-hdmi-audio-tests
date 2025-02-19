#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <exec/execbase.h>
#include <clib/debug_protos.h>
#include <devices/inputevent.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/input.h>

#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "support.h"
#include "devicetree.h"
#include "mbox.h"
#include "dma.h"

#define CLK_BASE (ARM_BASE + 0x00101000)
#define A2W_PLLH_ANA0 ((volatile ULONG *)(CLK_BASE + 0x1070))
#define A2W_PLLH_CTRL ((volatile ULONG *)(CLK_BASE + 0x1160))
#define A2W_PLLH_FRAC ((volatile ULONG *)(CLK_BASE + 0x1260))


/* vc4 - HDMI */
/* HD - F2808000 */ 
#define HD_BASE       (ARM_BASE + 0x00808000)
#define HDMI_MAI_CTL  ((volatile ULONG *)(HD_BASE + 0x0014)) /* Audio control */
#define HDMI_MAI_THR  ((volatile ULONG *)(HD_BASE + 0x0018)) /* Audio DMA DREQ thresholds config */
#define HDMI_MAI_FMT  ((volatile ULONG *)(HD_BASE + 0x001C)) /* Audio format */
#define HDMI_MAI_DATA ((volatile ULONG *)(HD_BASE + 0x0020)) /* Audio FIFO */
#define HDMI_MAI_SMP  ((volatile ULONG *)(HD_BASE + 0x002C)) /* Audio clock division */

#define HD_BUS_BASE   (ARM_BUS_BASE + 0x00808000)
#define HDMI_MAI_DATA_BUS ((volatile ULONG *)(HD_BUS_BASE + 0x0020)) /* Audio FIFO */

/* HDMI - F2902000*/
#define HDMI_BASE                (ARM_BASE + 0x00902000)
#define HDMI_MAI_CHANNEL_MAP     ((volatile ULONG *)(HDMI_BASE + 0x0090)) /* Audio channel map */
#define HDMI_MAI_CONFIG          ((volatile ULONG *)(HDMI_BASE + 0x0094)) /* Audio config */
#define HDMI_AUDIO_PACKET_CONFIG ((volatile ULONG *)(HDMI_BASE + 0x009C)) /* Audio packet config */
#define HDMI_RAM_PACKET_CONFIG   ((volatile ULONG *)(HDMI_BASE + 0x00A0)) /* Info frame config */
#define HDMI_RAM_PACKET_STATUS   ((volatile ULONG *)(HDMI_BASE + 0x00A4)) /* Info frame status */
#define HDMI_CRP_CFG             ((volatile ULONG *)(HDMI_BASE + 0x00A8)) /* Content type reporting packet config */
#define HDMI_CTS_0               ((volatile ULONG *)(HDMI_BASE + 0x00AC)) /* Clock to service 0 */
#define HDMI_CTS_1               ((volatile ULONG *)(HDMI_BASE + 0x00B0)) /* Clock to service 1 */

/* RAM packet */
#define HDMI_RAM_PACKET_START  (HDMI_BASE + 0x400)
#define HDMI_RAM_PACKET(i, j)  ((volatile ULONG *)(HDMI_RAM_PACKET_START + (0x24 * (i) + (0x04 * (j)))))

#if  0
/* vc6 - HDMI0 */

TODO: Check base addresses.

/* HD */ 
#define HD_BASE       (ARM_BASE + 0x00808000)
#define HDMI_MAI_CTL  ((volatile ULONG *)(HD_BASE + 0x0010))
#define HDMI_MAI_THR  ((volatile ULONG *)(HD_BASE + 0x0014))
#define HDMI_MAI_FMT  ((volatile ULONG *)(HD_BASE + 0x0018))
#define HDMI_MAI_DATA ((volatile ULONG *)(HD_BASE + 0x001C))
#define HDMI_MAI_SMP  ((volatile ULONG *)(HD_BASE + 0x0020))

/* HDMI */
#define HDMI_BASE                (ARM_BASE + 0x00902000)
#define HDMI_MAI_CHANNEL_MAP     ((volatile ULONG *)(HDMI_BASE + 0x00A4))
#define HDMI_MAI_CONFIG          ((volatile ULONG *)(HDMI_BASE + 0x00A8))
#define HDMI_RAM_PACKET_CONFIG   ((volatile ULONG *)(HDMI_BASE + 0x00C4))
#define HDMI_RAM_PACKET_STATUS   ((volatile ULONG *)(HDMI_BASE + 0x00CC))
#define HDMI_CRP_CFG             ((volatile ULONG *)(HDMI_BASE + 0x00D0))
#define HDMI_CTS_0               ((volatile ULONG *)(HDMI_BASE + 0x00D4))
#define HDMI_CTS_1               ((volatile ULONG *)(HDMI_BASE + 0x00D8))

/* RAM packet */
#define HDMI_RAM_PACKET_BASE  (HDMI_BASE + 0x400)
#define HDMI_RAM_PACKET(i, j) ((volatile ULONG *)(HDMI_RAM_PACKET_BASE + (0x24 * (i) + (0x04 * (j)))))

/* vc6 - HDMI1 */

/* HD */ 
#define HD_BASE       (ARM_BASE + 0x00808000)
#define HDMI_MAI_CTL  ((volatile ULONG *)(HD_BASE + 0x0030))
#define HDMI_MAI_THR  ((volatile ULONG *)(HD_BASE + 0x0034))
#define HDMI_MAI_FMT  ((volatile ULONG *)(HD_BASE + 0x0038))
#define HDMI_MAI_DATA ((volatile ULONG *)(HD_BASE + 0x003C))
#define HDMI_MAI_SMP  ((volatile ULONG *)(HD_BASE + 0x0040))

/* HDMI */
#define HDMI_BASE                (ARM_BASE + 0x00902000)
#define HDMI_MAI_CHANNEL_MAP     ((volatile ULONG *)(HDMI_BASE + 0x00A4))
#define HDMI_MAI_CONFIG          ((volatile ULONG *)(HDMI_BASE + 0x00A8))
#define HDMI_RAM_PACKET_CONFIG   ((volatile ULONG *)(HDMI_BASE + 0x00C4))
#define HDMI_RAM_PACKET_STATUS   ((volatile ULONG *)(HDMI_BASE + 0x00CC))
#define HDMI_CRP_CFG             ((volatile ULONG *)(HDMI_BASE + 0x00D0))
#define HDMI_CTS_0               ((volatile ULONG *)(HDMI_BASE + 0x00D4))
#define HDMI_CTS_1               ((volatile ULONG *)(HDMI_BASE + 0x00D8))

/* RAM packet */
#define HDMI_RAM_PACKET_BASE  (HDMI_BASE + 0x400)
#define HDMI_RAM_PACKET(i, j) ((volatile ULONG *)(HDMI_RAM_PACKET_BASE + (0x24 * (i) + (0x04 * (j)))))
#endif

#define SAMPLE_RATE  48000 /* sample rate 32000, 44100, 48000... */

/* #define HDMI_CLOCK 163683000 magic number */

enum {
    RATE_INVALID = 0,
#if 0
    RATE_8000   = 0x01,
    RATE_11025  = 0x02,
    RATE_12000  = 0x03,
    RATE_16000  = 0x04,
    RATE_22050  = 0x05,
    RATE_24000  = 0x06,
#endif
    RATE_32000  = 0x07,
    RATE_44100  = 0x08,
    RATE_48000  = 0x09,
    /*      RATE_64000  = 0x0a,*/
    RATE_88200  = 0x0b,
    RATE_96000  = 0x0c,
    /*      RATE_128000 = 0x0d,*/
    RATE_176400 = 0x0e,
    RATE_192000 = 0x0f
};

enum hdmi_audio_sample_frequency {
    HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM,
    HDMI_AUDIO_SAMPLE_FREQUENCY_32000,
    HDMI_AUDIO_SAMPLE_FREQUENCY_44100,
    HDMI_AUDIO_SAMPLE_FREQUENCY_48000,
    HDMI_AUDIO_SAMPLE_FREQUENCY_88200,
    HDMI_AUDIO_SAMPLE_FREQUENCY_96000,
    HDMI_AUDIO_SAMPLE_FREQUENCY_176400,
    HDMI_AUDIO_SAMPLE_FREQUENCY_192000,
    HDMI_AUDIO_SAMPLE_FREQUENCY_INVALID
};


static char Version[]="\0$VER: audio test 0.3 (28.11.2024)\0";

static uint32_t pixelclock = 0; /* pixel clock */

extern struct DosLibrary *DOSBase;

/* from pistorm unicam */
static void myusleep(ULONG us)
{
    ULONG count;
    for (volatile uint32_t count = us*100; count > 0; count--) {
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
        asm volatile("nop");asm volatile("nop");asm volatile("nop");
    }
}

/* https://asurati.github.io/wip/post/2021/10/14/pixel-clock-pllh_pix/ */
/* https://github.com/torvalds/linux/blob/master/drivers/clk/bcm/clk-bcm2835.c */
#define A2W_PLL_FRAC_BITS 20
#define A2W_PLL_FRAC_MASK ((1 << A2W_PLL_FRAC_BITS) - 1)
#define A2W_PLL_CTRL_NDIV_MASK 0x0000003ff
#define A2W_PLL_CTRL_NDIV_SHIFT 0
#define A2W_PLL_CTRL_PDIV_MASK 0x000007000
#define A2W_PLL_CTRL_PDIV_SHIFT 12

/* this works with rpi3 only */
static ULONG calculate_pixel_clock (void)
{
    ULONG ctrl = LE32(*A2W_PLLH_CTRL);
    printf ("ctrl: 0x%08x\n", ctrl);
    ULONG fdiv = LE32(*A2W_PLLH_FRAC) & A2W_PLL_FRAC_MASK;
    printf ("fdiv: 0x%08x\n", fdiv);
    ULONG ndiv = (ctrl & A2W_PLL_CTRL_NDIV_MASK) >> A2W_PLL_CTRL_NDIV_SHIFT;
    printf ("ndiv: 0x%08x\n", ndiv);
    ULONG pdiv = (ctrl & A2W_PLL_CTRL_PDIV_MASK) >> A2W_PLL_CTRL_PDIV_SHIFT;
    printf ("pdiv: 0x%08x\n", pdiv);
    ULONG prediv_mask = (1 << 11);
    /* FIXME: if BCM2711 prediv_mask = 0 ...check others also */
    ULONG using_prediv = LE32(*A2W_PLLH_ANA0) & prediv_mask;
    printf ("using_prediv: %u\n", using_prediv);

    if (pdiv == 0) {
        printf ("pdiv == 0\n");
        return 0;
    }

    if (using_prediv != 0) {
        printf ("mutiply ndiv & \n");
        ndiv = ndiv * 2;
        fdiv = fdiv * 2;
    }

    ULONG val = (ndiv << A2W_PLL_FRAC_BITS) + fdiv;
    printf ("val: %u\n", val);
    uint64_t rate = (uint64_t)val * 1950000 / pdiv;
    rate = rate >> A2W_PLL_FRAC_BITS;
    return (ULONG)rate;
}

static uint32_t audio_frequency_to_rpi_rate (int32_t freq)
{
    uint32_t rate = RATE_INVALID;
    switch (freq) {
#if 0
        case 8000:
            rate = RATE_8000;
            break;
        case 11025:
            rate = RATE_11025;
            break;
        case 22050:
            rate = RATE_22050;
            break;
#endif
        case 32000:
            rate = RATE_32000;
            break;
        case 44100:
            rate = RATE_44100;
            break;
        case 48000:
            rate = RATE_48000;
            break;
#if 0
        case 64000:
            rate = RATE_64000;
            break;
#endif
        case 88200:
            rate = RATE_88200;
            break;
        case 96000:
            rate = RATE_96000;
            break;
#if 0
        case 128000:
            rate = RATE_128000;
            break;
#endif
        case 176400:
            rate = RATE_176400;
            break;
        case 192000:
            rate = RATE_192000;
            break;
        default:
            rate = RATE_INVALID;
            break;
    }

    return rate;
}

static uint32_t audio_frequency_to_hdmi_audio_sample_frequency (int32_t freq)
{
    uint32_t hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_44100;
    switch (freq) {
        case 0:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM;
            break;
        case 32000:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_32000;
            break;
        case 44100:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_44100;
            break;
        case 48000:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_48000;
            break;
        case 88200:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_88200;
            break;
        case 96000:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_96000;
            break;
        case 176400:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_176400;
            break;
        case 192000:
            hdmi_freq = HDMI_AUDIO_SAMPLE_FREQUENCY_192000;
            break;
        default:
            break;
    }
    return hdmi_freq;
}

static void hdmi_audio_stop_packet(int isforce)
{
    return;
    printf("hdmi_audio_stop_packet\n");
    *HDMI_RAM_PACKET_CONFIG &= LE32(~(1 << 4));
    if(isforce)
        return;
    while(1) {
        if(*HDMI_RAM_PACKET_STATUS & LE32(1 << 4))
            break;
    };
}

static void hdmi_audio_start_packet(int isforce)
{
    printf("hdmi_audio_start_packet\n");
    *HDMI_RAM_PACKET_CONFIG |= LE32(1 << 4);
    if(isforce)
        return;
    while(1) {
        if(*HDMI_RAM_PACKET_STATUS & LE32(1 << 4))
            break;
    };
}

static void hdmi_audio_reset(void)
{
    printf("hdmi_audio_reset\n");
    uint32_t mai_ctl = 0;
    hdmi_audio_stop_packet(1);
    mai_ctl = (1 << 0);  /* RST */
    mai_ctl = (1 << 2);  /* UF */
    mai_ctl = (1 << 9);  /* FLUSH */
    *HDMI_MAI_CTL = LE32(mai_ctl);
}

static void hdmi_audio_startup()
{
    printf("hdmi_audio_startup\n");
    uint32_t mai_ctl = 0;
    mai_ctl |= (1 << 0);  /* RST */
    mai_ctl |= (1 << 1);  /* OF */
    mai_ctl |= (1 << 2);  /* UF */
    mai_ctl |= (1 << 15); /* DLATE */
    mai_ctl |= (1 << 9);  /* FLUSH */
    *HDMI_MAI_CTL = LE32(mai_ctl);
}

#define min(x,y) ((x)<(y)?(x):(y))

/* from Linux kernel */
static void rational_best_approximation(
        uint32_t given_numerator, uint32_t given_denominator,
        uint32_t max_numerator, uint32_t max_denominator,
        uint32_t *best_numerator, uint32_t *best_denominator)
{
    uint32_t n, d, n0, d0, n1, d1, n2, d2;
    n = given_numerator;
    d = given_denominator;
    n0 = d1 = 0;
    n1 = d0 = 1;

    for (;;) {
        uint32_t dp, a;

        if (d == 0)
            break;

        dp = d;
        a = n / d;
        d = n % d;
        n = dp;

        n2 = n0 + a * n1;
        d2 = d0 + a * d1;

        if ((n2 > max_numerator) || (d2 > max_denominator)) {
            uint32_t t = UINT32_MAX;

            if (d1)
                t = (max_denominator - d0) / d1;
            if (n1)
                t = min(t, ((max_numerator - n0) / n1));

            if (!d1 || 2u * t > a || (2u * t == a && d0 * dp > d1 * d)) {
                n1 = n0 + t * n1;
                d1 = d0 + t * d1;
            }
            break;
        }
        n0 = n1;
        n1 = n2;
        d0 = d1;
        d1 = d2;
    }
    *best_numerator = n1;
    *best_denominator = d1;
}


static int hdmi_audio_prepare(void)
{
    printf("hdmi_audio_prepare\n");

    int32_t freq = SAMPLE_RATE;
    uint32_t rate = audio_frequency_to_rpi_rate (freq);
    if (rate == RATE_INVALID) {
        fprintf (stderr, "Could not find suitable RPI rate to audio freq (%d)\n", freq);
        return 1;
    }
    uint32_t hdmi_freq = audio_frequency_to_hdmi_audio_sample_frequency (freq);
    if (hdmi_freq == HDMI_AUDIO_SAMPLE_FREQUENCY_INVALID) {
        fprintf (stderr, "Could not find suitable HDMI sample frequency to audio freq (%d)\n", freq);
        return 1;
    }

    uint32_t n, m;
    rational_best_approximation(pixelclock, freq, 0xFFFFF, 0xFF, &n, &m);
    printf("n=%d\n", n);
    printf("m=%d\n", m);
    *HDMI_MAI_SMP = LE32((n << 8) | m);

    /* MAI */
    uint32_t mai_ctl = 0;
    int ch = 2; /* channels */
    mai_ctl |= (1 << 3);  /* enable */
    mai_ctl |= (ch << 4); /* channels */
    mai_ctl |= (1 << 12); /* ? */
    mai_ctl |= (1 << 13); /* ? */
    /* mai_ctl |= (1 << 15); starvation - new */
    *HDMI_MAI_CTL = LE32(mai_ctl);

    *HDMI_MAI_FMT = LE32((2 << 16) | ((rate & 0xFF) << 8)); /* PCM = 2 BCM = 200 */

    *HDMI_MAI_THR = LE32(0x08080608); /* DREQ ...or b24-31=Set panic data request threshold, b16-b23=Clear panic .., normal data ??? */

    *HDMI_MAI_CONFIG = LE32((1 << 27) | (1 << 26) | (1 << 1) | (1 << 0)); /* ? | ? | ch1 | ch0 */
    *HDMI_MAI_CHANNEL_MAP = LE32((1 << 3) | (0 << 0)); /* map ch1 -> ch1 | ch0 -> ch0 */

    uint32_t cfg = 0;
    cfg |= (1 << 29); /* ZERO DATA SAMPLE_FLAT */
    cfg |= (1 << 24); /* ZERO DATA INACTIVE CH */
    //cfg |= (0x08 << 10); /* B FRAME IDENT (0x08) */
    cfg |= (0x00 << 10); /* B FRAME IDENT (0x08) */
    cfg |= (1 << 1) | (1 << 0); /* Left aka ch1, Right...*/
    printf("audio packet config=0x%08x\n", cfg);
    *HDMI_AUDIO_PACKET_CONFIG = LE32(cfg);

    int cts_n = 128 * freq / 1000;
    *HDMI_CRP_CFG = LE32(cts_n | (1 << 24)); /* EXTERNAL CTS EN */
    *HDMI_CTS_0 = LE32(pixelclock / 1000);
    *HDMI_CTS_1 = LE32(pixelclock / 1000);

    //*HDMI_RAM_PACKET_CONFIG |= LE32((1 << 16)); // = LE32((0x0a << 16) | (1 << 8) | 0x84);
    *HDMI_RAM_PACKET_CONFIG = LE32((0x0a << 16) | (1 << 8) | 0x84);
    hdmi_audio_stop_packet(0);

    *HDMI_RAM_PACKET(0, (9 * 4) + 0) = LE32((0x0A << 16) | (1 << 8) | 0x84);
    *HDMI_RAM_PACKET(0, (9 * 4) + 1) = LE32((1 << 25) | (1 << 24) | (hdmi_freq << 10) | (1 << 8) | (1 << 4) | 1);
    *HDMI_RAM_PACKET(0, (9 * 4) + 2) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 3) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 4) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 5) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 6) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 7) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 4) + 8) = LE32(0x00000000);

    *HDMI_RAM_PACKET(0, (9 * 5) + 0) = LE32((0x0A << 16) | (1 << 8) | 0x84);
    *HDMI_RAM_PACKET(0, (9 * 5) + 1) = LE32((1 << 25) | (1 << 24) | (hdmi_freq << 10) | (1 << 8) | (1 << 4) | 1);
    *HDMI_RAM_PACKET(0, (9 * 5) + 2) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 3) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 4) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 5) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 6) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 7) = LE32(0x00000000);
    *HDMI_RAM_PACKET(0, (9 * 5) + 8) = LE32(0x00000000);
    hdmi_audio_start_packet(0);

    return 0;
}

static int hdmi_audio_setup(void)
{
    hdmi_audio_reset();
    hdmi_audio_startup();
    return hdmi_audio_prepare();
}

static int open_libs(void)
{
    DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0L);
    if (DOSBase == NULL) {
        return 1;
    }
    return 0;
}


static void close_libs(void)
{
    if (DOSBase != NULL) {
        CloseLibrary((struct Library *)DOSBase);
    }
}

static int hdmi_reset_if_busy (void)
{
    int busy = 0;
    int loop_count = 0;
    static int busy_th = 2;
    while(*HDMI_MAI_CTL & LE32(1 << 11)) {
        loop_count++;
        if(loop_count > busy_th) {
            loop_count = 0;
            busy_th <<= 1;
            *HDMI_MAI_CTL |= LE32(1 << 0); /* reset */
            myusleep(2000);
            *HDMI_MAI_CTL |= LE32(1 << 2); /* UF */
            *HDMI_MAI_CTL |= LE32(1 << 1); /* OF */
            busy = 1;
            break;
        }
    }
    return busy;
}

/* parity : https://github.com/michaelwu/alsa-lib/blob/master/src/pcm/pcm_iec958.c  */
//#define ADD_PREAMBLE 1
//#define ADD_PARITY 1
#ifdef ADD_PARITY
static inline uint32_t iec958_parity(uint32_t data)
{
    uint32_t parity;
    int32_t bit;

    data >>= 4;     /* start from bit 4 */
    parity = 0;
    for (bit = 4; bit <= 30; bit++) {
        if (data & 1) {
            parity++;
        }
        data >>= 1;
    }
    return (parity & 1);
}
#endif

int main(int argc, char **argv)
{
    BPTR f = 0;
    struct dcb *cb[2] = { NULL, NULL };
    uint32_t *buffer = NULL;
    uint8_t *rbuf = NULL;
    LONG pos = 0;
    LONG readlen;
    LONG filesize = 0;
    int retval = 0;
    uint32_t ch = 100; /* init as invalid */
    int index = 0;

    if (argc != 2) {
        printf("Usage: audio-test <file>\n");
        return 1;
    }
    printf("Play: %s\n", argv[1]);

    if (open_libs () != 0) {
        fprintf(stderr, "Could not open libs\n");
        return 1;
    }

    if (devicetree_init() != 0) {
        fprintf(stderr, "Could not init device tree\n");
        retval = 1; 
        goto error;
    }
    APTR mbox = devicetree_mbox_get();

    if (mbox_init (mbox) != 0) {
        fprintf(stderr, "Could not init mbox.\n");
        retval = 1; 
        goto error;
    }

    uint32_t cmask = mbox_dma_mask_get();
    printf ("DMA channel mask: 0x%08x\n", cmask);
    int i = 0;
    for (; i < 17; i++) {
        if (i > 15) break;
        if ((cmask & (1 << i)) > 0) {
            ch = i;
            printf("DMA channel %d\n", ch);
            break;
        }
    }
    if (i == 16) {
        fprintf(stderr, "Could not find free DMA channel.\n");
        retval = 1; 
        goto error;
    }

    pixelclock = calculate_pixel_clock ();
    printf ("Pixel clock: %u\n", pixelclock);

    if (dma_init() != 0) {
        fprintf(stderr, "Could not init DMA\n");
        retval = 1;
        goto error;
    }
    dma_reset(ch);

    if (hdmi_audio_setup() != 0) {
        fprintf(stderr, "Could not setup HDMI audio\n");
        retval = 1;
        goto error;
    }

    f = Open (argv[1], MODE_OLDFILE);
    if (f ==  0) {
        fprintf(stderr, "Could not open file\n");
        retval = 1;
        goto error;
    }

    if (Seek (f, 0, OFFSET_END) != 0) {
        fprintf(stderr, "Could not seek to end\n");
        retval = 1;
        goto error;
    }
    filesize = Seek (f, 0, OFFSET_BEGINNING);
    if (filesize < 2) {
        fprintf(stderr, "Too short file: %d\n", filesize);
        retval = 1;
        goto error;
    }
    if (filesize > BUF_SIZE/2) {
        printf("Limiting read to %dMB\n", BUF_SIZE/2/1024/1024);
        filesize = BUF_SIZE/2;
    }

    #define RBUF_SIZE 100
    rbuf = AllocVec (RBUF_SIZE, MEMF_CLEAR|MEMF_FAST);
    if (rbuf == NULL) {
        fprintf(stderr, "Could not alloc mem: %d\n", RBUF_SIZE);
        retval = 1; 
        goto error;
    }
    printf("Setting up DMA\n");

    for (int i = 0; i < 2; i++) {
        cb[i] = dma_get_mem(sizeof(struct dcb));
        if (cb[i] == NULL) {
            fprintf(stderr, "Could not get mem for cb[%d]: %d\n", i, sizeof(struct dcb));
            retval = 1; 
            goto error;
        }
    }
    buffer = (uint32_t *)dma_get_mem(filesize*2);
    if (buffer == NULL) {
        fprintf(stderr, "Could not get mem for buffer size: %d\n", filesize*2);
        retval = 1; 
        goto error;
    }

    dma_setup(cb[0], (uint32_t)buffer, (uint32_t)HDMI_MAI_DATA_BUS, filesize*2);
    cb[0]->next_cb = LE32((intptr_t)cb[1]);
    cb[1]->next_cb = LE32((intptr_t)cb[0]);

    printf("Reading file and converting to IEC958...\n");
    uint32_t bpos2 = 0;
#ifdef ADD_PREAMBLE
    uint32_t counter = 0;
#endif

    while ((readlen = Read (f, rbuf, RBUF_SIZE)) > 0) {
#ifdef ADD_STATUS_BIT
        uint32_t byte = counter >> 3;
        uint32_t mask = 1 << (counter - (byte << 3));
#endif
        for (int bpos = 0; bpos < readlen; bpos += 2) {
            ULONG *b = (ULONG *)&rbuf[bpos];
            ULONG data = (ULONG)LE32(*b);

            /* result (bits): 0000 xxxx xxxx xxxx xxxx yyyy yyyy 0000 - y:s for 24bit*/
            data <<= 16;
            data >>= 4;
            data &= ~0xF;
#ifdef ADD_STATUS_BIT

#endif

#ifdef ADD_PARITY
            if (iec958_parity(data))	/* parity bit 4-30 */
                data |= 0x80000000;
#endif
#ifdef ADD_PREAMBLE
            if (counter == 0) {
                data |= 0x00000008; // block start
            } else if (bpos2 % 2) {
                data |= 0x00000002; // X even
            } else {
                data |= 0x00000004; // Y odd
            }
            counter++;
            counter %= 192;
#endif
            buffer[bpos2++] = LE32(data);
        }
        if ((bpos2 % (1024*4)) == 0) printf ("\r%03d%", 200*bpos2/filesize);
    }
    printf ("\r%03d%\n", 200*bpos2/filesize);
    Close (f);
    f = 0;
    FreeVec(rbuf);
    rbuf = NULL;

    CacheClearE(buffer, filesize/2, CACRF_ClearD);
    CacheClearE(cb[0], sizeof(struct dcb), CACRF_ClearD);
    CacheClearE(cb[1], sizeof(struct dcb), CACRF_ClearD);
    printf("Playing...\n");
    dma_start(ch, cb[0]);

    printf("Press ctrl-c to quit. Playback is looping.\n");
    int quit = 0;
    ULONG signalCtrlC = SIGBREAKF_CTRL_C;
    while (quit == 0) {
        ULONG signals = Wait(signalCtrlC);
        if (signals & signalCtrlC) quit = 1;
    }
error:
    printf("Cleanup\n");
    dma_reset(ch);
    if (f != 0) Close (f);
    if (rbuf != NULL) FreeVec(rbuf);
    dma_free();
    mbox_free();
    close_libs();
    return retval;
}
