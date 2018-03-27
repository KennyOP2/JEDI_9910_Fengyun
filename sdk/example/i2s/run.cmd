cd ..\..\..\tool
bin\glamomem -t target\jedi.dat -n spi -i -q
bin\glamomem -t target\jedi.dat -n spi -l ..\build\freertos\sdk\example\i2s\test_i2s.bin
bin\glamomem -t target\jedi.dat -n spi -R 0x1001 -a 0x168C
bin\glamomem -t target\jedi.dat -n spi -m -a 0x0 -s 512
REM pause