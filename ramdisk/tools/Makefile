# begin_generated_IBM_copyright_prolog                             
#                                                                  
# This is an automatically generated copyright prolog.             
# After initializing,  DO NOT MODIFY OR MOVE                       
# ================================================================ 
#                                                                  
# Licensed Materials - Property of IBM                             
# Blue Gene/Q                                                      
# (C) Copyright IBM Corp. 2012 All Rights Reserved                 
# US Government Users Restricted Rights - Use,                     
# duplication or disclosure restricted by GSA ADP                  
# Schedule contract with IBM Corp.                                 
#                                                                  
# This software is available to you under the                      
# GNU General Public License (GPL).                                
#                                                                  
# ================================================================ 
#                                                                  
# end_generated_IBM_copyright_prolog                               
#
# Top level makefile for ramdisk/tools
#
# Author: Jay S. Bryant <jsbryant@us.ibm.com>
#
#
include ../../Make.rules

BASE_PATH=$(shell echo ${BGQ_DISTROFS_BASE_PATH} | awk 'BEGIN{FS="/"} ; {print $$NF}')
DIRS=$(shell find mlxFwUpdate/* -type d  | grep -ve "\.svn" )
MLXFWUPDATE_DIRS=$(addprefix ${BGQ_RAMDISK_PATH}/tools/, ${DIRS})
FILES=$(shell find mlxFwUpdate/* -type f \! -empty | grep -v Makefile | grep -ve "\.svn"; \
              find mlxFwUpdate/* -type l \! -empty | grep -v Makefile | grep -ve "\.svn")
MLXFWUPDATE_FILES=$(addprefix ${BGQ_RAMDISK_PATH}/tools/, ${FILES})

.PHONY: mlxFwUpdate ${MLXFWUPDATE_DIRS} ${MLXFWUPDATE_FILES} FORCE

default: install

install: MAKE_TOOLS_DIR ${MLXFWUPDATE_DIRS} ${MLXFWUPDATE_FILES} bgqDistrofsInstall.sh bgqDistrofsPostInstall.sh bgqPopulateInitramfs.sh bgqDistrofsUninstall.sh bgqDistrofsPreInstall.sh
	@${CP} ./*.sh ${BGQ_RAMDISK_PATH}/tools
	@${CP} ./*.rpmlist ${BGQ_RAMDISK_PATH}/tools
	@${CP} ./functions ${BGQ_RAMDISK_PATH}/tools
	chmod o+rx ${BGQ_RAMDISK_PATH}/tools/*.sh
	chmod o+r ${BGQ_RAMDISK_PATH}/tools/*

MAKE_TOOLS_DIR:
	@${MKDIR} -p ${BGQ_RAMDISK_PATH}/tools
	chmod o+rx ${BGQ_RAMDISK_PATH}/tools

# Create directories in the mlxFwUpdate directory.
${MLXFWUPDATE_DIRS}: | ${MAKE_TOOLS_DIR}
	@${MKDIR} -p $@
	@chmod 777 $@

${MLXFWUPDATE_FILES}: ${BGQ_RAMDISK_PATH}/tools/% : % | ${MLXFWUPDATE_DIRS}
	@${CP} -d $< $@

bgqDistrofsPostInstall.sh: bgqDistrofsPostInstall.sh.templ
	sed 's,BGQ_LINUX_PATH,${BGQ_LINUX_PATH},g ; s,BGQ_INSTALL_DIR,${BGQ_INSTALL_DIR},g ; s,BGQ_DISTROFS_BASE_PATH,${BGQ_DISTROFS_BASE_PATH},g ; s,BGQ_LINUX_OS_PATH,${BGQ_LINUX_OS_PATH},g ; s,BGQ_RAMDISK_PATH,${BGQ_RAMDISK_PATH},g ; s,BGQ_INITRAMFS_PATH,${BGQ_INITRAMFS_PATH},g ; s,BGQ_LINUX_KERNEL_VERSION,${BGQ_LINUX_KERNEL_VERSION},g ; s,BGQ_DISTRO_REPO_PATH,${BGQ_DISTRO_REPO_PATH},g; s,BGQ_DISTRO,${BGQ_DISTRO},g ' < bgqDistrofsPostInstall.sh.templ > bgqDistrofsPostInstall.sh
	@chmod a+x bgqDistrofsPostInstall.sh

bgqDistrofsInstall.sh: bgqDistrofsInstall.sh.templ
	sed 's,BGQ_DISTROFS_BASE_PATH,${BASE_PATH},g ; s,BGQ_RAMDISK_PATH,${BGQ_RAMDISK_PATH},g; s,BGQ_DISTRO_REPO_PATH,${BGQ_DISTRO_REPO_PATH},g; s,BGQ_DISTRO,${BGQ_DISTRO},g ' < bgqDistrofsInstall.sh.templ > bgqDistrofsInstall.sh
	@chmod a+x bgqDistrofsInstall.sh

bgqPopulateInitramfs.sh: bgqPopulateInitramfs.sh.templ
	sed  's,BGQ_INITRAMFS_PATH,${BGQ_INITRAMFS_PATH},g; s,BGQ_RAMDISK_PATH,${BGQ_RAMDISK_PATH},g; s,BGQ_LINUX_PATH,${BGQ_LINUX_PATH},g; s,BGQ_LINUX_KERNEL_VERSION,${BGQ_LINUX_KERNEL_VERSION},g; s,BGQ_DISTROFS_BASE_PATH,${BGQ_DISTROFS_BASE_PATH},g' < bgqPopulateInitramfs.sh.templ > bgqPopulateInitramfs.sh
	@chmod a+x bgqPopulateInitramfs.sh

bgqDistrofsUninstall.sh: bgqDistrofsUninstall.sh.templ
	sed 's,BGQ_LINUX_OS_PATH,${BGQ_LINUX_OS_PATH},g ; s,BGQ_INITRAMFS_PATH,${BGQ_INITRAMFS_PATH},g ; s,BGQ_INSTALL_DIR,${BGQ_INSTALL_DIR},g ; s,BGQ_RAMDISK_PATH,${BGQ_RAMDISK_PATH},g ' < bgqDistrofsUninstall.sh.templ > bgqDistrofsUninstall.sh
	@chmod a+x bgqDistrofsUninstall.sh

bgqDistrofsPreInstall.sh: bgqDistrofsPreInstall.sh.templ
	sed 's,BGQ_RAMDISK_PATH,${BGQ_RAMDISK_PATH},g ; s,BGQ_DISTROFS_BASE_PATH,${BGQ_DISTROFS_BASE_PATH},g; s,BGQ_DISTRO_REPO_PATH,${BGQ_DISTRO_REPO_PATH},g; s,BGQ_DISTRO,${BGQ_DISTRO},g ' < bgqDistrofsPreInstall.sh.templ > bgqDistrofsPreInstall.sh
	@chmod a+x bgqDistrofsPreInstall.sh

clean:
	@echo "Cleaning tools ..."
	@${RM} -rf ${BGQ_RAMDISK_PATH}/tools/*
	@${RM} -rf mlxFwUpdate/bin/*
	@${RM} -rf mlxFwUpdate/lib64/*
	@${RM} -rf mlxFwUpdate/usr/bin/*
	@${RM} -rf mlxFwUpdate/usr/lib64/*
	@${RM} -f bgqDistrofsPostInstall.sh	
	@${RM} -f bgqDistrofsPreInstall.sh
	@${RM} -f bgqDistrofsInstall.sh
	@${RM} -f bgqPopulateInitramfs.sh
	@${RM} -r bgqDistrofsUninstall.sh

distclean: clean
