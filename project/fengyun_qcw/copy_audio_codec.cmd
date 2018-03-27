@echo off
set SRC_DIR= freertos\lib

rem set DEST_DIR= openrtos\lib
rem IF NOT EXIST %DEST_DIR% goto NOT_READY
rem IF NOT EXIST %SRC_DIR%\mp3.codecs goto NOT_FOUND
rem IF NOT EXIST %SRC_DIR%\wave.codecs goto NOT_FOUND
rem IF NOT EXIST %SRC_DIR%\aac.codecs goto NOT_FOUND
rem IF NOT EXIST %SRC_DIR%\amr.codecs goto NOT_FOUND
rem IF NOT EXIST %SRC_DIR%\wma.codecs goto NOT_FOUND
rem IF NOT EXIST %SRC_DIR%\flac.codecs goto NOT_FOUND
rem goto OK

rem :NOT_READY
rem echo Do not found the path "%DEST_DIR%"
rem mkdir openrtos
rem cd openrtos
rem mkdir lib
rem cd ../
rem goto OK

rem :NOT_FOUND
rem echo Can not found the plugins of audio CODEC
rem goto END

:OK
echo Copy MP2 Encoder CODEC ...
rem copy %SRC_DIR%\mp3.codecs %DEST_DIR%\ > NUL
..\..\tool\bin\dataconv -x %SRC_DIR%\mp2encode.codecs -o mp2encode.bin
move mp2encode.bin ..\..\sdk\driver\aud\
echo Copy AAC Encoder CODEC ...
..\..\tool\bin\dataconv -x %SRC_DIR%\aacencode.codecs -o aacencode.bin
move aacencode.bin ..\..\sdk\driver\aud\

:END
