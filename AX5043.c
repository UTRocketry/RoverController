#include "ax5043.h"
/* AX5043 library.
* Should provide all initialization and control
* functionality for the AX5043 radio IC. */


int ax5043_power_up() {
	/* Need to borrow the SPI pins
	* and select the 5043 until the MISO -> 1*/
	int count;
	int ret;
	count = 0;
	MISO_PT.pcr[MISO] = MISO_GPIO_SEL;
	MISO_PORT->PDDR |= (1 << MISO);
	ax5043_sel();
	while (((MISO_PORT->PDIR && (1 << MISO)) == 0) & (count++ < PWR_UP_TIMEOUT));
	if ((MISO_PORT->PDIR && (1 << MISO)) == 0) {
		/* Then we cant get it to power up. */
		ret = 1;
	}
	else {
		ret = 0;
	}
	ax5043_desel();
	MISO_PT.pcr[MISO] = MISO_SPI_SEL;
}
int spi_send(char[] tx_data, char[] rx_data, int num_bytes);
/* The chunk of data has a header.
* In this header, there was a byte (first byte)
* And in that byte, there was a length (upper 3 bits)
* And in that len, there are invalids (100, 101, and 110)*/


int ax_full_tx() {
	ax5043_sel();
	/* Need to put into full tx
	* Need to wait untill SVMODEM is high in the POWSTAT*/
}

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
void ax_wr88reg(struct ax_driver_s, uint8_t reg, uint8_t data); /* Set a 8 bit register with 8 bits ofdata*/
uint8_t ax_rd8reg(struct ax_driver_s, uint8_t reg); /* Dont know if this would be better with a pointer, allowing the function to return failure.....*/
void ax_wr168reg(struct ax_driver_s, uint16_t reg, uint16_t data);
int ax_check_comms(struct ax_driver_s) {
	uint8_t stat;
	stat = ax_rd8reg(ax_driver_s, AX_REG_SILICONREVISION);
	if (stat == AX_REG_SILICONREVISION_DEFAULT) {
		return AX_SUCCESS;
	}
	else if (stat == 0x00 || stat == 0xFF) {
		return AX_COMM_FAIL;
	}
	else {
		return AX_GENERAL_FAIL;
	}
}
int ax_bootup(struct ax_driver_s) {
	/* So bootstrap procedure.
	* Need to reset the chip. Bring SEL high.
	* Wait 1us, wait until MISO goes high.
	* Set, clear the RST bit of the PWRMODE register.
	* Throw the chip into POWERDOWN. PWRMODE register again.
	* Need to turn on.
	* Need to check comms.
	* Bring up the oscilator.
	* PRogram all the register shtuffs
	* */
	/* Delay 1us. */
	uint8_t status;
	int count;
	ax_sel(ax_driver_s);  /* Need to set the SEL and wait for 1us. MISO should go high when ready.  */
						  /* Need to delay a us.  */
	ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_RST_MASK);  /* Puts the chip into RST mode. */
																	   /* Delay a bit here.  */
	ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, 0);  /* UGH. Magic numbers. This basically sets the chip into powerdown, and clears the RST bit. */
												 /* Delay a bit here.  */

												 /* Should be able to set all the register contents here.
												 * In power down, the register file is still up. */

	status = ax_check_comms(ax_driver_s);
	if (status != AX_SUCCESS) {
		return status;
		/* IDK. Maybe do this more than once? IDK. Im new at this. */
	}
	/* In that case, it seems pretty legit.
	* Need to set up the osc. */
	ax_wr168reg(ax_driver_s, AX_REG_XTALOSC, 0x04);  /* Magic numbers. Came from DS.  */
	ax_wr168reg(ax_driver_s, AX_REG_XTALAMPL, 0x00);  /* Once again, from DS. */
	ax_wr88reg(ax_driver_s, AX_REG_XTALCAP, 0);  /* Set minimal XTAL load, 3pf */

												 /* UH.... PLL HALP */
												 /* Need to autorange the VCO.  */
												 /* Methinks that here, we should check the voltages.....*/



	ax_wr88reg(ax_driver_s, AX_REG_PWRMODE, AX_REG_PWRMODE_XTALEN_MASK); /* Enable the xtal osc. We need to wait until the xtal osc is up and running. */

																		 /* Maybe we should do this in a struct........
																		 * There is a craptonne of stuf to change.*/
																		 /* Struct lets goooooooooo */
