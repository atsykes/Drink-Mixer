#include "rfid.h"



 void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);
    UCSR0A = 0;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

//  void uart_init(void) {
//     /* 9600 baud @ 7.3728 MHz, normal speed */
//     UBRR0H = 0;
//     UBRR0L = 47;
//     UCSR0A = 0;
//     UCSR0B = (1 << TXEN0);
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
// }

 void uart_tx(char c) {
    while (!(UCSR0A & (1 << UDRE0))) {
        ;
    }
    UDR0 = (uint8_t)c;
}

 void uart_print(const char *s) {
    while (*s) {
        uart_tx(*s++);
    }
}

 void uart_print_hex8(uint8_t v) {
    const char hex[] = "0123456789ABCDEF";
    uart_tx(hex[(v >> 4) & 0x0F]);
    uart_tx(hex[v & 0x0F]);
}

 void spi_init(void) {
    // /* MOSI, SCK, SS as outputs; MISO as input */
    // DDRB |= (1 << PB3) | (1 << PB5) | (1 << RC522_CS_PIN);
    // DDRB &= ~(1 << PB4);

        /* ATmega1284P SPI pins */
    DDRB |= (1 << PB5) | (1 << PB7) | (1 << PB4) | (1 << RC522_CS_PIN);
    DDRB &= ~(1 << PB6);

    /* Keep chip-select high */
    RC522_CS_PORT |= (1 << RC522_CS_PIN);

    /* Enable SPI master, clock = fosc/2 (SPI2X=1) */
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);
}

 uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF))) {
        ;
    }
    return SPDR;
}

 inline void cs_low(void) {
    RC522_CS_PORT &= ~(1 << RC522_CS_PIN);
}

 inline void cs_high(void) {
    RC522_CS_PORT |= (1 << RC522_CS_PIN);
}

 inline void rst_low(void) {
    RC522_RST_PORT &= ~(1 << RC522_RST_PIN);
}

 inline void rst_high(void) {
    RC522_RST_PORT |= (1 << RC522_RST_PIN);
}

 void rc522_write_reg(uint8_t reg, uint8_t value) {
    cs_low();
    spi_transfer((reg << 1) & 0x7E);
    spi_transfer(value);
    cs_high();
}

 uint8_t rc522_read_reg(uint8_t reg) {
    uint8_t value;
    cs_low();
    spi_transfer(((reg << 1) & 0x7E) | 0x80);
    value = spi_transfer(0x00);
    cs_high();
    return value;
}

 void rc522_set_bitmask(uint8_t reg, uint8_t mask) {
    rc522_write_reg(reg, rc522_read_reg(reg) | mask);
}

 void rc522_clear_bitmask(uint8_t reg, uint8_t mask) {
    rc522_write_reg(reg, rc522_read_reg(reg) & (uint8_t)(~mask));
}

 void rc522_antenna_on(void) {
    uint8_t temp = rc522_read_reg(RC522_TX_CONTROL_REG);
    if ((temp & 0x03) != 0x03) {
        rc522_set_bitmask(RC522_TX_CONTROL_REG, 0x03);
    }
}

 void rc522_reset(void) {
    rc522_write_reg(RC522_COMMAND_REG, PCD_SOFT_RESET);
    _delay_ms(50);
}

 void rc522_init(void) {
    rc522_reset();

    /* Timer and modulation setup */
    rc522_write_reg(RC522_T_MODE_REG, 0x8D);
    rc522_write_reg(RC522_T_PRESCALER_REG, 0x3E);
    rc522_write_reg(RC522_T_RELOAD_L_REG, 30);
    rc522_write_reg(RC522_T_RELOAD_H_REG, 0);

    rc522_write_reg(RC522_TX_ASK_REG, 0x40);
    rc522_write_reg(RC522_MODE_REG, 0x3D);

    /* Recommended defaults */
    rc522_write_reg(RC522_TX_MODE_REG, 0x00);
    rc522_write_reg(RC522_RX_MODE_REG, 0x00);
    rc522_write_reg(0x24, 0x26);

    rc522_antenna_on();
}

 uint8_t rc522_to_card(uint8_t command,
                             const uint8_t *send_data,
                             uint8_t send_len,
                             uint8_t *back_data,
                             uint16_t *back_len) {
    uint8_t status = MI_ERR;
    uint8_t irq_en = 0x00;
    uint8_t wait_irq = 0x00;
    uint8_t n;
    uint16_t i;
    uint8_t last_bits;

    if (command == PCD_TRANSCEIVE) {
        irq_en = 0x77;
        wait_irq = 0x30;
    } else if (command == PCD_CALCCRC) {
        irq_en = 0x04;
        wait_irq = 0x04;
    }

    rc522_write_reg(RC522_COM_IEN_REG, irq_en | 0x80);
    rc522_clear_bitmask(RC522_COM_IRQ_REG, 0x80);
    rc522_set_bitmask(RC522_FIFO_LEVEL_REG, 0x80);
    rc522_write_reg(RC522_COMMAND_REG, PCD_IDLE);

    for (i = 0; i < send_len; i++) {
        rc522_write_reg(RC522_FIFO_DATA_REG, send_data[i]);
    }

    rc522_write_reg(RC522_COMMAND_REG, command);
    if (command == PCD_TRANSCEIVE) {
        rc522_set_bitmask(RC522_BIT_FRAMING_REG, 0x80);
    }

    i = 2000;
    do {
        n = rc522_read_reg(RC522_COM_IRQ_REG);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & wait_irq));

    rc522_clear_bitmask(RC522_BIT_FRAMING_REG, 0x80);

    if (i != 0) {
        uint8_t error;
        error = rc522_read_reg(RC522_ERROR_REG);
        if ((error & 0x1B) == 0x00) {
            status = MI_OK;

            if (n & irq_en & 0x01) {
                status = MI_NOTAGERR;
            }

            if (command == PCD_TRANSCEIVE) {
                n = rc522_read_reg(RC522_FIFO_LEVEL_REG);
                last_bits = rc522_read_reg(RC522_CONTROL_REG) & 0x07;

                if (last_bits) {
                    *back_len = (uint16_t)((n - 1) * 8 + last_bits);
                } else {
                    *back_len = (uint16_t)(n * 8);
                }

                if (n == 0) {
                    n = 1;
                }
                if (n > 16) {
                    n = 16;
                }

                for (i = 0; i < n; i++) {
                    back_data[i] = rc522_read_reg(RC522_FIFO_DATA_REG);
                }
            }
        }
    }

    return status;
}

 uint8_t rc522_request(uint8_t req_mode, uint8_t *tag_type) {
    uint8_t status;
    uint16_t back_bits;

    rc522_write_reg(RC522_BIT_FRAMING_REG, 0x07);
    tag_type[0] = req_mode;

    status = rc522_to_card(PCD_TRANSCEIVE, tag_type, 1, tag_type, &back_bits);
    if ((status != MI_OK) || (back_bits != 0x10)) {
        status = MI_ERR;
    }

    return status;
}

 uint8_t rc522_anticoll(uint8_t *ser_num) {
    uint8_t status;
    uint8_t i;
    uint8_t ser_num_check = 0;
    uint16_t un_len;

    rc522_write_reg(RC522_BIT_FRAMING_REG, 0x00);

    ser_num[0] = PICC_ANTICOLL;
    ser_num[1] = 0x20;

    status = rc522_to_card(PCD_TRANSCEIVE, ser_num, 2, ser_num, &un_len);
    if (status == MI_OK) {
        for (i = 0; i < 4; i++) {
            ser_num_check ^= ser_num[i];
        }
        if (ser_num_check != ser_num[4]) {
            status = MI_ERR;
        }
    }

    return status;
}

 void print_uid(const uint8_t *uid, uint8_t len) {
    uint8_t i;

    uart_print("Card UID: ");
    for (i = 0; i < len; i++) {
        uart_print_hex8(uid[i]);
        if (i + 1 < len) {
            uart_tx(':');
        }
    }
    uart_print("\r\n");
}

 void hardware_init(void) {
    /* RC522 reset pin */
    // RC522_RST_DDR |= (1 << RC522_RST_PIN);
    // rst_high();
    RC522_RST_DDR |= (1 << RC522_RST_PIN);
    DDRB |= (1 << PB4);   // <-- VERY IMPORTANT (SS must be output)

    uart_init();
    spi_init();

    /* Give the RC522 a clean reset pulse */
    rst_low();
    _delay_ms(10);
    rst_high();
    _delay_ms(10);
}

