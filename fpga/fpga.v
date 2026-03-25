module fpga(
    input  CLOCK_50, UART_RXD,
	 input  [0:0] KEY,

	 output UART_TXD, LCD_ON, LCD_BLON, LCD_EN, LCD_RW, LCD_RS, 
	 output [7:0] LCD_DATA,
	 output [17:0] LEDR
);
	assign LEDR[17:1] = 17'h00;
	
	system u0(
	  .clk_clk                             (CLOCK_50),
	  .uart_0_external_connection_rxd      (UART_RXD),
	  .uart_0_external_connection_txd      (UART_TXD),
	  .button_external_connection_export   (KEY[0]),
	  .lcd_on_external_connection_export   (LCD_ON),
	  .lcd_blon_external_connection_export (LCD_BLON),
	  .lcd_en_external_connection_export   (LCD_EN),
	  .lcd_rw_external_connection_export   (LCD_RW),
	  .lcd_rs_external_connection_export   (LCD_RS),
	  .lcd_d_external_connection_export    (LCD_DATA),
	  .buzz_external_connection_export     (LEDR[0])
	);
endmodule
