/*
 * Wmapro compatible decoder, with a flush callback for the xmaframes decoder
 * Copyright (c) 2007 Baptiste Coudurier, Benjamin Larsson, Ulion
 * Copyright (c) 2008 - 2011 Sascha Sommer, Benjamin Larsson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Builds libavcodec/wmaprodec.c in place of the submodule's copy and replaces
 * its ff_xmaframes_decoder with one that has a flush callback. The submodule's
 * xmaframes decoder declares none, so avcodec_flush_buffers() cannot clear the
 * overlap-add tail and per-channel history it keeps between frames, and the
 * only way to start a new stream was to reopen the codec.
 */

#include <stddef.h>

#define ff_xmaframes_decoder ff_xmaframes_decoder_without_flush
#include "libavcodec/wmaprodec.c"
#undef ff_xmaframes_decoder

/**
 *@brief Start a new stream: the next frame decodes as the first one after
 *       xmaframes_decode_init would, without rebuilding the MDCT tables.
 *@param avctx codec context
 */
static void xmaframes_flush(AVCodecContext *avctx)
{
    WMAProDecodeCtx *s = avctx->priv_data;
    int i;

    flush(s);
    /** everything ahead of out is per-stream state that decode_init leaves
        zeroed, except the previous block length, which starts at a whole
        frame */
    for (i = 0; i < s->nb_channels; i++) {
        memset(&s->channel[i], 0, offsetof(WMAProChannelCtx, out));
        s->channel[i].prev_block_len = s->samples_per_frame;
    }
}

AVCodec ff_xmaframes_decoder = {
    .name           = "xmaframes",
    .long_name      = NULL_IF_CONFIG_SMALL("Xbox Media Audio raw frames"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_XMAFRAMES,
    .priv_data_size = sizeof(WMAProDecodeCtx),
    .init           = xmaframes_decode_init,
    .close          = xmaframes_decode_end,
    .decode         = xmaframes_decode_packet,
    .flush          = xmaframes_flush,
    .capabilities   = AV_CODEC_CAP_DR1,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
    .sample_fmts    = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                      AV_SAMPLE_FMT_NONE },
};
