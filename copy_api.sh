#!/bin/sh
SRCDIR=/home/staacke/git/RedPitaya
DESTDIR=/opt/redpitaya
HOST=192.168.131.250
USER=root

ssh $USER@$HOST systemctl stop redpitaya_scpi
ssh $USER@$HOST mount -o remount rw $DESTDIR
scp $SRCDIR/api/lib/librp.so $USER@$HOST:$DESTDIR/lib/
scp $SRCDIR/scpi-server/scpi-server $USER@$HOST:$DESTDIR/bin/
scp $SRCDIR/fpga/prj/classic/project/redpitaya.runs/impl_1/red_pitaya_top.bit $USER@$HOST:$DESTDIR/fpga/fpga_0.94.bit
ssh $USER@$HOST mount -o remount ro $DESTDIR
ssh $USER@$HOST systemctl reboot

