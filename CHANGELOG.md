
# Change Log



## [2.0.0] - 2020-10-04

### Added

- Added support for MTU larger than 20 bytes for adapting applications running BLE4.2 and above.
- Added a new dp data receiving and sending function, which can send and receive dp data with a length of more than 255 bytes, and supports the sending mode without response.
- Added device unbinding function, which is different from the factory reset function and will not lose cloud historical data after binding again.
- Added asynchronous response of unbinding and factory reset functions.
- Added support for BEACON KEY.

### Changed

- Changed the queue scheduling mechanism under non-RTOS systems to prevent SDK from occupying CPU for a long time.
- Modify `TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE` to be automatically configured by the SDK.

### Fixed

- Fixed a bug that the device will automatic restart when continuously calling the `tuya_ble_device_factory_reset()`function.



## [1.3.0] - 2020-08-07

### Changed

- Use the new Tuya exclusive service uuid and characteristic uuid.
- Changed advertising packets format  to adapt to the new service uuid.

### Fixed

- Changed the time point at which the application callback is executed during unbinding to prevent the application from performing a reset operation in the corresponding callback function to cause unbinding failure.



## [1.2.6] - 2020-07-13

### Added

- Added support for transmitting character dp point data with length 0.

### Changed

- Changed  the definition of the critical section interface function of the port layer.

### Fixed

- Fixed the bug that the single packet data instruction cannot be parsed (Although there is no such instruction currently).



## [1.2.5] - 2020-06-03

### Changed

- Changed the dynamic memory release logic in some event handlers, the previous method may cause the dynamic memory free space to become smaller in extreme scenarios.

### Fixed

- Fix the bug of test link failure based on Tuya protocol.



## [1.2.4] - 2020-05-22

### Changed

- Changed flash interface asynchronous.
- Changed sdk information storage function based on asynchronous flash interface.
- Modify the SDK initialization function to be an asynchronous interface.



## [1.2.3] - 2020-05-07

### Added
- Added flash interface asynchronous.
- Added sdk information storage function based on asynchronous flash interface(Currently only supports nrf52832).


## [1.2.2] - 2020-04-23
### Added
- Added on-demand connection interface.
- Added the interface to get the total size, space and number of events of the scheduling queue under no OS environment.
- Added configuration item to configure whether to use the MAC address of tuya authorization information.
- Added tuya app log interface.

### Changed
- Changed the production test code structure, and remove the application layer header file to obtain the application version number, fingerprint and other information methods.
- Changed the encryption method of ble channel used in production test to support unencrypted transmission.

### Fixed
- Fix a problem that caused a compilation error in the SEGGER Embedded Studio environment.

## [1.2.1] - 2020-03-20
### Changed
- Optimized production test function, added data channel parameters


## [1.2.0] - 2020-03-06
### Added
- New platform memory management port.
- Added gatt send queue.
- New production test interface for customer-defined applications.
- Added tuya app log interface.

### Changed
- The maximum length of dp point array data that can be sent at one time increases to 258.
- Optimize RAM usage.
- Stripping uart general processing functions to customer applications.

