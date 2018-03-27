@echo off
call freertos-config.cmd
..\..\tool\bin\glamomem -t .\jedi.dat -n spi -i -q
..\..\tool\bin\glamomem -t .\jedi.dat -n spi -l freertos\project\%PROJECT_TYPE%\jedi.bin
..\..\tool\bin\glamomem -t .\jedi.dat -n spi -R 0x003A -a 0x44
..\..\tool\bin\jp2_usb2spi pci 8888 -d 0 -t 0
pause