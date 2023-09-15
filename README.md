# low_level_sudoku
System implemented developed along ![@llauragonzalezz](https://github.com/llauragonzalezz) during the 2021/2022 Proyecto Hardware course at Universidad de Zaragoza, consisting of: 
- A low level (ARM Assembly/C) sudoku candidates generator
- A fully functional C sudoku game implemented for the LPC2105 ARM7TDMI-S CPU, with:
  - A FIFO event-based job queue
  - Interaction via GPIO buttons
  - Visualization via GPIO LEDs (cell values and candidates) and UART (used as a console to fully visualize the board)
  - IRQ handling
  - RTC usage with different timers (tracking played time, polling for events...)
  - Idle and power-down support
  - Sudoku game stuff (start/restart/stop a game, write values with error checking, show candidates...)

# Usage
The project was developed using Keil uVision, emulating a LPC2105 CPU. Project files are not included, but all source code is provided (it should be easy to set up)

# Notes
All documentation is written in Spanish
