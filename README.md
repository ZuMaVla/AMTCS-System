# AMTCS – Automating Measurements Using a Temperature Controller and Spectrometer

## Overview

AMTCS is a laboratory automation system designed to coordinate temperature control and spectroscopic measurements within a single workflow.

The system integrates a temperature controller and a spectrometer setup (monochromator and CCD detector), using a Raspberry Pi for experiment orchestration and a C++ Windows application for user interaction and hardware control.

AMTCS automates temperature-resolved experiments, including temperature ramping, stabilisation, and spectrum acquisition. It is designed to improve the reliability, repeatability, and efficiency of laboratory measurements.

## System Architecture

The system consists of the following main components:

- **C++ Windows application (UI app)** — provides the graphical user interface for hardware connectivity, spectrometer communication, experiment configuration, and local control and monitoring.
- **Raspberry Pi controller (PLC)** — coordinates the experiment workflow and communicates with the temperature controller and Windows UI app.
- **Remote Control (API server and browser-based client)** — provides remote experiment monitoring and supports basic experiment control operations.
- **Laboratory equipment** — temperature controller, monochromator, and CCD detector.

Communication between components uses the following interfaces:

- **Spectrometer ↔ UI app:** communication through the vendor-provided SDK.
- **PLC ↔ UI app:** TCP/IP.
- **PLC ↔ temperature controller:** serial communication.
- **PLC ↔ Remote Control server ↔ browser-based client:** HTTP.

## Features

- **Measurement configuration** — configure experiment parameters, including wavelength range, slit settings, and acquisition options.
- **LASER Exclusion** — exclude specified wavelength regions when calculating the maximum signal.
- **Cosmic ray removal and automatic exposure adjustment** — help improve the quality of acquired spectra and make effective use of the detector's dynamic range.
- **Automated temperature control** — ramp temperature and stabilise at defined setpoints.
- **Spectrometer integration** — control the spectrometer and acquire measurement data through the vendor-provided SDK.
- **Experiment management** — configure and execute measurement sequences across multiple temperature setpoints.
- **Temperature setpoint handling** — sort setpoints, remove duplicates, and add extra temperatures to an experiment on request.
- **Remote Control** — monitor experiment status and perform supported remote actions, including pausing, continuing, and cancelling an experiment.
- **Distributed control architecture** — coordinate the experiment using the Raspberry Pi controller and TCP/IP and serial communication.
- **Data acquisition and logging** — collect measurement data and maintain operational logs.
- **Fault detection and recovery** — detect failures in the UI application or PLC and support experiment recovery by the surviving component.

## Technologies

- **Programming languages:** C++, Python, JavaScript, C#
- **Web technologies:** HTML, CSS
- **Communication:** TCP/IP, serial communication, HTTP
- **Platforms:** Windows PC, Raspberry Pi 4/5
- **Desktop application:** C++ with Microsoft Foundation Classes (MFC)
- **Data exchange and configuration:** ASCII, JSON
- **Hardware integration:** HORIBA SynerJY SDK

## Hardware and Software Requirements

AMTCS was developed for a specific laboratory environment. The original setup includes:

- Cryo-Con 32 temperature controller from Cryogenic Control Systems Inc.
- HORIBA *i*HR-320 monochromator
- HORIBA Synapse CCD detector
- Raspberry Pi 5 controller (also compatible with Raspberry Pi 4)
- A compatible Windows environment for the desktop application and spectrometer SDK. The UI application was developed for Windows 7.

Spectrometer integration requires the vendor-provided HORIBA SynerJY SDK, the appropriate USB hardware key (dongle), and the necessary permissions. Without the hardware key and the necessary permissions, the system can operate only in simulation mode. The HORIBA SynerJY SDK must still be installed.

The system has been used with the equipment and configuration described above. Compatibility with other hardware or software configurations is not expected.

## Documentation

The [**AMTCS System User Guide**](./Docs/USER_GUIDE.pdf) describes system setup, experiment configuration, execution, and remote operation.

See the documentation files in this repository for the latest available version.

## Project Status

The system has been implemented and validated under real laboratory conditions with the intended equipment.

The repository includes the desktop application, Raspberry Pi control software, and supporting components. Hardware-specific dependencies, SDK access, and configuration may be required to build or run the system.

See [Releases](https://github.com/ZuMaVla/AMTCS-System/releases) for published versions, release notes, and available downloads.

## Future Work

No additional features are currently planned. General maintenance, support, and bug fixes will continue.