// TEST CODE FOR RFID: 
// ***************************MAIN CODE***********************************
// int main(void) {
//     uint8_t status;
//     uint8_t tag_type[2];
//     uint8_t uid[5];

//     hardware_init();
//     rc522_init();

//     uart_print("RC522 ready\r\n");

//     while (1) {
//         uart_print("RC522 ready\r\n");
//         status = rc522_request(PICC_REQIDL, tag_type);
//         if (status == MI_OK) {
//             uart_print("RC522 REQUESTED\r\n");
//             status = rc522_anticoll(uid);
//             if (status == MI_OK) {
//                 uart_print("RC522 RECEIVED\r\n");
//                 print_uid(uid, 4);
//                 _delay_ms(500);
//             }
//         }
//         // uart_print("==============");
//         // uart_print("Reprinting: ");
//         // print_uid(uid, 4);
//         // uart_print("==============");
        
//         _delay_ms(1000);
//     }
// }
// ***************************MAIN CODE***********************************

// int main(void)
// {
//     uint8_t status;
//     uint8_t tag_type[2];
//     uint8_t uid[5];

//     hardware_init();   // keep your existing init
//     rc522_init();

//     uart_print("RC522 ready\r\n");

//     while (1)
//     {
//         status = rc522_request(PICC_REQIDL, tag_type);

