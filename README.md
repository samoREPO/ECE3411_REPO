# ECE3411_REPO
microprocessor application laboratory

This repository contains lab assignments and experiments for ECE 3411
(Microprocessor Applications Lab) at UConn.

## Hardware
- AVR128DB48 Curiosity Nano
- Onboard LED, pushbuttons

## Toolchain
- Microchip Studio
- AVR-GCC
- GitHub Desktop for version control

## Creating new project
1. Open Microchip Studio
2. Goto File->New->Project
3.a. Select GCC C Executable Project
3.b. Set Solution: to Add to Solution
4. Create src and include folders within the new project
5. Update within connected repository
## Repository Structure
// in PowerShell type "tree /F" to print directory

## Notes
- Each lab is self-contained in its own directory
- Build artifacts are excluded via `.gitignore`
- Code is written in C targeting bare-metal AVR
