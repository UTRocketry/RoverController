#include "AX5043.h"
/* AX5043 library.
* Should provide all initialization and control
* functionality for the AX5043 radio IC. */
#define fifo_commit 0b00000100
/* The chunk of data has a header.
* In this header, there was a byte (first byte)
* And in that byte, there was a length (upper 3 bits)
* And in that len, there are invalids (100, 101, and 110)*/

int ax_autorange() {
	/* This is needed if the frequency is changed more than 1MHz.
	* Each frequency must be ranged, FREQA and B.
	* Set power mode ta standby, enable TXCO
	* Wait until the osc is ready
	* Set RNGSTART of PLLRANGINGA/B
	* wait until RNGSTART = 0
	* If RNGERR == 1, error
	* esle set pwrmode to desired. */
}
int ax_send_data() {
	/* Set pwrmode to fulltx
	* enable tcxo
	* write pramble
	* write packet
	* wait for osc
	* commit fifo
	* wat for xmit to complete
	* set power mode*/
    //Needs 00010001 for preamble
    SPI_RW_8(AX_REG_PWRMODE,PWRMODE_FULLTX,0);
    _delay_ms(100);
    SPI_RW_8(AX_REG_FIFODATA,0b00010001,0);
    //SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    SPI_RW_8(AX_REG_FIFODATA,0b01100010,0);
    //SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    SPI_RW_8(AX_REG_FIFODATA,0b00000000,0);
    //SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    SPI_RW_8(AX_REG_FIFODATA,0b0000010,0); //Repeat twice 
    //SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    //THREE BYTES OF DATA
    SPI_RW_8(AX_REG_FIFODATA,0x01,0);
   // SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    SPI_RW_8(AX_REG_FIFODATA,0x02,0);
    //SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    SPI_RW_8(AX_REG_FIFODATA,0x03,0);
    _delay_ms(3000);
    //while(SPI_RW_8(AX_REG_XTALSTATUS,0x00,1)==0b00000000);
    SPI_RW_8(AX_REG_FIFOSTAT,fifo_commit,0);
    //while(SPI_RW_8(AX_REG_XTALSTATUS,0x00,1)==0b00000001);
    SPI_RW_8(AX_REG_PWRMODE,AX_REG_PWRMODE_POWERDOWN_MASK,0);
    //SPI_RW_8(AX_REG_PWRMODE,PWRMODE_SYNTHRX,0);
    //SPI_RW_8(AX_REG_PWRMODE,PWRMODE_FULLRX,0);
	
}
char * ax_read_packet() {
	/* Assumes a packet is ready and waiting
	* reads the header
	* reads the data until max len or data all read
	* empty the fifo if max is reached. */
    char pre;
    char len;
    char set;
    char data[]={0,0,0};
    SPI_RW_8(AX_REG_PWRMODE,PWRMODE_FULLRX,0);
    if(SPI_RW_8(AX_REG_FIFOSTAT,0x00,1) & (1<<8)){
        _delay_ms(100);
        pre = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        len = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        set = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        data[0] = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        data[1] = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        data[2] = SPI_RW_8(AX_REG_FIFODATA,0x00,1);
        return data;
    }else{
        return data;
    }
}

int ax_check_comms() {
	uint8_t stat;
	stat = SPI_RW_8(AX_REG_SILICONREVISION,0x00,0);
	if (stat == AX_REG_SILICONREVISION_DEFAULT) {
		return stat;
	}
	else if (stat == 0x00 || stat == 0xFF) {
		return stat;
	}
}
int ax_bootup() {
	/* So bootstrap procedure.
	* Need to reset the chip. Bring SEL high.
	* Wait 1us, wait until MISO goes high.
	* Set, clear the RST bit of the PWRMODE register.
	* Throw the chip into POWERDOWN. PWRMODE register again.
	* Need to turn on.
	* Need to check comms.
	* Bring up the oscilator.
	* PRogram all the register stuffs
	* */
	/* Delay 1us. */
	//uint8_t status;
	//int count;
	//ax_sel(ax_driver_s);  /* Need to set the SEL and wait for 1us. MISO should go high when ready.  */
						  /* Need to delay a us.  */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_RST_MASK);  /* Puts the chip into RST mode. */
	PORTB |= (1<<DDB5); //SS high
	while(!(PINB & (1 << PB5)));
	SPI_RW_8(AX_REG_PWRMODE,AX_REG_PWRMODE_REST_MASK,0);
	_delay_ms(100);																   /* Delay a bit here.  */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, 0);  /* UGH. Magic numbers. This basically sets the chip into powerdown, and clears the RST bit. */
	SPI_RW_8(AX_REG_PWRMODE,AX_REG_PWRMODE_POWERDOWN_MASK,0);
    _delay_ms(100);
    /* Delay a bit here.  */
												 /* Should be able to set all the register contents here.
												 * In power down, the register file is still up. */
	/* In that case, it seems pretty legit.
	* Need to set up the osc. */
	SPI_RW_A16_R8(AX_REG_XTALOSC,0x04,0);
    _delay_ms(1);
	SPI_RW_A16_R8(AX_REG_XTALAMPL,0x00,0);
    _delay_ms(1);
	SPI_RW_A16_R8(AX_REG_XTALCAP,0,0);
    _delay_ms(1);
	//SPI_RW_A16_R8(AX_REG_PWRMODE,PWRMODE_SYNTHTX,0);
    _delay_ms(1);
    SPI_RW_8(AX_REG_PWRMODE,PWRMODE_FIFOON,0);
    _delay_ms(1);
	//ax_wr168reg(ax_driver_s, AX_REG_XTALOSC, 0x04);  /* Magic numbers. Came from DS.  */
	//ax_wr168reg(ax_driver_s, AX_REG_XTALAMPL, 0x00);  /* Once again, from DS. */
	//ax_wr88reg(ax_driver_s, AX_REG_XTALCAP, 0);  /* Set minimal XTAL load, 3pf */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_XTALEN_MASK); /* Enable the xtal osc. We need to wait until the xtal osc is up and running. */

}