//         if (status == MI_OK)
//         {
//             uart_print("Card detected\r\n");

//             status = rc522_anticoll(uid);

//             if (status == MI_OK)
//             {
//                 uart_print("UID: ");

//                 uint8_t i;
//                 for (i = 0; i < 4; i++)
//                 {
//                     uart_print_hex8(uid[i]);
//                     uart_tx(' ');
//                 }

//                 uart_print("\r\n");

//                 _delay_ms(500); // debounce
//             }
//         }
//     }
// }
// ---------------------------------------------------------------------------


// #define F_CPU 1000000UL

// #include <avr/io.h>
// // #include <util/delay.h>
// // #include "rfid.h"

// /* ---------------- UART ---------------- */

// #define BAUD 9600
// #define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

// #include "rfid.h"
// #include <util/delay.h>

// /* ------------------------------------------------------------
//  * SPI INIT for ATmega1284P
//  * ------------------------------------------------------------ */
// void spi_init(void)
// {
//     /* MOSI (PB5), SCK (PB7), SS (PB4), CS as outputs */
//     DDRB |= (1 << PB5) | (1 << PB7) | (1 << PB4) | (1 << RC522_CS_PIN);

//     /* MISO (PB6) as input */
//     DDRB &= ~(1 << PB6);

//     /* Enable SPI, Master mode, fosc/16 (safe for slow MCU) */
//     SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
//     SPSR = 0;

//     /* CS high (inactive) */
//     RC522_CS_PORT |= (1 << RC522_CS_PIN);
// }

// /* ------------------------------------------------------------
//  * SPI TRANSFER
//  * ------------------------------------------------------------ */
// uint8_t spi_transfer(uint8_t data)
// {
//     SPDR = data;
//     while (!(SPSR & (1 << SPIF)));
//     return SPDR;
// }

// /* ------------------------------------------------------------
//  * RC522 LOW-LEVEL
//  * ------------------------------------------------------------ */
// void rc522_write(uint8_t reg, uint8_t val)
// {
//     RC522_CS_PORT &= ~(1 << RC522_CS_PIN);

//     spi_transfer((reg << 1) & 0x7E);  // write address
//     spi_transfer(val);

//     RC522_CS_PORT |= (1 << RC522_CS_PIN);
// }

// uint8_t rc522_read(uint8_t reg)
// {
//     uint8_t val;

//     RC522_CS_PORT &= ~(1 << RC522_CS_PIN);

//     spi_transfer(((reg << 1) & 0x7E) | 0x80);  // read address
//     val = spi_transfer(0x00);

//     RC522_CS_PORT |= (1 << RC522_CS_PIN);

//     return val;
// }

// /* ------------------------------------------------------------
//  * RESET
//  * ------------------------------------------------------------ */
// void rc522_reset(void)
// {
//     RC522_RST_PORT &= ~(1 << RC522_RST_PIN);
//     _delay_ms(10);
//     RC522_RST_PORT |= (1 << RC522_RST_PIN);
//     _delay_ms(10);
// }

