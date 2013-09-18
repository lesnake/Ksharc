/*
 * Centaurean Sharc
 * http://www.centaurean.com/sharc
 *
 * Copyright (c) 2013, Guillaume Voirin
 * All rights reserved.
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation. For the terms of this
 * license, see http://www.gnu.org/licenses/gpl.html
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this library under a commercial
 * license, see http://www.centaurean.com/sharc for more
 * information.
 *
 * 01/06/13 13:08
 */

#include "kernelincludes.h"
#include <api.h>
#include <globals.h>

struct sharc_ctx {
	SHARC_COMPRESSION_MODE compressionMode;
	SHARC_ENCODE_OUTPUT_TYPE outputType;
	SHARC_BLOCK_TYPE blockType;

	sharc_stream *CompressionStream;
	sharc_stream *DecompressionStream;
};

/* Compression / Decompression */
int sharc_compress(struct crypto_tfm *tfm, const uint8_t *src, unsigned int slen, uint8_t *dst, unsigned int *dlen)
{
	struct sharc_ctx *ctx = crypto_tfm_ctx(tfm);

	SHARC_STREAM_STATE returnState;
	sharc_stream *stream ;

	stream = ctx->CompressionStream;
	memset(ctx->CompressionStream,0,sizeof(sharc_stream));

	if ((returnState = sharc_stream_prepare(stream, src, slen, dst, *dlen, vmalloc, vfree)))
		return -EIO;

	if ((returnState = sharc_stream_compress_init(stream, ctx->compressionMode, ctx->outputType, ctx->blockType, NULL)))
		return -EIO;

	if ((returnState = sharc_stream_compress(stream, true)))
		return -EIO;

	if ((returnState = sharc_stream_compress_finish(stream)))
		return -EIO;

	*dlen = stream->out_total_written;

	return 0;
} 

int sharc_decompress(struct crypto_tfm *tfm, const uint8_t *src, unsigned int slen, uint8_t *dst, unsigned int *dlen)
{
	struct sharc_ctx *ctx = crypto_tfm_ctx(tfm);
	SHARC_STREAM_STATE returnState;

	sharc_stream *stream ;
	stream = ctx->DecompressionStream;
	memset(ctx->DecompressionStream,0,sizeof(sharc_stream));

	if ((returnState = sharc_stream_prepare(stream, src, slen, dst, *dlen, vmalloc, vfree)))
		return -EIO;

	if ((returnState = sharc_stream_decompress_init(stream)))
		return -EIO;

	if ((returnState = sharc_stream_decompress(stream, true)))
		return -EIO;

	if ((returnState = sharc_stream_decompress_finish(stream)))
		return -EIO;

	*dlen = stream->out_total_written;

    return 0;
}

/* transfmormation API handling */

static int sharc_init(struct crypto_tfm *tfm)
{
	struct sharc_ctx *ctx = crypto_tfm_ctx(tfm);

	ctx->CompressionStream = (sharc_stream *)vmalloc(sizeof(sharc_stream));

	if (ctx->CompressionStream == NULL)
		return -ENOMEM;

	ctx->DecompressionStream = (sharc_stream *)vmalloc(sizeof(sharc_stream));

	if (ctx->DecompressionStream == NULL)
	{
		vfree(ctx->CompressionStream);
		return -ENOMEM;
	}

	ctx->compressionMode = SHARC_COMPRESSION_MODE_FASTEST;
	ctx->outputType = SHARC_ENCODE_OUTPUT_TYPE_WITHOUT_HEADER_NOR_FOOTER;
	ctx->blockType = SHARC_BLOCK_TYPE_DEFAULT;

	return 0;
}

static void sharc_exit(struct crypto_tfm *tfm)
{
	struct sharc_ctx *ctx = crypto_tfm_ctx(tfm);
	vfree(ctx->CompressionStream);
	vfree(ctx->DecompressionStream);
}

static struct crypto_alg alg = {
	.cra_name		= "sharc",
	.cra_flags		= CRYPTO_ALG_TYPE_COMPRESS,
	.cra_ctxsize	= sizeof(struct sharc_ctx),
	.cra_module		= THIS_MODULE,
	.cra_init		= sharc_init,
	.cra_exit		= sharc_exit,
	.cra_u			= { .compress = {
	.coa_compress 		= sharc_compress,
	.coa_decompress  	= sharc_decompress } }
};

/* Module handling */
static int __init sharc_mod_init (void)
{
	return crypto_register_alg(&alg);
}

static void __exit sharc_mod_exit (void)
{
	crypto_unregister_alg(&alg);
}

module_init(sharc_mod_init);
module_exit(sharc_mod_exit);

/* Pour que le parser ne me prenne pas la tête avec des
 * indicateurs de symboles non utilisés */
#ifdef __CDT_PARSER__
int main(void)
{
	__initcall_sharc_mod_init6();
	__exitcall_sharc_mod_exit();
	return 0;
}

#endif

#define STR_HELPER(x) #x
#define VAL(x) STR_HELPER(x)

//#define VAL(x) #x
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Mazein");	/* Who wrote this module? */
MODULE_DESCRIPTION("Based on Sharc v" VAL(SHARC_MAJOR_VERSION) "." VAL(SHARC_MINOR_VERSION) "." VAL(SHARC_REVISION));
MODULE_DESCRIPTION("Ksharc compression module v" KSHARC_MAJOR_VERSION "." KSHARC_MINOR_VERSION "." KSHARC_REVISION);
