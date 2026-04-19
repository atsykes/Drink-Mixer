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
    /* MOSI, SCK, SS as outputs; MISO as input */
    DDRB |= (1 << PB3) | (1 << PB5) | (1 << RC522_CS_PIN);
    DDRB &= ~(1 << PB4);

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
    RC522_RST_DDR |= (1 << RC522_RST_PIN);
    rst_high();

    uart_init();
    spi_init();

    /* Give the RC522 a clean reset pulse */
    rst_low();
    _delay_ms(10);
    rst_high();
    _delay_ms(10);
}

// TEST CODE FOR RFID: 

// int main(void) {
//     uint8_t status;
//     uint8_t tag_type[2];
//     uint8_t uid[5];

//     hardware_init();
//     rc522_init();

//     uart_print("RC522 ready\r\n");

//     while (1) {
//         status = rc522_request(PICC_REQIDL, tag_type);
//         if (status == MI_OK) {
//             status = rc522_anticoll(uid);
//             if (status == MI_OK) {
//                 print_uid(uid, 4);
//                 _delay_ms(500);
//             }
//         }
//     }
// }