#
# Centaurean Sharc for GNU/Linux kernel
# http://www.github.com/lesnake/ksharc
#
# Copyright (c) 2013, Pierre Mazein
# All rights reserved.
#
# This software is dual-licensed: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation. For the terms of this
# license, see http://www.gnu.org/licenses/gpl.html
#
# You are free to use this software under the terms of the GNU General
# Public License, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# Alternatively, you can license this library under a commercial
# license, see http://www.github.com/lesnake/ksharc for more
# information.
#
# 01/06/13 17:27
#

# Specify kernel version to work on
# If not specified, will use current kernel
ifndef UNAME-R
UNAME-R=$(shell uname -r)
endif

RELATIVEPATH = ../sharc/src

SHARCSRCLIST =\
block_footer.c byte_buffer.c dictionary.c hash_decode.c stream.c \
block_header.c encode.c hash_encode.c \
block_mode_marker.c footer.c\
buffers.c decode.c metadata.c block_encode.c block_decode.c
SHARCOBJLIST = $(SHARCSRCLIST:.c=.o)
SHARCSRCPATH = $(realpath $(PWD)/$(RELATIVEPATH))

KSHARCSRCPATH = $(realpath $(PWD))

# Nom du ou des modules créés
obj-m += sharc_tfm.o device.o
# Dépendances du module sharc
sharc_tfm-y += sharc.o $(addprefix $(RELATIVEPATH)/,$(SHARCOBJLIST))

ccflags-y += -I$(SHARCSRCPATH) -include $(KSHARCSRCPATH)/kernelincludes.h -D_FILE_OFFSET_BITS=64 -std=gnu99
# -fdata-sections -ffunction-sections -Wl,--gc-sections

patched :
	cd ../sharc &&\
	for i in ../patches/*.patch ; do \
	echo Applying $$i &&\
	patch -f -p1 < $$i || exit 1 ;\
	done ;
	touch patched

all: patched
	@echo $(sharc_tfm-y)
	make -C /lib/modules/$(UNAME-R)/build SHARC_USE_AS_LIBRARY=1 M=$(PWD) V=1 modules

.PHONY : precommitclean clean removepatches

clean:
	make -C /lib/modules/$(UNAME-R)/build SHARC_USE_AS_LIBRARY=1 M=$(PWD) clean
	make -C /lib/modules/$(UNAME-R)/build SHARC_USE_AS_LIBRARY=1 M=$(PWD)/$(RELATIVEPATH) clean

precommitclean: clean removepatches

removepatches:
	cd ../sharc &&\
	for i in ../patches/*.patch ; do \
	echo Applying $$i &&\
	patch -f -p1 -R < $$i || exit 1 ;\
	done ;
	rm patched
