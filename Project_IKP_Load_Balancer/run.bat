@echo off
setlocal

rem Definišemo putanje do izvršnih fajlova (ažurirano za x64\Debug)
set LB_EXEC=x64\Debug\Load_Balancer.exe
set CLIENT_EXEC=x64\Debug\Client.exe
set WORKER_EXEC=x64\Debug\Worker.exe

rem Proveravamo da li fajlovi postoje
if not exist "%LB_EXEC%" (
    echo Load Balancer nije pronađen na "%LB_EXEC%"
    pause
    exit /b
)
if not exist "%CLIENT_EXEC%" (
    echo Client nije pronađen na "%CLIENT_EXEC%"
    pause
    exit /b
)
if not exist "%WORKER_EXEC%" (
    echo Worker nije pronađen na "%WORKER_EXEC%"
    pause
    exit /b
)

rem Pokrećemo Load Balancer
start cmd /k "%LB_EXEC%"

rem Pauza od 2 sekunde
timeout /t 2 /nobreak

rem Pokrećemo 2 klijenta
start cmd /k "%CLIENT_EXEC%"
start cmd /k "%CLIENT_EXEC%"

rem Pokrećemo 3 workera
start cmd /k "%WORKER_EXEC%"
start cmd /k "%WORKER_EXEC%"
start cmd /k "%WORKER_EXEC%"

echo Svi procesi su pokrenuti.
pause