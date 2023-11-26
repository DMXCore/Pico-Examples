openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000; program build\\DmxRepeater.elf verify reset exit"
