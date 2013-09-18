/*
 * kernelincludes.h
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#ifndef KERNELINCLUDES_H_
#define KERNELINCLUDES_H_

#include <linux/init.h>		/* Needed for the macros */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/crypto.h>
#include <linux/vmalloc.h>
#include <linux/err.h>
#include <linux/time.h>

typedef u64 uint_fast64_t;
typedef u32 uint_fast32_t;
typedef u16 uint_fast16_t;
typedef u8 uint_fast8_t;

#define KSHARC_MAJOR_VERSION 	"0"
#define KSHARC_MINOR_VERSION 	"0"
#define KSHARC_REVISION 		"1"

#define restrict

#define malloc vmalloc
#define free vfree

#endif /* KERNELINCLUDES_H_ */
