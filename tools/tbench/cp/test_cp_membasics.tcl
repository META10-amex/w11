# $Id: test_cp_membasics.tcl 1170 2019-06-22 20:58:52Z mueller $
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2014-2019 by Walter F.J. Mueller <W.F.J.Mueller@gsi.de>
#
# Revision History:
# Date         Rev Version  Comment
# 2019-06-22  1170   1.1    add membe tests for memory accesses
# 2014-03-02   552   1.0    Initial version
#
# Test very basic memory interface gymnastics
#  1. write/read address register
#  2. write/read memory via wm/wmi/rm/rmi (16 bit mode)
#  3. write/read memory via bwm/brm (16 bit mode)
#

# ----------------------------------------------------------------------------
rlc log "test_cp_membasics: Test very basic memory interface gymnastics ------"

# --------------------------------------------------------------------
rlc log "  write/read address register"

# test wal
$cpu cp -wal 002000 \
        -ral -edata 002000 \
        -rah -edata 000000

# test wah+wal
$cpu cp -wal 003000 \
        -wah 000001 \
        -ral -edata 003000 \
        -rah -edata 000001

# --------------------------------------------------------------------
rlc log "  write/read memory via wm/wmi/rm/rmi (16 bit mode)"

# simple write/read without increment
$cpu cp -wal 002000 \
        -wm  001100 \
        -ral -edata 002000 \
        -rah -edata 000000 \
        -rm  -edata 001100

# double write + single read, check overwrite
$cpu cp -wal 002000 \
        -wm  002200 \
        -wm  002210 \
        -ral -edata 002000 \
        -rah -edata 000000 \
        -rm  -edata 002210

# double write/read with increment
$cpu cp -wal 002100 \
        -wmi 003300 \
        -wmi 003310 \
        -wmi 003320 \
        -ral -edata 002106 \
        -rah -edata 000000

$cpu cp -wal 002100 \
        -rmi -edata 003300 \
        -rmi -edata 003310 \
        -rmi -edata 003320 \
        -ral -edata 002106 \
        -rah -edata 000000

# --------------------------------------------------------------------
rlc log "  write/read memory via bwm/brm (16 bit mode)"
$cpu cp -wal 02200 \
        -bwm {007700 007710 007720 007730}

$cpu cp -wal 02200 \
        -brm 4 -edata {007700 007710 007720 007730}

# --------------------------------------------------------------------
rlc log "  write/read memory - test membe (16 bit mode)"

# init 4 words
$cpu cp -wal           002300 \
        -bwm           {0xaaaa 0xbbbb 0xcccc 0xdddd} \
        -wal           002300 \
        -brm 4 -edata  {0xaaaa 0xbbbb 0xcccc 0xdddd}
# overwrite with membe '00' '01' '10' '11'
# verify that membe state not persistent (re-init to 11 after write
$cpu cp -wal           002300 \
        -wmembe        [bvi b2 "00"] \
        -rmembe -edata [bvi b3 "000"] \
        -wmi           0x0000 \
        -rmembe -edata [bvi b3 "011"] \
        -wmembe        [bvi b2 "01"] \
        -rmembe -edata [bvi b3 "001"] \
        -wmi           0x1111 \
        -rmembe -edata [bvi b3 "011"] \
        -wmembe        [bvi b2 "10"] \
        -rmembe -edata [bvi b3 "010"] \
        -wmi           0x2222 \
        -rmembe -edata [bvi b3 "011"] \
        -wmembe        [bvi b2 "11"] \
        -rmembe -edata [bvi b3 "011"] \
        -wmi           0x3333 \
        -rmembe -edata [bvi b3 "011"] \
        -wal           002300 \
        -brm 4 -edata  {0xaaaa 0xbb11 0x22cc 0x3333}
# verify thay wmembe without -sticky acts on single write only
$cpu cp -wal           002300 \
        -wmembe        [bvi b2 "01"] \
        -rmembe -edata [bvi b3 "001"] \
        -wmi           0x0000 \
        -rmembe -edata [bvi b3 "011"] \
        -wmi           0x9999 \
        -rmembe -edata [bvi b3 "011"] \
        -wal           002300 \
        -brm 4 -edata  {0xaa00 0x9999 0x22cc 0x3333}
# verify thay wmembe with -sticky is persistent
$cpu cp -wal           002300 \
        -wmembe        [bvi b2 "10"] -stick  \
        -rmembe -edata [bvi b3 "110"] \
        -wmi           0x4444 \
        -wmi           0x5555 \
        -wmi           0x6666 \
        -wmi           0x7777 \
        -rmembe -edata [bvi b3 "110"] \
        -wal           002300 \
        -brm 4 -edata  {0x4400 0x5599 0x66cc 0x7733} \
        -wmembe        [bvi b2 "11"] \
        -rmembe -edata [bvi b3 "011"]
