Pairing and Bonding with Peripheral Device Sample Code
======================================================

NOTE: If you use this sample application for your own purposes, follow
      the licensing agreement specified in `Software_Use_Agreement.rtf`
      in the home directory of the installed RSL10 Software
      Development Kit (SDK).

Overview
--------
This sample project generates a battery service and a custom service. It then
begins undirected connectable advertising. It supports up to four simultaneous
connections. Any central device can scan, connect, pair/bond, perform service 
discovery and read characteristics' values from both battery and custom 
service. The application sends periodic notifications of the battery level and
a custom service characteristic to the connected clients. The application 
stores the bond information in the RSL10 NVR2 flash and has the ability to 
perform Private Address Resolution. Additionally, it demonstrates how to use 
the BATMON alarm hardware of RSL10 to trigger an interrupt when the battery 
level falls below a configured threshold.

This sample project is equipped with a Bluetooth Low Energy abstraction layer
that provides a higher level application programming interface (API) to
abstract the Bluetooth GAP and GATT layers. The abstraction layer has been
designed to improve flexibility and simplicity by providing the following
features:

    - An event subscription mechanism that allows the application to subscribe 
      and receive callback notifications for any Kernel or Bluetooth event. 
      This improves encapsulation/integration of different modules, as they 
      can be implemented while isolated in their own files.
    - Generic definition of custom services with callback notification support
    - Security handling and bond list implementation in RSL10 flash
    - Code structure and API naming aligned with RivieraWaves documentation, 
      so you can map the code into the documentation more easily

The sample code has been refactored to keep the generic abstraction layer code 
and application-specific code in separate files. This increases flexibility, 
maintainability and reusability of components between distinct applications.

After the connection is established, for any profiles/services that need to
be enabled, an enable request message is sent to the corresponding profile of
the Bluetooth stack. Once a response is received for each of those profiles,
the application starts advertising.

**Custom Service:** any central device can read and write custom attributes. 
                The custom service on the peripheral includes two long 
                characteristic attributes. The value written on the attribute 
                with the `RX_LONG_VALUE` characteristic name is inverted and 
                written back to the `TX_VALUE_LONG` by the custom service. In 
                addition, the custom service sends a notification with an 
                incremental value every 6 seconds. If the user want it is possible to
                send indications for the `TX_VALUE_LONG` characteriscic. This
                is selected by the `TX_VALUE_LONG_INDICATION` define at 
                `app_customss.h`.

**Battery Service:** this service database is configured for a single battery 
                 instance. A second battery can be added by modifying the 
                 `APP_BAS_NB` definition in `app_bass.h`. The application provides
                 a callback function to read the battery level using the 
                 average of 16 reads of RSL10 ADC. 

The application notifies clients about the battery level on two occasions:

**Periodically:** the notification timeout is configured using the 
              `BASS_NotifyOnTimeout()` function.

**On battery level change:** The application monitors the battery level periodically and sends
                         a notification when a change is detected. The 
                         monitoring timeout is configured using the 
                         `BASS_NotifyOnBattLevelChange()` function.


The message subscription mechanism allows the application and services to 
subscribe and receive callback notifications based on the Kernel message ID 
or task ID. This allows each module of the application to be independently 
implemented in its own files. The subscription is performed using the 
`MsgHandler_Add()` function. The application subscribes to receive GAPM and 
GAPC events (see `app.c`). The services subscribe to receive Kernel events in 
their initialization function (see `BASS_Initialize()` in `ble_bass.c`, for an 
example). The application event handlers are implemented in `app_msg_handler.c`.

The basic sequence of operations and event messages exchanged between the 
application and the Bluetooth stack is presented below: 

  APP <--> Bluetooth Low Energy Stack
  
  Startup
  
      --->  GAPM_ResetCmd() - app.c
      <---  GAPM_CMP_EVT / GAPM_RESET
      --->  GAPM_SetDevConfigCmd() - app_msg_handler.c
      <---  GAPM_CMP_EVT / GAPM_SET_DEV_CONFIG
      --->  GATTM_AddAttributeDatabase() - app_msg_handler.c
      --->  GAPM_ProfileTaskAddCmd() - ble_bass.c
      <---  GATTM_ADD_SVC_RSP
      <---  GAPM_PROFILE_ADDED_IND
      --->  GAPM_StartAdvertiseCmd() - app_msg_handler.c
            
  Connection request / parameters update request / device info request
  
      <--- GAPC_CONNECTION_REQ_IND
      ---> GAPM_ResolvAddrCmd() - app_msg_handler.c
      ---> GAPC_ConnectionCfm() - app_msg_handler.c
      ---> GAPM_StartAdvertiseCmd() - app_msg_handler.c
      <--- GAPC_PARAM_UPDATE_REQ_IND
      ---> GAPC_ParamUpdateCfm() - app_msg_handler.c
      <--- GAPC_GET_DEV_INFO_REQ_IND
      ---> GAPC_GetDevInfoCfm() - app_msg_handler.c

  Pairing / Bonding request
  
      <--- GAPC_BOND_REQ_IND / GAPC_PAIRING_REQ
      GAPC_BondCfm() - app_msg_handler.c
      <--- GAPC_BOND_REQ_IND / GAPC_LTK_EXCH
      GAPC_BondCfm() - app_msg_handler.c
      <--- GAPC_BOND_REQ_IND / GAPC_IRK_EXCH
      GAPC_BondCfm() - app_msg_handler.c
      <--- GAPC_BOND_REQ_IND / GAPC_CSRK_EXCH
      GAPC_BondCfm() - app_msg_handler.c

  Encrypt request
  
      <--- GAPC_ENCRYPT_REQ_IND
      ---> GAPC_EncryptCfm() - app_msg_handler.c

  Disconnection
  
      <--- GAPC_DISCONNECT_IND
      ---> GAPM_StartAdvertiseCmd() - app_msg_handler.c