// /* ------------------------------------------------------------
//  * INIT
//  * ------------------------------------------------------------ */
// void rc522_init(void)
// {
//     /* Set RST pin as output */
//     RC522_RST_DDR |= (1 << RC522_RST_PIN);

//     spi_init();
//     rc522_reset();

//     rc522_write(CommandReg, PCD_SoftReset);
//     _delay_ms(50);

//     /* Timer settings */
//     rc522_write(TModeReg, 0x8D);
//     rc522_write(TPrescalerReg, 0x3E);
//     rc522_write(TReloadRegL, 30);
//     rc522_write(TReloadRegH, 0);

//     rc522_write(TxASKReg, 0x40);
//     rc522_write(ModeReg, 0x3D);

//     /* Turn antenna ON */
//     uint8_t temp = rc522_read(TxControlReg);
//     if (!(temp & 0x03))
//     {
//         rc522_write(TxControlReg, temp | 0x03);
//     }
// }

// void uart_init(void)
// {
//     UBRR0H = (UBRR_VALUE >> 8);
//     UBRR0L = UBRR_VALUE;

//     UCSR0B = (1 << RXEN0) | (1 << TXEN0);
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
// }

// void uart_tx(char c)
// {
//     while (!(UCSR0A & (1 << UDRE0)));
//     UDR0 = c;
// }

// void uart_print(const char *s)
// {
//     while (*s) uart_tx(*s++);
// }



// void print_uid(const uint8_t *uid, uint8_t len) {
//     uint8_t i;

//     uart_print("Card UID: ");
//     for (i = 0; i < len; i++) {
//         uart_print_hex(uid[i]);
//         if (i + 1 < len) {
//             uart_tx(':');
//         }
//     }
//     uart_print("\r\n");
// }

// // void uart_print_hex8(uint8_t v) {
// //     const char hex[] = "0123456789ABCDEF";
// //     uart_tx(hex[(v >> 4) & 0x0F]);
// //     uart_tx(hex[v & 0x0F]);
// // }

// void uart_print_hex(uint8_t val)
// {
//     char hex[] = "0123456789ABCDEF";
//     uart_tx(hex[val >> 4]);
//     uart_tx(hex[val & 0x0F]);
// }

// uint8_t rc522_to_card(uint8_t command, uint8_t *sendData, uint8_t sendLen,
//                       uint8_t *backData, uint16_t *backLen)
// {
//     uint8_t status = 1;
//     uint8_t irqEn = 0x00;
//     uint8_t waitIRq = 0x00;
//     uint8_t lastBits;
//     uint8_t n;
//     uint16_t i;

//     if (command == PCD_Transceive) {
//         irqEn = 0x77;
//         waitIRq = 0x30;
//     }

//     rc522_write(ComIEnReg, irqEn | 0x80);
//     rc522_write(ComIrqReg, 0x7F);
//     rc522_write(FIFOLevelReg, 0x80);

//     rc522_write(CommandReg, PCD_Idle);

//     for (i = 0; i < sendLen; i++)
//         rc522_write(FIFODataReg, sendData[i]);

//     rc522_write(CommandReg, command);

//     if (command == PCD_Transceive)
//         rc522_write(BitFramingReg, 0x80);

//     i = 2000;
//     do {
//         n = rc522_read(ComIrqReg);
//         i--;
//     } while (i && !(n & 0x01) && !(n & waitIRq));

//     rc522_write(BitFramingReg, 0x00);

//     if (i != 0 && !(rc522_read(ErrorReg) & 0x1B)) {
//         status = 0;

//         if (n & irqEn & 0x01)
//             status = 1;

//         if (command == PCD_Transceive) {
//             n = rc522_read(FIFOLevelReg);
//             lastBits = rc522_read(ControlReg) & 0x07;

//             if (lastBits)
//                 *backLen = (n - 1) * 8 + lastBits;
//             else
//                 *backLen = n * 8;

//             for (i = 0; i < n; i++)
//                 backData[i] = rc522_read(FIFODataReg);
//         }
//     }

//     return status;
// }

// uint8_t rc522_request(uint8_t *tagType)
// {
//     uint8_t status;
//     uint16_t backBits;
//     uint8_t buf[1];

//     buf[0] = PICC_REQIDL;

