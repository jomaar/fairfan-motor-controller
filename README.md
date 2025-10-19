# FairFan Motor Controller

A sophisticated dual-motor control system for the FairFan project, built on the Controllino MAXI Automation platform (ATmega2560).

## Features

- **Dual Motor Control**: Independent control of two stepper motors with different gear ratios
- **Smooth Speed Profiling**: S-curve acceleration/deceleration for gentle starts and stops
- **Homing System**: Automatic limit switch detection and position calibration for Motor 2
- **Synchronized Sequences**: Coordinated oscillation patterns with configurable direction modes
- **Serial Command Interface**: Interactive control via serial monitor
- **Safety Features**: Emergency stop, soft stop with re-homing requirement, position validation

## Hardware

- **Platform**: Controllino MAXI Automation (ATmega2560, 16MHz, 8KB RAM, 248KB Flash)
- **Motor 1**: 200 steps/rev × 8 microsteps × 20:1 gear ratio, 15 RPM
- **Motor 2**: 200 steps/rev × 8 microsteps × 50:1 gear ratio, 5 RPM (inverted wiring)
- **Limit Switches**: NC (normally closed) switches on DI0/DI1 for Motor 2 homing

## Motor Configuration

### Motor 1 (Main Motor)
- Step Pin: 6
- Direction Pin: 4
- Acceleration: 18° (5% of 360°)
- Deceleration: 18° (5% of 360°)
- Power Curve: 0.8 (gentle)
- Min Speed: 10%

### Motor 2 (Oscillation Motor)
- Step Pin: 7
- Direction Pin: 8
- Enable Pin: 9
- Acceleration: 36° (10% of 360°)
- Deceleration: 36° (10% of 360°)
- Power Curve: 0.8 (gentle)
- Min Speed: 10%
- **Note**: Inverted wiring (HIGH = CCW/LEFT)

## Commands

### Motor 1
- `go1` - Start Motor 1 (uses custom degrees or 180° default)
- `stop1` - Stop Motor 1
- `deg<n>` - Set custom degrees (e.g., deg360, deg720, deg90)
- `deg` - Show current degree setting

### Motor 2
- `home` - Home Motor 2 (find limit switches)
- `stop2` - Stop Motor 2

### Sequence Control
- `seq1` - Start oscillation sequence
- `stopseq` - Stop sequence immediately
- `softstop` - Stop after current movement (requires re-homing)

### Emergency
- `stopall` - STOP ALL motors and sequence

### Configuration
- `sync` / `same` - Motor1 follows Motor2 (same direction)
- `opposite` / `alt` - Motor1 opposite to Motor2
- `mode` / `status` - Show current direction mode

### Other
- `help` - Show command list

## Project Structure

```
fairfanpio01/
├── include/
│   ├── CommandHandler.h       # Serial command interface
│   ├── Config.h                # Centralized configuration
│   ├── MainMotor.h             # Motor 1 control
│   ├── OscillationMotor.h      # Motor 2 with homing
│   ├── SequenceStateMachine.h  # Coordinated sequences
│   └── StepperMotor.h          # Base stepper motor class
├── src/
│   └── fairfanpio.cpp          # Main program
├── platformio.ini              # PlatformIO configuration
└── README.md                   # This file
```

## Building and Uploading

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- Controllino MAXI Automation board

### Build
```bash
platformio run
```

### Upload
```bash
platformio run --target upload
```

### Monitor Serial
```bash
platformio device monitor --baud 115200
```

## Configuration

All hardware parameters and behavior settings are centralized in `include/Config.h`:

- Motor pins and timing parameters
- Speed profiles (acceleration, deceleration, power curves)
- Limit switch pins
- Serial baud rate
- Sequence behavior (auto-start, direction mode)

## Development Notes

### Speed Profiling
Acceleration and deceleration zones are calculated **relative to 360°** (one full rotation) rather than total movement distance. This ensures consistent acceleration feel regardless of whether you move 90° or 720°.

### Motor 2 Inverted Wiring
Motor 2 has inverted wiring where HIGH signal = CCW/LEFT direction. All direction commands in the code are marked with "Inverted" comments.

### Safety Features
- Soft stop invalidates homing status, requiring `home` command before restart
- Emergency stop immediately disables all motors
- Position validation prevents movement without proper homing

## Memory Usage

- **Flash**: ~17,854 bytes (7.0% of 253,952 bytes)
- **RAM**: ~549 bytes (6.7% of 8,192 bytes)

## License

[Add your license here]

## Author

[Add your name here]
