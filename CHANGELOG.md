
# Change Log



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

