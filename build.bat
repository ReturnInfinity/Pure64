@echo off

REM This script assumes that 'nasm' is in your path

cd src/bootsectors
echo | set /p x=Building bmfs_mbr.sys - 
call nasm bmfs_mbr.asm -o bmfs_mbr.sys && (echo Success) || (echo Error!)
echo | set /p x=Building multiboot.sys - 
call nasm multiboot.asm -o multiboot.sys && (echo Success) || (echo Error!)
echo | set /p x=Building pxestart.sys - 
call nasm pxestart.asm -o pxestart.sys && (echo Success) || (echo Error!)
cd ../..

cd src/lib
echo | set /p x=Building dir.o - 
call gcc -c dir.c -o dir.o -I ../../include && (echo Success) || (echo Error!)
echo | set /p x=Building file.o - 
call gcc -c file.c -o file.o -I ../../include && (echo Success) || (echo Error!)
echo | set /p x=Building fs.o - 
call gcc -c fs.c -o fs.o -I ../../include && (echo Success) || (echo Error!)
echo | set /p x=Building misc.o - 
call gcc -c misc.c -o misc.o -I ../../include && (echo Success) || (echo Error!)
echo | set /p x=Building path.o - 
call gcc -c path.c -o path.o -I ../../include && (echo Success) || (echo Error!)
echo | set /p x=Building libpure64.a - 
call ar rcs libpure64.a dir.o file.o fs.o misc.o path.o && (echo Success) || (echo Error!)
cd ../..

cd src
echo | set /p x=Building pure64.o - 
call nasm pure64.asm -f elf64 -F dwarf -o pure64.o && (echo Success) || (echo Error!)
echo | set /p x=Building load.o - 
call gcc -c load.c -o load.o -I ../include && (echo Success) || (echo Error!)
echo | set /p x=Building pure64 - 
call ld load.o pure64.o -o pure64 -T pure64.ld && (echo Success) || (echo Error!)
echo | set /p x=Building pure64.sys - 
call objcopy -O binary pure64 pure64.sys && (echo Success) || (echo Error!)
cd ..