This sample project is structured as follows:

The source code exists in a `source` folder. Application-related header files 
are in the `include` folder. The `main()` function is implemented in the `app.c`
file, which is located in the parent directory.

The Bluetooth Low Energy abstraction layer contains support functions for the 
GAP and GATT layers and has a message handling mechanism that allows the 
application to subscribe to Kernel events. It also contains standard profile-
related files.

The device address type and source are set in app.h. If `APP_DEVICE_PARAM_SRC`
is set to `FLASH_PROVIDED_or_DFLT` and the address type is set to 
`GAPM_CFG_ADDR_PUBLIC`, the stack loads the public device address stored in NVR3
flash. Otherwise, the address provided in the application is used.

Erasing NVR2 leads to an empty bondlist. If the desired behavior of this
application or test configuration is to start from a new bondlist at startup, 
the following steps can be undertaken:

1.  Connect DIO12 to ground.
2.  Press the RESET button (this restarts the application, which pauses at the
    start of its initialization routine).

Note that this erases the flash, but only if there is anything in the NVR2 drive.

Application-specific files
--------------------------
`app.c`                  - Main application file 

source
------
    app_config.c      - Device configuration and definition of application-
                        specific Bluetooth Low Energy messages.
    app_msg_handler.c - Application-specific message handlers
    app_bass.c        - Application-specific battery service functions and 
                        message handlers (ADC read, BATMON alarm, etc.)
    app_customss.c    - Application-specific custom service server support 
                        functions and handlers
    app_trace.c       - Debugging (printf) utility functions

include
-------
    app.h             - Main application header file
    app_bass.h        - Application-specific Battery Service Server header
    app_customss.h    - Application-specific custom service server header
    
Bluetooth Low Energy abstraction files (CMSIS-Pack component is generic for any application)
---------------------

    ble_gap.c         - GAP layer support functions and message handlers          
    ble_gatt.c        - GATT layer support functions and message handlers               
    msg_handler.c     - Message handling subscribing mechanism implementation


    ble_gap.h         - GAP layer abstraction header
    ble_gatt.h        - GATT layer abstraction header
    msg_handler.h     - Message handling subscribing mechanism header       
    

    ble_bass.c        - Battery Service Server support functions and handlers
                     

    ble_bass.h        - Battery Service Server header

       
Hardware Requirements
---------------------
This application can be executed on any RSL10 Evaluation and Development Board 
with no external connections required.

Importing a Project
-------------------
To import the sample code into your IDE workspace, refer to the 
Getting Started Guide for your IDE for more information.

Select the project configuration according to the required optimization level. 
Use **Debug** configuration for optimization level **None** or **Release**
configuration for optimization level **More** or **O2**.

Verification
------------
To verify if this application is functioning correctly, use RSL10 or another 
third-party central device application to establish a connection and execute 
pairing and bonding functionality. In addition to establishing a connection, 
this application can be used to read/write characteristics and receive 
notifications.

To show how an application can send notifications, the battery service is 
configured to send the battery level every 6 seconds or less (if a battery level 
change is detected in VBAT). The custom service also sends an incrementing 
value of the first attribute every 6 seconds.

Alternatively, you can observe the behavior of the LED on the RSL10 Evaluation 
and Development Board (DIO6). The LED behavior is controlled by the 
`APP_LED_Timeout_Handler` function (in `app_msg_handler.c`) and can be one of the 
following:

    - If the device has not started advertising, the LED is off.
    - If the device is advertising but it has not connected to any peer, the 
      LED blinks every 200 ms.
    - If the device is connected to fewer than APP_NB_PEERS peers, the LED 
      blinks every 2 seconds according to the number of connected peers (i.e., 
      once if one peer is connected, twice if two peers are connected, etc.).
    - If the device is connected to APP_NB_PEERS peers the LED is steady on 
      and the application is no longer advertising.

Notes
-----
Sometimes the firmware in RSL10 cannot be successfully re-flashed, due to the
application going into Sleep Mode or resetting continuously (either by design 
or due to programming error). To circumvent this scenario, a software recovery
mode using DIO12 can be implemented with the following steps:

1.  Connect DIO12 to ground.
2.  Press the RESET button (this restarts the application, which pauses at the
    start of its initialization routine).
3.  Re-flash RSL10. After successful re-flashing, disconnect DIO12 from
    ground, and press the RESET button so that the application can work
    properly.
    
Note that the use of this method also resets the NVR2 memory.

***
Copyright (c) 2019 Semiconductor Components Industries, LLC
(d/b/a ON Semiconductor).
