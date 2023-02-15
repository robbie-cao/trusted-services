# UEFI disk image creation instructions

This directory contains a set of UEFI disk images, formatted with a
protective MBR and GPT. The images are intended for test purposes.
For convenience, there is a C version of the disk image that can be
built into test deployments.

Tools used to create images were:

  gdisk
  sgdisk
  srec_cat (to generate C from binary image - http://srecord.sourceforge.net/)

 Steps to create:
 1. Create a file to represent an empty flash device. All data set to
    0xff (the normal erased value). Use ('bs' is block size, 'count' is
    number of blocks):

    dd if=/dev/zero bs=512 count=267 | tr "\000" "\377" >flash.img

 2. Create MBR+GPT using:

    gdisk flash.img
      use 'n' command to add partitions
      use 'c' to define partition names
      use 'w' to save to file

 3. Set partition GUID using (for partition 1):

    sgdisk --partition-guid=1:92f7d53b-127e-432b-815c-9a95b80d69b7 flash.img

 4. Create C code:

    srec_cat flash.img -Binary -o ref_partition_data.c -C-Array ref_partition_data -INClude

--------------

*Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause