
#include <msp430.h>

#include "commands.h"
#include "spi_usci_CC1101_hal.h"
#include "various.h"
#include "rf_CC1101_hal.h"

#define PACKET_LENGTH 0x10;
    
void RF_init()
{
  /* Generated by SmartRF studio using the following config:
    RF_write_reg( @RN@@<<@, @<<@0x@VH@);  @<<@// @Rd@   

     Deviation = 19.042969 
     Base frequency = 867.999939 
     Carrier frequency = 867.999939 
     Channel number = 0 
     Carrier frequency = 867.999939 
     Modulated = true 
     Modulation format = GFSK 
     Manchester enable = false 
     Sync word qualifier mode = 30/32 sync word bits detected 
     Preamble count = 4 
     Channel spacing = 199.951172 
     Carrier frequency = 867.999939 
     Data rate = 9.9926 
     RX filter BW = 101.562500 
     Data format = Normal mode 
     Length config = Variable packet length mode. Packet length configured by the first byte after sync word 
     CRC enable = true 
     Packet length = 255 
     Device address = 0 
     Address config = No address check 
     CRC autoflush = false 
     PA ramping = false 
     TX power = 0 
    */
  
  /* RF settings LarsRF */

  RF_write_reg( FSCTRL1 ,     0x06);      // Frequency Synthesizer Control   
  RF_write_reg( IOCFG0  ,     0x06);      // GDO0 Output Pin Configuration   
  RF_write_reg( FSCTRL0 ,     0x00);      // Frequency Synthesizer Control   
  RF_write_reg( FREQ2   ,     0x21);      // Frequency Control Word, High Byte   
  RF_write_reg( FREQ1   ,     0x62);      // Frequency Control Word, Middle Byte   
  RF_write_reg( FREQ0   ,     0x76);      // Frequency Control Word, Low Byte   
  RF_write_reg( MDMCFG4 ,     0xC8);      // Modem Configuration   
  RF_write_reg( MDMCFG3 ,     0x93);      // Modem Configuration   
  RF_write_reg( MDMCFG2 ,     0x13);      // Modem Configuration   
  RF_write_reg( MDMCFG1 ,     0x22);      // Modem Configuration   
  RF_write_reg( MDMCFG0 ,     0xF8);      // Modem Configuration   
  RF_write_reg( CHANNR  ,     0x00);      // Channel Number   
  RF_write_reg( DEVIATN ,     0x34);      // Modem Deviation Setting   
  RF_write_reg( FREND1  ,     0x56);      // Front End RX Configuration   
  RF_write_reg( FREND0  ,     0x10);      // Front End TX Configuration   
  RF_write_reg( MCSM0   ,     0x18);      // Main Radio Control State Machine Configuration   
  RF_write_reg( FOCCFG  ,     0x16);      // Frequency Offset Compensation Configuration   
  RF_write_reg( BSCFG   ,     0x6C);      // Bit Synchronization Configuration   
  RF_write_reg( AGCCTRL2,     0x43);      // AGC Control   
  RF_write_reg( AGCCTRL1,     0x40);      // AGC Control   
  RF_write_reg( AGCCTRL0,     0x91);      // AGC Control   
  RF_write_reg( FSCAL3  ,     0xE9);      // Frequency Synthesizer Calibration   
  RF_write_reg( FSCAL2  ,     0x2A);      // Frequency Synthesizer Calibration   
  RF_write_reg( FSCAL1  ,     0x00);      // Frequency Synthesizer Calibration   
  RF_write_reg( FSCAL0  ,     0x1F);      // Frequency Synthesizer Calibration   
  RF_write_reg( FSTEST  ,     0x59);      // Frequency Synthesizer Calibration Control   
  RF_write_reg( TEST2   ,     0x81);      // Various Test Settings   
  RF_write_reg( TEST1   ,     0x35);      // Various Test Settings   
  RF_write_reg( TEST0   ,     0x09);      // Various Test Settings   
  RF_write_reg( FIFOTHR ,     0x07);      // RX FIFO and TX FIFO Thresholds   
  RF_write_reg( IOCFG2  ,     0x29);      // GDO2 Output Pin Configuration   
  RF_write_reg( PKTCTRL1,     0x04);      // Packet Automation Control   
  RF_write_reg( PKTCTRL0,     0x05);      // Packet Automation Control   
  RF_write_reg( ADDR    ,     0x00);      // Device Address   
  RF_write_reg( PKTLEN  ,     0xFF);      // Packet Length   

}


void RF_strobe( uint8 strobe )
{
    SPI_strobe( strobe);
}

uint8 RF_read_reg( uint8 addr )
{
    return SPI_read_single( addr);
}

void RF_write_reg( uint8 addr, uint8 value)
{
    SPI_write_single( addr, value);
}

uint8 RF_read_status( uint8 addr )
{
    return SPI_read_status( addr );
}

void RF_reset()
{
  RF_strobe( SRES );
}

uint8 RF_RSSI()
{
  return RF_read_status( RSSI );
}

void RF_send_packet(uint8 *txBuffer, uint8 length)
{
    SPI_strobe(SFTX); // Flush TX fifo
    SPI_write_single(TXFIFO, length); // First write packet length
    SPI_write_burst(TXFIFO, txBuffer, length); // Fill the TX fifo with our packet
    __delay_cycles(100); // Wait a bit
    SPI_strobe(STX); // Transmit
    
    uint8 txbytes = RF_read_status( TXBYTES );
    while (txbytes > 0 ) {
      uint8 marc_state = SPI_read_status(MARCSTATE);
      if (marc_state & STATE_TX) {
        __delay_cycles(100);
      }
      txbytes = RF_read_status( TXBYTES );
    }
          
    // TODO: Should we check that the packet was sent? 
    // It should be configured to go back to IDLE by itself
}

uint8 RF_receive_packet(uint8 *rxBuffer)
{
    long timeout = 100;
    
    SPI_strobe(SRX);
    
    // Check if bytes are received. Should maybe check for overflow also.
    while (SPI_read_status(RXBYTES) == 0 && timeout-- > 0) {
      uint8 marc_state = SPI_read_status(MARCSTATE);
      uint8 rssi = RF_RSSI();
      __delay_cycles(100);
    }
    
    if (SPI_read_status(RXBYTES) == 0) {
      // Didn't get a packet
      return 0;
    }
    
    uint8 packet_length = RF_read_reg(RXFIFO); // first byte is packet length
    SPI_read_burst(RXFIFO, rxBuffer, packet_length); // read packet into rxbuffer
    uint8 status1 = SPI_read_single(RXFIFO); // RSSI
    uint8 status2 = SPI_read_single(RXFIFO); // CRC and Signal quality
    
    SPI_strobe(SFRX); // only allow one packet at the time... flush the RX FIFO
    
    // If CRC ok, return packet length. If not, return 0
    if (status2 & BIT7) { 
      return packet_length;
    }
    else {
      return 0;
    }

}
