#!/bin/bash
set -e

password="123456"
qemu_path=~/QEMULoongson
setting="
set arch mips
target remote localhost:50010
"
usage="
This is a script for UCAS's OS projects. USAGE:

    bash pass.sh [option]

Options:

    -q:     Compile the image and test in QEMU of Loongson.
            (Make sure there has been a visual \"disk\" file under your qemu_path.)
    -g:     Connect GDB with symbol-file under arch mips.
            (The fucking annoying and useless version info will not be printed.)
    -c:     Create \"pass.sh\" under \"~/\".
            (Bash it with the same usage. No need inputting \"sudo mount...\" after reconnection.)
        -n:     Connect net bridge tap0.
            (Bash it before using parameter \"-q\" in P5)
    -s:     Execute pktRxTx with parameter \"-m -2\" to send packages.
            (Bash with parameter \"-n\" first)
    -r:     Call tcpdump to check package recieving in tap0.
            (Bash with parameter \"-n\" first)
    -l:     Copy \"gzram\" file to qemu's bios directory.
    -f:     Make clean, make all and make floppy.
    -h:     Print this message.

Report bugs to \"<cacaogattoxy@gmail.com>\", although the author seldom checks his mailbox.XD
"
workpath=$(cd $(dirname $0); pwd)
remote="
#!/bin/bash
set -e

echo $password | sudo -S mount -t vboxsf share /mnt/shared
echo
cd $workpath
bash pass.sh \$1
"
guide="Use \`bash pass.sh -h' for a complete list of options."

if [ ! $# == 1 ] ; then
    echo -e "$usage"
    exit
fi
while getopts "qgcnsrlfh" opt; do
    case $opt in
        q)
          make clean
          make all
          echo
          cp image "$qemu_path"
          cd "$qemu_path"
          dd if=image of=disk conv=notrunc
          sh run_pmon.sh
          ;;
        g)
          echo -e "$setting" > gdb_settings
          gdb-multiarch -x gdb_settings --symbols=main --quiet
          rm -f gdb_settings
          ;;
        c)
          echo "$password" | sudo -S echo -e "$remote" > ~/pass.sh
          echo
          echo "[NOTE] If it is the first time you use this script, make sure the password and qemu_path has been set correctly."
          ;;
        n)
          echo "$password" | sudo -S brctl addbr br0
          echo "$password" | sudo -S tunctl -t tap0 -u stu
          echo "$password" | sudo -S brctl addif br0 tap0
          echo "$password" | sudo -S ifconfig tap0 up
          ;;
        s)
          cd ../pktRxTx/
          sudo ./pktRxTx -m 2
          ;;
        r)
          echo "$password" | sudo -S tcpdump -i tap0 -n -xx
          ;;
        l)
          cp gzram "$qemu_path/bios"
          ;;
        f)
          make clean
          make all
          echo
          make floppy
          ;;
        h)
          echo -e "$usage"
          ;;
        \?)
          echo -e "Unrecognized option '$1'."
          echo -e "$guide"
          ;;
    esac
done
