# AMTCS – Automating Measurements Using a Temperature Controller and Spectrometer

## Overview
AMTCS is a laboratory automation system designed to coordinate temperature control and spectrometer-based measurements within a single workflow. The system integrates a temperature controller and a spectrometer setup (monochromator and CCD detector) using an embedded controller (Raspberry Pi) and a C++-based application (user interface and hardware gateway).

It enables automated execution of temperature-resolved experiments, including temperature ramping, stabilisation, and spectrum acquisition, improving reliability, repeatability, and efficiency.

## Features
- Automated temperature control and stabilisation  
- Spectrometer integration via vendor SDK  
- PLC-style orchestration logic (Raspberry Pi)  
- TCP and serial communication between components  
- Automated data acquisition and logging  
- Basic fault tolerance and recovery mechanisms  

## Technologies
C++, Python, TCP/IP, Serial Communication, Raspberry Pi, Visual Studio (MFC), JSON  

## Disclaimer
This project was developed for a specific laboratory setup and is tightly coupled to the hardware used at Tyndall National Institute, including a particular temperature controller (Cryo-Con 32 from Cryogenic Control Systems Inc.) and spectrometer system (iHR-320 and Synapse CCD from HORIBA Scientific).

The spectrometer integration is based on the **HORIBA SynerJY SDK**, which requires:
- installation of the vendor-provided SDK  
- a compatible Windows environment  
- a USB hardware key (dongle) with the appropriate permissions to enable SDK functionality  

Without this hardware key and SDK access, spectrometer control will not function.

Therefore, the system is **not intended to be plug-and-play** or directly reusable in other environments without modification. Adapting it to different hardware configurations would require significant changes to communication layers, device control logic, and system architecture.

## Status
The system has been implemented and validated under real laboratory conditions.

## Future Work
Planned improvements include:
- Server-based logging and notification system  
- Mobile or web-based interface for remote monitoring  
- Further robustness and usability enhancements  