//     rc522_write(BitFramingReg, 0x07);

//     status = rc522_to_card(PCD_Transceive, buf, 1, tagType, &backBits);

//     if ((status != 0) || (backBits != 0x10))
//         status = 1;

//     return status;
// }

// uint8_t rc522_anticoll(uint8_t *serNum)
// {
//     uint8_t status;
//     uint8_t i;
//     uint16_t backBits;
//     uint8_t buf[2];

//     buf[0] = PICC_ANTICOLL;
//     buf[1] = 0x20;

//     rc522_write(BitFramingReg, 0x00);

//     status = rc522_to_card(PCD_Transceive, buf, 2, serNum, &backBits);

//     if (status == 0) {
//         uint8_t check = 0;
//         for (i = 0; i < 4; i++)
//             check ^= serNum[i];

//         if (check != serNum[4])
//             status = 1;
//     }

//     return status;
// }

//  uint8_t rc522_anticoll(uint8_t *ser_num) {
//     uint8_t status;
//     uint8_t i;
//     uint8_t ser_num_check = 0;
//     uint16_t un_len;

//     rc522_write_reg(RC522_BIT_FRAMING_REG, 0x00);

//     ser_num[0] = PICC_ANTICOLL;
//     ser_num[1] = 0x20;

//     status = rc522_to_card(PCD_TRANSCEIVE, ser_num, 2, ser_num, &un_len);
//     if (status == MI_OK) {
//         for (i = 0; i < 4; i++) {
//             ser_num_check ^= ser_num[i];
//         }
//         if (ser_num_check != ser_num[4]) {
//             status = MI_ERR;
//         }
//     }

//     return status;
// }



/* ---------------- MAIN ---------------- */

// int main(void)
// {
//     uart_init();
//     rc522_init();

//     uart_print("RFID Ready\r\n");
//     uart_print("Version: 0x");

//     while (1)
//     {
//         uart_print("RFID Ready\r\n");
//         uint8_t status;
//         uint8_t tagType[2];
//         uint8_t uid[5];

//         status = rc522_request(tagType);

//         if (status == 0)
//         {
//             uart_print("Card detected\r\n");

//             status = rc522_anticoll(uid);

//             if (status == 0)
//             {
                
//                 uart_print("UID: ");

//                 uint8_t i;
//                 for (i = 0; i < 4; i++)
//                 {
//                     uart_print_hex(uid[i]);
//                     uart_print(" ");
//                 }

//                 uart_print("\r\n");
//             }
//             status = rc522_request(PICC_REQIDL, tag_type);
//             // ------------------------------
//             if (status == MI_OK) {
//                 status = rc522_anticoll(uid);
//                 if (status == MI_OK) {
//                     print_uid(uid, 4);
//                     _delay_ms(500);
//                 }
//             }
//         }

//         _delay_ms(500);
//     }
//     // while (1)
//     // {
//     //     uint8_t version = rc522_read(0x37);  // VersionReg

//     //     uart_print("Version: 0x");
//     //     uart_print_hex(version);
//     //     uart_print("\r\n");

//     //     _delay_ms(1000);
//     // }
// }

// int main(void)
// {
//     uart_init();

//     uart_print("START\r\n");

//     rc522_init();

//     uart_print("RC522 INIT DONE\r\n");

//     while (1)
//     {
//         uint8_t status;
//         uint8_t tagType[2];
//         uint8_t uid[5];

//         // 🔹 Always show RC522 is alive
//         uint8_t version = rc522_read(0x37);
//         uart_print("Version: 0x");
//         uart_print_hex(version);
//         uart_print("\r\n");

//         // 🔹 Try to detect card
//         status = rc522_request(tagType);

//         if (status == 0)
//         {
//             uart_print("CARD DETECTED\r\n");

//             status = rc522_anticoll(uid);

//             if (status == 0)
//             {
//                 uart_print("UID: ");

//                 uint8_t i;
//                 for (i = 0; i < 4; i++)
//                 {
//                     uart_print_hex(uid[i]);
//                     uart_print(" ");
//                 }

//                 uart_print("\r\n");
//             }
//             else
//             {
//                 uart_print("ANTICOLL FAILED\r\n");
//             }
//         }
//         else
//         {
//             uart_print("No card\r\n");
//         }

//         _delay_ms(500);
//     }
// }