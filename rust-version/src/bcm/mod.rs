//--------------------------------------------------------------------------------------------------
// Public Definitions
//--------------------------------------------------------------------------------------------------

pub mod uart;
pub mod common;
pub mod gpio;

use crate::bcm::{ common::map, uart::Uart, gpio::GPIO };

//--------------------------------------------------------------------------------------------------
// Global instances
// -------------------------------------------------------------------------------------------------

pub static UART: Uart = unsafe { Uart::new(map::mmio::UART_START) };
pub static GPIO: GPIO = unsafe { GPIO::new(map::mmio::GPIO_START) };

//--------------------------------------------------------------------------------------------------
// Public Code
// -------------------------------------------------------------------------------------------------

pub fn hardware_init() {
    UART.init();
    GPIO.init();
}
