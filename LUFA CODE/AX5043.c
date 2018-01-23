#include "AX5043.h"
/* AX5043 library.
* Should provide all initialization and control
* functionality for the AX5043 radio IC. */

int ax5043_power_up() {
	/* Need to borrow the SPI pins
	* and select the 5043 until the MISO -> 1*/
	//PORTB |= (1<<DDB0); //SS high
	//PORTB &= ~(1<<DDB0); //SS low
	


}
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
int ax_send_data(char* tx_data, int len) {
	/* Set pwrmode to fulltx
	* enable tcxo
	* write pramble
	* write packet
	* wait for osc
	* commit fifo
	* wat for xmit to complete
	* set power mode*/
	
}
int ax_read_packet(char* rx_data, int max) {
	/* Assumes a packet is ready and waiting
	* reads the header
	* reads the data until max len or data all read
	* empty the fifo if max is reached. */
}

int ax_check_comms() {
	uint8_t stat;
	stat = SPI_RW_8(AX_REG_SILICONREVISION,0,1);
	if (stat == AX_REG_SILICONREVISION_DEFAULT) {
		return 1;
	}
	else if (stat == 0x00 || stat == 0xFF) {
		return 0;
	}
	else {
		return 1;
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
	uint8_t status;
	int count;
	//ax_sel(ax_driver_s);  /* Need to set the SEL and wait for 1us. MISO should go high when ready.  */
						  /* Need to delay a us.  */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_RST_MASK);  /* Puts the chip into RST mode. */
	PORTB |= (1<<DDB0); //SS high
	_delay_ms(1);
	while((PINB & (1 << PB4))) _delay_ms(2);
	SPI_RW_8(AX_REG_PWRMODE,AX_REG_PWRMODE_REST_MASK,0);
	_delay_ms(3);																   /* Delay a bit here.  */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, 0);  /* UGH. Magic numbers. This basically sets the chip into powerdown, and clears the RST bit. */
	SPI_RW_8(AX_REG_PWRMODE,AX_REG_PWRMODE_POWERDOWN_MASK,0);											 /* Delay a bit here.  */
												 /* Should be able to set all the register contents here.
												 * In power down, the register file is still up. */
	/* In that case, it seems pretty legit.
	* Need to set up the osc. */
	SPI_RW_A16_R8(AX_REG_XTALOSC,0x04,0);
	SPI_RW_A16_R8(AX_REG_XTALAMPL,0x00,0);
	SPI_RW_A16_R8(AX_REG_XTALCAP,0,0);
	SPI_RW_A16_R8(AX_REG_PWRMODE,0x0C,0);

	//ax_wr168reg(ax_driver_s, AX_REG_XTALOSC, 0x04);  /* Magic numbers. Came from DS.  */
	//ax_wr168reg(ax_driver_s, AX_REG_XTALAMPL, 0x00);  /* Once again, from DS. */
	//ax_wr88reg(ax_driver_s, AX_REG_XTALCAP, 0);  /* Set minimal XTAL load, 3pf */
	//ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_XTALEN_MASK); /* Enable the xtal osc. We need to wait until the xtal osc is up and running. */

																		 /* Maybe we should do this in a struct........
																		 * There is a craptonne of stuf to change.*/
																		 /* Struct lets goooooooooo */
}
