# EtchASketch MPLAB VS Code Workflow

## Recommended Daily Flow

Use the VS Code tasks from `Terminal > Run Task`:

- `EtchASketch: Build` compiles the firmware with XC8.
- `EtchASketch: Upload` builds, then programs the Curiosity Nano through MPLAB MDB using `pkobnano` and UPDI.
- `EtchASketch: Connect Probe` checks that MDB can reserve the board and find the AVR128DB48 target.
- `EtchASketch: List Tools` prints the MPLAB hardware tools currently visible to the Mac.
- `EtchASketch: Clean` removes local build output.
- `EtchASketch: Dist Clean` removes build output plus old IDE-generated folders.

The upload path intentionally avoids copying the HEX to `/Volumes/CURIOSITY`; that path was hanging on macOS. MDB detected this board as:

```text
Tool Type: pkobnano
Description: nEDBG CMSIS-DAP
Serial: MCHP3372043000002412
```

## Importing Into The MPLAB Extensions Project System

The MPLAB VS Code importer is installed on this Mac. To convert the existing Microchip Studio project:

1. Open this folder in VS Code: `lab/EtchASketch`.
2. Run `MPLAB: Scan workspace for Microchip Studio project(s)` from the Command Palette.
3. Import `EtchASketch/EtchASketch.cproj` when prompted.
4. Use `MPLAB: Edit Project Properties (JSON)` or the MPLAB UI to confirm:
   - Device: `AVR128DB48`
   - Tool: `pkobnano`
   - Interface: `UPDI`
   - Toolchain: XC8 `v3.10`
5. Build with `MPLAB CMake: Build`.

The plain VS Code tasks remain available even after import, so there is always a simple fallback build/upload path.
