@echo off
@rem This batch file backs up the contents (including subdirectories) to a date stamped rar file


@rem delete obj and bin directories
@rem From-> http://stackoverflow.com/questions/755382/i-want-to-delete-all-bin-and-obj-folders-to-force-all-projects-to-rebuild-everyth
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S bin') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S obj') DO RMDIR /S /Q "%%G"


SET ZIPPATH="C:\Program Files\WinRAR\WinRAR.exe"

set year=%date:~6,4%
set month=%date:~3,2%
set day=%date:~0,2%
set hour=%Time:~0,2%
if "%hour:~0,1%"==" " set hour=0%Time:~1,1%
set minute=%Time:~3,2%
set TIMESTAMP=%year%%month%%day%_%hour%%minute%


@rem Note that the _archive and _build foldes are excluded 
%ZIPPATH% a -afzip -r -x"_archive" -x"_build" "Driver_%TIMESTAMP%" "*.*"

