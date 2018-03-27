@echo off
call freertos-config.cmd
..\..\tool\bin\glamomem -t jedi.dat -n spi -i -q
..\..\tool\bin\glamomem -t jedi.dat -n spi -l freertos\project\%PROJECT_TYPE%\jedi.bin
..\..\tool\bin\glamomem -t jedi.dat -n spi -R 0x1001 -a 0x168C
..\..\tool\bin\glamomem -t jedi.dat -n spi -m -a 0x0 -s 512